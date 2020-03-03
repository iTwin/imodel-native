/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageInsider/MyPropertyPage1.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

// MyPropertyPage1.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "MyPropertyPage1.h"
#include "ImageInsider.h"
#include <afxole.h>
#include <afxadv.h>

USING_NAMESPACE_IMAGEPP

#define tlpcstr LPWSTR
#define cftxt CF_UNICODETEXT



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


// exception
#include <Imagepp/all/h/HPAException.h>
#include <Imagepp/all/h/HRFException.h>
#include <stdexcpt.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HCDException.h>
#include <ImagePP/all/h/HCPGeoTiffKeys.h>

//-----------------------------------------------------------------------------
// Header SLO Files
//-----------------------------------------------------------------------------
#include <Imagepp/all/h/HRFSLOModelComposer.h>
#include <Imagepp/all/h/HGF2DAffine.h>

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "Labeler.h"
#include "toolbox.h"
#include "HFCStringTokenizer.h"

#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFSLOStripAdapter.h>
#include <Imagepp/all/h/HRFIntergraphFile.h>
#include <Imagepp/all/h/HRFCalsFile.h>

//#include <ImageppApps/IppWinLib/HFCSysCfg.h>
#include <Imagepp/all/h/HFCStat.h>

#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>
#include <Imagepp/all/h/HRABitmap.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HRFPageFileFactory.h>
#include <Imagepp/all/h/HRFRasterFilePageDecorator.h>

#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HRACopyFromOptions.h>
#include <ImagePP/all/h/HCPGeoTiffKeys.h>



// Default page
uint32_t                      PageNumber = 0;
HFCPtr<HRFRasterFile>         pi_rpRasterFile = 0;
bool                         MarkWriteBlock = true;

// Image information string
Utf8String                       FileSizeString;
Utf8String                       FilenameString;              
Utf8String                       FormatTypeString;            
Utf8String                       LocationString;              
Utf8String                       UncompressedSizeString;      
Utf8String                       ImageDimensionString;        
Utf8String                       CompressionTypesString;      
Utf8String                       ColorSpaceString;            
Utf8String                       ScanlineOrientationString;   
Utf8String                       OverviewDimensionString;     
Utf8String                       NumberOfBlockString;         
Utf8String                       BlockDimensionString;        


// Thumbnail section
bool                         IsQualityThumbnailCalculated = false;
bool                         IsFastThumbnailCalculated = false;

HFCPtr<HRFThumbnail>          pSrcQualityPixels;
HFCPtr<HRFThumbnail>          pSrcFastPixels;

// Benchmark values string
Utf8String                       ThumbnailQualityMarkString;
Utf8String                       ThumbnailFastMarkString;

Utf8String                       AverageReadBlockString;
Utf8String                       BestReadBlockString;     
Utf8String                       WorstReadBlockString;    

Utf8String                       AverageDecompressionString; 
Utf8String                       BestDecompressionString;     
Utf8String                       WorstDecompressionString;    

Utf8String                       AverageWriteBlockString;  
Utf8String                       BestWriteBlockString;     
Utf8String                       WorstWriteBlockString;    

Utf8String                       AverageCompressionString;  
Utf8String                       BestCompressionString;     
Utf8String                       WorstCompressionString;    

Utf8String                       ReadTimeString;
Utf8String                       DeCompressionTimeString;
Utf8String                       WriteTimeString;
Utf8String                       CompressionTimeString;


#define IsCTRLpressed()  ( (GetKeyState(VK_CONTROL) & (1 << (sizeof(SHORT)*8-1))) != 0 )


//-----------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CMyPropertyPage1, CPropertyPage)
IMPLEMENT_DYNCREATE(CMyPropertyPage2, CPropertyPage)
IMPLEMENT_DYNCREATE(CMyPropertyPage3, CPropertyPage)

//-----------------------------------------------------------------------------
// CMyPropertyPage1 property page
//-----------------------------------------------------------------------------
CMyPropertyPage1::CMyPropertyPage1() : CPropertyPage(CMyPropertyPage1::IDD), HFCProgressDurationListener()
{
    //{{AFX_DATA_INIT(CMyPropertyPage1)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

//-----------------------------------------------------------------------------

CMyPropertyPage1::~CMyPropertyPage1()
{
    pi_rpRasterFile = 0;
}

//-----------------------------------------------------------------------------

void CMyPropertyPage1::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CMyPropertyPage1)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

//-----------------------------------------------------------------------------

BOOL CMyPropertyPage1::OnInitDialog() 
{
    CPropertyPage::OnInitDialog();


    // Open a file passed as the first command line parameter.
    // To get the size
    // try to instanciate directly the given argument
    HFCPtr<HFCURL>  SrcFileName(HFCURL::Instanciate(Utf8String(AfxGetApp()->m_lpCmdLine)));
    if (SrcFileName == 0)
    {
        // Open the raster file as a file
        SrcFileName = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(AfxGetApp()->m_lpCmdLine));     
    }

    HFCStat Stat((HFCPtr<HFCURL>)SrcFileName);

    WChar buffer[200];  
    if (Stat.GetSize() < 1024)
        _stprintf(buffer, _TEXT("%lld bytes (%lld bytes)"), Stat.GetSize(), Stat.GetSize());
    else
        if (Stat.GetSize() < 1024 * 1000)
            _stprintf(buffer, _TEXT("%.1lf KB (%lld bytes)"), (double)Stat.GetSize() / (double)1024, Stat.GetSize());
        else
            if (Stat.GetSize() < 1024 * 1000 * 1000)
                _stprintf(buffer, _TEXT("%.2lf MB (%lld bytes)"), (double)Stat.GetSize() / ((double)1024 * (double)1000), Stat.GetSize());
            else
                _stprintf(buffer, _TEXT("%.2lf GB (%lld bytes)"), (double)Stat.GetSize() / ((double)1024 * (double)1000 * (double)1000), Stat.GetSize());

    FileSizeString = Utf8String(buffer);

    // Compute string description
    pi_rpRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)SrcFileName, true);   

    // Add the Decoration HGR or the TFW Page File
    if (HRFPageFileFactory::GetInstance()->HasFor(pi_rpRasterFile))
        pi_rpRasterFile = new HRFRasterFilePageDecorator(pi_rpRasterFile, HRFPageFileFactory::GetInstance()->FindCreatorFor(pi_rpRasterFile));

    FormatTypeString            = GetFormatType(pi_rpRasterFile);


    if (SrcFileName->GetSchemeType() == HFCURLFile::s_SchemeName())
    {
        // Extract the Path
        //string Path(((HFCPtr<HFCURLFile>&)pi_rpRasterFile->GetURL())->GetHost()+string("\\")+((HFCPtr<HFCURLFile>&)pi_rpRasterFile->GetURL())->GetPath());
        Utf8String Path(GetLongPathName(Utf8String(AfxGetApp()->m_lpCmdLine)));

        // Find the file extension
        Utf8String::size_type DotPos = Path.rfind('\\');

        // Extract the extension and the drive dir name
        if (DotPos != Utf8String::npos)
        {
            FilenameString = Path.substr(DotPos+1, Path.length() - DotPos - 1);
            LocationString = Path.substr(0, DotPos);
        }
        else
        {
            FilenameString         = pi_rpRasterFile->GetURL()->GetURL();
            LocationString         = Path;
        }
    }
    else
    {
        FilenameString         = pi_rpRasterFile->GetURL()->GetURL();
        LocationString         = "";
    }
    UncompressedSizeString      = GetUncompressedSize(pi_rpRasterFile, PageNumber);
    ImageDimensionString        = GetImageDimension(pi_rpRasterFile, PageNumber);
    CompressionTypesString      = GetCompressionTypes(pi_rpRasterFile, PageNumber);
    ColorSpaceString            = GetColorSpace(pi_rpRasterFile, PageNumber);   
    ScanlineOrientationString   = GetScanlineOrientation(pi_rpRasterFile, PageNumber);
    OverviewDimensionString     = GetOverviewDimension(pi_rpRasterFile, PageNumber);
    NumberOfBlockString         = GetNumberOfBlock(pi_rpRasterFile, PageNumber);
    BlockDimensionString        = GetBlockDimension(pi_rpRasterFile, PageNumber);

    return true;  // return true unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return false
}

//-----------------------------------------------------------------------------

