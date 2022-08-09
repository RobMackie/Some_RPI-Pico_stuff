add_executable(count_pulses_with_pause)

pico_generate_pio_header(count_pulses_with_pause ${CMAKE_CURRENT_LIST_DIR}/count_pulses_with_pause.pio)

target_sources(count_pulses_with_pause PRIVATE count_pulses_with_pause.cpp)

target_link_libraries(count_pulses_with_pause PRIVATE
        pico_stdlib
        hardware_pio
        )

# enable usb output, disable uart output
pico_enable_stdio_usb(count_pulses_with_pause 1)
pico_enable_stdio_uart(count_pulses_with_pause 0)

pico_add_extra_outputs(count_pulses_with_pause)

# add url via pico_set_program_url
# example_auto_set_url(count_pulses_with_pause)
