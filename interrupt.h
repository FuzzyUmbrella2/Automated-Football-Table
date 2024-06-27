#include "xil_io.h"
#include "stdlib.h"
#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "PWM.h"


#define GIC_ID			XPAR_PS7_SCUGIC_0_DEVICE_ID
#define TIMER0_ID		XPAR_TMRCTR_0_DEVICE_ID
#define TIMER0_GI_ID	XPAR_FABRIC_TMRCTR_0_VEC_ID
#define TIMER1_ID		XPAR_TMRCTR_2_DEVICE_ID
#define TIMER1_GI_ID	XPAR_FABRIC_TMRCTR_2_VEC_ID


#define STEP_SIZE		20
#define HOLD_SPEED		30
#define DECEL			10
#define ROT_CYCLE		2
#define MAX_STEPS		100//((MAX_CYCLE)*(STEP_SIZE))+(HOLD_SPEED)+(DECEL)


#define SPINDLE_CYCLE	10

/*
#define START_SPEED		400
#define END_SPEED		1000
#define INCREASE_SPEED	((END_SPEED)-(START_SPEED))/((MAX_STEPS)/(STEP_SIZE))
*/


#define MAX_COUNT  0xFFFFFFFF
#define PB_FRQ 100000000 //100MHz
#define TMR_LOAD(per)  (u32)((int)MAX_COUNT + 2 - (int)(per * (float)PB_FRQ)) //per is period in sec


void initInterrupt();
void turnBar();
void moveSpindle();
void stopSpindle();
