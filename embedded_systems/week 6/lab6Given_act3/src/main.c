/*----------------------------------------------------------------------------
    Code for Lab 6

    In this project the PIT and the TPM are used to create a tone. 
 *---------------------------------------------------------------------------*/

#include <cmsis_os.h>
#include <MKL25Z4.H>
#include "../include/gpio.h"
#include "../include/pit.h"
#include "../include/tpmPwm.h"

osThreadId main_id;      /* id of the main task */
osThreadId t_tone;        /*  task id of task to flash led */
osThreadId t_button;     /* task id of task to read button */
osThreadId t_buttonTwo;
osThreadId t_volume;


/*--------------------------------------------------------------
 *     Tone task - switch tone on and off
 *--------------------------------------------------------------*/
// Audio tone states
#define TONE_ON (1)
#define TONE_OFF (0)

void toneTask (void const *arg) {
	const uint32_t scales[8] = {45867, 34361, 25742, 19284, 14447, 10823, 8108, 6074};
	uint32_t currentValue = 0;
	int audioState = TONE_OFF ; 
  while (1) {
		osSignalWait(0x0001, osWaitForever) ; // wait for an event flag 0x0001
		switch (audioState) {
			case TONE_ON:
				stopTimer(0) ;
				audioState = TONE_OFF ;
			  break ;
			case TONE_OFF:
				setTimer(0, scales[currentValue]) ;
				if(currentValue == 7){
					currentValue = 0;
				}
				else{
					currentValue++;
				}
				startTimer(0) ;
				audioState = TONE_ON ;
			  break ;
		}
		osSignalClear (t_tone, 0x0001); // discard pending notifications
  }
}

#define VOLUME_ON (1)
#define VOLUME_OFF (2)
void volumeTask(void const *arg){
	const uint32_t scales[9] = {0, 1, 2, 4, 8, 16, 32, 64, 128}; 
	uint32_t currentVolume = 0;
	uint32_t volumeState = VOLUME_OFF;
	while(1){
		osSignalWait(0x0001, osWaitForever);
			if(currentVolume == 9){
				currentVolume = 0;
			} else{
				currentVolume++;
			}
			setPWMDuty(scales[currentVolume]);
			osSignalClear (t_tone, 0x0001); // discard pending notifications
	}
}

/*------------------------------------------------------------
 *     Button task - poll button and send signal when pressed
 *------------------------------------------------------------*/

#define UP (0)
#define BOUNCE (1)
#define DOWN (2)

#define BOUNCEP (5)  // x delay give bounce time out


void buttonTask (void const *arg) {
	int bState = UP ;
	int bCounter = 0 ;
	
	while (1) {
		osDelay(10) ;
		if (bCounter) bCounter-- ;
		switch (bState) {
			case UP:
				if (isPressed()) {
					osSignalSet (t_tone, 0x0001);
					bState = DOWN ;
				}
				break ;
			case DOWN:
				if (!isPressed()) {
					bCounter = BOUNCEP ;
					bState = BOUNCE ;
				}
				break ;
			case BOUNCE:
				if (isPressed()) {
  				bCounter = BOUNCEP ;
					bState = DOWN ;
				} else {
					if (!bCounter) {
						bState = UP ;
					} 
				}
				break ;
		}
	}
}

void buttonTwoTask (void const *arg) {
	int bState = UP ;
	int bCounter = 0 ;
	
	while (1) {
		osDelay(10) ;
		if (bCounter) bCounter-- ;
		switch (bState) {
			case UP:
				if (isPressedTwo()) {
					osSignalSet (t_volume, 0x0001);
					bState = DOWN ;
				}
				break ;
			case DOWN:
				if (!isPressedTwo()) {
					bCounter = BOUNCEP ;
					bState = BOUNCE ;
				}
				break ;
			case BOUNCE:
				if (isPressedTwo()) {
  				bCounter = BOUNCEP ;
					bState = DOWN ;
				} else {
					if (!bCounter) {
						bState = UP ;
					} 
				}
				break ;
		}
	}
}

/*----------------------------------------------------------------------------
 *        Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/

osThreadDef (buttonTask, osPriorityHigh, 1, 0);
osThreadDef (buttonTwoTask, osPriorityHigh, 1, 0);
osThreadDef (toneTask, osPriorityHigh, 1, 0);
osThreadDef (volumeTask, osPriorityHigh, 1, 0);

int main (void) {
  configureGPIOinput() ;       // Initialise button
  configureGPIOoutput() ;      // Initialise output	
	configurePIT(0) ;            // Configure PIT channel 0
  //setTimer(0, 45867) ; // Frequency for MIDI 60 - middle C - 45867
	                     // One octave up is 22934
	configureTPM0forPWM() ;
	//setPWMDuty(64) ;     // 50% volume
	                     // Max is 128; off is 0 
	SystemCoreClockUpdate() ;
  osKernelInitialize ();
	
	// Create tasks
	t_tone = osThreadCreate(osThread(toneTask), NULL) ;
	t_button = osThreadCreate(osThread(buttonTask), NULL) ;
	t_buttonTwo = osThreadCreate(osThread(buttonTwoTask), NULL) ;
	t_volume = osThreadCreate(osThread(volumeTask), NULL) ;
  osKernelStart ();
	
	// main thread does not continue
	for (;;) ;

}
