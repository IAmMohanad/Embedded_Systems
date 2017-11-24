	/*  -------------------------------------------
    Lab 5: Demonstration of simple ADC (Version 2)

    1. The DAC input is ADC0_SE8 (ALT0 of PTB0), 
		     which is pin 2 on FRDM-KL25Z J10
	  2. Measures voltage when button pressed
		
		The system uses code from 
		  adc.c   - provides 4 functions for using the ADC
			SysTick.c  - provides 2 functions for using SysTick
    ------------------------------------------- */
#include <MKL25Z4.H>
#include <stdbool.h>
#include <stdint.h>

#include "..\include\gpio_defs.h"
#include "..\include\adc_defs.h"
#include "..\include\SysTick.h"


/* ------------------------------------------
   Initialise on board LEDs
	    Configuration steps
	      1. Enable pins as GPIO ports
	      2. Set GPIO direction to output
	      3. Ensure LEDs are off

   Red LED connected to Port B (PTB), bit 18 (RED_LED_POS)
   Green LED connected to Port B (PTB), bit 19 (GREEN_LED_POS)
   Blue LED connected to Port D (PTD), bit 1 (BLUE_LED_POS)
   Active-Low outputs: Write a 0 to turn on an LED
   ------------------------------------------ */
void init_LED() {

  // Make 3 pins GPIO
	PORTB->PCR[RED_LED_POS] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[RED_LED_POS] |= PORT_PCR_MUX(1);          
	PORTB->PCR[GREEN_LED_POS] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[GREEN_LED_POS] |= PORT_PCR_MUX(1);          
	PORTD->PCR[BLUE_LED_POS] &= ~PORT_PCR_MUX_MASK;          
	PORTD->PCR[BLUE_LED_POS] |= PORT_PCR_MUX(1);          

	// Set ports to outputs
	PTB->PDDR |= MASK(RED_LED_POS) | MASK(GREEN_LED_POS);
	PTD->PDDR |= MASK(BLUE_LED_POS);

    // Turn off LEDs
	PTB->PSOR = MASK(RED_LED_POS) | MASK(GREEN_LED_POS);
	PTD->PSOR = MASK(BLUE_LED_POS);
}

/*----------------------------------------------------------------------------
  GPIO Input Configuration

  Initialse a Port D pin as an input, with no interrupt
  Bit number given by BUTTON_POS
 *----------------------------------------------------------------------------*/
// 
void init_ButtonGPIO(void) {
	SIM->SCGC5 |=  SIM_SCGC5_PORTD_MASK; /* enable clock for port D */

	/* Select GPIO and enable pull-up resistors and no interrupts */
	PORTD->PCR[BUTTON_POS] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(0x0);
	
	/* Set port D switch bit to inputs */
	PTD->PDDR &= ~MASK(BUTTON_POS);
	
	SIM->SCGC5 |=  SIM_SCGC5_PORTE_MASK; /* enable clock for port E */
	
	
	
	
}


/*----------------------------------------------------------------------------
  isPressed: test the switch

  Operating the switch connects the input to ground. A non-zero value
  shows the switch is not pressed.
 *----------------------------------------------------------------------------*/
bool isPressed(void) {
	if (PTD->PDIR & MASK(BUTTON_POS)) {
			return false ;
	}
	return true ;
}

/*----------------------------------------------------------------------------
  checkButton

This function checks whether the button has been pressed
*----------------------------------------------------------------------------*/
int buttonState ; // current state of the button
int bounceCounter ; // counter for debounce
bool pressed ; // signal if button pressed

void init_ButtonState() {
	buttonState = BUTTONUP ;
	pressed = false ; 
}

void task1ButtonPress() {
	if (bounceCounter > 0) bounceCounter-- ;
	switch (buttonState) {
		case BUTTONUP:
			if (isPressed()) {
				buttonState = BUTTONDOWN ;
				pressed = true ; 
			}
		  break ;
		case BUTTONDOWN:
			if (!isPressed()) {
				buttonState = BUTTONBOUNCE ;
				bounceCounter = BOUNCEDELAY ;
			}
			break ;
		case BUTTONBOUNCE:
			if (isPressed()) {
				buttonState = BUTTONDOWN ;
			}
			else if (bounceCounter == 0) {
				buttonState = BUTTONUP ;
			}
			break ;
	}				
}

/*  -----------------------------------------
     Task 2: MeasureVoltage
       res - raw result
       measured_voltage - scaled
    -----------------------------------------   */
// declare volatile to ensure changes seen in debugger
volatile float measured_voltage ;  // scaled value
//code for section 4, activity 3
int cycles = 0;
float measurments[5];
float average;
//end code

void task2MeasureVoltage(void) {
	if (pressed) {
		// take a voltage reading
		MeasureVoltage() ;    // updates sres variable
		pressed = false ;     // acknowledge event
		
  	// scale to an actual voltage, assuming VREF accurate
    measured_voltage = (VREF * sres) / ADCRANGE ;	
		//code for section 4, activity 3
		if(cycles <= 4){
			measurments[cycles] = measured_voltage;
			cycles++;
		} else{
			average = 0;
			for(int i=0; i<cycles; i++){
				average += measurments[i];
			}
			average = average / cycles;
		}
		//end code
		
		// toggle red LED
		PTB->PTOR = MASK(RED_LED_POS) ;
	}
}

void task3MeasureVoltageDiff(void){
	
}

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
volatile uint8_t calibrationFailed ; // zero expected
int main (void) {
	// Enable clock to ports B, D and E
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK ;

	init_LED() ; // initialise LED
	init_ButtonGPIO() ; // initialise GPIO input
  init_ButtonState() ; // initialise button state variables
	Init_ADC() ; // Initialise ADC
	calibrationFailed = ADC_Cal(ADC0) ; // calibrate the ADC 
	while (calibrationFailed) ; // block progress if calibration failed
	Init_ADC() ; // Reinitialise ADC
	Init_SysTick(1000) ; // initialse SysTick every 1ms
	waitSysTickCounter(10) ;

	while (1) {		
		task1ButtonPress() ;
		task2MeasureVoltage() ;
		// delay
  	waitSysTickCounter(10) ;  // cycle every 10 ms
	}
}
