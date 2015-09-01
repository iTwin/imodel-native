
//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFJpegFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFJpegFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFJpegFile.h>

#include <libjpeg-turbo/jpeglib.h>
#include <libjpeg-turbo/jerror.h>

#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRPChannelOrgGray.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRFJpegLineEditor.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HRPPIxelTypeFactory.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>

#include <Imagepp/all/h/HTIFFFile.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#include <Imagepp/all/h/HFCURLMemFile.h>



// This structure is used by the error handling functions
// of the IJG HPEG library
struct HRFJpegFileErrorManager
    {
    struct jpeg_error_mgr  pub;
    HRFJpegFile*           m_pJpegFile;
    };

// This structure is used by the error handling functions
// of the IJG HPEG library during the call to the function IsKindOf(...)
struct HRFJpegFileIsKindOfErrorManager
    {
    struct jpeg_error_mgr  pub;
    WString                m_Url;
    };

//-----------------------------------------------------------------------------
// HRFJpegBlockCapabilities
//-----------------------------------------------------------------------------
class HRFJpegBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFJpegBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Line Capability
        Add(new HRFLineCapability(HFC_READ_WRITE_CREATE,        // AccessMode
                                  65000L,                       // MaxWidth
                                  HRFBlockAccess::SEQUENTIAL)); // BlockAccess
        }
    };

//-----------------------------------------------------------------------------
// HRFJpegCodecCapabilities
//-----------------------------------------------------------------------------
class HRFJpegCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFJpegCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec HMRGIF (LZW)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIJG::CLASS_ID,
                                   new HRFJpegBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFJpegCapabilities
//-----------------------------------------------------------------------------
HRFJpegCapabilities::HRFJpegCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV24R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFJpegCodecCapabilities()));
    // PixelTypeV8Gray8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFJpegCodecCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeResolutionUnit(0)));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeXResolution(1.0)));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeYResolution(1.0)));


    //GPS tags
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSVersionID));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSLatitudeRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSLatitude));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSLongitudeRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSLongitude));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSAltitudeRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSAltitude));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSTimeStamp));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSSatellites));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSStatus));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSMeasureMode));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDOP));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSSpeedRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSSpeed));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSTrackRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSTrack));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSImgDirectionRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSImgDirection));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSMapDatum));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestLatitudeRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestLatitude));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestLongitudeRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestLongitude));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestBearingRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestBearing));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestDistanceRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestDistance));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSProcessingMethod));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSAreaInformation));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDateStamp));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDifferential));


    //Based on the EXIF tags
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeExposureTime));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFNumber));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeExposureProgram));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSpectralSensitivity));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeISOSpeedRatings));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeOptoElectricConversionFactor));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeExifVersion));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDateTimeOriginal));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDateTimeDigitized));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeComponentsConfiguration));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeCompressedBitsPerPixel));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeShutterSpeedValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeApertureValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeBrightnessValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeExposureBiasValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeMaxApertureValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSubjectDistance));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeMeteringMode));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeLightSource));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFlash));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFocalLength));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSubjectArea));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeMakerNote));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeUserComment));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSubSecTime));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSubSecTimeOriginal));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSubSecTimeDigitized));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFlashpixVersion));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeColorSpace));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributePixelXDimension));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributePixelYDimension));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeRelatedSoundFile));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFlashEnergy));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSpatialFrequencyResponse));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFocalPlaneXResolution));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFocalPlaneYResolution));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFocalPlaneResolutionUnit));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSubjectLocation));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeExposureIndex));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSensingMethod));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFileSource));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSceneType));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeCFAPattern));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeCustomRendered));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeExposureMode));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeWhiteBalance));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDigitalZoomRatio));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFocalLengthIn35mmFilm));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSceneCaptureType));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGainControl));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeContrast));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSaturation));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSharpness));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDeviceSettingDescription));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSubjectDistanceRange));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeImageUniqueID));

    //Baseline TIFF tags used by EXIF
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeMake));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeModel));
    }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*
 * jdatasrc.c
 *
 * Copyright (C) 1994, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains decompression data source routines for the case of
 * reading JPEG data from a file (or any stdio stream).  While these routines
 * are sufficient for most applications, some will want to use a different
 * source manager.
 * IMPORTANT: we assume that fread() will correctly transcribe an array of
 * JOCTETs from 8-bit-wide elements on external storage.  If char is wider
 * than 8 bits on your machine, you may need to do some tweaking.
 */

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */



/* Expanded data source object for stdio input */

typedef struct {
    struct jpeg_source_mgr pub;                /* public fields */

    JOCTET*                   buffer;            /* start of buffer */
    boolean                   start_of_file;    /* have we gotten any data yet? */
    HFCBinStream*             pInfile;        /* source stream */
    } my_source_mgr;

typedef my_source_mgr* my_src_ptr;

#define INPUT_BUF_SIZE  4096    /* choose an efficiently fread'able size */


/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

METHODDEF (void)
init_source (j_decompress_ptr cinfo)
    {
    my_src_ptr src = (my_src_ptr) cinfo->src;

    /* We reset the empty-input-file flag for each image,
     * but we don't clear the input buffer.
     * This is correct behavior for reading a series of images from one source.
     */
    src->start_of_file = true;
    }


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return true
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a false return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns false.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

METHODDEF (boolean)
fill_input_buffer (j_decompress_ptr cinfo)
    {
    my_src_ptr src = (my_src_ptr) cinfo->src;
    size_t nbytes;

    nbytes = src->pInfile->Read(src->buffer, INPUT_BUF_SIZE);

    if (nbytes <= 0) {
        if (src->start_of_file)    /* Treat empty input file as fatal error */
            ERREXIT(cinfo, JERR_INPUT_EMPTY);
        WARNMS(cinfo, JWRN_JPEG_EOF);
        /* Insert a fake EOI marker */
        src->buffer[0] = (JOCTET) 0xFF;
        src->buffer[1] = (JOCTET) JPEG_EOI;
        nbytes = 2;
        }

    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = nbytes;
    src->start_of_file = false;

    return true;
    }


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

