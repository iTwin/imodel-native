/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageInsider/Labeler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// labeler
//

#include "stdafx.h"

#include "labeler.h"
#include <Imagepp/all/h/HUTClassIDDescriptor.h>

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HFCURL.h>

//-----------------------------------------------------------------------------
// Codec
//-----------------------------------------------------------------------------
#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecJPEG.h>


//-----------------------------------------------------------------------------

Utf8String GetImageDimension(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber)
{
    if ((pi_rpRasterFile->CountPages() > pi_PageNumber) && (pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions() > 0))
    {
       HFCPtr<HRFResolutionDescriptor> pResolution = pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->GetResolutionDescriptor(0);
            
       WChar buffer[200];
       _stprintf(buffer, _TEXT("%I64ux%I64u"), pResolution->GetWidth(), pResolution->GetHeight());
       return Utf8String(buffer);
    }
    else
        return "Unknown";
}

//-----------------------------------------------------------------------------

Utf8String GetUncompressedSize(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber)
{
    if ((pi_rpRasterFile->CountPages() > pi_PageNumber) && (pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions() > 0))
    {
        HFCPtr<HRFResolutionDescriptor> pResolution = pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->GetResolutionDescriptor(0);
            
       WChar buffer[200];

       if (pResolution->GetSizeInBytes() < 1024)
            _stprintf(buffer, _TEXT("%I64u bytes (%I64u bytes) - Main Image"), pResolution->GetSizeInBytes(), pResolution->GetSizeInBytes());
       else
           if (pResolution->GetSizeInBytes() < 1024 * 1000)
                _stprintf(buffer, _TEXT("%.1lf KB (%I64u bytes) - Main Image"), (double)pResolution->GetSizeInBytes() / (double)1024, pResolution->GetSizeInBytes());
           else
                _stprintf(buffer, _TEXT("%.2lf MB (%I64u bytes) - Main Image"), (double)pResolution->GetSizeInBytes() / ((double)1024 * (double)1000), pResolution->GetSizeInBytes());
       return Utf8String(buffer);
    }
    else
        return "Unknown";
}

//-----------------------------------------------------------------------------

Utf8String GetCompressionTypes(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber)
{
    if ((pi_rpRasterFile->CountPages() > pi_PageNumber) && (pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions() > 0))
    {
        Utf8String StrResult;

        //for (uint32_t ResNumber = 0; ResNumber< pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions(); ResNumber++)
        unsigned short ResNumber = 0; 
        {
            if (ResNumber > 0)
                StrResult += ", ";

            HFCPtr<HRFResolutionDescriptor> pResolution = pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->GetResolutionDescriptor(ResNumber);
            if (pResolution->GetCodec() != 0)
                StrResult += ConvertCodecToString(pResolution->GetCodec());
        }

       return StrResult;
    }
    else
        return "Unknown";
}

//-----------------------------------------------------------------------------

Utf8String GetColorSpace(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber)
{
    if ((pi_rpRasterFile->CountPages() > pi_PageNumber) && (pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions() > 0))
    {
        Utf8String StrResult;

        //for (uint32_t ResNumber = 0; ResNumber< pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions(); ResNumber++)
        unsigned short ResNumber = 0; 
        {
            if (ResNumber > 0)
                StrResult += ", ";

            HFCPtr<HRFResolutionDescriptor> pResolution = pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->GetResolutionDescriptor(ResNumber);
			StrResult += ConvertPixelTypeToString(pResolution->GetPixelType()->GetClassID());
        }

       return StrResult;
    }
    else
        return "Unknown";
}

//-----------------------------------------------------------------------------

Utf8String GetScanlineOrientation(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber)
{
    if ((pi_rpRasterFile->CountPages() > pi_PageNumber) && (pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions() > 0))
    {
        Utf8String StrResult;

        //for (uint32_t ResNumber = 0; ResNumber< pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions(); ResNumber++)
        unsigned short ResNumber = 0; 
        {
            if (ResNumber > 0)
                StrResult += ", ";

            HFCPtr<HRFResolutionDescriptor> pResolution = pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->GetResolutionDescriptor(ResNumber);
            // ScanlineOrientation
            
            StrResult = HUTClassIDDescriptor::GetInstance()->GetClassLabelSLO(pResolution->GetScanlineOrientation());
        }

       return StrResult;
    }
    else
       return "Bad page number or resolution number";

}

