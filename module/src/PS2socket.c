/****************************************************************************/
/*!
 *\MODULE              JN-AN-1072 Jenie Wireless Keyboard
 *
 *\COMPONENT           $RCSfile: PS2socket.c,v $
 *
 *\VERSION             $Name: HEAD $
 *
 *\REVISION            $Revision: 1.2 $
 *
 *\DATED               $Date: 2007/11/22 08:33:21 $
 *
 *\STATUS              $State: Exp $
 *
 *\AUTHOR              APV Ward
 *
 *\DESCRIPTION         PS2 Socket - implementation.
 *
 * This file contains the code that provides a full PS2 socket service.
 * It also supports the application's need to read from the PS2 bus,
 * as well as enableing/disabling the bus. PS2 is a bi-directional bus, and
 * this file provides both bus read and bus write functions - however, the
 * keyboard application only uses the bus read function.
 *
 * This file also defines which DIO pins on the J4 connector are used to
 * connect to the PS2 bus.
 */
/* CHANGE HISTORY
 *
 * $Log: PS2socket.c,v $
 * Revision 1.2  2007/11/22 08:33:21  mlook
 * JPI updates
 * Project and makefile renaming
 *
 * Revision 1.1  2007/11/14 10:18:24  mlook
 * Initial checkin
 *
 *
 *
 * LAST MODIFIED BY    $Author: mlook $
 *                     $Modtime: $
 *
 ****************************************************************************
 *
 * This software is owned by Jennic and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on Jennic products. You, and any third parties must reproduce
 * the copyright and warranty notice and any other legend of ownership on each
 * copy or partial copy of the software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS". JENNIC MAKES NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * ACCURACY OR LACK OF NEGLIGENCE. JENNIC SHALL NOT, IN ANY CIRCUMSTANCES,
 * BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT LIMITED TO, SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON WHATSOEVER.
 *
 * Copyright Jennic Ltd 2005, 2006, 2007. All rights reserved
 *
 ****************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <jendefs.h>
#include <JPI.h>
#include <AppHardwareApi.h>

#include "PS2socket.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* External triger DIO18 - for debug scope 	*/
/* Clock:	       DIO19					*/
/* Data:	       DIO20					*/

#define	DEBUG_EXT_TRIG			((uint32) (1 << 18))
#define	PS2_DIO_CLOCK			((uint32) (1 << 19))
#define	PS2_DIO_DATA			((uint32) (1 << 20))

/* Clock and data-line control macros: easier to read */

#define vSET_CLOCK_HIGH	vJPI_DioSetDirection( PS2_DIO_CLOCK, (uint32) 0    )
#define vSET_CLOCK_LOW	vJPI_DioSetDirection( (uint32) 0,    PS2_DIO_CLOCK )

#define vSET_DATA_HIGH	vJPI_DioSetDirection( PS2_DIO_DATA,  (uint32) 0    )
#define vSET_DATA_LOW	vJPI_DioSetDirection( (uint32) 0,    PS2_DIO_DATA  )

#define	boCLOCK_IS_HIGH	  (u32JPI_DioReadInput() & PS2_DIO_CLOCK)
#define	boCLOCK_IS_LOW	(!(u32JPI_DioReadInput() & PS2_DIO_CLOCK))

#define	boDATA_IS_HIGH	  (u32JPI_DioReadInput() & PS2_DIO_DATA)
#define	boDATA_IS_LOW	(!(u32JPI_DioReadInput() & PS2_DIO_DATA))

/* for debug scope */
#define vSET_TRIG_HIGH	vJPI_DioSetDirection( DEBUG_EXT_TRIG, (uint32) 0    )
#define vSET_TRIG_LOW	vJPI_DioSetDirection( (uint32) 0,    DEBUG_EXT_TRIG )

/* Timers: using macro overloading, so that timer choice can easily be changed */
#define	PS2_SOCKET_TIMER		E_JPI_TIMER_1
#define	PS2_DATA_WIDTH			8

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PRIVATE uint16 u16FindStartBit			(void);
PRIVATE uint16 u16FindDataBit			(void);
PRIVATE uint16 u16FindParityBit			(uint8);
PRIVATE uint16 u16FindStopBit			(void);
PRIVATE uint16 u16FindClkFallingEdge	(void);
PRIVATE void   vIdle                    (void);