METHODDEF (void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
    {
    my_src_ptr src = (my_src_ptr) cinfo->src;

    /* Just a dumb implementation for now.  Could use fseek() except
     * it doesn't work on pipes.  Not clear that being smart is worth
     * any trouble anyway --- large skips are infrequent.
     */
    if (num_bytes > 0) {
        while (num_bytes > (long) src->pub.bytes_in_buffer) {
            num_bytes -= (long) src->pub.bytes_in_buffer;
            (void) fill_input_buffer(cinfo);
            /* note we assume that fill_input_buffer will never return false,
             * so suspension need not be handled.
             */
            }
        src->pub.next_input_byte += (size_t) num_bytes;
        src->pub.bytes_in_buffer -= (size_t) num_bytes;
        }
    }


/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF (void)
term_source (j_decompress_ptr cinfo)
    {
    /* no work necessary here */
    }


/*
 * Prepare for input from a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */

GLOBAL (void)
HRF_jpeg_stdio_src (j_decompress_ptr cinfo, HFCBinStream* infile)
    {
    my_src_ptr src;

    /* The source object and input buffer are made permanent so that a series
     * of JPEG images can be read from the same file by calling jpeg_stdio_src
     * only before the first one.  (If we discarded the buffer at the end of
     * one image, we'd likely lose the start of the next one.)
     * This makes it unsafe to use this manager and a different source
     * manager serially with the same JPEG object.  Caveat programmer.
     */
    if (cinfo->src == NULL) {    /* first time for this JPEG object? */
        cinfo->src = (struct jpeg_source_mgr*)
                     (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                                 sizeof(my_source_mgr));
        src = (my_src_ptr) cinfo->src;
        src->buffer = (JOCTET*)
                      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                                  INPUT_BUF_SIZE * sizeof(JOCTET));
        }

    src = (my_src_ptr) cinfo->src;
    src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_input_buffer;
    src->pub.skip_input_data = skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
    src->pub.term_source = term_source;
    src->pInfile = infile;
    src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
    src->pub.next_input_byte = NULL; /* until buffer loaded */
    }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*
 * jdatadst.c
 *
 * Copyright (C) 1994, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains compression data destination routines for the case of
 * emitting JPEG data to a file (or any stdio stream).  While these routines
 * are sufficient for most applications, some will want to use a different
 * destination manager.
 * IMPORTANT: we assume that fwrite() will correctly transcribe an array of
 * JOCTETs into 8-bit-wide elements on external storage.  If char is wider
 * than 8 bits on your machine, you may need to do some tweaking.
 */

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */

/* Expanded data destination object for stdio output */

typedef struct {
    struct jpeg_destination_mgr pub; /* public fields */

    JOCTET*             buffer;       /* start of buffer */
    HFCBinStream*       pOutfile;    /* target stream */
    } my_destination_mgr;

typedef my_destination_mgr* my_dest_ptr;

#define OUTPUT_BUF_SIZE  4096    /* choose an efficiently fwrite'able size */


/*
 * Initialize destination --- called by jpeg_start_compress
 * before any data is actually written.
 */

METHODDEF (void)
init_destination (j_compress_ptr cinfo)
    {
    my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

    /* Allocate the output buffer --- it will be released when done with image */
    dest->buffer = (JOCTET*)
                   (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
                                               OUTPUT_BUF_SIZE * sizeof(JOCTET));

    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
    }


/*
 * Empty the output buffer --- called whenever buffer fills up.
 *
 * In typical applications, this should write the entire output buffer
 * (ignoring the current state of next_output_byte & free_in_buffer),
 * reset the pointer & count to the start of the buffer, and return true
 * indicating that the buffer has been dumped.
 *
 * In applications that need to be able to suspend compression due to output
 * overrun, a false return indicates that the buffer cannot be emptied now.
 * In this situation, the compressor will return to its caller (possibly with
 * an indication that it has not accepted all the supplied scanlines).  The
 * application should resume compression after it has made more room in the
 * output buffer.  Note that there are substantial restrictions on the use of
 * suspension --- see the documentation.
 *
 * When suspending, the compressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_output_byte & free_in_buffer
 * indicate where the restart point will be if the current call returns false.
 * Data beyond this point will be regenerated after resumption, so do not
 * write it out when emptying the buffer externally.
 */

METHODDEF (boolean)
empty_output_buffer (j_compress_ptr cinfo)
    {
    my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

    if (dest->pOutfile->Write(dest->buffer, OUTPUT_BUF_SIZE) !=
        (size_t) OUTPUT_BUF_SIZE)
        ERREXIT(cinfo, JERR_FILE_WRITE);

    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

    return true;
    }


/*
 * Terminate destination --- called by jpeg_finish_compress
 * after all data has been written.  Usually needs to flush buffer.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF (void)
term_destination (j_compress_ptr cinfo)
    {
    my_dest_ptr dest = (my_dest_ptr) cinfo->dest;
    HASSERT_X64(OUTPUT_BUF_SIZE - dest->pub.free_in_buffer < ULONG_MAX);
    uint32_t datacount = (uint32_t)(OUTPUT_BUF_SIZE - dest->pub.free_in_buffer);

    /* Write any data remaining in the buffer */
    if (datacount > 0) {
        if (dest->pOutfile->Write(dest->buffer, datacount) != datacount)
            ERREXIT(cinfo, JERR_FILE_WRITE);
        }
    /* Make sure we wrote the output file OK */
    dest->pOutfile->Flush();

    // ynm disabled if (ferror(dest->outfile))
    //  ynm disabled ERREXIT(cinfo, JERR_FILE_WRITE);
    }

/*
 * Prepare for output to a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing compression.
 */

GLOBAL (void)
HRF_jpeg_stdio_dest (j_compress_ptr cinfo, HFCBinStream* outfile)
    {
    my_dest_ptr dest;

    /* The destination object is made permanent so that multiple JPEG images
     * can be written to the same file without re-executing jpeg_stdio_dest.
     * This makes it dangerous to use this manager and a different destination
     * manager serially with the same JPEG object, because their private object
     * sizes may be different.  Caveat programmer.
     */
    if (cinfo->dest == NULL) {    /* first time for this JPEG object? */
        cinfo->dest = (struct jpeg_destination_mgr*)
                      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                                  sizeof(my_destination_mgr));
        }

    dest = (my_dest_ptr) cinfo->dest;
    dest->pub.init_destination = init_destination;
    dest->pub.empty_output_buffer = empty_output_buffer;
    dest->pub.term_destination = term_destination;
    dest->pOutfile = outfile;
    }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


HFC_IMPLEMENT_SINGLETON(HRFJpegCreator)

