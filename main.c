
/******************************************************************************	
* File Name    : main.c
* Version      : 1.0.0
* Device(s)    : R5F104PJ
* Tool-Chain   : CA78K0R
* Description  : This file implements for main function.
* Creation Date: 11-Sep-15
******************************************************************************/
#pragma interrupt INTIT r_it_interrupt
/******************************************************************************
Includes 
******************************************************************************/
#include "r_macro.h"  /* System macro and standard type definition */
#include "r_spi_if.h" /* SPI driver interface */
#include "lcd.h"      /* LCD driver interface */
#include "adc.h"      /* ADC driver interface */
#include <string.h>
#include "timer.h" /*API for this exercise*/
#include "uart.h"
#include "sw.h"
/******************************************************************************
Macro definitions
******************************************************************************/
#define FSR 			3.3 					/* Full scale range voltage input value */
#define LSB			0.00098*FSR				/* Percentage of analog input voltage per bit of digital output when resolution is 10 bits */
#define OVERALL_ERROR_VALUE 	1.2*LSB					/* Typical value of overall error */

/******************************************************************************
Private global variables and functions
******************************************************************************/
/* Declare a variable for A/D results */
volatile uint16_t gADC_Result = 0;
volatile float result;
volatile float previous;
volatile float diff;
volatile int G_elapsedTime = 0;   // Timer Counter

	char str1[21] = "Embedded SW tranining";
  	char str2[2]  = {0x0D, 0x0A};
  	char str3[18] = "UART communication";
  	unsigned char LCD_Line = LCD_LINE3;

/* Global buffer array for storing the ADC
result integer to string conversion  */
static uint8_t lcd_buffer[] = "=H\'WXYZ ";

void LCD_Reset(void);
void r_main_userinit(void);
extern void initial_LED(void);
void switch_LED(int, int, int, int, int, int, int, int, int, int, int, int);

/******************************************************************************
* Function Name: main
* Description  : Main program
* Arguments    : none
* Return Value : none
******************************************************************************/
void main(void)
{
	/* Declare a temporary variable */
	char *string[10];
    
   	/* Initialize UART1 communication */
   	Uart_Init();
	R_IT_Create();
	R_IT_Start();
	/* Initialize ADC module */
        ADC_Create();
	ADC_Set_OperationOn();
	/* Initialize external interrupt - SW1 */
   	 INTC_Create();
	/* Enable interrupt */
	EI();
	LCD_Reset();
	r_main_userinit();
	initial_LED();
	/* Display information on the debug LCD.*/
   	 DisplayLCD(LCD_LINE1, (uint8_t *)"UART 9600bps");
   	 DisplayLCD(LCD_LINE2, (uint8_t *)"Interface");
    
  	  /* Start UART1 communication */
  	  Uart_Start();
    
   	 /* Start external interrupt - SW1 */
   	 INTC10_Start();
	
	/* Main loop - Infinite loop */
	while (1U){	
	    if (G_elapsedTime >= 10){
			G_elapsedTime = 0;
			P4_bit.no1 = 0;
			/* Start an A/D conversion */
			ADC_Start();
			/* Wait for the A/D conversion to complete */
			while(Adc_Status != ADC_DONE);
			/* Clear ADC flag */
			Adc_Status = 0;
			/* Shift the ADC result contained in the 16-bit ADCR register */
			gADC_Result = ADCR >> 6;
			
			result = gADC_Result*FSR/1023;
			diff = (result - previous);
			
			if ((diff >= OVERALL_ERROR_VALUE)||(diff <= OVERALL_ERROR_VALUE)) {
				previous = result;
				if (result > 0.270)
				switch_LED(0,0,1,1,1,1,1,1,1,1,1,1);
			}
	    }
	    /* Check UART1 receive status */
     	 	if(status == UART_RECEIVE_DONE){
       			 status = 0;
			/* Replace the last element by NULL */
       			rx_buff[UART_RX_BUFFER_LEN - 1] = '\0';
	
        		/* Display string in receive buffer */
			DisplayLCD(LCD_Line, (uint8_t *)(&rx_buff[0]));
	
			/* Cheking for display in next line */
			if(0 == (rx_count%12)){
           		Uart_ClearBuff(&rx_buff[0], UART_RX_BUFFER_LEN - 1);
	   		LCD_Line = (unsigned char)(rx_count/12)*8 + LCD_LINE3;
			}else{
	 		 /* Do nothing */
			}
     		}else{
			if (UART_CLEAR == cmd){
        			cmd = 0;
				rx_count = 0;
        			ClearLCD();
        			DisplayLCD(LCD_LINE1, (uint8_t *)"UART 9600bps");
        			DisplayLCD(LCD_LINE2, (uint8_t *)"Interface");
        			Uart_ClearBuff(&rx_buff[0], UART_RX_BUFFER_LEN - 1);
				LCD_Line = LCD_LINE3;
     			}
		}
     
      
     		 /* Check Swtich 1 edge */
     		if('1' == gSwitchFlag){
      			Uart_Transmit(&(str1[0]), 21);
      			Uart_Transmit(&(str2[0]), 2);
        		Uart_Transmit(&(str3[0]), 18);
        		Uart_Transmit(&(str2[0]), 2);
	
			gSwitchFlag = 0;
     		}
	}
	 R_IT_Stop();
}
/******************************************************************************
* Function Name: switch_LED
* Description  : Switch led 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
* Arguments    : int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l
* Return Value : none
******************************************************************************/
void switch_LED(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l) {
	
			P4_bit.no2 = l;		// switch led 4
			P6_bit.no3 = k;		// switch led 5
			P4_bit.no3 = j;		// switch led 6
			P6_bit.no4 = i;		// switch led 7
			P4_bit.no4 = h;		// switch led 8
			P6_bit.no5 = g;		// switch led 9
			P4_bit.no5 = f;		// switch led 10
			P6_bit.no6 = e;		// switch led 11
			P15_bit.no2 = d;	// switch led 12
			P6_bit.no7 = c;		// switch led 13
			P10_bit.no1 = b; 	// switch led 14
			P6_bit.no2 = a; 	// switch led 3
}

__interrupt static void r_it_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
    G_elapsedTime++;
    /* End user code. Do not edit comment generated here */
}
/******************************************************************************
End of file
******************************************************************************/
