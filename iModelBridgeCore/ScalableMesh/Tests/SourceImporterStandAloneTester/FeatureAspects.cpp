/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/SourceImporterStandAloneTester/FeatureAspects.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"

//#include    <UStationInternal.h>
#include    <windows.h>
#define winNT
#include    "Bentley\Bentley.h"
#include    "DgnPlatform\DgnPlatform.h"
#include    "DgnPlatform\Tools\ConfigurationManager.h"


USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM

//BEGIN_EXTERN_C

/*----------------------------------------------------------------------+
|   Private variables                                                   |
+----------------------------------------------------------------------*/
static bool         s_dllsLoaded;
static int          s_dllsCount;

namespace Bentley
    {
    struct StringList{};

    typedef StringList* StringListP;
    }


/*----------------------------------------------------------------------+
| Typedefs for signatures of functions in DLL                           |
+----------------------------------------------------------------------*/
typedef StatusInt   (*__PF__LoadFeatureAspects) (void);
typedef StatusInt   (*__PF__UnloadFeatureAspects) (void);
typedef bool        (*__PF__IsFeatureAspectAllowed)
(
int const           featureId,
bool *              foundP
);
typedef int         (*__PF__GetFeatureIdByName)
(
WCharCP             featureName,
bool    *           foundP
);

/*----------------------------------------------------------------------+
| Pointers to runtime code loaded in the DLL                            |
+----------------------------------------------------------------------*/
typedef struct interfacestruct
    {
    __PF__LoadFeatureAspects            LoadFeatureAspects;
    __PF__UnloadFeatureAspects          UnloadFeatureAspects;
    __PF__IsFeatureAspectAllowed        IsFeatureAspectAllowed;
    __PF__GetFeatureIdByName            GetFeatureIdByName;
    } InterfaceStruct;

#define MAX_FA_DLLS     10
static InterfaceStruct  interfaceStructs[MAX_FA_DLLS];

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanEast         08/07
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   featureAspects_loadDLL
(
WCharCP         fileNameP,
int             index
)
    {
    StatusInt   status = SUCCESS;
    HMODULE     hDLL = 0;

    /* Try to get the module handle of the loaded .dll first, then if not loaded, load it */
    hDLL = GetModuleHandleW ( fileNameP );
    if (0 == hDLL)
        hDLL = LoadLibraryW ( fileNameP );
    if (0 == hDLL)
        return ERROR;

    if ( 0 == (interfaceStructs[index].LoadFeatureAspects = (__PF__LoadFeatureAspects) GetProcAddress( hDLL, "LoadFeatureAspects" )))
        return ERROR;

    if ( 0 == (interfaceStructs[index].UnloadFeatureAspects = (__PF__UnloadFeatureAspects) GetProcAddress( hDLL, "UnloadFeatureAspects" )))
        return ERROR;

    if ( 0 == (interfaceStructs[index].IsFeatureAspectAllowed = (__PF__IsFeatureAspectAllowed) GetProcAddress( hDLL, "IsFeatureAspectAllowed" )))
        return ERROR;

    if ( 0 == (interfaceStructs[index].GetFeatureIdByName = (__PF__GetFeatureIdByName) GetProcAddress( hDLL, "GetFeatureIdByName" )))
        return ERROR;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanEast         06/13
+---------------+---------------+---------------+---------------+---------------+------*/
SemiPrivate StatusInt   featureAspects_initialize ()
    {
    if (!s_dllsLoaded)
        {
        memset (&interfaceStructs, 0, sizeof interfaceStructs);

        /* Search in the libraries (DLLs) for the Feature Aspect */
        WString libEnv;
        if (SUCCESS == ConfigurationManager::GetVariable (libEnv, L"_USTN_FEATUREASPECTSDLLLIST"))
            {
            StringListP         libList = NULL;

            /* get list of libraries */
            int     numLibs = mdlFileList_fromString (&libList, libEnv.c_str());

            s_dllsCount = 0;

            /* For each library, try to load it */
            for (int iLib=0; iLib < numLibs; iLib++)
                {
                StatusInt       status;
                WCharCP         libFileNameP;

                mdlStringList_getMember (&libFileNameP, NULL, libList, iLib);

                if (SUCCESS == (status = featureAspects_loadDLL (libFileNameP, iLib)))
                    {
                    (*interfaceStructs[s_dllsCount].LoadFeatureAspects)();
                    s_dllsCount++;

                    // Only MAX_FA_DLLS in the array
                    if (s_dllsCount == MAX_FA_DLLS)
                        {
                        printf ("Too many DLLs listed for _USTN_FEATUREASPECTSDLLLIST\n");
                        break;
                        }
                    }
                else
                    {
                    printf ("Load for '%ls' listed in _USTN_FEATUREASPECTSDLLLIST failed.  Continuing...\n", libFileNameP);
                    }
                }  // end for each file

            mdlStringList_destroy (libList);
            }

        s_dllsLoaded = true;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanEast         08/07
+---------------+---------------+---------------+---------------+---------------+------*/
SemiPrivate bool        featureAspects_isFeatureAllowed
(
int const featureId
)
    {
    if (!s_dllsLoaded)
        featureAspects_initialize();

    /* Call the IsFeatureAspectAllowed from the DLLs until the Feature itself is found */
    bool        isAllowed = false;
    bool        isFound = false;

    // Call the DLLs IsFeatureAspectAllowed function
    for (int iLib=0; iLib < s_dllsCount && !isFound; iLib++)
        {
        isAllowed = (*interfaceStructs[iLib].IsFeatureAspectAllowed)(featureId, &isFound);
        }

    return isAllowed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanEast         08/07
+---------------+---------------+---------------+---------------+---------------+------*/
SemiPrivate StatusInt featureAspects_shutdown
(
void
)
    {
    if (s_dllsLoaded)
        {
        int     iLib;

        // Call the DLLs IsFeatureAspectAllowed function
        for (iLib=0; iLib < s_dllsCount; iLib++)
            {
            (*interfaceStructs[iLib].UnloadFeatureAspects)();
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanEast         10/11
+---------------+---------------+---------------+---------------+---------------+------*/
SemiPrivate int     featureAspects_getFeatureIdByName
(
WCharCP         featureName
)
    {
    int         featureId = 0;
    
    if (s_dllsLoaded)
        {
        int     iLib;
        bool    isFound = false;

        // Call the DLLs GetFeatureIdByName function
        for (iLib=0; iLib < s_dllsCount && !isFound; iLib++)
            {
            featureId = (*interfaceStructs[iLib].GetFeatureIdByName)(featureName, &isFound);
            }
        }

    return featureId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanEast         08/07
+---------------+---------------+---------------+---------------+---------------+------*/
/*MSCORE_EXPORT*/ bool    mdlSystem_isFeatureAspectAllowed
(
int const   featureId
)
    {
    if (0 == featureId)
        return true;
        
    return featureAspects_isFeatureAllowed (featureId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanEast         10/07
+---------------+---------------+---------------+---------------+---------------+------*/
/*MSCORE_EXPORT*/ void     mdlSystem_commandNotSupported
(
void
)
    {
    // "Command is not supported in this product"
    //mdlOutput_errorNum (MsgList_ERRORS, 685);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanEast         10/07
+---------------+---------------+---------------+---------------+---------------+------*/
/*MSCORE_EXPORT*/ void     mdlSystem_functionNotSupported
(
void
)
    {
    // "Function is not supported in this product"
    //mdlOutput_errorNum (MsgList_ERRORS, 688);
    }

//END_EXTERN_C
