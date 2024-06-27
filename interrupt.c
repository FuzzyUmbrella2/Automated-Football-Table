#include "interrupt.h"
#include "xstatus.h"

#include "xscugic.h"
#include "xtmrctr.h"
#include "xgpio.h"
#include "xil_exception.h"

XScuGic GicInst;
XTmrCtr rotationTimer, spindleTimer;

typedef struct motor
{
	u8 turning;
	int currentSpeed;
	int currentSteps;
} motor;

motor* rotation;
motor* spindle;


int LUT[SPINDLE_CYCLE] = {
		550, 600, 650, 700, 750, 800, 850, 900, 950, 1000, 2200, 2300, 2400, 2500
};

int speedRot[ROT_CYCLE] = {
		380, 450
};

void rotationInterrupt()
{
	disableRotation();
	XTmrCtr_Stop(&rotationTimer, 0);
	if (rotation->currentSteps < (ROT_CYCLE)*STEP_SIZE)
	{
		int Speed = speedRot[rotation->currentSteps/STEP_SIZE];
		rotation->currentSpeed = Speed;
		float period = (float)rpmToPeriod(Speed);
		configRotation(period);
		XTmrCtr_SetResetValue(&rotationTimer, 0, TMR_LOAD((period*STEP_SIZE)/1000000000.0));
		XTmrCtr_Start(&rotationTimer, 0);
		rotation->currentSteps += STEP_SIZE;
	}
	else if (rotation->currentSteps<(MAX_STEPS-DECEL))
	{
    	float period = (float)rpmToPeriod(rotation->currentSpeed);
    	configRotation(period);
    	XTmrCtr_SetResetValue(&rotationTimer, 0, TMR_LOAD((period*(MAX_STEPS-DECEL-rotation->currentSteps))/1000000000.0));
    	XTmrCtr_Start(&rotationTimer, 0);
    	rotation->currentSteps = MAX_STEPS-DECEL;
	}
	else if (rotation->currentSteps<(MAX_STEPS))
	{
		float period = (float)rpmToPeriod(speedRot[0]);
		configRotation(period);
		XTmrCtr_SetResetValue(&rotationTimer, 0, TMR_LOAD((period*DECEL)/1000000000.0));
		XTmrCtr_Start(&rotationTimer, 0);
    	rotation->currentSpeed = speedRot[0];
    	rotation->currentSteps += DECEL;
	}
	else
	{
		disableRotation();
		XTmrCtr_Stop(&rotationTimer, 0);
		rotation->currentSpeed = 0;
		rotation->currentSteps = 0;
		rotation->turning = 0;
		printf("done\n");
	}

}


void spindleInterrupt()
{
	printf("interrupt\n");
	static int accel = 1;

	if (spindle->currentSteps < (SPINDLE_CYCLE)*STEP_SIZE)
	{
		int Speed = LUT[spindle->currentSteps/STEP_SIZE];
		spindle->currentSpeed = Speed;
		float period = (float)rpmToPeriod(Speed);
		configSpindle(period);
		XTmrCtr_SetResetValue(&spindleTimer, 0, TMR_LOAD((period*STEP_SIZE)/1000000000.0));
		XTmrCtr_Start(&spindleTimer, 0);
		spindle->currentSteps += STEP_SIZE;
		accel = 1;
	}
	else
	{
		accel = 0;
	}

	if (accel == 0)
	{
		XTmrCtr_Stop(&spindleTimer, 0);
	}
}

void initInterrupt()
{
	rotation = malloc(sizeof(motor));
	rotation->currentSpeed = 0;
	rotation->currentSteps = 0;
	rotation->turning = 0;

	spindle = malloc(sizeof(motor));
	spindle->currentSpeed = 0;
	spindle->currentSteps = 0;
	spindle->turning = 0;


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
    Status =  XTmrCtr_Initialize(&rotationTimer, TIMER0_ID);
    if (Status != XST_SUCCESS) {
    	printf("Could not initialize timer \n\r");
    }

    Status =  XTmrCtr_Initialize(&spindleTimer, TIMER1_ID);
    if (Status != XST_SUCCESS) {
    	printf("Could not initialize timer \n\r");
    }

    // link the timer and the interrupt it triggers
    XTmrCtr_SetHandler(&rotationTimer, (XTmrCtr_Handler)rotationInterrupt, (void*) 0x12345678);
    XTmrCtr_SetHandler(&spindleTimer, (XTmrCtr_Handler)spindleInterrupt, (void*) 0x12345678);

    /*------------set prioroty to place 160 (aka 0xA0) and the trigger to risign edge (aka 0x3)----------*/
    XScuGic_SetPriorityTriggerType(&GicInst, TIMER0_GI_ID, 0x98, 0x3);
    XScuGic_SetPriorityTriggerType(&GicInst, TIMER1_GI_ID, 0xA0, 0x3);

    /*-------------------------connect the interrupt controller to the timer-----------------------------*/
    Status = XScuGic_Connect(&GicInst, TIMER0_GI_ID,(Xil_InterruptHandler)XTmrCtr_InterruptHandler,&rotationTimer);
    if (Status != XST_SUCCESS) {
    	printf("\n\r XScuGic_Connect() failed");
    }

    Status = XScuGic_Connect(&GicInst, TIMER1_GI_ID,(Xil_InterruptHandler)XTmrCtr_InterruptHandler,&spindleTimer);
    if (Status != XST_SUCCESS) {
    	printf("\n\r XScuGic_Connect 2() failed");
    }

    /*-----------------------------enable the interrupt controller-------------------------------------*/
    XScuGic_Enable(&GicInst, TIMER0_GI_ID);
    XScuGic_Enable(&GicInst, TIMER1_GI_ID);

    // Initialize the exception table.
    Xil_ExceptionInit();

    // Register the interrupt controller handler with the exception table.
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,(void *)&GicInst);

    // Enable exceptions.
    Xil_ExceptionEnable();

    XTmrCtr_SetOptions(&rotationTimer, 0,   XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
    XTmrCtr_SetOptions(&spindleTimer, 0,   XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
}

void turnBar()
{
	printf("will it turn?\n");
	if (rotation->turning == 0)
	{
		printf("yes it will\n");
		XTmrCtr_SetResetValue(&rotationTimer, 0, TMR_LOAD(1/1000.0));
		XTmrCtr_Start(&rotationTimer, 0);
		rotation->turning = 1;
	}
}

void moveSpindle()
{
	printf("Will it move?\n");
	if (spindle->turning == 0)
	{
		printf("yes it will\n");
		XTmrCtr_SetResetValue(&spindleTimer, 0, TMR_LOAD(1/1000.0));
		XTmrCtr_Start(&spindleTimer, 0);
		spindle->turning = 1;
	}
}

void stopSpindle()
{
	printf("Will it stop?\n");
	if (spindle->turning == 1)
	{
		printf("yes it will\n");
		XTmrCtr_Stop(&spindleTimer, 0);
		disableSpindle();
		spindle->currentSpeed = 0;
		spindle->currentSteps = 0;
		spindle->turning = 0;
	}
}
