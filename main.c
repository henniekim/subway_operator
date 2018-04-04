/*
 --------------------------
| Subway operation display	|
| Program 2			|
| 20121449 Dong Hyun Kim	|
| 2017. 06. 15			|
 --------------------------
 */
 
 // Button Input Configuration : [ PC 13 : IN_BND] [ PC 14 : OUT_BND ] [ PC 15 : EMERG ] 
 // LED Output Configuration  : [ PC0 ~ PC7 ]	
 
#include <stm32f10x.h>

#define MAIN_STATE 0x00
#define IN_BND_STATE 0x01
#define OUT_BND_STATE 0x02
#define EMERG_STATE 0x03

#define LED_BLINK_SPEED 0x4FF

u8 led_data = 0;
u8 system_state = 0;
u8 save_state = 0;
u8 current_data = 0x80;

int main (void)
{
	RCC->APB2ENR &= 0x00000000; // Clock	RESET
	RCC->APB2ENR |= 0x00000010; // GPIOC Clock Enable : Use For LEDs
	
	RCC->APB1ENR |= 0x00000001; // TIM2 Clock Enable : Use For Blincking LEDs 
	
	GPIOC->CRL = 0x33333333;  // LED Output Configuration : PC0 ~ PC7	
	GPIOC->CRH = 0x88800000; // Button Input Configuration : [ PC 13 : IN_BND] [ PC 14 : OUT_BND ] [ PC 15 : EMERG ] 
  
	// EXTI Interrupt Configuration
	NVIC->ISER[1]   |= (1<<8); 	// Enable EXTI15_10 Interrupt 
	EXTI->IMR 	|= 0x0000E000; 	// Mask on Interrupt ( 1 : Not masked )
	EXTI->RTSR 	|= 0x0000E000;	// Rising Edge Trigger
	AFIO->EXTICR[3] |= 0x00002220;  // Interrupt use For 'PC'  
	
	// TIM2 Interrupt Configuration : LED BLINK CONTROL
	TIM2->CR1 = 0x00; 
	TIM2->CR2 = 0x00;
	TIM2->PSC = 0xFFF;
	TIM2->ARR = 0xFFF;
	TIM2->DIER = 0x0000;
	NVIC->ISER[0] |= (1<<28);
	TIM2->CR1 |= 0x0001;
	
	// Initializing
	GPIOC->ODR = 0x00000000;
	led_data = 0x00000000; // initializing main data 
	system_state = MAIN_STATE;
	
	while(1) // input code here
	{
		if (system_state == MAIN_STATE)
		{
			GPIOC->ODR = 0x00000000;
			EXTI->IMR |= 0x0000E000; 	// Mask on Interrupt ( 1 : Not masked )
			TIM2->DIER &= 0x0000;
		}
		else
		{	
			TIM2->DIER |= 0x0001;
		}
			
		if (system_state == IN_BND_STATE)
		{
			TIM2->PSC = LED_BLINK_SPEED ;
		}
		else if (system_state == OUT_BND_STATE)
		{
			TIM2->PSC = LED_BLINK_SPEED ;
			EXTI->IMR |= 0x0000E000; 	// Mask on Interrupt ( 1 : Not masked )
		}
		else if (system_state == EMERG_STATE)
		{
			TIM2->PSC = LED_BLINK_SPEED * 0.5;
		}
	}
}

void EXTI15_10_IRQHandler (void)
{
	if (system_state == MAIN_STATE)
	{	
		if (( EXTI->PR & 0x2000) != 0)  // FOR PC 13 : IN_BND
		{	
			led_data = 0x01;													system_state = IN_BND_STATE												EXTI->PR &= (1<<13);
		}	
		else if (( EXTI->PR & 0x4000) !=0) // FOR PC 14 : OUT_BND
		{																led_data = 0x80;
			system_state = OUT_BND_STATE;
			EXTI->PR &= (1<<14);
		}
		else if (( EXTI->PR & 0x8000) !=0) // FOR PC 15 : EMERG									{
			EXTI->PR &= (1<<15);
		}
	}
	else if (system_state == IN_BND_STATE)
	{
		if (( EXTI->PR & 0x2000) != 0)  // FOR PC 13 : IN_BND
		{																if (led_data != 0xFF)
			{
				led_data = led_data*2 + 1;
				system_state = IN_BND_STATE;
			}
			else if (led_data == 0xFF)
			{
				led_data = 0x00;
				system_state = MAIN_STATE;
			}
			EXTI->PR &= (1<<13); 
		}
		else if (( EXTI->PR & 0x4000) !=0) // FOR PC 14 : OUT_BND
		{								
			system_state = IN_BND_STATE;
			EXTI->PR &= (1<<14);
		}
		else if (( EXTI->PR & 0x8000) !=0) // FOR PC 15 : EMERG
		{
			save_state = system_state;
			system_state = EMERG_STATE;
			EXTI->PR &= (1<<15);
		}
	}
	else if (system_state == OUT_BND_STATE)
	{
		if (( EXTI->PR & 0x2000) != 0)  // FOR PC 13 : IN_BND
		{
			system_state = OUT_BND_STATE;
			EXTI->PR &= (1<<13); 
		}
		else if (( EXTI->PR & 0x4000) !=0) // FOR PC 14 : OUT_BND
		{
			if (led_data != 0xFF)
			{
				current_data = current_data /2 ;
				led_data  = led_data + current_data;
				system_state = OUT_BND_STATE;
			}
			else if (led_data == 0xFF)
			{
				led_data = 0x00;
				current_data = 0x80;
				system_state = MAIN_STATE;
			}
			EXTI->PR &= (1<<14);
		}
		else if (( EXTI->PR & 0x8000) !=0) // FOR PC 15 : EMERG
		{
			save_state = system_state;
			system_state = EMERG_STATE;
			EXTI->PR &= (1<<15);
		}
	}
	else if (system_state == EMERG_STATE)
	{
		if (( EXTI->PR & 0x2000) != 0)  // FOR PC 13 : IN_BND
		{
			EXTI->PR &= (1<<13); 
		}
		else if (( EXTI->PR & 0x4000) !=0) // FOR PC 14 : OUT_BND
		{
			EXTI->PR &= (1<<14);
		}
		else if (( EXTI->PR & 0x8000) !=0) // FOR PC 15 : EMERG
		{
			system_state = save_state;
			EXTI->PR &= (1<<15);
		}
	}
	NVIC->ICPR[1] |= (1<<8);
}
								

void TIM2_IRQHandler (void)
{
	if ((TIM2->SR & 0x0001) != 0)
	{
		if (GPIOC->ODR == 0x00000000)
		GPIOC->ODR = led_data;
		else
		GPIOC->ODR = 0x000000000;
		
		TIM2->SR &= ~(1<<0);
	}
}

