/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/OpenRawFileDialog.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// OpenRawFileDialog.cpp : implementation file
//

#include "stdafx.h"
#include "activeimage.h"
#include "OpenRawFileDialog.h"
#include <Imagepp/h/HTypes.h>
#include <Imagepp/all/h/HRFRawFile.h>
#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HUTClassIDDescriptor.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HRFHGRPageFile.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCIniFileBrowser.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define GEOREFSETTING       "GeoRefSetting"
#define IMAGE_WIDTH         "Image_Width"
#define IMAGE_HEIGHT        "Image_Height"

/////////////////////////////////////////////////////////////////////////////
// COpenRawFileDialog dialog



COpenRawFileDialog::COpenRawFileDialog(CWnd* pParent /*=NULL*/, 
                                       const WChar* pi_pFileName, 
                                       const uint32_t pi_Offset,
                                       const uint32_t pi_Footer)
	: CDialog(COpenRawFileDialog::IDD, pParent),     
    m_pFileName(new HFCURLFile(HFCURLFile::s_SchemeName() + "://" +Utf8String(pi_pFileName)))
{    

    int  Pos      = (CString(pi_pFileName)).ReverseFind(_TEXT('\\')) + 1;
    
    //{{AFX_DATA_INIT(COpenRawFileDialog)
	m_Height = 0;
	m_Width = 0;
	m_Footer = pi_Footer;
	m_UseSisterFile = false;
	m_Offset = pi_Offset;
	m_FileName = "<" + CString(&pi_pFileName[Pos]) + ">";
	m_FileSize = _T("");
	m_AutoDetect = true;
	//}}AFX_DATA_INIT
}


void COpenRawFileDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpenRawFileDialog)
	DDX_Control(pDX, IDC_PIXELTYPE, m_PixelTypeCmb);
	DDX_Text(pDX, IDC_HEIGHT, m_Height);
	DDX_Text(pDX, IDC_WIDTH, m_Width);
	DDX_Text(pDX, IDC_FOOTER, m_Footer);
	DDX_Check(pDX, IDC_USE_SISTER_FILE, m_UseSisterFile);
	DDX_Text(pDX, IDC_OFFSET, m_Offset);
	DDX_Text(pDX, IDC_FILENAME, m_FileName);
	DDX_Text(pDX, IDC_FILESIZE, m_FileSize);
	DDX_Check(pDX, IDC_AUTODETECT, m_AutoDetect);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COpenRawFileDialog, CDialog)
	//{{AFX_MSG_MAP(COpenRawFileDialog)
	ON_EN_CHANGE(IDC_HEIGHT, OnChangeHeight)
	ON_EN_CHANGE(IDC_WIDTH, OnChangeWidth)
	ON_BN_CLICKED(IDC_SWAP, OnSwap)
	ON_CBN_SELCHANGE(IDC_PIXELTYPE, OnSelchangePixeltype)
	ON_EN_CHANGE(IDC_OFFSET, OnChangeOffset)
	ON_EN_CHANGE(IDC_FOOTER, OnChangeFooter)
	ON_BN_CLICKED(IDC_AUTODETECT, OnAutodetect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpenRawFileDialog message handlers

BOOL COpenRawFileDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

    HFCPtr<HFCURL>  URLForPageFile = HRFHGRPageFileCreator::GetInstance()->ComposeURLFor(m_pFileName);
    if (HasFor(URLForPageFile))
    {
        ReadHGRFile(URLForPageFile, &m_Width, &m_Height);
        m_AutoDetect = false;
        m_UseSisterFile = true;
        GetDlgItem(IDC_AUTODETECT)->EnableWindow(false);
        GetDlgItem(IDC_WIDTH)->EnableWindow(false);
        GetDlgItem(IDC_HEIGHT)->EnableWindow(false);        
    }
    else
        HRFRawCreator::GetInstance()->AutoDetectFileSize(m_pFileName, m_Offset, m_Footer, 24, m_Width, m_Height);

    
    HFCPtr<HRPPixelType>                pPixelType = 
        HRFRawCreator::GetInstance()->AutoDetectPixelType(m_pFileName, m_Offset, m_Footer, m_Width, m_Height);

    HFCPtr<HRFRasterFileCapabilities>   pListPixelTypeCap = 
        HRFRawCreator::GetInstance()->GetCapabilities()->GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID, HFC_READ_ONLY);
    int                                 ItemIndex;
    int                                 SelectedItem;
    uint64_t                           FileSize = HRFRawCreator::GetInstance()->GetFileSize();
    ostringstream                       cFileSize;

    cFileSize << "<" << FileSize << " bytes>";
    m_FileSize = (CString)(cFileSize.str().c_str());

    m_PixelTypeCmb.ResetContent();

    // Fill the combo box with the possible pixel type for this format.
    for (uint32_t Index=0; Index<pListPixelTypeCap->CountCapabilities(); Index++)
    {
        HFCPtr<HRFPixelTypeCapability> pPixelTypeCap = 
            (const HFCPtr<HRFPixelTypeCapability>&)pListPixelTypeCap->GetCapability(Index);
       
        WString labelW(HUTClassIDDescriptor::GetInstance()->GetClassLabelPixelType(pPixelTypeCap->GetPixelTypeClassID()).c_str(), BentleyCharEncoding::Utf8);
        ItemIndex = m_PixelTypeCmb.AddString(labelW.c_str());

        m_PixelTypeCmb.SetItemData(ItemIndex, pPixelTypeCap->GetPixelTypeClassID());

        if (pPixelType->GetClassID() == pPixelTypeCap->GetPixelTypeClassID())
            SelectedItem = ItemIndex;
    }
    m_PixelTypeCmb.SetCurSel(SelectedItem);



    UpdateData(false);
	
	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}

