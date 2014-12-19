//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/ImagePPErrors.r.h $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "ImagePPExceptionMessages.xliff.h"

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum HSTATUS
    {
    H_SUCCESS                   = 0L,  /* Indicates success */
    H_ERROR                     = 1L,  /* Undefined error */
    H_NOT_FOUND                 = 2L,  /* Desired ressource was not found */
    H_PERMISSION_DENIED         = 3L,  /* No permission to access resource */
    H_ALREADY_EXISTS            = 4L,  /* Resource already exists */
    H_TIME_OUT                  = 5L,  /* Time out */
    H_BUSY                      = 6L,  /* Resource is busy/in use */
    H_NOT_ENOUGH_MEMORY         = 7L,  /* Not enough memory */
    H_NO_SPACE_LEFT             = 8L,  /* No space left on resource */
    H_OUT_OF_RANGE              = 9L,  /* Value is out of the valid range of values */
    H_UNREACHABLE               = 10L, /* Resource cannot be reached/accessed */
    H_NOT_SUPPORTED             = 11L, /* Feature is not supported */
    H_INVALID                   = 12L, /* Invalid operation or value */
    H_ABORT                     = 13L, /* Abortion of operation user or automatic */
    H_IOERROR                   = 14L, /* General undefined IO error */
    H_READ_ONLY_ACCESS          = 15L, /* Only ReadOnly access is available */
    H_READ_ERROR                = 16L,
    H_WRITE_ERROR               = 17L,
    H_DATA_CORRUPTED            = 18L, /* The data s corrupted... */
    H_DATA_NOT_AVAILABLE        = 19L, /* the requested data is not available */
    H_INTERNET_CONNECTION_LOST  = 20L, /* Internet connection error */
    H_INTERNET_CANNOT_CONNECT   = 21L,
    H_INTERNET_UNKNOWN_ERROR    = 22L,
    H_EMPTY_TRANSACTION         = 23L, /* used by HRAStoredRaster */
    H_PROXY_PERMISSION_DENIED   = 24L,
    };

/*
** Macro for Error Code determination
*/
#define HISERROR(status)    ((status) != H_SUCCESS)

//--------------------------------------------------------------------------------------
// How To Add a New Exception
//
// This document describes how to add a new exception in Imagepp.
//
// Steps :
//
// 1 – Add a new exception id in the enumeration ExceptionID located in ImagePPExceptionMessages.xliff.h.
//     Beware to add the new exception at the end of one exception group, to ensure
//     that the exception ID for a particular exception remains the same for the
//     lifespan of the library.
//
// 2 – Update the exception count in ImagePPErrors.r.h for the specific exception group.
//
// 3 – Add an exception message in the string table of LocalizedStrings.resx and save
//     the string table.
//
// 4 - Rebuild ImageppUtl.
//
// 5 - Add a new test case in HFCTestedExceptions.h.
//
// 6 - If the new exception is triggered by a specific access mode, the exception ID should be added
//     to s_ippInvalidAccessModeExceptionTable located rasterlib/rastercore/HIEPriv.cpp
//
//
// Creating a new exception class :
//
// It is possible that a new exception class is needed if some specific information
// about the exception has to be saved for further use during the exception catch.
//
// Steps :
//
// 1 - Create a new information structure in HFCException.h that will hold the specific
//     exception information.
//
// 2 – Create the new exception class from an existent exception class. The function in
//     the new class should be the same as the existent exception, except for the
//     constructor, which should have as parameters the specific information of the new
//     exception.
//
// 3 – In the implementation of the functions, replace the old information structure by
//     the new one and pass in the order appearing in the new exception message the
//     different information parameter to the macro FORMAT_EXCEPTION_MSG in the function
//     FormatExceptionMessage(WString& pio_rMessage).
//--------------------------------------------------------------------------------------

enum ExceptionGroup
    {
    HFC = 0,
    HCD,
    HFS,
    HGF,
    HPA,
    HPS,
    HRFII, //Internet Imaging
    HRP,
    HRF,
    HCS,
    HCP,
    HVE,
    //The enumerator value below must always be the last
    NB_OF_EXCEPTION_GROUPS
    };

#define HFC_EXCEPTION_COUNT       55 //23
#define HCD_EXCEPTION_COUNT        6 //4
#define HFS_EXCEPTION_COUNT        9
#define HGF_EXCEPTION_COUNT        3
#define HPA_EXCEPTION_COUNT        7
#define HPS_EXCEPTION_COUNT       30
#define HRFII_EXCEPTION_COUNT     11
#define HRP_EXCEPTION_COUNT        4
#define HRF_EXCEPTION_COUNT       78
#define HCS_EXCEPTION_COUNT        8
#define HCP_EXCEPTION_COUNT        1
#define HVE_EXCEPTION_COUNT        6

//Fixed number of digits uses when formatting different message ID
//append at the end of the base key string.before querying the managed
//resource
#define IPP_ERR_MSG_NB_KEY_DIGITS       6

