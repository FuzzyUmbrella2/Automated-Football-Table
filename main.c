#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "PWM.h"
#include "sleep.h"
#include "interrupt.h"
#include "xgpio.h"
#include "xparameters.h"
#include "xstatus.h"

XGpio BtnGpio;

int main()
{
    init_platform();

    initPWM();

/*    for(int i = 0; currentSpeed < endSpeed; i++)
    {
    	int newSpeed = startSpeed+((i*i)/6);
    	currentSpeed = newSpeed;
    	int period = rpmToPeriod(newSpeed);
    	configPWM(period);
    	usleep(1500);
    }*/

    int status = XGpio_Initialize(&BtnGpio, XPAR_AXI_GPIO_0_DEVICE_ID);
    if (status != XST_SUCCESS)
    {
    	printf("gpio init failed\n");
    }
    XGpio_SetDataDirection(&BtnGpio, 1, 0x000);

    //float period = (float)rpmToPeriod(startSpeed);
    //configPWM(period);
    initInterrupt();

    while(1)
    {
    	static int pressed = 0;;
    	int btn = XGpio_DiscreteRead(&BtnGpio, 1);
    	if (btn == 1 && pressed == 0)
    	{
    		startTimer();
    		//enable();
    		pressed = 1;
    	}
    	else if(btn == 0 && pressed == 1)
    	{
    		pressed = 0;
    	}
    }


    cleanup_platform();
    return 0;
}
