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
    CountPulsesWithPause(uint input)
    {

	// initialize local counts
        _current_pulse_count = 0;
        _cumulative_pulse_count = 0;

        // pio 0 is used
        pio = pio0;

        // state machine 0
        sm = 0;

        // configure the used pin
        pio_gpio_init(pio, input);

        // load the pio program into the pio memory
        uint offset = pio_add_program(pio, &count_pulses_with_pause_program);

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
    void read_pulses(void)
    {
         // wait for the FIFO to contain a data item
        while (pio_sm_get_rx_fifo_level(pio, sm) < 1) {
            sleep_ms(10);
            printf("waiting around\n");
        }
        // whle the fifo has entries, consume them, keeping track of count of pulses
        do {
            printf("Reading\n");
            _current_pulse_count = pio_sm_get(pio, sm);
            _cumulative_pulse_count += _current_pulse_count;
        } while (pio_sm_get_rx_fifo_level(pio, sm) > 0);
    }

    uint32_t get_current() {
        return _current_pulse_count;
    }

    uint32_t get_cumulative() {
        return _cumulative_pulse_count;
    }

    void clear_queues() {
        pio_sm_clear_fifos(pio, sm);
    }



private:
    // the pio instance
    PIO pio;
    // the state machine
    uint sm;
    __uint32_t _current_pulse_count;
    __uint32_t _cumulative_pulse_count;

};


// pin to monitor for pulses
#define STEP_PIN 28

int main()
{
    // needed for printf
    stdio_init_all();
 
    printf("Place Holder\n\n");
    sleep_ms(1000);

    // the instance of the CountPulsesWithPause.
    //  Note the input pin is 28 in this example
    CountPulsesWithPause pulse_counter(STEP_PIN);

    // clear the FIFO: before starting measurement
    pulse_counter.clear_queues();



    // infinite loop to print pulse measurements
    while (true)
    {
        printf("Good Opening\n");
        pulse_counter.read_pulses();
        uint current_count = pulse_counter.get_current();
        uint total_count = pulse_counter.get_cumulative();

        printf("current count of pulses = %zu, \n total count of pulses = %zu\n\n", current_count, total_count);
    }
 
}
