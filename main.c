/*

Students:

Emil Dark           2564926 	 	 	darkemil@student.oulu.fi
Lauri Klemettilä    2591012             lare.klemettila@gmail.com

*/
/* Usual lib*/
#include <stdio.h>
#include <math.h>

/* XDCtools files */
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

/* Board Header files */
#include "Board.h"
#include "wireless/comm_lib.h"
#include <driverlib/aon_batmon.h>

/*   SENSORS   */
#include "sensors/tmp007.h"
#include "sensors/bmp280.h"
#include "sensors/mpu9250.h"
#include "buzzer.h"

/* Task */
#define STACKSIZE 2048
Char labTaskStack[STACKSIZE];
Char commTaskStack[STACKSIZE];
Char sensorTaskStack[STACKSIZE];
Char displayTaskStack[STACKSIZE];

/* Display */
Display_Handle hDisplayLcd;

/* Pin configuration and variables */
static PIN_Handle buttonHandle;
static PIN_State buttonState;

static PIN_Handle hButtonShut;
static PIN_State bStateShut;

static PIN_Handle buzzerHandle;
static PIN_State buzzerState;

static PIN_Handle ledHandle;
static PIN_State ledState;

static PIN_Handle hMpuPin;
static PIN_State MpuPinState;


/* Global variables */
char lampo[16];
char press[16];
char hi5count[16] = "Highfives: 0";
char wavecount[16]= "Handwaves: 0";
char payload[16] = "None";
char lahettaja[16] = "None";
double pres ;
double temp ;
double TMPtemp;
uint16_t senderAddr = 0x0000;
int viesti = 0;
int check = 1;

enum state { 
        MAIN=1, 
        gesture, 
        INFO, 
        READ_Message,
};


enum state myState = MAIN;

/*Battery variables*/
char batteryLevel[6];
int batteryInt;
int batteryFrac;
int REG_BAT_OFFSET = 0x00000028; //Not sure if correct address, missing documentation

// LEFT Button is used for ON / OFF
PIN_Config buttonShut[] = {
   Board_BUTTON1 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
   PIN_TERMINATE
};
PIN_Config buttonWake[] = {
   Board_BUTTON1 | PIN_INPUT_EN | PIN_PULLUP | PINCC26XX_WAKEUP_NEGEDGE,
   PIN_TERMINATE
};

// RIGHT Button is used to change state machine
PIN_Config buttonConfig[] = {
   Board_BUTTON0  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
   PIN_TERMINATE
};

// Led1
PIN_Config ledConfig[] = {
   Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX, 
   PIN_TERMINATE // MÃ¤Ã¤ritys lopetetaan aina tÃ¤hÃ¤n vakioon
};

// Buzzer
PIN_Config buzzerConfig[] = {
    Board_BUZZER     | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE 
};