//-----------------------------------------------------------------------------
// Creator
// This is the creator to instantiate Jpeg format
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Friend
// JPEGErrorExit
//-----------------------------------------------------------------------------
METHODDEF (void) HRFJpegIsKindOfErrorExit(j_common_ptr cinfo)
    {
    struct HRFJpegFileIsKindOfErrorManager* pErrorManager;

    // The IJG JPEG Library wants us to display the error message
    // and to exit.
    // Instead, we will longjump back to the execution error and
    // throw an exception

    //cinfo->err really points to a HRFJpegFileErrorManager, so coerce pointer
    pErrorManager = (struct HRFJpegFileIsKindOfErrorManager*) cinfo->err;

    // call a Jpeg exception
    HRFJpegFile::ThrowExBasedOnJPGErrCode(pErrorManager->pub.msg_code,
                                          pErrorManager->m_Url);
    }


HRFJpegCreator::HRFJpegCreator()
    : HRFRasterFileCreator(HRFJpegFile::CLASS_ID)
    {
    // JPEG capabilities instance member initialization
    m_pCapabilities = 0;
    }

// Identification information
WString HRFJpegCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_Jpeg()); //JPEG File Format
    }

// Identification information
WString HRFJpegCreator::GetSchemes() const
    {
    return HFCURLFile::s_SchemeName() + L";" + HFCURLMemFile::s_SchemeName();
    }

// Identification information
WString HRFJpegCreator::GetExtensions() const
    {
    return WString(L"*.jpg;*.jpeg;*.jpe;*.jfif");
    }

// allow to Open an image file
HFCPtr<HRFRasterFile> HRFJpegCreator::Create(
    const HFCPtr<HFCURL>& pi_rpURL,
    HFCAccessMode         pi_AccessMode,
    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFJpegFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

// Opens the file and verifies if it is the right type
bool HRFJpegCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                   uint64_t             pi_Offset) const
    {
    bool                   bResult = false;
    HAutoPtr<HFCBinStream> pFile;
    Byte                   Marker[4];

    HPRECONDITION(pi_rpURL != 0);

    //TFS#132036: Sharing control is not thread-safe and we do not understand why it would be required.

    //            disable it for now.

    //(const_cast<HRFJpegCreator*>(this))->SharingControlCreate(pi_rpURL);

    //HFCLockMonitor SisterFileLock(GetLockManager());


    // A JPEG image starts with the SOI segment which contain only the
    // SOI marker, hex FF D8
    // The SOI segment is followed by a misc marker (which starts with FF)
    // So if the first three bytes are hex FF D8 FF , we know that it is
    // a JPEG file.
    // Open the JPEG File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile != 0 && pFile->GetLastException() == 0)
        {
        // File exists. Check format
        pFile->SeekToBegin();

        // read the first 4 bytes of the file & close it
        if (pFile->Read(Marker, 4) != 4)
            goto WRAPUP;

        // verify if the four bytes have the content described above
        bResult = true; // Innocent until found guilty ;)

        // verify if first byte is FF
        if (Marker[0] != (Byte) 0xff)
            bResult = false;

        // verify if second byte is D8
        if (Marker[1] != (Byte) 0xd8)
            bResult = false;

        // verify if third byte is FF
        if (Marker[2] != (Byte) 0xff)
            bResult = false;

        if (bResult == true)
            {
            // Test the pixel type of the current file
            // rewind to the begin
            pFile->SeekToBegin();

            // create the decompression structure
            HAutoPtr<struct jpeg_decompress_struct> pDecompress;
            pDecompress = new struct jpeg_decompress_struct;
            HASSERT(pDecompress != 0);
            memset(pDecompress, 0, sizeof(struct jpeg_decompress_struct));

            /*ORI
            // set the jpeg error manager to a standard
            struct jpeg_error_mgr pub;
            pDecompress->err = jpeg_std_error(&pub);
            pub.error_exit = HRFJpegIsKindOfErrorExit;
            */
            HRFJpegFileIsKindOfErrorManager ErrManager;
            ErrManager.m_Url = pi_rpURL->GetURL();

            // set the jpeg error manager to a standard
            pDecompress->err = jpeg_std_error(&(ErrManager.pub));
            ErrManager.pub.error_exit = HRFJpegIsKindOfErrorExit;

            // Create the decompression object
            // The error handlers must be set before the call to the create
            // functions, because they can fail and the handler will be called
            jpeg_create_decompress(pDecompress);

            // Assign the file as the source of the decompress struct
            HRF_jpeg_stdio_src(pDecompress, pFile);

            // Reading the header

            // Read the header of the file and abort the decompression process.
            // We can always re-start it later.  That way, we will have all the
            // information about the file in the Decompression struct of the
            // m_Jpeg member
            jpeg_read_header(pDecompress, true);

            // Compute the output parameters for the JPEG
            //jpeg_start_decompress(pDecompress);

            if ((pDecompress->out_color_space != JCS_GRAYSCALE) &&
                (pDecompress->out_color_space != JCS_RGB)       &&
                (pDecompress->out_color_space != JCS_CMYK) )
                {
                bResult = false;
                }

            // destroy the decompression object
            //jpeg_finish_decompress(pDecompress);
            jpeg_destroy_decompress(pDecompress);
            }
        }

WRAPUP:
    //TFS#132036: Sharing control is not thread-safe and we do not understand why it would be required.

    //            disable it for now.

    //SisterFileLock.Release();

    //HASSERT(!(const_cast<HRFJpegCreator*>(this))->m_pSharingControl->IsLocked());

    //(const_cast<HRFJpegCreator*>(this))->m_pSharingControl = 0;


    return bResult;
    }

// Create or get the singleton capabilities of JPEG file.
const HFCPtr<HRFRasterFileCapabilities>& HRFJpegCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFJpegCapabilities();

    return m_pCapabilities;
    }


// This macro return the number of complete elements
#define COMPLETE_ELEMENTS(x, y)    ((x / y) + ((x%y) ? 1 : 0))


#if defined(_WIN32)
#pragma warning(disable: 4505)      //  since VS2013: unreferenced local function has been removed
#endif                              // for the function HRFJpegErrorExit


//
//-----------------------------------------------------------------------------
// Friend
// JPEGErrorExit
//-----------------------------------------------------------------------------
METHODDEF(void) ImagePP::HRFJpegErrorExit(j_common_ptr cinfo)
    {
    struct HRFJpegFileErrorManager* pErrorManager;

    // The IJG JPEG Library wants us to display the error message
    // and to exit.
    // Instead, we will longjump back to the execution error and
    // throw an exception

    // cinfo->err really points to a HRFJpegFileErrorManager, so coerce pointer
    pErrorManager = (struct HRFJpegFileErrorManager*) cinfo->err;

    if (!pErrorManager->m_pJpegFile->IsDestroying())
        HRFJpegFile::ThrowExBasedOnJPGErrCode(pErrorManager->pub.msg_code,
                                              pErrorManager->m_pJpegFile->
                                              GetURL()->GetURL());
    }


