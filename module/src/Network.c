/****************************************************************************/
/*!
 *\MODULE              JN-AN-1072 Jenie Wireless Keyboard
 *
 *\COMPONENT           $RCSfile: Network.c,v $
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
 *\DESCRIPTION         Network common tasks handling.
 *
 * This file provides code that is common to most Jenie applications for all
 * node types. The first set of
 * functions are equivalent to the standard Jenie callback functions and should
 * be called by those functions from the node type specific files, (Coordinator.c,
 * Router.c, EndDevice.c).
 *
 * Functions are included that will add to the list of services that are to
 * be registered, (for receiving data), and requested, (for sending data).
 * When services are added in this way code is included to register and request
 * the services, handling the Jenie stack operations that make this possible.
 *
 * The registered services, which receive data, are registered as soon as the
 * application starts. It is possible to specify the maximum number of requesting
 * services that can bind to a registered service, once the maximum number of bindings
 * are in place the template will not allow any further bindings.
 *
 * The requested services, which transmit data, are requested as soon as the
 * application starts. It is possible to specify a minimum number of bindings for a
 * requested service. The template will continously attempt to request services until
 * the minimum number of bindings are in place.
 *
 * Functions are also included to transmit data to a requested service and to
 * receive data for a registered service.
 */
/*\CHANGE HISTORY
 *
 * $Log: Network.c,v $
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

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
/* Jennic include files */
#include <jendefs.h>
#include <jenie.h>
#include <JPI.h>
#include <AppHardwareApi.h>
#include <LedControl.h>
#include <Printf.h>
/* Standard library include files */
#include <string.h>
/* Debugging include files */
//#include <gdb.h>
/* Local include files */
#include "Network.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
	E_SERVICE_STATE_IDLE	= 0,	/**< Not setting up services  			*/
	E_SERVICE_STATE_REGISTER,		/**< Attempting to register services	*/
	E_SERVICE_STATE_REQUEST,		/**< Attempting to request services		*/
} teServiceState;

typedef struct
{
	uint32 u32Add;				/**< Services to be added */
	uint32 u32Pend;				/**< Services pending a response */
	uint32 u32Ready;			/**< Services ready, (registered services or newly bound requested services). */
	uint32 u32Bound;			/**< Services bound */
	uint8  au8BindLimit[32];	/**< Services binding limits */
	uint8  au8BindCount[32];	/**< Services binding counts */
} tsServices;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void 				vNetwork_Service_State 		 	(teServiceState);
PRIVATE void 				vNetwork_Service_Timeout 		(void);
PRIVATE void 				vNetwork_Services_RegisterStart (void);
PRIVATE void 				vNetwork_Services_RequestStart  (void);
PRIVATE teJenieStatusCode	eNetwork_Service_BindTx			(char, uint8, uint64);
PRIVATE void 				vNetwork_Service_BindRx			(uint64, uint16, uint8 *);
PRIVATE teJenieStatusCode	eNetwork_Service_Bind 			(uint8, uint64);

#if NETWORK_DEBUG
PRIVATE bool_t 	bNetwork_IsString				(uint16, uint8 *);
#endif

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE teJenieDeviceType	  eDeviceType;		  /**< Device type 			*/
PRIVATE bool_t				  bNetworkUp;		  /**< Network is up 		*/
PRIVATE bool_t				  bRegisterUp;		  /**< Register Services are up */
PRIVATE bool_t				  bRequestUp;		  /**< Request services are up  */
PRIVATE bool_t				  bUartUp;		  	  /**< Uart is open   		*/
PRIVATE	teServiceState		  eServiceState;	  /**< Service state        */
PRIVATE	uint8				 u8ServiceStateTimer; /**< Service timer        */
PRIVATE tsServices			  sRegister;		  /**< Register services    */
PRIVATE tsServices			  sRequest;		  	  /**< Register services    */
PRIVATE uint8				 u8Children;		  /**< Number of children   */
PRIVATE uint8				 u8MaxChildren;		  /**< Maximum children     */
PRIVATE uint8				 u8ServiceRxTimer;	  /**< Service receive timer */
PRIVATE uint8 				 u8ServiceTxTimer;	  /**< Service transmit timer */

/****************************************************************************/
/***        Local Constants                                               ***/
/****************************************************************************/
/* Timeouts in 100ms units */
PRIVATE const uint8 			    au8ServiceStateTimeout[] = {0, 10, 30};

/* Debugging strings */
#if NETWORK_DEBUG
PRIVATE const char 					aszDeviceType[][20] = {
										"COORDINATOR",
										"ROUTER",
										"END_DEVICE"};
PRIVATE const char 					aszServiceState[][20] = {
										"IDLE",
										"REGISTER",
										"REQUEST"};
PRIVATE const char 					aszEventType[][20]  = {
										"REG_SVC_RSP",
										"SVC_REQ_RSP",
										"POLL_CMPLT",
										"PACKET_SENT",
										"PACKET_FAILED",
										"NETWORK_UP",
										"CHILD_JOINED",
										"DATA",
										"DATA_TO_SERVICE",
										"DATA_ACK",
										"DATA_TO_SERVICE_ACK",
										"STACK_RESET",
										"CHILD_LEAVE",
										"CHILD_REJECTED"};
PRIVATE const char 				    aszStatusCode[][20] = {
										"SUCCESS",
										"DEFERRED",
										"ERR_UNKNOWN",
										"ERR_INVLD_PARAM",
										"ERR_STACK_RSRC",
										"ERR_STACK_BUSY"};
#endif

/****************************************************************************/
/***        Public network functions                                      ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME 		vNetwork_ConfigureNetwork
 */
/*!
 *\DESCRIPTION 	Configures the network before the stack is initialised.
 *
 * This function performs network configuration before the stack is initialised.
 *
 * This function should be called from vJenie_CbConfigureNetwork() as it performs
 * the network configuration tasks common to all Jenie device types.
 *
 * This function is only called during a cold start.
 */
/* RETURNS
 * None
 *
 ****************************************************************************/