static PIN_Config MpuPinConfig[] = {
    Board_MPU_POWER  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

// MPU9250 I2C CONFIG
static const I2CCC26XX_I2CPinCfg i2cMPUCfg = {
    .pinSDA = Board_I2C0_SDA1,
    .pinSCL = Board_I2C0_SCL1
};

//______________________________________________________________________________
void soita_musiikkia(int ele){
    // NOTES 
    uint16_t Bb5    = 932;
  //uint16_t C5     = 523;
  //uint16_t G5     = 784;
  //uint16_t Db5    = 554;
    uint16_t C6     = 1067;
    uint16_t D6     = 1175;
    uint16_t Dsharp6 = 1245;
    uint16_t E6     = 1319;
    uint16_t F6     = 1397;
    uint16_t G6     = 1568;
    uint16_t Gsharp6 = 1661;
    uint16_t B6     = 1975;
    uint16_t C7     = 2093;
    
    //The Lick! https://www.youtube.com/watch?v=krDxhnaKD7Q
    uint16_t theLickNotes[7] = {C6, D6, Dsharp6, F6, D6, Bb5, C6};
    uint16_t theLickLengths[7] = {1,1,1,1,2,1,1};
    uint16_t theLickNumberOfNotes = 7;
    
    uint16_t arpeggioNotes[5] = {C6, E6, G6, B6, C7};
    uint16_t arpeggioLengths[5] = {1,1,1,1,1};
    uint16_t arpeggioNumberOfNotes = 5;
    
    //Junalla kulkee! https://www.youtube.com/watch?v=uUN5vMIkIeM
    uint16_t junaKulkeeNotes[9] = {C6, 0, C6, 0, C6, 0, Gsharp6, G6, F6};
    uint16_t junaKulkeeLengths[9] = {1, 1, 1, 1, 1, 1, 4, 4, 4};
    uint16_t junaKulkeeNumberOfNotes = 9;
    
    
    /*uint16_t arpeggio2Notes[5] = {C7,  B6, G6, E6, C6};
    uint16_t arpeggio2Lengths[5] = {1,1,1,1,1};
    uint16_t arpeggio2NumberOfNotes = 5;*/
    
    int i = 0;
    if(ele == 1){
        buzzerOpen(buzzerHandle);
                for (i=0; i<theLickNumberOfNotes; i++){
                    buzzerSetFrequency(theLickNotes[i]);
                    Task_sleep(theLickLengths[i]* 0.15 * 1000000/Clock_tickPeriod);
                }
                buzzerClose();
    }
    if(ele == 2){
    buzzerOpen(buzzerHandle);
            for (i=0; i<arpeggioNumberOfNotes; i++){
                buzzerSetFrequency(arpeggioNotes[i]);
                Task_sleep(arpeggioLengths[i] * 0.15 * 1000000/Clock_tickPeriod);
            }
            buzzerClose();
    }// MUSIC state didn't workout. Problem calling function
    if(ele == 3){
        for (i=0; i<junaKulkeeNumberOfNotes; i++){
            if (junaKulkeeNotes[i] != 0){
                buzzerSetFrequency(junaKulkeeNotes[i]);
                Task_sleep(junaKulkeeLengths[i] * 0.1 * 1000000/Clock_tickPeriod);
            }else{
                buzzerClose();
                Task_sleep(junaKulkeeLengths[i] * 0.1 * 1000000/Clock_tickPeriod);
                buzzerOpen(buzzerHandle);
            }
            buzzerClose();
        }
    }
}


//______________________________________________________________________________
/* DISPLAY TASK */
Void displayTaskFxn(UArg arg0, UArg arg1) {

   // Initialise display task
    Display_Params params;
    Display_Params_init(&params);
    params.lineClearMode = DISPLAY_CLEAR_BOTH;

    // Create display handle
    hDisplayLcd = Display_open(Display_Type_LCD, &params);
    
    //Printing variables
    char strPres[] = "";
    char strTempC[] = "";
    char strTemp2[] = "";
    char strTempF[] = "";
    int counter = 0;
  
    
    while (1){
        if (check == 1){ 
            
            switch(myState){
                
                case MAIN://----------------------------------------------------
                    
                    //Opening screen
                    Display_clear(hDisplayLcd);
                    Display_print0(hDisplayLcd, 2, 1, "   __ ___ ____"); 
                    Display_print0(hDisplayLcd, 3, 1, "  / // (_) __/");   
                    Display_print0(hDisplayLcd, 4, 1, " / _  / /__ \ ");
                    Display_print0(hDisplayLcd, 5, 1, "/_//_/_/____/ ");
                    Display_print0(hDisplayLcd, 10, 2, "Push button");
                    
                    check = 0;
                    break;
                
                case gesture://-------------------------------------------------
                // 
                
                    Display_clear(hDisplayLcd);
                    Display_print0(hDisplayLcd, 2, 2, "Recognizing");
                    Display_print0(hDisplayLcd, 3, 2, "Gestures");
                    Display_print0(hDisplayLcd, 5, 1, hi5count);
                    Display_print0(hDisplayLcd, 7, 1, wavecount);
                    check = 0;
                    break;
                
                case INFO://----------------------------------------------------
                // Update to screen all info

                    Task_sleep(100000 / Clock_tickPeriod);
                    Display_clear(hDisplayLcd);
                    
                    Display_print0(hDisplayLcd, 1, 0, "Air pressure:");
                    sprintf(strPres, "%.2f hPa", pres);
                    Display_print0(hDisplayLcd, 2, 0, strPres);
                    
                    Display_print0(hDisplayLcd, 4, 0, "Temperature C: ");
                    
                    sprintf(strTempC,"%.2f BMP280", temp);
                    Display_print0(hDisplayLcd, 5, 0, strTempC);
                    sprintf(strTemp2,"%.2f TMP007", TMPtemp);
                    Display_print0(hDisplayLcd, 6, 0, strTemp2);
                    double farh = (TMPtemp * 1.8 + 32);
                    sprintf(strTempF,"%.2f F", farh);
                    Display_print0(hDisplayLcd, 7, 0, strTempF);
                    
                    Display_print0(hDisplayLcd, 9, 0, "Battery level:");
                    Display_print0(hDisplayLcd, 10, 0, batteryLevel);
                    check = 0;
                    break;
                
                case READ_Message://--------------------------------------------
                
                    Display_clear(hDisplayLcd);
                    if (viesti == 1){
                        Display_print0(hDisplayLcd, 2, 2, "New message!");
                        Display_print0(hDisplayLcd, 3, 0, payload);
                        Display_print0(hDisplayLcd, 5, 1, "From: ");
                        Display_print0(hDisplayLcd, 7, 2, lahettaja);
                        check = 0;
                    
                    }else{
                        /*
                         ________________     ______
                        |\              /|   |\    /|
                        | \            / |   | \__/ |
                        | /\__________/\ |   |______|
                        |/              \| 
                        |________________|
                        1234567890abcdef
                        
                        */
                        Display_print0(hDisplayLcd, 2, 2, "No Messages");
                        Display_print0(hDisplayLcd, 4, 3, " _______");
                        Display_print0(hDisplayLcd, 5, 3, "|\\     /|");
                        Display_print0(hDisplayLcd, 6, 3, "| \\___/ |");
                        Display_print0(hDisplayLcd, 7, 3, "|_______|");
                        
                        Display_print0(hDisplayLcd, 9, 2, "To Display!");
                        
                        check = 0;
                    }
                    break;
                    
                /*case music://-------------------------------------------------
                    
                    if(counter > 1){
                        Display_clear(hDisplayLcd);
                        Display_print0(hDisplayLcd, 4, 1, "Playing music!");
                        break; 
                    }
                    counter++;
                 */
            }
        }
        Task_sleep(100000 / Clock_tickPeriod);
    }
}

//______________________________________________________________________________
/* SENSOR TASK */
Void sensorTaskFxn(UArg arg0, UArg arg1) {

    // USE TWO DIFFERENT I2C INTERFACES
    printf("( ^o^)ノ");
    System_flush();
	// INTERFACE FOR OTHER SENSORS
	I2C_Handle i2c;
	I2C_Params i2cParams;
	
	// INTERFACE FOR MPU9250 SENSOR
	I2C_Handle i2cMPU;
	I2C_Params i2cMPUParams;
	
    float sum = 0;
    int hi5 = 0;
    int handwave = 0;
	float ax, ay, az, gx, gy, gz;
	

    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;

    I2C_Params_init(&i2cMPUParams);
    i2cMPUParams.bitRate = I2C_400kHz;
    i2cMPUParams.custom = (uintptr_t)&i2cMPUCfg;

    // MPU OPEN I2C
    i2cMPU = I2C_open(Board_I2C, &i2cMPUParams);
    if (i2cMPU == NULL) {
        System_abort("Error Initializing I222CMPU\n");
    }


    // MPU POWER ON
    PIN_setOutputValue(hMpuPin,Board_MPU_POWER, Board_MPU_POWER_ON);

    // WAIT 100MS FOR THE SENSOR TO POWER UP
	Task_sleep(100000 / Clock_tickPeriod);
    System_printf("MPU9250: Power ON\n");
    System_flush();

    // MPU9250 SETUP AND CALIBRATION
	System_printf("MPU9250: Setup and calibration...\n");
	System_flush();

	mpu9250_setup(&i2cMPU);

	System_printf("MPU9250: Setup and calibration OK\n");
	System_flush();


    // MPU CLOSE I2C
    I2C_close(i2cMPU);

    // BMP280 OPEN I2C
    i2c = I2C_open(Board_I2C, &i2cParams);
    if (i2c == NULL) {
        System_abort("Error Initializing I2C\n");
    }

    // BMP280 SETUP
    bmp280_setup(&i2c);

    // BMP280 CLOSE I2C
    I2C_close(i2c);
    
    // open i2c for TMP007
    i2c = I2C_open(Board_I2C0, &i2cParams);
    
    if (i2c == NULL) {
        System_abort("Error Initializing I2C\n");
    }
    
    //TMP gets temperature
    TMPtemp = tmp007_get_data(&i2c);
    
    // TMP0007 CLOSE I2C
    I2C_close(i2c);
    
    // LOOP FOREVER
	while (1) {

        switch(myState){
            
            case MAIN://--------------------------------------------------------
                break;
        
            case gesture://-----------------------------------------------------
                //Battery status function
                battery();

        	    // MPU OPEN I2C
        	    i2cMPU = I2C_open(Board_I2C, &i2cMPUParams);
        	    if (i2cMPU == NULL) {
        	        System_abort("Error Initializing I2CMPU\n");
        	    }
        
        	    // MPU ASK DATA
                // Accelerometer values: ax,ay,az
        	 	// Gyroscope values: gx,gy,gz

        		mpu9250_get_data(&i2cMPU, &ax, &ay, &az, &gx, &gy, &gz);
                
                sum = sqrt((ax * ax)+(ay * ay)+(az * az));
                if(sum > 3 && ((ay <(sum/2)) && (ay >-(sum/2)))
                ){
                    hi5++;
                    Send6LoWPAN(IEEE80154_SERVER_ADDR, "hi5( ^o^)ノ＼(^_^ ", 16);
                    
                    
                    StartReceive6LoWPAN();
                    sprintf(hi5count,"Highfives: %d",hi5);
                    check = 1;
                    soita_musiikkia(1);
                   
                            
                }
                if(sum > 1.7 && (-0.6 < az && az < 0.6)){
                    handwave++;
                    Send6LoWPAN(IEEE80154_SERVER_ADDR, "(^-^)ノ HELLO", 16);
                    StartReceive6LoWPAN();
                    sprintf(wavecount,"Handwaves: %d",handwave);
                    check = 1;
                    soita_musiikkia(2);
                    
                }
                
        	    // MPU CLOSE I2C
        	    I2C_close(i2cMPU);
        	    break;
    	    
    	    case INFO://--------------------------------------------------------

        	    // BMP280 OPEN I2C
        	    i2c = I2C_open(Board_I2C, &i2cParams);
        	    
        	    if (i2c == NULL) {
        	        System_abort("Error Initializing I2C\n");
        	    }
        
        	    // BMP280 ASK DATA
        	    bmp280_get_data(&i2c, &pres, &temp);
                
        	    // BMP280 CLOSE I2C
        	    I2C_close(i2c);
        	    
        	    break;
                }
        	    // WAIT 10MS
            	Task_sleep(100000 / Clock_tickPeriod);
    	}
	// MPU9250 POWER OFF 
    PIN_setOutputValue(hMpuPin,Board_MPU_POWER, Board_MPU_POWER_OFF);
    
}
//______________________________________________________________________________
/* COMMUNICATION TASK */
Void commTaskFxn(UArg arg0, UArg arg1) {

    // Radio to receive mode
	int32_t result = StartReceive6LoWPAN();
	uint8_t maxLen = 0x0f;
	
	if(result != true) {
		System_abort("Wireless receive mode failed");
	}

    while (1) {

        // If true, we have a message
    	if (GetRXFlag() == true) {
    	    viesti = 1;
    	    memset(payload,0,16);
            Receive6LoWPAN(&senderAddr, &payload, maxLen);
            sprintf(lahettaja,"%x",senderAddr);
    	}

    	// Absolutely NO Task_sleep in this task!!
    }
}

//______________________________________________________________________________
void buttonFxn(PIN_Handle handle, PIN_Id pinId) {
    
    PIN_setOutputValue( ledHandle, Board_LED0, !PIN_getOutputValue( Board_LED0 ));
    
    check = 1;
    switch(myState){
        
        case MAIN://------------------------------------------------------------
            myState = gesture;
            break;
        
        case gesture://---------------------------------------------------------
            myState = INFO;
            break;
        
        case INFO://------------------------------------------------------------
            myState = READ_Message;
            break;
        
        case READ_Message://----------------------------------------------------
            if(viesti==1){
                viesti = 0;
            }
            myState = MAIN;
            break;
            
        /*case music:
            //soita_musiikkia(3);
            myState = MAIN;
            break;
        */
        }
}

//______________________________________________________________________________
Void buttonShutFxn(PIN_Handle handle, PIN_Id pinId) {

    //CopyPaste from LOVELACE
    Display_clear(hDisplayLcd);
    Display_close(hDisplayLcd);
   
    PIN_setOutputValue(hMpuPin, Board_MPU_POWER, Board_MPU_POWER_OFF);
    Task_sleep(100000 / Clock_tickPeriod);
    
    PIN_close(hButtonShut);
    PINCC26XX_setWakeup(buttonWake);
    Power_shutdown(NULL,0);
   
}

//______________________________________________________________________________
Void battery() {
    
    uint32_t batreg = HWREG(AON_BATMON_BASE + AON_BATMON_O_BAT);
    batteryInt = (batreg >> 8);
    batteryFrac = (batreg & 0xff) - 80;
    sprintf(batteryLevel, "%d.%d V", batteryInt, batteryFrac);
    
}
//______________________________________________________________________________
Int main(void) {

    // Task variables
	Task_Handle commTask;
	Task_Params commTaskParams;
	Task_Handle sensorTask;
	Task_Params sensorTaskParams;
	Task_Handle displayTask;
	Task_Params displayTaskParams;
	
    // Initialize board
    Board_initGeneral();
    
//----------------------------------------------------------------BUTTON HANDLES
    // Buttons and led hande
    buttonHandle = PIN_open(&buttonState, buttonConfig);
    if(!buttonHandle) {
        System_abort("Error initializing button pins\n");
    }
    ledHandle = PIN_open(&ledState, ledConfig);
    if(!ledHandle) {
        System_abort("Error initializing LED pins\n");
    };
   
    hButtonShut = PIN_open(&bStateShut, buttonShut);
    if( !hButtonShut ) {
        System_abort("Error initializing button shut pins\n");
    }
    if (PIN_registerIntCb(hButtonShut, &buttonShutFxn) != 0) {
        System_abort("Error registering button callback function");
    }

	// Register the interrupt handler for the button
     if (PIN_registerIntCb(buttonHandle, &buttonFxn) != 0) {
      System_abort("Error registering button callback function");
    }
    
    
    //-----------------------------------------------------------TASK PARAMETERS 
    /* Sensor Task */
    Task_Params_init(&sensorTaskParams);
    sensorTaskParams.stackSize = STACKSIZE;
    sensorTaskParams.stack = &sensorTaskStack;
    sensorTaskParams.priority=2;
    sensorTask = Task_create(sensorTaskFxn, &sensorTaskParams, NULL);
    if (sensorTask == NULL) {
    	System_abort("sensorTask create failed!");
    }
    
    /* Display Task*/
    Task_Params_init(&displayTaskParams);
    displayTaskParams.stackSize = STACKSIZE;
    displayTaskParams.stack = &displayTaskStack;
    displayTaskParams.priority=2;
    displayTask = Task_create(displayTaskFxn, &displayTaskParams, NULL);
    if (displayTask == NULL) {
    	System_abort("displayTask create failed!");
    }

    /* Communication Task */
    Init6LoWPAN(); // This function call before use!

    Task_Params_init(&commTaskParams);
    commTaskParams.stackSize = STACKSIZE;
    commTaskParams.stack = &commTaskStack;
    commTaskParams.priority=1;

    commTask = Task_create(commTaskFxn, &commTaskParams, NULL);
    if (commTask == NULL) {
    	System_abort("Task create failed!");
    }
    System_printf("It's on!\n");
    System_flush();
    
    /* Start BIOS */
    BIOS_start();

    return (0);
}
