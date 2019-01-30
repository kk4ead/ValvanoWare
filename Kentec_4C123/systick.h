//*********************** systick.h ***********************
// Written by:
// -    Steven Prickett
// -    YeonGi Jeon
// Date Created: 11/3/2012
// Last Modified: 11/8/2012
//
// Description:
// - Provides funcitons to access the SysTick timer
// ********************************************************

// ************** SysTick_Init ****************************
// - Initializes the Systick timer
// ********************************************************
// Input: none
// Output: none
// ********************************************************
void SysTick_Init(void);

// ************** SysTick_SetPeriod ***********************
// - Set Systick reload value
// ********************************************************
// Input: period in clock cycles
// Output: none
// ********************************************************
void SysTick_SetPeriod(unsigned long period);

// ************** SysTick_Reset ***************************
// - Resets the current SysTick value
// ********************************************************
// Input: none
// Output: none
// ********************************************************
void SysTick_Reset(void);

// ************** SysTick_Enable **************************
// - Enables the SysTick counter
// ********************************************************
// Input: none
// Output: none
// ********************************************************
void SysTick_Enable(void);

// ************** SysTick_Disable *************************
// - Disables the SysTick counter
// ********************************************************
// Input: none
// Output: none
// ********************************************************
void SysTick_Disable(void);
   
// ************** SysTick_EnableInterrupts ****************
// - Enables SysTick interrupts
// ********************************************************
// Input: none
// Output: none
// ********************************************************     
void SysTick_EnableInterrupts(void);

// ************** SysTick_DisableInterrupts ***************
// - Disables SysTick interrupts
// ********************************************************
// Input: none
// Output: none
// ********************************************************
void SysTick_DisableInterrupts(void);

// ************** SysTick_SetInterruptPriority ************
// - Sets the SysTick interrupt priority to 2 (of 5)
// ********************************************************
// Input: none
// Output: none
// ********************************************************
void SysTick_SetInterruptPriority(void);

// ************** SysTick_GetPeriod ***********************
// - Returns the SysTick reload value 
// ********************************************************
// Input: none
// Output: SysTick reload value
// ********************************************************
unsigned long SysTick_GetPeriod(void);
 
// ************** SysTick_GetCurrent **********************
// - Returns the current SysTick value
// ********************************************************
// Input: none
// Output: current SysTick value
// ******************************************************** 
unsigned long SysTick_GetCurrent(void);

// ************** SysTick_Handler  ************************
// - Default SysTick interrupt handler
// ********************************************************
// Input: none
// Output: none
// ********************************************************
void SysTick_Handler(void);
