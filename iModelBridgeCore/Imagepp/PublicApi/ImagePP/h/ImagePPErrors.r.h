//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/ImagePPErrors.r.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "ImagePPExceptionMessages.xliff.h"

BEGIN_IMAGEPP_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum HSTATUS
    {
    H_SUCCESS                   = 0L,  /* Indicates success */
    H_ERROR                     = 1L,  /* Undefined error */
    H_NOT_FOUND                 = 2L,  /* Desired Resource was not found */
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
    H_ERROR_BAD_NUMBER_POINTS   = 25L, /* Not enough number of point*/
    H_ERROR_MATRIX              = 26L, /* Matrix computation impossible*/
    };

/*
** Macro for Error Code determination
*/
#define HISERROR(status)    ((status) != H_SUCCESS)


enum ImageErrorCategories
    {
    IMAGEPP_STATUS_BASE         = 0x10000,
    IMAGEOP_ERROR_BASE          = 0x11000,
    COPYFROM_ERROR_BASE         = 0x12000,
    };

enum ImagePPStatus
    {
    IMAGEPP_STATUS_Success              = SUCCESS,
    IMAGEPP_STATUS_UnknownError         = ERROR,

    // IMAGEPP_STATUS_BASE:  General purpose status.
    IMAGEPP_STATUS_NoImplementation     = IMAGEPP_STATUS_BASE + 0x01,
    IMAGEPP_STATUS_OutOfMemory          = IMAGEPP_STATUS_BASE + 0x02,
        
    // IMAGEOP_ERROR_BASE: related to HRAImageOp
    IMAGEOP_STATUS_InvalidPixelType             = IMAGEOP_ERROR_BASE + 0x01, 
    IMAGEOP_STATUS_NoMorePixelType              = IMAGEOP_ERROR_BASE + 0x02,  
    IMAGEOP_STATUS_CannotNegociatePixelType     = IMAGEOP_ERROR_BASE + 0x03,  
    
    // COPYFROM_ERROR_BASE
    COPYFROM_STATUS_VoidRegion                     = COPYFROM_ERROR_BASE + 0x01,
    COPYFROM_STATUS_IncompatiblePixelTypeReplacer  = COPYFROM_ERROR_BASE + 0x02 
    };

END_IMAGEPP_NAMESPACE