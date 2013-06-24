/****************************************************************************/
/*!
 *\MODULE              JN-AN-1072 Jenie Wireless Keyboard
 *
 *\COMPONENT           $RCSfile: Display.c,v $
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
 *\DESCRIPTION         Display - implementation.
 *
 * This file handles the interface with the display.
 *
 * vDisplay_Init() is first called to initialise the display.
 *
 * vDisplay_Rx() is called whenever data is received over the network.
 * Received key sequences are passed into u8PS2protocol() for decoding,
 * sequences that decode to ASCII text are then added to the display.
 * Finally acknowledgements are transmitted back to the router/keyboard node.
 *
 * vDisplay_Char() adds a single ASCII character to the display.
 *
 * vDisplay_Scroll() scrolls the display up by a line.
 */
/* CHANGE HISTORY
 *
 * $Log: Display.c,v $
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

#include "jendefs.h"
#include "LcdDriver.h"
#include "LcdFont.h"
#include "JennicLogo.h"
#include "printf.h"

/* Wireless display specific */
#include "Network.h"
#include "Display.h"
#include "PS2protocol.h"

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/* Virtual display driver variables */
typedef struct
{
    uint8   u8Row;
    uint8   u8Col;
} tsVdu;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE	tsVdu    sVdu;
PRIVATE uint8	 u8Sequence;

/****************************************************************************/
/***        Local functions                                               ***/
/****************************************************************************/

PRIVATE void vDisplay_Char    (uint8 u8Char);
PRIVATE void vDisplay_Scroll  (void);

/****************************************************************************
 *
 * NAME: vDisplay_Init
 */
/*!
 *\DESCRIPTION Initialises display.
 *
 ****************************************************************************/
PUBLIC void vDisplay_Init(void)
{
	/* Initialise the LCD panel */
    vLcdReset(3, 3); /* for UC1601 type displays */
    vLcdClear();

	/* Write the Jennic logo */
    vLcdWriteBitmap(&sJennicLogo, 0, 0);
    vLcdWriteText("Jenie Wireless Keyboard", 4, 0);
    vLcdRefreshAll();

	/* Initialise the virtual display driver, only use half	*/
	/* the screen to display keyboard codes.				*/
	sVdu.u8Row = 5;
	sVdu.u8Col = 0;

	/* Initialise data packet */
	u8Sequence = 0x80;	/* Sequence */

}

/****************************************************************************
 *
 * NAME: vDisplay_Rx
 */
/*!
 *\DESCRIPTION Process and display incoming data.
 *
 ****************************************************************************/
PUBLIC void vDisplay_Rx(uint16 u16Length, uint8 *pu8DataRx)
{
    uint8 u8Key;
	uint8  au8Data[3];

	#if NETWORK_DEBUG
    	if (bNetwork_UartUp()) vPrintf("    Display Rx(%c, %x, %x) {%x}\n", pu8DataRx[0], pu8DataRx[1], pu8DataRx[2], u8Sequence);
    #endif

	/* Is this an keypress message ? */
	if (pu8DataRx[0] == 'K')
	{
		/* Are the network services up ? */
		if (bNetwork_Services_Up())
		{
			/* Does the sequence number match ? */
			if (pu8DataRx[1] == u8Sequence)
			{
				/* New packet, convert from PS2 sequence to ASCII code */
				u8Key = u8PS2protocol(pu8DataRx[2]);
				/* Printable character ? */
				if (u8Key)
				{
					#if NETWORK_DEBUG
					    if (bNetwork_UartUp()) vPrintf("    Display KEY(%x, %c)\n", pu8DataRx[2], (char) u8Key);
					#endif
					/* Add to display */
					vDisplay_Char(u8Key);
				}
			}
			/* Build packet - always ack to avoid resends */
			au8Data[0] = 'A';
			au8Data[1] = pu8DataRx[1];
			au8Data[2] = pu8DataRx[2];
			#if NETWORK_DEBUG
		    	if (bNetwork_UartUp()) vPrintf("    Display Tx(%c, %x, %x)\n", au8Data[0], au8Data[1], au8Data[2]);
		    #endif
			/* Send packet - always ack or the keyboard won't stop sending */
			(void) eNetwork_Service_Tx(NETWORK_SERVICE, 3, au8Data);
			/* Update sequence for next packet */
			u8Sequence++;
		}
	}
}

/****************************************************************************
 *
 * NAME: vDisplay_Char
 */
/*!
 *\DESCRIPTION Add character to VDU.
 * A virtual VDU manager.  Provides scrolling, line-wrap and character/font
 * translation.  The top four lines ofd the display do not scroll, the bottom
 * four do.
 *
 ****************************************************************************/
PRIVATE void vDisplay_Char(uint8 u8Char)
{
    char acText[2];
    char *pu8CharMap;
    uint8 u8CharWidth;

	if (u8Char == '\n')
	{
		vDisplay_Scroll();
	}
	else if (u8Char == '\r')
	{
		sVdu.u8Col = 0;
		vDisplay_Scroll();
	}
	else if ((u8Char == ' ')
         || ((u8Char >= '0') && (u8Char <= '9'))
         || ((u8Char >= 'a') && (u8Char <= 'z'))
         || ((u8Char >= 'A') && (u8Char <= 'Z')))
    {
        acText[0] = u8Char;
        acText[1] = 0;

        /* Get charcter width */
        pu8CharMap = (char *) pu8LcdFontGetChar(u8Char);
        u8CharWidth = *pu8CharMap;

        /* Check if character will fit on current row, change row if not */
        if ((sVdu.u8Col + u8CharWidth) > 127)
        {
            sVdu.u8Col = 0;
			vDisplay_Scroll();
        }

        /* Write character and undate column position */
        vLcdWriteText(acText, sVdu.u8Row, sVdu.u8Col);
        sVdu.u8Col = sVdu.u8Col + u8CharWidth + 1;
    }

	/* Copy LCD shadow memory to the panel - takes approx 4.5 mS */
	vLcdRefreshAll();
}

/****************************************************************************
 *
 * NAME: vScroll
 */
/*!
 *\DESCRIPTION Handles VDU scrolling.
 *
 ****************************************************************************/
PRIVATE void vDisplay_Scroll(void)
{
	if (sVdu.u8Row == 7)
	{
		/* Scroll screen up from row 2, ie rows 0 and 1 don't scroll */
		vLcdScrollUp(4);
	}
	else
	{
		sVdu.u8Row = (sVdu.u8Row + 1) % 8;
    }
}
