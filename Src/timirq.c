#include "timirq.h"
#include "usart.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


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
  
  if(!ticksToPeriodic) // if command time expired, running periodic 
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
  static char s[10];
  if(printDutyFlag) // waiting for print flag
  {
    printDutyFlag = 0; // reset print flag
    //HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    float d = (float)TIM21->CCR1 * 100.0 / (float)htim21.Init.Period; // calculating duty percent
    sprintf(s, "%.1f\r\n\0", d); 
    HAL_UART_Transmit(&huart1, s, strlen(s), 0xFFFF); // sending value to usart
  }
  
  static char inbuf[10];
  static int cnt = 0;
  while(HAL_UART_Receive(&huart1, inbuf + cnt, 1, 0) == HAL_OK) // receive value from usart
  {
    if(inbuf[cnt] == 13 || inbuf[cnt] == 10 || inbuf[cnt] == 0) // if we got end of line
    {
      inbuf[cnt] = 0;
      cnt = 0;
      int i;
      if(sscanf(inbuf, "%d", &i)) // trying to convrt string to int
        setDutyCycle(i);
        //HAL_UART_Transmit(&huart1, inbuf, strlen(inbuf), 0xFFFF);
    }
    else
      if(cnt < 9) // if we still have free space, increase counter
        cnt++;
  }
}