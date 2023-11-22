#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// Define the pins for the joystick
#define JOYSTICK_X_PIN 26 // ADC0, which is GP26 on the Pico
#define JOYSTICK_Y_PIN 27 // ADC1, which is GP27 on the Pico
#define SLIGHT_TILT_MOD 500// For slight tilting
#define MINOR_TILT_MOD 500 //test if its viable

void setup() {
    stdio_init_all(); // Initialize all standard IO including USB serial
    adc_init();       // Initialize the ADC

    // Initialize ADC input for the joystick's X axis (GP26)
    adc_gpio_init(JOYSTICK_X_PIN);

    // Initialize ADC input for the joystick's Y axis (GP27)
    adc_gpio_init(JOYSTICK_Y_PIN);
}

void loop() {
    // Read the raw joystick X and Y values
    adc_select_input(0); // Select ADC0 for the X axis
    uint16_t x = adc_read(); //variable to hold X Axis

    adc_select_input(1); // Select ADC1 for the Y axis
    uint16_t y = adc_read(); //variable to hold Y axis

    // Output the X and Y values over the USB serial connection
    printf("Joystick X: %u, Joystick Y: %u\n", x, y);
    // printf("Joystick at neutral\n");
    // // If loops to check if its tilting diagonal or leftrightupdown
    
    // //direction if loops,replace print statements with variables to send to the car
    // if(x>2110){
    //     printf("Joystick is tilting right\n");
    //     if(x>2110+SLIGHT_TILT_MOD){
    //         printf("Joystick is tilting slightly right\n");
    //         if(x>4094){
    //             printf("Joystick is tilting full right\n");
    //         }
    //     }

    // }
    // if(x< 2000){
    //     printf("Joystick is tilting left\n");
    //     if(x<2000-SLIGHT_TILT_MOD){
    //         printf("Joystick is tilting slightly left\n");
    //         if(x<20){
    //             printf("Joystick is tilting full left\n");
    //         }
    //     }
    // }
    // if(y>2110){
    //     printf("Joystick is tliting up\n");
    //     if(y>2110+SLIGHT_TILT_MOD){
    //         printf("Joystick is tiltight slightly up\n");
    //         if(y>4094){
    //             printf("Joystick is tilting full up\n");
    //         }
    //     }
    // }
    // if (y<2000){
    //     printf("Joystick is tliting down\n");
    //     if(y<2000-SLIGHT_TILT_MOD){
    //         printf("Joystick is tilting slightly down\n");
    //         if(y<20){
    //             printf("Joystick is tilting full down\n");
    //         }
    //     }
    // }
    // if(x>2110 && y>2110){
    //     printf("Joystick is tlting top right\n");
    //     if(x>2110+SLIGHT_TILT_MOD && y>2110+SLIGHT_TILT_MOD){
    //         printf("Joystick is tilting slightly to top right\n");
    //         if(x>4094 && y>4094){
    //         printf("Joystick is tilting full top right\n");
    //         }
    //     }
    //  }
    //  if(x<2000 && y>2110){
    //     printf("Joystick is tlting top left\n");
    //     if(x<2000-SLIGHT_TILT_MOD && y>2110+SLIGHT_TILT_MOD){
    //         printf("Joystick is tilting slightly to top left\n");
    //         if(x<20 && y>4094){
    //         printf("Joystick is tilting full top left\n");
    //         }
    //     }
    //  }
    //  if(x>2110 && y<2000){
    //     printf("Joystick is tlting bottom right\n");
    //     if(x>2110+SLIGHT_TILT_MOD && y<2000-SLIGHT_TILT_MOD){
    //         printf("Joystick is tilting slightly to bottom right\n");
    //         if(x>4094 && y<20){
    //         printf("Joystick is tilting full bottom right\n");
    //         }
    //     }
    //  }

    //  if(x<2000 && y<2000){
    //     printf("Joystick is tlting bottom left\n");
    //     if(x<2000-SLIGHT_TILT_MOD && y<2000-SLIGHT_TILT_MOD){
    //         printf("Joystick is tilting slightly to bottom left\n");
    //         if(x<20 && y<20){
    //         printf("Joystick is tilting full bottom left\n");
    //         }
    //     }
    //  }
        //send it here
        
    
    
    sleep_ms(50); // Delay between reads to not overload the CPU
}

int main() {
    setup();
    while (true) {
        loop();
    }
    return 0;
}