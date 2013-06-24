/****************************************************************************/
/*!
 *\MODULE              JN-AN-1072 Jenie Wireless Keyboard
 *
 *\COMPONENT           $RCSfile: PS2keyCodes.h,v $
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
 *\DESCRIPTION         PS2 Key Codes - public interface.
 */
/* CHANGE HISTORY
 *
 * $Log: PS2keyCodes.h,v $
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
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define	KBD_CNTRL_CODE_BASE	0x80

#define	KBD_TAB		KBD_CNTRL_CODE_BASE
#define	KBD_SPACE	KBD_CNTRL_CODE_BASE + 1
#define	KBD_ENTER	KBD_CNTRL_CODE_BASE + 2
#define	KBD_END		KBD_CNTRL_CODE_BASE + 3
#define	KBD_L_ARROW	KBD_CNTRL_CODE_BASE + 4
#define	KBD_HOME	KBD_CNTRL_CODE_BASE + 5
#define	KBD_DELETE	KBD_CNTRL_CODE_BASE + 6
#define	KB_D_ARROW	KBD_CNTRL_CODE_BASE + 7
#define	KBD_R_ARROW	KBD_CNTRL_CODE_BASE + 8
#define	KBD_U_ARROW	KBD_CNTRL_CODE_BASE + 9
#define	KBD_PG_DN	KBD_CNTRL_CODE_BASE + 10
#define	KBD_PG_UP	KBD_CNTRL_CODE_BASE + 11
#define	KBD_BKSP	KBD_CNTRL_CODE_BASE + 12


/* special keys */

#define	KBD_L_SHIFT		0x12
#define	KBD_R_SHIFT		0x59
#define	KBD_CAPS_LOCK	0x58

#define	KBD_CHR_SIZE	240


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC const uint8 au8alphabet       [KBD_CHR_SIZE];
PUBLIC const uint8 au8shiftedAlphabet[KBD_CHR_SIZE];
