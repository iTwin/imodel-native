/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnXf/DgnDbXf.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__
#include <BentleyApi/BentleyApi.h>
#include <Bentley/BeFileName.h>
#include <DgnXfLib/DgnXfLib.h>

#if defined (_WIN32)
    #if defined (__DGNDBTOXFDLL_BUILD__)
        #define FORCE_DGNDBTOXFDLL_EXPORT __declspec(dllexport) 
    #else
        #define FORCE_DGNDBTOXFDLL_EXPORT __declspec(dllimport) 
    #endif
#else
    #define FORCE_DGNDBTOXFDLL_EXPORT __attribute__((visibility ("default")))
#endif

BEGIN_BENTLEY_DGNXFLIB_NAMESPACE

//=======================================================================================
//! Functions to export data from a DgnDb project file.
// @bsiclass                                                    Sam.Wilson      10/13
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnDbToDgnXf
{
    //! Export the contents of the specified DgnDb project
    //! @param processor The object that will receive DgnXf messages 
    //! @param projectName The DgnDb file to read from
    //! @return non-zero error status if the export failed
    //! @remarks The caller must have already initialized a DgnPlatformLib::Host before calling this function.
    FORCE_DGNDBTOXFDLL_EXPORT static DgnXfLib::DgnXfStatus ProcessMessages (DgnXfLib::IMessageProcessor& processor, BeFileNameCR projectName);

#ifdef BENTLEY_WIN32 // This api can be used only by a Windows-based 

    //! Export the contents of the specified DgnDb project. This function initializes and uses a temporary DgnDb DgnPlatformLib::Host for the duration of the export, so that
    //! the caller does not have to do that.
    //! @param processor The object that will receive DgnXf messages 
    //! @param projectName The DgnDb file to read from
    //! @return non-zero error status if the export failed
    FORCE_DGNDBTOXFDLL_EXPORT static DgnXfLib::DgnXfStatus ProcessMessagesWithTemporaryHost (DgnXfLib::IMessageProcessor& processor, BeFileNameCR projectName);

#endif
};

END_BENTLEY_DGNXFLIB_NAMESPACE