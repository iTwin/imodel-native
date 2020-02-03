//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTCertifyExport.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// HUTCertifyExport.cpp : implementation file
//-------------------------------------------------------------------

#include "stdafx.h"

#include "HUTCertifyExport.h"

#ifdef __HMR_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-------------------------------------------------------------------
// HUTCertifyExport constructor
//-------------------------------------------------------------------

HUTCertifyExport::HUTCertifyExport(CWnd* pParent /*=NULL*/)
    : CDialog(HUTCertifyExport::IDD, pParent)
    {
    //{{AFX_DATA_INIT(HUTCertifyExport)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    m_IsGeoRefCompare       = false;
    m_IsImageCompare        = false;
    m_IsUsingThreshold      = false;
    m_IsSaveImageCompResult = false;

    m_ThresholdMax   = 255;
    m_ThresholdMin   = 0;
    m_ErrorTolerence = 0;

    m_ImageResultPath    = _T("");
    m_ImageCompareMethod = _T("");
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTCertifyExport::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(HUTCertifyExport)
    DDX_Control(pDX, IDC_STATIC_THRESHOLD_MIN, m_StaticThresholdMin);
    DDX_Control(pDX, IDC_STATIC_THRESHOLD_MAX, m_StaticThresholdMax);
    DDX_Control(pDX, IDC_STATIC_ERROR_TOLERENCE, m_StaticErrorTolerence);
    DDX_Control(pDX, IDC_BT_BROWSE, m_BtBrowse);
    DDX_Control(pDX, IDC_SPINIDC_THRESHLOD_MIN, m_SpinThresholdMin);
    DDX_Control(pDX, IDC_SPINIDC_THRESHLOD_MAX, m_SpinThresholdMax);
    DDX_Control(pDX, IDC_SPIN_ERROR_TOLERENCE,  m_SpinErrorTolerence);
    DDX_Control(pDX, IDC_EDIT_THRESHLOD_MIN, m_EditThresholdMin);
    DDX_Control(pDX, IDC_EDIT_THRESHLOD_MAX, m_EditThresholdMax);
    DDX_Control(pDX, IDC_EDIT_IMAGE_RESULT_PATH, m_EditImageResultPath);
    DDX_Control(pDX, IDC_EDIT_ERROR_TOLERENCE, m_EditErrorTolerence);
    DDX_Control(pDX, IDC_COMBO_IMAGE_COMP_METHOD, m_CmbImageCompareMethod);
    DDX_Control(pDX, IDC_CHK_TRESHOLD, m_ChkThreshold);
    DDX_Control(pDX, IDC_CHK_SAVE_IMAGE_RESULT, m_ChkSaveImageResult);
    DDX_Control(pDX, IDC_CHK_IMAGE_COMP, m_ChkImageCompare);
    DDX_Control(pDX, IDC_CHK_GEOREF_COMP, m_ChkGeoRefCompare);
    //}}AFX_DATA_MAP
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(HUTCertifyExport, CDialog)
    //{{AFX_MSG_MAP(HUTCertifyExport)
    ON_BN_CLICKED(IDC_CHK_GEOREF_COMP, OnChkGeorefComp)
    ON_BN_CLICKED(IDC_CHK_IMAGE_COMP, OnChkImageComp)
    ON_BN_CLICKED(IDC_CHK_TRESHOLD, OnChkTreshold)
    ON_BN_CLICKED(IDC_CHK_SAVE_IMAGE_RESULT, OnChkSaveImageResult)
    ON_BN_CLICKED(IDC_BT_BROWSE, OnBtBrowse)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------
// HUTCertifyExport message handlers
//-------------------------------------------------------------------

BOOL HUTCertifyExport::OnInitDialog()
    {
    CDialog::OnInitDialog();

    m_CmbImageCompareMethod.SetCurSel(0);

    m_SpinThresholdMin.SetRange  (0, 255);
    m_SpinThresholdMax.SetRange  (0, 255);

    m_SpinErrorTolerence.SetRange(0, 100);

    m_EditThresholdMin.SetWindowText  (L"0");
    m_EditThresholdMax.SetWindowText  (L"255");
    m_EditErrorTolerence.SetWindowText(L"0");

    // Not yet functionnal so, disable the CheckBox until finished...
    //m_ChkImageCompare.EnableWindow(false);

    EnableImageCompSection(false);

    return true;  // return true unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return false
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BOOL HUTCertifyExport::IsCertifyExportEnabled()
    {
    BOOL CertifyExportEnabled = false;

    // Check other condition like image check here...
    if (m_IsGeoRefCompare || m_IsImageCompare)
        CertifyExportEnabled = true;

    return CertifyExportEnabled;

    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTCertifyExport::OnChkGeorefComp()
    {
    if (m_ChkGeoRefCompare.GetCheck())
        m_IsGeoRefCompare = true;
    else
        m_IsGeoRefCompare = false;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTCertifyExport::OnChkImageComp()
    {
    if (m_ChkImageCompare.GetCheck())
        m_IsImageCompare = true;
    else
        m_IsImageCompare = false;

    EnableImageCompSection(m_IsImageCompare);
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTCertifyExport::OnChkTreshold()
    {
    if (m_ChkThreshold.GetCheck())
        m_IsUsingThreshold = true;
    else
        m_IsUsingThreshold = false;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTCertifyExport::OnChkSaveImageResult()
    {
    if (m_ChkSaveImageResult.GetCheck())
        {
        m_IsSaveImageCompResult = true;
        OnBtBrowse();
        }
    else
        m_IsSaveImageCompResult = false;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTCertifyExport::EnableImageCompSection(BOOL pi_EnableSection)
    {
    m_SpinThresholdMin.EnableWindow(pi_EnableSection);
    m_SpinThresholdMax.EnableWindow(pi_EnableSection);
    m_SpinErrorTolerence.EnableWindow(pi_EnableSection);

    m_ChkThreshold.EnableWindow(pi_EnableSection);
    m_ChkSaveImageResult.EnableWindow(pi_EnableSection);

    m_EditThresholdMin.EnableWindow(pi_EnableSection);
    m_EditThresholdMax.EnableWindow(pi_EnableSection);
    m_EditImageResultPath.EnableWindow(pi_EnableSection);
    m_EditErrorTolerence.EnableWindow(pi_EnableSection);

    m_CmbImageCompareMethod.EnableWindow(pi_EnableSection);
    m_BtBrowse.EnableWindow(pi_EnableSection);

    m_StaticThresholdMin.EnableWindow(pi_EnableSection);
    m_StaticThresholdMax.EnableWindow(pi_EnableSection);
    m_StaticErrorTolerence.EnableWindow(pi_EnableSection);
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BOOL HUTCertifyExport::IsGeoReferenceCompare() const
    {
    return m_IsGeoRefCompare;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BOOL HUTCertifyExport::IsImageCompare() const
    {
    return m_IsImageCompare;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

long HUTCertifyExport::GetErrorTolerence() const
    {
    return m_ErrorTolerence;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

long HUTCertifyExport::GetThresholdMin() const
    {
    return m_ThresholdMin;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

long HUTCertifyExport::GetThresholdMax() const
    {
    return m_ThresholdMax;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BOOL HUTCertifyExport::IsSaveImageCompResult() const
    {
    return m_IsSaveImageCompResult;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

CString HUTCertifyExport::GetImageResultPath() const
    {
    return m_ImageResultPath;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BOOL HUTCertifyExport::IsUsingThreshold() const
    {
    return m_IsUsingThreshold;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTCertifyExport::OnOK()
    {
    CString TempBuffer;

    // Update value only if using image compare, else keep default value
    if (m_IsImageCompare)
        {
        if (m_IsUsingThreshold)
            {
            m_EditThresholdMin.GetWindowText(TempBuffer);
            m_ThresholdMin = BeStringUtilities::Wtoi((LPCWSTR)TempBuffer);

            m_EditThresholdMax.GetWindowText(TempBuffer);
            m_ThresholdMax = BeStringUtilities::Wtoi((LPCWSTR)TempBuffer);
            }

        // Mandatory field, so always update the field when using ImageCompare
        m_EditErrorTolerence.GetWindowText(TempBuffer);
        m_ErrorTolerence = BeStringUtilities::Wtoi((LPCWSTR)TempBuffer);

        // Mandatory field, so always update the field when using ImageCompare
        m_CmbImageCompareMethod.GetWindowText(m_ImageCompareMethod);
        }
    CDialog::OnOK();
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

CString HUTCertifyExport::GetImageCompareMethod() const
    {
    return m_ImageCompareMethod;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTCertifyExport::OnBtBrowse()
    {
    m_ImageResultPath = DirectoryBrowser("Choose destination folder");
    m_EditImageResultPath.SetWindowText(m_ImageResultPath);
    }

//-------------------------------------------------------------------
// DirectoryBrowser
//-------------------------------------------------------------------

CString HUTCertifyExport::DirectoryBrowser(CString pi_Title)
    {
    CString PathDirectory;

    // Setup the Browse info structure
    WChar sDisplayName[255];

    BROWSEINFO BrowseInfoStruct;
    BrowseInfoStruct.hwndOwner  = m_hWnd;
    BrowseInfoStruct.pidlRoot   = NULL;
    BrowseInfoStruct.pszDisplayName = sDisplayName;
    BrowseInfoStruct.lpszTitle  =  LPCWSTR(pi_Title);
    BrowseInfoStruct.ulFlags    = BIF_RETURNONLYFSDIRS;
    BrowseInfoStruct.lpfn       = NULL;
    BrowseInfoStruct.lParam     = NULL;

    // Browse for the folder
    LPITEMIDLIST pItemIDList;
    if (pItemIDList = SHBrowseForFolder(&BrowseInfoStruct))
        {
        // get the folder and set the new string
        WChar sPath[260];

        if (SHGetPathFromIDList(pItemIDList, sPath))
            PathDirectory = sPath;

        // Avoid memory leaks by deleting the PIDL
        // using the shells task allocator
        IMalloc* pMalloc;
        if (SHGetMalloc(&pMalloc) != NOERROR)
            {
            TRACE(L"Failed to get pointer to shells task allocator");
            return "Error in ConfigSection::DirectoryBrowser";
            }
        pMalloc->Free(pItemIDList);
        if (pMalloc)
            pMalloc->Release();
        }
    return PathDirectory;
    }
