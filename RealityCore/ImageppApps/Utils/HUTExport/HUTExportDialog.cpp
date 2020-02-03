//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTExportDialog.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// HUTExportDialog.cpp : implementation file
//
#include "stdafx.h"

#include "HUTExportDialog.h"
#include <ImagePP/all/h/HFCURLFile.h>
#include <ImagePP/all/h/HRFRasterFileFactory.h>
#include "HUTClassID.h"

#ifdef __HMR_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------
// HUTExportDialog dialog
//---------------------------------------------------------------------

HUTExportDialog::HUTExportDialog(HRFImportExport* pi_pImportExport,
                                 CWnd*            pi_pParent)
    : HUTDialogContainer(HUTExportDialog::IDD, pi_pParent)
    {
    //{{AFX_DATA_INIT(CExportAsDlg)
    m_UseBestOption = false;
    //}}AFX_DATA_INIT

    m_pImportExport = pi_pImportExport;

    m_pPixelType               = new HUTPixelType(GetImportExport());
    m_pSubresPixelType         = new HUTSubresPixelType(GetImportExport());
    m_pImageSize               = new HUTImageSize(GetImportExport());
    m_pDestinationSection      = new HUTDestination(GetImportExport());
    m_pSmallCompressionSection = new HUTSmallCompressionSection(GetImportExport());
    m_pCompressionSection      = new HUTCompressionSection(GetImportExport());
    m_pEncodingSection         = new HUTEncodingSection(GetImportExport());
    m_pBlockSection            = new HUTBlockSection(GetImportExport());
    m_pGeoReferenceSection     = new HUTGeoReferenceSection(GetImportExport());
    }


//---------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------
HUTExportDialog::~HUTExportDialog()
    {
    delete m_pPixelType;
    delete m_pImageSize;
    delete m_pDestinationSection;
    delete m_pSmallCompressionSection;
    delete m_pCompressionSection;
    delete m_pEncodingSection;
    delete m_pBlockSection;
    delete m_pGeoReferenceSection;
    delete m_pSubresPixelType;
    }

//---------------------------------------------------------------------
// DoDataExchange
//---------------------------------------------------------------------
void HUTExportDialog::DoDataExchange(CDataExchange* pDX)
    {
    HUTDialogContainer::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(HUTExportDialog)
    DDX_Check(pDX, IDC_EXPORTAS_BESTOPTION, m_UseBestOption);
    DDX_Control(pDX, IDC_EXPORTAS_FOLDER, m_Folder);
    //}}AFX_DATA_MAP
    }

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(HUTExportDialog, HUTDialogContainer)
    //{{AFX_MSG_MAP(HUTExportDialog)
    ON_BN_CLICKED(IDC_EXPORTAS_BESTOPTION, OnExportasBestOption)
    ON_BN_CLICKED(IDC_EXPORTAS_EXPORT, OnExportasExport)
    ON_BN_CLICKED(IDC_EXPORTAS_SELECT, OnExportasSelect)
    ON_WM_DESTROY()
    ON_CBN_SELCHANGE(IDC_EXPORTAS_FORMAT, OnSelchangeExportasFormat)
    ON_EN_CHANGE(IDC_EXPORTAS_FOLDER, OnChangeExportasFolder)
    ON_WM_CLOSE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()



//---------------------------------------------------------------------
// On Export As Best Option
//---------------------------------------------------------------------
void HUTExportDialog::OnExportasBestOption()
    {
    UpdateData(true);

    // If we want to use the best option, hide the section.
    // Otherwise, show the section
    if (m_UseBestOption)
        {
        HideDisplaySection();
        m_pImportExport->BestMatchSelectedValues();
        }
    else
        RefreshDisplaySection(0);
    }

//---------------------------------------------------------------------
// On Export As Export
//---------------------------------------------------------------------
void HUTExportDialog::OnExportasExport()
    {
    UpdateData(true);

    m_Folder.GetWindowText(m_SelectedFolder);

    if (!VerifyFile(m_SelectedFolder))
        AfxMessageBox(L"Invalid output directory.");
    else
        EndDialog(IDOK);
    }

