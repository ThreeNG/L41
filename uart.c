
/***********************************************************************************************************************
* File Name    : uart.c
* Version      : Initial version 1.0.0
* Device(s)    : R5F104PJ
* Tool-Chain   : CA78K0R
* Description  : This file implements device driver for UART module.
* Creation Date: 09-Sep-15
***********************************************************************************************************************/

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
#pragma interrupt INTSR1 UartRx_isr
#pragma interrupt INTSRE1 UartError_isr

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_macro.h"
#include "uart.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
char rx_buff[UART_RX_BUFFER_LEN];
char* rx_buffer_ptr = &(rx_buff[0]);

char recieve_buff[UART_RX_BUFFER_LEN];
//char str2[25];
int first_check=0;
int error_flag=0;
unsigned int rx_Len;
unsigned char status = 0;
char cmd = 0;
unsigned int rx_count = 0;

unsigned int count = 0;



/*char rx_buff[UART_RX_BUFFER_LEN];
char* rx_buffer_ptr = &(rx_buff[0]);
unsigned int rx_Len;
unsigned char status = 0;
char cmd = 0;
unsigned int rx_count = 0;*/
/***********************************************************************************************************************
* Function Name: Uart_Init
* Description  : This function initializes the UART1.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void Uart_Init(void)
{
    /* Uart init UART0 */
    SAU0EN = 1U;   /*serial array unit 0 Enable*/
    NOP();
    NOP();
    NOP();
    NOP();
    SPS0 = 0x0004|0x0040; /*select clock for SAU*/
   
    ST0 = 0x04;      /* stop all channel */
    STMK1 = 1U;      /* disable INTST0 interrupt */
    STIF1 = 0U;      /* clear INTST0 interrupt flag */
    SRMK1 = 1U;      /* disable INTSR0 interrupt */
    SRIF1 = 0U;      /* clear INTSR0 interrupt flag */
    SREMK1 = 1U;     /* disable INTSRE0 interrupt */
    SREIF1 = 0U;     /* clear INTSRE0 interrupt flag */
    /* Set INTST0 low priority */
    STPR11 = 1U;
    STPR01 = 1U;
    /* Set INTSR0 low priority */
    SRPR11 = 1U;
    SRPR01 = 1U;
    /* Set INTSRE0 low priority */
    SREPR11 = 1U;
    SREPR01 = 1U;
    
    SMR02 = 0x0022;   /* UART mode, empty buffer interrupt*/
    SCR02 = 0x8797;   /*Transmit only, mask error interrupt, ODD parity, LSB first, stop bit 1 bit, data length 8 bit*/
    SDR02 = 0x3200U;  /*set clock transfer:38400kbps */
    NFEN0 = 0x04;     /*noise enable for RXD0*/
    
    SIR03 = 0x07;     /*clear flag trigger */
    SMR03 = 0x0122;   /*UART mode, valid edge of the RxDq pin, empty buffer interrupt*/
    SCR03 = 0x4797;   /*Recieve only, mask error interrupt, ODD parity, LSB first, stop bit 1 bit, data length 8 bit*/
    SDR03 = 0x3200U;  /*set clock transfer:38400kbps */
    
    SO0 |= 0x04;     /* output level normal */
    SOL0 |= 0x00;    /* output level normal */
    SOE0 |= 0x04;    /* enable UART1 output */
    
    /* Port inint for UART communicate */
    PMC0 &= 0xF7U;
    PM0 |= 0x08U;
    /* Set TxD1 pin */
    PMC0 &= 0xFBU;
    P0 |= 0x04U;
    PM0 &= 0xFBU;
}

/***********************************************************************************************************************
* Function Name: Uart_Start
* Description  : This function start UART1 operation.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void Uart_Start(void)
{
    SO0 |= 0x04;     /* output level normal */
    SOE0 |= 0x04;    /* enable UART0 output */
    SS0 |= 0x04|0x08;     /* enable UART0 receive and transmit */
    
    STIF1 = 0U;    /* clear INTST0 interrupt flag */
    SRIF1 = 0U;    /* clear INTSR0 interrupt flag */
    SREIF1 = 0U;   /* clear INTSRE0 interrupt flag */
    STMK1 = 1U;    /* disable INTST0 interrupt */
    SRMK1 = 0U;    /* enable INTSR0 interrupt */
    SREMK1 = 0U;   /* enable INTSRE0 interrupt */
}

/***********************************************************************************************************************
* Function Name: Uart_Stop
* Description  : This function stop UART1 operation.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void Uart_Stop(void)
{
    STMK1 = 1U;    /* disable INTST0 interrupt */
    SRMK1 = 1U;    /* disable INTSR0 interrupt */
    SREMK1 = 1U;   /* disable INTSRE0 interrupt */
    
    ST0 |= 0x04;     /* enable UART0 receive and transmit */
    SOE0 &= ~0x04;    /* enable UART0 output */
    
    STIF1 = 0U;    /* clear INTST0 interrupt flag */
    SRIF1 = 0U;    /* clear INTSR0 interrupt flag */
    SREIF1 = 0U;   /* clear INTSRE0 interrupt flag */
}

/***********************************************************************************************************************
* Function Name: Uart_Transmit
* Description  : This function transmit number of data bytes, using UART.
* Arguments    : Transmission data pointer, data length.
* Return Value : None
***********************************************************************************************************************/
void Uart_Transmit(const char* tx_ptr, int Len)
{
  unsigned int LuiCount = 0;
  unsigned int LuiTimeOut = 0;
  
  for(LuiCount = 0; LuiCount < Len; LuiCount++)
  {
    TXD1 = *(tx_ptr + LuiCount);
    LuiTimeOut = UART_TIMEOUT;
    while((STIF1 == 0)||(LuiTimeOut--));
    STIF1 = 0;
  }
}

/***********************************************************************************************************************
* Function Name: Uart_ClearRxBuff
* Description  : This function clear Rx buffer of UART
* Arguments    : Buffer pointer, buffer length.
* Return Value : None
***********************************************************************************************************************/
void Uart_ClearBuff(char* buff_ptr, unsigned int Len)
{
  unsigned int LuiCount = 0;
  for(LuiCount = 0; LuiCount < Len; LuiCount ++)
  {
    *(buff_ptr + LuiCount) = ' ';
  }
}

/***********************************************************************************************************************
* Function Name: UartRx_isr
* Description  : This interrupt service routine of UART1
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt void UartRx_isr(void)
{
            if(first_check==0){
		if('$' != RXD1){
			error_flag=MSG_ERR;
		} else{
			first_check=1;
		}
            }
            
            if(RXD1=='^'){
 		if(first_check==1){
 			if(count==0){
				error_flag=MSG_ERR;
			}
 			count=0;
                        rx_count=0;
                        first_check=0;
                        /* Update status after receiving */
                        status = UART_RECEIVE_DONE;            
                }
            }
            else{
                 recieve_buff[count]=RXD1;
                 count++;
                 rx_count++;
                 if(count > (UART_RX_BUFFER_LEN-2)){
                     count = 0;
                 }else{
                        // Do no thing 
                 }
            }
}

/***********************************************************************************************************************
* Function Name: UartError_isr
* Description  : This interrupt service routine of UART1 when error occurs
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt void UartError_isr(void){
	if ((SSR03&0x0004U) == 0x0004U){
	      status = UART_FRAMING_ERROR;
	}
      	else if ((SSR03&0x0002U) == 0x0002U){
	      status = UART_PARITY_ERROR;
	}
        else{
	      /* Do no thing */
        }
}
/******************************************************************************
End of file
******************************************************************************/