PUBLIC void vNetwork_ConfigureNetwork (
	teJenieDeviceType  eConfigDeviceType,	/**< Type of device to run as */
   	uint8 			  u8ConfigMaxChildren,	/**< Maximum child devices allowed */
   	bool_t			   bInitFfd)			/**< Initialise full function devices flag */
{
	/* Debug hooks: include these regardless of whether debugging or not */
	//HAL_GDB_INIT();
    //HAL_BREAKPOINT();

	/* Initialise and turn on LEDs */
	if (bInitFfd)
	{
		vLedInitFfd();
		vLedControl(2,FALSE);
		vLedControl(3,FALSE);
	}
	else
	{
		vLedInitRfd();
	}
	vLedControl(0,TRUE);
	vLedControl(1,TRUE);

	/* Set network identification parameters */
	gJenie_NetworkApplicationID  = NETWORK_APPLICATION_ID;
	gJenie_PanID 			 	 = NETWORK_PAN_ID;
	gJenie_Channel 			 	 = NETWORK_CHANNEL;
	#if NETWORK_CHANNEL
		gJenie_ScanChannels		 = (1 << NETWORK_CHANNEL);
	#else
		gJenie_ScanChannels		 = NETWORK_SCAN_CHANNELS;
	#endif

	/* Note initialisation settings */
	eDeviceType          = eConfigDeviceType;
	u8MaxChildren        = u8ConfigMaxChildren;

	/* Limit maximum children to the absolute maximum allowed (10) */
	if (u8MaxChildren > 10) u8MaxChildren = 10;
	/* Don't allow any children for an end device */
	if (eDeviceType == E_JENIE_END_DEVICE) u8MaxChildren = 0;

	/* Set up maximum children */
	gJenie_MaxChildren = u8MaxChildren;
	u8Children  	   = 0;

	/* Network is not up yet */
	bNetworkUp  = FALSE;
	bRegisterUp = FALSE;
	bRequestUp  = FALSE;
	bUartUp 	= FALSE;
	u8ServiceRxTimer = 0;
	u8ServiceTxTimer = 0;

	/* No services set up yet */
	memset(&sRegister, 0, sizeof(sRegister));
	memset(&sRequest,  0, sizeof(sRequest));

	/* Service state is idle */
	vNetwork_Service_State(E_SERVICE_STATE_IDLE);
}

/****************************************************************************
 *
 * NAME 		vNetwork_Init
 */
/*!
 *\DESCRIPTION 	Initialise the network and start the stack.
 *
 * Allows initialisation to take place and then starts the stack.
 *
 * This function should be called from vJenie_CbInit() as it performs
 * the network initialisation tasks common to all Jenie device types.
 *
 * This function called during both a cold and warm start.
 */
/* RETURNS
 * None
 *
 ****************************************************************************/
PUBLIC void vNetwork_Init (
	bool_t 			  bWarmStart,	/**< Specifies if a warm start is taking place */
   	bool_t			  bInitUart)	/**< Specifies if the UART should be opened for printf support */
{

	bool_t bUart;
	teJenieStatusCode eStatus;

	/* Note passed in uart setting */
	bUart              = bInitUart;

	/* Using GDB ? */
	#ifdef GDB
		/* Override passed in UART setting to turn it off - reserved for GDB */
		bUart = FALSE;
	#else
		/* Want to run network debugging ? */
		#if NETWORK_DEBUG
			/* Override passed in UART setting to turn it on - needed for debugging */
			bUart = TRUE;
		#endif
	#endif

	/* Uart not yet up */
	bUartUp 	= FALSE;

	/* Want to open UART ? */
	if (bUart)
	{
		/* Open UART */
		vUART_printInit();
		/* Note it is open */
		bUartUp = TRUE;
	}

	/* Enable wake timer 1 with interrupt */
	vJPI_WakeTimerEnable(E_JPI_WAKE_TIMER_1, TRUE);
	/* Run the timer for 100ms */
	vJPI_WakeTimerStart(E_JPI_WAKE_TIMER_1, 3200);

	/* Start the stack running for our device type */
	eStatus = eJenie_Start(eDeviceType);

	#if NETWORK_DEBUG
		if (bUartUp)
		{
			if (! bWarmStart)
			{
				vPrintf("\n------------------------------\n");
				vPrintf("Version(%d)\n", NETWORK_VERSION);
				vPrintf("ApplicationId(%x)\n", gJenie_NetworkApplicationID);
				vPrintf("PanId(%x)\n", gJenie_PanID);
				vPrintf("Channel(%d)\n", gJenie_Channel);
				vPrintf("MaxChildren(%d)\n", gJenie_MaxChildren);
				vPrintf("DeviceType(%s)\n", aszDeviceType[eDeviceType]);
			}
			vPrintf("vNetwork_Init(%d, %d)\n", bWarmStart, bInitUart);
			vPrintf("eJenie_Start(%s) = %s\n", aszDeviceType[eDeviceType], aszStatusCode[eStatus]);
		}
	#endif
}

/****************************************************************************
 *
 * NAME: 		vNetwork_Main
 */
/*!
 *\DESCRIPTION	Main application task, called repeatedly by the stack.
 *
 * This function should be called from vJenie_CbMain() as it performs
 * the network tasks common to all Jenie device types.
 *
 * This function should be non-blocking.
 */
/* RETURNS
 * None
 *
 ****************************************************************************/
PUBLIC void vNetwork_Main (void)
{
	/* Is the network up ? */
	if (bNetworkUp)
	{
		/* Service state is idle ? */
		if (eServiceState == E_SERVICE_STATE_IDLE)
		{
			/* Got services to register ? */
			if (sRegister.u32Add != 0)
			{
				/* Attempt to register services */
				vNetwork_Services_RegisterStart();
			}
			/* Got services to request without a button press ? */
			else if (sRequest.u32Add != 0)
			{
				/* Attempt to request non-button services */
				vNetwork_Services_RequestStart();
			}

			/* Have new services just been bound ? */
			if (sRequest.u32Ready)
			{
				/* Clear newly bound services flag */
				sRequest.u32Ready = 0;
			}
		}
		/* Registered services not up yet ? */
		if (! bRegisterUp)
		{
			/* Nothing waiting to come up ? */
			if (sRegister.u32Add == 0 && sRegister.u32Pend == 0)
			{
				/* Registered services are up */
				bRegisterUp = TRUE;
				/* Light LED0 */
				vLedControl(0, FALSE);
			}
		}
		/* Requested services not yet up ? */
		if (! bRequestUp)
		{
			/* Nothing waiting to come up ? */
			if (sRequest.u32Add == 0 && sRequest.u32Pend == 0)
			{
				/* Requested services are up */
				bRequestUp = TRUE;
				/* Light LED1 */
				vLedControl(1, FALSE);
			}
		}
	}
}

