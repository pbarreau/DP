/****************************************************************************/
/*!
 *\MODULE              JN-AN-1072 Jenie Wireless Keyboard
 *
 *\COMPONENT           $RCSfile: Keyboard.h,v $
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
 *\DESCRIPTION         Keyboard - public interface.
 *
 * This file handles the interface with the keyboard.
 *
 * vKeyboard_Init() is first called to initialise the keyboard data. This
 * includes calls to u16PS2socketInit() to initialise the PS2 socket hardware.
 *
 * vKeyboard_Main() is called repeatedly. If the application has timed out
 * waiting for an acknowledgement for a previous transmission that transmission
 * is resent. Otherwise a call is made to u16Keyboard_GetKey() to get the next
 * key sequence and if one is available it is transmitted.
 *
 * vKeyboard_Tick() is called every 100ms the timer for acks is updated here.
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
 * $Log: Keyboard.h,v $
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

#ifndef  KEYBOARD_H_INCLUDED
#define  KEYBOARD_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <jenie.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Function Prototypes                                  ***/
/****************************************************************************/
PUBLIC void vKeyboard_Init	(void);
PUBLIC void vKeyboard_Main	(void);
PUBLIC void vKeyboard_Rx	(uint16, uint8 *);
PUBLIC void vKeyboard_Tick	(void);

#if defined __cplusplus
}
#endif

#endif  /* KEYBOARD_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/


