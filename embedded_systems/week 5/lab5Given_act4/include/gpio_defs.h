#ifndef GPIO_DEFS_H
#define GPIO_DEFS_H

#define MASK(x) (1UL << (x))

// Freedom KL25Z LEDs
#define RED_LED_POS (18)		// on port B
#define GREEN_LED_POS (19)	// on port B
#define BLUE_LED_POS (1)		// on port D

// Switches is on port D, pin 6
#define BUTTON_POS (6)
#define BUTTONUP 1 
#define BUTTONDOWN 2 
#define BUTTONBOUNCE 3
#define BOUNCEDELAY 5      // cycles to delay for 


#endif
// *******************************ARM University Program Copyright © ARM Ltd 2013*************************************   
