/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/Labeler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// labeler. Code taken from Utils/ImageInsider/Labeler.cpp
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
            
       Utf8Char buffer[200];
       sprintf(buffer, "%I64ux%I64u", pResolution->GetWidth(), pResolution->GetHeight());
       return buffer;
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
            
        Utf8Char buffer[200];

       if (pResolution->GetSizeInBytes() < 1024)
           sprintf(buffer, "%I64u bytes (%I64u bytes) - Main Image", pResolution->GetSizeInBytes(), pResolution->GetSizeInBytes());
       else
           if (pResolution->GetSizeInBytes() < 1024 * 1000)
               sprintf(buffer, "%.1lf KB (%I64u bytes) - Main Image", (double)pResolution->GetSizeInBytes() / (double)1024, pResolution->GetSizeInBytes());
           else
               sprintf(buffer, "%.2lf MB (%I64u bytes) - Main Image", (double)pResolution->GetSizeInBytes() / ((double)1024 * (double)1000), pResolution->GetSizeInBytes());
       return buffer;
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
            
            StrResult += HUTClassIDDescriptor::GetInstance()->GetClassLabelSLO(pResolution->GetScanlineOrientation()).c_str();
        }

       return StrResult;
    }
    else
       return "Bad page number or resolution number";

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
                StrResult += (", ");

            if (Pass == 5)
            {
                StrResult += ("\r");
                Pass = 0;
            }

            HFCPtr<HRFResolutionDescriptor> pResolution = pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->GetResolutionDescriptor(ResNumber);
            Utf8Char buffer[200];
            sprintf(buffer, ("%I64ux%I64u"), pResolution->GetBlocksPerWidth(), pResolution->GetBlocksPerHeight());
            StrResult += buffer;
            Pass++;
        }

       return StrResult;
    }
    else
        return ("Unknown");
}

//-----------------------------------------------------------------------------

Utf8String GetBlockDimension(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber)
{
    if ((pi_rpRasterFile->CountPages() > pi_PageNumber) && (pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions() > 0))
    {
        ostringstream StrResult;
        uint32_t Pass = 0;

        for (unsigned short ResNumber = 0; ResNumber< pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions(); ResNumber++)
        {
            if (ResNumber > 0)
                StrResult << ", ";

            if (Pass == 4)
            {
                StrResult << "\r";
                Pass = 0;
            }

            HFCPtr<HRFResolutionDescriptor> pResolution = pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->GetResolutionDescriptor(ResNumber);
                        
            // Storage Type
            if (pResolution->GetBlockType().m_BlockType == HRFBlockType::LINE)
                
                StrResult << HUTClassIDDescriptor::GetInstance()->GetClassLabelBlockType(pResolution->GetBlockType());
            
            else if (pResolution->GetBlockType().m_BlockType == HRFBlockType::TILE || 
                     pResolution->GetBlockType().m_BlockType == HRFBlockType::IMAGE)
                
                StrResult << HUTClassIDDescriptor::GetInstance()->GetClassLabelBlockType(pResolution->GetBlockType()) << " " << pResolution->GetBlockWidth() << "x" << pResolution->GetBlockHeight();
          
            else if (pResolution->GetBlockType().m_BlockType == HRFBlockType::STRIP)
                
                StrResult << HUTClassIDDescriptor::GetInstance()->GetClassLabelBlockType(pResolution->GetBlockType()) << " " << pResolution->GetBlockHeight();
            
            else
                
                StrResult << "Unknown Block Type " << pResolution->GetBlockWidth() << "x" << pResolution->GetBlockHeight();

            Pass++;
        }

       return StrResult.str().c_str();
    }
    else
        return "Bad page number or resolution number";
}

//-----------------------------------------------------------------------------

Utf8String GetFormatType(HFCPtr<HRFRasterFile>& pi_rpRasterFile)
{
    const HRFRasterFileCreator*   pRasterFileCreator = HRFRasterFileFactory::GetInstance()->FindCreator(pi_rpRasterFile->GetURL()); 

    Utf8String FormatType;
    Utf8Char buffer[400];
    sprintf(buffer, "%s",pRasterFileCreator->GetLabel().c_str());
    FormatType += buffer;
    FormatType += " (";
    sprintf(buffer, "%s",pRasterFileCreator->GetExtensions().c_str());
    FormatType += buffer;
    FormatType += ")";

    return FormatType;
}

//-----------------------------------------------------------------------------

WString GetName(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber)
    {
    if ((pi_rpRasterFile->CountPages() > pi_PageNumber) && (pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions() > 0))
        {
        WString StrResult;

        GeoCoordinates::BaseGCSCPtr pGcs = pi_rpRasterFile->GetPageDescriptor(0)->GetGeocodingCP();
        if (pGcs != NULL && pGcs->IsValid())
            {
            StrResult += L"Name : ";
            StrResult += pGcs->GetName();
            StrResult += L"\r\n\r\n";

            StrResult += L"Description : ";
            StrResult += pGcs->GetDescription();
            StrResult += L"\r\n\r\n";
            
            StrResult += L"Source : ";
            WString StrTemp;
            pGcs->GetSource(StrTemp);
            StrResult += StrTemp;

            return StrResult;
            }
        }

    return L"No GCS or invalid GCS.";

    }