/* Write byte functions */
PRIVATE void   vWait100us				(void);
PRIVATE bool_t boDeviceClkStarted		(void);
PRIVATE bool_t boDataBitClocked			(void);
PRIVATE bool_t boDataWentLow			(void);
PRIVATE bool_t boClockWentLow			(void);
PRIVATE bool_t boDataClkReleased		(void);

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


/****************************************************************************
 *
 * NAME: u16PS2socketInit
 */
/*!
 *\DESCRIPTION Initialisation.
 *
 * Initialises the PS2 interface into a known state by setting the PS2
 * bus to idle.
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		PS2_STATUS_SUCCESS
 *
 ****************************************************************************/

PUBLIC uint16 u16PS2socketInit(void)
{
	/* set debug scope trigger as output and low */
	vJPI_DioSetOutput   ( (uint32) 0, DEBUG_EXT_TRIG);
	vJPI_DioSetDirection( (uint32) 0, DEBUG_EXT_TRIG);

	vIdle();
	return PS2_STATUS_SUCCESS;
}

/****************************************************************************
 *
 * NAME: u16PS2socketClose
 */
/*!
 *\DESCRIPTION Closes the PS2 interface by setting the PS2 bus to idle.
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		PS2_STATUS_SUCCESS
 *
 ****************************************************************************/

PUBLIC void vPS2socketClose(void)
{
	vIdle();
}

/****************************************************************************
 *
 * NAME: vPS2socketBusEnable
 */
/*!
 *\DESCRIPTION Enable the PS2 bus by releasing the open-collector clock line.
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		Void
 *
 ****************************************************************************/

PUBLIC void vPS2socketBusEnable(void)
{
	/* set clock and data lines as inputs */
	vJPI_DioSetDirection( (PS2_DIO_DATA | PS2_DIO_CLOCK), (uint32) 0);
}

/****************************************************************************
 *
 * NAME: vPS2socketBusDisable
 */
/*!
 *\DESCRIPTION Disable the PS2 bus by pulling the open-collector clock line low.
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		Void
 *
 ****************************************************************************/

PUBLIC void vPS2socketBusDisable(void)
{
	vIdle();
}

/****************************************************************************
 *
 * NAME: boPS2socketBusReady
 */
/*!
 *\DESCRIPTION Returns the current status of the PS2 clock and data lines.
 * Both lines low indicates bus active - also consistant with a start bit.
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		TRUE if both clock and data lines are low.
 *
 ****************************************************************************/

PUBLIC bool_t boPS2socketBusReady(void)
{
	/* if clock and data lines are low, then PS2 data burst has started */
	return (bool_t) ( ! (u32JPI_DioReadInput() & (PS2_DIO_CLOCK | PS2_DIO_DATA)) );
}

/****************************************************************************
 *
 * NAME: boPS2socketBusIdle
 */
/*!
 *\DESCRIPTION Returns the current status of the PS2 clock and data lines.
 * Both lines high indicates bus idle.
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		TRUE if both clock and data lines are high (bus idle).
 *
 ****************************************************************************/

PUBLIC bool_t boPS2socketBusIdle(void)
{
	/* if BOTH clock and data lines are high, then PS2 bus is idle */
	return	(bool_t)( (u32JPI_DioReadInput() & (PS2_DIO_CLOCK | PS2_DIO_DATA))
	  	                                    == (PS2_DIO_CLOCK | PS2_DIO_DATA)
		            );
}

/****************************************************************************
 *
 * NAME: boPS2socketBusStarted
 */
/*!
 *\DESCRIPTION Returns the current status of the PS2 clock and data lines.
 * Both lines low indicates bus active - also consistant with a start bit.
 * Scan lines for at least 100 us, return earlier if bus cycle starts.
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		TRUE if both clock and data lines are low.
 *
 ****************************************************************************/

PUBLIC bool_t boPS2socketBusStarted(void)
{
	/* Start timer */
	vJPI_TimerStart(PS2_SOCKET_TIMER,
					E_JPI_TIMER_MODE_SINGLESHOT,
	                100,
	                100);

	while ( ! (u8JPI_TimerFired(PS2_SOCKET_TIMER) & E_JPI_TIMER_INT_PERIOD) )
	{
		/* if clock and data lines are low, then PS2 data burst has started */
		if( ! (u32JPI_DioReadInput() & (PS2_DIO_CLOCK | PS2_DIO_DATA)) )
		{
			vJPI_TimerStop(PS2_SOCKET_TIMER);
			return TRUE;
		}
	}
	return FALSE;
}

