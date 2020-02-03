//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTConvertToDlg.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// HUTConvertToDlg.cpp : implementation file
//-------------------------------------------------------------------

#include "stdafx.h"

#include "HUTConvertToDlg.h"
#include "HUTCertifyExport.h"
#include <ImagePP/all/h/HRFImportExport.h>

#ifdef __HMR_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-------------------------------------------------------------------
// HUTConvertToDlg constructor
//-------------------------------------------------------------------

HUTConvertToDlg::HUTConvertToDlg(HRFImportExport* pi_pImportExport, BOOL pi_MultiSelectionMode, CWnd* pParent )
    : HUTDialogContainer(HUTConvertToDlg::IDD, pParent)
    {
    //{{AFX_DATA_INIT(HUTConvertToDlg)
    //}}AFX_DATA_INIT

    m_pImportExport = pi_pImportExport;

    m_MultiSelectionMode = pi_MultiSelectionMode;

    m_pSmallCompressionSection = 0;
    m_pCompressionSection      = 0;
    m_pBlockSection            = 0;
    m_pEncodingSection         = 0;
    m_pImageSize               = 0;
    m_pPixelType               = 0;
    m_pSubresPixelType         = 0;
    m_pGeoReferenceSection     = 0;
    m_pDestinationSection      = 0;

    m_ReplaceExtension         = true;
    m_ExportWithAllOption      = false;
    m_ResumeInFilename         = false;
    m_ExportToSourceFormat     = false;
    m_ChkBestOption            = false;
    m_ChkExcludeJPEG           = false;

    m_IsGeoRefCompare = false;
    m_IsImageCompare  = false;

    m_IsSaveImageCompResult = false;
    m_IsUsingThreshold      = false;

    m_ThresholdMax   = 255;
    m_ThresholdMin   = 0;
    m_ErrorTolerence = 0;

    m_ImageResultPath    = _T("");
    m_ImageCompareMethod = _T("");
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

HUTConvertToDlg::~HUTConvertToDlg()
    {
    /*
    for (int i = m_SectionArray.GetSize() - 1; i >= 0; i--)
        RemoveSection(i);
    */

    delete m_pSmallCompressionSection;
    delete m_pCompressionSection;
    delete m_pBlockSection;
    delete m_pEncodingSection;
    delete m_pImageSize;
    delete m_pPixelType;
    delete m_pSubresPixelType;
    delete m_pGeoReferenceSection;
    delete m_pDestinationSection;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTConvertToDlg::DoDataExchange(CDataExchange* pDX)
    {
    HUTDialogContainer::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(HUTConvertToDlg)
    DDX_Control(pDX, IDC_RD_RETAIN_EXT, m_RDRetainExt);
    DDX_Control(pDX, IDC_RD_REPLACE_EXTENSION, m_RDReplaceExt);
    DDX_Control(pDX, IDC_CHK_EXPORT_INTEGRITY, m_ChkExportIntegrity);
    DDX_Control(pDX, IDC_CHK_RESUME_IN_FILENAME, m_CbResumeInFileName);
    DDX_Control(pDX, IDC_CHK_ALL_POSSIBILITY, m_CbAllPossibility);
    DDX_Control(pDX, IDC_CHK_BEST_OPTION, m_BtUseBestOption);
    DDX_Control(pDX, IDC_CHK_EXCLUDE_JPEG, m_BtExcludeJPEG);
    DDX_Control(pDX, IDC_EXPORTAS_SOURCE, m_BtExportToSourceFormat);
    DDX_Control(pDX, IDC_CHK_KEEP_SRCPATH, m_CbKeepSrcPath);
    DDX_Control(pDX, IDC_CMB_FILE_FORMAT, m_CmbFileFormat);
    //}}AFX_DATA_MAP
    }
//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(HUTConvertToDlg, HUTDialogContainer)
    //{{AFX_MSG_MAP(HUTConvertToDlg)
    ON_BN_CLICKED(IDC_RD_RETAIN_EXT, OnRdRetainExt)
    ON_BN_CLICKED(IDC_RD_REPLACE_EXTENSION, OnRdReplaceExtension)
    ON_BN_CLICKED(IDC_CHK_BEST_OPTION, OnChkBestOption)
    ON_BN_CLICKED(IDC_CHK_EXCLUDE_JPEG, OnChkExcludeJPEG)
    ON_BN_CLICKED(IDC_CHK_KEEP_SRCPATH, OnChkKeepSrcpath)
    ON_BN_CLICKED(IDC_BT_CANCEL, OnBtCancel)
    ON_BN_CLICKED(IDC_BT_OK, OnBtOk)
    ON_BN_CLICKED(IDC_EXPORTAS_SOURCE, OnExportToSourceFormat)
    ON_BN_CLICKED(IDC_CHK_ALL_POSSIBILITY, OnChkAllPossibility)
    ON_BN_CLICKED(IDC_CHK_RESUME_IN_FILENAME, OnChkResumeInFilename)
    ON_BN_CLICKED(IDC_CHK_EXPORT_INTEGRITY, OnChkExportIntegrity)
    ON_CBN_SELCHANGE(IDC_CMB_FILE_FORMAT, OnSelchangeCmbFileFormat)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------
// HUTConvertToDlg message handlers
//-------------------------------------------------------------------

void HUTConvertToDlg::OnChkBestOption()
    {
    if (m_BtUseBestOption.GetCheck())
        {
        m_ChkBestOption = true;
        m_CbAllPossibility.EnableWindow(false);
        m_BtExcludeJPEG.EnableWindow(true);
        HideAllSection(true);
        }
    else
        {
        m_ChkBestOption = false;
        m_CbAllPossibility.EnableWindow(true);
        m_BtExcludeJPEG.EnableWindow(false);
        HideAllSection(false);
        }

    RefreshAllDisplaySection();
    }
//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTConvertToDlg::OnChkExcludeJPEG()
    {
    if ((m_BtUseBestOption.GetCheck())&&(m_BtExcludeJPEG.GetCheck()))
        {
        m_ChkExcludeJPEG = true;
        m_BtUseBestOption.EnableWindow(true);
        m_CbAllPossibility.EnableWindow(false);
        HideAllSection(true);
        }
    else
        {
        m_ChkExcludeJPEG = false;
        m_CbAllPossibility.EnableWindow(true);
        HideAllSection(false);
        }

    RefreshAllDisplaySection();
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------
void HUTConvertToDlg::OnExportToSourceFormat()
    {
    if (m_BtExportToSourceFormat.GetCheck())
        {
        HideAllSection(true);
        m_ExportToSourceFormat = true;
        m_BtExcludeJPEG.EnableWindow(false);
        m_BtUseBestOption.EnableWindow(false);
        m_CbAllPossibility.EnableWindow(false);
        m_CmbFileFormat.EnableWindow(false);
        }
    else
        {
        if(!m_BtUseBestOption)
            {
            m_CbAllPossibility.EnableWindow(false);
            HideAllSection(false);
            }
        else
            {
            m_CbAllPossibility.EnableWindow(true);
            if(m_MultiSelectionMode)
                HideAllSection(false);
            else
                HideAllSection(true);
            RefreshAllDisplaySection();
            }

        m_ExportToSourceFormat = false;
        m_BtExcludeJPEG.EnableWindow(false);
        m_BtUseBestOption.EnableWindow(true);
        m_CbAllPossibility.EnableWindow(true);
        m_CmbFileFormat.EnableWindow(true);
        }

    RefreshAllDisplaySection();
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTConvertToDlg::OnChkKeepSrcpath()
    {
    if (m_CbKeepSrcPath.GetCheck())
        m_pDestinationSection->ForceVisibility(false);
    else
        m_pDestinationSection->ForceVisibility(true);

    RefreshAllDisplaySection();
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTConvertToDlg::OnBtCancel()
    {
    OnCancel();
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTConvertToDlg::OnBtOk()
    {
    OnOK();
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BOOL HUTConvertToDlg::OnInitDialog()
    {
    HUTDialogContainer::OnInitDialog();

    m_BtUseBestOption.SetCheck(0);
    m_CbKeepSrcPath.SetCheck(1);

    m_ChkExportIntegrity.SetWindowText(L"Dont Certify export integrity");
    m_ChkExportIntegrity.SetCheck(0);

    FillComboFileFormat();
    m_CmbFileFormat.SetCurSel(0);

    m_RDRetainExt.SetCheck(0);
    m_RDReplaceExt.SetCheck(1);
    OnRdReplaceExtension();

    GetClientRect(m_HUTExportDialogClientRect);
    GetWindowRect(m_HUTExportDialogWindowRect);

    for (int SectionIndex = 0; SectionIndex < HUTEXPORT_MAX_SECTION; SectionIndex++)
        m_openSection[SectionIndex] = 1;

    CreateSections();

    if (!m_MultiSelectionMode)
        OnChkBestOption();
    else
        {
        // Disable some unavailable section in
        // multiselection file mode.. like Image Size and Block section.
        m_openSection[HUTEXPORT_IMG_SIZE_SECTION] = 0;
        HideSection(HUTEXPORT_IMG_SIZE_SECTION);
        m_pImageSize->HiddenSection(true);

        m_openSection[HUTEXPORT_BLOCK_SECTION]    = 0;
        HideSection(HUTEXPORT_BLOCK_SECTION);
        m_pBlockSection->HiddenSection(true);

        m_BtExcludeJPEG.EnableWindow(false);

        RefreshAllDisplaySection();
        }

    return true;  // return true unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return false
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTConvertToDlg::FillComboFileFormat()
    {
    WString Filters;

    for (uint32_t Index=0; Index < m_pImportExport->CountExportFileFormat(); Index++)
        {
        Filters.AssignUtf8(((HRFRasterFileCreator*)m_pImportExport->GetExportFileFormat(Index))->GetLabel().c_str());
        Filters += L" Files (";
        Filters.AppendUtf8(((HRFRasterFileCreator*)m_pImportExport->GetExportFileFormat(Index))->GetExtensions().c_str());
        Filters += L")";

        m_CmbFileFormat.AddString(Filters.c_str());
        }
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTConvertToDlg::CreateSections()
    {
    // Section Construction
    m_pDestinationSection = new HUTDestination(m_pImportExport);
    AddSection(m_pDestinationSection, false);

    m_pImageSize = new HUTImageSize(m_pImportExport);
    AddSection(m_pImageSize, false);

    m_pPixelType = new HUTPixelType(m_pImportExport);
    AddSection(m_pPixelType, false);

    m_pSubresPixelType = new HUTSubresPixelType(m_pImportExport);
    AddSection(m_pSubresPixelType, false);

    m_pSmallCompressionSection = new HUTSmallCompressionSection(m_pImportExport);
    AddSection(m_pSmallCompressionSection, false);

    m_pCompressionSection = new HUTCompressionSection(m_pImportExport);
    AddSection(m_pCompressionSection, false);

    m_pEncodingSection = new HUTEncodingSection(m_pImportExport);
    AddSection(m_pEncodingSection, false);

    m_pBlockSection = new HUTBlockSection(m_pImportExport);
    AddSection(m_pBlockSection, false);

    m_pGeoReferenceSection = new HUTGeoReferenceSection(m_pImportExport);
    AddSection(m_pGeoReferenceSection, false);

    // Section Initialization
    m_pDestinationSection->Create(HUTDestination::IDD, this);
    m_pImageSize->Create(HUTImageSize::IDD, this);
    m_pPixelType->Create(HUTPixelType::IDD, this);
    m_pSubresPixelType->Create(HUTSubresPixelType::IDD, this);
    m_pSmallCompressionSection->Create(HUTSmallCompressionSection::IDD, this);
    m_pCompressionSection->Create(HUTCompressionSection::IDD, this);
    m_pEncodingSection->Create(HUTEncodingSection::IDD, this);
    m_pBlockSection->Create(HUTBlockSection::IDD, this);
    m_pGeoReferenceSection->Create(HUTGeoReferenceSection::IDD, this);
    }

//-------------------------------------------------------------------
// RefreshDisplaySection
//-------------------------------------------------------------------

void HUTConvertToDlg::RefreshDisplaySection(UINT pi_SectionID)
    {
    HUTSectionDialog* pDialog = 0;

    pDialog = m_SectionArray.GetAt(pi_SectionID);

    if (pDialog->IsShowSection())
        m_openSection[pi_SectionID] = 0;
    else
        m_openSection[pi_SectionID] = 1;

    RedrawAllSection(CPoint(m_HUTExportDialogClientRect.left,m_HUTExportDialogClientRect.BottomRight().y),
                     m_HUTExportDialogWindowRect,
                     m_HUTExportDialogWindowRect.Height() - m_HUTExportDialogClientRect.Height());
    }

//-------------------------------------------------------------------
// RefreshAllDisplaySection
//-------------------------------------------------------------------

bool HUTConvertToDlg::RefreshAllDisplaySection()
    {
    bool Status = false;

    // Get the pointer to the system menu...
    for (int SectionIndex = 0; SectionIndex < HUTEXPORT_MAX_SECTION; SectionIndex++)
        {
        if (m_openSection[SectionIndex] == 0)
            HideSection(SectionIndex);
        else
            ShowSection(SectionIndex);
        }

    RedrawAllSection(CPoint(0,m_HUTExportDialogClientRect.BottomRight().y),
                     m_HUTExportDialogWindowRect,
                     m_HUTExportDialogWindowRect.Height() - m_HUTExportDialogClientRect.Height());
    return Status;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

HRFImportExport* HUTConvertToDlg::GetImportExport() const
    {
    return m_pImportExport;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------
/*
void HUTConvertToDlg::SetImportExport(HRFImportExport* pi_pImportExport)
{
    m_pImportExport = pi_pImportExport;
}
*/

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTConvertToDlg::SetOpenSectionShowState(UINT pi_SectionIndex, bool pi_Show)
    {
    if (pi_Show)
        m_openSection[pi_SectionIndex] = 1;
    else
        m_openSection[pi_SectionIndex] = 0;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTConvertToDlg::OnSelchangeCmbFileFormat()
    {
    m_pImportExport->SelectExportFileFormatByIndex(m_CmbFileFormat.GetCurSel());
    RefreshDisplaySection(0);
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

Utf8String HUTConvertToDlg::GetNewSelectedPath()
    {
    Utf8String NewSelectedPath;

    if (m_pDestinationSection)
        NewSelectedPath =  m_pDestinationSection->GetNewSelectedPath();

    return NewSelectedPath;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTConvertToDlg::OnRdRetainExt()
    {
    m_ReplaceExtension = false;

    if (!m_RDRetainExt.GetCheck())
        m_RDReplaceExt.SetCheck(true);
    else
        m_RDReplaceExt.SetCheck(false);
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTConvertToDlg::OnRdReplaceExtension()
    {
    m_ReplaceExtension = true;

    if (!m_RDReplaceExt.GetCheck())
        m_RDRetainExt.SetCheck(true);
    else
        m_RDRetainExt.SetCheck(false);
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BOOL HUTConvertToDlg::IsReplaceExtension() const
    {
    return m_ReplaceExtension;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTConvertToDlg::HideAllSection(BOOL pi_HideAllSection)
    {
    HUTSectionDialog* pDialog = 0;

    for (int SectionIndex = 0; SectionIndex < HUTEXPORT_MAX_SECTION; SectionIndex++)
        {
        pDialog = m_SectionArray.GetAt(SectionIndex);
        pDialog->HiddenSection(pi_HideAllSection);
        }
    RefreshAllDisplaySection();
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTConvertToDlg::OnChkAllPossibility()
    {
    if (m_CbAllPossibility.GetCheck())
        {
        m_ExportWithAllOption = true;
        m_BtUseBestOption.EnableWindow(false);
        HideAllSection(true);
        }
    else
        {
        m_ExportWithAllOption = false;
        m_BtUseBestOption.EnableWindow(true);
        HideAllSection(false);
        }
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTConvertToDlg::OnChkResumeInFilename()
    {
    if (m_CbResumeInFileName.GetCheck())
        m_ResumeInFilename = true;
    else
        m_ResumeInFilename = false;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BOOL HUTConvertToDlg::IsResumeInFilename() const
    {
    return m_ResumeInFilename;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BOOL HUTConvertToDlg::IsExportWithAllOption() const
    {
    return m_ExportWithAllOption;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------
BOOL    HUTConvertToDlg::IsExportToSourceFormat()const
    {
    return m_ExportToSourceFormat;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------
BOOL    HUTConvertToDlg::IsChkBestOption()const
    {
    return m_ChkBestOption;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------
BOOL    HUTConvertToDlg::IsChkExcludeJPEG()const
    {
    return m_ChkExcludeJPEG;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTConvertToDlg::OnChkExportIntegrity()
    {
    if(m_ChkExportIntegrity.GetCheck())
        {
        HUTCertifyExport dlg;

        if (dlg.DoModal() == IDOK && dlg.IsCertifyExportEnabled())
            {
            m_ChkExportIntegrity.SetWindowText(L"Certify export integrity");
            m_IsGeoRefCompare = dlg.IsGeoReferenceCompare();
            m_IsImageCompare  = dlg.IsImageCompare();
            if (m_IsImageCompare)
                {
                // Mandatory field, always need be be valid value.
                m_ErrorTolerence        = dlg.GetErrorTolerence();
                m_ImageCompareMethod    = dlg.GetImageCompareMethod();
                m_IsSaveImageCompResult = dlg.IsSaveImageCompResult();
                m_IsUsingThreshold      = dlg.IsUsingThreshold();
                m_ImageResultPath       = dlg.GetImageResultPath();

                // If specific trehsold value has been request,
                // get the user selection, if not keep using the default value.
                if (m_IsUsingThreshold)
                    {
                    m_ThresholdMax = dlg.GetThresholdMax();
                    m_ThresholdMin = dlg.GetThresholdMin();
                    }
                }
            }
        else
            {
            m_ChkExportIntegrity.SetCheck(0);
            m_ChkExportIntegrity.SetWindowText(L"Dont Certify export integrity");
            }
        }
    else
        m_ChkExportIntegrity.SetWindowText(L"Dont Certify export integrity");
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BOOL HUTConvertToDlg::IsGeoReferenceCompare() const
    {
    return m_IsGeoRefCompare;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BOOL HUTConvertToDlg::IsImageCompare() const
    {
    return m_IsImageCompare;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BOOL HUTConvertToDlg::IsSaveImageCompResult() const
    {
    return m_IsSaveImageCompResult;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BOOL HUTConvertToDlg::IsUsingThreshold() const
    {
    return m_IsUsingThreshold;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

CString HUTConvertToDlg::GetImageResultPath() const
    {
    return m_ImageResultPath;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

long HUTConvertToDlg::GetThresholdMax() const
    {
    return m_ThresholdMax;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

long HUTConvertToDlg::GetThresholdMin() const
    {
    return m_ThresholdMin;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

long HUTConvertToDlg::GetErrorTolerence() const
    {
    return m_ErrorTolerence;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

CString HUTConvertToDlg::GetImageCompareMethod() const
    {
    return m_ImageCompareMethod;
    }