void CMyPropertyPage1::Progression(HFCProgressIndicator* pi_pProgressIndicator, 
                                   uint32_t              pi_Processed,		 
                                   uint32_t              pi_CountProgression)  
{
    Utf8String TimeToBeProcessed;

    if (!pi_pProgressIndicator->IsLifeSignal())
        HFCProgressDurationListener::Progression(pi_pProgressIndicator, pi_Processed, pi_CountProgression);


    if (!pi_pProgressIndicator->IsLifeSignal())
    {
        // Rest number of items to be processed approx duration.
        WChar buffer[200];
        double TimeToDo = (pi_CountProgression - pi_Processed) * GetAverageDuration();
        uint32_t Hours    = ((uint32_t)TimeToDo / 3600);
        uint32_t Minutes  = ((uint32_t)TimeToDo % 3600) / 60;
        uint32_t Seconds  = ((uint32_t)TimeToDo % 3600) % 60;

        Utf8String TimeStr("");

        if (Hours > 0)
        {
            _stprintf(buffer, _TEXT("%02ldh"), Hours); 
            TimeStr += Utf8String(buffer);
        }
        else
            TimeStr += "   ";

        if ((Hours > 0) || (Minutes > 0))
        {
            _stprintf(buffer, _TEXT("%02ldm"), Minutes); 
            TimeStr += Utf8String(buffer);
        }
        else
            TimeStr += "   ";

        _stprintf(buffer, _TEXT("%02lds"), Seconds); 
        TimeStr += Utf8String(buffer);

        TimeToBeProcessed = TimeStr;
    }

    // Draw the progression
    WChar  LifeString[200] = _TEXT(".................... Approx time to be processed: ");
    wmemset(LifeString, L'', (size_t)(20.0 / (double)pi_CountProgression * (double)pi_Processed));

    Utf8String lString(LifeString);
    lString += TimeToBeProcessed;

    GetDlgItem(IDC_PROGRESS)->SetWindowText((WString(lString.c_str(), BentleyCharEncoding::Utf8)).c_str());

    // Check the ESC to CANCEL the iteration
    if (GetAsyncKeyState(VK_ESCAPE))
    {
        HRFThumbnailProgressIndicator::GetInstance()->StopIteration();         
        m_IsThumbnailCanceled = true;
    }

}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
void CMyPropertyPage1::DrawGDIThumbnail(Byte* pSrcPixels, uint32_t PreferedWidth, uint32_t PreferedHeight)
    {
    // Draw the thumbnail with GDI

    // Convert the Pixel Type
    HFCPtr<HRPPixelType> pPixelType = pi_rpRasterFile->GetPageDescriptor(PageNumber)->GetResolutionDescriptor(0)->GetPixelType();
    uint32_t SrcSizeInBytes = (((pPixelType->CountPixelRawDataBits() * PreferedWidth) + 7) /8) * PreferedHeight;    
    HFCPtr<HRPPixelType> pDstPixelType = new HRPPixelTypeV24B8G8R8();
    uint32_t DstBytesPerWidth = ((pDstPixelType->CountPixelRawDataBits() * PreferedWidth) + 7) /8;
    HFCPtr<HCDPacket>    pSourcePacket(new HCDPacket(pSrcPixels, SrcSizeInBytes, SrcSizeInBytes));
    HFCPtr<HCDPacket>    pConvertedPacket = new HCDPacket(0, 0, 0);

    ConvertThePixels(PreferedWidth, 
        PreferedHeight, 
        pPixelType, 
        pSourcePacket, 
        pDstPixelType,
        pConvertedPacket);


    // Buffer allocation for bitmap
    // Each line in the DIB must be padded on a long [(+3)&-4]
    long DIBWidth = (((PreferedWidth*3L) + 3L) & -4L);
    HArrayAutoPtr<BYTE> pBitmapData;
    pBitmapData = new BYTE[DIBWidth * PreferedHeight];

    // Convert the thumbnail data into the  buffer - due to the padding
    for (uint32_t i=0 ; i<PreferedHeight ; i++)
        memcpy(pBitmapData+(DIBWidth * (PreferedHeight - i -1)), 
        pConvertedPacket->GetBufferAddress() + (DstBytesPerWidth * i), 
        DstBytesPerWidth);

    // Bitmap initialization
    BITMAPINFOHEADER BitInfo;

    BitInfo.biSize          = sizeof(BITMAPINFOHEADER);
    BitInfo.biWidth         = PreferedWidth;
    BitInfo.biHeight        = PreferedHeight;
    BitInfo.biPlanes        = 1;
    BitInfo.biBitCount      = 24;
    BitInfo.biCompression   = BI_RGB;
    BitInfo.biSizeImage     = 0;
    BitInfo.biXPelsPerMeter = 0;
    BitInfo.biYPelsPerMeter = 0;
    BitInfo.biClrUsed       = 0;
    BitInfo.biClrImportant  = 0;

    // Draw the thumbnail
    StretchDIBits (GetDC()->m_hDC,   
        12+((128 - PreferedWidth) / 2),  11 + ((128 - PreferedHeight) / 2),  
        PreferedWidth, PreferedHeight,
        0       , 0,
        PreferedWidth, PreferedHeight,
        (LPVOID) pBitmapData,
        (BITMAPINFO*)&BitInfo,
        DIB_RGB_COLORS,
        SRCCOPY);
    }

//-----------------------------------------------------------------------------

