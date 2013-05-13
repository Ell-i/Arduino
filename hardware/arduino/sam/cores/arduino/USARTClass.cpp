/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "USARTClass.h"

// Constructors ////////////////////////////////////////////////////////////////

extern "C" { volatile int __init_magic = 0; }

USARTClass::USARTClass( Usart* pUsart, IRQn_Type dwIrq, uint32_t dwId, RingBuffer* pRx_buffer )
{
  _rx_buffer = pRx_buffer ;

  _pUsart=pUsart ;
  _dwIrq=dwIrq ;
  _dwId=dwId ;

  __init_magic = 1;
}

// Public Methods //////////////////////////////////////////////////////////////

void USARTClass::begin( const uint32_t dwBaudRate )
{
#if defined(__SAM3X8E__)
  // Configure PMC
  pmc_enable_periph_clk( _dwId ) ;

  // Disable PDC channel
  _pUsart->US_PTCR = US_PTCR_RXTDIS | US_PTCR_TXTDIS ;

  // Reset and disable receiver and transmitter
  _pUsart->US_CR = US_CR_RSTRX | US_CR_RSTTX | US_CR_RXDIS | US_CR_TXDIS ;

  // Configure mode
  _pUsart->US_MR = US_MR_USART_MODE_NORMAL | US_MR_USCLKS_MCK | US_MR_CHRL_8_BIT | US_MR_PAR_NO |
                   US_MR_NBSTOP_1_BIT | US_MR_CHMODE_NORMAL;

  // Configure baudrate, asynchronous no oversampling
  _pUsart->US_BRGR = (SystemCoreClock / dwBaudRate) / 16 ;

  // Configure interrupts
  _pUsart->US_IDR = 0xFFFFFFFF;
  _pUsart->US_IER = US_IER_RXRDY | US_IER_OVRE | US_IER_FRAME;

  // Enable UART interrupt in NVIC
  NVIC_EnableIRQ( _dwIrq ) ;

  // Enable receiver and transmitter
  _pUsart->US_CR = US_CR_RXEN | US_CR_TXEN ;
#endif
#if defined(__STM32F051__)
  /* Change the GPIO pins PA9 and PA10 to the USART mode */
  /* XXX: Works only for Serial for now, not for Serial1 */
  GPIOA->AFR[1] &= ~(GPIO_AFRH_AFRH1     | GPIO_AFRH_AFRH2    );
  GPIOA->AFR[1] |=  (GPIO_AFRx_AFRx1_AF1 | GPIO_AFRx_AFRx2_AF1);
  GPIOA->MODER  &= ~(GPIO_MODER_MODER9_0  | GPIO_MODER_MODER10_0);
  GPIOA->MODER  |=  (GPIO_MODER_MODER9_1  | GPIO_MODER_MODER10_1);

  /* Set the baud rate -- use 16 bit oversampling */
  _pUsart->BRR  = SystemCoreClock / dwBaudRate;

#if 1
  if (_pUsart == USART1) {
      GPIOC->ODR   |=  GPIO_ODR_6;
  }
#endif

  /* Enable the transmitter and the USART */
  _pUsart->CR1 |= USART_CR1_TE | USART_CR1_UE;
#endif
}

void USARTClass::end( void )
{
  // clear any received data
  _rx_buffer->_iHead = _rx_buffer->_iTail ;

#if defined(__SAM3X8E__)
  // Disable UART interrupt in NVIC
  NVIC_DisableIRQ( _dwIrq ) ;

  // Wait for any outstanding data to be sent
  flush();

  pmc_disable_periph_clk( _dwId ) ;
#endif
}

int USARTClass::available( void )
{
  return (uint32_t)(SERIAL_BUFFER_SIZE + _rx_buffer->_iHead - _rx_buffer->_iTail) % SERIAL_BUFFER_SIZE ;
}

int USARTClass::peek( void )
{
  if ( _rx_buffer->_iHead == _rx_buffer->_iTail )
    return -1 ;

  return _rx_buffer->_aucBuffer[_rx_buffer->_iTail] ;
}

int USARTClass::read( void )
{
  // if the head isn't ahead of the tail, we don't have any characters
  if ( _rx_buffer->_iHead == _rx_buffer->_iTail )
    return -1 ;

  uint8_t uc = _rx_buffer->_aucBuffer[_rx_buffer->_iTail] ;
  _rx_buffer->_iTail = (unsigned int)(_rx_buffer->_iTail + 1) % SERIAL_BUFFER_SIZE ;
  return uc ;
}

void USARTClass::flush( void )
{
#if defined(__SAM3X8E__)
  // Wait for transmission to complete
  while ((_pUsart->US_CSR & US_CSR_TXRDY) != US_CSR_TXRDY)
	;
#endif
}

size_t USARTClass::write( const uint8_t uc_data )
{
#if defined(__SAM3X8E__)
  // Check if the transmitter is ready
  while ((_pUsart->US_CSR & US_CSR_TXRDY) != US_CSR_TXRDY)
    ;

  // Send character
  _pUsart->US_THR = uc_data ;
#endif
#if defined(__STM32F051__)
#endif
  /* Send the character */
  _pUsart->TDR = uc_data;

  /* Wait until the data has been consumed */
  while ((_pUsart->ISR & USART_ISR_TXE) == 0) {
      GPIOC->ODR ^= GPIO_ODR_9;
      /* XXX yield() */;
  }

  return 1;
}

void USARTClass::IrqHandler( void )
{
#if defined(__SAM3X8E__)
  uint32_t status = _pUsart->US_CSR;

  // Did we receive data ?
  if ((status & US_CSR_RXRDY) == US_CSR_RXRDY)
    _rx_buffer->store_char( _pUsart->US_RHR ) ;

  // Acknowledge errors
  if ((status & US_CSR_OVRE) == US_CSR_OVRE ||
		  (status & US_CSR_FRAME) == US_CSR_FRAME)
  {
	// TODO: error reporting outside ISR
    _pUsart->US_CR |= US_CR_RSTSTA;
  }
#endif
}

