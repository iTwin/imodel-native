/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/h/ImagePPExceptionMessages.xliff.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <BeSQLite/L10N.h>

BEGIN_IMAGEPP_NAMESPACE

BENTLEY_TRANSLATABLE_STRINGS_START(ImagePPExceptions,ImagePPExceptions)
    L10N_STRING(Unknown)                                     // =="Unknown exception"==
    L10N_STRING(HFCUndefined)                                // =="Foundation class exception"==
    L10N_STRING(HFCMemory)                                   // =="Memory exception"==
    L10N_STRING(HFCOutOfMemory)                              // =="Out of memory"==
    L10N_STRING(HFCGenericFile)                              // =="File exception"==
    L10N_STRING(HFCCannotLockFile)                           // =="Cannot lock the file"==
    L10N_STRING(HFCFileLockViolation)                        // =="File locking violation"==
    L10N_STRING(HFCCorruptedFile)                            // =="Corrupted file"==
    L10N_STRING(HFCFileNotFound)                             // =="File not found"==
    L10N_STRING(HFCFileExist)                                // =="The file already exists"==
    L10N_STRING(HFCFileReadOnly)                             // =="The file is in read-only mode"==
    L10N_STRING(HFCFileNotCreated)                           // =="The file cannot be created"==
    L10N_STRING(HFCFileNotSupported)                         // =="The file type is not supported"==
    L10N_STRING(HFCFileOutOfRange)                           // =="Out of range exception"==
    L10N_STRING(HFCFilePermissionDenied)                     // =="File permission denied"==
    L10N_STRING(HFCDllNotFound)                              // =="Cannot find the DLL %ls"==
    L10N_STRING(HFCUndefinedDevice)                          // =="An exception occurred with the following device : %ls"==
    L10N_STRING(HFCNoDiskSpaceLeft)                          // =="There is not enough disk space left"==
    L10N_STRING(HFCDeviceAbort)                              // =="The following device aborted : %ls"==
    L10N_STRING(HFCInternetConnection)                       // =="Internet connection exception (device : %ls, error code : %i)"==
    L10N_STRING(HFCObjectNotInFactory)                       // =="The object is not in the factory"==
    L10N_STRING(HFCErrorCode)                                // =="An error code triggered an exception"==
    L10N_STRING(HFCCannotCreateSynchroObj)                   // =="Cannot create a synchronization object (object : %i)"==
    L10N_STRING(HFCInvalidFileName)                          // =="The file name is not valid"==
    L10N_STRING(HFCSharingViolation)                         // =="The process cannot access the file because it is being used by another process"==
    L10N_STRING(HFCCannotUnlock)                             // =="Cannot unlock the file"==
    L10N_STRING(HFCHtppRequestString)                        // =="Invalid HTTP request string"==
    L10N_STRING(HFCCannotOpenFile)                           // =="Cannot open file"==
    L10N_STRING(HFCCannotConnectToDB)                        // =="Cannot connect to the database"==
    L10N_STRING(HFCInvalidUrlForSisterFile)                  // =="Cannot create a sister file for that URL"==
    L10N_STRING(HFCDirectoryReadOnly)                        // =="%ls is in read-only mode"==
    L10N_STRING(HFCSisterFileNotCreated)                     // =="Cannot open the following sister file : %ls"==
    L10N_STRING(HFCDirectoryNotCreated)                      // =="Cannot create the following directory : %ls"==
    L10N_STRING(HFCCannotCloseFile)                          // =="Cannot close the file"==
    L10N_STRING(HFCWriteFault)                               // =="A write fault occurred"==
    L10N_STRING(HFCReadFault)                                // =="A read fault occurred"==
    L10N_STRING(HFCFileIOError)                              // =="A file IO exception occurred"==
    L10N_STRING(HFCLoginInformationNotAvailable)             // =="The login information is not available"==
    L10N_STRING(HFCFileNotSupportedInWriteMode)              // =="This file is not supported in write mode"==
    L10N_STRING(HFCUnsupportedFileVersion)                   // =="Unsupported file version"==

    L10N_STRING(HCDIJLError)                                 // =="An error occurred in the JPEG library"==
    L10N_STRING(HCDIJGLibraryError)                          // =="A JPEG error occurred"==
    L10N_STRING(HCDCorruptedPackbitsData)                    // =="The PackBits encoded data are corrupted"==

    L10N_STRING(HFSGenericInvalidPath)                       // =="%ls is an invalid file server path"==
    L10N_STRING(HFSHIBPInvalidResponseException)             // =="Invalid IBP response"==
    L10N_STRING(HFSHIBPErrorException)                       // =="The following IBP error occurred : %ls"==
    L10N_STRING(HFSUrlSchemeNotSupported)                    // =="The scheme of the following URL is not supported by the file server : %ls"==
    L10N_STRING(HFSHIBPProtocolNotSupported)                 // =="The requested Internet browsing protocol requested is not supported"==
    L10N_STRING(HFSInvalidDirectory)                         // =="%ls is an invalid directory path on the file server"==
    L10N_STRING(HFSCannotStartNexNetResourceEnum)            // =="Cannot start the enumeration of network resources"==
    L10N_STRING(HFSCannotEnumNextNetResource)                // =="Cannot enumerate the next network resource"==

    L10N_STRING(HGFDomain)                                   // =="The requested is outside the grid model's domain"==
    L10N_STRING(HGFmzGCoordException)                        // =="A GeoCoord exception occurred"==

    L10N_STRING(HPANoToken)                                  // =="The parser cannot find a token (line : %lu, column : %lu)"==
    L10N_STRING(HPARecursiveInclusion)                       // =="The file or stream currently being parsed has already been included (line : %lu, column : %lu)"==
    L10N_STRING(HPAGeneric)                                  // =="The following generic parser exception occurred : %ls"==
    L10N_STRING(HPACannotFindProduction)                     // =="The parser cannot find a valid production (line : %lu, column : %lu)"==
    L10N_STRING(HPACannotResolveStartRule)                   // =="The parser cannot resolve the starting rule"==
    L10N_STRING(HPANodeLeftToParse)                          // =="Some unparsed nodes remain after finding the starting rule  (line : %lu, column : %lu)"==

    L10N_STRING(HPSIncludeNotFound)                          // =="Cannot find the file included in the PSS (line : %lu, column : %lu)"==
    L10N_STRING(HPSInvalidObject)                            // =="The object in the PSS is invalid (line : %lu, column : %lu)"==
    L10N_STRING(HPSTypeMismatch)                             // =="Type mismatch in the PSS (line : %lu, column : %lu)"==
    L10N_STRING(HPSQualifierExpected)                        // =="A qualifier in the PSS is expected (line : %lu, column : %lu)"==
    L10N_STRING(HPSExpressionExpected)                       // =="An expression in the PSS is expected (line : %lu, column : %lu)"==
    L10N_STRING(HPSInvalidNumeric)                           // =="Invalid numeric expression in the PSS (line : %lu, column : %lu)"==
    L10N_STRING(HPSInvalidUrl)                               // =="The URL specified in the PSS is invalid (line : %lu, column : %lu)"==
    L10N_STRING(HPSFileNotFound)                             // =="Cannot find the file specified in the PSS (line : %lu, column : %lu)"==
    L10N_STRING(HPSNoImageInFile)                            // =="The file specified in the PSS is not an image (line : %lu, column : %lu)"==
    L10N_STRING(HPSOutOfRange)                               // =="Out of range exception in the PSS (line : %lu, column : %lu)"==
    L10N_STRING(HPSAlphaPaletteNotSupported)                 // =="The alpha palette specified in the PSS is not supported  (line : %lu, column : %lu)"==
    L10N_STRING(HPSInvalidWorld)                             // =="The world specified in the PSS is invalid (line : %lu, column : %lu)"==
    L10N_STRING(HPSTooFewParam)                              // =="There are too few parameters specified in the PSS (line : %lu, column : %lu)"==
    L10N_STRING(HPSTooManyParam)                             // =="There are too many parameters specified in the PSS (line : %lu, column : %lu)"==
    L10N_STRING(HPSAlreadyDefined)                           // =="Redefinition found in the PSS (line : %lu, column : %lu)"==
    L10N_STRING(HPSTransfoParameterInvalid)                  // =="Invalid transformation parameters specified in the PSS (line : %lu, column : %lu)"==
    L10N_STRING(HPSImageHasNoSize)                           // =="The image specified in the PSS has no size (line : %lu, column : %lu)"==
    L10N_STRING(HPSInvalidCoord)                             // =="Invalid coordinate system specified in the PSS (line : %lu, column : %lu)"==
    L10N_STRING(HPSPageNotFound)                             // =="Cannot find the page specified in the PSS (line : %lu, column : %lu)"==
    L10N_STRING(HPSInvalidPolygon)                           // =="The polygon specified in the PSS is invalid (line : %lu, column : %lu)"==
    L10N_STRING(HPSShapeExpected)                            // =="A shape is expected in the PSS (line : %lu, column : %lu)"==
    L10N_STRING(HPSWorldAlreadyUsed)                         // =="The world specified in the PSS is already used (line : %lu, column : %lu)"==
    L10N_STRING(HPSTranslucentInfoNotFound)                  // =="Cannot find the translucency information in the PSS (line : %lu, column : %lu)"==

    L10N_STRING(HRFIIConnection)                             // =="An Internet connection exception occurred (error type : %i)"==
    L10N_STRING(HRFIIServer)                                 // =="A server exception occurred (class : %lu, number : %lu, object : %ls, message : %ls)"==
    L10N_STRING(HRFIICancelled)                              // =="The request has been cancelled"==
    L10N_STRING(HRFIIInvalidDataFormat)                      // =="Invalid IIP data format (handler : %i)"==
    L10N_STRING(HRFIIInvalidValue)                           // =="Invalid IIP value (handler : %i)"==
    L10N_STRING(HRFIITileNotFound)                           // =="Cannot find the pool space for the received tile over IIP"==
    L10N_STRING(HRFIINoResolutionIndexLeft)                  // =="There is no resolution index left for IIP"==
    L10N_STRING(HRFIIInvalidResponse)                        // =="The server response is invalid"==
    L10N_STRING(HRFIIInvalidProtocolCharset)                 // =="The HIP character set is invalid"==
    L10N_STRING(HRFIIInvalidResolutionValue)                 // =="The resolution value in the IIP response is invalid (handler : %i)"==
    L10N_STRING(HRFIIInvalidRedirectionURL)                  // =="The redirection URL in the IIP response is invalid"==

    L10N_STRING(HRFGeneric)                                  // =="An exception related to a raster file operation occurred"==
    L10N_STRING(HRFBadPageNumber)                            // =="The page number is invalid"==
    L10N_STRING(HRFBadSubImage)                              // =="The sub-image is invalid"==
    L10N_STRING(HRFPixelTypeNotSupported)                    // =="The pixel type is not supported"==
    L10N_STRING(HRFCodecNotSupported)                        // =="The codec is not supported"==
    L10N_STRING(HRFMultiPageNotSupported)                    // =="Multi-page is not supported"==
    L10N_STRING(HRFAccessModeForPixelTypeNotSupported)       // =="The requested access mode for the pixel type is not supported"==
    L10N_STRING(HRFAccessModeForCodecNotSupported)           // =="The requested access mode for the codec is not supported"==
    L10N_STRING(HRFTransfoModelNotSupported)                 // =="The transformation model is not supported"==
    L10N_STRING(HRFXCHChannelsAreNotOfTheSameFormat)         // =="The X channels do not have the same format"==
    L10N_STRING(HRFXCHChannelsDoNotHaveSameCompression)      // =="The X channels do not have the same compression"==
    L10N_STRING(HRFXCHChannelsIsNotAValidGrayscale)          // =="The X channel does not have a valid grayscale pixel type"==
    L10N_STRING(HRFFXHChannelsDoNotHaveTheSameDimensions)    // =="The X channels do not have the same dimensions"==
    L10N_STRING(HRFFXHChannelsDoNotHaveTheSameResCount)      // =="The X channels do not have the dame resolution count"==
    L10N_STRING(HRFNeedOpenPassword)                         // =="An open password is required"==
    L10N_STRING(HRFNeedRestrictionPassword)                  // =="A restriction password is required"==
    L10N_STRING(HRFInvalidOpenPassword)                      // =="The open password is invalid"==
    L10N_STRING(HRFInvalidRestrictionPassword)               // =="The restriction password is invalid"==
    L10N_STRING(HRFUnsupportedITiffVersion)                  // =="Unsupported iTIFF version"==
    L10N_STRING(HRFGeotiffCompressedTableLock)               // =="The GeoTIFF compression table is locked"==
    L10N_STRING(HRFPWNoHandler)                              // =="Cannot find a ProjectWise handler"==
    L10N_STRING(HRFCannotOpenChildFile)                      // =="Cannot open the following child file : %ls"==
    L10N_STRING(HRFChildFileParameter)                       // =="The following parameter is invalid : (child file : %ls, parameter : %ls)"==
    L10N_STRING(HRFSisterFileInvalidParamValue)              // =="The following parameter is invalid : (sister file : %ls, parameter : %ls)"==
    L10N_STRING(HRFSisterFileMissing)                        // =="The following parameter is missing : (sister file : %ls, parameter : %ls)"==
    L10N_STRING(HRFSisterFileMissingParamGroup)              // =="The following parameter group is missing : (sister file : %ls, parameter : %ls)"==
    L10N_STRING(HRFSisterFileSlotNotSupported)               // =="The SLO of the following sister file is not supported : %ls"==
    L10N_STRING(HRFInvalidSisterFile)                        // =="The following sister file is invalid : %ls"==
    L10N_STRING(HRFInvalidTransfoForSisterFile)              // =="The following sister file cannot store the transformation model : %ls"==
    L10N_STRING(HRFInvalidParamValue)                        // =="The value of the following parameter is invalid : %ls"==
    L10N_STRING(HRFXCHChannelsBlockDimensionsDiffer)         // =="The X channels do not have the same block dimension"==
    L10N_STRING(HRFMimeFormatNotSupported)                   // =="The MIME format is not supported for the following parameter : %ls"==
    L10N_STRING(HRFMissingParameter)                         // =="The following parameter is missing : %ls"==
    L10N_STRING(HRFUnsupportedxWMSVersion)                   // =="Unsupported xWMS version"==
    L10N_STRING(HRFUSGSBandNotFoundInHeaderFile)             // =="Cannot find the following USGS band cannot be found in the header file : %ls"==
    L10N_STRING(HRFTiffError)                                // =="The following error occurred in the TIFF library : %ls"==
    L10N_STRING(HRFSloNotSupported)                          // =="The SLO is not supported"==
    L10N_STRING(HRFSubResAcessDifferReadOnly)                // =="The file is only supported in read-only mode because of different block access modes between the first resolution and at least one sub-resolution"==
    L10N_STRING(HRFIntergraphLutReadOnly)                    // =="The Intergraph LUT is only supported in read-only mode"==
    L10N_STRING(HRFPixelTypeCodecNotSupported)               // =="The combination of pixel type/codec is not supported"==
    L10N_STRING(HRFAnimatedGifReadOnly)                      // =="An animated GIF is only supported in read-only mode"==
    L10N_STRING(HRFMoreThan24TiePointReadOnly)               // =="The use of more than 24 tie points is only supported in read-only mode"==
    L10N_STRING(HRFUnsupportedBMPVersion)                    // =="Unsupported BMP version"==
    L10N_STRING(HRFTransfoCannotBeAMatrix)                   // =="The transformation model cannot be represented by a matrix"==
    L10N_STRING(HRFUnsupportedHGRVersion)                    // =="Unsupported HGR version"==
    L10N_STRING(HRFCannotDownloadToInternetCache)            // =="Cannot download remote file to local Internet cache"==
    L10N_STRING(HRFERSDataFoundOutsideDatasetHeader)         // =="No data can be present outside the dataset header block (sister file : %ls, parameter : %ls)"==
    L10N_STRING(HRFERSUnmatchRegSpaceCoordType)              // =="The following sister file has different types for the registration coordinates and space coordinates : %ls"==
    L10N_STRING(HRFERSUnmatchRegCoordType)                   // =="The coordinates space block of the following sister file has at least two parameters of different types : %ls"==
    L10N_STRING(HRFTooSmallForEcwCompression)                // =="The dimensions of the image to compress are too small for ECW compression"==
    L10N_STRING(HRFTooBigForEcwCompression)                  // =="Input image size was exceeded for this version of the SDK"==
    L10N_STRING(HRFInvalidExportOption)                      // =="The export options are invalid"==
    L10N_STRING(HRFOperationRestrictedPDFNotSupported)       // =="PDF files with any operation restrictions are not supported"==
    L10N_STRING(HRFInvalidNewFileDimension)                  // =="The image cannot be exported to the selected file format because its size is too large (width limit : %I64u, height limit : %I64u)"==
    L10N_STRING(HRFDataHaveBeenScaleReadOnly)                // =="The image is only supported in read-only mode because its pixel values have been scaled."==
    L10N_STRING(HRFPDFGeneric)                               // =="An error occurred in the PDF library (Error code: %d, Error message: %ls)"==
    L10N_STRING(HRFAuthenticationMaxRetryCountReadched)      // =="Maximum retry count has been reached"==
    L10N_STRING(HRFOracleGeneric)                            // =="An error occurred while using the Oracle database (Error code: %ls, Error message: %ls)"==
    L10N_STRING(HRFAuthenticationCancelled)                  // =="Authentication was cancelled by the user"==
    L10N_STRING(HRFAuthenticationInvalidLogin)               // =="The specified authentication login name or password is invalid"==
    L10N_STRING(HRFAuthenticationInvalidService)             // =="The specified service is invalid or could not be reached"==

    L10N_STRING(HCSNotConnected)                             // =="%ls is not connected"==
    L10N_STRING(HCSCannotAcceptConnection)                   // =="%ls cannot accept connection"==
    L10N_STRING(HCSCannotGetName)                            // =="Cannot get %ls's name"==
    L10N_STRING(HCSCannotListen)                             // =="%ls cannot listen"==
    L10N_STRING(HCannotBindConnection)                       // =="%ls cannot bind"==
    L10N_STRING(HCSInvalidDomainName)                        // =="The domain name for the %ls is not valid"==
    L10N_STRING(HCSCannotCreate)                             // =="Cannot create %ls"==

BENTLEY_TRANSLATABLE_STRINGS_END

END_IMAGEPP_NAMESPACE
