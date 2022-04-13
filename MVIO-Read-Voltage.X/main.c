/*
    (c) 2022 Microchip Technology Inc. and its subsidiaries.
    Subject to your compliance with these terms, you may use Microchip software and any
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party
    license terms applicable to your use of third party software (including open source software) that
    may accompany Microchip software.
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS
    FOR A PARTICULAR PURPOSE.
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS
    SOFTWARE.
*/

#define F_CPU 4000000UL
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>

#define USART0_BAUD_RATE(BAUD_RATE)     (uint16_t)((float)(F_CPU * 64 / (16 * (float)(BAUD_RATE))) + 0.5)

/* 12 bit ADC, 16 samples accumulated, VDDIO2 is divided by 10 */
#define ADC_VREF                        1.024
#define VALUE_TO_VOLTAGE(x)             (float)(( (x) * ADC_VREF * 10.0) / (4096.0 * 16.0) )
#define DELAY                           500

/* Default fuses configuration:
- BOD disabled
- Oscillator in High-Frequency Mode
- UPDI pin active(WARNING: DO NOT CHANGE!)
- RESET pin used as GPIO
- CRC disabled
- MVIO enabled for dual supply
- Watchdog Timer disabled
*/
FUSES =
{
.BODCFG = ACTIVE_DISABLE_gc | LVL_BODLEVEL0_gc | SAMPFREQ_128Hz_gc | SLEEP_DISABLE_gc,
.BOOTSIZE = 0x0,
.CODESIZE = 0x0,
.OSCCFG = CLKSEL_OSCHF_gc,
.SYSCFG0 = CRCSEL_CRC16_gc | CRCSRC_NOCRC_gc | RSTPINCFG_GPIO_gc | UPDIPINCFG_UPDI_gc,
.SYSCFG1 = MVSYSCFG_DUAL_gc | SUT_0MS_gc,
.WDTCFG = PERIOD_OFF_gc | WINDOW_OFF_gc,
};

void     ADC_Init(void);
void     USART0_Init(void);
uint16_t VDDIO2_Read(void);
int      USART_printChar(char c, FILE *stream);

FILE     USART_stream = FDEV_SETUP_STREAM(USART_printChar, NULL, _FDEV_SETUP_WRITE);


void ADC_Init(void)
{
    VREF.ADC0REF = VREF_REFSEL_1V024_gc;
	ADC0.CTRLB = ADC_SAMPNUM_ACC16_gc; /* accumulation of 16 samples */
	ADC0.CTRLA = ADC_ENABLE_bm;
}

void USART0_Init(void)
{
    PORTD.DIRSET = PIN4_bm;
    USART0.BAUD = USART0_BAUD_RATE(115200);
    USART0.CTRLB = USART_TXEN_bm;
    stdout = &USART_stream;
    PORTMUX.USARTROUTEA = PORTMUX_USART0_ALT3_gc;   /* USART0 routed to PORTD that is connected to CDC */
}

int USART_printChar(char c, FILE *stream)
{
    (void)stream;
    while (!(USART0.STATUS & USART_DREIF_bm));
    USART0.TXDATAL = c;
    return 0;    
}

uint16_t VDDIO2_Read(void)
{
    /* reading VDDIO2 voltage divided by 10 */
    ADC0.MUXPOS = ADC_MUXPOS_VDDIO2DIV10_gc;
	ADC0.COMMAND = ADC_STCONV_bm;

	while(!(ADC0.INTFLAGS & ADC_RESRDY_bm));
    return ADC0.RES;
}

int main(void)
{
    ADC_Init();
    USART0_Init();
    
    while(1)
    {
        uint16_t adcval;
        adcval = VDDIO2_Read();
        printf("Measured VDDIO2 voltage: %.4f V\n\r", VALUE_TO_VOLTAGE(adcval));
        _delay_ms(DELAY);
    }
}
