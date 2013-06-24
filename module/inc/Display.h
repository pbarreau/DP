/****************************************************************************/
/*!
 *\MODULE              JN-AN-1072 Jenie Wireless Keyboard
 *
 *\COMPONENT           $RCSfile: Display.h,v $
 *
 *\VERSION             $Name: HEAD $
 *
 *\REVISION            $Revision: 1.1 $
 *
 *\DATED               $Date: 2007/11/14 10:18:22 $
 *
 *\STATUS              $State: Exp $
 *
 *\AUTHOR              Martin Looker
 *
 *\DESCRIPTION         Display - public interface.
 *
 */
/* CHANGE HISTORY
 *
 * $Log: Display.h,v $
 * Revision 1.1  2007/11/14 10:18:22  mlook
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

#ifndef  DISPLAY_H_INCLUDED
#define  DISPLAY_H_INCLUDED

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
PUBLIC void vDisplay_Init	(void);
PUBLIC void vDisplay_Rx		(uint16, uint8 *);

#if defined __cplusplus
}
#endif

#endif  /* DISPLAY_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/


