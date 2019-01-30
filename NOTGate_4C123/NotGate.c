/*; Runs on LM4F120 and TM4C123
 Implements a NOT gate described in class
 PD3 is an output to LED, Positive logic
 PD0 is an input from switch, Positive logic
 Switch pressed causes LED to go OFF and 
 release causes LED to go ON.
 **** To run this example in Simulator 
 make sure and copy the C0DLL.dll file 
(in the folder where this [NotGate.c] file is)
 to your Keil ARM/Bin folder
*/

#include "tm4c123gh6pm.h"
unsigned long In,Out;
int main(void){    volatile unsigned long delay;
// one time initialization goes here
	SYSCTL_RCGCGPIO_R |= 0x08;	// Turn clock on PortD
	delay = 100;					// Wait
	GPIO_PORTD_DIR_R |= 0x08;
	GPIO_PORTD_DIR_R &= ~(0x01);
	GPIO_PORTD_AFSEL_R &= ~(0x09);
	GPIO_PORTD_DEN_R |= 0x09;
	
  while(1){
// operations to be executed over and over go here
		In = GPIO_PORTD_DATA_R & 0x01;
		Out = ~In;
		GPIO_PORTD_DATA_R = Out<<3;
  }
}
