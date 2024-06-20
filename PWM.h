#ifndef __PWM_H_
#define __PMW_H_

#include <stdio.h>
#include "platform.h"
#include "xparameters.h"

int initPWM();

void configPWM(int Period);

int rpmToPeriod(int rpm);

void disable();
void enable();


#endif
