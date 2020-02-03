/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageInsider/Labeler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// LABELER.h : header file
//

#ifndef __LABELER_H__
#define __LABELER_H__

#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HCDCodec.h>
#include <Imagepp/all/h/HRFRasterFile.h>

// Get information to string
Utf8String GetImageDimension(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber);
Utf8String GetUncompressedSize(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber);
Utf8String GetCompressionTypes(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber);
Utf8String GetColorSpace(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber);
Utf8String GetScanlineOrientation(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber);
Utf8String GetOverviewDimension(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber);
Utf8String GetNumberOfBlock(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber);
Utf8String GetBlockDimension(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber);
Utf8String GetFormatType(HFCPtr<HRFRasterFile>& pi_rpRasterFile);

// Convertion to string
Utf8String ConvertPixelTypeToString(HCLASS_ID pi_ClassKey);
Utf8String ConvertCodecToString(const HFCPtr<HCDCodec>& pi_rpCodec);
Utf8String ConvertFilterToString(HCLASS_ID pi_ClassKey);
Utf8String ConvertTransfoModelToString(HCLASS_ID pi_ClassKey);

#endif // __LABELER_H__