void COpenRawFileDialog::OnChangeHeight() 
{   
    UpdateData();

    if (m_AutoDetect)
        m_Width = HRFRawCreator::GetInstance()->GetBestFitWidth(m_Height, m_Offset, m_Footer);

    UpdateData(false);
}

void COpenRawFileDialog::OnChangeWidth() 
{
    UpdateData();

    if (m_AutoDetect)
        m_Height = HRFRawCreator::GetInstance()->GetBestFitHeight(m_Width, m_Offset, m_Footer);

    UpdateData(false);
}

void COpenRawFileDialog::OnSwap() 
{
    UINT Swap;

    UpdateData();

    Swap = m_Width;
    m_Width = m_Height;
    m_Height = Swap;

    UpdateData(false);
}

void COpenRawFileDialog::OnSelchangePixeltype() 
{
    UpdateData();

    HFCPtr<HRPPixelType> pPixelType = HRPPixelTypeFactory::GetInstance()->Create((HCLASS_ID)m_PixelTypeCmb.GetItemData(m_PixelTypeCmb.GetCurSel()));

    HRFRawCreator::GetInstance()->SetImagePixelType(pPixelType);

    if (m_AutoDetect)
        HRFRawCreator::GetInstance()->AutoDetectFileSize(m_pFileName, 
                                                         m_Offset, 
                                                         m_Footer, 
                                                         (Byte)pPixelType->CountPixelRawDataBits(), 
                                                         m_Width, 
                                                         m_Height);
 
    UpdateData(false);
	
}

void COpenRawFileDialog::OnChangeOffset() 
{
    UpdateData();

    if (m_AutoDetect)
        HRFRawCreator::GetInstance()->AutoDetectFileSize(m_pFileName, 
                                                         m_Offset, 
                                                         m_Footer, 
                                                         (Byte)HRFRawCreator::GetInstance()->GetImagePixelType()->CountPixelRawDataBits(),
                                                         m_Width,
                                                         m_Height);

    UpdateData(false);
}

void COpenRawFileDialog::OnChangeFooter() 
{
    UpdateData();

    if (m_AutoDetect)
        HRFRawCreator::GetInstance()->AutoDetectFileSize(m_pFileName, 
                                                         m_Offset, 
                                                         m_Footer, 
                                                         (Byte)HRFRawCreator::GetInstance()->GetImagePixelType()->CountPixelRawDataBits(),
                                                         m_Width,
                                                         m_Height);

    UpdateData(false);
}

void COpenRawFileDialog::OnAutodetect() 
{
    UpdateData();

    if (m_AutoDetect)
        HRFRawCreator::GetInstance()->AutoDetectFileSize(m_pFileName,
                                                         m_Offset,
                                                         m_Footer,
                                                         (Byte)HRFRawCreator::GetInstance()->GetImagePixelType()->CountPixelRawDataBits(),
                                                         m_Width,
                                                         m_Height);

    UpdateData(false);
}

void COpenRawFileDialog::OnOK() 
{
    UpdateData();

    HRFRawCreator::GetInstance()->SetImageData(m_Width, m_Height, m_Offset);
    
	CDialog::OnOK();
}

// Grayed the checkbox if return false
bool COpenRawFileDialog::HasFor(HFCPtr<HFCURL>&  pi_rpForRasterFile) const
{
    HPRECONDITION(pi_rpForRasterFile != 0);
    bool HasPageFile = false;
//    HFCPtr<HFCURL>  URLForPageFile = HRFHGRPageFileCreator::GetInstance()->ComposeURLFor(pi_rpForRasterFile);
    if (pi_rpForRasterFile != 0)
    {
        HFCStat PageFileStat(pi_rpForRasterFile);
    
        // Check if the decoration file exist and the time stamp.
        if (PageFileStat.IsExistent())
            HasPageFile = true;
    }
    
    return HasPageFile;
}

void COpenRawFileDialog::ReadHGRFile(const HFCPtr<HFCURL>& pi_rpURL, 
                                     uint32_t* po_ImageWidth, 
                                     uint32_t* po_ImageHeight)
{
    HAutoPtr<HFCBinStream> pFile(HFCBinStream::Instanciate(pi_rpURL, HFC_READ_ONLY));

    HFCIniFileBrowser InitFile(pFile);
    AString            Value;

    // Read the GoeRefSetting section
    if (!InitFile.FindTopic(GEOREFSETTING))
        throw HFCFileNotSupportedException(pi_rpURL->GetURL());

    // ImageWidth
    if (!InitFile.GetVariableValue(IMAGE_WIDTH, Value))
        throw HFCFileNotSupportedException(pi_rpURL->GetURL());

    if (!ConvertStringToUnsignedLong(Value, po_ImageWidth))
        throw HFCCorruptedFileException(pi_rpURL->GetURL());

    // ImageHeight
    if (!InitFile.GetVariableValue(IMAGE_HEIGHT, Value))
        throw HFCFileNotSupportedException(pi_rpURL->GetURL());

    if (!ConvertStringToUnsignedLong(Value, po_ImageHeight))
        throw HFCCorruptedFileException(pi_rpURL->GetURL());

}

//-----------------------------------------------------------------------------
// Private
// ConvertStringToUndignedLong
//-----------------------------------------------------------------------------
bool COpenRawFileDialog::ConvertStringToUnsignedLong(const AString& pi_rString, uint32_t* po_pLong) const
{
    HPRECONDITION(po_pLong != 0);

    char* pStopPtr;
    *po_pLong= strtol(pi_rString.c_str(), &pStopPtr, 10);

    return (pStopPtr - pi_rString.c_str() == pi_rString.length());
}