//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFJpegFile::HRFJpegFile(const HFCPtr<HFCURL>& pi_rURL,
                         HFCAccessMode         pi_AccessMode,
                         uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_Destructor    = false;
    m_IsOpen        = false;
    m_pErrorManager = new HRFJpegFileErrorManager;

    m_IfCompressionNotTerminated = false;

    if (GetAccessMode().m_HasCreateAccess)
        {
        Create();
        }
    else
        {
        // if Open success and it is not a new file
        Open();
        // Create Page and Res Descriptors.
        CreateDescriptors();
        }
    }

//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFJpegFile::HRFJpegFile(const HFCPtr<HFCURL>& pi_rURL,
                         HFCAccessMode         pi_AccessMode,
                         uint64_t             pi_Offset,
                         bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_Destructor            = false;
    m_IsOpen                = false;
    m_pErrorManager  = new HRFJpegFileErrorManager;

    m_IfCompressionNotTerminated = false;
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFJpegFile::~HRFJpegFile()
    {
    m_Destructor = true;

    try
        {
        // Close the JPG file and initialize the Compression Structure
        SaveJpegFile(true);
        }
    catch(...)
        {
        // Simply stop exceptions in the destructor
        // We want to known if a exception is throw.
        HASSERT(0);
        }
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFJpegFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                         unsigned short pi_Resolution,
                                                         HFCAccessMode  pi_AccessMode)
    {
    // verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    return new HRFJpegLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFJpegFile::Save()
    {

    //Keep last file position
    uint64_t CurrentPos = m_pJpegFile->GetCurrentPos();

    SaveJpegFile(false);

    //Set back position
    m_pJpegFile->SeekToPos(CurrentPos);

    }

//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFJpegFile::GetFileCurrentSize() const
    {
    return HRFRasterFile::GetFileCurrentSize(m_pJpegFile);
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFJpegFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(CountPages() == 0);

    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);

    AssignPageToStruct();

    return true;
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFJpegFile::AssignPageToStruct()
{
    return AssignPageToStruct2(0);
}

bool HRFJpegFile::AssignPageToStruct2 (jpeg_compress_struct* pi_pTable)
{
    ////////////////////////////////////////
    // Set the compression options
    ////////////////////////////////////////

    // set the image size
    m_Jpeg.m_pCompress->image_width  = (uint32_t)GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth();
    m_Jpeg.m_pCompress->image_height = (uint32_t)GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight();

    // set the image color space
    if (GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType()->GetClassID() ==
        HRPPixelTypeV8Gray8::CLASS_ID)
        {
        m_Jpeg.m_pCompress->in_color_space = JCS_GRAYSCALE;
        m_Jpeg.m_pCompress->input_components = 1;
        }
    else if (GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType()->GetClassID() ==
             HRPPixelTypeV24R8G8B8::CLASS_ID)
        {
        m_Jpeg.m_pCompress->in_color_space = JCS_RGB;
        m_Jpeg.m_pCompress->input_components = 3;
        }

    /*
    HFCPtr<HRFLineCapability> pLineCapability = (HFCPtr<HRFLineCapability>& )GetCapabilities()->GetCapabilityOfType(HRFLineCapability::CLASS_ID, HFC_CREATE_ONLY);
    unsigned long MaxSizeInPixel = pLineCapability()->GetMaxSizeInBytes() / m_Jpeg.m_pCompress->input_components;

    if ((m_Jpeg.m_pCompress->image_width  > MaxSizeInPixel) ||
        (m_Jpeg.m_pCompress->image_height > MaxSizeInPixel))
         throw HFCFileNotCreatedException(GetURL()->GetURL());
    */

    // Set the default compression options (size and color space must be set)
    jpeg_set_defaults(m_Jpeg.m_pCompress);

    // Set the quality of the compression
    // The baseline parameter is always true because it makes a difference
    // only for low quality (<25) compressions that yield very small files.
    // See the IJG JPEG library reference to jpeg_set_quality for more info.

    // default quality
    const HFCPtr<HCDCodec>& pCodec = GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetCodec();
    HASSERT(pCodec->GetClassID() == HCDCodecIJG::CLASS_ID);
    jpeg_set_quality(m_Jpeg.m_pCompress, ((HFCPtr<HCDCodecIJG>&)pCodec)->GetQuality(), true);

    // Set the encoding optimization "default optimization false
    m_Jpeg.m_pCompress->optimize_coding = false;

    ////////////////////////////////////////
    // Start compression
    ////////////////////////////////////////

    // Resolution Tags
    HPMAttributeSet::HPMASiterator TagIterator; 
    for (TagIterator  = GetPageDescriptor(0)->GetTags().begin();
         TagIterator != GetPageDescriptor(0)->GetTags().end(); TagIterator++)
        {
        HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);

        if (GetPageDescriptor(0)->TagHasChanged(*pTag) || GetAccessMode().m_HasCreateAccess)
            {
            m_Jpeg.m_pCompress->write_JFIF_header = true;

            // RESOLUTIONUNIT Tag
            if (pTag->GetID() == HRFAttributeResolutionUnit::ATTRIBUTE_ID)
                m_Jpeg.m_pCompress->density_unit = (Byte)((HFCPtr<HRFAttributeResolutionUnit>&)pTag)->GetData() - 1;

            // XRESOLUTION Tag
            else if (pTag->GetID() == HRFAttributeXResolution::ATTRIBUTE_ID)
                m_Jpeg.m_pCompress->X_density = (uint16_t)((HFCPtr<HRFAttributeXResolution>&)pTag)->GetData();

            // YRESOLUTION Tag
            else if (pTag->GetID() ==HRFAttributeYResolution::ATTRIBUTE_ID)
                m_Jpeg.m_pCompress->Y_density = (uint16_t)((HFCPtr<HRFAttributeYResolution>&)pTag)->GetData();
        }
    }

    // Set Quant tables - to keep same compression quality
    if(pi_pTable != 0)
    {
        JQUANT_TBL** qtblptr;
        int tblno;
        for (tblno = 0; tblno < NUM_QUANT_TBLS; tblno++) 
        {
            if (pi_pTable->quant_tbl_ptrs[tblno] != 0) 
            {
                qtblptr = &m_Jpeg.m_pCompress->quant_tbl_ptrs[tblno];
                if (*qtblptr == 0)
                    *qtblptr = jpeg_alloc_quant_table((j_common_ptr)m_Jpeg.m_pCompress);
                 BeStringUtilities::Memcpy((*qtblptr)->quantval, sizeof((*qtblptr)->quantval), 
                         pi_pTable->quant_tbl_ptrs[tblno]->quantval, sizeof((*qtblptr)->quantval));
                (*qtblptr)->sent_table = FALSE;
            }
        }
    }

    jpeg_start_compress(m_Jpeg.m_pCompress, true);

    return true;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFJpegFile::GetCapabilities () const
    {
    return (HRFJpegCreator::GetInstance()->GetCapabilities());
    }


//-----------------------------------------------------------------------------
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFJpegFile::Open()
    {
    // Open the file
    if (!m_IsOpen)
        {
        // Open the actual jpeg file specified in the parameters.  The library
        // uses stdio FILE*, so open the file and satisfy the library
        m_pJpegFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

        // create the decompression structure
        m_Jpeg.m_pDecompress = new struct jpeg_decompress_struct;
        HASSERT(m_Jpeg.m_pDecompress != 0);
        memset(m_Jpeg.m_pDecompress, 0, sizeof(struct jpeg_decompress_struct));

        // initialize the compression structure to 0.
        m_Jpeg.m_pCompress   = 0;

        ////////////////////////////////////////
        // Set a custom error handler
        ////////////////////////////////////////

        // Set custom error handlers in the compression/decompression
        // structure of the m_Jpeg member.  This is done to avoid using
        // the default handlers which use exit() when an error occurs

        // set the object pointer in the error manager
        m_pErrorManager->m_pJpegFile = this;

        // set the jpeg error manager to a standard
        m_Jpeg.m_pDecompress->err = jpeg_std_error(&(m_pErrorManager->pub));
        m_pErrorManager->pub.error_exit = HRFJpegErrorExit;


        ////////////////////////////////////////
        // Create the decompression object
        ////////////////////////////////////////

        // The error handlers must be set before the call to the create
        // functions, because they can fail and the handler will be called
        jpeg_create_decompress(m_Jpeg.m_pDecompress);


        ////////////////////////////////////////
        // Assign the opened file to the jpeg object
        ////////////////////////////////////////

        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        // Lock the sister file for the read_header operation
        HFCLockMonitor SisterFileLock (GetLockManager());

        // Assign the file as the source of the decompress struct
        // of the m_Jpeg member
        HRF_jpeg_stdio_src(m_Jpeg.m_pDecompress, m_pJpegFile);

        ////////////////////////////////////////
        // Reading the header
        ////////////////////////////////////////

        // Read the header of the file and abort the decompression process.
        // We can always re-start it later.  That way, we will have all the
        // information about the file in the Decompression struct of the
        // m_Jpeg member
        jpeg_read_header(m_Jpeg.m_pDecompress, true);

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        ////////////////////////////////////////
        // start the decompression
        ////////////////////////////////////////

        // Compute the output parameters for the JPEG
        jpeg_start_decompress(m_Jpeg.m_pDecompress);
        m_IsOpen = true;
        }

    return true;
    }


//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFJpegFile::CreateDescriptors ()
    {
    // Allow to obtain the width of the resolution
    uint32_t Width;

    // Verify if the file is in compress or decompress
    if (m_Jpeg.m_pDecompress)
        Width = (m_Jpeg.m_pDecompress->output_width);
    else
        Width = (m_Jpeg.m_pCompress->image_width);

    // Allow to obtain the height of the resolution
    uint32_t Height;

    // Verify if the file is in compress or decompress
    if (m_Jpeg.m_pDecompress)
        Height = (m_Jpeg.m_pDecompress->output_height);
    else
        Height = (m_Jpeg.m_pCompress->image_height);

    // Create Page and Resolution Description/Capabilities for this file
    HFCPtr<HRFResolutionDescriptor>     pResolution;
    HFCPtr<HRFPageDescriptor>           pPage;

    pResolution =  new HRFResolutionDescriptor(
        GetAccessMode(),                                // AccessMode,
        GetCapabilities(),                              // Capabilities,
        1.0,                                            // XResolutionRatio,
        1.0,                                            // YResolutionRatio,
        CreatePixelTypeFromFile(),                      // PixelType,
        new HCDCodecIJG(),                              // Codec,
        HRFBlockAccess::SEQUENTIAL,                     // RBlockAccess,
        HRFBlockAccess::SEQUENTIAL,                     // WBlockAccess,
        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
        HRFInterleaveType::PIXEL,                       // InterleaveType
        false,                                          // IsInterlace,
        Width,                                          // Width,
        Height,                                         // Height,
        Width,                                          // BlockWidth,
        1,                                              // BlockHeight,
        0,                                              // BlocksDataFlag
        HRFBlockType::LINE);

    // JFIF APP0 marker was found
    HPMAttributeSet TagList;
    if(m_Jpeg.m_pDecompress->saw_JFIF_marker && (m_Jpeg.m_pDecompress->density_unit != 0))
        {
        HFCPtr<HPMGenericAttribute> pTag;

        // RESOLUTIONUNIT Tag
        pTag = new HRFAttributeResolutionUnit((unsigned short)(m_Jpeg.m_pDecompress->density_unit + 1));
        TagList.Set(pTag);

        // XRESOLUTION Tag
        pTag = new HRFAttributeXResolution((double)(m_Jpeg.m_pDecompress->X_density));
        TagList.Set(pTag);

        // YRESOLUTION Tag
        pTag = new HRFAttributeYResolution((double)(m_Jpeg.m_pDecompress->Y_density));
        TagList.Set(pTag);
        }

    GetExifTags(true, true, TagList);

    pPage = new HRFPageDescriptor (GetAccessMode(),
                                   GetCapabilities(),   // Capabilities,
                                   pResolution,         // ResolutionDescriptor,
                                   0,                   // RepresentativePalette,
                                   0,                   // Histogram,
                                   0,                   // Thumbnail,
                                   0,                   // ClipShape,
                                   0,                   // TransfoModel,
                                   0,                   // Filters
                                   &TagList);

    m_ListOfPageDescriptor.push_back(pPage);
    }


//-----------------------------------------------------------------------------
// Private
// This method saves and close the file if needed.
//-----------------------------------------------------------------------------
void HRFJpegFile::SaveJpegFile(bool pi_CloseFile)
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // is thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {

        // if this is a decompression object
        if (m_Jpeg.m_pDecompress)
            {

            if(pi_CloseFile)
                {
                // destroy the decompression object
                jpeg_destroy_decompress(m_Jpeg.m_pDecompress);
                delete m_Jpeg.m_pDecompress;
                m_Jpeg.m_pDecompress = 0;
                }

            }
        // if this is a compression object
        else if (m_Jpeg.m_pCompress)
            {

            // Lock the sister file for the jpeg_finish_compress method
            HFCLockMonitor SisterFileLock (GetLockManager());

            // Check if the image data was saved, cause we can't finish the
            // compression if no data was saved.
            if (m_IfCompressionNotTerminated && m_Jpeg.m_pCompress->image_height == m_Jpeg.m_pCompress->next_scanline)
                {
                jpeg_finish_compress(m_Jpeg.m_pCompress);
                m_IfCompressionNotTerminated = false;
                }


            if(pi_CloseFile)
                {
                // Unlock the sister file.
                SisterFileLock.ReleaseKey();

                // destroy the decompression object
                jpeg_destroy_compress(m_Jpeg.m_pCompress);
                delete m_Jpeg.m_pCompress;
                m_Jpeg.m_pCompress = 0;
                }
            }

        if(pi_CloseFile)
            {
            m_pJpegFile = 0;
            m_IsOpen = false;
            }
        else
            {
            m_pJpegFile->Flush();
            }
        }
    }

