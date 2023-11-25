#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "ultrasonic.h"

volatile bool encoder_event_left = false;
volatile bool encoder_event_right = false;
volatile u_int64_t encoder_count_left = 0;
volatile u_int64_t encoder_count_right = 0;
volatile uint64_t last_time = 0;
volatile uint64_t speed = 0;
volatile int controller_x = 0;
volatile int controller_y = 0;

#define WIFI_SSID "SB"
#define WIFI_PASSWORD "bogt7083"
#define TCP_PORT 4242
#define BUF_SIZE 2048

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 16
#define UART_RX_PIN 17

#define MOTOR_A_IN1 4
#define MOTOR_A_IN2 5
#define MOTOR_B_IN3 6
#define MOTOR_B_IN4 7
#define ECHO_PIN 8
#define TRIG_PIN 9

#define SPEED1 1
#define SPEED2 2.5
#define SPEED3 3
#define SPEED4 5

#define GPIODirChange 18
#define GPIOForward 19

static err_t on_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    } else {
        if (p->len == sizeof(uint16_t) * 2) {
            uint16_t x, y;
            memcpy(&x, p->payload, sizeof(uint16_t));
            memcpy(&y, (uint8_t*)p->payload + sizeof(uint16_t), sizeof(uint16_t));
            
            printf("Joystick Position - X: %u, Y: %u\n", x, y);
            controller_x = (int) x;
            controller_y = (int) y;
        }

        tcp_recved(tpcb, p->len);
        pbuf_free(p);
        return ERR_OK;
    }
}

static err_t on_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, on_recv);
    return ERR_OK;
}

void setupAllPins() {
    // Ultrasonic setup on GPIO 8 and 9
    gpio_init(TRIG_PIN);
    gpio_init(ECHO_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_set_dir(ECHO_PIN, GPIO_IN);

    // Encoder setup on GPIO 2 and 3
    gpio_init(2);
    gpio_set_dir(2, GPIO_IN);
    gpio_pull_up(2);
    gpio_init(3);
    gpio_set_dir(3, GPIO_IN);
    gpio_pull_up(3);

    // UART initialization for debugging
    uart_init(UART_ID, 115200);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, false);
    
    // Motor PWM setup on GPIO 0 and 1
    gpio_set_function(0, GPIO_FUNC_PWM);
    gpio_set_function(1, GPIO_FUNC_PWM);

    // Direction control init
    gpio_init(MOTOR_A_IN1);
    gpio_set_dir(MOTOR_A_IN1, GPIO_OUT);
    gpio_init(MOTOR_A_IN2);
    gpio_set_dir(MOTOR_A_IN2, GPIO_OUT);

    gpio_init(MOTOR_B_IN3);
    gpio_set_dir(MOTOR_B_IN3, GPIO_OUT);
    gpio_init(MOTOR_B_IN4);
    gpio_set_dir(MOTOR_B_IN4, GPIO_OUT);

    //Set Initial Direction
    gpio_put(MOTOR_A_IN1, 1);
    gpio_put(MOTOR_A_IN2, 0);
    gpio_put(MOTOR_B_IN3, 1);
    gpio_put(MOTOR_B_IN4, 0);
}

void setMotorDirection(bool forward) {
    if (forward) {
        gpio_put(MOTOR_A_IN1, 0);
        gpio_put(MOTOR_A_IN2, 1);
        gpio_put(MOTOR_B_IN3, 0);
        gpio_put(MOTOR_B_IN4, 1);
    } else {
        gpio_put(MOTOR_A_IN1, 1);
        gpio_put(MOTOR_A_IN2, 0);
        gpio_put(MOTOR_B_IN3, 1);
        gpio_put(MOTOR_B_IN4, 0);
    }
}

void moveReverse(){
    uint slice_num = pwm_gpio_to_slice_num(0);
    setMotorDirection(false);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 14000 * SPEED4 / 2);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500 * SPEED4 / 2);
}
void moveForward() {
    uint slice_num = pwm_gpio_to_slice_num(0);
    setMotorDirection(true);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 14000 * SPEED4 / 2);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500 * SPEED4 / 2);
}

void stop() {
    uint slice_num = pwm_gpio_to_slice_num(0);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 0);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 0);
}

void turnLeft() {
    setMotorDirection(true);
    uint slice_num = pwm_gpio_to_slice_num(0);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 0); // Stop left wheel
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500 * SPEED4 / 2); // Speed up right wheel
}