/****************************************************************************
 *
 * NAME: u16PS2socketRead
 */
/*!
 *\DESCRIPTION Read a data burst from the PS2 device.
 *
 * A full description of the PS2 bus protocol would not be appropriate here.
 * However, here is a simple overview of a read sequence:
 *
 * PS2 device drives the clock line, unless the line is already low(device disabled).
 * Falling edge of the clock line strobes valid date.
 * Data burst starts with an active low start bit,
 * followed by 8 data bits (LSB first)
 * followed by an ODD parity bit,
 * followed by an active high stop bit.
 *
 * At any stage a data burst can be aborted if the open-collector clock line
 * is pulled low for more than 100 uS.  If this happens before the 11th data bit
 * period has completed, then the data burst must be sent again (after the clock
 * line has been released for more than 50 uS).
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		uint16 - PS2 code (8 bits), or error code (16 bits)
 *
 ****************************************************************************/

PUBLIC uint16 u16PS2socketRead(void)
{
	uint8	u8Loop;
	uint8	u8Read = 0;
	uint8	u8ParityCount = 0;
	uint16	u16Status;

	/* enter here after detecting a falling edge on PS2 clock line */

	/* check for the start bit from the bus, abort if not present */
	u16Status = u16FindStartBit();
	if (u16Status != PS2_STATUS_START_BIT)
		return u16Status;

	/*	Now look for the data part of the transmission */
	for (u8Loop = 0; u8Loop < PS2_DATA_WIDTH; u8Loop++)
	{
		u16Status = u16FindDataBit(); /* look for single clocked-data bit */

		switch (u16Status)
		{
			case PS2_STATUS_DATA_1_BIT:
				u8Read |= (0x01 << u8Loop);
				u8ParityCount++;
				break;

			case PS2_STATUS_DATA_0_BIT:
				/* bit accumulator already 0 */
				break;

			case PS2_ERROR_DEVICE_CLK_DATA_TO: /* timeout */
				return u16Status;
				break;

			default:
				/* Error condition catch-all */
				return PS2_ERROR_UNKNOWN;
				break;
		}
	}

	/* look for the parity bit, abort if not correct */
	u16Status = u16FindParityBit(u8ParityCount);
	if (u16Status != PS2_STATUS_SUCCESS)
		return u16Status;

	/* look for a stop bit, abort if not present */
	u16Status = u16FindStopBit();
	if (u16Status == PS2_ERROR_NO_STOP_BIT)
		return u16Status;

	/* successful read from the PS2 bus, return code */
	return (uint16) u8Read;
}


/****************************************************************************
 *
 * NAME: u16PS2socketWrite
 */
/*!
 *\DESCRIPTION PS2 bus write sequence.
 * The PS2 bus is bidirectional, for a PS2 keyboard
 * the back channel carries config and status commands (auto-repeat rates,
 * shift, caps lock LED control etc).
 *
 * This function NOT used in this keyboard demo application.
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		PS2_STATUS_SUCCESS if write successful, else these error coes
 *				PS2_ERROR_DEVICE_CLK_START_TO
 *				PS2_ERROR_DEVICE_CLK_DATA_TO
 *				PS2_ERROR_DEVICE_CLK_PARY_TO
 *				PS2_ERROR_DEVICE_ACK_0
 *				PS2_ERROR_DEVICE_ACK_1
 *				PS2_ERROR_DEVICE_ACK_2
 *
 ****************************************************************************/

