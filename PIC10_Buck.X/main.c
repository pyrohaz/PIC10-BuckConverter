/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */

//3.3*255/5 = 168.3
#define DREF    ((uint8_t)168)

volatile uint8_t aConv = 0, dutyCycle;
volatile uint8_t tick = 0;

//Delay for 'd' ticks. Maximum delay of 255 (~4ms)
void delay(uint8_t d){
    tick = 0;
    while(tick<d);
}


void interrupt isr(void){ 
    //Timer 2 overflow ISR
    if(PIR1bits.TMR2IF){
        PIR1bits.TMR2IF = 0;
        
        //Get ADC value from previous conversion
        aConv = ADRES;
        
        //Execute controller! If the read analog value is lower than the 
        //reference and the duty cycle hasn't saturated, increment the duty
        //cycle by 1. If the read analog value is more than the reference
        //and the duty cycle is more than 0, decrement the duty cycle.
        
        if(aConv<DREF && dutyCycle < 64) dutyCycle++;
        else if(dutyCycle>0) dutyCycle--;
        
        //Write the duty cycle to the PWM register
        PWM1DCH = dutyCycle;
        
        //Increment the tick counter
        tick++;
        
        //Start conversion for next cycle
        ADCONbits.GO_nDONE = 1;
    }
}

void main(void)
{    
    //Set up ADC
    TRISAbits.TRISA2 = 1;
    ADCON = 0xA9;
    
    //Enable timer 2 interrupt
    PIE1bits.TMR2IE = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    
    //Set up PWM
    OSCCONbits.IRCF = 0x7;
    TRISAbits.TRISA0 = 1;
    PWM1CON = 0;
    PR2 = 64;
    PWM1DCH = 0;
    PWM1DCL = 0;
    PIR1bits.TMR2IF = 0;
    T2CON = 0x04;
    PWM1CON = 0xF0;
    
    //Wait for timer overflow
    while(!PIR1bits.TMR2IF);
    TRISAbits.TRISA0 = 0;
    
    //Initialize RA1 for step response sink
    TRISAbits.TRISA1 = 0;

    while(1)
    {
        //Sink current
        PORTAbits.RA1 = 0;
        delay(100);
        
        //Stop sinking
        PORTAbits.RA1 = 1;
        delay(100);
        delay(200);
    }
}

