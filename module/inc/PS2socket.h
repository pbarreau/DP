/****************************************************************************/
/*!
 *\MODULE              JN-AN-1072 Jenie Wireless Keyboard
 *
 *\COMPONENT           $RCSfile: PS2socket.h,v $
 *
 *\VERSION             $Name: HEAD $
 *
 *\REVISION            $Revision: 1.1 $
 *
 *\DATED               $Date: 2007/11/14 10:18:24 $
 *
 *\STATUS              $State: Exp $
 *
 *\AUTHOR              APV Ward
 *
 *\DESCRIPTION         PS2 Socket - public interface.
 */
/* CHANGE HISTORY
 *
 * $Log: PS2socket.h,v $
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
/***        Include Files                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/* Status returns */
/* For ease of debugging these are incremental codes - something that's		*/
/* easy to interpret from a debug serial port.								*/

#define	PS2_STATUS						0x4000
#define	PS2_STATUS_SUCCESS				(PS2_STATUS) + 1
#define	PS2_STATUS_START_BIT			(PS2_STATUS) + 2

#define	PS2_STATUS_DATA_0_BIT			(PS2_STATUS) + 3
#define	PS2_STATUS_DATA_1_BIT			(PS2_STATUS) + 4

/* Error returns */
#define	PS2_ERROR						0x8000
#define	PS2_ERROR_NO_START_BIT			(PS2_ERROR) + 1
#define	PS2_ERROR_NO_STOP_BIT			(PS2_ERROR) + 2
#define	PS2_ERROR_PARITY				(PS2_ERROR) + 3

#define	PS2_ERROR_DEVICE_CLK_START_TO	(PS2_ERROR) + 4
#define	PS2_ERROR_DEVICE_CLK_DATA_TO	(PS2_ERROR) + 5
#define	PS2_ERROR_DEVICE_CLK_PARY_TO	(PS2_ERROR) + 6
#define	PS2_ERROR_DEVICE_CLK_STOP_TO	(PS2_ERROR) + 7
#define PS2_ERROR_DEVICE_CLK_EDGE_TO	(PS2_ERROR) + 8
#define	PS2_ERROR_DEVICE_ACK_1			(PS2_ERROR) + 9
#define	PS2_ERROR_DEVICE_ACK_2			(PS2_ERROR) + 10
#define	PS2_ERROR_DEVICE_ACK_3			(PS2_ERROR) + 11

#define	PS2_ERROR_UNKNOWN				(PS2_ERROR) + 20

#define PS2_SOCKET_ERROR_MASK			PS2_ERROR
#define PS2_SOCKET_KEYCODE_MASK			0x00ff

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC uint16 	u16PS2socketInit		(void);
PUBLIC void		  vPS2socketClose		(void);
PUBLIC void 	  vPS2socketBusEnable	(void);
PUBLIC void 	  vPS2socketBusDisable	(void);
PUBLIC bool_t 	 boPS2socketBusReady	(void);
PUBLIC uint16	u16PS2socketRead		(void);
PUBLIC uint16 	u16PS2socketWrite		(uint8 u8Datum);


/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
