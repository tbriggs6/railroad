/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC32MX MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC32MX MCUs - pic32mx : v1.35
        Device            :  PIC32MX130F064B
        Driver Version    :  2.00
    The generated drivers are tested against the following:
        Compiler          :  XC32 1.42
        MPLAB             :  MPLAB X 3.55
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

#include "mcc_generated_files/mcc.h"

typedef enum {
    LED1, LED2, LED3
} led_state_t;

typedef enum {
    OFF, NORMAL, ON
} run_mode_t;

typedef struct {
    run_mode_t run_mode;
    uint32_t run_delay;
    uint32_t current_delay;
    led_state_t run_state;
    uint32_t daylight_count;
} global_state_t;

volatile global_state_t state;

typedef struct {
    uint32_t xmin;
    uint32_t ymin;
} lut_t;

typedef enum {
    DELAY, DAYLIGHT, DAYLIGHT_SET
} adc_channel_t;

#define SETPOINT_DEADZONE (50)
#define SETPOINT_OFF (0 + SETPOINT_DEADZONE)
#define SETPOINT_ON (1024 - SETPOINT_DEADZONE)
#define DAYLIGHT_MAX_COUNT 1000

typedef struct {
    uint32_t min;
    uint32_t max;
} range_t;

uint32_t compute_ratio(range_t x_range, range_t y_range, uint32_t x)
{
    // the fixed-point scale factors
    const uint32_t scale_delta = 1 << 7;
    const uint32_t scale_x = 1 << 14;
    
    // compute deltas
    uint32_t x_delta = x_range.max - x_range.min;
    uint32_t y_delta = y_range.max - y_range.min;
    
    // compute x's place in the range
    uint32_t x_in_range = x - x_range.min;
    
    // convert to fixed point
    uint32_t scaled_x_delta = x_delta * scale_delta;
    uint32_t scaled_x = x_in_range * scale_x;
    
    // compute the ratio (in fixed point)
    uint32_t ratio = scaled_x / scaled_x_delta;
    
    // compute the ratio
    uint32_t scaled_y = ratio * y_delta;
    
    // convert back from fixed point
    uint32_t value = scaled_y / (scale_x / scale_delta);
    
    // slide the scaled y back to its place in the range
    value = value + y_range.min;
    
    return value;
}


uint32_t compute_delay(uint32_t adc_value)
{
    static const lut_t lookup[5] = {
        { .xmin = SETPOINT_OFF, .ymin = 20 },
        { .xmin = 500, .ymin = 200 },
        { .xmin = 900, .ymin = 1500 },
        { .xmin = SETPOINT_ON, .ymin = 6000 },
        { .xmin = UINT32_MAX, .ymin = 50000 }
    };
    
    // gracefully handle errors in other parts of the code
    if (adc_value <= SETPOINT_OFF) return lookup[0].ymin;
    if (adc_value >= SETPOINT_ON) return lookup[4].ymin; 
    
    // find the entry in the table that adc_value comes *between*
    int i=0;
    while( (adc_value > lookup[i].xmin) && (adc_value > lookup[i+1].xmin)) {
        i++;
    }
    
    range_t adc_range = { .min = lookup[i].xmin, .max = lookup[i+1].xmin };
    range_t lut_range = { .min = lookup[i].ymin, .max = lookup[i+1].ymin };
    
    uint32_t ratio = compute_ratio(adc_range, lut_range, adc_value);

    if (ratio < lookup[0].ymin) ratio = lookup[0].ymin;
    if (ratio > lookup[4].ymin) return lookup[4].ymin; 
    return ratio;
}


void handle_adc( )
{
    uint32_t samps[ADC1_MAX_CHANNEL_COUNT];
  
    ADC1_ConversionResultBufferGet((uint32_t *)&samps, ADC1_MAX_CHANNEL_COUNT);

    if (samps[DAYLIGHT_SET] < SETPOINT_OFF) 
        state.run_mode = OFF;
    else if (samps[DAYLIGHT_SET] > SETPOINT_ON)
        state.run_mode = ON;
    else {
        
        if (samps[DAYLIGHT] < samps[DAYLIGHT_SET]) {
            state.daylight_count++;
            
            if (state.daylight_count > DAYLIGHT_MAX_COUNT) {
                // saturate the daylight counter and then turn off
                state.daylight_count = DAYLIGHT_MAX_COUNT; 
                state.run_mode = OFF;
            }
            else
                state.run_mode = NORMAL;
        }
        else {
            if (state.daylight_count > 0)
                state.daylight_count--;
            
            if (state.daylight_count == 0)
                state.run_mode = NORMAL;
            else
                state.run_mode = OFF;
            
        }
    }
    
    if ((state.run_mode == NORMAL) || (state.run_mode == ON)){
        uint32_t value_delay = samps[DELAY];
        state.run_delay = compute_delay(value_delay);
    }
    
 
    
}

void set_leds(int ledA, int ledB, int ledC)
{
    if (ledA == 1) {
        LATBbits.LATB2 = 1;
        LATBbits.LATB8 = 1;
    }
    else {
        LATBbits.LATB2 = 0;
        LATBbits.LATB8 = 0;
    }
    
    if (ledB == 1) {
        LATBbits.LATB3 = 1;
        LATBbits.LATB7 = 1;
    }
    else {
        LATBbits.LATB3 = 0;
        LATBbits.LATB7 = 0;
    }
    
    if (ledC == 1) {
        LATAbits.LATA2 = 1;
        LATBbits.LATB6 = 1;
    }
    else {
        LATAbits.LATA2 = 0;
        LATBbits.LATB6 = 0;
    }
}


void handle_timer( )
{
    WDTCONSET = 0x01; // Service the WDT
    
    if (state.run_mode == OFF) {
        state.current_delay = 0;
        state.run_state = LED1;
    }
    
    state.current_delay++;
    if (state.current_delay > state.run_delay) {
        state.current_delay = 0;
        
        switch(state.run_state) {
            case LED1: state.run_state = LED2;
            break;
            
            case LED2: state.run_state = LED3;
            break;
            
            case LED3: state.run_state = LED1;
            break;
            
            default: state.run_state = LED1;
            break;
            
        } // end switch
    } // end if
    
    if (state.run_mode == OFF){
        set_leds(0,0,0);
    }
    else {
        switch(state.run_state) {
            case LED1: set_leds(1,0,0);
            break;
            
            case LED2: set_leds(0,1,0);
            break;
            
            case LED3: set_leds(0,0,1);
            break;
            
            default: set_leds(0,0,0);
            break;
        } // end switch
    } // end else

       ADC1_Sample();
}  // end handle_timer

/*
                         Main application
 */
int main(void)
{
//    // initialize the device
    SYSTEM_Initialize();
    
    
    while (1)
    {
        // Add your application code
    }

    return -1;
}
/**
 End of File
*/