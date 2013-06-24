/****************************************************************************/
/*!
 *\MODULE              JN-AN-1072 Jenie Wireless Keyboard
 *
 *\COMPONENT           $RCSfile: Coordinator.c,v $
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
 *\DESCRIPTION         Coordinator.
 *
 * This file forms the basic Jenie application for the coordinator
 * node type. This file contains the standard set of Jenie callback functions
 * that the Jenie stack calls. As the majority of the application code is the same
 * regardless of device type the functions in this file simply calls the
 * equivalent functions in Network.c.
 *
 * This file also makes calls to functions in Display.c to initialise the display
 * and to pass on received key sequence data for display.
 *
 */
/* CHANGE HISTORY
 *
 * $Log: Coordinator.c,v $
 * Revision 1.2  2007/11/22 08:33:21  mlook
 * JPI updates
 * Project and makefile renaming
 *
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

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <jenie.h>
#include <JPI.h>
//#include <gdb.h>

#include "Network.h"
#include "Display.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

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
/** Routing table storage */
PRIVATE tsJenieRoutingTable asRoutingTable[NETWORK_ROUTING_TABLE_SIZE];

/****************************************************************************/
/***        Local Constants                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Jenie Callback Functions                                      ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vJenie_CbConfigureNetwork
 *
 * DESCRIPTION:
 * Set stack parameters prior to stack initialisation.
 *
 * RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC void vJenie_CbConfigureNetwork (void)
{
	/* Set up routing table */
	gJenie_RoutingEnabled    = TRUE;
	gJenie_RoutingTableSize  = NETWORK_ROUTING_TABLE_SIZE;
	gJenie_RoutingTableSpace = (void *) asRoutingTable;

	/*  Configure the network */
	vNetwork_ConfigureNetwork(E_JENIE_COORDINATOR, NETWORK_MAX_CHILDREN, TRUE);
	/* Configure services */
	(void) bNetwork_Service_Register(NETWORK_SERVICE, 1);
	(void) bNetwork_Service_Request (NETWORK_SERVICE, 1);
}

/****************************************************************************
 *
 * NAME: vJenie_CbInit
 *
 * DESCRIPTION:
 * Perform application initialisation after stack has initialised.
 * Start running the stack.
 *
 * RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC void vJenie_CbInit (bool_t bWarmStart)
{
	/* Initialise integrated peripherals */
	(void) u32JPI_Init();

	/* Initialise display */
	vDisplay_Init();

    /* Initialise network */
	vNetwork_Init(bWarmStart, FALSE);
}

/****************************************************************************
 *
 * NAME: vJenie_CbMain
 *
 * DESCRIPTION:
 * Main application task, called repeatedly by the stack.
 * This function should be non-blocking.
 *
 * RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC void vJenie_CbMain (void)
{
#ifdef WATCHDOG_ENABLED
	vAHI_WatchdogRestart();
#endif

	/* Call the shared network main function */
	vNetwork_Main();
}

/****************************************************************************
 *
 * NAME: vJenie_CbStackMgmtEvent
 *
 * DESCRIPTION:
 * Called when stack management event has occurred.
 *
 * RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC void vJenie_CbStackMgmtEvent (teEventType   eEventType,
								   void        *pvEventPrim)
{
	/* Call the shared network stack management event function */
	vNetwork_StackMgmtEvent(eEventType, pvEventPrim);
}

/****************************************************************************
 *
 * NAME: vJenie_CbStackDataEvent
 *
 * DESCRIPTION:
 * Called when data event has occurred.
 *
 * RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC void vJenie_CbStackDataEvent (teEventType   eEventType,
								   void             *pvEventPrim)
{
	/* Call the shared network stack data event function */
	vNetwork_StackDataEvent(eEventType, pvEventPrim);
}

/****************************************************************************
 *
 * NAME: vJenie_CbHwEvent
 *
 * DESCRIPTION:
 *
 *
 * RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC void vJenie_CbHwEvent (uint32 u32DeviceId,
							uint32 u32ItemBitmap)
{
	/* Call the shared network hardware event function */
	vNetwork_HwEvent(u32DeviceId, u32ItemBitmap);
}

/****************************************************************************
 *
 * NAME 		vDevice_Service_Rx
 *
 *
 * DESCRIPTION	Receivess data over the network for a registered service.
 *
 * RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC void vDevice_Service_Rx	(
	uint64 u64Address, 	/**< Address data received from */
	uint8   u8Service, 	/**< Service to receive data */
	uint16 u16Length, 	/**< Length of data */
	uint8 *pu8Data)		/**< Pointer to data */
{
	/* Keyboard service */
	if (u8Service == NETWORK_SERVICE)
	{
		/* Pass data to keyboard task */
		vDisplay_Rx(u16Length, pu8Data);
	}
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
