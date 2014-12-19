/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/h/ImagePPExceptionMessages.xliff.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <BeSQLite/L10N.h>

BENTLEY_TRANSLATABLE_STRINGS2_START(ExceptionID,ImagePPExceptionMessage,ImagePPExceptions)
    {
    EXCEPTION_RANGE=1000,                                       //==NoMessage==

    HFC_BASE=2,                                                 //==NoMessage==
    HFC_RANGE=EXCEPTION_RANGE,                                  //==NoMessage==
    HCD_BASE=(HFC_BASE + HFC_RANGE),                            //==NoMessage==
    HCD_RANGE=EXCEPTION_RANGE,                                  //==NoMessage==
    HFS_BASE=(HCD_BASE + HCD_RANGE),                            //==NoMessage==
    HFS_RANGE=EXCEPTION_RANGE,                                  //==NoMessage==
    HGF_BASE=(HFS_BASE + HFS_RANGE),                            //==NoMessage==
    HGF_RANGE=EXCEPTION_RANGE,                                  //==NoMessage==
    HPA_BASE=(HGF_BASE + HGF_RANGE),                            //==NoMessage==
    HPA_RANGE=EXCEPTION_RANGE,                                  //==NoMessage==
    HPS_BASE=(HPA_BASE + HPA_RANGE),                            //==NoMessage==
    HPS_RANGE=EXCEPTION_RANGE,                                  //==NoMessage==
    HRFII_BASE=(HPS_BASE + HPS_RANGE),                          //==NoMessage==
    HRFII_RANGE=EXCEPTION_RANGE,                                //==NoMessage==
    HRP_BASE=(HRFII_BASE + HRFII_RANGE),                        //==NoMessage==
    HRP_RANGE=EXCEPTION_RANGE,                                  //==NoMessage==
    HRF_BASE=(HRP_BASE + HRP_RANGE),                            //==NoMessage==
    HRF_RANGE=EXCEPTION_RANGE,                                  //==NoMessage==
    HCS_BASE=(HRF_BASE + HRF_RANGE),                            //==NoMessage==
    HCS_RANGE=EXCEPTION_RANGE,                                  //==NoMessage==
    HCP_BASE=(HCS_BASE + HCS_RANGE),                            //==NoMessage==
    HCP_RANGE=EXCEPTION_RANGE,                                  //==NoMessage==
    HVE_BASE=(HCP_BASE + HCP_RANGE),                            //==NoMessage==
    HVE_RANGE=EXCEPTION_RANGE,                                  //==NoMessage==

    NO_EXCEPTION=0,                                             //==NoMessage==
    UNKNOWN_EXCEPTION=1,                                        // =="Unknown exception"==
    //HFC Exceptions
    HFC_EXCEPTION = HFC_BASE,                                   // =="Foundation class exception"==
    HFC_MEMORY_EXCEPTION,                                       // =="Memory exception"==
    HFC_OUT_OF_MEMORY_EXCEPTION,                                // =="Out of memory"==
    HFC_FILE_EXCEPTION,                                         // =="File exception"==
    HFC_CANNOT_LOCK_FILE_EXCEPTION,                             // =="Cannot lock the file"==
    HFC_LOCK_VIOLATION_FILE_EXCEPTION,                          // =="File locking violation"==
    HFC_CORRUPTED_FILE_EXCEPTION,                               // =="Corrupted file"==
    HFC_FILE_NOT_FOUND_EXCEPTION,                               // =="File not found"==
    HFC_FILE_EXIST_EXCEPTION,                                   // =="The file already exists"==
    HFC_FILE_READ_ONLY_EXCEPTION,                               // =="The file is in read-only mode"==
    HFC_FILE_NOT_CREATED_EXCEPTION,                             // =="The file cannot be created"==
    HFC_FILE_NOT_SUPPORTED_EXCEPTION,                           // =="The file type is not supported"==
    HFC_FILE_OUT_OF_RANGE_EXCEPTION,                            // =="Out of range exception"==
    HFC_FILE_PERMISSION_DENIED_EXCEPTION,                       // =="File permission denied"==
    HFC_DLL_NOT_FOUND_EXCEPTION,                                // =="Cannot find the DLL %s"==
    HFC_DEVICE_EXCEPTION,                                       // =="An exception occurred with the following device : %s"==
    HFC_NO_DISK_SPACE_LEFT_EXCEPTION,                           // =="There is not enough disk space left"==
    HFC_DEVICE_ABORT_EXCEPTION,                                 // =="The following device aborted : %s"==
    HFC_INTERNET_CONNECTION_EXCEPTION,                          // =="Internet connection exception (device : %s, error code : %i)"==
    HFC_OBJECT_NOT_IN_FACTORY_EXCEPTION,                        // =="The object is not in the factory"==
    HFC_ERROR_CODE_EXCEPTION,                                   // =="An error code triggered an exception"==
    HFC_CANNOT_CREATE_SYNCHRO_OBJ_EXCEPTION,                    // =="Cannot create a synchronization object (object : %i)"==
    HFC_INVALID_FILE_NAME_EXCEPTION,                            // =="The file name is not valid"==
    HFC_SHARING_VIOLATION_EXCEPTION,                            // =="The process cannot access the file because it is being used by another process"==
    HFC_CANNOT_UNLOCK_FILE_EXCEPTION,                           // =="Cannot unlock the file"==
    HFC_INVALID_HTTP_REQUEST_STR_EXCEPTION,                     // =="Invalid HTTP request string"==
    HFC_CANNOT_OPEN_FILE_EXCEPTION,                             // =="Cannot open file"==
    HFC_CORRUPTED_DLL_EXCEPTION,                                // =="The DLL %s is corrupted"==
    HFC_CANNOT_CONNECT_TO_DB_EXCEPTION,                         // =="Cannot connect to the database"==
    HFC_INVALID_URL_FOR_SISTER_FILE_EXCEPTION,                  // =="Cannot create a sister file for that URL"==
    HFC_DIRECTORY_READ_ONLY_EXCEPTION,                          // =="%s is in read-only mode"==
    HFC_SISTER_FILE_NOT_CREATED_EXCEPTION,                      // =="Cannot open the following sister file : %s"==
    HFC_DIRECTORY_NOT_CREATED_EXCEPTION,                        // =="Cannot create the following directory : %s"==
    HFC_CANNOT_CLOSE_FILE_EXCEPTION,                            // =="Cannot close the file"==
    HFC_WRITE_FAULT_EXCEPTION,                                  // =="A write fault occurred"==
    HFC_READ_FAULT_EXCEPTION,                                   // =="A read fault occurred"==
    HFC_FILE_IO_ERROR_EXCEPTION,                                // =="A file IO exception occurred"==
    HFC_LOGIN_INFORMATION_NOT_AVAILABLE_EXCEPTION,              // =="The login information is not available"==
    HFC_FILE_NOT_SUPPORTED_IN_WRITE_MODE,                       // =="This file is not supported in write mode"==

    //New HFC exception must be added above this line.
    //HFC_EXCEPTION_COUNT must be incremented by one for each newly added exception.    
    HFC_SEPARATOR_ID,                                           //==NoMessage==

    //HCD Exceptions
    HCD_EXCEPTION = HCD_BASE,                                   // =="A codec exception occurred"==
    HCD_IJL_ERROR_EXCEPTION,                                    // =="An error occurred in the JPEG library"==
    HCD_IJL_CHANNEL_COUNT_NOT_SUPPORTED_EXCEPTION,              // =="The file's channel count is not supported by the JPEG library"==
    HCD_IJG_LIBRARY_ERROR_EXCEPTION,                            // =="A JPEG error occurred"==
    HCD_CORRUPTED_PACKBITS_DATA_EXCEPTION,                      // =="The PackBits encoded data are corrupted"==
    HCD_CORRUPTED_DATA_EXCEPTION,                               // =="The encoded data are corrupted"==

    //New HCD exception must be added above this line
    //HCD_EXCEPTION_COUNT must be incremented by one for each newly added exception.
    HCD_SEPARATOR_ID,                                           //==NoMessage==

    //HFS Exceptions
    HFS_EXCEPTION = HFS_BASE,                                   // =="A file system exception occurred"==
    HFS_INVALID_PATH_EXCEPTION,                                 // =="%s is an invalid file server path"==
    HFS_IBP_INVALID_RESPONSE_EXCEPTION,                         // =="Invalid IBP response"==
    HFS_IBP_ERROR_EXCEPTION,                                    // =="The following IBP error occurred : %s"==
    HFS_URL_SCHEME_NOT_SUPPORTED_EXCEPTION,                     // =="The scheme of the following URL is not supported by the file server : %s"==
    HFS_IBP_PROTOCOL_NOT_SUPPORTED_EXCEPTION,                   // =="The requested Internet browsing protocol requested is not supported"==
    HFS_INVALID_DIRECTORY_PATH_EXCEPTION,                       // =="%s is an invalid directory path on the file server"==
    HFS_CANNOT_START_NET_RESOURCE_ENUM_EXCEPTION,               // =="Cannot start the enumeration of network resources"==
    HFS_CANNOT_ENUM_NEXT_NET_RESOURCE_EXCEPTION,                // =="Cannot enumerate the next network resource"==

    //New HFS exception must be added above this line
    //HFS_EXCEPTION_COUNT must be incremented by one for each newly added exception.
    HFS_SEPARATOR_ID,                                           //==NoMessage==
    
    HGF_REPROJECTION_EXCEPTION = HGF_BASE,                      // =="A reprojection exception occurred"==
    HGF_MZ_G_COORD_EXCEPTION,                                   // =="A GeoCoord exception occurred"==
    HGF_DOMAIN_EXCEPTION,                                       // =="The requested is outside the grid model's domain"==

    //New HGF exception must be added above this line
    //HGF_EXCEPTION_COUNT must be incremented by one for each newly added exception.
    HGF_SEPARATOR_ID,                                           //==NoMessage==

    //HPA Exceptions
    HPA_EXCEPTION = HPA_BASE,                                   // =="The parser found a syntax error (line : %lu, column : %lu)"==
    HPA_NO_TOKEN_EXCEPTION,                                     // =="The parser cannot find a token (line : %lu, column : %lu)"==
    HPA_RECURSIVE_INCLUSION_EXCEPTION,                          // =="The file or stream currently being parsed has already been included (line : %lu, column : %lu)"==
    HPA_GENERIC_EXCEPTION,                                      // =="The following generic parser exception occurred : %s"==
    HPA_CANNOT_FIND_PRODUCTION_EXCEPTION,                       // =="The parser cannot find a valid production (line : %lu, column : %lu)"==
    HPA_CANNOT_RESOLVE_START_RULE_EXCEPTION,                    // =="The parser cannot resolve the starting rule"==
    HPA_NODE_LEFT_TO_PARSE_EXCEPTION,                           // =="Some unparsed nodes remain after finding the starting rule  (line : %lu, column : %lu)"==

    //New HPA exception must be added above this line
    //HPA_EXCEPTION_COUNT must be incremented by one for each newly added exception.
    HPA_SEPARATOR_ID,                                           //==NoMessage==

    //HPS Exceptions
    HPS_EXCEPTION = HPS_BASE,                                   // =="A Picture Script exception occurred  (line : %lu, column : %lu)"==
    HPS_INCLUDE_NOT_FOUND_EXCEPTION,                            // =="Cannot find the file included in the PSS (line : %lu, column : %lu)"==
    HPS_INVALID_OBJECT_EXCEPTION,                               // =="The object in the PSS is invalid (line : %lu, column : %lu)"==
    HPS_TYPE_MISMATCH_EXCEPTION,                                // =="Type mismatch in the PSS (line : %lu, column : %lu)"==
    HPS_IMAGE_EXPECTED_EXCEPTION,                               // =="An image in the PSS is expected (line : %lu, column : %lu)"==
    HPS_TRANSFO_EXPECTED_EXCEPTION,                             // =="A transformation in the PSS is expected (line : %lu, column : %lu)"==
    HPS_FILTER_EXPECTED_EXCEPTION,                              // =="A filter in the PSS is expected (line : %lu, column : %lu)"==
    HPS_WORLD_EXPECTED_EXCEPTION,                               // =="A world identifier in the PSS is expected (line : %lu, column : %lu)"==
    HPS_QUALIFIER_EXPECTED_EXCEPTION,                           // =="A qualifier in the PSS is expected (line : %lu, column : %lu)"==
    HPS_EXPRESSION_EXPECTED_EXCEPTION,                          // =="An expression in the PSS is expected (line : %lu, column : %lu)"==
    HPS_INVALID_NUMERIC_EXCEPTION,                              // =="Invalid numeric expression in the PSS (line : %lu, column : %lu)"==
    HPS_INVALID_URL_EXCEPTION,                                  // =="The URL specified in the PSS is invalid (line : %lu, column : %lu)"==
    HPS_FILE_NOT_FOUND_EXCEPTION,                               // =="Cannot find the file specified in the PSS (line : %lu, column : %lu)"==
    HPS_NO_IMAGE_IN_FILE_EXCEPTION,                             // =="The file specified in the PSS is not an image (line : %lu, column : %lu)"==
    HPS_OUT_OF_RANGE_EXCEPTION,                                 // =="Out of range exception in the PSS (line : %lu, column : %lu)"==
    HPS_ALPHA_PALETTE_NOT_SUPPORTED_EXCEPTION,                  // =="The alpha palette specified in the PSS is not supported  (line : %lu, column : %lu)"==
    HPS_INVALID_WORLD_EXCEPTION,                                // =="The world specified in the PSS is invalid (line : %lu, column : %lu)"==
    HPS_TOO_FEW_PARAM_EXCEPTION,                                // =="There are too few parameters specified in the PSS (line : %lu, column : %lu)"==
    HPS_TOO_MANY_PARAM_EXCEPTION,                               // =="There are too many parameters specified in the PSS (line : %lu, column : %lu)"==
    HPS_ALREADY_DEFINED_EXCEPTION,                              // =="Redefinition found in the PSS (line : %lu, column : %lu)"==
    HPS_TRANSFO_PARAMETERS_INVALID_EXCEPTION,                   // =="Invalid transformation parameters specified in the PSS (line : %lu, column : %lu)"==
    HPS_IMAGE_HAS_NO_SIZE_EXCEPTION,                            // =="The image specified in the PSS has no size (line : %lu, column : %lu)"==
    HPS_INVALID_COORDS_EXCEPTION,                               // =="Invalid coordinate system specified in the PSS (line : %lu, column : %lu)"==
    HPS_PAGE_NOT_FOUND_EXCEPTION,                               // =="Cannot find the page specified in the PSS (line : %lu, column : %lu)"==
    HPS_INVALID_POLYGON_EXCEPTION,                              // =="The polygon specified in the PSS is invalid (line : %lu, column : %lu)"==
    HPS_SHAPE_EXPECTED_EXCEPTION,                               // =="A shape is expected in the PSS (line : %lu, column : %lu)"==
    HPS_WORLD_ALREADY_USED_EXCEPTION,                           // =="The world specified in the PSS is already used (line : %lu, column : %lu)"==
    HPS_COLOR_SET_EXPECTED_EXCEPTION,                           // =="A color set is expected in the PSS (line : %lu, column : %lu)"==
    HPS_TRANSLUCENT_INFO_NOT_FOUND_EXCEPTION,                   // =="Cannot find the translucency information in the PSS (line : %lu, column : %lu)"==
    HPS_IMAGE_CONTEXT_EXPECTED_EXCEPTION,                       // =="An image context identifier in the PSS is expected (line : %lu, column : %lu)"==

    //New HPS exception must be added above this line
    //HPS_EXCEPTION_COUNT must be incremented by one for each newly added exception.
    HPS_SEPARATOR_ID,                                           //==NoMessage==

    //HRF Internet Imaging  Exceptions
    HRFII_CONNECTION_EXCEPTION = HRFII_BASE,                    // =="An Internet connection exception occurred (error type : %i)"==
    HRFII_SERVER_EXCEPTION,                                     // =="A server exception occurred (class : %lu, number : %lu, object : %s, message : %s)"==
    HRFII_CANCELLED_EXCEPTION,                                  // =="The request has been cancelled"==
    HRFII_INVALID_DATA_FORMAT_EXCEPTION,                        // =="Invalid IIP data format (handler : %i)"==
    HRFII_INVALID_VALUE_EXCEPTION,                              // =="Invalid IIP value (handler : %i)"==
    HRFII_TILE_NOT_FOUND_EXCEPTION,                             // =="Cannot find the pool space for the received tile over IIP"==
    HRFII_NO_RESOLUTION_INDEX_LEFT_EXCEPTION,                   // =="There is no resolution index left for IIP"==
    HRFII_INVALID_RESPONSE_EXCEPTION,                           // =="The server response is invalid"==
    HRFII_INVALID_PROTOCOL_CHARSET_EXCEPTION,                   // =="The HIP character set is invalid"==
    HRFII_INVALID_RESOLUTION_VALUE_EXCEPTION,                   // =="The resolution value in the IIP response is invalid (handler : %i)"==
    HRFII_INVALID_REDIRECTION_URL_EXCEPTION,                    // =="The redirection URL in the IIP response is invalid"==

    //New HRFII exception must be added above this line
    //HRFII_EXCEPTION_COUNT must be incremented by one for each newly added exception.
    HRFII_SEPARATOR_ID,                                         //==NoMessage==

    //HRP Exceptions
    HRP_EXCEPTION = HRP_BASE,                                   //==NoMessage== Class to throw : HRPException 
    //New HRP exception must be added above this line
    //HRP_EXCEPTION_COUNT must be incremented by one for each newly added exception.
    HRP_SEPARATOR_ID,                                           //==NoMessage==
    //HRF Exceptions
    HRF_EXCEPTION = HRF_BASE,                                   // =="An exception related to a raster file operation occurred"==
    HRF_BAD_PAGE_NUMBER_EXCEPTION,                              // =="The page number is invalid"==
    HRF_BAD_SUB_IMAGE_EXCEPTION,                                // =="The sub-image is invalid"==
    HRF_PIXEL_TYPE_NOT_SUPPORTED_EXCEPTION,                     // =="The pixel type is not supported"==
    HRF_CODEC_NOT_SUPPORTED_EXCEPTION,                          // =="The codec is not supported"==
    HRF_MULTI_PAGE_NOT_SUPPORTED_EXCEPTION,                     // =="Multi-page is not supported"==
    HRF_ACCESS_MODE_FOR_PIXEL_TYPE_NOT_SUPPORTED_EXCEPTION,     // =="The requested access mode for the pixel type is not supported"==
    HRF_ACCESS_MODE_FOR_CODEC_NOT_SUPPORTED_EXCEPTION,          // =="The requested access mode for the codec is not supported"==
    HRF_TRANSFO_MODEL_NOT_SUPPORTED_EXCEPTION,                  // =="The transformation model is not supported"==
    HRF_XCH_CHANNELS_ARE_NOT_OF_THE_SAME_FORMAT_EXCEPTION,      // =="The X channels do not have the same format"==
    HRF_XCH_CHANNELS_DO_NOT_HAVE_THE_SAME_COMPRESSION_EXCEPTION,// =="The X channels do not have the same compression"==
    HRF_XCH_CHANNEL_IS_NOT_A_VALID_GRAYSCALE_EXCEPTION,         // =="The X channel does not have a valid grayscale pixel type"==
    HRF_XCH_CHANNELS_DO_NOT_HAVE_THE_SAME_DIMENSIONS_EXCEPTION, // =="The X channels do not have the same dimensions"==
    HRF_XCH_CHANNELS_DO_NOT_HAVE_THE_SAME_RES_COUNT_EXCEPTION,  // =="The X channels do not have the dame resolution count"==
    HRF_NEED_OPEN_PASSWORD_EXCEPTION,                           // =="An open password is required"==
    HRF_NEED_RESTRICTION_PASSWORD_EXCEPTION,                    // =="A restriction password is required"==
    HRF_INVALID_OPEN_PASSWORD_EXCEPTION,                        // =="The open password is invalid"==
    HRF_INVALID_RESTRICTION_PASSWORD_EXCEPTION,                 // =="The restriction password is invalid"==
    HRF_UNSUPPORTED_ITIFF_VERSION_EXCEPTION,                    // =="Unsupported iTIFF version"==
    HRF_WMS_EXCEPTION,                                          // =="A WMS exception occurred : %s"==
    HRF_GEOTIFF_COMPRESSED_TABLE_LOCK_EXCEPTION,                // =="The GeoTIFF compression table is locked"==
    HRF_PW_NO_HANDLER_EXCEPTION,                                // =="Cannot find a ProjectWise handler"==
    HRF_CANNOT_OPEN_CHILD_FILE_EXCEPTION,                       // =="Cannot open the following child file : %s"==
    HRF_CANNOT_FIND_CHILD_FILE_EXCEPTION,                       // =="Cannot find the following child file : %s"==
    HRF_INVALID_CHILD_FILE_URL_EXCEPTION,                       // =="The following child file's URL is invalid : %s"==
    HRF_CHILD_FILE_INVALID_PARAM_VALUE_EXCEPTION,               // =="The following parameter is invalid : (child file : %s, parameter : %s)"==
    HRF_CHILD_FILE_MISSING_PARAMETER_EXCEPTION,                 // =="The following parameter is missing : (child file : %s, parameter : %s)"==
    HRF_SISTER_FILE_INVALID_PARAM_VALUE_EXCEPTION,              // =="The following parameter is invalid : (sister file : %s, parameter : %s)"==
    HRF_SISTER_FILE_MISSING_PARAMETER_EXCEPTION,                // =="The following parameter is missing : (sister file : %s, parameter : %s)"==
    HRF_SISTER_FILE_MISSING_PARAMETER_GROUP_EXCEPTION,          // =="The following parameter group is missing : (sister file : %s, parameter : %s)"==
    HRF_SISTER_FILE_SLO_NOT_SUPPORTED_EXCEPTION,                // =="The SLO of the following sister file is not supported : %s"==
    HRF_INVALID_SISTER_FILE_EXCEPTION,                          // =="The following sister file is invalid : %s"==
    HRF_INVALID_TRANSFO_FOR_SISTER_FILE_EXCEPTION,              // =="The following sister file cannot store the transformation model : %s"==
    HRF_INVALID_PARAM_VALUE_EXCEPTION,                          // =="The value of the following parameter is invalid : %s"==
    HRF_XCH_CHANNELS_BLOCK_DIMENSIONS_DIFFER_EXCEPTION,         // =="The X channels do not have the same block dimension"==
    HRF_MIME_FORMAT_NOT_SUPPORTED_EXCEPTION,                    // =="The MIME format is not supported for the following parameter : %s"==
    HRF_MISSING_PARAMETER_EXCEPTION,                            // =="The following parameter is missing : %s"==
    HRF_UNSUPPORTED_xWMS_VERSION_EXCEPTION,                     // =="Unsupported xWMS version"==
    HRF_USGS_BAND_NOT_FOUND_IN_HEADER_FILE_EXCEPTION,           // =="Cannot find the following USGS band cannot be found in the header file : %s"==
    HRF_TIFF_ERROR_EXCEPTION,                                   // =="The following error occurred in the TIFF library : %s"==
    HRF_SLO_NOT_SUPPORTED_EXCEPTION,                            // =="The SLO is not supported"==
    HRF_SUB_RES_ACCESS_DIFFER_READ_ONLY_EXCEPTION,              // =="The file is only supported in read-only mode because of different block access modes between the first resolution and at least one sub-resolution"==
    HRF_INTERGRAPH_LUT_READ_ONLY_EXCEPTION,                     // =="The Intergraph LUT is only supported in read-only mode"==
    HRF_PIXEL_TYPE_CODEC_NOT_SUPPORTED_EXCEPTION,               // =="The combination of pixel type/codec is not supported"==
    HRF_ANIMATED_GIF_READ_ONLY_EXCEPTION,                       // =="An animated GIF is only supported in read-only mode"==
    HRF_MORE_THAN_24_TIE_POINTS_READ_ONLY_EXCEPTION,            // =="The use of more than 24 tie points is only supported in read-only mode"==
    HRF_UNSUPPORTED_BMP_VERSION_EXCEPTION,                      // =="Unsupported BMP version"==
    HRF_TRANSFO_CANNOT_BE_A_MATRIX_EXCEPTION,                   // =="The transformation model cannot be represented by a matrix"==
    HRF_UNSUPPORTED_HGR_VERSION_EXCEPTION,                      // =="Unsupported HGR version"==
    HRF_CANNOT_DOWNLOAD_TO_INTERNET_CACHE_EXCEPTION,            // =="Cannot download remote file to local Internet cache"==
    HRF_WMS_CANNOT_UNCOMPRESS_DATA,                             // =="Cannot uncompress WMS server data"==
    HRF_WMS_CANNOT_CONNECT_TO_SERVER,                           // =="Cannot connect to WMS server"==
    HRF_ERS_DATA_FOUND_OUTSIDE_DATASET_HEADER_EXCEPTION,        // =="No data can be present outside the dataset header block (sister file : %s, parameter : %s)"==
    HRF_ERS_UNMATCH_REG_SPACE_COORD_TYPE_EXCEPTION,             // =="The following sister file has different types for the registration coordinates and space coordinates : %s"==
    HRF_ERS_UNMATCH_REG_COORD_TYPE_EXCEPTION,                   // =="The coordinates space block of the following sister file has at least two parameters of different types : %s"==
    HRF_TOO_SMALL_FOR_ECW_COMPRESSION_EXCEPTION,                // =="The dimensions of the image to compress are too small for ECW compression"==
    HRF_INVALID_EXPORT_OPTION_EXCEPTION,                        // =="The export options are invalid"==
    HRF_OPERATION_RESTRICTED_PDF_NOT_SUPPORTED_EXCEPTION,       // =="PDF files with any operation restrictions are not supported"==
    HRF_DIMENSION_TOO_LARGE_FOR_FILE_CREATION_EXCEPTION,        // =="The image cannot be exported to the selected file format because its size is too large (width limit : %I64u, height limit : %I64u)"==
    HRF_DATA_HAVE_BEEN_SCALED_READ_ONLY_EXCEPTION,              // =="The image is only supported in read-only mode because its pixel values have been scaled."==
    HRF_PDF_EXCEPTION,                                          // =="An error occurred in the PDF library (Error code: %d, Error message: %s)"==
    HRF_AUTHENTICATION_MAX_RETRY_COUNT_REACHED_EXCEPTION,       // =="Maximum retry count has been reached"==
    HRF_ORACLE_EXCEPTION,                                       // =="An error occurred while using the Oracle database (Error code: %s, Error message: %s)"==
    HRF_AUTHENTICATION_CANCELLED_EXCEPTION,                     // =="Authentication was cancelled by the user"==
    HRF_AUTHENTICATION_INVALID_LOGIN_EXCEPTION,                 // =="The specified authentication login name or password is invalid"==
    HRF_AUTHENTICATION_INVALID_SERVICE_EXCEPTION,               // =="The specified service is invalid or could not be reached"==
        
    //New HRF exception must be added above this line
    //HRF_EXCEPTION_COUNT must be incremented by one for each newly added exception.
    HRF_SEPARATOR_ID,                                           //==NoMessage==
    
    //HCS Exceptions
    HCS_EXCEPTION = HCS_BASE,                                   // =="A server connection exception occurred"==
    HCS_NOT_CONNECTED_EXCEPTION,                                // =="%s is not connected"==
    HCS_CANNOT_ACCEPT_CONNECTION_EXCEPTION,                     // =="%s cannot accept connection"==
    HCS_CANNOT_GET_NAME_EXCEPTION,                              // =="Cannot get %s's name"==
    HCS_CANNOT_LISTEN_EXCEPTION,                                // =="%s cannot listen"==
    HCS_CANNOT_BIND_EXCEPTION,                                  // =="%s cannot bind"==
    HCS_INVALID_DOMAIN_NAME_EXCEPTION,                          // =="The domain name for the %s is not valid"==
    HCS_CANNOT_CREATE_EXCEPTION,                                // =="Cannot create %s"==
        
    //New HCS exception must be added above this line
    //HCS_EXCEPTION_COUNT must be incremented by one for each newly added exception.
    HCS_SEPARATOR_ID,                                           //==NoMessage==
    
    //HCP Exceptions
    HCP_GCOORD_EXCEPTION = HCP_BASE,                            // =="An exception occurred in GCoord (error code : %i)"==

    //New HCP exception must be added above this line
    //HCP_EXCEPTION_COUNT must be incremented by one for each newly added exception.
    HCP_SEPARATOR_ID,                                           //==NoMessage==
    
    //HVE Exceptions
    HVE_EXCEPTION = HVE_BASE,                                   // =="A vector related exception occurred"==
    HVE_NO_POINT_FOUND_SHAPE_DECOMP_EXCEPTION,                  // =="Could not find a matching point during shape decomposition"==
    HVE_TOLERANCE_TOO_SMALL_POSSIBLE_INFINITE_LOOP,             // =="Tolerance is too small for shape operation"==

    //New HVE exception must be added above this line
    //HVE_EXCEPTION_COUNT must be incremented by one for each newly added exception.
    HVE_SEPARATOR_ID                                            //==NoMessage==
    };
BENTLEY_TRANSLATABLE_STRINGS2_END