/****************************************************************************
 *
 * NAME		 	vNetwork_StackMgmtEvent
 */
/*!
 *\DESCRIPTION	Called when stack management event has occurred.
 *
 * This function should be called from vJenie_CbStackMgmtEvent() as it performs
 * the network management tasks common to all Jenie device types.
 */
/* RETURNS
 * None
 *
 ****************************************************************************/
PUBLIC void vNetwork_StackMgmtEvent (
	teEventType   eEventType,	/**< Type of stack management event received */
	void        *pvEventPrim)	/**< Pointer to event primitive */
{
	teJenieStatusCode eStatus;
	uint8   u8Service;
	uint32 u32Service;

	/* Which event has occurred ? */
	switch (eEventType)
	{

	/* Register Service Response ? */
	case E_JENIE_REG_SVC_RSP:
		{
			/* Empty primitive structure for this event */
			;
			/* Network debugging */
			#if NETWORK_DEBUG
				if (bUartUp)
				{
					vPrintf("vNetwork_StackMgmtEvent(%s)\n",
							aszEventType[eEventType]);
				}
			#endif

			/* Are we waiting for a register response ? */
			if (eServiceState == E_SERVICE_STATE_REGISTER)
			{
				/* Note the services we registered */
				sRegister.u32Ready |= sRegister.u32Pend;
				sRegister.u32Pend  = 0;
				/* Return to idle state */
				vNetwork_Service_State(E_SERVICE_STATE_IDLE);
			}
		}
		break;

	/* Request Service Response ? */
	case E_JENIE_SVC_REQ_RSP:
		{
			/* Get pointer to correct primitive structure */
			tsSvcReqRsp *psSvcReqRsp = (tsSvcReqRsp *) pvEventPrim;

			/* Network debugging */
			#if NETWORK_DEBUG
				if (bUartUp)
				{
					vPrintf("vNetwork_StackMgmtEvent(%s, %x:%x, %x)\n",
							aszEventType[eEventType],
							(uint32) (psSvcReqRsp->u64SrcAddress >> 32),
							(uint32) (psSvcReqRsp->u64SrcAddress  & 0xFFFFFFFF),
							psSvcReqRsp->u32Services);
				}
			#endif

			/* Are we waiting for a request response ? */
			if (eServiceState == E_SERVICE_STATE_REQUEST)
			{
				/* Loop through individual services */
				for (u8Service = 1; u8Service <= 32; u8Service++)
				{
					/* Get mask for service */
					u32Service = u32Network_Service_Mask(u8Service);
					/* Is this service supported ? */
					if (psSvcReqRsp->u32Services & u32Service)
					{
						/* Send bind query for service */
						(void) eNetwork_Service_BindTx('q', u8Service, psSvcReqRsp->u64SrcAddress);
					}
				}
				/* No more requested services pending ? */
				if (sRequest.u32Pend == 0)
				{
					/* Return to idle state */
					vNetwork_Service_State(E_SERVICE_STATE_IDLE);
				}
			}
		}
		break;

	/* Poll complete ? */
	case E_JENIE_POLL_CMPLT:
		/* Don't do any debugging - too many of these */
		;
		break;

	/* Network up ? */
	case E_JENIE_NETWORK_UP:
		{
			/* Network debugging */
			#if NETWORK_DEBUG
				if (bUartUp)
				{
					/* Get pointer to correct primitive structure */
					tsNwkStartUp *psNwkStartUp = (tsNwkStartUp *) pvEventPrim;
					/* Output to UART */
					vPrintf("vNetwork_StackMgmtEvent(NETWORK_UP, %x:%x, %x:%x, %d, %x, %d)\n",
						(uint32)(psNwkStartUp->u64ParentAddress >> 32),
						(uint32)(psNwkStartUp->u64ParentAddress &  0xFFFFFFFF),
						(uint32)(psNwkStartUp->u64LocalAddress  >> 32),
						(uint32)(psNwkStartUp->u64LocalAddress  &  0xFFFFFFFF),
						psNwkStartUp->u16Depth,
						psNwkStartUp->u16PanID,
						psNwkStartUp->u8Channel);
				}
			#endif

			/* Note network is now up */
			bNetworkUp = TRUE;

			/* End device or we have all the children we want ? */
			if (eDeviceType == E_JENIE_END_DEVICE || u8Children >= u8MaxChildren )
			{
				/* Don't allow devices to join to us */
				eStatus = eJenie_SetPermitJoin(FALSE);

				/* Network debugging */
				#if NETWORK_DEBUG
					if (bUartUp)
					{
						vPrintf("eJenie_SetPermitJoin(FALSE) = %s\n",
								aszStatusCode[eStatus]);
					}
				#endif
			}
			else
			{
				/* Allow devices to join to us */
				eStatus = eJenie_SetPermitJoin(TRUE);

				/* Network debugging */
				#if NETWORK_DEBUG
					if (bUartUp)
					{
						vPrintf("eJenie_SetPermitJoin(TRUE) = %s\n",
							aszStatusCode[eStatus]);
					}
				#endif
			}
		}
		break;

	/* Child joined ? */
	case E_JENIE_CHILD_JOINED:
		{
			/* Network debugging */
			#if NETWORK_DEBUG
				if (bUartUp)
				{
					/* Get pointer to correct primitive structure */
					tsChildJoined *psChildJoined = (tsChildJoined *) pvEventPrim;
					vPrintf("vNetwork_StackMgmtEvent(%s, %x:%x)\n",
							aszEventType[eEventType],
							(uint32) (psChildJoined->u64SrcAddress >> 32),
							(uint32) (psChildJoined->u64SrcAddress  & 0xFFFFFFFF));
				}
			#endif

			/* Update number of children */
			u8Children++;

			/* Got all the children we want ? */
			if (u8Children >= u8MaxChildren )
			{
				/* Don't allow devices to join to us */
				eStatus = eJenie_SetPermitJoin(FALSE);

				/* Network debugging */
				#if NETWORK_DEBUG
					if (bUartUp)
					{
						vPrintf("eJenie_SetPermitJoin(FALSE) = %s\n",
								aszStatusCode[eStatus]);
					}
				#endif
			}
		}
		break;

	/* Others ? */
	default:
		{
			/* Network debugging */
			#if NETWORK_DEBUG
				if (bUartUp)
				{
					vPrintf("vNetwork_StackMgmtEvent(%s)\n",
                	    	aszEventType[eEventType]);
                }
			#endif
		}
		break;
	}
}

