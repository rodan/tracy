
//  read adc conversions from any of the P6 ports
//
//  at least a 1ms delay should be inserted between two adc10_read()s or
//  between an adc10_read(port, &rv) and the use of rv.
//
//  author:          Petre Rodan <2b4eda@subdimension.ro>
//  available from:  https://github.com/rodan/
//  license:         GNU GPLv3

#include "adc.h"

volatile uint16_t *adc10_rv;
volatile uint8_t adcready;

// port: 0 = P6.A0, 1 = P6.A1, .., 0xa = P6.A10 = internal temp sensor
// vref is one of:  REFVSEL_0  - 1.5v vref
//                  REFVSEL_1  - 2.0v vref
//                  REFVSEL_2  - 2.5v vref
void adc10_read(const uint8_t port, uint16_t * rv, const uint8_t vref)
{
    //*((uint16_t *)portreg) |= 1 << port;
    // if ref or adc10 are busy then wait
    while (REFCTL0 & REFGENBUSY) ;
    while (ADC10CTL1 & ADC10BUSY) ;
    // enable reference
    if ((REFCTL0 & 0x30) != vref) {
        // need to change vref
        REFCTL0 &= ~(0x30 + REFON);
        REFCTL0 |= REFMSTR + vref + REFON;
    } else {
        REFCTL0 |= REFMSTR + REFON;
    }
    ADC10CTL0 &= ~ADC10ENC;
    // enable ADC10_A, single channel single conversion
    ADC10CTL0 = ADC10SHT_2 + ADC10ON;
    ADC10CTL1 = ADC10SHP + ADC10DIV1 + ADC10DIV0;
    // use internal Vref(+) AVss (-)
    ADC10MCTL0 = ADC10SREF_1 + port;
    ADC10CTL2 |= ADC10PDIV_2 + ADC10SR;
    adcready = 0;
    adc10_rv = rv;
    // trigger conversion
    ADC10IE = ADC10IE0;
    ADC10CTL0 |= ADC10ENC + ADC10SC;
    while (!adcready) ;
}

void adc10_halt(void)
{
    ADC10CTL0 &= ~ADC10ON;
    REFCTL0 &= ~REFON;
}

// calculate internal temperature based on the linear regression 
// established by the two calibration registers flashed into the chip
// qtemp the adc value on channel 10 with a 1.5V reference
// function returns the temperature in degrees C
int16_t calc_temp(const uint16_t qtemp)
{
    uint16_t x1 = *(uint16_t *)0x1a1a; // value at 30dC
    uint16_t x2 = *(uint16_t *)0x1a1c; // value at 85dC, see datasheet
    uint16_t y1 = 30;
    uint16_t y2 = 85;
    int32_t sumxsq;
    int32_t sumx, sumy, sumxy;
    int32_t coef1, coef2, t10;
    int32_t rv;

    sumx = x1 + x2;
    sumy = y1 + y2;
    sumxsq = (int32_t)x1 * (int32_t)x1 + (int32_t)x2 * (int32_t)x2;
    sumxy = (int32_t)x1 * (int32_t)y1 + (int32_t)x2 * (int32_t)y2;

    coef1 = ((sumy*sumxsq)-(sumx*sumxy))/((2*sumxsq)-(sumx*sumx))*100;
    coef2 = 100*((2*sumxy)-(sumx*sumy))/((2*sumxsq)-(sumx*sumx));

    t10 = (qtemp * coef2 + coef1)/10;
    rv = t10/10;

    // add 1 if first digit after decimal is > 4
    if ( (t10 % 10) > 4 ) {
        if (t10 > 0) {
            rv += 1;
        } else {
            rv -= 1;
        }
    }
    return rv;
}

__attribute__ ((interrupt(ADC10_VECTOR)))
void adc10_ISR(void)
{
    uint16_t iv = ADC10IV;
    if (iv == ADC10IV_ADC10IFG) {
        *adc10_rv = ADC10MEM0;
        adcready = 1;
    }
}
