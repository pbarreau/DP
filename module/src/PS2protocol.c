/****************************************************************************/
/*!
 *\MODULE              JN-AN-1072 Jenie Wireless Keyboard
 *
 *\COMPONENT           $RCSfile: PS2protocol.c,v $
 *
 *\VERSION             $Name: HEAD $
 *
 *\REVISION            $Revision: 1.1 $
 *
 *\DATED               $Date: 2007/11/14 10:18:23 $
 *
 *\STATUS              $State: Exp $
 *
 *\AUTHOR              APV Ward
 *
 *\DESCRIPTION         PS2 Protocol - implementation.
 *
 * This file contains the code that provides the controller board with a simple state machine to
 * decode incoming PS2 multi-byte key sequences. Three-byte (nominal) and five-byte (extended)
 * make/break keycode sequences are translated into single-byte ASCII printing codes.
 * Non-printable characters are ignored, but some control keys are supported, (such as return
 * and linefeed keys), The shift keys and caps lock key, (LED D3 indicates caps lock status),
 * are also supported.
 */
/* CHANGE HISTORY
 *
 * $Log: PS2protocol.c,v $
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
#include <jendefs.h>
#include <LedControl.h>

#include "PS2protocol.h"
#include "PS2keyCodes.h"

/****************************************************************************/
/***        Local Defines                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/* pseudo state-machine variables */
bool_t   boBreakSeqStarted   = FALSE;
bool_t   boExtendSeqStarted  = FALSE;
bool_t   boShifted           = FALSE;
bool_t   boCapsLockOn        = FALSE;

/****************************************************************************/
/***        Local functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: u8PS2protocol
 *
 * DESCRIPTION:
 * In keyboard demo mode all printable key presses are displayed on the
 * coordinator's LCD - this function maps PS2 keycode sequences (3 to 5 bytes)
 * to single ASCII printable characters.
 *
 * PARAMETERS:      key code from PS2 bus
 *
 * RETURNS:         Either ASCII code or 0
 *
 ****************************************************************************/

PUBLIC uint8 u8PS2protocol(uint8 u8Key)
{
    uint8  u8Temp = 0;

	switch (u8Key)
	{
		case 0xF0:

			/* hanldes    BREAK sequences ( F0, xx) and  */
			/*         EXTENDED sequences ( E0, F0, xx). */
			boBreakSeqStarted  = TRUE;
			boExtendSeqStarted = FALSE;
			break;

		case 0xE0:
			boExtendSeqStarted = TRUE;
			break;

		case 0xE1: 	/* special extended seq, such as PAUSE key */
			break;	/* ignore for this demo */

		default: /* ordinary keycode */
		{
			if (boBreakSeqStarted) /* last code was a BREAK code */
			{
				boBreakSeqStarted = FALSE;

				/* we only look for a release sequence on the SHIFT keys */
				if ( (u8Key == KBD_L_SHIFT) || (u8Key == KBD_R_SHIFT) )
				{
					boShifted = FALSE; /* shift key released */
				}
			}
			else if (boExtendSeqStarted)
			{
				boExtendSeqStarted = FALSE; /* ignore extended BREAK codes */
			}
			else
			{
				if ( (u8Key == KBD_L_SHIFT) || (u8Key == KBD_R_SHIFT) )
				{
					boShifted = TRUE; /* shift key is down ;-(  */
				}
				else if (u8Key == KBD_CAPS_LOCK)
				{
					boCapsLockOn = ! boCapsLockOn; /* toggles state */
					vLedControl(2, boCapsLockOn);
				}
				else
				{
					/* Found a character key, so translate into ASCII.		*/
					/* Lookup table translates non-printing keys to 0x00	*/
					if (boShifted)
					{
						/* lookup in shifted key table */
						u8Key = au8shiftedAlphabet[u8Key];
					}
					else
					{
						u8Temp = u8Key;
						u8Key  = au8alphabet[u8Key];

						/* if CAPS LOCK on, then re-lookup keycode */
						if (boCapsLockOn && u8Key >= 'a' && u8Key <= 'z' )
							u8Key = au8shiftedAlphabet[u8Temp];
					}

					u8Temp = u8Key; /* return the keycode */
				}
			}
		}
	}

	return u8Temp;
}

