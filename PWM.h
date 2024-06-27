#ifndef __PWM_H_
#define __PMW_H_

#include <stdio.h>
#include "platform.h"
#include "xparameters.h"

int initPWM();

void configRotation(int Period);
void configSpindle(int Period);

int rpmToPeriod(int rpm);

void disableRotation();
void disableSpindle();


#endif
