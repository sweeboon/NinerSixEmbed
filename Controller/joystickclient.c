#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "hardware/adc.h"

#define JOYSTICK_X_PIN 26 // ADC0, which is GP26 on the Pico
#define JOYSTICK_Y_PIN 27 // ADC1, which is GP27 on the Pico
#define TCP_PORT 4242
#define BUF_SIZE 1024

void init_joystick() {
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
}

void read_joystick(uint16_t *x, uint16_t *y) {
    adc_select_input(0); // X axis
    *x = adc_read();
    adc_select_input(1); // Y axis
    *y = adc_read();
}

void tcp_client_error(void *arg, err_t err) {
    printf("TCP Client Error: %d\n", err);
    if (err == ERR_ABRT || err == ERR_RST) {
        printf("TCP connection was aborted or reset.\n");
    } else {
        printf("Unexpected TCP error occurred.\n");
    }
    // You can add more error handling logic here if needed
}

err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err) {
    if (err != ERR_OK) {
        printf("Connection error: %d\n", err);
        return err;
    }

    printf("Connected to TCP server.\n");

    // Send joystick data once connected
    uint16_t x, y;
    read_joystick(&x, &y);

    // Print joystick position before sending
    // printf("Joystick Position - X: %u, Y: %u\n", x, y);

    char buffer[BUF_SIZE];
    snprintf(buffer, sizeof(buffer), "X: %u, Y: %u\n", x, y);
    tcp_write(tpcb, buffer, strlen(buffer), TCP_WRITE_FLAG_COPY);
    return ERR_OK;
}

void send_joystick_data(struct tcp_pcb *tpcb) {
    uint16_t x, y;
    read_joystick(&x, &y);

   // Serialize the x and y values into a byte stream
    uint8_t buffer[sizeof(uint16_t) * 2];
    memcpy(buffer, &x, sizeof(uint16_t));
    memcpy(buffer + sizeof(uint16_t), &y, sizeof(uint16_t));
    
    // Ensure there is enough space in the send buffer
    if (tcp_sndbuf(tpcb) >= sizeof(buffer)) {
        tcp_write(tpcb, buffer, sizeof(buffer), TCP_WRITE_FLAG_COPY);
    }
}

struct tcp_pcb *setup_tcp_client() {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Error creating PCB.\n");
        return NULL;
    }

    ip_addr_t remote_addr;
    ipaddr_aton(TEST_TCP_SERVER_IP, &remote_addr);
    tcp_connect(pcb, &remote_addr, TCP_PORT, tcp_client_connected);

    // Register the error callback
    tcp_err(pcb, tcp_client_error);

    return pcb;
}


int main() {
    stdio_init_all();

    // Initialize CYW43 (WiFi driver)
    if (cyw43_arch_init()) {
        printf("Failed to initialize CYW43.\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    // Connect to WiFi using the defined macros
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000) != 0) {
        printf("Failed to connect to Wi-Fi.\n");
        return 1;
    }
    printf("Connected to Wi-Fi.\n");

    // Initialize joystick
    init_joystick();

    // Setup TCP client
    struct tcp_pcb *tcp_client_pcb = setup_tcp_client();
    if (!tcp_client_pcb) {
        printf("TCP client setup failed.\n");
        cyw43_arch_deinit();
        return 1;
    }

    while (1) {
        tight_loop_contents(); // Essential for proper operation of the TCP stack

       if (tcp_client_pcb != NULL) {
            send_joystick_data(tcp_client_pcb);
        }

        sleep_ms(50); // Add a delay to control the rate of data sending
    }

    cyw43_arch_deinit();
    return 0;
}

