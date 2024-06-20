#include "PWM.h"
#include "xstatus.h"
#include "xtmrctr.h"

#define DeviceId XPAR_AXI_TIMER_PWM_DEVICE_ID
#define duty 10	// in percentages
XTmrCtr PWMTimer;

int initPWM()
{
	int Status =  XTmrCtr_Initialize(&PWMTimer, DeviceId);
	if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly. Timer0 is used for self test
	*/
	Status = XTmrCtr_SelfTest(&PWMTimer, 0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	printf("init PWM complete\n");
	return XST_SUCCESS;
}

void configPWM(int Period)
{
	int HighTime = Period/duty;
	//printf("config PWM with HighTime = %d\n", HighTime);
	XTmrCtr_PwmDisable(&PWMTimer);
	XTmrCtr_PwmConfigure(&PWMTimer, Period, HighTime);
	XTmrCtr_PwmEnable(&PWMTimer);
}

int rpmToPeriod(int rpm)
{
	int stepsPerMin = (rpm*360)/1.8;
	float period = 1/(((float)stepsPerMin/60.0)/1000000000.0);
	//printf("period = %f\n", period);
	return period;
}

void disable()
{
	XTmrCtr_PwmDisable(&PWMTimer);
}

void enable()
{
	XTmrCtr_PwmEnable(&PWMTimer);
}