//-----------------------------------------------------------------------------

Utf8String GetOverviewDimension(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber)
{
    if ((pi_rpRasterFile->CountPages() > pi_PageNumber) && (pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions() > 1))
    {
        Utf8String StrResult;
        uint32_t Pass = 0;

        for (unsigned short ResNumber = 1; ResNumber< pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions(); ResNumber++)
        {
            if (ResNumber > 1)
                StrResult += ", ";

            if (Pass == 4)
            {
                StrResult += "\r";
                Pass = 0;
            }

            HFCPtr<HRFResolutionDescriptor> pResolution = pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->GetResolutionDescriptor(ResNumber);
            WChar  buffer[200];
            _stprintf(buffer, _TEXT("%I64ux%I64u"), pResolution->GetWidth(), pResolution->GetHeight());
            StrResult += Utf8String(buffer);
            Pass++;
        }

       return StrResult;
    }
    else
        return "";
}

//-----------------------------------------------------------------------------

Utf8String GetNumberOfBlock(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber)
{
    if ((pi_rpRasterFile->CountPages() > pi_PageNumber) && (pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions() > 0))
    {
        Utf8String StrResult;
        uint32_t Pass = 0;

        for (unsigned short ResNumber = 0; ResNumber< pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions(); ResNumber++)
        {
            if (ResNumber > 0)
                StrResult += ", ";

            if (Pass == 5)
            {
                StrResult += "\r";
                Pass = 0;
            }

            HFCPtr<HRFResolutionDescriptor> pResolution = pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->GetResolutionDescriptor(ResNumber);
            WChar buffer[200];
            _stprintf(buffer, _TEXT("%I64ux%I64u"), pResolution->GetBlocksPerWidth(), pResolution->GetBlocksPerHeight());
            StrResult += Utf8String(buffer);
            Pass++;
        }

       return StrResult;
    }
    else
        return "Unknown";
}

//-----------------------------------------------------------------------------

Utf8String GetBlockDimension(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber)
{
if ((pi_rpRasterFile->CountPages() > pi_PageNumber) && (pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions() > 0))
    {
    wostringstream StrResult;
    uint32_t Pass = 0;

    for (unsigned short ResNumber = 0; ResNumber < pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions(); ResNumber++)
        {
        if (ResNumber > 0)
            StrResult << _TEXT(", ");

        if (Pass == 4)
            {
            StrResult << _TEXT("\r");
            Pass = 0;
            }

        HFCPtr<HRFResolutionDescriptor> pResolution = pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->GetResolutionDescriptor(ResNumber);

        // Storage Type
        if (pResolution->GetBlockType().m_BlockType == HRFBlockType::LINE)

            StrResult << HUTClassIDDescriptor::GetInstance()->GetClassLabelBlockType(pResolution->GetBlockType()).c_str();

        else if (pResolution->GetBlockType().m_BlockType == HRFBlockType::TILE ||
                 pResolution->GetBlockType().m_BlockType == HRFBlockType::IMAGE)

            StrResult << HUTClassIDDescriptor::GetInstance()->GetClassLabelBlockType(pResolution->GetBlockType()).c_str() << _TEXT(" ") << pResolution->GetBlockWidth() << _TEXT("x") << pResolution->GetBlockHeight();

        else if (pResolution->GetBlockType().m_BlockType == HRFBlockType::STRIP)

            StrResult << HUTClassIDDescriptor::GetInstance()->GetClassLabelBlockType(pResolution->GetBlockType()).c_str() << _TEXT(" ") << pResolution->GetBlockHeight();

        else

            StrResult << _TEXT("Unknown Block Type ") << pResolution->GetBlockWidth() << _TEXT("x") << pResolution->GetBlockHeight();

        Pass++;
        }

    return Utf8String(StrResult.str().c_str());
    }

    else
        return "Bad page number or resolution number";
}

//-----------------------------------------------------------------------------

Utf8String GetFormatType(HFCPtr<HRFRasterFile>& pi_rpRasterFile)
{
    const HRFRasterFileCreator*   pRasterFileCreator = HRFRasterFileFactory::GetInstance()->FindCreator(pi_rpRasterFile->GetURL()); 

    Utf8String FormatType;
    WChar buffer[400];
    WString LabelW(pRasterFileCreator->GetLabel().c_str(), BentleyCharEncoding::Utf8);
    _stprintf(buffer, _TEXT("%s"), LabelW.c_str());
    FormatType += Utf8String(buffer);
    FormatType += " (";
    WString ExtensionW(pRasterFileCreator->GetExtensions().c_str(), BentleyCharEncoding::Utf8);
    _stprintf(buffer, _TEXT("%s"), ExtensionW.c_str());
    FormatType += Utf8String(buffer);
    FormatType += ")";

    return FormatType;
}

