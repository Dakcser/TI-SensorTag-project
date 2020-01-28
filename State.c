#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/i2c/I2CCC26XX.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/mw/display/Display.h>
#include <ti/mw/display/DisplayExt.h>
#include <string.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <driverlib/timer.h>
/* Board Header files */
#include "Board.h"
#include <ti/drivers/UART.h>
#include "wireless/comm_lib.h"
#include "sensors/bmp280.h"
#include "sensors/tmp007.h"
#include "sensors/mpu9250.h"
#include "buzzer.h"
#include "State.h"




void start_statemachine(int i) {
    
    step_state(START_LOOPING);
    step_state(PRINT_HELLO);
    
}


void step_state(enum events event) {
    
    switch(state) {
        
        case START:
            switch(event) {
            case START_LOOPING:
                state = LOOP;
                break;
            default:
                exit(1);
                break;
            }       
            break;
        case LOOP:
            switch(event) {
            case PRINT_HELLO:
                printf("Statemachine is working!\n");
                System_flush();
                
                break;
            case STOP_LOOPING:
                state = END;
                break;
            default:
                exit(1);
                break;
            }
            break;
        case END:
        exit(1);
        break;
    }
}
