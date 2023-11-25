
add_executable(joystickclient
joystickclient.c
        )
target_compile_definitions(joystickclient PRIVATE
        WIFI_SSID="SB"
        WIFI_PASSWORD="bogt7083"
        TEST_TCP_SERVER_IP="192.168.30.70"
        )
target_include_directories(joystickclient PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )
target_link_libraries(joystickclient
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        hardware_adc
        )

pico_add_extra_outputs(joystickclient)
pico_enable_stdio_usb(joystickclient 1)
