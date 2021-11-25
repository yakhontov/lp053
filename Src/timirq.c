#include "timirq.h"
#include <stddef.h>
#include <stdio.h>

volatile int dutyCommand = -1; // Value retiverd from usart
volatile int printDutyFlag = 0;

// set duty cycle as percent
void setDutyCycle(int duty)
{
  duty = duty < 0 ? 0 : (duty > 100 ? 100 : duty); // constrain input value to 0-100%
  dutyCommand = duty * htim21.Init.Period / 100; // calculate duty cycle as timer counter
}

void TIM21IRQ()
{
  static int periodicValue = 0; // Last periodic value
  static int ticksToPeriodic = 0; // Timer ticks count before enable periodic values
  static int printDutyCounter = 0;
  
  if(printDutyCounter++ >= 1000)
  {
    printDutyCounter = 0;
    printDutyFlag = 1;
  }
  
  if(dutyCommand > -1) // if we got new command
  {
    ticksToPeriodic = 5000; // set time to execute 
    TIM21->CCR1 = dutyCommand; // set duty cycle
    dutyCommand = -1; // reset command
    return;
  }
  
  if(ticksToPeriodic) // if command time expired, running periodic 
  { // Saw signal
    periodicValue += 3; 
    if(periodicValue > htim21.Init.Period)
      periodicValue = 0;
    TIM21->CCR1 = periodicValue;
  }
  else
    ticksToPeriodic--;
}

void readPrint()
{
  if(printDutyFlag)
  {
    printDutyFlag = 0;
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    float d = (float)TIM21->CCR1 * 100.0 / (float)htim21.Init.Period;
    //printf("123213213216546546546546543\n");
    
  }
}