/****************************************************************************
 *
 * NAME 		vNetwork_StackDataEvent
 */
/*!
 *\DESCRIPTION	Called when data event has occurred.
 *
 * This function should be called from vJenie_CbStackDataEvent() as it passes
 * the received data onto vNetwork_Service_Rx() for data sent to a registered service
 * and vNetwork_Rx() for addressed directly to the device.
 */
/* RETURNS
 * None
 *
 ****************************************************************************/
PUBLIC void vNetwork_StackDataEvent (
	teEventType   eEventType,	/**< Type of data event received */
    void        *pvEventPrim)	/**< Pointer to event primitive */
{
	/* Which event has occurred ? */
	switch (eEventType)
	{
	/* Data to service ? */
	case E_JENIE_DATA_TO_SERVICE:
		{
			/* Get pointer to correct primitive structure */
			tsDataToService *psDataToService = (tsDataToService *) pvEventPrim;

			/* Network debugging */
			#if NETWORK_DEBUG
				if (bUartUp)
				{
					/* Data is a string ? */
					if (bNetwork_IsString(psDataToService->u16Length, psDataToService->pau8Data))
					{
						/* Debug with data string */
						vPrintf("Data pour service \"Texte\" (%s, %x:%x, %d, %d, %x, %d, \"%s\")\n",
							aszEventType[eEventType],
							(uint32) (psDataToService->u64SrcAddress >> 32),
							(uint32) (psDataToService->u64SrcAddress  & 0xFFFFFFFF),
							psDataToService->u8SrcService,
							psDataToService->u8DestService,
							psDataToService->u8MsgFlags,
							psDataToService->u16Length,
							(char *) psDataToService->pau8Data);
					}
					else
					{
						/* Debug without data */
						vPrintf("Data pour service(%s, %x:%x, %d, %d, %x, %d)\n",
							aszEventType[eEventType],
							(uint32) (psDataToService->u64SrcAddress >> 32),
							(uint32) (psDataToService->u64SrcAddress  & 0xFFFFFFFF),
							psDataToService->u8SrcService,
							psDataToService->u8DestService,
							psDataToService->u8MsgFlags,
							psDataToService->u16Length);
					}
				}
			#endif

			/* Receive the data */
			vNetwork_Service_Rx(psDataToService->u64SrcAddress,
							    psDataToService->u8SrcService,
								psDataToService->u16Length,
								psDataToService->pau8Data);
			}
		break;

	/* Data ? */
	case E_JENIE_DATA:
		{
			/* Get pointer to correct primitive structure */
			tsData *psData = (tsData *) pvEventPrim;

			/* Network debugging */
			#if NETWORK_DEBUG
				if (bUartUp)
				{
					/* Data is a string ? */
					if (bNetwork_IsString(psData->u16Length, psData->pau8Data))
					{
						/* Debug with data string */
						vPrintf("Data \"Texte\" (%s, %x:%x, %x, %d, \"%s\")\n",
							aszEventType[eEventType],
							(uint32) (psData->u64SrcAddress >> 32),
							(uint32) (psData->u64SrcAddress  & 0xFFFFFFFF),
							psData->u8MsgFlags,
							psData->u16Length,
							(char *) psData->pau8Data);
					}
					else
					{
						/* Debug without data */
						vPrintf("Data (%s, %x:%x, %x, %d)\n",
							aszEventType[eEventType],
							(uint32) (psData->u64SrcAddress >> 32),
							(uint32) (psData->u64SrcAddress  & 0xFFFFFFFF),
							psData->u8MsgFlags,
							psData->u16Length);
					}
				}
			#endif

			/* Receive the data */
			vNetwork_Rx(psData->u64SrcAddress,
					    psData->u16Length,
						psData->pau8Data);
			}
		break;

	/* Others ? */
	default:
		/* Network debugging */
		#if NETWORK_DEBUG
			if (bUartUp)
			{
				vPrintf("vNetwork_StackDataEvent(%s)\n", aszEventType[eEventType]);
			}
		#endif
		break;
	}
}

/****************************************************************************
 *
 * NAME 		vNetwork_HwEvent
 */
/*!
 *\DESCRIPTION	Called when a stack hardware event has occurred.
 *
 * This function should be called from vJenie_HwEvent() as it performs
 * the network management tasks common to all Jenie device types.
 */
/* RETURNS
 * None
 *
 ****************************************************************************/
PUBLIC void vNetwork_HwEvent (
	uint32 u32DeviceId,		/**< Device that generated the event. */
 	uint32 u32ItemBitmap)	/**< Source within device that generated the event. */
{
	teJenieStatusCode eStatus;

	/* Wake timer 1 event ? */
	if (u32DeviceId  == E_JPI_DEVICE_SYSCTRL &&
	   (u32ItemBitmap & E_JPI_SYSCTRL_WK1_MASK) != 0)
	{
		/* End Device and network is up ? */
		if (eDeviceType == E_JENIE_END_DEVICE && bNetworkUp)
		{
			/* Poll parent for data */
			eStatus = eJenie_PollParent();
		}
		/* Timer running ? */
		if (u8ServiceStateTimer > 0)
		{
			/* Decrement timer */
			u8ServiceStateTimer--;
			/* Timer expired ? */
			if (u8ServiceStateTimer == 0)
			{
				/* Handle the timeout */
				vNetwork_Service_Timeout();
			}
		}
		/* Receive LED timer running ? */
		if (u8ServiceRxTimer > 0)
		{
			u8ServiceRxTimer--;
			/* Expired - turn out LED */
			if (u8ServiceRxTimer == 0 && bRegisterUp) vLedControl(0, FALSE);
			/* Running - turn on */
			else vLedControl(0, TRUE);
		}
		/* Transmit LED timer running ? */
		if (u8ServiceTxTimer > 0)
		{
			u8ServiceTxTimer--;
			/* Expired - turn out LED */
			if (u8ServiceTxTimer == 0 && bRequestUp) vLedControl(1, FALSE);
			/* Running - turn on */
			else vLedControl(1, TRUE);
		}

		/* Run the timer for another 100ms */
		vJPI_WakeTimerStart(E_JPI_WAKE_TIMER_1, 3200);
	}
}

