/*
 * Copyright (c) 2017, NXP Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    clase4.c
 * @brief   Application entry point.
 */

#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_pit.h"

#define SW3_PORT PORTA
#define SW2_PORT PORTC
#define SW2_PIN 6
#define SW3_PIN 4

#define BLUE_RED_LED_PORT PORTB
#define GREEN_LED_PORT PORTE
#define BLUE_LED_PIN 21
#define RED_LED_PIN 22
#define GREEN_LED_PIN 26

/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */

/*
 * @brief   Application entry point.
 */

uint8_t state_stop = 0;
uint8_t state_reverse = 0;

void PORTA_IRQHandler()
{
	static uint8_t Pin_clearer = 1;
	PORT_ClearPinsInterruptFlags(SW3_PORT, Pin_clearer<<SW3_PIN);
	state_reverse = ( 0 == state_reverse ) ? 1 : 0;
}

void PORTC_IRQHandler()
{
	static uint8_t Pin_clearer = 1;
	PORT_ClearPinsInterruptFlags(SW2_PORT, Pin_clearer<<SW2_PIN);
	state_stop = ( 0 == state_stop ) ? 1 : 0;
}

void PIT0_IRQHandler()
{
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
	uint32_t period = 1;
	PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, period*CLOCK_GetBusClkFreq());
	PIT_StartTimer(PIT, kPIT_Chnl_0);
}

void blue_led_on()
{
	GPIO_WritePinOutput(GPIOB,BLUE_LED_PIN,0);
	GPIO_WritePinOutput(GPIOB,RED_LED_PIN,1);
	GPIO_WritePinOutput(GPIOE,GREEN_LED_PIN,1);
}

void red_led_on()
{
	GPIO_WritePinOutput(GPIOB,BLUE_LED_PIN,1);
	GPIO_WritePinOutput(GPIOB,RED_LED_PIN,0);
	GPIO_WritePinOutput(GPIOE,GREEN_LED_PIN,1);
}

void green_led_on()
{
	GPIO_WritePinOutput(GPIOB,BLUE_LED_PIN,1);
	GPIO_WritePinOutput(GPIOB,RED_LED_PIN,1);
	GPIO_WritePinOutput(GPIOE,GREEN_LED_PIN,0);
}

int main(void)
{

	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();

	CLOCK_EnableClock(kCLOCK_PortA);
	CLOCK_EnableClock(kCLOCK_PortB);
	CLOCK_EnableClock(kCLOCK_PortC);
	CLOCK_EnableClock(kCLOCK_PortE);

	pit_config_t Pit_config;
	PIT_GetDefaultConfig(&Pit_config);
	CLOCK_EnableClock(kCLOCK_Pit0);
	PIT_Init(PIT, &Pit_config);
	uint32_t period = 1;
	PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, period*CLOCK_GetBusClkFreq());
	PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
	PIT_StartTimer(PIT, kPIT_Chnl_0);
	EnableIRQ(PIT0_IRQn);


	port_pin_config_t config_led =
	{ kPORT_PullDisable, kPORT_SlowSlewRate, kPORT_PassiveFilterDisable,
			kPORT_OpenDrainDisable, kPORT_LowDriveStrength, kPORT_MuxAsGpio,
			kPORT_UnlockRegister, };

	PORT_SetPinConfig(BLUE_RED_LED_PORT, BLUE_LED_PIN, &config_led);
	PORT_SetPinConfig(BLUE_RED_LED_PORT, RED_LED_PIN, &config_led);


	port_pin_config_t config_switch =
	{ kPORT_PullDisable, kPORT_SlowSlewRate, kPORT_PassiveFilterDisable,
			kPORT_OpenDrainDisable, kPORT_LowDriveStrength, kPORT_MuxAsGpio,
			kPORT_UnlockRegister};
	PORT_SetPinInterruptConfig(SW2_PORT, SW2_PIN, kPORT_InterruptFallingEdge);
	PORT_SetPinInterruptConfig(SW3_PORT, SW3_PIN, kPORT_InterruptFallingEdge);

	PORT_SetPinConfig(SW2_PORT, SW2_PIN, &config_switch);
	PORT_SetPinConfig(SW3_PORT, SW3_PIN, &config_switch);

	gpio_pin_config_t led_config_gpio =
	{ kGPIO_DigitalOutput, 1 };

	GPIO_PinInit(GPIOB, BLUE_LED_PIN, &led_config_gpio);

	gpio_pin_config_t switch_config_gpio =
	{ kGPIO_DigitalInput, 1 };

	GPIO_PinInit(GPIOA, SW3_PIN, &switch_config_gpio);
	GPIO_PinInit(GPIOC, SW2_PIN, &switch_config_gpio);

	NVIC_EnableIRQ(PORTA_IRQn);
	NVIC_EnableIRQ(PORTC_IRQn);

	/* Force the counter to be placed into memory. */
	volatile static int i = 0;
	/* Enter an infinite loop, just incrementing a counter. */

	PRINTF("Hola mundo!");

	for(;;)
	{

	}
	return 0;
}
