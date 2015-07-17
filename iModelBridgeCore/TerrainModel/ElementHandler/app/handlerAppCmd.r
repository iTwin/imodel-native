/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/app/handlerAppCmd.r $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Mstn\MdlApi\rscdefs.r.h>
#include <Mstn\MdlApi\cmdclass.r.h>

#pragma suppressREQCmds

/*-----------------------------------------------------------------------
 Setup for native code only MDL app
-----------------------------------------------------------------------*/
#define  DLLAPP_PRIMARY     1

DllMdlApp   DLLAPP_PRIMARY =
    {
    L"TerrainModelHandlerApp", L"TerrainModelHandlerApp"    // taskid, dllName
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   Handler IDs.                                                        |
|   This application is included in the MS_HANDLER_LOAD configuration   |
|   variable.  It will be loaded "on demand" if handlers with IDs from  |
|   the table below are required.                                       |
|                                                                       |
|   MS_HANDLER_LOAD > exampleHandler                                    |
|                                                                       |
+----------------------------------------------------------------------*/
#define XATTRIBUTEID_DTMElement 22764
#define XATTRIBUTEID_MRDTMElement 22775
//#include <TerrainModel\TerrainModelHandler.h>
#include <DgnPlatform\TerrainModel\TMPersistentAppIDs.h>

ResourceHandlerIdTable 1 =
    {
    /* majorID				, minor ID */        
    /* Element Handler Ids */
        { XATTRIBUTEID_DTMElement, ELEMENTHANDLER_DTMELEMENT },
        { XATTRIBUTEID_DTMElement, ELEMENTHANDLER_DTMDATA },        
        { XATTRIBUTEID_DTMElement, ELEMENTHANDLER_DTMOVERRIDESYMBOLOGY },
        { XATTRIBUTEID_MRDTMElement, ELEMENTHANDLER_MRDTMELEMENT },

    /* XAttribute Handler Ids */
        { XATTRIBUTEID_DTMElement, XATTRIBUTES_SUBID_DTM_REFERENCE },
        { XATTRIBUTEID_DTMElement, XATTRIBUTES_SUBID_DTM_OVERRIDEDISPLAYINFO },
        { XATTRIBUTEID_DTMElement, XATTRIBUTES_SUBID_DTM_OVERRIDEDISPLAYREF },
    };
    