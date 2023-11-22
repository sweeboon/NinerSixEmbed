add_executable(main
    main.c
)

target_link_libraries(main
    pico_stdlib
    hardware_pwm
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_adc
)
target_compile_definitions(main PRIVATE
    WIFI_SSID="SB"
    WIFI_PASSWORD="bogt7083"
    TEST_TCP_SERVER_IP="192.168.236.70"
)
target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
)

pico_add_extra_outputs(main)
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 1)

# add url via pico_set_program_url
example_auto_set_url(main)
