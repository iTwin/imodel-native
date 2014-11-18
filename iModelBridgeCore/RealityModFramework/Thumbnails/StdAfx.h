/*--------------------------------------------------------------------------------------+
|
|     $Source: Thumbnails/StdAfx.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <windows.h>

#include <Imagepp/h/hstdcpp.h>
#include <ImagePP/all/h/HFCMacros.h>
#include <ImagePP/all/h/HUTClassIDDescriptor.h>
// Supported codecs
#include <ImagePP\all\h\HCDCodecBMPRLE4.h>
#include <ImagePP\all\h\HCDCodecBMPRLE8.h>
#include <ImagePP\all\h\HCDCodecCCITT.h>
#include <ImagePP\all\h\HCDCodecCRL8.h>
#include <ImagePP\all\h\HCDCodecDeflate.h>
#include <ImagePP\all\h\HCDCodecFlashpix.h>
#include <ImagePP\all\h\HCDCodecFPXSingleColor.h>
#include <ImagePP\all\h\HCDCodecGIF.h>
#include <ImagePP\all\h\HCDCodecHMRCCITT.h>
#include <ImagePP\all\h\HCDCodecHMRGIF.h>
#include <ImagePP\all\h\HCDCodecHMRPackBits.h>
#include <ImagePP\all\h\HCDCodecHMRRLE1.h>
#include <ImagePP\all\h\HCDCodecIdentity.h>
#include <ImagePP\all\h\HCDCodecIJG.h>
#include <ImagePP\all\h\HCDCodecJPEG.h>
#include <ImagePP\all\h\HCDCodecLZW.h>
#include <ImagePP\all\h\HCDCodecPackBits.h>
#include <ImagePP\all\h\HCDCodecPCX.h>
#include <ImagePP\all\h\HCDCodecRLE1.h>
#include <ImagePP\all\h\HCDCodecRLE8.h>
#include <ImagePP\all\h\HCDCodecSingleColor.h>
#include <ImagePP\all\h\HCDCodecTGARLE.h>
#include <ImagePP\all\h\HCDCodecZlib.h>
#include <ImagePP\all\h\HCDCodecLRDRLE.h>
#include <ImagePP\all\h\HCDCodecFLIRLE8.h>
#include <ImagePP\all\h\HCDCodecJPEG2000.h>
#include <ImagePP\all\h\HCDCodecECW.h>

#ifdef _MANAGED
#error File type handlers cannot be built as managed assemblies.  Set the Common Language Runtime options to no CLR support in project properties.
#endif

#ifndef _UNICODE
#error File type handlers must be built Unicode.  Set the Character Set option to Unicode in project properties.
#endif

#include <Thumbnails/ThumbnailsApi.h>
#include <Thumbnails/RasterFileFormats.h>
#include "ThumbnailUtil.h"
#include "PointCloudVortex.h"

