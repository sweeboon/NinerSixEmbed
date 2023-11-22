add_executable(controller
    controller.c
        )

target_link_libraries(controller pico_stdlib hardware_pwm hardware_adc)

# enable usb output, enable uart output
pico_enable_stdio_usb(controller 1)
pico_enable_stdio_uart(controller 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(controller)

# add url via pico_set_program_url
example_auto_set_url(controller)