/****************************************************************************
 *
 * NAME 		vNetwork_Tx
 */
/*!
 *\DESCRIPTION	Transmits data over the network - independent of a service.
 *
 * Transmits data over the network to a specified address, no service is
 * used.
 *
 * This function is used to implement to binding permission protocol.
 *
 * This function simply adds debugging of the call to eJenie_SendData().
 */
/* RETURNS
 * None
 *
 ****************************************************************************/
PUBLIC teJenieStatusCode eNetwork_Tx (
	uint64 u64Address, 	/**< Address to transmit data to */
	uint16 u16Length,  	/**< Length of data */
	uint8 *pu8Data)		/**< Pointer to data */
{
	teJenieStatusCode  eStatus;

	/* Transmit the data */
	eStatus = eJenie_SendData(u64Address, pu8Data, u16Length, TXOPTION_ACKREQ);

	/* Network debugging */
	#if NETWORK_DEBUG
		if (bUartUp)
		{
			if (bNetwork_IsString(u16Length, pu8Data))
			{
				/* Debug with data */
				vPrintf("eJenie_SendData(%x:%x, \"%s\", %d, %x) = %s\n",
								(uint32) (u64Address >> 32),
								(uint32) (u64Address  & 0xFFFFFFFF),
								(char *) pu8Data,
								u16Length,
								TXOPTION_ACKREQ,
								aszStatusCode[eStatus]);
			}
			else
			{
				/* Debug without data */
				vPrintf("eJenie_SendData(%x:%x, %d, %x) = %s\n",
								(uint32) (u64Address >> 32),
								(uint32) (u64Address  & 0xFFFFFFFF),
								u16Length,
								TXOPTION_ACKREQ,
								aszStatusCode[eStatus]);
			}
		}
	#endif

	return eStatus;	/**< \return Status code from eJenie_SendData() */
}

/****************************************************************************
 *
 * NAME	 		vNetwork_Rx
 */
/*!
 *\DESCRIPTION	Receive data from network - independent of service.
 *
 * This function is called by vNetwork_StackDataEvent() when a data message
 * has been received from an address, no service is used.
 *
 * This function is used to implement to binding permission protocol.
 */
/* RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC void vNetwork_Rx	(
	uint64 u64Address, 	/**< Address data received from */
	uint16 u16Length, 	/**< Length of data */
	uint8 *pu8Data)		/**< Pointer to data */
{

	/* Network debugging */
	#if NETWORK_DEBUG
		if (bUartUp)
		{
			if (bNetwork_IsString(u16Length, pu8Data))
			{
				/* Debug with data */
				vPrintf("vNetwork_Rx(%x:%x, %d, \"%s\")\n",
								(uint32) (u64Address >> 32),
								(uint32) (u64Address  & 0xFFFFFFFF),
								u16Length,
								(char *) pu8Data);
			}
			else
			{
				/* Debug with data */
				vPrintf("vNetwork_Rx(%x:%x, %d)\n",
								(uint32) (u64Address >> 32),
								(uint32) (u64Address  & 0xFFFFFFFF),
								u16Length);
			}
		}
	#endif

	/* Does this look like an binding protocol message ? */
	if (pu8Data[0] == '<' && pu8Data[1] == 'B')
	{
		/* Handle the binding message */
		vNetwork_Service_BindRx(u64Address, u16Length, pu8Data);
	}
	else
	{
		/* PROCESS DATA HERE */
		;
	}
}

/****************************************************************************
 *
 * NAME	 		eNetwork_Sleep
 */
/*!
 *\DESCRIPTION	Go to sleep.
 */
/* RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC teJenieStatusCode eNetwork_Sleep	(
	uint32 				u32SleepPeriodMs, 	/**< Length of time to sleep for (ms) */
	teJenieSleepMode 	  eSleepMode)		/**< Sleep mode */
{
	teJenieStatusCode  eStatus = E_JENIE_ERR_INVLD_PARAM;

	/* Are we an end device ? */
	if (eNetwork_DeviceType() == E_JENIE_END_DEVICE)
	{
		/* Disable wake timer 1 interrupt so we don't get woken from sleeping */
		vJPI_WakeTimerEnable(E_JPI_WAKE_TIMER_1, FALSE);
		/* Set the sleep period (this is set in ms) */
		(void) eJenie_SetSleepPeriod(u32SleepPeriodMs);
		/* Go to sleep */
		eStatus = eJenie_Sleep(eSleepMode);
		/* Network debugging */
		#if NETWORK_DEBUG
			if (bUartUp)
			{
				vPrintf("eJenie_Sleep(%d) = %s\n",
						eSleepMode,
						aszStatusCode[eStatus]);
			}
		#endif
	}

	return eStatus;
}

/****************************************************************************/
/***        Public service functions                                      ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME 	 	bNetwork_Service_Register
 */
/*!
 *\DESCRIPTION	Add a service to be registered.
 *
 * This function adds a service to be registered in order to receive data.
 * and specifies how many requesting services can bind to it
 *
 * This function would normally be called from vJenie_CbConfigureNetwork()
 * after the call to vNetwork_ConfigureNetwork(), though it can be called to
 * register new services at any time.
 *
 * Services added using this function will be automatically registered by code
 * in the remainder of Network.c.
 */
/* RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC bool_t bNetwork_Service_Register (
	uint8 u8Service,	/**< Service to be registered. */
	uint8 u8BindLimit)	/**< Number of devices that can bind to service, (0 unlimited). */
{

	bool_t   bAdded = FALSE;
	uint32 u32Service;
	uint32 u32Mask;

	/* Get mask for service to be registered */
	u32Service = u32Network_Service_Mask(u8Service);

	/* Get mask for registered services we don't already have in progress */
	u32Mask = ~(sRegister.u32Add | sRegister.u32Pend | sRegister.u32Ready | sRegister.u32Bound);

	/* Combine with service we want to register */
	u32Mask &= u32Service;

	/* If bind limit is 0 max it out */
	if (u8BindLimit == 0) u8BindLimit = 0xFF;

	/* Have we got a new service to add ? */
	if (u32Mask)
	{
		/* Prepare to add any services we don't have already */
		sRegister.u32Add |= u32Mask;
		/* Set the binding limit for the service */
		sRegister.au8BindLimit[u8Service-1] = u8BindLimit;
		/* Registered services are not up yet */
		bRegisterUp = FALSE;
		/* Turn on LED prior to registering service */
		vLedControl(0, TRUE);
		/* Note we prepared to register the service */
		bAdded = TRUE;
	}

	return bAdded; /**< \return TRUE when new service is added for registering */
}

