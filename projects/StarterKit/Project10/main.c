/*
 * ArduinoStarterKitProject10.c
 *
 * PCINT1 (PB1 / 9)  Pin Change Interrupt Request 1 used to switch on/off motor
 * PCINT2 (PB2 / 10) Pin Change Interrupt Request 2 used to switch motor direction
 *
 *
 * Created: 23.09.2019 18:54:08
 * Author : Michael
 */ 

#include "globalDefines.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>

#include "myUSART.h"
#include "myADC.h"
#include "myVariableTypes.h"

// global Variable (volatile) that can be changed by interrupt routines and used throughout the program.
volatile uint8_t motorSwitch = 0b00000000; // switched by PB1
volatile uint8_t motorDirection = 0b00000000; // switched by PB2

volatile uint8_t historyOfPortB = 0x00;

void PWM16_init();

int main(void)
{
	// initialize USART functionality
	USART_init();
	USART_writeString("Motor Switch: ");
	USART_writeString(uint82char(motorSwitch));
	USART_writeString(", Motor Direction: ");
	USART_writeString(uint82char(motorDirection));
	USART_writeString("\r\n");
	
	// initialize PWM on Pin PB1 (Arduino Pin 9)
	PWM16_init();
	
	// clear global interrupt flag to allow for interrupted calibration of the input analog Pin without any interrupts
	cli();
	
	// initialize ADC and calibrate poti for 5 seconds
	ADC_init();
	struct pairOfTwoUint16 detectedMinMaxPotiValue = ADC_calibrateAnalogPin(0, 10);
	
	// set PD2 and PD3 as output Pins and start with 0
	// Those two pins will steer the motor direction
	DDRD |= (1 << DDD3) | (1 << DDD2);
	PORTD &= ~((1 << PORTD3) | (1 << PORTD2));
	

	// set PB4 and PB5 as input PIN (pull-up resistor in PORTB not required as we use pull-down register on bread board)
	// PB4 and PB5 are used as for the input of the two switches
	DDRB &= ~((1 << DDB5) | (1 << DDB4));
	
	// Pin Change Interrupt Control Register
	// When the PCIE0 bit is set (one) and the I-bit in the Status Register (SREG) is set (one), pin change interrupt 0 is enabled. 
	// Any change on any enabled PCINT[7:0] pin will cause an interrupt. The corresponding interrupt of Pin Change Interrupt Request
	// is executed from the PCI0 Interrupt Vector. PCINT[7:0] pins are enabled individually by the PCMSK0 Register.
	PCICR |= (1 << PCIE0);
	
	// Pin Change Mask Register 0
	// Each PCINT[7:0] bit selects whether pin change interrupt is enabled on the corresponding I/O pin. 
	// If PCINT[7:0] is set and the PCIE0 bit in PCICR is set, pin change interrupt is enabled on the corresponding I/O pin.
	// If PCINT[7:0] is cleared, pin change interrupt on the corresponding I/O pin is disabled.
	PCMSK0 |= (1 << PCINT5) | (1 << PCINT4);
	
	// enable global interrupts
	sei();

    while (1) 
    {
		uint16_t potiValue = ADC_readAnalogPin(0); // read analog input pin A0 on Arduino
		uint16_t calibratedPotiValue = mapSensorValueToFullRange(potiValue, detectedMinMaxPotiValue.sensorLowerBound, detectedMinMaxPotiValue.sensorUpperBound, 5000, 29999);
		USART_writeString("Poti Value: ");
		USART_writeString(uint162char(potiValue));
		USART_writeString(", Calibrated Poti Value: ");
		USART_writeString(uint162char(calibratedPotiValue));
		USART_writeString("\r\n");
		
		if (motorSwitch == 1) {
			// adjust the direction of motor rotation
			if (motorDirection == 0) {
				sbi(PORTD,3);
				cbi(PORTD,2);
			} else if (motorDirection == 1) {
				cbi(PORTD,3);
				sbi(PORTD,2);
			} else {
				USART_writeString("Unknown Motor Direction!\r\n");
			}
			
			// add PWM (by setting the comparison value)
			OCR1A = calibratedPotiValue;
			
		} else {
			// when both driver input pin of the half-H are set to HIGH or LOW at the same time, the motor will stop
			cbi(PORTD,3);
			cbi(PORTD,2);
			USART_writeString("Motor switched off\r\n");
		}

		_delay_ms(1000);
    }
}

ISR(PCINT0_vect) {
	
	uint8_t changedBits;
	
	changedBits = PINB ^ historyOfPortB;
	historyOfPortB = PINB;

	if(changedBits & (1 << DDB4))
	{
		// invert bit 0 of the uint8_t motorSwitch
		motorSwitch ^= (1 << 0);
		USART_writeString("Motor Switch: ");
		USART_writeString(uint82char(motorSwitch));
		USART_writeString("\r\n");
	}
	
	if(changedBits & (1 << DDB5))
	{
		// invert bit 0 of the uint8_t motorSwitch
		motorDirection ^= (1 << 0);
		USART_writeString("Motor Direction: ");
		USART_writeString(uint82char(motorDirection));
		USART_writeString("\r\n");
	}

}

// initialize PWM output pin PB1 (Arduino Pin ~9) with pre-scaler 8 in Waveform Generation Mode and Phase Correct non-inverting mode starting with counter = 0.
void PWM16_init(){

	// set non-inverting fast PWM mode for both pins
	// COM = Compare Output Mode. Only setting COMnx1 to 1 equals the non-inverting mode
	TCCR1A = (1 << COM1A1) | (1 << COM1B1);
	
	// WGM = Waveform Generation Mode
	// Phase Correct and ICR1 as TOP
	TCCR1A |= (1 << WGM11);
	TCCR1B = (1 << WGM13);
	
	// set pre-scaler to 8
	TCCR1B |= (1 << CS11);
	
	// set cycle length (ICR1 = TOP in chosen Waveform Generation Mode)
	ICR1 = 39999; // #define TOP_COUNTER 39999
	
	// set timer/counter to 0
	TCNT1 = 0;
	
	// set pins OC1A (= PB1 = ~9) to Output.
	DDRB |= (1 << DDB1);
}