void CMyPropertyPage1::OnPaint() 
{
    CPaintDC dc(this); // device context for painting

    HCURSOR TheCursor = ::SetCursor(LoadCursor(NULL, IDC_WAIT));

    // Do not call CPropertyPage::OnPaint() for painting messages
    // Draw the bitmap in the Device Context 
    // Stretch the bitmap dimension to the specified client dimension

    GetDlgItem(IDC_STATIC_1)->UpdateWindow();
    GetDlgItem(IDC_STATIC_2)->UpdateWindow();
    GetDlgItem(IDC_STATIC_3)->UpdateWindow();
    GetDlgItem(IDC_STATIC_4)->UpdateWindow();
    GetDlgItem(IDC_STATIC_5)->UpdateWindow();
    GetDlgItem(IDC_STATIC_6)->UpdateWindow();
    GetDlgItem(IDC_STATIC_7)->UpdateWindow();
    GetDlgItem(IDC_STATIC_8)->UpdateWindow();
    GetDlgItem(IDC_STATIC_9)->UpdateWindow();
    GetDlgItem(IDC_STATIC_10)->UpdateWindow();
    GetDlgItem(IDC_STATIC_11)->UpdateWindow();
    GetDlgItem(IDC_STATIC_12)->UpdateWindow();
    GetDlgItem(IDC_STATIC_13)->UpdateWindow();
    GetDlgItem(IDC_STATIC_14)->UpdateWindow();

    GetDlgItem(IDC_FILENAME)->SetWindowText((WString(FilenameString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_TYPE)->SetWindowText((WString(FormatTypeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_LOCATION)->SetWindowText((WString(LocationString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_IMAGESIZE)->SetWindowText((WString(FileSizeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_USIZE)->SetWindowText((WString(UncompressedSizeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_IMAGED)->SetWindowText((WString(ImageDimensionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_COMPRESSIONT)->SetWindowText((WString(CompressionTypesString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_COLORS)->SetWindowText((WString(ColorSpaceString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_SCANLINEO)->SetWindowText((WString(ScanlineOrientationString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_OVERVIEWSD)->SetWindowText((WString(OverviewDimensionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_NUMBEROB)->SetWindowText((WString(NumberOfBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_BLOCKD)->SetWindowText((WString(BlockDimensionString.c_str(), BentleyCharEncoding::Utf8)).c_str());

    // Calc the Thumbnail
    if ((((CButton*)GetDlgItem(IDC_CHECK_QUALITY_THUMB))->GetCheck() && (!IsQualityThumbnailCalculated)) ||
        (!((CButton*)GetDlgItem(IDC_CHECK_QUALITY_THUMB))->GetCheck() && (!IsFastThumbnailCalculated)))
    {
        // Re-Open the image file
        pi_rpRasterFile = 0;

        HFCPtr<HFCURL>  SrcFileName(HFCURL::Instanciate(Utf8String(AfxGetApp()->m_lpCmdLine)));
        if (SrcFileName == 0)
        {
            // Open the raster file as a file
            SrcFileName = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(AfxGetApp()->m_lpCmdLine));     
        }
        pi_rpRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)SrcFileName, true); 

        // Adapt Scan Line Orientation (1 bit images)
        bool CreateSLOAdapter = true;

        if ((pi_rpRasterFile->IsCompatibleWith(HRFIntergraphFile::CLASS_ID)) ||
            (pi_rpRasterFile->IsCompatibleWith(HRFCalsFile::CLASS_ID))       )
        {
            if (HRFSLOStripAdapter::NeedSLOAdapterFor(pi_rpRasterFile))
            {
                // Adapt only when the raster file has not a standard scan line orientation
                // i.e. with an upper left origin, horizontal scan line.
                //pi_rpRasterFile = HRFSLOStripAdapter::CreateBestAdapterFor(pi_rpRasterFile);

                CreateSLOAdapter = true;
            }
        }


        if (((CButton*)GetDlgItem(IDC_CHECK_QUALITY_THUMB))->GetCheck())
        {
            m_IsThumbnailCanceled = false;
            HRFThumbnailProgressIndicator::GetInstance()->AddListener(this);

            uint32_t PreferedWidthQuality   = 128;
            uint32_t PreferedHeightQuality  = 128;

            Chronometer QualityThumbnailChrono;
            QualityThumbnailChrono.Start();
            pSrcQualityPixels = HRFThumbnailMaker(pi_rpRasterFile, PageNumber, &PreferedWidthQuality, &PreferedHeightQuality, true);
            IsQualityThumbnailCalculated = !m_IsThumbnailCanceled;
            QualityThumbnailChrono.Stop();
            ThumbnailQualityMarkString       = QualityThumbnailChrono.GetDuration() + " seconds";

            GetDlgItem(IDC_PROGRESS)->SetWindowText(_TEXT(""));
            HRFThumbnailProgressIndicator::GetInstance()->RemoveListener(this);

        }
        else
        {
            m_IsThumbnailCanceled = false;
            HRFThumbnailProgressIndicator::GetInstance()->AddListener(this);

            uint32_t PreferedWidthFast      = 128;
            uint32_t PreferedHeightFast     = 128;

            Chronometer FastThumbnailChrono;
            FastThumbnailChrono.Start();
            pSrcFastPixels = HRFThumbnailMaker(pi_rpRasterFile, PageNumber, &PreferedWidthFast, &PreferedHeightFast, false);
            IsFastThumbnailCalculated = !m_IsThumbnailCanceled;
            FastThumbnailChrono.Stop();
            ThumbnailFastMarkString       = FastThumbnailChrono.GetDuration() + " seconds";

            GetDlgItem(IDC_PROGRESS)->SetWindowText(_TEXT(""));
            HRFThumbnailProgressIndicator::GetInstance()->RemoveListener(this);
        }


        // Remove the SLO Adapter
        if (CreateSLOAdapter)
        {
            pi_rpRasterFile = 0;

            HFCPtr<HFCURL>  SrcFileName(HFCURL::Instanciate(Utf8String(AfxGetApp()->m_lpCmdLine)));
            if (SrcFileName == 0)
            {
                // Open the raster file as a file
                SrcFileName = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(AfxGetApp()->m_lpCmdLine));     
            }
            pi_rpRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)SrcFileName, true);   

            // Add the Decoration HGR or the TFW Page File
            if (HRFPageFileFactory::GetInstance()->HasFor(pi_rpRasterFile))
                pi_rpRasterFile = new HRFRasterFilePageDecorator(pi_rpRasterFile, HRFPageFileFactory::GetInstance()->FindCreatorFor(pi_rpRasterFile));
        }
    }

    // Use the selected Thumb quality
    HArrayAutoPtr<Byte>  pSrcPixels;
    uint32_t PreferedWidth   = 128;
    uint32_t PreferedHeight  = 128;

    if (((CButton*)GetDlgItem(IDC_CHECK_QUALITY_THUMB))->GetCheck())
    {
        PreferedWidth   = pSrcQualityPixels->GetWidth();
        PreferedHeight  = pSrcQualityPixels->GetHeight();
        pSrcPixels = new Byte[pSrcQualityPixels->GetSizeInBytes()];

        pSrcQualityPixels->Read(pSrcPixels);
    }
    else
    {
        PreferedWidth   = pSrcFastPixels->GetWidth();
        PreferedHeight  = pSrcFastPixels->GetHeight();
        pSrcPixels = new Byte[pSrcFastPixels->GetSizeInBytes()];

        pSrcFastPixels->Read(pSrcPixels);
    }

    // Erase background
    HDC hdc             = GetDC()->m_hDC; 

    RECT  ClearRect;
    ClearRect.left   = 11+((128 - PreferedWidth) / 2);  
    ClearRect.top    = 10 + ((128 - PreferedHeight) / 2);
    ClearRect.right  = 12+((128 - PreferedWidth) / 2) + PreferedWidth + 10;
    ClearRect.bottom = 11 + ((128 - PreferedHeight) / 2) + PreferedHeight + 10;
    HBRUSH BackgroundBrush    = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
    FillRect(hdc, &ClearRect, BackgroundBrush);
    DeleteObject(BackgroundBrush);

    // Draw the Shadow
    HBRUSH GrayBrush    = CreateSolidBrush(RGB(170, 170, 170));
    HPEN   GrayPen      = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
    HGDIOBJ gdiObjBrush = SelectObject(hdc, GrayBrush); 
    HGDIOBJ gdiObjPen   = SelectObject(hdc, GrayPen);          
    RoundRect(hdc, 17+((128 - PreferedWidth) / 2),  16 + ((128 - PreferedHeight) / 2),
        17+((128 - PreferedWidth) / 2) + PreferedWidth + 1,  16 + ((128 - PreferedHeight) / 2) + PreferedHeight + 1,
        2, 2);

    // restore the old attributes.
    SelectObject(hdc, gdiObjBrush); 
    SelectObject(hdc, gdiObjPen); 
    DeleteObject(GrayBrush);
    DeleteObject(GrayPen);

    // Draw the border
    HBRUSH BlackBrush    = CreateSolidBrush(RGB(0, 0, 0));
    HPEN   BlackPen      = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    gdiObjBrush = SelectObject(hdc, BlackBrush); 
    gdiObjPen   = SelectObject(hdc, BlackPen);  
    RoundRect(hdc, 11+((128 - PreferedWidth) / 2),  10 + ((128 - PreferedHeight) / 2),
        12+((128 - PreferedWidth) / 2) + PreferedWidth + 1,  11 + ((128 - PreferedHeight) / 2) + PreferedHeight + 1,
        2, 2);
    // restore the old attributes.
    SelectObject(hdc, gdiObjBrush); 
    SelectObject(hdc, gdiObjPen); 
    DeleteObject(BlackBrush);
    DeleteObject(BlackPen);

    if (((CButton*)GetDlgItem(IDC_CHECK_GEO_THUMB))->GetCheck())
    {
        // Draw the thumbnail with Geo Reference

        HFCPtr<HRFPageDescriptor> pPageDescriptor = pi_rpRasterFile->GetPageDescriptor(PageNumber);


        // CoordSys Note:
        // Logical Coord Sys (HRF WorldID  <--------> HGFHMRStdWorldCluster)
        // Physical Coord Sys <----- HRF Transfo Model <----- Logical Coord Sys         

        // World and HRF CoordSys
        HFCPtr<HGFHMRStdWorldCluster> pWorldCluster(new HGFHMRStdWorldCluster());
        HFCPtr<HGF2DCoordSys>         pHRFLogicalCoordSys = pWorldCluster->GetCoordSysReference(pi_rpRasterFile->GetWorldIdentificator()); // HGF2DWorld_UNKNOWNWORLD);

        // Get the transfo model from the HRF.
        HFCPtr<HGF2DTransfoModel> pHRFTransfoModel;

        if (pPageDescriptor->HasTransfoModel())
            pHRFTransfoModel = pPageDescriptor->GetTransfoModel();
        else
        {
            if (HRFSLOStripAdapter::NeedSLOAdapterFor(pi_rpRasterFile))
            {
                // Create Model for each slo with unknown world to slo 4
                // The translation is not applied 

                // The matrix is either not present, invalid or the Intergraph version too small
                // We here build a simple model relevant of positionless files
                // Instanciate affine
                // Instanciate pointer to the affine required                
                HFCPtr<HGF2DAffine> pNewAffine;
                pNewAffine = new HGF2DAffine();

                // Obtain physical width and height (pixels per scan line and number of scan lines)
                // from base resolution
                HASSERT(pPageDescriptor->GetResolutionDescriptor(0)->GetWidth() <= LONG_MAX);
                HASSERT(pPageDescriptor->GetResolutionDescriptor(0)->GetHeight() <= LONG_MAX);

                int32_t NumPixelsPerScanline = (int32_t)pPageDescriptor->GetResolutionDescriptor(0)->GetWidth();
                int32_t NumScanlines         = (int32_t)pPageDescriptor->GetResolutionDescriptor(0)->GetHeight();

                // The following transformation models are transformations from file SLO to SLO 4
                // to which is added a flip for the fact SLO 4 has an Y inversion compared to
                // its logical coordinate system.
                // NOTE: By transformation from file SLO to SLO 4 we mean that the visual upper-left
                // corner of the image will be located at the origin of the logical coordinate system
                // We did not use the strange Intergraph convention where flips and inversions are
                // perfomed but where the origin of the physical coordinate system remains at the origin 
                // of the logical coordinate system.
                // All files will be located at the same position if they are only different by their
                // SLO and byte order
                // SLO 0
                HRFScanlineOrientation ScanlineOrientation = pPageDescriptor->GetResolutionDescriptor(0)->GetScanlineOrientation();

                if (ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)
                {
                    pNewAffine->SetByMatrixParameters(0.0, 0.0, 1.0, 0.0, 1.0, 0.0);    
                }
                // SLO 1
                else if (ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)
                {
                    pNewAffine->SetByMatrixParameters(NumScanlines, 0.0, -1.0, 0.0, 1.0, 0.0);
                }      
                // SLO 2
                else if (ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)
                {
                    pNewAffine->SetByMatrixParameters(0.0, 0.0, 1.0, -NumPixelsPerScanline, -1.0, 0.0);
                }
                // SLO 3
                else if (ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
                {
                    pNewAffine->SetByMatrixParameters(NumScanlines, 0.0, -1.0, -NumPixelsPerScanline, -1.0, 0.0);
                }
                // SLO 4
                else if (ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)
                {
                    pNewAffine->SetByMatrixParameters(0.0, 1.0, 0.0, 0.0, 0.0, -1.0);
                }  
                // SLO 5
                else if (ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)
                {
                    pNewAffine->SetByMatrixParameters(NumPixelsPerScanline, -1.0, 0.0, 0.0, 0.0, 1.0);
                }
                // SLO 6
                else if (ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
                {
                    pNewAffine->SetByMatrixParameters(0.0, 1.0, 0.0, -NumScanlines, 0.0, -1.0);
                }
                // SLO 7
                else if (ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)
                {
                    pNewAffine->SetByMatrixParameters(NumPixelsPerScanline, -1.0, 0.0, -NumScanlines, 0.0, -1.0);
                }
                pHRFTransfoModel = pNewAffine;
            }
            else
                pHRFTransfoModel = new HGF2DIdentity();
        }

        // Create the physical 1:1 coordsys (link the HRF transfo model with the HRF CoordSys)
        HFCPtr<HGF2DCoordSys> pHRFPhysicalCoordSys(new HGF2DCoordSys(*pHRFTransfoModel, pHRFLogicalCoordSys));        

        // Create the transfo model Stretch (1:1 scale to the Thumbnail scale factor) - Keep aspect ratio
        double ScalingX = (double)PreferedWidth  / (double)pPageDescriptor->GetResolutionDescriptor(0)->GetWidth();
        double ScalingY = (double)PreferedHeight / (double)pPageDescriptor->GetResolutionDescriptor(0)->GetHeight();
        HFCPtr<HGF2DTransfoModel> pThumbnailStretchModel(new HGF2DStretch (HGF2DDisplacement(), 1.0 / ScalingX, 1.0 / ScalingY));

        // Store Pixels in packet
        HFCPtr<HRPPixelType> pPixelType = pPageDescriptor->GetResolutionDescriptor(0)->GetPixelType();
        uint32_t SrcSizeInBytes = (((pPixelType->CountPixelRawDataBits() * PreferedWidth) + 7) /8) * PreferedHeight;    
        HFCPtr<HCDPacket>    pSourcePacket(new HCDPacket(pSrcPixels, SrcSizeInBytes, SrcSizeInBytes));

        // Store the thumbnail into HRA Bitmap
        HFCPtr<HRABitmap> pBitmapSrc( HRABitmap::Create( PreferedWidth,
            PreferedHeight,
            pThumbnailStretchModel, 
            pHRFPhysicalCoordSys, 
            pPixelType)); 
        pBitmapSrc->SetPacket(pSourcePacket);


        // Apply the HRF shape to the HRA Bitmap
        if (pPageDescriptor->HasClipShape ())
        {
            HVEShape Shape(*pPageDescriptor->GetClipShape());

            if (pPageDescriptor->GetClipShape()->GetCoordinateType() == HRFCoordinateType::PHYSICAL)
                Shape.SetCoordSys(pHRFPhysicalCoordSys);
            else
                Shape.SetCoordSys(pHRFLogicalCoordSys);            

            pBitmapSrc->SetShape(Shape);     
        }

        HFCPtr<HVEShape> pShape(new HVEShape(*pBitmapSrc->GetEffectiveShape()));
        pShape->ChangeCoordSys(pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));

        HGF2DExtent extent = pShape->GetExtent();

        HFCPtr<HGF2DTransfoModel> pModel(new HGF2DStretch(HGF2DDisplacement(extent.GetOrigin().GetX(),extent.GetOrigin().GetY()),
                                                          extent.GetWidth() / PreferedWidth,
                                                          extent.GetHeight() / PreferedHeight));

        HFCPtr<HRABitmap> pToDrawBitmap =  HRABitmap::Create(PreferedWidth, PreferedHeight, pModel.GetPtr(), pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD), pPixelType);

        pToDrawBitmap->Clear();
        pToDrawBitmap->CopyFrom(*pBitmapSrc, HRACopyFromOptions());

        DrawGDIThumbnail(pToDrawBitmap->GetPacket()->GetBufferAddress(), PreferedWidth, PreferedHeight);
    }
    else
    {
        DrawGDIThumbnail(pSrcPixels, PreferedWidth, PreferedHeight);
    }

    ::SetCursor(TheCursor);
}

//-----------------------------------------------------------------------------

void CMyPropertyPage1::OnCheckQualityThumb() 
{
    // TODO: Add your control notification handler code here
    OnPaint();	
}

//-----------------------------------------------------------------------------

void CMyPropertyPage1::OnCheckGeoThumb() 
{
    // TODO: Add your control notification handler code here
    OnPaint();
}

//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CMyPropertyPage1, CPropertyPage)
    //{{AFX_MSG_MAP(CMyPropertyPage1)
    ON_WM_PAINT()
    ON_BN_CLICKED(IDC_CHECK_QUALITY_THUMB, OnCheckQualityThumb)
    ON_BN_CLICKED(IDC_CHECK_GEO_THUMB, OnCheckGeoThumb)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
// CMyPropertyPage2 property page
//-----------------------------------------------------------------------------

CMyPropertyPage2::CMyPropertyPage2() : CPropertyPage(CMyPropertyPage2::IDD)
{
    //{{AFX_DATA_INIT(CMyPropertyPage2)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

//-----------------------------------------------------------------------------

CMyPropertyPage2::~CMyPropertyPage2()
{
}

//-----------------------------------------------------------------------------

void CMyPropertyPage2::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CMyPropertyPage2)
    DDX_Control(pDX, IDC_LIST2, m_Resource);
    //}}AFX_DATA_MAP
}

//-----------------------------------------------------------------------------
// GetHMROrigin
// Private
//-----------------------------------------------------------------------------
#if 0
HGF2DLocation GetHMROrigin () 
{
    HGF2DExtent extent =  ((HFCPtr<HRAStoredRaster>&) m_pOriginalHRARaster)->GetExtent();
    extent.ChangeCoordSys(((HFCPtr<HRAStoredRaster>&) m_pOriginalHRARaster)->GetPhysicalCoordSys());

    double orgX;
    double orgY; 

    if (m_format == IP_IMAGE_FORMAT_BMP)
    {
        orgX = extent.GetXMin();
        orgY = extent.GetYMin();
    }
    else
    {
        orgX = extent.GetXMin();
        orgY = extent.GetYMax();
    }

    HGF2DLocation location(orgX,  orgY, ((HFCPtr<HRAStoredRaster>&) m_pOriginalHRARaster)->GetPhysicalCoordSys());

    location.ChangeCoordSys(m_pWorldCluster()->GetCoordSysReference(HGF2DWorld_HMRWORLD));

    return location;
}
#endif

BOOL CMyPropertyPage2::OnInitDialog() 
{
    CPropertyPage::OnInitDialog();

    m_Resource.InsertColumn( 0, _TEXT("Resource Type"), LVCFMT_LEFT, 150);
    m_Resource.InsertColumn( 1, _TEXT("Value"), LVCFMT_LEFT, 2000);

    if (pi_rpRasterFile->CountPages() > PageNumber)
    {
        HFCPtr<HRFPageDescriptor> pPageDescriptor = pi_rpRasterFile->GetPageDescriptor(PageNumber);
        uint32_t row=0;
        WChar ValueStr[4096];

        if (pi_rpRasterFile->IsCompatibleWith(HRFIntergraphFile::CLASS_ID) &&
            ((HFCPtr<HRFIntergraphFile >&)pi_rpRasterFile)->HasLUTColorCorrection())
        {
            m_Resource.InsertItem(row, _TEXT("LUT Color Correction")); 
            m_Resource.SetItemText(row, 1, _TEXT("Is present, the file will be read only"));
            row++;            
        }

        // Page information
        if (pPageDescriptor->HasTransfoModel())
        {
            m_Resource.InsertItem(row, _TEXT("Transformation Model"));
            Utf8String TransfoStr(ConvertTransfoModelToString(pPageDescriptor->GetTransfoModel()->GetClassID()));
            m_Resource.SetItemText(row, 1, (WString(TransfoStr.c_str(), BentleyCharEncoding::Utf8)).c_str());
            row++;            

            m_Resource.InsertItem(row, _TEXT(" "));
            // Get the values to put into the matrix.
            HFCMatrix<3, 3> TheMatrix = pPageDescriptor->GetTransfoModel()->GetMatrix();
            _stprintf(ValueStr, _TEXT("%27.15lf, %27.15lf, %27.15lf,"), 
                TheMatrix[0][0], TheMatrix[0][1], TheMatrix[0][2]);

            TransfoStr = Utf8String(ValueStr);
            m_Resource.SetItemText(row, 1, (WString(TransfoStr.c_str(), BentleyCharEncoding::Utf8)).c_str());
            row++;            

            m_Resource.InsertItem(row, _TEXT(" "));

            // Get the values to put into the matrix.
            _stprintf(ValueStr, _TEXT("%27.15lf, %27.15lf, %27.15lf,"), 
                TheMatrix[1][0], TheMatrix[1][1], TheMatrix[1][2]);

            TransfoStr = Utf8String(ValueStr);
            m_Resource.SetItemText(row, 1, (WString(TransfoStr.c_str(), BentleyCharEncoding::Utf8)).c_str());
            row++;            

            m_Resource.InsertItem(row, _TEXT(" "));

            // Get the values to put into the matrix.
            _stprintf(ValueStr, _TEXT("%27.15lf, %27.15lf, %27.15lf"), TheMatrix[2][0], 
                TheMatrix[2][1], 
                TheMatrix[2][2]);

            TransfoStr = Utf8String(ValueStr);
            m_Resource.SetItemText(row, 1, (WString(TransfoStr.c_str(), BentleyCharEncoding::Utf8)).c_str());
            row++;     
            
//-------------------------------------------- GeoTiff Tag Begin     
            GeoCoordinates::BaseGCSCP pFileGeocoding = pPageDescriptor->GetGeocodingCP();
            
            if (pFileGeocoding != 0)
                {
                HCPGeoTiffKeys geoKeyList;
                pFileGeocoding->GetGeoTiffKeys(&geoKeyList, true);

                GeoCoordinates::IGeoTiffKeysList::GeoKeyItem GeoTiffKey;
                if(geoKeyList.GetFirstKey(&GeoTiffKey))
                    {
                    m_Resource.InsertItem(row, _TEXT("GeoTiff Tags")); 
                    m_Resource.SetItemText(row, 1, _TEXT("are present"));
                    row++;            
            
                    bool LineAdded;
                    Utf8String TagName;
                    do 
                    {
                        LineAdded = false;
                    
                        // GTModelType                            
                        if (GeoTiffKey.KeyID == GTModelType)
                        {
                            TagName = "  GTModelType";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        }

                        // GTRasterType
                        else if (GeoTiffKey.KeyID == GTRasterType)
                        {
                            TagName = "  GTRasterType";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        }

                        // PCSCitation
                        else if (GeoTiffKey.KeyID == PCSCitation)
                        {
                            TagName = "  PCSCitation";
                            _stprintf(ValueStr, _TEXT("%hs"), GeoTiffKey.KeyValue.StringVal);
                            LineAdded = true;
                        }

                        // ProjectedCSType
                        else if (GeoTiffKey.KeyID == ProjectedCSType)
                        {
                            TagName = "  ProjectedCSType";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        }

                        // GTCitation
                        else if (GeoTiffKey.KeyID == GTCitation)
                        {
                            TagName = "  GTCitation";
                            _stprintf(ValueStr, _TEXT("%hs"), GeoTiffKey.KeyValue.StringVal);
                            LineAdded = true;
                        }

                        // Projection
                        else if (GeoTiffKey.KeyID == Projection)
                        {
                            TagName = "  Projection";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        }

                        // ProjCoordTrans
                        else if (GeoTiffKey.KeyID == ProjCoordTrans)
                        {
                            TagName = "  ProjCoordTrans";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        }

                        // ProjLinearUnits
                        else if (GeoTiffKey.KeyID == ProjLinearUnits)
                        {
                            TagName = "  ProjLinearUnits";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        }

                        // ProjLinearUnitSize
                        else if (GeoTiffKey.KeyID == ProjLinearUnitSize)
                        {
                            TagName = "  ProjLinearUnitSize";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // GeographicType
                        else if (GeoTiffKey.KeyID == GeographicType)
                        {
                            TagName = "  GeographicType";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        }

                        // GeogCitation
                        else if (GeoTiffKey.KeyID == GeogCitation)
                        {
                            TagName = "  GeogCitation";
                            _stprintf(ValueStr, _TEXT("%hs"), GeoTiffKey.KeyValue.StringVal);
                            LineAdded = true;
                        }

                        // GeogGeodeticDatum
                        else if (GeoTiffKey.KeyID == GeogGeodeticDatum)
                        {
                            TagName = "  GeogGeodeticDatum";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        }

                        // GeogPrimeMeridian
                        else if (GeoTiffKey.KeyID == GeogPrimeMeridian)
                        {
                            TagName = "  GeogPrimeMeridian";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        }

                        // GeogLinearUnits
                        else if (GeoTiffKey.KeyID == GeogLinearUnits)
                        {
                            TagName = "  GeogLinearUnits";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        }

                        // GeogLinearUnitSize
                        else if (GeoTiffKey.KeyID == GeogLinearUnitSize)
                        {
                            TagName = "  GeogLinearUnitSize";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        }

                        // GeogAngularUnits
                        else if (GeoTiffKey.KeyID == GeogAngularUnits)
                        {
                            TagName = "  GeogAngularUnits";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        }

                        // GeogAngularUnitSize
                        else if (GeoTiffKey.KeyID == GeogAngularUnitSize)
                        {
                            TagName = "  GeogAngularUnitSize";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // GeogEllipsoid
                        else if (GeoTiffKey.KeyID == GeogEllipsoid)
                        {
                            TagName = "  GeogEllipsoid";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        }

                        // GeogSemiMajorAxis
                        else if (GeoTiffKey.KeyID == GeogSemiMajorAxis)
                        {
                            TagName = "  GeogSemiMajorAxis";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // GeogSemiMinorAxis
                        else if (GeoTiffKey.KeyID == GeogSemiMinorAxis)
                        {
                            TagName = "  GeogSemiMinorAxis";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // GeogInvFlattening
                        else if (GeoTiffKey.KeyID == GeogInvFlattening)
                        {
                            TagName = "  GeogInvFlattening";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // GeogAzimuthUnits
                        else if (GeoTiffKey.KeyID == GeogAzimuthUnits)
                        {
                            TagName = "  GeogAzimuthUnits";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        }

                        // GeogPrimeMeridianLong
                        else if (GeoTiffKey.KeyID == GeogPrimeMeridianLong)
                        {
                            TagName = "  GeogPrimeMeridianLong";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // ProjStdParallel1
                        else if (GeoTiffKey.KeyID == ProjStdParallel1)
                        {
                            TagName = "  ProjStdParallel1";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // ProjStdParallel2
                        else if (GeoTiffKey.KeyID == ProjStdParallel2)
                        {
                            TagName = "  ProjStdParallel2";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // ProjNatOriginLong
                        else if (GeoTiffKey.KeyID == ProjNatOriginLong)
                        {
                            TagName = "  ProjNatOriginLong";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // ProjNatOriginLat
                        else if (GeoTiffKey.KeyID == ProjNatOriginLat)
                        {
                            TagName = "  ProjNatOriginLat";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // ProjFalseEasting
                        else if (GeoTiffKey.KeyID == ProjFalseEasting)
                        {
                            TagName = "  ProjFalseEasting";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // ProjFalseNorthing
                        else if (GeoTiffKey.KeyID == ProjFalseNorthing)
                        {
                            TagName = "  ProjFalseNorthing";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // ProjFalseOriginLong
                        else if (GeoTiffKey.KeyID == ProjFalseOriginLong)
                        {
                            TagName = "  ProjFalseOriginLong";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // ProjFalseOriginLat
                        else if (GeoTiffKey.KeyID == ProjFalseOriginLat)
                        {
                            TagName = "  ProjFalseOriginLat";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // ProjFalseOriginEasting
                        else if (GeoTiffKey.KeyID == ProjFalseOriginEasting)
                        {
                            TagName = "  ProjFalseOriginEasting";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // ProjFalseOriginNorthing
                        else if (GeoTiffKey.KeyID == ProjFalseOriginNorthing)
                        {
                            TagName = "  ProjFalseOriginNorthing";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // ProjCenterLong
                        else if (GeoTiffKey.KeyID == ProjCenterLong)
                        {
                            TagName = "  ProjCenterLong";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // ProjCenterLat
                        else if (GeoTiffKey.KeyID == ProjCenterLat)
                        {
                            TagName = "  ProjCenterLat";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // ProjCenterEasting
                        else if (GeoTiffKey.KeyID == ProjCenterEasting)
                        {
                            TagName = "  ProjCenterEasting";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        }

                        // ProjCenterNorthing
                        else if (GeoTiffKey.KeyID == ProjCenterEasting)
                        {
                            TagName = "  ProjCenterEasting";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        } 

                        // ProjScaleAtNatOrigin
                        else if (GeoTiffKey.KeyID == ProjScaleAtNatOrigin)
                        {
                            TagName = "  ProjScaleAtNatOrigin";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        } 

                        // ProjScaleAtCenter
                        else if (GeoTiffKey.KeyID == ProjScaleAtCenter)
                        {
                            TagName = "  ProjScaleAtCenter";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        } 

                        // ProjAzimuthAngle
                        else if (GeoTiffKey.KeyID == ProjAzimuthAngle)
                        {
                            TagName = "  ProjAzimuthAngle";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        } 

                        // ProjStraightVertPoleLong
                        else if (GeoTiffKey.KeyID == ProjStraightVertPoleLong)
                        {
                            TagName = "  ProjStraightVertPoleLong";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        } 

                        // ProjRectifiedGridAngle
                        else if (GeoTiffKey.KeyID == ProjRectifiedGridAngle)
                        {
                            TagName = "  ProjRectifiedGridAngle";
                            _stprintf(ValueStr, _TEXT("%lf"), GeoTiffKey.KeyValue.DoubleVal);
                            LineAdded = true;
                        } 

                        // VerticalCSType
                        else if (GeoTiffKey.KeyID == VerticalCSType)                   
                        {
                            TagName = "  VerticalCSType";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        } 

                        // VerticalCitation
                        else if (GeoTiffKey.KeyID == VerticalCitation)             
                        {
                            TagName = "  VerticalCitation";
                            _stprintf(ValueStr, _TEXT("%hs"), GeoTiffKey.KeyValue.StringVal);
                            LineAdded = true;
                        } 

                        // VerticalDatum
                        else if (GeoTiffKey.KeyID == VerticalDatum)
                        {
                            TagName = "  VerticalDatum";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        } 

                        // VerticalUnits
                        else if (GeoTiffKey.KeyID == VerticalUnits)
                        {
                            TagName = "  VerticalUnits";
                            _stprintf(ValueStr, _TEXT("%u"), GeoTiffKey.KeyValue.LongVal);
                            LineAdded = true;
                        } 

                        if (LineAdded)
                        {
                            m_Resource.InsertItem(row, (WString(TagName.c_str(), BentleyCharEncoding::Utf8)).c_str());
                            m_Resource.SetItemText(row, 1, ValueStr);
                            row++;            
                        }
                    } while(geoKeyList.GetNextKey(&GeoTiffKey));
                }
            }

//-------------------------------------------- GeoTiff Tag End                  
        }

        if (pPageDescriptor->HasFilter())
        {
            m_Resource.InsertItem(row, _TEXT("Filter"));
            m_Resource.SetItemText(row, 1, (WString(ConvertFilterToString(pPageDescriptor->GetFilter().GetClassID()).c_str(), BentleyCharEncoding::Utf8)).c_str());
            row++;            
        }

        if (pPageDescriptor->HasClipShape())
        {
            size_t ShapeLength;

            double* dShape = ExportClipShapeToArrayOfDouble(*pPageDescriptor->GetClipShape(),
                &ShapeLength,
                1.0);

            Utf8String ShapeStr;

            if (ShapeLength > 1)
            {
                _stprintf(ValueStr, _TEXT("%.4lf"), dShape[0]);
                ShapeStr += Utf8String(ValueStr);

                for (size_t Index = 1; Index < ShapeLength; Index++)
                {              
                    _stprintf(ValueStr, _TEXT(", %.4lf"), dShape[Index]);
                    ShapeStr += Utf8String(ValueStr);
                }
            }

            delete dShape;

            m_Resource.InsertItem(row, _TEXT("Clip Shape"));    
            m_Resource.SetItemText(row, 1, (WString(ShapeStr.c_str(), BentleyCharEncoding::Utf8)).c_str());

            row++;            
        }

        if (pPageDescriptor->HasHistogram())
        {
            m_Resource.InsertItem(row, _TEXT("Histogram"));

            HFCPtr<HRPHistogram> pHistogram = pPageDescriptor->GetHistogram();
            size_t HistogramLength = pHistogram->GetEntryFrequenciesSize();
            uint32_t* pHistogramValues;
            pHistogramValues = new uint32_t[HistogramLength];
            pHistogram->GetEntryFrequencies(pHistogramValues);

            Utf8String HistogramStr;

            if (HistogramLength > 1)
            {
                _stprintf(ValueStr, _TEXT("%ld"), pHistogramValues[0]);
                HistogramStr += Utf8String(ValueStr);

                for (size_t Index = 1; Index < HistogramLength; Index++)
                {              
                    _stprintf(ValueStr, _TEXT(", %ld"), pHistogramValues[Index]);
                    HistogramStr += Utf8String(ValueStr);
                }

                m_Resource.SetItemText(row, 1, (WString(HistogramStr.c_str(), BentleyCharEncoding::Utf8)).c_str());
            }
            else             
                m_Resource.SetItemText(row, 1, _TEXT("True"));

            delete pHistogramValues;

            row++;            
        }

        if (pPageDescriptor->HasThumbnail())
        {
            m_Resource.InsertItem(row, _TEXT("Thumbnail"));
            m_Resource.SetItemText(row, 1, _TEXT("True"));
            row++;            
        }


        // Display each tag.
        HPMAttributeSet::HPMASiterator TagIterator;

        for (TagIterator  = pPageDescriptor->GetTags().begin(); TagIterator != pPageDescriptor->GetTags().end(); TagIterator++)    
        {        
            HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);

            m_Resource.InsertItem(row, (WString(pTag->GetName().c_str(), BentleyCharEncoding::Utf8)).c_str());

            bool First = true;
            bool PreviousIsALine = false;

            Utf8String StrData(pTag->GetDataAsString());
            WString StrDataW(StrData.c_str(), BentleyCharEncoding::Utf8);
            HFCStringTokenizer Lines(StrDataW, _TEXT("\r\n"));

            WString Line;
            while (Lines.Tokenize(Line))
            {
                if (!Line.empty())
                {
                    // if not the first item, add a row
                    if (!First)
                        m_Resource.InsertItem(row, _TEXT(" "));
                    First = false;

                    // add the content of this line to the resources
                    _stprintf(ValueStr, _TEXT("%s"), Line.c_str()); 
                    m_Resource.SetItemText(row, 1, ValueStr);
                    ++row;
                    PreviousIsALine = true;
                }
                else
                {
                    if (!PreviousIsALine)
                    {                     
                        m_Resource.SetItemText(row, 1, _TEXT(" "));
                        ++row;
                    }

                    PreviousIsALine = false;
                }
            }
        }

    }

    return true;  // return true unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return false
}

//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CMyPropertyPage2, CPropertyPage)
    //{{AFX_MSG_MAP(CMyPropertyPage2)
    //}}AFX_MSG_MAP

END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(ResourceListCtrl, CListCtrl)
    //{{AFX_MSG_MAP(ResourceListCtrl)
    ON_WM_KEYDOWN()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------

void  ResourceListCtrl::OnKeyDown(
                                  UINT pi_Char,
                                  UINT pi_RepCnt,
                                  UINT pi_Flags )
{
    if (IsCTRLpressed() && pi_Char == _TEXT('C'))
    {
        OnEditCopy();
    }
}



//-----------------------------------------------------------------------------

void  ResourceListCtrl::OnEditCopy()
{
    CString strData = _T("");

    for (int32_t i=0; i < GetItemCount() ;i++)
    {
        strData += GetItemText(i, 0);
        strData += _TEXT("\t \t");
        strData += GetItemText(i, 1);
        strData += _TEXT("\n");

    }


    if (OpenClipboard())
    {
        EmptyClipboard();

        HGLOBAL g_hClipboardData;
        g_hClipboardData = GlobalAlloc(GMEM_DDESHARE, 
            (strData.GetLength()+1) * sizeof(TCHAR) );

        TCHAR * pchData;
        pchData = (TCHAR*)GlobalLock(g_hClipboardData);

        _tcscpy_s(pchData, strData.GetLength() + 1, tlpcstr(strData.GetString()));

        GlobalUnlock(g_hClipboardData);

        SetClipboardData(cftxt,g_hClipboardData);

        CloseClipboard();
    }
}


//-----------------------------------------------------------------------------
// CMyPropertyPage3 property page
//-----------------------------------------------------------------------------

CMyPropertyPage3::CMyPropertyPage3() : CPropertyPage(CMyPropertyPage3::IDD)
{
    //{{AFX_DATA_INIT(CMyPropertyPage3)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

//-----------------------------------------------------------------------------

CMyPropertyPage3::~CMyPropertyPage3()
{
}

//-----------------------------------------------------------------------------

BOOL CMyPropertyPage3::OnInitDialog() 
{
    CPropertyPage::OnInitDialog();

    const HRFRasterFileCreator*   pRasterFileCreator = HRFRasterFileFactory::GetInstance()->FindCreator(pi_rpRasterFile->GetURL()); 

    if (!pRasterFileCreator->GetSupportedAccessMode().m_HasWriteAccess)
        ((CButton*)GetDlgItem(IDC_CHECK_WB))->EnableWindow(false);


    ((CButton*)GetDlgItem(IDC_CHECK_EMAIL))->SetCheck(true);

    GetDlgItem(IDC_THUMBNAILBQ)->SetWindowText(_TEXT(" "));
    GetDlgItem(IDC_THUMBNAILNN)->SetWindowText(_TEXT(" "));

    AverageReadBlockString         = " ";
    BestReadBlockString            = " ";
    WorstReadBlockString           = " ";   
    ReadTimeString                 = " ";

    AverageDecompressionString     = " ";
    BestDecompressionString        = " ";
    WorstDecompressionString       = " ";
    DeCompressionTimeString        = " ";

    GetDlgItem(IDC_READTIME)->SetWindowText((WString(ReadTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_DECOMPRESSIONTIME)->SetWindowText((WString(DeCompressionTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_AVERAGERB)->SetWindowText((WString(AverageReadBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_BESTRB)->SetWindowText((WString(BestReadBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_WORSTRB)->SetWindowText((WString(WorstReadBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_AVERAGE_DB)->SetWindowText((WString(AverageDecompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_BESTDB)->SetWindowText((WString(BestDecompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_WORSTDB)->SetWindowText((WString(WorstDecompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());

    AverageWriteBlockString         = " ";
    BestWriteBlockString            = " ";
    WorstWriteBlockString           = " ";    
    WriteTimeString                 = " ";

    AverageCompressionString        = " ";
    BestCompressionString           = " ";   
    WorstCompressionString          = " ";
    CompressionTimeString           = " ";

    GetDlgItem(IDC_WRITETIME)->SetWindowText((WString(WriteTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_COMPRESSIONTIME)->SetWindowText((WString(CompressionTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_AVERAGEWB)->SetWindowText((WString(AverageWriteBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_BESTWB)->SetWindowText((WString(BestWriteBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_WORSTWB)->SetWindowText((WString(WorstWriteBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_AVERAGECB)->SetWindowText((WString(AverageCompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_BESTCB)->SetWindowText((WString(BestCompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_WORSTCB)->SetWindowText((WString(WorstCompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());

    return true;  // return true unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return false
}

//-----------------------------------------------------------------------------

void CMyPropertyPage3::OnPaint() 
{
    CPaintDC dc(this); // device context for painting
    // Do not call CPropertyPage::OnPaint() for painting messages
    GetDlgItem(IDC_THUMBNAILBQ)->SetWindowText((WString(ThumbnailQualityMarkString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_THUMBNAILNN)->SetWindowText((WString(ThumbnailFastMarkString.c_str(), BentleyCharEncoding::Utf8)).c_str());
}

//-----------------------------------------------------------------------------

void CMyPropertyPage3::DoDataExchange(CDataExchange* pDX) 
{	
    CPropertyPage::DoDataExchange(pDX);
}

//-----------------------------------------------------------------------------

void CMyPropertyPage3::OnStartBench() 
{
    // TODO: Add your control notification handler code here
    bool                           PrintCodecTime = false;

    // Reset Stat Mark
    AverageReadBlockString         = " ";
    BestReadBlockString            = " ";
    WorstReadBlockString           = " ";   
    ReadTimeString                 = " ";

    AverageDecompressionString     = " ";
    BestDecompressionString        = " ";
    WorstDecompressionString       = " ";
    DeCompressionTimeString        = " ";

    GetDlgItem(IDC_READTIME)->SetWindowText((WString(ReadTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_DECOMPRESSIONTIME)->SetWindowText((WString(DeCompressionTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_AVERAGERB)->SetWindowText((WString(AverageReadBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_BESTRB)->SetWindowText((WString(BestReadBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_WORSTRB)->SetWindowText((WString(WorstReadBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_AVERAGE_DB)->SetWindowText((WString(AverageDecompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_BESTDB)->SetWindowText((WString(BestDecompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_WORSTDB)->SetWindowText((WString(WorstDecompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());

    AverageWriteBlockString         = " ";
    BestWriteBlockString            = " ";
    WorstWriteBlockString           = " ";    
    WriteTimeString                 = " ";

    AverageCompressionString        = " ";
    BestCompressionString           = " ";   
    WorstCompressionString          = " ";
    CompressionTimeString           = " ";

    GetDlgItem(IDC_WRITETIME)->SetWindowText((WString(WriteTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_COMPRESSIONTIME)->SetWindowText((WString(CompressionTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_AVERAGEWB)->SetWindowText((WString(AverageWriteBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_BESTWB)->SetWindowText((WString(BestWriteBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_WORSTWB)->SetWindowText((WString(WorstWriteBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_AVERAGECB)->SetWindowText((WString(AverageCompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_BESTCB)->SetWindowText((WString(BestCompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_WORSTCB)->SetWindowText((WString(WorstCompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());

    try 

    {
        // Use Env for the location
        WChar pDir[MAX_PATH + MAX_PATH];

        // Try to get the HMR temp directory ("HMRTempDirectory")
        if (_tgetenv_s (0, pDir, MAX_PATH + MAX_PATH, _TEXT("HMRTempDirectory")) == 0 ||
            _tgetenv_s (0, pDir, MAX_PATH + MAX_PATH, _TEXT("TMP")) == 0       ||
            _tgetenv_s (0, pDir, MAX_PATH + MAX_PATH, _TEXT("TEMP")) == 0      )
        {
            HCURSOR TheCursor = ::SetCursor(LoadCursor(NULL, IDC_WAIT));

            // Re-Open the image file
            pi_rpRasterFile = 0;
            HFCPtr<HFCURLFile>    SrcFileName = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(GetLongPathName(Utf8String(AfxGetApp()->m_lpCmdLine))));
            pi_rpRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)SrcFileName, true); 

            MarkWriteBlock = ((CButton*)GetDlgItem(IDC_CHECK_WB))->GetCheck() != 0;

            // Source      
            HAutoPtr<HRFResolutionEditor>   pSrcResolutionEditor;
            HFCPtr<HRFResolutionDescriptor> pSrcResolutionDescriptor;
            HFCAccessMode                   SrcEditorAccess = HFC_READ_ONLY;

            // Destination
            HFCPtr<HRFRasterFile>           pDestination;
            HAutoPtr<HRFResolutionEditor>   pDstResolutionEditor;
            HFCPtr<HFCURLFile>              DestFileName;
            HFCAccessMode                   DstEditorAccess = HFC_WRITE_ONLY;
            Chronometer                     SrcChrono;
            Chronometer                     DstChrono;
            Chronometer                     SrcCodecChrono;
            Chronometer                     DstCodecChrono;
            Chronometer                     GlobalChrono;
            Chronometer                     OpenChrono;
            Chronometer                     CreateChrono;
            bool                           CalcCodecTime = true;

            GlobalChrono.Start();

            //----------------------------------------------------------------------------------------
            // Create the destination URL 
            Utf8String Extension;
            Utf8String DriveDirName;

            // Extract the Path
            Utf8String Path(((HFCPtr<HFCURLFile>&)pi_rpRasterFile->GetURL())->GetHost() + "\\" + ((HFCPtr<HFCURLFile>&)pi_rpRasterFile->GetURL())->GetPath());

            // Find the file extension
            Utf8String::size_type DotPos = Path.rfind('.');

            // Extract the extension and the drive dir name
            if (DotPos != Utf8String::npos)
            {
                Extension = Path.substr(DotPos+1, Path.length() - DotPos - 1);
                //DriveDirName = Path.substr(0, DotPos);
                DriveDirName = Utf8String(pDir);
                DriveDirName += "\\";
            }
            else
            {
                cout << "Can't create the destination file." << endl;
                exit(1);
            }

            if (MarkWriteBlock)
            {
                //----------------------------------------------------------------------------------------
                // Create the destination file.
                DestFileName = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + DriveDirName + "ImageInsider.clone." + Extension); 

                CreateChrono.Start();
                pDestination = HRFRasterFileFactory::GetInstance()->NewFile((HFCPtr<HFCURL>)DestFileName);
                CreateChrono.Stop();
                if (pDestination == 0)
                {
                    cout << "Enable to create the output file." << endl;
                    exit(1);
                }
            }

            //----------------------------------------------------------------------------------------
            // Clone the file
            {
                uint32_t                    Page = 0;
                HFCPtr<HRFPageDescriptor>   pPageDescriptor = pi_rpRasterFile->GetPageDescriptor(Page);        

                if (MarkWriteBlock)
                {
                    // Add the page to the destination
                    CreateChrono.Start();
                    pDestination->AddPage(pPageDescriptor);
                    CreateChrono.Stop();
                }

                // Copy all resolutions.
                for (unsigned short Resolution=0 ; Resolution<pPageDescriptor->CountResolutions() ; Resolution++)
                {            
                    // Create the destination and source editor.
                    pSrcResolutionEditor     = pi_rpRasterFile->CreateResolutionEditor(Page, Resolution, SrcEditorAccess);
                    if (MarkWriteBlock)
                        pDstResolutionEditor     = pDestination->CreateResolutionEditor(Page, Resolution, DstEditorAccess);
                    pSrcResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor(Resolution);

                    Byte* pTileBuffer = (Byte*)GlobalAlloc(GMEM_FIXED, pSrcResolutionDescriptor->GetBlockSizeInBytes());

                    if (pSrcResolutionDescriptor->GetCodec() != 0 &&  
                        pSrcResolutionDescriptor->GetCodec()->GetClassID() == HCDCodecIdentity::CLASS_ID)
                    {
                        CalcCodecTime = false;
                        PrintCodecTime = false;
                    }
                    else
                    {
                        CalcCodecTime = true;
                        PrintCodecTime = true;
                    }
                    // Copy the resolution block by block.
                    for (uint32_t PosY=0; PosY<pSrcResolutionDescriptor->GetHeight(); PosY += pSrcResolutionDescriptor->GetBlockHeight())
                    {
                        for (uint32_t PosX=0; PosX<pSrcResolutionDescriptor->GetWidth(); PosX += pSrcResolutionDescriptor->GetBlockWidth())
                        {
                            if (CalcCodecTime)
                            {
                                HFCPtr<HCDPacket> pSrcPacket = new HCDPacket();
                                SrcChrono.Start();
                                pSrcResolutionEditor->ReadBlock(PosX, PosY, pSrcPacket);
                                SrcChrono.Stop();

                                if ((pSrcPacket->GetCodec() == 0) || 
                                    (pSrcPacket->GetCodec()->GetClassID() == HCDCodecIdentity::CLASS_ID))
                                    PrintCodecTime = false;

                                // create a packet to store the source pixels uncompressed
                                HArrayAutoPtr<Byte> pSrcPixels(new Byte[pSrcResolutionDescriptor->GetBlockSizeInBytes()]);
                                HFCPtr<HCDPacket> pDecompressedSrcPacket(new HCDPacket(pSrcPixels, pSrcResolutionDescriptor->GetBlockSizeInBytes(), pSrcResolutionDescriptor->GetBlockSizeInBytes()));
                                HASSERT (pSrcPacket->GetCodec() != 0);

                                SrcCodecChrono.Start();
                                pSrcPacket->Decompress(pDecompressedSrcPacket);
                                SrcCodecChrono.Stop();

                                if  (MarkWriteBlock)
                                {
                                    HFCPtr<HCDPacket> pDstCompressedPacket = new HCDPacket(pSrcPacket->GetCodec(), 0, 0, 0);
                                    pDstCompressedPacket->SetBufferOwnership(true);	

                                    DstCodecChrono.Start();
                                    pDecompressedSrcPacket->Compress(pDstCompressedPacket);
                                    DstCodecChrono.Stop();

                                    DstChrono.Start();
                                    pDstResolutionEditor->WriteBlock(PosX, PosY, pDstCompressedPacket);
                                    DstChrono.Stop();
                                }
                            }
                            else
                            {
                                SrcChrono.Start();
                                pSrcResolutionEditor->ReadBlock(PosX, PosY, pTileBuffer);
                                SrcChrono.Stop();
                                if (MarkWriteBlock)
                                {
                                    DstChrono.Start();
                                    pDstResolutionEditor->WriteBlock(PosX, PosY, pTileBuffer);
                                    DstChrono.Stop();
                                }
                            }

                        }
                        WChar Buffer[1024];
                        _stprintf(Buffer, _TEXT(""));
                        //sprintf(Buffer, ", block %ld of %ld", pSrcResolutionDescriptor->ComputeBlockIndex(PosX, PosY), pSrcResolutionDescriptor->CountBlocks());

                        AverageReadBlockString         = SrcChrono.GetAverageDuration() + " s.";
                        BestReadBlockString            = SrcChrono.GetBestDuration() + " s.";     
                        WorstReadBlockString           = SrcChrono.GetWorstDuration() + " s.";    
                        ReadTimeString                 = SrcChrono.GetDuration() + " s." + Utf8String(Buffer);

                        GetDlgItem(IDC_READTIME)->SetWindowText((WString(ReadTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
                        GetDlgItem(IDC_AVERAGERB)->SetWindowText((WString(AverageReadBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
                        GetDlgItem(IDC_BESTRB)->SetWindowText((WString(BestReadBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
                        GetDlgItem(IDC_WORSTRB)->SetWindowText((WString(WorstReadBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());

                        if (PrintCodecTime)
                        {
                            AverageDecompressionString        = SrcCodecChrono.GetAverageDuration() + " s."; 
                            BestDecompressionString            = SrcCodecChrono.GetBestDuration() + " s." ;     
                            WorstDecompressionString           = SrcCodecChrono.GetWorstDuration() + " s." ;    
                            DeCompressionTimeString = SrcCodecChrono.GetDuration() + " s." + Utf8String(Buffer);
                            GetDlgItem(IDC_DECOMPRESSIONTIME)->SetWindowText((WString(DeCompressionTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
                            GetDlgItem(IDC_AVERAGE_DB)->SetWindowText((WString(AverageDecompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
                            GetDlgItem(IDC_BESTDB)->SetWindowText((WString(BestDecompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
                            GetDlgItem(IDC_WORSTDB)->SetWindowText((WString(WorstDecompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
                        }

                        if (MarkWriteBlock)
                        {
                            AverageWriteBlockString         = DstChrono.GetAverageDuration() + " s." ;
                            BestWriteBlockString            = DstChrono.GetBestDuration() + " s." ;     
                            WorstWriteBlockString           = DstChrono.GetWorstDuration() + " s." ;    
                            WriteTimeString         = DstChrono.GetDuration() + " s." + Utf8String(Buffer);
                            GetDlgItem(IDC_WRITETIME)->SetWindowText((WString(WriteTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
                            GetDlgItem(IDC_AVERAGEWB)->SetWindowText((WString(AverageWriteBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
                            GetDlgItem(IDC_BESTWB)->SetWindowText((WString(BestWriteBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
                            GetDlgItem(IDC_WORSTWB)->SetWindowText((WString(WorstWriteBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());

                            if (PrintCodecTime)
                            {
                                AverageCompressionString         = DstCodecChrono.GetAverageDuration() + " s." ;
                                BestCompressionString            = DstCodecChrono.GetBestDuration() + " s." ;     
                                WorstCompressionString           = DstCodecChrono.GetWorstDuration() + " s." ;    
                                CompressionTimeString   = DstCodecChrono.GetDuration() + " s." + Utf8String(Buffer);
                                GetDlgItem(IDC_COMPRESSIONTIME)->SetWindowText((WString(CompressionTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
                                GetDlgItem(IDC_AVERAGECB)->SetWindowText((WString(AverageCompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
                                GetDlgItem(IDC_BESTCB)->SetWindowText((WString(BestCompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
                                GetDlgItem(IDC_WORSTCB)->SetWindowText((WString(WorstCompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
                            }
                        }

                    }
                    GlobalFree(pTileBuffer);
                }
            }
            GlobalChrono.Stop();

            AverageReadBlockString         = SrcChrono.GetAverageDuration() + " s.";
            BestReadBlockString            = SrcChrono.GetBestDuration() + " s.";     
            WorstReadBlockString           = SrcChrono.GetWorstDuration() + " s.";    
            ReadTimeString                 = SrcChrono.GetDuration() + " s.";

            if (PrintCodecTime)
            {
                AverageDecompressionString        = SrcCodecChrono.GetAverageDuration() + " s."; 
                BestDecompressionString            = SrcCodecChrono.GetBestDuration() + " s.";     
                WorstDecompressionString           = SrcCodecChrono.GetWorstDuration() + " s.";    
                DeCompressionTimeString = SrcCodecChrono.GetDuration() + " s.";
            }

            if (MarkWriteBlock)
            {
                AverageWriteBlockString         = DstChrono.GetAverageDuration() + " s.";
                BestWriteBlockString            = DstChrono.GetBestDuration() + " s.";     
                WorstWriteBlockString           = DstChrono.GetWorstDuration() + " s.";    
                WriteTimeString         = DstChrono.GetDuration() + " s.";

                if (PrintCodecTime)
                {
                    AverageCompressionString         = DstCodecChrono.GetAverageDuration() + " s.";
                    BestCompressionString            = DstCodecChrono.GetBestDuration() + " s.";     
                    WorstCompressionString           = DstCodecChrono.GetWorstDuration() + " s.";    
                    CompressionTimeString   = DstCodecChrono.GetDuration() + " s.";
                }
            }


#ifdef __HMR_WINDOWS
            if (MarkWriteBlock)
            {
                // Close the destination file
                pDestination = 0;

                // Erase the destination file
                Utf8String FileName(DestFileName->GetHost());
                FileName += "\\";
                FileName += DestFileName->GetPath();   
                _tunlink(FileName.c_str());
            }
#endif

            if (((CButton*)GetDlgItem(IDC_CHECK_EMAIL))->GetCheck())
            {
            HASSERT(!"Disabled 'mail.hmrinc.com' doesn't exist anymore");
#if 0 
                HFCSMTPConnection Connection(_TEXT("mail.hmrinc.com"), 25);

                HFCSysCfg SysCfg;
                Utf8String Msg;


                Msg += "Sender information:\r\n\r\n"; 

                Msg += "   Operating System.............: " + Utf8String(SysCfg.GetOperatingSystemStr()) + "\r\n";
                Msg += "   Architecture.................: " + Utf8String(SysCfg.GetArchitectureTypeStr()) + "\r\n";
                Msg += "   Processor....................: " + Utf8String(SysCfg.GetProcessorTypeStr()) + "\r\n";
                Msg += "   Processor Level..............: " + Utf8String(SysCfg.GetProcessorLevelStr()) + "\r\n";
                Msg += "   Additional Information.......: " + Utf8String(SysCfg.GetAdditionalInfoStr()) + "\r\n";

                Msg += "\r\n\r\nImage Information:\r\n\r\n"; 

                Msg += "   Filename.....................: " + FilenameString + "\r\n";
                Msg += "   Location.....................: " + LocationString + "\r\n";
                Msg += "   Format.......................: " + FormatTypeString + "\r\n";
                Msg += "   Size.........................: " + FileSizeString + "\r\n";
                Msg += "   Uncompressed Size............: " + UncompressedSizeString + "\r\n";
                Msg += "   Image Dimensions.............: " + ImageDimensionString + "\r\n";    
                Msg += "   Compression..................: " + CompressionTypesString + "\r\n";
                Msg += "   Color Space..................: " + ColorSpaceString + "\r\n";   
                Msg += "   Scanline Orientation.........: " + ScanlineOrientationString + "\r\n";
                Msg += "   Overviews Dimensions.........: " + OverviewDimensionString + "\r\n";
                Msg += "   Number Of Block..............: " + NumberOfBlockString + "\r\n";
                Msg += "   Blocks Dimensions............: " + BlockDimensionString + "\r\n";


                Msg += "\r\n\r\nBenchmark:\r\n\r\n"; 

                if (IsQualityThumbnailCalculated)
                    Msg += "   Thumbnail Quality............:" + ThumbnailQualityMarkString + "\r\n";  

                if (IsFastThumbnailCalculated)
                    Msg += "   Thumbnail NN.................:" + ThumbnailFastMarkString + "\r\n";   

                Msg += "   Average read block...........:" + AverageReadBlockString + "\r\n";         
                Msg += "   Best read block..............:" + BestReadBlockString + "\r\n";                    
                Msg += "   Worst read block.............:" + WorstReadBlockString + "\r\n";                   
                Msg += "   Read Time....................:" + ReadTimeString + "\r\n";                  

                if (PrintCodecTime)
                {
                    Msg += "   Average Decompression block..:" + AverageDecompressionString + "\r\n";                
                    Msg += "   Best Decompression block.....:" + BestDecompressionString + "\r\n";                    
                    Msg += "   Worst Decompression block....:" + WorstDecompressionString + "\r\n";                   
                    Msg += "   Decompression Time...........:" + DeCompressionTimeString + "\r\n";         
                }

                if (MarkWriteBlock)
                {
                    Msg += "   Average Write Block..........:" + AverageWriteBlockString + "\r\n";                 
                    Msg += "   Best Write Block.............:" + BestWriteBlockString + "\r\n";                    
                    Msg += "   Worst Write Block............:" + WorstWriteBlockString + "\r\n";                   
                    Msg += "   Write Time...................:" + WriteTimeString + "\r\n";                 

                    if (PrintCodecTime)
                    {   
                        Msg += "   Average Compression Block....:" + AverageCompressionString + "\r\n";                 
                        Msg += "   Best Compression Block.......:" + BestCompressionString + "\r\n";                    
                        Msg += "   Worst Compression Block......:" + WorstCompressionString + "\r\n";                   
                        Msg += "   Compression Time.............:" + CompressionTimeString + "\r\n";           
                    }
                }

                Connection.SetReplyAddress("bench@hmrinc.com");
                Connection.SetSender("bench@hmrinc.com");
                Connection.AddRecipient("Donald.Morissette@hmrinc.com");
                Connection.SetSubject("Image Insider");
                Connection.SetMessageText(Msg.c_str());
                Connection.Send();
#endif
            }

            ::SetCursor(TheCursor);
        }
    }
    catch (...)
    {
        Utf8String Msg;

        try
        {
            throw;
        }
        catch(HFCException& rException)
        {               
            CImageInsiderApp::GetInstance()->HandleIppException(rException, Msg);        
        }  
        catch(logic_error&)
        {
            Msg = "Logic error";
        }
        catch(runtime_error&)
        {
            Msg = "Runtime error";
        }
        catch(exception&)
        {
            Msg = "Standard lib. error";
        }
        catch(...)
        {
            Msg = "Unknown error";
        }

        AfxMessageBox(WString(Msg.c_str(), BentleyCharEncoding::Utf8).c_str());
    }

    GetDlgItem(IDC_READTIME)->SetWindowText((WString(ReadTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_AVERAGERB)->SetWindowText((WString(AverageReadBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_BESTRB)->SetWindowText((WString(BestReadBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    GetDlgItem(IDC_WORSTRB)->SetWindowText((WString(WorstReadBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());

    if (PrintCodecTime)
    {
        GetDlgItem(IDC_DECOMPRESSIONTIME)->SetWindowText((WString(DeCompressionTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
        GetDlgItem(IDC_AVERAGE_DB)->SetWindowText((WString(AverageDecompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
        GetDlgItem(IDC_BESTDB)->SetWindowText((WString(BestDecompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
        GetDlgItem(IDC_WORSTDB)->SetWindowText((WString(WorstDecompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
    }

    if (MarkWriteBlock)
    {
        GetDlgItem(IDC_WRITETIME)->SetWindowText((WString(WriteTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
        GetDlgItem(IDC_AVERAGEWB)->SetWindowText((WString(AverageWriteBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
        GetDlgItem(IDC_BESTWB)->SetWindowText((WString(BestWriteBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());
        GetDlgItem(IDC_WORSTWB)->SetWindowText((WString(WorstWriteBlockString.c_str(), BentleyCharEncoding::Utf8)).c_str());

        if (PrintCodecTime)
        {
            GetDlgItem(IDC_AVERAGECB)->SetWindowText((WString(AverageCompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
            GetDlgItem(IDC_BESTCB)->SetWindowText((WString(BestCompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
            GetDlgItem(IDC_WORSTCB)->SetWindowText((WString(WorstCompressionString.c_str(), BentleyCharEncoding::Utf8)).c_str());
            GetDlgItem(IDC_COMPRESSIONTIME)->SetWindowText((WString(CompressionTimeString.c_str(), BentleyCharEncoding::Utf8)).c_str());
        }
    }
}

//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CMyPropertyPage3, CPropertyPage)
    //{{AFX_MSG_MAP(CMyPropertyPage3)
    ON_WM_PAINT()
    ON_BN_CLICKED(IDC_BUTTON1, OnStartBench)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------