/****************************************************************************
 *
 * NAME 	 	bNetwork_Service_Request
 */
/*!
 *\DESCRIPTION	Add a service to be requested and bound.
 *
 * This function adds a service to be requested and bound in order to transmit
 * data and specifies how many registered services it should attempt to bind to.
 *
 * This function would normally be called either from vJenie_CbConfigureNetwork()
 * after the call to vNetwork_ConfigureNetwork() in order to request services
 * that always need to be available or it could be called to initiate the request
 * process when controlling the binding process through the use of buttons.
 *
 * Services added using this function will be automatically requested and bound by code
 * in the remainder of Network.c.
 */
/* RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC bool_t bNetwork_Service_Request (
	uint8 u8Service,	/**< Service to be requested. */
	uint8 u8BindLimit)	/**< Number of devices that service can bind to.
						 *	 Minimum bindings that will be made, (0 is changed to one).*/
{

	bool_t   bAdded = FALSE;
	uint32 u32Service;
	uint32 u32Mask;

	/* Get mask for service to be requested */
	u32Service = u32Network_Service_Mask(u8Service);

	/* Get mask for requested services we don't already have in progress */
	u32Mask = ~(sRequest.u32Add | sRequest.u32Pend | sRequest.u32Ready | sRequest.u32Bound);

	/* Combine with service we want to request */
	u32Mask &= u32Service;

	/* If bind limit is 0 update it to one */
	if (u8BindLimit == 1) u8BindLimit = 1;

	/* Have we got a new service to add ? */
	if (u32Mask)
	{
		/* Prepare to add any services we don't have already */
		sRequest.u32Add |= u32Mask;
		/* Set the binding limit for the service */
		sRequest.au8BindLimit[u8Service-1] = u8BindLimit;
		/* Requested services are not up yet */
		bRequestUp = FALSE;
		/* Turn on LED prior to requesting service */
		vLedControl(1, TRUE);
		/* Note we prepared to request the service */
		bAdded = TRUE;
	}

	return bAdded; /**< \return TRUE when new service is added for requesting */
}

/****************************************************************************
 *
 * NAME: 		eNetwork_Service_Tx
 */
/*!
 *\DESCRIPTION	Transmits data over the network to a requested service.
 */
/* RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC teJenieStatusCode eNetwork_Service_Tx (
	uint8   u8Service, 	/**< Service to transmit data to */
	uint16 u16Length, 	/**< Length of data */
	uint8 *pu8Data)		/**< Pointer to data */
{
	bool_t 			  bLive = FALSE;
	teJenieStatusCode eStatus = E_JENIE_ERR_INVLD_PARAM;
	uint32			 u32Service;

	/* Get mask for service */
	u32Service = u32Network_Service_Mask(u8Service);

	/* Is this service live ? */
	if (u32Service & sRequest.u32Bound)
	{
		/* Note we're live */
		bLive = TRUE;
		/* Set transmit timer - used to flash LED */
		u8ServiceTxTimer = 2;
		/* Send the data */
		eStatus = eJenie_SendDataToBoundService(u8Service, pu8Data, u16Length, TXOPTION_ACKREQ);
	}

	/* Network debugging */
	#if NETWORK_DEBUG
		if (bUartUp)
		{
			if (bNetwork_IsString(u16Length, pu8Data))
			{
				/* Debug with data */
				vPrintf("bNetwork_Service_Tx(%d, %d, \"%s\") = %d\n",
								u8Service,
								u16Length,
								(char *) pu8Data,
								bLive);
				/* Sent the data ? */
				if (bLive)
				{
					/* Debug with data */
					vPrintf("eJenie_SendDataToBoundService(%d, \"%s\", %d, %x) = %s\n",
									u8Service,
									(char *) pu8Data,
									u16Length,
									TXOPTION_ACKREQ,
									aszStatusCode[eStatus]);
				}
			}
			else
			{
				/* Debug with data */
				vPrintf("bNetwork_Service_Tx(%d, %d) = %d\n",
								u8Service,
								u16Length,
								bLive);
				/* Sent the data */
				if (bLive)
				{
					/* Debug without data */
					vPrintf("eJenie_SendDataToBoundService(%d, %d, %x) = %s\n",
									u8Service,
									u16Length,
									TXOPTION_ACKREQ,
									aszStatusCode[eStatus]);
				}
			}
		}
	#endif

	return eStatus;	/**< \return Status of call to eJenie_SendDataToBoundService(). */
}

/****************************************************************************
 *
 * NAME 		vNetwork_Service_Rx
 */
/*!
 *\DESCRIPTION	Receivess data over the network for a registered service.
 */
/* RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC void vNetwork_Service_Rx	(
	uint64 u64Address, 	/**< Address data received from */
	uint8   u8Service, 	/**< Service to receive data */
	uint16 u16Length, 	/**< Length of data */
	uint8 *pu8Data)		/**< Pointer to data */
{
	bool_t 			  bLive = FALSE;
	uint32			 u32Service;

	/* Get mask for service */
	u32Service = u32Network_Service_Mask(u8Service);

	/* Is this service live ? */
	if (u32Service & sRegister.u32Bound)
	{
		/* Note we're live */
		bLive = TRUE;
		/* Set receive timer - used to flash LED */
		u8ServiceRxTimer = 2;

		/* Pass data on to device module one of Coordinator.c, Router.c or EndDevice.c
		 * though function is declared in Network.h */
		vDevice_Service_Rx(u64Address, u8Service, u16Length, pu8Data);
	}

	/* Network debugging */
	#if NETWORK_DEBUG
		if (bUartUp)
		{
			if (bNetwork_IsString(u16Length, pu8Data))
			{
				/* Debug with data */
				vPrintf("vNetwork_Service_Rx(%x:%x, %d, %d, \"%s\") = %d\n",
								(uint32) (u64Address >> 32),
								(uint32) (u64Address  & 0xFFFFFFFF),
								u8Service,
								u16Length,
								(char *) pu8Data,
								bLive);
			}
			else
			{
				/* Debug with data */
				vPrintf("vNetwork_Service_Rx(%x:%x, %d, %d) = %d\n",
								(uint32) (u64Address >> 32),
								(uint32) (u64Address  & 0xFFFFFFFF),
								u8Service,
								u16Length,
								bLive);
			}
		}
	#endif
}