void turnRight() {
    setMotorDirection(true);
    uint slice_num = pwm_gpio_to_slice_num(0);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 14000 * SPEED4 / 2); // Speed up left wheel
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 0); // Stop right wheel
}

void controller_logic(int controller_x, int controller_y) {
    if (controller_x > 3000) {
        // Turn right
        turnRight();
        sleep_ms(1000);
    } else if (controller_x < 1000) {
        // Turn left
        turnLeft();
        sleep_ms(1000);
    } else if(controller_y < 1000) {
        // Move backward
        moveReverse();
        sleep_ms(1000);
    } else if(controller_y>3000){
        // Move forward
        moveForward();
        sleep_ms(1000);
    }
    else {
        // Move forward
        moveForward();
    }
}

void resetAfterAvoidance() {
    encoder_count_left = 0;
    encoder_count_right = 0;
    last_time = 0;
    speed = 0;
}

bool ultrasonic_callback(){
    uint64_t distanceCm = getCm(TRIG_PIN, ECHO_PIN);
    // printf("Distance: %llu cm\n", distanceCm);
    if (distanceCm < 10) {
        printf("Executing Avoidance\n");
        turnLeft();
        sleep_ms(500);
        moveForward();
        sleep_ms(1000);
        turnRight();
        sleep_ms(500);
        moveForward();
        sleep_ms(1000);
        turnRight();
        sleep_ms(500);
        moveForward();
        sleep_ms(1000);
        turnLeft();
        sleep_ms(500);
        moveForward();
        return true;
    } else {
        sleep_ms(50);
        return false;
    }
}

void encoder_callback(uint gpio, uint32_t events) {
    if (gpio == 2) {
        // Increment left encoder count
        encoder_count_left++;
        encoder_event_left = true;
    } else if (gpio == 3) {
        // Increment right encoder count
        encoder_count_right++;
        encoder_event_right = true;
    }
}

int main() {
    stdio_init_all();
    stdio_usb_init();
    setupAllPins();

    if (cyw43_arch_init()) {
        printf("Failed to initialize CYW43.\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000) != 0) {
        printf("Failed to connect to Wi-Fi.\n");
        return 1;
    }
    printf("Connected to Wi-Fi.\n");

    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Error creating PCB.\n");
        return 1;
    }

    if (tcp_bind(pcb, IP_ADDR_ANY, TCP_PORT) != ERR_OK) {
        printf("Unable to bind to port %d.\n", TCP_PORT);
        return 1;
    }

    pcb = tcp_listen(pcb);
    tcp_accept(pcb, on_accept);

    printf("TCP server listening on port %d.\n", TCP_PORT);

    //For demo
    gpio_init(GPIODirChange);
    gpio_set_dir(GPIODirChange, GPIO_IN);

    uint slice_num = pwm_gpio_to_slice_num(0);
    pwm_set_clkdiv(slice_num, 100);
    pwm_set_wrap(slice_num, 12500);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 12500 * SPEED1 / 2); // Adjust Speed Here
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500 * SPEED4 / 2); // Adjust Speed Here
    pwm_set_enabled(slice_num, true);

    // gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE, true, &encoder_callback);
    // gpio_set_irq_enabled_with_callback(3, GPIO_IRQ_EDGE_RISE, true, &encoder_callback);

    uint64_t last_print_time = 0;

    while(1){
        cyw43_arch_poll();
        sleep_ms(10);

        if (encoder_event_left || encoder_event_right) {
            uint64_t current_time = time_us_64();
            uint64_t time_diff = current_time - last_time;
            if (time_diff > 0) {
                speed = (encoder_count_right * 1.0e6) / (float)time_diff;
            }
            last_time = current_time;
            encoder_event_left = false;
            encoder_event_right = false;
        }
        if (time_us_64() - last_print_time > 1000000) {
            printf("Left count: %d, Right count: %d, Speed: %f counts/sec\n", encoder_count_left, encoder_count_right, speed);
            last_print_time = time_us_64();
        }

        if (ultrasonic_callback() == true){
            printf("Obstacle detected\n");
        } else {
            moveForward();
            // encoder_callback();
            printf("Controller X: %d, Controller Y: %d\n", controller_x, controller_y);
            controller_logic(controller_x, controller_y);
        }

        sleep_ms(10);
    }

    cyw43_arch_deinit();
    return 0;
}