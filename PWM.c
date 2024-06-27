#include "PWM.h"
#include "xstatus.h"
#include "xtmrctr.h"

#define ROTATION_PWM_ID XPAR_AXI_TIMER_PWM_DEVICE_ID
#define SPINDLE_PWM_ID	XPAR_AXI_TIMER_SPINDLE_DEVICE_ID
#define duty 10	// in percentages
XTmrCtr rotationPWM, spindlePWM;

int initPWM()
{
	int Status =  XTmrCtr_Initialize(&rotationPWM, ROTATION_PWM_ID);
	if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly. Timer0 is used for self test
	*/
	Status = XTmrCtr_SelfTest(&rotationPWM, 0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status =  XTmrCtr_Initialize(&spindlePWM, SPINDLE_PWM_ID);
	if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly. Timer0 is used for self test
	*/
	Status = XTmrCtr_SelfTest(&spindlePWM, 0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	printf("init PWM complete\n");
	return XST_SUCCESS;
}

void configRotation(int Period)
{
	int HighTime = Period/duty;
	XTmrCtr_PwmDisable(&rotationPWM);
	XTmrCtr_PwmConfigure(&rotationPWM, Period, HighTime);
	XTmrCtr_PwmEnable(&rotationPWM);
}

void configSpindle(int Period)
{
	printf("config spindle\n");
	int HighTime = Period/duty;
	XTmrCtr_PwmDisable(&spindlePWM);
	XTmrCtr_PwmConfigure(&spindlePWM, Period, HighTime);
	XTmrCtr_PwmEnable(&spindlePWM);
}

int rpmToPeriod(int rpm)
{
	int stepsPerMin = (rpm*360)/1.8;
	float period = 1/(((float)stepsPerMin/60.0)/1000000000.0);
	return period;
}

void disableRotation()
{
	XTmrCtr_PwmDisable(&rotationPWM);
}

void disableSpindle()
{
	printf("disabled spindle\n");
	XTmrCtr_PwmDisable(&spindlePWM);
}
