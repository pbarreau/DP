/****************************************************************************
 *
 * MODULE:             config.h
 *
 * COMPONENT:          $RCSfile: services.h,v $
 *
 * VERSION:            $Name: $
 *
 * REVISION:           $Revision: 1.5 $
 *
 * DATED:              $Date: 2008-10-15 09:30:44 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             IM
 *
 * DESCRIPTION
 *
 * CHANGE HISTORY:
 *
 * LAST MODIFIED BY:   $Author: ja $
 *                     $Modtime: $
 *
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
 * Copyright Jennic Ltd 2008. All rights reserved
 *
 ****************************************************************************/

#ifndef  SERVICES_H_INCLUDED
#define  SERVICES_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/* Services */
#define FIRST_SERVICE     10
#define SECOND_SERVICE    11
#define THIRD_SERVICE     12

/* Service masks */
#define FIRST_SERVICE_MASK  (1 << (FIRST_SERVICE-1))
#define SECOND_SERVICE_MASK (1 << (SECOND_SERVICE-1))
#define THIRD_SERVICE_MASK  (1 << (THIRD_SERVICE-1))


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* SERVICES_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
