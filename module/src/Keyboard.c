/****************************************************************************/
/*!
 *\MODULE              JN-AN-1072 Jenie Wireless Keyboard
 *
 *\COMPONENT           $RCSfile: Keyboard.c,v $
 *
 *\VERSION             $Name: HEAD $
 *
 *\REVISION            $Revision: 1.1 $
 *
 *\DATED               $Date: 2007/11/14 10:18:23 $
 *
 *\STATUS              $State: Exp $
 *
 *\AUTHOR              APV Ward, Martin Looker
 *
 *\DESCRIPTION         Keyboard - implementation.
 *
 * This file handles the interface with the keyboard.
 *
 * vKeyboard_Init() is first called to initialise the keyboard data. This
 * includes a call to u16PS2socketInit() to initialise the PS2 socket hardware.
 *
 * vKeyboard_Main() is called repeatedly. If the application has timed out
 * waiting for an acknowledgement for a previous transmission that transmission
 * is resent. Otherwise a call is made to u16Keyboard_GetKey() to get the next
 * key sequence and if one is available it is transmitted.
 *
 * vKeyboard_Tick() is called every 100ms, the timer for acks is updated here.
 *
 * vKeyboard_Rx() is called whenever data is received over the network, acks
 * for transmitted data are handled here.
 *
 * vKeyboard_Tx() is called to transmit the current key sequence.
 *
 * u16Keyboard_GetKey() makes use of functions in PS2socket.c to check for
 * and read a key sequence from the keyboard. This function enables the PS2 bus
 * only for the duration of the read forcing the keyboard to buffer keystrokes
 * internally until the key sequence transmission has been acked.
 */
/* CHANGE HISTORY
 *
 * $Log: Keyboard.c,v $
 * Revision 1.1  2007/11/14 10:18:23  mlook
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
#include "jendefs.h"
#include <AppHardwareApi.h>
#include "Utilities.h"
//#include "gdb.h"
#include "printf.h"

/* Wireless keyboard specific */
#include "Network.h"
#include "Keyboard.h"
#include "PS2socket.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define	KBD_NO_KEY_PRESS	0xffff /* No keypress return for GetKey function */

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE uint8 	u8Timer;
PRIVATE uint8 	u8Sequence;
PRIVATE uint8 	u8KeyCode;
PRIVATE bool_t	bPending;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
PRIVATE void 	 vKeyboard_Tx(void);
PRIVATE uint16 u16Keyboard_GetKey(void);

/****************************************************************************
 *
 * NAME vKeyboard_Init
 */
/*!
 *\DESCRIPTION Initialise keyboard data.
 *
 ****************************************************************************/
PUBLIC void vKeyboard_Init (void)
{
    /* Initialise the PS2 bus and disable the keyboard */
    (void) u16PS2socketInit();
    vPS2socketBusDisable();

	/* Initialise ack timer and pending flag */
	u8Timer = 0;
    bPending = FALSE;

    /* Initialise sequence number */
    u8Sequence = 0x80;	/* Sequence */
    u8KeyCode  = 0;
}

/****************************************************************************
 *
 * NAME: vKeyboard_Main
 */
/*!
 *\DESCRIPTION Main keyboard function.
 *
 * Called regularly by the task scheduler.
 * This is the application task, where the PS2 keyboard is scanned and
 * keycodes are sent to the coordinator keyboard display application.
 *
 ****************************************************************************/
PUBLIC void vKeyboard_Main(void)
{
	uint16 u16Key;

	/* Waiting for an ack ? */
	if (bPending)
	{
		/* Timed out ? */
		if (u8Timer == 0)
		{
			/* Re-transmit keycode */
			vKeyboard_Tx();
		}
	}
	/* Ready for a new keyboard scan ? */
	else
	{
		/* Look for a new keycode from PS2 port */
		u16Key = u16Keyboard_GetKey();

		if (u16Key != KBD_NO_KEY_PRESS)
		{
			/* Note keycode */
			u8KeyCode = (uint8) u16Key;
			/* Transmit keycode */
			vKeyboard_Tx();
		}
		else
		{
			/* This would be an appropriate place to enter low power mode.		 	*/
			/* If no keyboard activity has been seen for a while, then a wakeup		*/
			/* interrupt could be enabled on the PS2 clock line after which the		*/
			/* JN5121 could be put into sleep mode.									*/
		}
	}
}


