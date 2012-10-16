//*****************************************************************************
//
// Stepper drive code based on TI's timer.c
//
//*****************************************************************************

#include "src/TM1638.h"
#include "src/constants.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include <string.h>
#include <stdlib.h>



// Set variable for stepper speed
unsigned int stepSpeed;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

/* reverse:  reverse string s in place */
void strreverse(char s[])
{
    int c, i, j;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/* itoa:  convert n to characters in s */
char *
itoa (char *nptr, int n)
{
  char *cptr = nptr;

  if (n < 0)
    *cptr++ = '-';

  while (n)
    {
      *cptr++ = '0' + abs (n) % 10;
      n /= 10;
    }

  *cptr = '\0';

  if (n < 0)
    cptr = nptr + 1;
  else
    cptr = nptr;

  strreverse (cptr);

  return nptr;
}

//*****************************************************************************
//
// Set up the timer and update/read the display/buttons
//
//*****************************************************************************
int
main(void)
{
	stepSpeed = 1500;
    //
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    ROM_FPULazyStackingEnable();

    //
    // Set the clocking to run directly from the crystal.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);


    //
    // Enable the GPIO port that is used for the on-board LED.
    //
    ROM_SysCtlPeripheralEnable(LED_PERIPH);
    ROM_SysCtlPeripheralEnable(STP1_PERIPH);
    ROM_SysCtlPeripheralEnable(TM1_PERIPH);



    TM1638 segmentDisplay(TM1_PERIPH, TM1_BASE, TM1_CLK, TM1_DIO, TM1_STB, true, 7);
    //
    // Enable the GPIO pins for the Stepper and LEDs (PF1 & PF2).
    //
    ROM_GPIOPinTypeGPIOOutput(LED_BASE, RED_LED|BLUE_LED|GREEN_LED);
    ROM_GPIOPinTypeGPIOOutput(STP1_BASE, STP1_PLS | STP1_DIR | STP1_ENA);


    GPIOPinWrite(LED_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);
    GPIOPinWrite(STP1_BASE, STP1_PLS | STP1_DIR, 0x00);
    GPIOPinWrite(STP1_BASE, STP1_ENA, STP1_ENA);
    //
    // Enable the peripherals used by this example.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    //
    // Enable processor interrupts.
    //
    ROM_IntMasterEnable();

    //
    // Configure the two 32-bit periodic timers.
    //
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet() / stepSpeed);

    //
    // Setup the interrupts for the timer timeouts.
    //
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Enable the timers.
    //
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);

    //
    // Loop forever while the timers run.
    //
    unsigned char buttons;
	while (true) {
		buttons = segmentDisplay.getButtons();
		char buf[5];
		itoa(buf,stepSpeed);
		if(buttons == 1){

			//Drive Stepper Forward
			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);

		    GPIOPinWrite(STP1_BASE, STP1_DIR, STP1_DIR);

			segmentDisplay.clearDisplay();
			segmentDisplay.setDisplay(std::string("FWD"), 0x00, 0);
			SysCtlDelay(700000);
			segmentDisplay.clearDisplay();

		} else if(buttons == 2){

			//Drive Stepper Reverse
			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, RED_LED);

		    GPIOPinWrite(STP1_BASE, STP1_DIR, 0x00);

			segmentDisplay.clearDisplay();
			segmentDisplay.setDisplay(std::string("REV"), 0x00, 0);
			SysCtlDelay(700000);
			segmentDisplay.clearDisplay();

		} else if(buttons == 4){

			//Increase speed by 100
			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);

			stepSpeed += 50;
		    ROM_TimerDisable(TIMER0_BASE, TIMER_A);
		    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet() / stepSpeed);
		    ROM_TimerEnable(TIMER0_BASE, TIMER_A);

			segmentDisplay.clearDisplay();
			segmentDisplay.setDisplay(buf, 0, 0);
			segmentDisplay.setDisplay(std::string("+50"), 0, 5);
		    SysCtlDelay(SysCtlClockGet()/20);
			segmentDisplay.clearDisplay();

		} else if(buttons == 8){

			//Decrease speed by 100
			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, RED_LED);

			stepSpeed -= 50;
		    ROM_TimerDisable(TIMER0_BASE, TIMER_A);
		    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet() / stepSpeed);
		    ROM_TimerEnable(TIMER0_BASE, TIMER_A);

			segmentDisplay.clearDisplay();
			segmentDisplay.setDisplay(buf, 0, 0);
			segmentDisplay.setDisplay(std::string("-50"), 0, 5);
		    SysCtlDelay(SysCtlClockGet()/20);
			segmentDisplay.clearDisplay();

		} else {
			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
			segmentDisplay.setDisplay(buf, 0, 0);
		}
	}
}
