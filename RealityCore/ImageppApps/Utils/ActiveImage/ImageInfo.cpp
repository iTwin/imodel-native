/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ImageInfo.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ImageInfo.cpp : implementation file
//

#include "stdafx.h"
#include "ActiveImage.h"
#include "ImageInfo.h"
#include "afxdialogex.h"
#include "ActiveImageFrame.h"

#include "Labeler.h"
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFPageFileFactory.h>
#include <Imagepp/all/h/HRFRasterFilePageDecorator.h>



// ImageInfo dialog

IMPLEMENT_DYNAMIC(ImageInfo, CDialogEx)

/*=================================================================================**//**
*
* Generated VS MFC class. Functions used to fill member attributes are from ImageInsider
*
* @bsiclass                         Alexandre.Gagnon                              11/2014      
+===============+===============+===============+===============+===============+======*/
ImageInfo::ImageInfo(CWnd* pParent /*=NULL*/): CDialog(ImageInfo::IDD, pParent),
                                    m_Type(_T("")), m_Size(_T("")), m_SizeUncomp(_T("")),
                                    m_Dimensions(_T("")), m_TypeComp(_T("")), m_ColorSpace(_T("")),
                                    m_Scanline(_T("")), m_BlockCount(_T("")), m_BlockSize(_T("")),
                                    m_GcsName(_T("")), m_GcsUnits(_T("")), m_DatumName(_T(""))
    {
    Create(ImageInfo::IDD, pParent);
    }

ImageInfo::~ImageInfo()
    {
    }

BOOL ImageInfo::Create(UINT nID, CWnd* pParentWnd)
    {
    return CDialog::Create(nID, pParentWnd);
    }


void ImageInfo::EmptyValues()
    {
    m_Type = "";
    m_Size = "";
    m_SizeUncomp = "";
    m_Dimensions = "";
    m_TypeComp = "";
    m_ColorSpace = "";
    m_Scanline = "";
    m_BlockCount = "";
    m_BlockSize = "";
    m_GcsName = "";
    m_GcsUnits = "";
    m_DatumName = "";

    UpdateData(false);
    }

void ImageInfo::SetImageInfo()
    { 
    // Get reference on file
    CWnd* pMainWnd = AfxGetMainWnd();
    CActiveImageFrame* pFrame;
    CTreeCtrl*         pTree;
    pFrame = dynamic_cast<CActiveImageFrame*>(pMainWnd);
    pTree = pFrame->GetSelector();
    CStringW FileNameW = pTree->GetItemText(pTree->GetSelectedItem());

    if (FileNameW == L"Document")
        {
        // NO file is provided. Remove all (possibly) written values and return.
        EmptyValues();
        return;
        }

    Utf8String filename;
    BeStringUtilities::WCharToUtf8(filename, FileNameW.GetString());

    HFCPtr<HFCURL>  SrcFileName(HFCURL::Instanciate(filename));
    if (SrcFileName == 0)
        {
        // Open the raster file as a file
        SrcFileName = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + filename);
        }

    HFCStat Stat(SrcFileName);

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

    m_Size = WString(buffer).c_str();

    // Compute string description
    HFCPtr<HRFRasterFile> pi_rpRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)SrcFileName, true);
   
    // Add the Decoration HGR or the TFW Page File
    if (HRFPageFileFactory::GetInstance()->HasFor(pi_rpRasterFile))
        pi_rpRasterFile = new HRFRasterFilePageDecorator(pi_rpRasterFile, HRFPageFileFactory::GetInstance()->FindCreatorFor(pi_rpRasterFile));

    m_Type          = GetFormatType(pi_rpRasterFile).c_str();
    m_SizeUncomp    = GetUncompressedSize(pi_rpRasterFile, 0).c_str();
    m_Dimensions    = GetImageDimension(pi_rpRasterFile, 0).c_str();
    m_TypeComp      = GetCompressionTypes(pi_rpRasterFile, 0).c_str();
    m_ColorSpace    = GetColorSpace(pi_rpRasterFile, 0).c_str();   
    m_Scanline      = GetScanlineOrientation(pi_rpRasterFile, 0).c_str();
    m_BlockCount    = GetNumberOfBlock(pi_rpRasterFile, 0).c_str();
    m_BlockSize     = GetBlockDimension(pi_rpRasterFile, 0).c_str();
    m_GcsName       = GetName(pi_rpRasterFile, 0).c_str();
    m_GcsUnits      = GetUnits(pi_rpRasterFile, 0).c_str();
    m_DatumName     = GetDatumName(pi_rpRasterFile, 0).c_str();

    UpdateData(false);
    }

void ImageInfo::DoDataExchange(CDataExchange* pDX)
    {
	CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_INFO_TYPE, m_Type);
    DDX_Text(pDX, IDC_INFO_SIZE, m_Size);
    DDX_Text(pDX, IDC_INFO_SIZE_UNCOMP, m_SizeUncomp);
    DDX_Text(pDX, IDC_INFO_DIMENSIONS, m_Dimensions);
    DDX_Text(pDX, IDC_INFO_TYPE_COMP, m_TypeComp);
    DDX_Text(pDX, IDC_INFO_COLOR_SPACE, m_ColorSpace);
    DDX_Text(pDX, IDC_INFO_SCANLINE, m_Scanline);
    DDX_Text(pDX, IDC_INFO_BLOCK_COUNT, m_BlockCount);
    DDX_Text(pDX, IDC_INFO_BLOCK_SIZE, m_BlockSize);
    DDX_Text(pDX, IDC_INFO_GCS_NAME, m_GcsName);
    DDX_Text(pDX, IDC_INFO_GCS_UNITS, m_GcsUnits);
    DDX_Text(pDX, IDC_INFO_DATUM_NAME, m_DatumName);
    }

BEGIN_MESSAGE_MAP(ImageInfo, CDialog)
END_MESSAGE_MAP()

BOOL ImageInfo::DestroyWindow()
    {
    return CDialog::DestroyWindow();
    }

// ImageInfo message handlers