//-----------------------------------------------------------------------------
// Private
// This method create the file.
//-----------------------------------------------------------------------------
bool HRFJpegFile::Create()
    {

    // Open the actual jpeg file specified in the parameters.  The library
    // uses stdio FILE*, so open the file and satisfy the library
    m_pJpegFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

    // initialize the decompression structure to 0.
    m_Jpeg.m_pDecompress = 0;

    // create the compression structure.
    m_Jpeg.m_pCompress = new struct jpeg_compress_struct;
    HASSERT(m_Jpeg.m_pCompress != 0);
    memset(m_Jpeg.m_pCompress, 0, sizeof(struct jpeg_compress_struct));


    ////////////////////////////////////////
    // Set a custom error handler
    ////////////////////////////////////////

    // Set custom error handlers in the compression/decompression
    // structure of the m_Jpeg member.  This is done to avoid using
    // the default handlers which use exit() when an error occurs

    // set the object pointer in the error manager
    m_pErrorManager->m_pJpegFile = this;

    // set the jpeg error manager to a standard
    m_Jpeg.m_pCompress->err = jpeg_std_error(&(m_pErrorManager->pub));
    m_pErrorManager->pub.error_exit = HRFJpegErrorExit;


    ////////////////////////////////////////
    // Create the Compression object
    ////////////////////////////////////////

    // The error handlers must be set before the call to the create
    // functions, because they can fail and the handler will be called
    jpeg_create_compress(m_Jpeg.m_pCompress);
    m_IfCompressionNotTerminated = true;

    ////////////////////////////////////////
    // Assign the opened file to the jpeg object
    ////////////////////////////////////////

    // This creates the sister file for file sharing control if necessary.
    SharingControlCreate();

    // Lock the sister file for the read_header operation
    HFCLockMonitor SisterFileLock (GetLockManager());

    // Assign the file as the destination of the compress struct
    // of the m_Jpeg member
    HRF_jpeg_stdio_dest(m_Jpeg.m_pCompress, m_pJpegFile);

    SisterFileLock.ReleaseKey();

    m_IsOpen = true;
    return true;
    }

