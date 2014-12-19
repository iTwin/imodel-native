//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetImaging.h $
//:>
//:>  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// HRFInternetImaging.h
// Various IIP/HIP Information
//-----------------------------------------------------------------------------

#pragma once

//
// Colorspace Definition (IIP only)
//
#define IIP_COLORSPACE_COLORLESS                    0x00000000
#define IIP_COLORSPACE_MONOCHROME                   0x00000001  // AKA GrayScale
#define IIP_COLORSPACE_PHOTO_YCC                    0x00000002
#define IIP_COLORSPACE_NIF_RGB                      0x00000003

//
// Colorspace Field Values
//
#define IIP_COLORSPACE_FIELD_COLORLESS              0x00000000
#define IIP_COLORSPACE_FIELD_MONOCHROME             0x00000000  // Grayscale (Monochrome)
#define IIP_COLORSPACE_FIELD_LUMINANCE              0x00000000  // PhotoYCC
#define IIP_COLORSPACE_FIELD_CHROMINANCE1           0x00000001  // PhotoYCC
#define IIP_COLORSPACE_FIELD_CHROMINANCE2           0x00000002  // PhotoYCC
#define IIP_COLORSPACE_FIELD_RED                    0x00000000  // NIF RGB
#define IIP_COLORSPACE_FIELD_GREEN                  0x00000001  // NIF RGB
#define IIP_COLORSPACE_FIELD_BLUE                   0x00000002  // NIF RGB
#define IIP_COLORSPACE_FIELD_OPACITY                0x00007FFE  // All


//
// Compression Types (IIP and HIP)
//
// 0x0 to 0x7fffffff are reserved
//
#define IIP_COMPRESSION_INVALID                     0xFFFFFFFF
#define IIP_COMPRESSION_NONE                        0x00000000
#define IIP_COMPRESSION_SINGLE_COLOR                0x00000001
#define IIP_COMPRESSION_JPEG                        0x00000002

// Introduced in HIP 1.0
#define HIP_COMPRESSION_WAVELET                     0x80000000
#define HIP_COMPRESSION_DEFLATE                     0x80000001
#define HIP_COMPRESSION_PACKBITS                    0x80000002
#define HIP_COMPRESSION_CCITT3                      0x80000003
#define HIP_COMPRESSION_CCITT4                      0x80000004
#define HIP_COMPRESSION_RLE                         0x80000005

// Introduced in HIP 1.,2 and MSD-HIP 1.1
#define HIP_COMPRESSION_JPEG_RLE8                   0x80000006

//
// JPEG Compression Sub-Type index.
// Index in the four sub-type bytes
#define IIP_COMPRESSION_JPEG_INTERLEAVE_INDEX       0x00000000
#define IIP_COMPRESSION_JPEG_SUBSAMPLING_INDEX      0x00000001
#define IIP_COMPRESSION_JPEG_COLOR_INDEX            0x00000002
#define IIP_COMPRESSION_JPEG_TABLE_INDEX            0x00000003

//
// JPEG Compression Fields
//
#define IIP_COMPRESSION_JPEG_CHROMA11               0x00000011
#define IIP_COMPRESSION_JPEG_CHROMA21               0x00000021
#define IIP_COMPRESSION_JPEG_CHROMA22               0x00000022

//
// CCITT Compression Sub-Type index
//
#define IIP_COMPRESSION_CCITT_PHOTOMETRIC           0x00000000

//
// RLE Compression Sub-Type index.
//
#define IIP_COMPRESSION_RLE_ONE_LINE                0x00000000


// IIP Error classes
#define IIP_ERROR_CLASS_FILE                        0x00000001
#define IIP_ERROR_CLASS_COMMAND                     0x00000002
#define IIP_ERROR_CLASS_OBJECT                      0x00000003
#define IIP_ERROR_CLASS_SERVER                      0x00000004

// Error number (for error classes 1,2 & 3)
#define IIP_ERROR_NUMBER_SYNTAX                     0x00000001
#define IIP_ERROR_NUMBER_UNSUPPORTED                0x00000002
#define IIP_ERROR_NUMBER_UNAVAILABLE                0x00000003
#define IIP_ERROR_NUMBER_DENIED                     0x00000004

// Error number (for error class 4)
#define IIP_SERVER_ERROR_CONVERSION                 0x00000001
#define IIP_SERVER_ERROR_DENIED                     0x00000002
#define IIP_SERVER_ERROR_ALLOCATION                 0x00000003
#define IIP_SERVER_ERROR_OBJECT_CREATION            0x00000004
#define IIP_SERVER_ERROR_WRITE_ERROR                0x00000005
#define IIP_SERVER_ERROR_READ_ERROR                 0x00000006
#define IIP_SERVER_ERROR_VENDOR_DEFINED             0x00000007

