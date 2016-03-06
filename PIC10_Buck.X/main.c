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

void delay(uint8_t d){
    tick = 0;
    while(tick<d);
}

void interrupt isr(void){    
    if(PIR1bits.TMR2IF){
        PIR1bits.TMR2IF = 0;
        //Start ADC Conversion
        ADCONbits.GO_nDONE = 1;

        aConv = ADRES;
        if(aConv<DREF && dutyCycle < 64) dutyCycle++;
        else if(dutyCycle>0) dutyCycle--;
        
        PWM1DCH = dutyCycle;
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
    
    while(!PIR1bits.TMR2IF);
    TRISAbits.TRISA0 = 0;
    PWM1DCH = 10;
    
    TRISAbits.TRISA1 = 0;

    while(1)
    {
        PORTAbits.RA1 = 0;
        delay(100);
        PORTAbits.RA1 = 1;
        delay(100);
        delay(200);
    }
}