//-----------------------------------------------------------------------------
// Private
// CreatePixelTypeFromFile
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRFJpegFile::CreatePixelTypeFromFile() const
    {
    HFCPtr<HRPPixelType> pPixelType;

    // Verify if using a compression or decompression object
    if (m_Jpeg.m_pDecompress)
        {
        // The decompression structure has a member named out_color_space.
        // The value of this member tells us what color space is used
        // in the file.
        // Those values are:
        //
        //      JCS_UNKNOWN,    /* error/unspecified */
        //      JCS_GRAYSCALE,  /* monochrome */
        //      JCS_RGB,        /* red/green/blue */
        //      JCS_YCbCr,      /* Y/Cb/Cr (also known as YUV) */
        //      JCS_CMYK,       /* C/M/Y/K */
        //      JCS_YCCK        /* Y/Cb/Cr/K */

        // Build the pixel type based on the color space
        switch(m_Jpeg.m_pDecompress->out_color_space)
            {
            case JCS_GRAYSCALE :

                // Set a grayscale pixel type with the grayscale channel
                // org object use the IJG JPEG library BITS_IN_JSAMPLE
                // to specify the number of bits the grayscale channel
                pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgGray(BITS_IN_JSAMPLE,
                                                                                          HRPChannelType::UNUSED,
                                                                                          HRPChannelType::VOID_CH,
                                                                                          0),
                                                                        0);
                break;

            case JCS_CMYK :
            case JCS_RGB  :

                // We now must determine if the jpeg uses a colormap (a palette)
                // or if it uses full color (RGB)
                // This is specified by the output_components member of the
                // decompress structure.  If this member is 1, the image uses
                // a colormap, otherwise it is equal to the out_color_components


                // Verifying if a palette is used
                if (m_Jpeg.m_pDecompress->output_components == 1)
                    {
                    // create the pixel type for a RGB with
                    // an BITS_IN_JSAMPLE index
                    pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgRGB(BITS_IN_JSAMPLE,
                                                                                              BITS_IN_JSAMPLE,
                                                                                              BITS_IN_JSAMPLE,
                                                                                              0,
                                                                                              HRPChannelType::UNUSED,
                                                                                              HRPChannelType::VOID_CH,
                                                                                              0),
                                                                             BITS_IN_JSAMPLE);

                    // Now, set the palette
                        {
                        int32_t             Index;
                        HRPPixelPalette*    pPalette;
                        Byte               Value[3];

                        // get the palette from the pixel type
                        pPalette = (HRPPixelPalette*)&(pPixelType->GetPalette());

                        // The maximum number of colors is the decompress
                        // structure actual_number_of_colors member
                        // copy the jpeg's colormap to the palette
                        for (Index = 0;
                             Index < m_Jpeg.m_pDecompress->actual_number_of_colors;
                             Index++)
                            {
                            // get the current palette entry from the jpeg's colormap
                            Value[0] = (Byte) m_Jpeg.m_pDecompress->colormap[0][Index];
                            Value[1] = (Byte) m_Jpeg.m_pDecompress->colormap[1][Index];
                            Value[2] = (Byte) m_Jpeg.m_pDecompress->colormap[2][Index];

                            // add that entry to the pixel palette
                            pPalette->SetCompositeValue(Index, Value);
                            }
                        } // setting palette
                    } // Palette mode
                else
                    // Verifying is full color mode is used
                    if (m_Jpeg.m_pDecompress->output_components ==
                        m_Jpeg.m_pDecompress->out_color_components)
                        {
                        // Verify that the number of color components is 3.
                        // Set a full color pixel type with a RGB channel
                        // org.  Use the BITS_IN_JSAMPLE to specify the
                        // size of each channel
                        if ( (m_Jpeg.m_pDecompress->out_color_components == 3) ||
                             (m_Jpeg.m_pDecompress->out_color_space == JCS_CMYK) )
                            {
                            HASSERT (BITS_IN_JSAMPLE == 8);

                            pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgRGB(BITS_IN_JSAMPLE,
                                                                                                     BITS_IN_JSAMPLE,
                                                                                                     BITS_IN_JSAMPLE,
                                                                                                     0,
                                                                                                     HRPChannelType::UNUSED,
                                                                                                     HRPChannelType::VOID_CH,
                                                                                                     0),
                                                                                    0);
                            }
                        } // full color mode
                break;

            default:
                break;
            }
        }
    else
        {
        // The compression structure has a member named in_color_space.
        // The value of this member tells us what color space is used
        // in the file.
        // Those values are:
        //
        //      JCS_UNKNOWN,    /* error/unspecified */
        //      JCS_GRAYSCALE,  /* monochrome */
        //      JCS_RGB,        /* red/green/blue */
        //      JCS_YCbCr,      /* Y/Cb/Cr (also known as YUV) */
        //      JCS_CMYK,       /* C/M/Y/K */
        //      JCS_YCCK        /* Y/Cb/Cr/K */

        // Build the pixel type based on the color space
        switch(m_Jpeg.m_pCompress->in_color_space)
            {
            case JCS_GRAYSCALE:

                // Set a grayscale pixel type with the grayscale channel
                // org object use the IJG JPEG library BITS_IN_JSAMPLE
                // to specify the number of bits the grayscale channel
                pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgGray(BITS_IN_JSAMPLE,
                                                                                          HRPChannelType::UNUSED,
                                                                                          HRPChannelType::VOID_CH,
                                                                                          0),
                                                                        0);
                break;

            case JCS_RGB:

                pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgRGB(BITS_IN_JSAMPLE,
                                                                                         BITS_IN_JSAMPLE,
                                                                                         BITS_IN_JSAMPLE,
                                                                                         0,
                                                                                         HRPChannelType::UNUSED,
                                                                                         HRPChannelType::VOID_CH,
                                                                                         0),
                                                                        0);
            default:
                break;
            }
        }

    return (pPixelType);
    }