/****************************************************************************
 *
 * NAME 		vNetwork_Service_Mask
 */
/*!
 *\DESCRIPTION 	Calculate bitmask for a service.
 */
/* RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC uint32 u32Network_Service_Mask (uint8 u8Service) /**< Service to calculate bitmask for */
{
	uint32 u32Service = 0;

	if (u8Service >= 1 && u8Service <= 32)
	{
		u32Service = ((uint32) 1 << (u8Service - 1));
	}

	return u32Service;	/**< \return Bitmask for service. */
}

/****************************************************************************
 *
 * NAME 		bNetwork_Service_RegisterUp
 */
/*!
 *\DESCRIPTION 	Checks whether a registered service is up and running.
 */
/* RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC bool_t   bNetwork_Service_RegisterUp (uint8 u8Service)
{
	return (sRegister.u32Bound & u32Network_Service_Mask(u8Service) ? TRUE : FALSE);
}

/****************************************************************************
 *
 * NAME 		bNetwork_Service_RequestUp
 */
/*!
 *\DESCRIPTION 	Checks whether a requested service is up and running.
 */
/* RETURNS
 * None
 *
 ****************************************************************************/
PUBLIC bool_t   bNetwork_Service_RequestUp (uint8 u8Service)
{
	return (sRequest.u32Bound & u32Network_Service_Mask(u8Service) ? TRUE : FALSE);
}

/****************************************************************************/
/***        Public data access functions                                  ***/
/****************************************************************************/

PUBLIC bool_t 	bNetwork_Up 		 		 (void) { return  bNetworkUp;  }
PUBLIC bool_t 	bNetwork_Services_Up 		 (void) { return (bRegisterUp & bRequestUp); }
PUBLIC bool_t 	bNetwork_Services_RegisterUp (void) { return  bRegisterUp; }
PUBLIC bool_t 	bNetwork_Services_RequestUp  (void) { return  bRequestUp; }
PUBLIC bool_t 	bNetwork_UartUp 			 (void) { return  bUartUp; }
PUBLIC teJenieDeviceType eNetwork_DeviceType (void) { return  eDeviceType; }

/****************************************************************************/
/***        Private service functions                                     ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME 		vNetwork_Service_State
 */
/*!
 *\DESCRIPTION  Update service state machine and timer.
 */
/* RETURNS:
 * None
 *
 ****************************************************************************/
PRIVATE void vNetwork_Service_State (teServiceState eState)
{
	/* Service state changed ? */
	if (eServiceState != eState)
	{
		/* Note the new service state */
		eServiceState = eState;

		/* Set the service state timer */
		u8ServiceStateTimer = au8ServiceStateTimeout[eServiceState];

		/* Network debugging */
		#if NETWORK_DEBUG
			if (bUartUp)
			{
				vPrintf("vNetwork_Service_State(%s)\n",
						aszServiceState[eServiceState]);
			}
		#endif
	}
}

/****************************************************************************
 *
 * NAME: vNetwork_Service_Timeout
 */
/*!
 *\DESCRIPTION Handle a service state timeout.
 */
/* RETURNS:
 * None
 *
 ****************************************************************************/
PRIVATE void vNetwork_Service_Timeout (void)
{
	/* Network debugging */
	#if NETWORK_DEBUG
		if (bUartUp)
		{
			vPrintf("vNetwork_Service_Timeout(%s)\n",
				aszServiceState[eServiceState]);
		}
	#endif

	/* Are we waiting for a register response ? */
	if (eServiceState == E_SERVICE_STATE_REGISTER)
	{
		/* Try to register the pending service again */
		sRegister.u32Add |= sRegister.u32Pend;
		sRegister.u32Pend  = 0;
	}
	/* Are we waiting for a request response ? */
	else if (eServiceState == E_SERVICE_STATE_REQUEST)
	{
		/* Try to register the pending service again */
		sRequest.u32Add  |= sRequest.u32Pend;
		sRequest.u32Pend  = 0;
	}

	/* Return to idle state */
	vNetwork_Service_State(E_SERVICE_STATE_IDLE);
}

/****************************************************************************
 *
 * NAME: vNetwork_Services_RegisterStart
 */
/*!
 *\DESCRIPTION Starts registering services waiting to be added.
 */
/* RETURNS:
 * None
 *
 ****************************************************************************/
PRIVATE void vNetwork_Services_RegisterStart (void)
{
	teJenieStatusCode eStatus;

	/* Check we are idle and have services to register */
	if (eServiceState    == E_SERVICE_STATE_IDLE &&
		sRegister.u32Add != 0)
	{
		/* Note the services we are going to try to register */
		sRegister.u32Pend = sRegister.u32Add;
		sRegister.u32Add  = 0;

		/* Attempt to register services */
		eStatus = eJenie_RegisterServices(sRegister.u32Pend);

		/* Network debugging */
		#if NETWORK_DEBUG
			if (bUartUp)
			{
				vPrintf("eJenie_RegisterServices(%x) = %s\n",
						sRegister.u32Pend,
						aszStatusCode[eStatus]);
			}
		#endif

		/* Success ? */
		if (eStatus == E_JENIE_SUCCESS)
		{
			/* Note the services we registered */
			sRegister.u32Ready |= sRegister.u32Pend;
			sRegister.u32Pend  = 0;
		}
		else
		{
			/* Wait for register service response next */
			vNetwork_Service_State(E_SERVICE_STATE_REGISTER);
		}
	}
}

/****************************************************************************
 *
 * NAME: vNetwork_Services_RequestStart
 */
/*!
 *\DESCRIPTION Starts requesting services waiting to be added.
 */
/* RETURNS:
 * None
 *
 ****************************************************************************/
PRIVATE void vNetwork_Services_RequestStart (void)
{
	teJenieStatusCode 	  eStatus;

	/* Check we are idle and have services to request */
	if (eServiceState    == E_SERVICE_STATE_IDLE &&
		sRequest.u32Add != 0)
	{
		/* Note the services we are going to try to request */
		sRequest.u32Pend  = sRequest.u32Add;
		sRequest.u32Add   = 0;

		/* Attempt to request services */
		eStatus = eJenie_RequestServices(sRequest.u32Pend, FALSE);

		/* Network debugging */
		#if NETWORK_DEBUG
			if (bUartUp)
			{
				vPrintf("eJenie_RequestServices(%x) = %s\n",
						sRequest.u32Pend,
						aszStatusCode[eStatus]);
			}
		#endif

		/* Wait for request service response next */
		vNetwork_Service_State(E_SERVICE_STATE_REQUEST);
	}
}

