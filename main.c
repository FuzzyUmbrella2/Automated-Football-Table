#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "PWM.h"
#include "sleep.h"
#include "interrupt.h"
#include "xgpio.h"
#include "xparameters.h"
#include "xstatus.h"

XGpio BtnGpio, DirGPIO;

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
    XGpio_SetDataDirection(&BtnGpio, 2, 0x000);

    status = XGpio_Initialize(&DirGPIO, XPAR_GPIO_DIR_DEVICE_ID);
    if (status != XST_SUCCESS)
    {
    	printf("gpio init failed\n");
    }
    XGpio_SetDataDirection(&DirGPIO, 1, 0xFFF);

    initInterrupt();

    while(1)
    {
    	static int pressed = 0, pressed2 = 0;
    	static int forward = 0;
    	int btn = XGpio_DiscreteRead(&BtnGpio, 1);
    	if (pressed == 0)
		{
    		if (btn == 1)//turn right
    		{
    			printf("turn right\n");
    			XGpio_DiscreteWrite(&DirGPIO, 1, (0b10+forward));
    			turnBar();
    			pressed = 1;
    		}
    		else if (btn == 2)
    		{
    			printf("turn left\n");
    			XGpio_DiscreteWrite(&DirGPIO, 1, (0b00+forward));
    			turnBar();
    			pressed = 1;
    		}
		}
    	else if(btn == 0 && pressed == 1)
    	{
    		pressed = 0;
    	}

    	btn = XGpio_DiscreteRead(&BtnGpio, 2);

    	if (pressed2 == 0)
		{
    		if (btn == 1)
    		{
    			printf("move forwards\n");
    			forward = 1;
    			XGpio_DiscreteWrite(&DirGPIO, 1, forward);
    			moveSpindle();
    			pressed2 = 1;
    		}
    		else if(btn == 2)
    		{
    			printf("move backwards\n");
    			forward = 0;
    			XGpio_DiscreteWrite(&DirGPIO, 1, forward);
    			moveSpindle();
    			pressed2 = 1;
    		}
		}
    	else if(btn == 0 && pressed2 == 1)
    	{
    		pressed2 = 0;
    		stopSpindle();
    	}
    }


    cleanup_platform();
    return 0;
}
