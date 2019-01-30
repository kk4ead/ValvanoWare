// *********************** systick.c **********************
// Written by:
// -    Steven Prickett
// -    YeonGi Jeon
// Date Created: 11/3/2012
// Last Modified: 11/8/2012
//
// Description:
// - Provides funcitons to access the SysTick timer
// ********************************************************
#include "../inc/tm4c123gh6pm.h"
#include "systick.h"

// ************** SysTick_Init ****************************
// - Initializes the Systick timer
// ********************************************************
// Input: none
// Output: none
// ********************************************************
void SysTick_Init(){
    // disable systick while it is being initialized
    SysTick_Disable();
    
    // set period to maximum value
    SysTick_SetPeriod(0xFFFFFF);
    
    // reset counter value
    SysTick_Reset();
    
    // set interrupt priority
    SysTick_SetInterruptPriority();
    
    // enable systick
    SysTick_Enable();
}

// ************** SysTick_Reset ***************************
// - Resets the current SysTick value
// ********************************************************
// Input: none
// Output: none
// ********************************************************
void SysTick_Reset(void){
    NVIC_ST_CURRENT_R = 0;
}

// ************** SysTick_SetPeriod ***********************
// - Set Systick reload value
// ********************************************************
// Input: period in clock cycles
// Output: none
// ********************************************************
void SysTick_SetPeriod(unsigned long period){
    NVIC_ST_RELOAD_R = (period - 1);
    NVIC_ST_CURRENT_R = 0;
}

// ************** SysTick_GetPeriod ***********************
// - Returns the SysTick reload value 
// ********************************************************
// Input: none
// Output: SysTick reload value
// ********************************************************
unsigned long SysTick_GetPeriod(void){
    return (NVIC_ST_RELOAD_R + 1);
}
 
// ************** SysTick_GetCurrent **********************
// - Returns the current SysTick value
// ********************************************************
// Input: none
// Output: current SysTick value
// ********************************************************
unsigned long SysTick_GetCurrent(void){
    return NVIC_ST_CURRENT_R;
}

// ************** SysTick_Enable **************************
// - Enables the SysTick counter
// ********************************************************
// Input: none
// Output: none
// ********************************************************
void SysTick_Enable(void){
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE |
                      NVIC_ST_CTRL_CLK_SRC;
}

// ************** SysTick_Disable *************************
// - Disables the SysTick counter
// ********************************************************
// Input: none
// Output: none
// ********************************************************
void SysTick_Disable(void){
    NVIC_ST_CTRL_R &= ~(NVIC_ST_CTRL_ENABLE);
}
    
// ************** SysTick_EnableInterrupts ****************
// - Enables SysTick interrupts
// ********************************************************
// Input: none
// Output: none
// ********************************************************
void SysTick_EnableInterrupts(void){
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_INTEN;
}

// ************** SysTick_DisableInterrupts ***************
// - Disables SysTick interrupts
// ********************************************************
// Input: none
// Output: none
// ********************************************************
void SysTick_DisableInterrupts(void){
    NVIC_ST_CTRL_R &= ~(NVIC_ST_CTRL_INTEN);
}

// ************** SysTick_SetInterruptPriority ************
// - Sets the SysTick interrupt priority to 2 (of 5)
// ********************************************************
// Input: none
// Output: none
// ********************************************************
void SysTick_SetInterruptPriority(void){
    NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) |
                      0x40000000; 
}