PUBLIC uint16 u16PS2socketWrite(uint8 u8Datum)
{
	uint8 u8Loop;
	uint8 u8ParityCount = 0;

	/* DEBUG */
	vSET_TRIG_HIGH;

	// 1)   Bring the Clock line low for at least 100 microseconds.
	//      Both output latches low, but data is an input
	vIdle();
	vWait100us();

	// 2)   Bring the Data line low.
	vSET_DATA_LOW;

	// 3)   Release the Clock line.
	vSET_CLOCK_HIGH;

	// 4)   Wait for the device to bring the Clock line low.
	if ( ! boDeviceClkStarted() )
		return PS2_ERROR_DEVICE_CLK_START_TO;

	/* send data byte */
	for (u8Loop = 0; u8Loop < PS2_DATA_WIDTH; u8Loop++)
	{
		// 5)   Set/reset the Data line to send the first data bit
		if (u8Datum & (0x01 << u8Loop))
		{
			vSET_DATA_HIGH;
			u8ParityCount++;
		}
		else
		{
			vSET_DATA_LOW;
		}

		// 6)   Wait for the device to bring Clock high.
		// 7)   Wait for the device to bring Clock low.
		if ( ! boDataBitClocked() )
			return PS2_ERROR_DEVICE_CLK_DATA_TO;
	}

	// 8)   Set the parity bit
	if (u8ParityCount & 0x01)
	{
		/* already ODD parity count, so DATA line LOW */
		vSET_DATA_LOW;
	}
	else
	{
		/* parity even, add a parity (HIGH) bit */
		vSET_DATA_HIGH;
	}
	if ( ! boDataBitClocked() )
		return PS2_ERROR_DEVICE_CLK_PARY_TO;

	// 9)   Release the Data line by setting as input
	vSET_DATA_HIGH;

	// 10) Wait for the device to bring Data low.
	if ( ! boDataWentLow() )
		return PS2_ERROR_DEVICE_ACK_1;

	// 11) Wait for the device to bring Clock  low.
	if ( ! boClockWentLow() )
		return PS2_ERROR_DEVICE_ACK_2;

	// 12) Wait for the device to release Data and Clock
	if ( ! boDataClkReleased() )
		return PS2_ERROR_DEVICE_ACK_3;

	return PS2_STATUS_SUCCESS;
}

/****************************************************************************
 *
 * NAME: u16FindStartBit
 */
/*!
 *\DESCRIPTION Non-blocking scan of the bus, looking for a clocked low data-bit
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		PS2_STATUS_START_BIT    if seen a start bit, else
 *				PS2_ERROR_NO_START_BIT
 *
 ****************************************************************************/

PRIVATE uint16 u16FindStartBit(void)
{
	/* clock has already gone low, valid start bit should be ready */

	if (boDATA_IS_LOW)
		return PS2_STATUS_START_BIT;
	else
		return PS2_ERROR_NO_START_BIT;
}

/****************************************************************************
 *
 * NAME: u16FindDataBit
 */
/*!
 *\DESCRIPTION Blocking scan of the bus, looking for a clocked data bit
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		PS2_STATUS_DATA_1_BIT    if seen a high data bit, else
 *				PS2_STATUS_DATA_0_BIT
 *
 ****************************************************************************/

PRIVATE uint16 u16FindDataBit(void)
{
	/* To get here we must have seen a clock falling-edge, so wait for */
	/* a rising edge */

	if (u16FindClkFallingEdge() == PS2_ERROR_DEVICE_CLK_EDGE_TO)
		return PS2_ERROR_DEVICE_CLK_DATA_TO;

	/* clock cycle (lo, hi, lo) has occurred, so read next data bit */

	if (boDATA_IS_HIGH)
		return PS2_STATUS_DATA_1_BIT;
	else
		return PS2_STATUS_DATA_0_BIT;
}

/****************************************************************************
 *
 * NAME: u16FindParityBit
 */
/*!
 *\DESCRIPTION Blocking call - looks for a parity bit then checks for parity error.
 */
/* PARAMETERS: 	uint8 - number of high data bits in received 8 bit data word.
 *
 * RETURNS:		PS2_STATUS_SUCCESS    if ODD parity found, else
 *				PS2_ERROR_PARITY
 *
 ****************************************************************************/

PRIVATE uint16 u16FindParityBit(uint8 u8count)
{
	/* just seen last data bit, wait for a clock cycle */
	if (u16FindClkFallingEdge() == PS2_ERROR_DEVICE_CLK_EDGE_TO)
		return PS2_ERROR_DEVICE_CLK_PARY_TO;

	/* now read parity bit, if set add to 'set bits' counter */
	if (boDATA_IS_HIGH)
		u8count++;

	if (u8count & 0x01)
		return PS2_STATUS_SUCCESS;
	else
		return PS2_ERROR_PARITY;
}

/****************************************************************************
 *
 * NAME: u16FindStopBit
 */
/*!
 *\DESCRIPTION Blocking call - looks for a clocked HIGH (stop) bit.
 */
/* PARAMETERS: 	None
 *
 * RETURNS:		PS2_STATUS_SUCCESS    if stop bit found, else
 *				PS2_ERROR_NO_STOP_BIT
 *
 ****************************************************************************/