//---------------------------------------------------------------------
// On Export As Select
//---------------------------------------------------------------------
void HUTExportDialog::OnExportasSelect()
    {
    // Get the dialog
    CString Filters;
    CString ImageExt;

    // Setup the filters for the file dialog
    // Add all known formats filters
    for (uint32_t Index=0; Index < m_pImportExport->CountExportFileFormat(); Index++)
        {
        if (((HRFRasterFileCreator*)m_pImportExport->GetExportFileFormat(Index))->SupportsScheme(HFCURLFile::s_SchemeName()) &&
            ((HRFRasterFileCreator*)m_pImportExport->GetExportFileFormat(Index))->GetExtensions().c_str() != 0)
            {
            // add the current images to the image extensions
            ImageExt += ((HRFRasterFileCreator*)m_pImportExport->GetExportFileFormat(Index))->GetExtensions().c_str();
            if (Index < m_pImportExport->CountExportFileFormat())
                ImageExt += ";";
            }
        }

    // add the "All Image Files" filter
    Filters += "All Image Files (";
    Filters += ImageExt;
    Filters += ")|";
    Filters += ImageExt;
    Filters += "||";

    // Setup the file Save as dialog
    CFileDialog SaveAsDialog(false, NULL, NULL,
                             OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST,
                             Filters,
                             this);

    SaveAsDialog.m_ofn.nFilterIndex = 1;

    if (SaveAsDialog.DoModal() == IDOK)
        {
        m_Folder.SetWindowText(SaveAsDialog.GetPathName());
        }
    }

// ---------------------------------------------------------------------
// On selection change export as format.
// ---------------------------------------------------------------------
void HUTExportDialog::OnSelchangeExportasFormat()
    {
    CComboBox* pFormatBox = (CComboBox*)GetDlgItem(IDC_EXPORTAS_FORMAT);

    m_pImportExport->SelectExportFileFormatByIndex(pFormatBox->GetCurSel());

    if (!m_UseBestOption)
        RefreshDisplaySection(0);
    }

// ---------------------------------------------------------------------
// On Change Export as Folder
// ---------------------------------------------------------------------
void HUTExportDialog::OnChangeExportasFolder()
    {
    UpdateData(true);
    }

//---------------------------------------------------------------------
// On Init Dialog
//---------------------------------------------------------------------
BOOL HUTExportDialog::OnInitDialog()
    {
    HUTDialogContainer::OnInitDialog();

    // Centered with it's owner
    CenterWindow(GetOwner());

    GetClientRect(m_HUTExportDialogClientRect);
    GetWindowRect(m_HUTExportDialogWindowRect);

    for (int SectionIndex = 0; SectionIndex < HUTEXPORT_MAX_SECTION; SectionIndex++)
        m_openSection[SectionIndex] = 1;

    CComboBox* pFormatBox = (CComboBox*)GetDlgItem(IDC_EXPORTAS_FORMAT);

    WString Filters;
    for (uint32_t Index=0; Index < m_pImportExport->CountExportFileFormat(); Index++)
        {

        Filters.AssignUtf8(((HRFRasterFileCreator*)m_pImportExport->GetExportFileFormat(Index))->GetLabel().c_str());
        Filters += L" Files (";
        Filters.AppendUtf8(((HRFRasterFileCreator*)m_pImportExport->GetExportFileFormat(Index))->GetExtensions().c_str());
        Filters += L")";

        pFormatBox->AddString((LPCWSTR)Filters.c_str());
        }

    m_pImportExport->SelectExportFileFormatByIndex(0);

    pFormatBox->SetCurSel(0);

    CreateSections();

    // show all sections
    RedrawAllSection(CPoint(m_HUTExportDialogClientRect.left, m_HUTExportDialogClientRect.bottom),
                     m_HUTExportDialogWindowRect,
                     m_HUTExportDialogWindowRect.Height() - m_HUTExportDialogClientRect.Height());

    return true;  // return true unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return false
    }

