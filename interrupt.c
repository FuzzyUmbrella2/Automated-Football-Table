#include "interrupt.h"
#include "xstatus.h"

#include "xscugic.h"
#include "xtmrctr.h"
#include "xgpio.h"
#include "xil_exception.h"

XScuGic GicInst;
XTmrCtr timerInst;

int LUT[3] = {
		350, 600, 700
};

void interruptHandlerTimer()
{
	//disable();
	XTmrCtr_Stop(&timerInst, 0);
	static int currentSteps = 0;
	static int currentSpeed = 0;
	//static int currentCycle = 0;
	static float period = 0;
	printf("current steps = %d\n", currentSteps);
	if (currentSteps < MAX_CYCLE*STEP_SIZE)
	{
		currentSpeed = LUT[currentSteps/STEP_SIZE];
		period = (float)rpmToPeriod(currentSpeed);
		configPWM(period);
		printf("period1 = %f\n", period);
		XTmrCtr_SetResetValue(&timerInst, 0, TMR_LOAD((period*STEP_SIZE)/1000000000.0));
		XTmrCtr_Start(&timerInst, 0);
		currentSteps = currentSteps + STEP_SIZE;
	}
	else if (currentSteps<MAX_STEPS)
	{
    	period = (float)rpmToPeriod(currentSpeed);//+INCREASE_SPEED);
    	printf("steps to go = %d\n", (MAX_STEPS-currentSteps));
    	printf("period = %f\n", period);
    	configPWM(period);
    	XTmrCtr_SetResetValue(&timerInst, 0, TMR_LOAD((period*(MAX_STEPS-currentSteps))/1000000000.0));
    	XTmrCtr_Start(&timerInst, 0);
    	currentSteps = currentSteps+(MAX_STEPS-currentSteps);
	}
	else
	{
		printf("done\n");
    	currentSpeed = 0;
    	currentSteps = 0;
    	//currentCycle = 0;
    	disable();
	}

}

void startTimer()
{
	XTmrCtr_Start(&timerInst, 0);
}

void initInterrupt()
{
    /*--------------------------Get the config of the interrupt controller-----------------------------*/
    XScuGic_Config *IntcConfig = XScuGic_LookupConfig(GIC_ID);
    if (NULL == IntcConfig) {
    	printf("\n\r XScuGic_LookupConfig() failed\n");
    	return;
    }

    /*-----------------------------initialize the initerrupt controller--------------------------------*/
    int Status = XScuGic_CfgInitialize(&GicInst, IntcConfig,IntcConfig->CpuBaseAddress);
    if (Status != XST_SUCCESS) {
    	printf("\n\r XScuGic_CfgInitialize() failed\n");
    	return;
    }

	/*------------------------------------initialize the timer-----------------------------------------*/
    Status =  XTmrCtr_Initialize(&timerInst, TIMER0_ID);
    if (Status != XST_SUCCESS) {
    	printf("Could not initialize timer \n\r");
    }

    // link the timer and the interrupt it triggers
    XTmrCtr_SetHandler(&timerInst, (XTmrCtr_Handler)interruptHandlerTimer, (void*) 0x12345678);

    /*------------set prioroty to place 160 (aka 0xA0) and the trigger to risign edge (aka 0x3)----------*/
    XScuGic_SetPriorityTriggerType(&GicInst, TIMER0_GI_ID, 0x98, 0x3);

    /*-------------------------connect the interrupt controller to the timer-----------------------------*/
    Status = XScuGic_Connect(&GicInst, TIMER0_GI_ID,(Xil_InterruptHandler)XTmrCtr_InterruptHandler,&timerInst);
    if (Status != XST_SUCCESS) {
    	printf("\n\r XScuGic_Connect() failed");
    }

    /*-----------------------------enable the interrupt controller-------------------------------------*/
    XScuGic_Enable(&GicInst, TIMER0_GI_ID);

    // Initialize the exception table.
    Xil_ExceptionInit();

    // Register the interrupt controller handler with the exception table.
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,(void *)&GicInst);

    // Enable exceptions.
    Xil_ExceptionEnable();


    float period = (float)rpmToPeriod(LUT[0]);

    //set the timer to 48 kHz
    XTmrCtr_SetOptions(&timerInst, 0,   XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
    XTmrCtr_SetResetValue(&timerInst, 0, TMR_LOAD((period*STEP_SIZE)/1000000000.0));
    XTmrCtr_Start(&timerInst, 0);
}
