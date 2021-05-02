
/**
  TMR2 Generated Driver API Source File 

  @Company
    Microchip Technology Inc.

  @File Name
    tmr2.c

  @Summary
    This is the generated source file for the TMR2 driver using PIC32MX MCUs

  @Description
    This source file provides APIs for driver for TMR2. 
    Generation Information : 
        Product Revision  :  PIC32MX MCUs - pic32mx : v1.35
        Device            :  PIC32MX130F064B
        Driver Version    :  0.5
    The generated drivers are tested against the following:
        Compiler          :  XC32 1.42
        MPLAB 	          :  MPLAB X 3.55
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/

#include <xc.h>
#include "tmr2.h"
#include "../handlers.h"

/**
  Section: Data Type Definitions
*/

/** TMR Driver Hardware Instance Object

  @Summary
    Defines the object required for the maintainence of the hardware instance.

  @Description
    This defines the object required for the maintainence of the hardware
    instance. This object exists once per hardware instance of the peripheral.

  Remarks:
    None.
*/

typedef struct _TMR_OBJ_STRUCT
{
    /* Timer Elapsed */
    bool                                                    timerElapsed;
    /*Software Counter value*/
    uint8_t                                                 count;

} TMR_OBJ;

static TMR_OBJ tmr2_obj;

/**
  Section: Driver Interface
*/


void TMR2_Initialize (void)
{
    // TMR3 0; 
    TMR3 = 0x0;
    // TMR2 0; 
    TMR2 = 0x0;
    // PR3 0; 
    PR3 = 0x0;
    // Period = 0.01 s; Frequency = 1000000 Hz; PR2 10000; 
    PR2 = 0x2710;
    // TCKPS 1:1; T32 32 Bit; TCS PBCLK; SIDL disabled; TGATE disabled; ON enabled; 
    T2CON = 0x8008;

    IFS0CLR= 1 << _IFS0_T3IF_POSITION;
    IEC0bits.T3IE = true;
	
    tmr2_obj.timerElapsed = false;

}

void __ISR(_TIMER_3_VECTOR, IPL1AUTO) _T3Interrupt (  )
{

    //***User Area Begin

    // ticker function call;
    // ticker is 1 -> Callback function gets called everytime this ISR executes
    handle_timer( );

    //***User Area End

    tmr2_obj.count++;
    tmr2_obj.timerElapsed = true;
    IFS0CLR= 1 << _IFS0_T3IF_POSITION;
}

void TMR2_Period32BitSet( uint32_t value )
{
    /* Update the counter values */
    PR2 = (value & 0x0000FFFF);
    PR3 = ((value & 0xFFFF0000)>>16);
}

uint32_t TMR2_Period32BitGet( void )
{
    uint32_t periodVal = 0xFFFFFFFF;

    /* get the timer period value and return it */
    periodVal = (((uint32_t)PR3 <<16) | PR2);

    return( periodVal );

}

void TMR2_Counter32BitSet( uint32_t value )
{
    /* Update the counter values */
   TMR2 = (value & 0x0000FFFF);
   TMR3 = ((value & 0xFFFF0000)>>16);

}

uint32_t TMR2_Counter32BitGet( void )
{
    uint32_t countVal = 0xFFFFFFFF;

    /* get the current counter value and return it */
    countVal = (((uint32_t)TMR3<<16)| TMR2 );

    return( countVal );

}



void __attribute__ ((weak)) TMR2_CallBack(void)
{
    // Add your custom callback code here
}

void TMR2_Start( void )
{
    /* Reset the status information */
    tmr2_obj.timerElapsed = false;

    IFS0CLR= 1 << _IFS0_T3IF_POSITION;
    /*Enable the interrupt*/
    IEC0bits.T3IE = true;

    /* Start the Timer */
    T2CONbits.ON = 1;
}

void TMR2_Stop( void )
{
    /* Stop the Timer */
    T2CONbits.ON = false;

    /*Disable the interrupt*/
    IEC0bits.T3IE = false;
}

bool TMR2_GetElapsedThenClear(void)
{
    bool status;
    
    status = tmr2_obj.timerElapsed;

    if(status == true)
    {
        tmr2_obj.timerElapsed = false;
    }
    return status;
}

int TMR2_SoftwareCounterGet(void)
{
    return tmr2_obj.count;
}

void TMR2_SoftwareCounterClear(void)
{
    tmr2_obj.count = 0; 
}

/**
 End of File
*/
