/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelBridge/iModelBridgeErrorHandling.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <iModelBridge/iModelBridge.h>

#ifdef __IMODEL_BRIDGE_BUILD__
    #define IMODEL_BRIDGE_EXPORT EXPORT_ATTRIBUTE
#else
    #define IMODEL_BRIDGE_EXPORT IMPORT_ATTRIBUTE
#endif

#ifdef _WIN32
#define IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS  __try
#define IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS __except(iModelBridgeErrorHandling::ShouldProcessSEH())
#else
#define IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS  try
#define IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS  catch(...)
#endif

#define IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS_AND_LOG(CALLER_CLEAN_UP_CODE) IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS { CALLER_CLEAN_UP_CODE ; ::BentleyApi::Dgn::iModelBridgeErrorHandling::LogStackTrace(); }

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Helps with error-handling and reporting.
//!
//! Wrapping a block of code in the IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS/IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS macros 
//! will prevent a crash from terminating the process. The function will resume on the line following the catch.
//!
//! The IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS_AND_LOG macro both catches the exception and logs it. It also
//! calls the code that you supply in the event of a crash.
//! 
//! A function that uses the IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS and IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS macros
//! must not use C++ destructors. That is, the function must not construct C++ objects on the
//! stack or call functions that return objects by value.
//! 
//! Example:
//!
//! void Converter::SheetsConvertModelAndViewsWithExceptionHandling(ResolvedModelMapping const& v8mm, ViewFactory& nvvf)
//!     {
//!     IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS
//!         {
//!         SheetsConvertModelAndViews(v8mm, nvvf);
//!         }
//!     IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS_AND_LOG(ReportFailedModelConversion(v8mm);)
//!     }
//! 
//! This examples demonstrates both the user of the macros and the fact that the function that uses them must not
//! use C++ destructors. In this example, a helper function was written to stand between the caller and the
//! real function to be called. The helper function exists only as a place to put the error-handling macros.
//! 
//! @ingroup GROUP_iModelBridge
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct iModelBridgeErrorHandling
{
    IMODEL_BRIDGE_EXPORT static void Initialize();

    //! Helper function to get the current callstack, for logging exceptions.
    IMODEL_BRIDGE_EXPORT static Utf8String GetStackTraceDescription(size_t maxFrames, size_t nIgnoreFrames);

    //! Helper function to get the current callstack inside a structured exception block, for logging exceptions.
    IMODEL_BRIDGE_EXPORT static void GetStackTraceDescriptionFixed(char* buf, size_t bufSize);

    //! Helper function to log the current callstack.
    IMODEL_BRIDGE_EXPORT static void LogStackTrace();

    IMODEL_BRIDGE_EXPORT static int ShouldProcessSEH();
    };

END_BENTLEY_DGN_NAMESPACE
