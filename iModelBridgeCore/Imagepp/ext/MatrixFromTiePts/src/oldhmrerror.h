/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/oldhmrerror.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*====================================================================
**  oldhmrerror.h
**
**                      This file contains the declaration for the standard error.
**
**  FILES
**              oldhmrerror.h
**
**====================================================================*/
#ifndef __OLDHMRERROR_H__
#define __OLDHMRERROR_H__

#include "oldhmrtypes.h"


typedef long HSTATUS;

#define H_SUCCESS               0L
#define H_ERROR                 1L
#define H_FILENOTFOUND          2L
#define H_PERMISSIONDENIED      3L
#define H_FILEEXIST             4L
#define H_FILEREADONLY          5L
#define H_FILENOTEXIST          6L
#define H_NOTENOUGHMEMORY       7L
#define H_NOLEFTSPACEDEV        8L
#define H_OUTOFRANGE            9L
#define H_ERROR_CANT_UNDO      10L
#define H_FILENOTCREATED       11L
#define H_FILENOTSUPPORTED         12L
#define H_CORRUPT              13L    /* Indicates the coruption of an object or struct */
#define H_INVALID              14L    /* Indicates an invalid object/parameter was given */
#define H_ENDOFFILE            15L    /* Indicates that end of file was encounter */
#define H_USERABORT            16L
#define H_DUPLICATEENTRY       17L
#define H_FILTERNONE           18L    /* Filter is set to None */
#define H_FILTERALL            19L    /* Filter is set to All */
#define H_NOACTIVEIMAGE        20L    /* No active image */
#define H_CANNOTLOCKFILE       21L
#define H_WRONG_IMAGE          22L
#define H_AREA_OVERFLOW            23L    /* Maximum area overflow */
#define H_NO_OPEN_IMAGE            24L    /* No image opened */

/***********************************************************
** Error code for mosaic
*************************************************************/
#define H_BADINDEX             100L
#define H_MOSAIC_NOT_LOCKED    101L
#define H_MOSAIC_ERROR_SAVE    102L
#define H_MOSAIC_NOT_COMPLETE  103L


/***********************************************************
** Error code for copy project operation
*************************************************************/
#define H_CPYPROJECT_PRJINVALID            200L
#define H_CPYPROJECT_NOT_COMPLETE          201L
#define H_CPYPROJECT_ARCHPRJ_APP_NOTFOUND  202L


/*
** Macro for Error Code
*/
#define HDEF(status, error) HSTATUS status = error
#define HSET(status, error) (status = error)
#define HGET(status)        (status)
#define HRET(status)        return (status)
#define HRET_SUCCESS()      return (H_SUCCESS)
#define HISERROR(status)    ((status) != H_SUCCESS)

#endif /*__OLDHMRERROR_H__*/

