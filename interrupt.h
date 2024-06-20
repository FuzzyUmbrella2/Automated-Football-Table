#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "PWM.h"


#define GIC_ID			XPAR_PS7_SCUGIC_0_DEVICE_ID
#define TIMER0_ID		XPAR_TMRCTR_0_DEVICE_ID
#define TIMER0_GI_ID	XPAR_FABRIC_TMRCTR_0_VEC_ID


#define MAX_STEPS		100
#define STEP_SIZE		20
#define MAX_CYCLE		2
/*
#define START_SPEED		400
#define END_SPEED		1000
#define INCREASE_SPEED	((END_SPEED)-(START_SPEED))/((MAX_STEPS)/(STEP_SIZE))
*/


#define MAX_COUNT  0xFFFFFFFF
#define PB_FRQ 100000000 //100MHz
#define TMR_LOAD(per)  (u32)((int)MAX_COUNT + 2 - (int)(per * (float)PB_FRQ)) //per is period in sec


void initInterrupt();
void startTimer();
