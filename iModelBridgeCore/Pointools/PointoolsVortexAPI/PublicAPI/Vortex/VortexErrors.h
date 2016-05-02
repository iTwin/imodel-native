/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Vortex/VortexErrors.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#define PTV_SUCCESS						1
#define PTV_NO_ERROR					PTV_SUCCESS

#define PTV_IS_ERROR( a )	((a) < 0)
#define PTV_IS_WARNING( a ) ((a) > 1)

// File Errors
#define PTV_FILE_NOT_EXIST				-501
#define PTV_FILE_NOT_ACCESSIBLE			-502
#define PTV_FILE_WRONG_TYPE				-503
#define PTV_FILE_COM_ERROR				-504
#define PTV_FILE_USER_CANCELLED			 505
#define PTV_FILE_ALREADY_OPENED			-506
#define PTV_FILE_NOTHING_TO_WRITE		 507
#define PTV_FILE_WRITE_FAILURE			-508
#define PTV_FILE_READ_FAILURE			-509
#define PTV_FILE_FAILED_TO_CREATE		-510
#define PTV_FILE_INVALID_POD			-511
#define PTV_FILE_VERSION_NOT_HANDLED	-512

// Generic Errors
#define PTV_UNKNOWN_ERROR				-600
#define PTV_INVALID_PARAMETER			-601
#define PTV_VALUE_OUT_OF_RANGE			-602
#define PTV_INVALID_OPTION				-603
#define PTV_INVALID_VALUE_FOR_PARAMETER	-604
#define	PTV_INVALID_HANDLE				-605
#define PTV_VOID_POINTER				-606
#define PTV_NOT_INITIALIZED				-607
#define PTV_NOT_IMPLEMENTED_IN_VERSION	-608
#define PTV_OUT_OF_MEMORY				-609
#define PTV_MIN_OPENGL_VERSION_NA		-610

// Licensing
#define PTV_LICENSE_EXPIRY				-701
#define PTV_LICENSE_MODULE_ERROR		-702
#define PTV_LICENSE_CORRUPT				-703
#define PTV_NO_LICENSE_FOR_FEATURE		-704
#define PTV_PRODUCT_LICENSE_NA			-705

// Viewports
#define PTV_MAXIMUM_VIEWPORTS_USED		-801

// IO Block errors
#define PTV_INVALID_BLOCK_VERSION		-901

// Metadata
#define PTV_METATAG_NOT_FOUND			-1001
#define PTV_METATAG_EMPTY				 1002

//rendering


// Bentley specific client-server error codes

#define PTW_ERROR_OK					0001
#define PTW_ERROR_BENTLEYDATAEMPTY		1003
#define PTW_ERROR_PWERROR				1004      
#define PTW_ERROR_GETDATAERROR			1005
#define PTW_ERROR_FILEOUTDATED			1006 
