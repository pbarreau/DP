/****************************************************************************/
/*!
 *\MODULE              JN-AN-1072 Jenie Wireless Keyboard
 *
 *\COMPONENT           $RCSfile: Network.h,v $
 *
 *\VERSION             $Name: HEAD $
 *
 *\REVISION            $Revision: 1.2 $
 *
 *\DATED               $Date: 2007/11/22 08:33:21 $
 *
 *\STATUS              $State: Exp $
 *
 *\AUTHOR              Martin Looker
 *
 *\DESCRIPTION         Network common handling header.
 *
 * Contains the network configuration settings and the public function prototypes
 * for the common network functions.
 */
/*\CHANGE HISTORY
 *
 * $Log: Network.h,v $
 * Revision 1.2  2007/11/22 08:33:21  mlook
 * JPI updates
 * Project and makefile renaming
 *
 * Revision 1.1  2007/11/14 10:18:23  mlook
 * Initial checkin
 *
 *
 *
 *\LAST MODIFIED BY    $Author: mlook $
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

#ifndef  NETWORK_H_INCLUDED
#define  NETWORK_H_INCLUDED

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
#define NETWORK_VERSION				6			/**< Network version */
#define NETWORK_PAN_ID				0x1072		/**< Network ID             */
#define NETWORK_APPLICATION_ID		0x10721072	/**< Application ID         */
#define NETWORK_CHANNEL				0			/**< Channel				*/
#define NETWORK_SCAN_CHANNELS		0x07FFF800	/**< Scan channels (only used when CHANNEL is 0) */
#define NETWORK_SERVICE				1			/**< Service 			    */
#define NETWORK_MAX_CHILDREN		10			/**< Default max children - can't be larger than 10 */
#define NETWORK_ROUTING_TABLE_SIZE	100			/**< Entries in routing table */

#define NETWORK_DEBUG				TRUE  	  	/**< Debug network activity if wanted */
/****************************************************************************/
/***        Exported Function Prototypes                                  ***/
/****************************************************************************/
PUBLIC void 				vNetwork_ConfigureNetwork 		(teJenieDeviceType, uint8, bool_t);
PUBLIC void 				vNetwork_Init 			 		(bool_t, bool_t);
PUBLIC void 				vNetwork_Start 			  		(void);
PUBLIC void 				vNetwork_Main 			  		(void);
PUBLIC void 				vNetwork_StackMgmtEvent   		(teEventType, void *);
PUBLIC void 				vNetwork_StackDataEvent   		(teEventType, void *);
PUBLIC void 				vNetwork_HwEvent 		  		(uint32, uint32);
PUBLIC teJenieStatusCode 	eNetwork_Tx 					(uint64, uint16, uint8 *);
PUBLIC void 				vNetwork_Rx						(uint64, uint16, uint8 *);
PUBLIC teJenieStatusCode 	eNetwork_Sleep 					(uint32, teJenieSleepMode);
PUBLIC bool_t 				bNetwork_Service_Register 		(uint8, uint8);
PUBLIC bool_t 				bNetwork_Service_Request  		(uint8, uint8);
PUBLIC teJenieStatusCode	eNetwork_Service_Tx				(uint8, uint16, uint8 *);
PUBLIC void 				vNetwork_Service_Rx				(uint64, uint8, uint16, uint8 *);
PUBLIC void					vDevice_Service_Rx				(uint64, uint8, uint16, uint8 *);

PUBLIC bool_t 	bNetwork_Up 					(void);
PUBLIC bool_t 	bNetwork_Services_Up 			(void);
PUBLIC bool_t 	bNetwork_Services_RegisterUp 	(void);
PUBLIC bool_t 	bNetwork_Services_RequestUp  	(void);
PUBLIC bool_t 	bNetwork_UartUp 				(void);
PUBLIC bool_t   bNetwork_Service_RegisterUp 	(uint8);
PUBLIC bool_t   bNetwork_Service_RequestUp 	    (uint8);
PUBLIC uint32 u32Network_Service_Mask		 	(uint8);
PUBLIC teJenieDeviceType eNetwork_DeviceType	(void);

#if defined __cplusplus
}
#endif

#endif  /* NETWORK_H_INCLUDED */

/***************************************************************************/
/*!
 *\mainpage JN-AN-1072 Jenie Wireless Keyboard
 *
 *\section Software_Design Software Design
 *
 * This Application Note uses the Jenie API to network two nodes together
 * to form a wireless keyboard application.
 *
 *\section Coordinator_Display_Node Coordinator/Display Node
 *
 * The Jenie coordinator node sets up the Jenie network, receives and decodes
 * incoming PS2 key sequences and displays the ASCII text on the LCD.
 *
 * Coordinator.c contains the code to set up and run the Jenie
 * coordinator, heavy use is made of functions in Network.c that provide
 * code common to most Jenie networking applications. Coordinator.c also
 * makes use of functions in Display.c to handle received PS2 key sequences
 * and display them. In turn Display.c uses functions in PS2protocol.c to
 * decode the PS2 key sequences into ASCII text for the LCD.
 *
 *\section Router_Keyboard_Node Router/Keyboard Node
 *
 * The Jenie router node joins the Jenie network, reads PS2 key sequences
 * from the keyboard and transmits them to the Coordinator/Display node.
 *
 * Router.c contains the code to set up and run the Jenie
 * router, heavy use is made of functions in Network.c that provide
 * code common to most Jenie networking applications. Router.c also
 * makes use of functions in Keyboard.c to read keypresses from the
 * keyboard and transmit them. In turn Keyboard.c uses functions in PS2socket.c
 * to read the data from PS2 hardware socket.
 *
 ***************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/