PRIVATE uint16 u16FindStopBit(void)
{
	if (u16FindClkFallingEdge() == PS2_ERROR_DEVICE_CLK_EDGE_TO)
		return PS2_ERROR_DEVICE_CLK_STOP_TO;

	/* look for STOP bit, a HIGH data line	*/
	if (boDATA_IS_HIGH)
		return PS2_STATUS_SUCCESS;
	else
		return PS2_ERROR_NO_STOP_BIT;
}

/****************************************************************************
 *
 * NAME: u16FindClkFallingEdge
 */
/*!
 *\DESCRIPTION Blocking call - looks for a LOW-HIGH-LOW clock transition.
 * Uses a simple timeout counter to prevent function from hanging if no clock.
 * This mechanism is dependent upon processor clock (fixed for the JN5121),
 * but the scan period is too chort to use a sleep timer.
 */
/* PARAMETERS: 	None
 *
 * RETURNS:		PS2_STATUS_SUCCESS
 *			or  PS2_ERROR_DEVICE_CLK_EDGE_TO - if no clock transition.
 *
 ****************************************************************************/

PRIVATE uint16 u16FindClkFallingEdge(void)
{
	/* APVW use 16 meg timers */

	/* simple timeout counter to avoid hanging on no clock edge */
	uint16 u16TimeOut = 0;

	/* clock must be low now, so wait for a rising edge */
	while ( boCLOCK_IS_LOW && (++u16TimeOut) )
		;

	/* If timeout counter wrapped, then clock timeout error has occurred */
	if (u16TimeOut == 0)
		return PS2_ERROR_DEVICE_CLK_EDGE_TO;

	/* now wait for a falling edge and hence valid data */
	u16TimeOut = 0;
	while ( boCLOCK_IS_HIGH && (++u16TimeOut) )
		;

	/* If timeout counter wrapped, then clock timeout error has occurred */
	if (u16TimeOut == 0)
		return PS2_ERROR_DEVICE_CLK_EDGE_TO;

	/* successful clock LOW-HIGH-LOW transition */
	return PS2_STATUS_SUCCESS;
}

/****************************************************************************
 *
 * NAME: vIdle
 */
/*!
 *\DESCRIPTION Disable the PS2 bus by pulling the open-collector clock line low.
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		Void
 *
 ****************************************************************************/

PRIVATE void vIdle(void)
{
	/* set output latch to LOW for all lines */
	vJPI_DioSetOutput( (uint32) 0, PS2_DIO_CLOCK | PS2_DIO_DATA);

	/* input pins (data) will pull HIGH, output pins (clock) are driven LOW */
	vJPI_DioSetDirection( PS2_DIO_DATA, PS2_DIO_CLOCK);
}

/****************************************************************************
 *
 * NAME: vWait100us
 */
/*!
 *\DESCRIPTION Blocking wait for 100 us.
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		Void
 *
 ****************************************************************************/

PRIVATE void vWait100us(void)
{
	vJPI_TimerStart(PS2_SOCKET_TIMER,
					E_JPI_TIMER_MODE_SINGLESHOT,
	                100,
	                100);

	while ( ! (u8JPI_TimerFired(PS2_SOCKET_TIMER) & E_JPI_TIMER_INT_PERIOD) )
		;
}

/****************************************************************************
 *
 * NAME: boDeviceClkStarted
 */
/*!
 *\DESCRIPTION Look for start of data transfer sequence.
 * Look for the PS2 device (clock master) to pull clock low, thereby starting
 * PS2 data transfer sequence.
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		TRUE if clock negative edge seen, else FALSE
 *
 ****************************************************************************/

PRIVATE bool_t boDeviceClkStarted(void)
{
	/* allow PS2 device upto 10 ms to bring clock low */
	vJPI_TimerStart(PS2_SOCKET_TIMER,
					E_JPI_TIMER_MODE_SINGLESHOT,
	                10000,
	                10000);

	while ( ! (u8JPI_TimerFired(PS2_SOCKET_TIMER) & E_JPI_TIMER_INT_PERIOD) )
	{
		/* if clock lines goes low, then device is ready to read a burst */
		if( boCLOCK_IS_LOW )
		{
			vJPI_TimerStop(PS2_SOCKET_TIMER);
			return TRUE;
		}
	}
	return FALSE;
}


/****************************************************************************
 *
 * NAME: boDataBitClocked
 */
/*!
 *\DESCRIPTION Looks for a low-high-low sequence on the PS2 clock line.
 * Waits for 100us to
 * allow upto two clock cycles at the lowest PS2 clock frequency of 10 kHz.
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		TRUE if clock sequence seen, else FALSE
 *
 ****************************************************************************/