//-----------------------------------------------------------------------------
WString GetUnits(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber)
    {
    if ((pi_rpRasterFile->CountPages() > pi_PageNumber) && (pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions() > 0))
        {
        WString StrResult;

        GeoCoordinates::BaseGCSCPtr pGcs = pi_rpRasterFile->GetPageDescriptor(0)->GetGeocodingCP();
        if (pGcs != NULL && pGcs->IsValid())
            {
            pGcs->GetUnits(StrResult);
            if (!StrResult.empty())
                return StrResult;
            else
                return L"Empty units definition.";
            }
        }

    return L"No GCS or invalid GCS.";
    }

//-----------------------------------------------------------------------------
WString GetDatumName(HFCPtr<HRFRasterFile>& pi_rpRasterFile, uint32_t pi_PageNumber)
    {
    if ((pi_rpRasterFile->CountPages() > pi_PageNumber) && (pi_rpRasterFile->GetPageDescriptor(pi_PageNumber)->CountResolutions() > 0))
        {
        WString StrResult;

        GeoCoordinates::BaseGCSCPtr pGcs = pi_rpRasterFile->GetPageDescriptor(0)->GetGeocodingCP();
        if (pGcs != NULL && pGcs->IsValid())
            {
            StrResult += pGcs->GetDatumName();
            if (!StrResult.empty())
                return StrResult;
            else
                return L"Empty datum name.";
            }
        }

    return L"No GCS or invalid GCS.";
    }

//-----------------------------------------------------------------------------
Utf8String ConvertPixelTypeToString(HCLASS_ID pi_ClassKey)
{
    return HUTClassIDDescriptor::GetInstance()->GetClassLabelPixelType(pi_ClassKey); 
}

//-----------------------------------------------------------------------------

Utf8String ConvertCodecToString(const HFCPtr<HCDCodec>& pi_rpCodec)
{
    Utf8String Result;
    bool IsUnknown = false;
    Utf8String Loss(", Lossless");

    Result = HUTClassIDDescriptor::GetInstance()->GetClassLabelCodec(pi_rpCodec->GetClassID());
    if (Result != HUTClassIDDescriptor::GetInstance()->GetNotfoundString())
    {
        if (pi_rpCodec->GetClassID() == HCDCodecFlashpix::CLASS_ID)
        {

            if (((HFCPtr<HCDCodecFlashpix>&)pi_rpCodec)->IsInterleaveEnabled())
                Result += ", Interleaved";

            if (((HFCPtr<HCDCodecFlashpix>&)pi_rpCodec)->IsColorConversionEnabled())
                Result += ", Color conversion";

            char Quality[1024];
            sprintf(Quality, ", Quality %d", ((HFCPtr<HCDCodecFlashpix>&)pi_rpCodec)->GetQuality());
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

            char Quality[1024];
            sprintf(Quality, ", Quality %d", ((HFCPtr<HCDCodecIJG>&)pi_rpCodec)->GetQuality());
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

            char buffer[1024];
            sprintf(buffer, ", Compression ratio %.2lf", pi_rpCodec->GetEstimatedCompressionRatio());
            Result += buffer;
            Result += Loss;
        }
    }

    return Result;
}

//-----------------------------------------------------------------------------
Utf8String ConvertFilterToString(HCLASS_ID pi_ClassKey)
{
    return HUTClassIDDescriptor::GetInstance()->GetClassLabelFilter(pi_ClassKey);
}

//-----------------------------------------------------------------------------

Utf8String ConvertTransfoModelToString(HCLASS_ID pi_ClassKey)
{
   return HUTClassIDDescriptor::GetInstance()->GetClassLabelTransfoModel(pi_ClassKey);
}
//-----------------------------------------------------------------------------

WString GetLongPathName(WStringCR pi_Path)
{
    LPSHELLFOLDER psfDesktop = NULL;
    ULONG chEaten = 0;
    LPITEMIDLIST pidlShellItem = NULL;
    WCHAR szLongPath[_MAX_PATH] = { 0 };

    // Get the Desktop's shell folder interface
    HRESULT hr = SHGetDesktopFolder(&psfDesktop);

    // Request an ID list (relative to the desktop) for the short pathname
    USES_CONVERSION;
    hr = psfDesktop->ParseDisplayName(NULL,
        NULL,
        T2OLE((LPTSTR)pi_Path.c_str()),
        &chEaten,
        &pidlShellItem,
        NULL);
    psfDesktop->Release();  // Release the desktop's IShellFolder

    if (FAILED(hr))
    {
        // If we couldn't get an ID list for short pathname, it must not exist.
        return false;
        lstrcpyW(szLongPath, L"Error: Path not found!");
    }
    else
    {
        // We did get an ID list, convert it to a long pathname
        SHGetPathFromIDListW(pidlShellItem, szLongPath);

        // Free the ID list allocated by ParseDisplayName
        LPMALLOC pMalloc = NULL;
        SHGetMalloc(&pMalloc);
        pMalloc->Free(pidlShellItem);
        pMalloc->Release();
    }

    return WString(CString(W2T(szLongPath)).GetString());
}

//-----------------------------------------------------------------------------