/****************************************************************************
 *
 * NAME: vKeyboard_Tick
 */
/*!
 *\DESCRIPTION Called every 100ms to time the ack wait period.
 *
 ****************************************************************************/
PUBLIC void vKeyboard_Tick(void)
{
	if (u8Timer > 0) u8Timer--;
}

/****************************************************************************
 *
 * NAME: vKeyboard_Rx
 */
/*!
 *\DESCRIPTION Called when an ack is received in response to an earlier send data request.
 *
 * For the wireless keyboard application, this happens when the coordinator
 * has sent an application level ack in response to receiving a keycode message.
 *
 * This call represents the return half of a flow control mechanism that prevents
 * keyboard scans from occuring during stack interrupt activities.
 *
 * For a full PS2 keyboard implementation, the PS2 bus would be bi-directional
 * and control data would flow from, for example, a PC back to the PS2 keyboard.
 * Such control data might include keyboard repeat rates, or caps lock LED control.
 * This data would need to be handled here.
 ****************************************************************************/
PUBLIC void vKeyboard_Rx(uint16 u16Length, uint8 *pu8DataRx)
{
	#if NETWORK_DEBUG
    	if (bNetwork_UartUp()) vPrintf("\n    Clavier Rx(%c, %x, %x) {%x}\n\n", pu8DataRx[0], pu8DataRx[1], pu8DataRx[2], u8Sequence);
    #endif
	/* Is this an ack message ? */
	if (pu8DataRx[0] == 'A')
	{
		/* Does the sequence number match ? */
		if (pu8DataRx[1] == u8Sequence)
		{
			/* No longer pending */
			bPending = FALSE;
			/* Cancel timer */
			u8Timer = 0;
			/* Go to next sequnence */
			u8Sequence++;
		}
	}
}

/****************************************************************************
 *
 * NAME: vKeyboard_Tx
 */
/*!
 *\DESCRIPTION Transmit current keycode.
 ****************************************************************************/
PRIVATE void vKeyboard_Tx(void)
{
	uint8  au8Data[3];

	/* Are the network services up ? */
	if (bNetwork_Services_Up())
	{
		/* Build packet */
		au8Data[0] = 'K';
		au8Data[1] = u8Sequence;
		au8Data[2] = u8KeyCode;
		#if NETWORK_DEBUG
			if (bNetwork_UartUp()) vPrintf("\n    Clavier Tx(%c, %x, %x)\n\n", au8Data[0], au8Data[1], au8Data[2]);
		#endif
		/* Send packet */
		(void) eNetwork_Service_Tx(NETWORK_SERVICE, 3, au8Data);
		/* Note we are awaiting an ack now */
		bPending = TRUE;
		/* Restart timer approx 200ms */
		u8Timer = 2;
	}
}

/****************************************************************************
 *
 * NAME: u16Keyboard_GetKey
 */
/*!
 *\DESCRIPTION Get key sequence from keyboard.
 *
 * Reads keycodes from PS2 keyboard using the PS2 socket service.
 * Reads one PS2 code at a time, so this function would need to be called at
 * least three times to build a (typical) three-byte code sequence.
 * Between bytes the bus is disabled, thereby allowing control to return
 * to the ZigBee stack.
 *
 * Key sequence consists of:
 * 	- KEY PRESS code
 * 	- BREAK CODE (0xF0)
 * 	- KEY PRESS code (again)
 ****************************************************************************/
PRIVATE uint16 u16Keyboard_GetKey(void)
{
    uint16 u16kbdstatus = KBD_NO_KEY_PRESS;

    /* enable the PS2 bus */
    vPS2socketBusEnable();

	if ( boPS2socketBusReady() )  /* if keyboard is not asserting RTS */
	{
		u16kbdstatus = u16PS2socketRead();    /* read key codes */

		if (u16kbdstatus & PS2_SOCKET_ERROR_MASK)
		{
			/* either no key press or PS2 bus error, errors are ignored */
			u16kbdstatus = KBD_NO_KEY_PRESS;
		}
		else
		{
			/* good PS2 scan, keycode is in lower byte */
			u16kbdstatus &= PS2_SOCKET_KEYCODE_MASK;
		}
	}

    /* inhibit the  PS2 bus */
    vPS2socketBusDisable();

    return u16kbdstatus;
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