//-----------------------------------------------------------------------------
// Public
// ThrowExBasedOnJPGErrorCode
//-----------------------------------------------------------------------------
void HRFJpegFile::ThrowExBasedOnJPGErrCode(uint32_t pi_GetLastErrorCode,
                                           const WString& pi_rUrl)
    {
    switch (pi_GetLastErrorCode)
        {
        //no in jpeglib v8d case JERR_ARITH_NOTIMPL : //Sorry, there are legal restrictions on arithmetic coding
        case JERR_BAD_ALIGN_TYPE : //ALIGN_TYPE is wrong, please fix
        case JERR_BAD_ALLOC_CHUNK : //MAX_ALLOC_CHUNK is wrong, please fix
        case JERR_BAD_BUFFER_MODE : //Bogus buffer control mode
        case JERR_BAD_COMPONENT_ID : //Invalid component ID %d in SOS
        case JERR_BAD_HUFF_TABLE : //Bogus Huffman table definition
        case JERR_BAD_IN_COLORSPACE : //Bogus input colorspace
        case JERR_BAD_J_COLORSPACE : //Bogus JPEG colorspace
        case JERR_BAD_LENGTH : //Bogus marker length
        case JERR_BAD_SAMPLING : //Bogus sampling factors
        case JERR_CCIR601_NOTIMPL : //CCIR601 sampling not implemented yet
        case JERR_HUFF_MISSING_CODE : //Missing Huffman code table entry
        case JERR_NO_SOI : //Not a JPEG file: starts with 0x%02x 0x%02x
        case JERR_SOF_DUPLICATE : //Invalid JPEG file structure: two SOF markers
        case JERR_SOF_NO_SOS : //Invalid JPEG file structure: missing SOS marker
        case JERR_SOI_DUPLICATE : //Invalid JPEG file structure: two SOI markers
        case JERR_SOS_NO_SOF : //Invalid JPEG file structure: SOS before SOF
            throw HFCCorruptedFileException(pi_rUrl);
            break;
        case JERR_BAD_DCT_COEF : //DCT coefficient out of range
        case JERR_QUANT_FEW_COLORS : //Cannot quantize to fewer than %d colors
        case JERR_QUANT_MANY_COLORS : //Cannot quantize to more than %d colors
            throw HFCFileOutOfRangeException(pi_rUrl);
            break;
        case JERR_BAD_DCTSIZE : //IDCT output block size %d not supported
        case JERR_BAD_PRECISION : //Unsupported JPEG data precision %d
        case JERR_CONVERSION_NOTIMPL : //Unsupported color conversion request
        case JERR_EMPTY_IMAGE : //Empty JPEG image (DNL not supported)
        case JERR_FRACT_SAMPLE_NOTIMPL : //Fractional sampling not implemented yet
        case JERR_NOTIMPL : //Not implemented yet
        case JERR_NOT_COMPILED : //Requested feature was omitted at compile time
        case JERR_NO_BACKING_STORE : //Backing store not supported
        case JERR_NO_IMAGE : //JPEG datastream contains no image
        case JERR_SOF_UNSUPPORTED : //Unsupported JPEG process: SOF type 0x%02x
        case JERR_UNKNOWN_MARKER : //Unsupported marker type 0x%02x
            throw HFCFileNotSupportedException(pi_rUrl);
            break;
        case JERR_FILE_READ : //Input file read error
        case JERR_XMS_READ : //Read from XMS failed
        case JERR_EMS_READ : //Read from EMS failed
            throw HFCReadFaultException(pi_rUrl);
            break;
        case JERR_XMS_WRITE : //Write to XMS failed
        case JERR_EMS_WRITE : //Write to EMS failed
        case JERR_FILE_WRITE : //Output file write error --- out of disk space?
            throw HFCWriteFaultException(pi_rUrl);
            break;
        case JERR_HUFF_CLEN_OVERFLOW : //Huffman code size table overflow
        case JERR_IMAGE_TOO_BIG : //Maximum supported image dimension is %u pixels
        case JERR_WIDTH_OVERFLOW : //Image too wide for this implementation
            throw HFCFileOutOfRangeException(pi_rUrl);
            break;
        case JERR_OUT_OF_MEMORY : //Insufficient memory (case %d)
            throw HFCOutOfMemoryException();
            break;
        case JERR_QUANT_COMPONENTS : //Cannot quantize more than %d color components
            throw HRFPixelTypeNotSupportedException(pi_rUrl);
            break;
        case JERR_TFILE_CREATE : //Failed to create temporary file %s
        case JERR_TFILE_READ : //Read failed on temporary file
        case JERR_TFILE_SEEK : //Seek failed on temporary file
        case JERR_TFILE_WRITE : //Write failed on temporary file --- out of disk space?
        case JERR_BAD_LIB_VERSION : //Wrong JPEG library version: library is %d, caller expects %d
        case JERR_BAD_MCU_SIZE : //Sampling factors too large for interleaved scan
        case JERR_BAD_POOL_ID : //Invalid memory pool code %d
        case JERR_BAD_PROGRESSION : //Invalid progressive parameters Ss=%d Se=%d Ah=%d Al=%d
        case JERR_BAD_PROG_SCRIPT : //Invalid progressive parameters at scan script entry %d
        case JERR_BAD_SCAN_SCRIPT : //Invalid scan script at entry %d
        case JERR_BAD_STATE : //Improper call to JPEG library in state %d
        case JERR_BAD_STRUCT_SIZE : //JPEG parameter struct mismatch: library thinks size is %u, caller expects %u
        case JERR_BAD_VIRTUAL_ACCESS : //Bogus virtual array access
        case JERR_BUFFER_SIZE : //Buffer passed to JPEG library is too small
        case JERR_CANT_SUSPEND : //Suspension not allowed here
        case JERR_COMPONENT_COUNT : //Too many color components: %d, max %d
        case JERR_DAC_INDEX : //Bogus DAC index %d
        case JERR_DAC_VALUE : //Bogus DAC value 0x%x
        case JERR_DHT_INDEX : //Bogus DHT index %d
        case JERR_DQT_INDEX : //Bogus DQT index %d
        case JERR_EOI_EXPECTED : //Didn't expect more than one scan
        case JERR_INPUT_EMPTY : //Empty input file
        case JERR_INPUT_EOF : //Premature end of input file
        case JERR_MISMATCHED_QUANT_TABLE : //Cannot transcode due to multiple use of quantization table %d
        case JERR_MISSING_DATA : //Scan script does not transmit all data
        case JERR_MODE_CHANGE : //Invalid color quantization mode change
        case JERR_NO_HUFF_TABLE : //Huffman table 0x%02x was not defined
        case JERR_NO_QUANT_TABLE : //Quantization table 0x%02x was not defined
        case JERR_TOO_LITTLE_DATA : //Application transferred too few scanlines
        case JERR_VIRTUAL_BUG : //Virtual array controller messed up
        default :
            throw HRFGenericException(pi_rUrl);
            break;
        }
    }