PRIVATE bool_t boDataBitClocked(void)
{
	vJPI_TimerStart(PS2_SOCKET_TIMER,
					E_JPI_TIMER_MODE_SINGLESHOT,
	                100,
	                100);

	/* Wait for the device to bring Clock high */
	while ( (boCLOCK_IS_LOW)
	         && ! (u8JPI_TimerFired(PS2_SOCKET_TIMER) & E_JPI_TIMER_INT_PERIOD) )
	  	;

	/* if clock is still low then must have timed-out, error: no clock */
	if (boCLOCK_IS_LOW)
	return FALSE;

	/* Wait for the device to bring Clock low */
	vJPI_TimerStop(PS2_SOCKET_TIMER); // APVW is this necessary ?
	vJPI_TimerStart(PS2_SOCKET_TIMER,
					E_JPI_TIMER_MODE_SINGLESHOT,
	                100,
	                100);

	/* while still high, wait */
	while ( (boCLOCK_IS_HIGH)
	         && ! (u8JPI_TimerFired(PS2_SOCKET_TIMER) & E_JPI_TIMER_INT_PERIOD) )
	  	;

	vJPI_TimerStop(PS2_SOCKET_TIMER); // APVW is this necessary ?

	if (boCLOCK_IS_LOW)
		return TRUE;
	else
		return FALSE;
}

/****************************************************************************
 *
 * NAME: boDataWentLow
 */
/*!
 *\DESCRIPTION Allow 100us for PS2 device (clock master) to take data line low.
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		TRUE if data line negative edge seen, else FALSE
 *
 ****************************************************************************/

PRIVATE bool_t boDataWentLow(void)
{
	/* Maximum wait for the device to bring Data low */
	vJPI_TimerStart(PS2_SOCKET_TIMER,
					E_JPI_TIMER_MODE_SINGLESHOT,
	                100,
	                100);

	/* while data line still high keep looking, with timeout */
	while ( (boDATA_IS_HIGH)
	         && ! (u8JPI_TimerFired(PS2_SOCKET_TIMER) & E_JPI_TIMER_INT_PERIOD) )
	  	;

	vJPI_TimerStop(PS2_SOCKET_TIMER);

	if (boDATA_IS_LOW)
		return TRUE;
	else
		return FALSE;
}

/****************************************************************************
 *
 * NAME: boClockWentLow
 */
/*!
 *\DESCRIPTION Allow 100us for PS2 device (clock master) to take clock line low.
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		TRUE if clock negative edge seen, else FALSE
 *
 ****************************************************************************/

PRIVATE bool_t boClockWentLow(void)
{
	/* Wait for the device to bring Clock low */
	vJPI_TimerStart(PS2_SOCKET_TIMER,
					E_JPI_TIMER_MODE_SINGLESHOT,
	                100,
	                100);

	/* while clock line still high keep looking, with timeout */
	while ( (boCLOCK_IS_HIGH)
	         && ! (u8JPI_TimerFired(PS2_SOCKET_TIMER) & E_JPI_TIMER_INT_PERIOD) )
	  	;

	vJPI_TimerStop(PS2_SOCKET_TIMER);

	if (boCLOCK_IS_LOW)
		return TRUE;
	else
		return FALSE;
}

/****************************************************************************
 *
 * NAME: boDataClkReleased
 */
/*!
 *\DESCRIPTION Allow 100us for PS2 device (clock master) to take clock and data line high.
 */
/* PARAMETERS: 	None.
 *
 * RETURNS:		TRUE if clock and data positive edge seen, else FALSE
 *
 ****************************************************************************/

PRIVATE bool_t boDataClkReleased(void)
{
	/* Wait for the device to release (go high) clock and data lines  */
	vJPI_TimerStart(PS2_SOCKET_TIMER,
					E_JPI_TIMER_MODE_SINGLESHOT,
	                100,
	                100);

	/* while clock or data lines still high keep looking, with timeout */
	while ( (boCLOCK_IS_LOW || boDATA_IS_LOW)
	         && ! (u8JPI_TimerFired(PS2_SOCKET_TIMER) & E_JPI_TIMER_INT_PERIOD) )
	  	;

	vJPI_TimerStop(PS2_SOCKET_TIMER);

	if (boCLOCK_IS_HIGH && boDATA_IS_HIGH)
		return TRUE;
	else
		return FALSE;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
