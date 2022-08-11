#include <stdio.h>

#include "pico/stdlib.h"


#include "hardware/irq.h"
#include "hardware/pio.h"

#include "count_pulses_with_pause.pio.h"

/*
This class can be used for protocols where the data is encoded by a number of pulses in a pulse train followed by a pause.
E.g. the LMT01 temperature sensor uses this (see https://www.reddit.com/r/raspberrypipico/comments/nis1ew/made_a_pulse_counter_for_the_lmt01_temperature/)

The class itself only starts the state machine and, when called, read_pulses() gives the data the state machine has put in the Rx FIFO
*/

class CountPulsesWithPause
{
public:
    // input = pin that receives the pulses.
    CountPulsesWithPause(uint input, uint sm_select)
    {

	// initialize local counts
        _current_pulse_count = 0;
        _cumulative_pulse_count = 0;
        _report_count = 0;

        // pio 0 is used
        pio = pio0;

        // state machine 0
        sm = sm_select;

        // configure the used pin
        pio_gpio_init(pio, input);

        if (!is_program_loaded) 
        {
            // load the pio program into the pio memory if it isn't already loaded
            offset = pio_add_program(pio, &count_pulses_with_pause_program);
            is_program_loaded = true;
        }

        // make a sm config
        pio_sm_config c = count_pulses_with_pause_program_get_default_config(offset);

        // set the 'jmp' pin
        sm_config_set_jmp_pin(&c, input);


        // set the 'wait' pin (uses 'in' pins)
        sm_config_set_in_pins(&c, input);

        // set shift direction
        sm_config_set_in_shift(&c, false, false, 0);

        // init the pio sm with the config
        pio_sm_init(pio, sm, offset, &c);

        // enable the sm
        pio_sm_set_enabled(pio, sm, true);

    }

    // read the number of pulses in a pulse train
    auto read_pulses(void) -> bool
    {
        _current_pulse_count = 0;
         // wait for the FIFO to contain a data item
        if  (pio_sm_get_rx_fifo_level(pio, sm) < 1) {
            return false;
        }
        // whle the fifo has entries, consume them, keeping track of count of pulses
        do {
            _current_pulse_count += pio_sm_get(pio, sm);
            _report_count++;
        } while (pio_sm_get_rx_fifo_level(pio, sm) > 0);
        _cumulative_pulse_count += _current_pulse_count;
        return true;
    }

    auto get_current() -> uint32_t {
        return _current_pulse_count;
    }

    auto get_cumulative() -> uint32_t {
        return _cumulative_pulse_count;
    }

    auto get_report_count() -> uint32_t {
        return _report_count;
    }

    void clear_queues() {
        pio_sm_clear_fifos(pio, sm);
    }

private:
    // the pio instance
    PIO pio;
    // the state machine
    uint sm;
    uint32_t _current_pulse_count;
    uint32_t _cumulative_pulse_count;
    uint32_t _report_count;

// class level members
    static bool is_program_loaded;
    static uint offset;
};

bool CountPulsesWithPause::is_program_loaded = false;
uint CountPulsesWithPause::offset = 0;

// pin to monitor for pulses
#define STEP_PIN_UP 6
#define STEP_PIN_DN 7

int main()
{
    // needed for printf
    stdio_init_all();
 
    printf("Place Holder\n\n");
    sleep_ms(1000);

    // the instance of the CountPulsesWithPause.
    //  Note the input pin is 28 in this example
    CountPulsesWithPause pulse_counter_up(STEP_PIN_UP, 0);
    CountPulsesWithPause pulse_counter_down(STEP_PIN_DN, 1);

    // clear the FIFO: before starting measurement
    pulse_counter_up.clear_queues();
    pulse_counter_down.clear_queues();

    uint32_t current_count_up = 0;
    uint32_t total_count_up = 0;
    uint32_t report_count_up = 0;

    uint32_t current_count_down = 0;
    uint32_t total_count_down = 0;
    uint32_t report_count_down = 0;

    // infinite loop to print pulse measurements
    while (true)
    {
        if (pulse_counter_up.read_pulses()) {
            current_count_up = pulse_counter_up.get_current();
            total_count_up = pulse_counter_up.get_cumulative();
            report_count_up = pulse_counter_up.get_report_count();
            printf("X: UP: current report# = %zu\ncurrent count of pulses = %zu,\n   total count of pulses = %zu\n\n", 
                    report_count_up, current_count_up, total_count_up);
        } 
        if (pulse_counter_down.read_pulses()) {
            current_count_down = pulse_counter_down.get_current();
            total_count_down = pulse_counter_down.get_cumulative();
            report_count_down = pulse_counter_down.get_report_count();
            printf("X: DN: current report# = %zu\ncurrent count of pulses = %zu,\n   total count of pulses = %zu\n\n", 
                    report_count_down, current_count_down, total_count_down);
        }         
        
        printf("Sleeping before polling for data again\n");
        sleep_ms(1000);

    }
 
}
