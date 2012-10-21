#include "inc/hw_ints.h"
#include "../stepper.h"
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

//*****************************************************************************
//
// The interrupt handler for the timer interrupt.
//
//*****************************************************************************
void
Timer0IntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Toggle the flag for the timer.
    //
    HWREGBITW(&g_ulFlags, 0) = 1;
    GPIOPinWrite(STP1_BASE, STP1_PLS, STP1_PLS);
    SysCtlDelay(SysCtlClockGet() / 500000);
    GPIOPinWrite(STP1_BASE, STP1_PLS, 0x00);
}