/****************************************************************************
 *
 * NAME: vNetwork_Service_BindTx
 */
/*!
 *\DESCRIPTION Transmits a binding protocol message over the network.
 */
/* RETURNS:
 * None
 *
 ****************************************************************************/
PRIVATE teJenieStatusCode eNetwork_Service_BindTx (char cType, uint8 u8Service, uint64 u64Address)
{
	teJenieStatusCode  eStatus = E_JENIE_ERR_INVLD_PARAM;

	/* Valid service ? */
	if (u8Service >= 1 && u8Service <= 32)
	{
		/* Create bind message */
		char aszBind[7] = "<B_00>";
		/* Place command type into message */
		aszBind[2] = cType;
		/* Place service number into bind message */
		aszBind[3] += u8Service/10;
		aszBind[4] += u8Service%10;
		/* Transmit bind message */
		eStatus = eNetwork_Tx (u64Address, 7, (uint8 *) aszBind);
	}

	return eStatus;
}

/****************************************************************************
 *
 * NAME: vNetwork_Service_BindRx
 */
/*!
 *\DESCRIPTION Receives a binding protocol message from the network.
 */
/* RETURNS:
 * None
 *
 ****************************************************************************/
PRIVATE void vNetwork_Service_BindRx	(uint64 u64Address, uint16 u16Length, uint8 *pu8Data)
{
	uint8 	u8Service;
	uint32 u32Service;

	/* Does this look like an binding protocol message ? */
	if (pu8Data[0] == '<' && pu8Data[1] == 'B')
	{
		/* Extract service from message */
		u8Service  = (pu8Data[3] - '0') * 10;
		u8Service += (pu8Data[4] - '0');
		/* Get service mask */
		u32Service = u32Network_Service_Mask(u8Service);
		/* Sensible service specified ? */
		if (u32Service != 0)
		{
			/* What is the message type ? */
			switch (pu8Data[2])
			{
				/* Query - asking permission to bind ? */
				case 'q':
					{
						/* Is this service setup to receive data and
						   we've not reached the number of bindings limit ? */
						if ((u32Service & sRegister.u32Ready) != 0 &&
							 sRegister.au8BindCount[u8Service-1] < sRegister.au8BindLimit[u8Service-1])
						{
							/* Send the response allow the binding request */
							eNetwork_Service_BindTx('y', u8Service, u64Address);
						}
					}
					break;
				/* Bound - being told we've been bound to ? */
				case 'b':
					{
						/* Increment binding counter */
						sRegister.au8BindCount[u8Service-1]++;
						/* Note the service that has been bound */
						sRegister.u32Bound |= u32Service;
					}
					break;
				/* Accept - granted permission to bind ? */
				case 'y':
					{
						/* Attempt to bind to the service */
						(void) eNetwork_Service_Bind(u8Service, u64Address);
					}
					break;
				/* Others */
				default:
					break;
			}
		}
	}
}

/****************************************************************************
 *
 * NAME eNetwork_Service_Bind
 */
/*!
 *\DESCRIPTION Binds a requested service.
 */
/* RETURNS
 * None
 *
 ****************************************************************************/
PRIVATE teJenieStatusCode eNetwork_Service_Bind (uint8 u8Service, uint64 u64Address)
{
	teJenieStatusCode  eStatus = E_JENIE_ERR_INVLD_PARAM;
	uint32 			 u32Service;

	/* Get mask for service */
	u32Service = u32Network_Service_Mask(u8Service);

	/* Trying to bind to a service we requested and
	   we've not yet reached the bind limit ? */
	if ((sRequest.u32Pend & u32Service) != 0 &&
		sRequest.au8BindCount[u8Service-1] < sRequest.au8BindLimit[u8Service-1])
	{
		/* Attempt to bind to the service */
		eStatus = eJenie_BindService(u8Service,
									 u64Address,
									 u8Service);

		/* Network debugging */
		#if NETWORK_DEBUG
			if (bUartUp)
			{
				vPrintf("eJenie_BindService(%d, %x:%x, %d) = %s\n",
						u8Service,
						(uint32) (u64Address >> 32),
						(uint32) (u64Address  & 0xFFFFFFFF),
						u8Service,
						aszStatusCode[eStatus]);
			}
		#endif

		/* Bind successful ? */
		if (eStatus == E_JENIE_SUCCESS)
		{
			/* Increment the bind count */
			sRequest.au8BindCount[u8Service-1]++;

			/* Note the service we bound */
			sRequest.u32Ready |=  u32Service;
			sRequest.u32Bound |=  u32Service;

			/* Have we reached the bind limit ? */
			if (sRequest.au8BindCount[u8Service-1] == sRequest.au8BindLimit[u8Service-1])
			{
				/* Clear the pending flag - we can stop requesting that service now */
				sRequest.u32Pend &= (~u32Service);
				/* Go back to being idle */
				vNetwork_Service_State(E_SERVICE_STATE_IDLE);
			}

			/* Send bound message for service */
			(void) eNetwork_Service_BindTx('b', u8Service, u64Address);
		}
	}

	return eStatus;
}

/****************************************************************************
 *
 * NAME: bNetwork_IsString(u16Length, pu8Data);
 */
/*!
 *\DESCRIPTION Checks if a data array can be treated as a string for debugging.
 */
/* RETURNS:
 * None
 *
 ****************************************************************************/
#if NETWORK_DEBUG
PRIVATE bool_t bNetwork_IsString(uint16 u16Len, uint8 *pu8Data)
{
	bool_t   bReturn = TRUE;
	uint16 u16Pos;

	for (u16Pos = 0; u16Pos < u16Len && bReturn == TRUE; u16Pos++)
	{
		/* Terminator - stop the loop */
		if (pu8Data[u16Pos] == '\0') break;
		/* Not printable - not a string */
		else if (pu8Data[u16Pos] < ' ' || pu8Data[u16Pos] > '~') bReturn=FALSE;
	}

	return bReturn;
}
#endif

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