//-----------------------------------------------------------------------------
Utf8String ConvertPixelTypeToString(HCLASS_ID pi_ClassKey)
{
    Utf8String Result;
      
    Result = HUTClassIDDescriptor::GetInstance()->GetClassLabelPixelType(pi_ClassKey); 
    return Result;
}

//-----------------------------------------------------------------------------

Utf8String ConvertCodecToString(const HFCPtr<HCDCodec>& pi_rpCodec)                                                        
{
    Utf8String Result;
    bool IsUnknown = false;
    Utf8String Loss(", Lossless");

    Result = HUTClassIDDescriptor::GetInstance()->GetClassLabelCodec(pi_rpCodec->GetClassID()); 
    if(Result != HUTClassIDDescriptor::GetInstance()->GetNotfoundString())
    {
        if (pi_rpCodec->GetClassID() == HCDCodecFlashpix::CLASS_ID) 
        {             

            if (((HFCPtr<HCDCodecFlashpix>&)pi_rpCodec)->IsInterleaveEnabled())               
                Result += ", Interleaved";

            if (((HFCPtr<HCDCodecFlashpix>&)pi_rpCodec)->IsColorConversionEnabled())               
                Result += ", Color conversion";

            WChar Quality[1024];
            _stprintf(Quality, _TEXT(", Quality %d"),((HFCPtr<HCDCodecFlashpix>&)pi_rpCodec)->GetQuality());               
            //Result += string(Quality);

            Loss = ", Lossy";
        }
        else if (pi_rpCodec->GetClassID() == HCDCodecHMRCCITT::CLASS_ID)
        {   

            if (((HFCPtr<HCDCodecHMRCCITT>&)pi_rpCodec)->GetCCITT3())               
                Result += " 3";
            else
                Result += " 4";
        }
        else if (pi_rpCodec->GetClassID() == HCDCodecIdentity::CLASS_ID) 
        {
           IsUnknown = true;
        }
        else if (pi_rpCodec->GetClassID() == HCDCodecIJG::CLASS_ID)
        {
         
            if (((HFCPtr<HCDCodecIJG>&)pi_rpCodec)->GetOptimizeCoding())               
                Result += ", Optimize coding";

            if (((HFCPtr<HCDCodecIJG>&)pi_rpCodec)->IsProgressive())               
                Result += ", Progressive";

            if (((HFCPtr<HCDCodecIJG>&)pi_rpCodec)->GetAbbreviateMode())               
                Result += ", Abbreviate mode";       

            WChar Quality[1024];
            _stprintf(Quality, _TEXT(", Quality %d"),((HFCPtr<HCDCodecIJG>&)pi_rpCodec)->GetQuality());               
            //Result += string(Quality);

            Loss = ", Lossy";
        }
        else if (pi_rpCodec->GetClassID() == HCDCodecJPEG::CLASS_ID)                   
        {
           Loss = ", Lossy";
        }
        
        if (!IsUnknown)
        {

            // Add descriptive information
            if (pi_rpCodec->IsCodecImage())
                Result += ", Type image";

            if (pi_rpCodec->IsCodecVector())
                Result += ", Type vector";

            WChar buffer[1024];
            _stprintf(buffer, _TEXT(", Compression ratio %.2lf"), pi_rpCodec->GetEstimatedCompressionRatio());
            Result += Utf8String(buffer);
            Result += Loss;
        }
    }
    return Result;
}

//-----------------------------------------------------------------------------
Utf8String ConvertFilterToString(HCLASS_ID pi_ClassKey)
{
    Utf8String Result;

    Result = HUTClassIDDescriptor::GetInstance()->GetClassLabelFilter(pi_ClassKey); 

    return Result;
}

//-----------------------------------------------------------------------------

Utf8String ConvertTransfoModelToString(HCLASS_ID pi_ClassKey)
{
    Utf8String Result;
      
    Result = HUTClassIDDescriptor::GetInstance()->GetClassLabelTransfoModel(pi_ClassKey); 
    
    return Result;
}
//-----------------------------------------------------------------------------