// ---------------------------------------------------------------------
// Create Section
// ---------------------------------------------------------------------
void HUTExportDialog::CreateSections()
    {
    AddSection(m_pDestinationSection, false);
    AddSection(m_pImageSize, false);
    AddSection(m_pPixelType, false);
    AddSection(m_pSubresPixelType, false);
    AddSection(m_pSmallCompressionSection, false);
    AddSection(m_pCompressionSection, false);
    AddSection(m_pEncodingSection, false);
    AddSection(m_pBlockSection, false);
    AddSection(m_pGeoReferenceSection, false);

    // Section Construction
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

// ---------------------------------------------------------------------
// Get Import Export
// ---------------------------------------------------------------------
HRFImportExport* HUTExportDialog::GetImportExport() const
    {
    return m_pImportExport;
    }

// ---------------------------------------------------------------------
// Refresh Display Section
// ---------------------------------------------------------------------
void HUTExportDialog::RefreshDisplaySection(UINT pi_SectionID)
    {
    HUTSectionDialog* pDialog = 0;

    pDialog = m_SectionArray.GetAt(pi_SectionID);
    if (pDialog->IsShowSection())
        m_openSection[pi_SectionID] = 0;
    else
        m_openSection[pi_SectionID] = 1;

    RedrawAllSection(CPoint(m_HUTExportDialogClientRect.left, m_HUTExportDialogClientRect.bottom),
                     m_HUTExportDialogWindowRect,
                     m_HUTExportDialogWindowRect.Height() - m_HUTExportDialogClientRect.Height());
    }

// ---------------------------------------------------------------------
// Hide Display Section
// ---------------------------------------------------------------------
void HUTExportDialog::HideDisplaySection(void)
    {
    CRect TmpRect;
    GetWindowRect(TmpRect);

    TmpRect.right = TmpRect.left + m_HUTExportDialogWindowRect.Width();
    TmpRect.bottom = TmpRect.top + m_HUTExportDialogWindowRect.Height();

    MoveWindow(&TmpRect);
    }

// ---------------------------------------------------------------------
// Refresh All Display Section
// ---------------------------------------------------------------------
bool HUTExportDialog::RefreshAllDisplaySection()
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
    RedrawAllSection(CPoint(m_HUTExportDialogClientRect.left, m_HUTExportDialogClientRect.bottom),
                     m_HUTExportDialogWindowRect,
                     m_HUTExportDialogWindowRect.Height() - m_HUTExportDialogClientRect.Height());
    return Status;
    }

// ---------------------------------------------------------------------
// Verify File
// ---------------------------------------------------------------------
bool HUTExportDialog::VerifyFile (CString pi_FileName)
    {
    WIN32_FIND_DATA    FindData;
    HANDLE          handle;
    bool           ret = true;

    // verify if the destination file already exist
    if ((handle = FindFirstFile(pi_FileName, &FindData)) != INVALID_HANDLE_VALUE)
        {
        /*
                // if file is read only
                if (GetFileAttributes(pi_FileName) & FILE_ATTRIBUTE_READONLY)
                {
                    CString  Msg;
                    CString  Title;

                    Msg.Format(IDS_READONLYERROR, pi_FileName);
                    Title.Format(IDS_MSGBOX_ERROR);
                    MessageBox (Msg, Title, MB_OK | MB_ICONSTOP);

                    ret = false;
                }
                else
                {
                    CString  Msg;
                    CString  Title;

                    Msg.Format(IDS_FILEEXIST, pi_FileName);
                    Title.Format(IDS_PROP_WARNING);

                    if (MessageBox(Msg, Title, MB_YESNO| MB_ICONSTOP) == IDNO)
                    {
                        ret = false;
                    }
                }
        */
        FindClose(handle);
        }

    return (ret);
    }

// -------------------------------------------------------------------------------
// GetExportFileName
// -------------------------------------------------------------------------------
WString HUTExportDialog::GetExportFileName()
    {
    return (LPCWSTR)m_SelectedFolder;
    }

// -------------------------------------------------------------------------------
// SetOpenSectionShowState
// -------------------------------------------------------------------------------
void HUTExportDialog::SetOpenSectionShowState(UINT pi_SectionIndex, bool pi_Show)
    {
    if (pi_Show)
        m_openSection[pi_SectionIndex] = 1;
    else
        m_openSection[pi_SectionIndex] = 0;
    }