//-----------------------------------------------------------------------------
// Public
// ThrowExBasedOnJPGErrorCode
//-----------------------------------------------------------------------------
void HRFJpegFile::GetExifTags(bool            pi_ExifTags,
                              bool            pi_ExifRelatedGPSTags,
                              HPMAttributeSet& po_rTags)
    {
    if(!GetURL()->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return;

    uint64_t LastPos = m_pJpegFile->GetCurrentPos();

    /* -------------------------------------------------------------------- */
    /*      Search for APP1 chunk.                                          */
    /* -------------------------------------------------------------------- */
    Byte Header[10];
    int32_t HeaderPos = 2;
    int32_t App1Offset;

    while (true)
        {
        m_pJpegFile->SeekToPos(HeaderPos);

        if (m_pJpegFile->GetCurrentPos() != HeaderPos)
            break;

        if (m_pJpegFile->Read(Header, sizeof(Header)) != sizeof(Header))
            break;

        if ((Header[0] != 0xFF) ||
            (Header[1] & 0xf0) != 0xe0)
            break; // Not an APP chunk.

        if ((Header[1] == 0xe1) &&
            strncmp((const char*) Header + 4,"Exif",4) == 0 )
            {
            App1Offset = HeaderPos + 10;

            HAutoPtr<HTIFFFile> pHTIFFFile(new HTIFFFile(static_cast<HFCURLFile*>(GetURL().GetPtr())->GetAbsoluteFileName(),
                                                         HFC_READ_ONLY | HFC_SHARE_READ_WRITE,
                                                         App1Offset,
                                                         false,
                                                         false));

            HASSERT(pHTIFFFile->GetFilePtr()->GetLastException() == 0);

            HTIFFError* pErr;
            pHTIFFFile->IsValid(&pErr);
            if ((pErr == 0) || !pErr->IsFatal())
                {
                if (pi_ExifTags == true)
                    {
                    pHTIFFFile->GetEXIFTags(0, po_rTags);
                    }

                if (pi_ExifRelatedGPSTags == true)
                    {
                    pHTIFFFile->GetEXIFDefinedGPSTags(0, po_rTags);
                    }

                // MAKE Tag
                char*                      pTagValue;
                HFCPtr<HPMGenericAttribute> pTag;

                if (pHTIFFFile->GetField(MAKE, &pTagValue))
                    {
                    pTag = new HRFAttributeMake(WString(pTagValue,false));
                    po_rTags.Set(pTag);
                    }

                // MODEL Tag
                if (pHTIFFFile->GetField(MODEL, &pTagValue))
                    {
                    pTag = new HRFAttributeModel(WString(pTagValue,false));
                    po_rTags.Set(pTag);
                    }
                }
            else
                {   //Cannot read the EXIF tags in APP1
                HASSERT(0);
                }

            break; // APP1 - Exif
            }

        HeaderPos += 2 + Header[2] * 256 + Header[3];
        }

    m_pJpegFile->SeekToPos(LastPos);
    }
//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFJpegFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }


//-----------------------------------------------------------------------------
// protected
// GetFilePtr   - Get the JPEG file pointer.
//-----------------------------------------------------------------------------
HRFJpegFile::JPEG* HRFJpegFile::GetFilePtr  ()
    {
    // The m_JPEG member is aggragated, return its address to conform
    // with the parent function declaration

    return (&m_Jpeg);
    }
