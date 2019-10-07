/**
 ****************************************************************************************
 *
 * @file user_wakeup.c
 *
 * @brief uder wakeup source code.
 *
 * Copyright (c) 2015-2019 Dialog Semiconductor. All rights reserved.
 *
 * This software ("Software") is owned by Dialog Semiconductor.
 *
 * By using this Software you agree that Dialog Semiconductor retains all
 * intellectual property and proprietary rights in and to this Software and any
 * use, reproduction, disclosure or distribution of the Software without express
 * written permission or a license agreement from Dialog Semiconductor is
 * strictly prohibited. This Software is solely for use on or in conjunction
 * with Dialog Semiconductor products.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE
 * SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. EXCEPT AS OTHERWISE
 * PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * DIALOG SEMICONDUCTOR BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "wkupct_quadec.h" 
#include "user_wakeup.h"
#include "arch.h"
#include "user_periph_setup.h"
#include "uart.h"
#include "uart_utils.h"
#include "user_barebone.h"
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/
#if defined(__DA14531__)
/**
 ****************************************************************************************
 * @brief reset function for the event counter of the DA14531
 * @return void
 ****************************************************************************************
 */
void reset_event_counter(void){

   SetWord16(WKUP_IRQ_STATUS_REG, WKUP_CNTR_RST);

}
#endif

/**
 ****************************************************************************************
 * @brief Button press callback function. Registered in WKUPCT driver.
 * @return void
 ****************************************************************************************
 */
void app_wakeup_press_cb(void)
{
	SetBits16(WKUP_CTRL_REG, WKUP_ENABLE_IRQ, INTERRUPT_ENABLE); //Resets interupt
	if (arch_ble_ext_wakeup_get())
	{
		arch_set_sleep_mode(ARCH_SLEEP_OFF);
		arch_ble_force_wakeup();
		arch_ble_ext_wakeup_off();
		
		#if !defined (__DA14531__)
				if (GetBits16(SYS_STAT_REG, PER_IS_DOWN))
				{
						periph_init();
				}
		#endif


		// If wakeup is generated by SW2
		if(!GPIO_GetPinStatus(GPIO_SW2_PORT, GPIO_SW2_PIN))						
		{
				GPIO_SetActive(LED_PORT, LED_PIN);										
				#ifdef CFG_PRINTF_UART2
						printf_string(UART2,"\n\n\rWakeup source: SW2");
				#endif
		}
		else if (!GPIO_GetPinStatus(GPIO_SW3_PORT, GPIO_SW3_PIN))
		{
				#ifdef CFG_PRINTF_UART2
						printf_string(UART2,"\n\n\rWakeup source: SW3");
				#endif
		}

		// If state is idle, start advertising
		if (ke_state_get(TASK_APP) == APP_CONNECTABLE)
		{
				user_app_adv_start();
		}
		
	}
	else if (!arch_ble_ext_wakeup_get()){
		#ifdef CFG_PRINTF_UART2
				printf_string(UART2,"\n\n\rSystem going to sleep");
		#endif
		GPIO_SetInactive(LED_PORT, LED_PIN);

		#ifdef SLEEP_WITHOUT_ADVERTISING
				app_easy_gap_advertise_stop();
		#endif

		arch_set_sleep_mode(ARCH_EXT_SLEEP_ON);				
		arch_ble_ext_wakeup_on();
		
	}                                                               
	#if defined(__DA14531__)
	reset_event_counter(); // When callback is triggerd the event counter is not set to 0 for the DA14531, that is why this function is called. 
	#endif								 // For the DA14585/6 the event counter is set to 0 even before reaching this (app_wakeup_press_cb) callback funtion.
												  	
}


/**
 ****************************************************************************************
 * @brief Sets two buttons as wakeup trigger
 * @return void
 ****************************************************************************************
*/
void user_wakeup_example_init(void)
{
		wkupct_enable_irq((WKUPCT_PIN_SELECT(GPIO_SW2_PORT, GPIO_SW2_PIN) | WKUPCT_PIN_SELECT(GPIO_SW3_PORT, GPIO_SW3_PIN)), 						// When enabling more than one interruptsource use OR bitoperation. WKUPCT_PIN_SELECT will make sure the appropriate bit in the register is set. 
											(WKUPCT_PIN_POLARITY(GPIO_SW2_PORT, GPIO_SW2_PIN, WKUPCT_PIN_POLARITY_LOW) | WKUPCT_PIN_POLARITY(GPIO_SW3_PORT, GPIO_SW3_PIN, WKUPCT_PIN_POLARITY_LOW)),	// When enabling more than one interruptsource use OR bitoperation. WKUPCT_PIN_POLARITY will make sure the appriopriate bit in the register is set.
											EVENTS_BEFORE_INTERRUPT,																																																													
											DEBOUNCE_TIME);																																																												
	
		wkupct_register_callback(app_wakeup_press_cb);	// sets this function as wake-up interrupt callback
}

/// @} APP
