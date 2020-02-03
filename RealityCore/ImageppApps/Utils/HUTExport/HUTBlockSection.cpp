//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTBlockSection.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// HUTBlockSection.cpp : implementation file
//
#include "stdafx.h"

#include "HUTBlockSection.h"
#include "HUTDialogContainer.h"
#include <ImagePP/all/h/HUTClassIDDescriptor.h>

#ifdef __HMR_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// HUTBlockSection dialog


HUTBlockSection::HUTBlockSection(HRFImportExport* pi_pImportExport, CWnd* pParent /*=NULL*/)
    : HUTSectionDialog(pi_pImportExport, pParent)
    {
    //{{AFX_DATA_INIT(HUTBlockSection)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    }

void HUTBlockSection::DoDataExchange(CDataExchange* pDX)
    {
    HUTSectionDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(HUTBlockSection)
    DDX_Control(pDX, IDC_COMBO_BLOCK_TYPE, m_comboBlockType);
    DDX_Control(pDX, IDC_SPIN_WIDTH,  m_SpinWidth);
    DDX_Control(pDX, IDC_SPIN_HEIGHT, m_SpinHeight);
    DDX_Control(pDX, IDC_EDIT_WIDTH,  m_EditWidth);
    DDX_Control(pDX, IDC_EDIT_HEIGHT, m_EditHeight);
    //}}AFX_DATA_MAP
    }

BEGIN_MESSAGE_MAP(HUTBlockSection, HUTSectionDialog)
    //{{AFX_MSG_MAP(HUTBlockSection)
    ON_CBN_SELCHANGE(IDC_COMBO_BLOCK_TYPE, OnSelchangeComboBlockType)
    ON_EN_CHANGE(IDC_EDIT_HEIGHT, OnChangeEditHeight)
    ON_EN_CHANGE(IDC_EDIT_WIDTH, OnChangeEditWidth)
    ON_EN_KILLFOCUS(IDC_EDIT_HEIGHT, OnKillFocusEditHeight)
    ON_EN_KILLFOCUS(IDC_EDIT_WIDTH, OnKillFocusEditWidth)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// HUTBlockSection message handlers

/////////////////////////////////////////////////////////////////////////////

void HUTBlockSection::RefreshControls()
    {
    if (!m_HiddenSection && ((GetImportExport()->CountBlockType() > 1) || (GetImportExport()->CountBlockWidthIncrementStep() > 1)
                             || (GetImportExport()->CountBlockHeightIncrementStep() > 1)))
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_BLOCK_SECTION, true);
        SetShowState(true);

        m_comboBlockType.ResetContent();

        // Fill with the pixel type
        for (uint32_t Index = 0; Index < GetImportExport()->CountBlockType(); Index++)
            {
            WString BlockTypeW(HUTClassIDDescriptor::GetInstance()->GetClassLabelBlockType(GetImportExport()->GetBlockType(Index)).c_str(),
                               BentleyCharEncoding::Utf8);
            m_comboBlockType.AddString(BlockTypeW.c_str());
            }



        m_comboBlockType.SetCurSel(GetImportExport()->GetSelectedBlockTypeIndex());

        UDACCEL pWAccel;
        UDACCEL pHAccel;

        pWAccel.nSec =  1;
        if (GetImportExport()->GetBlockWidthIncrementStep() != 0)
            pWAccel.nInc = GetImportExport()->GetBlockWidthIncrementStep();       // Step Increment
        else
            pWAccel.nInc = 1;

        pHAccel.nSec =  1;
        if (GetImportExport()->GetBlockHeightIncrementStep() != 0)
            pHAccel.nInc = GetImportExport()->GetBlockHeightIncrementStep();       // Step Increment
        else
            pHAccel.nInc = 1;

        m_SpinWidth.SetAccel( 1, &pWAccel);
        m_SpinWidth.SetRange32(GetImportExport()->GetMinimumBlockWidth(), MIN(GetImportExport()->GetMinimumBlockWidth() +
                             (GetImportExport()->CountBlockWidthIncrementStep() *
                              GetImportExport()->GetBlockWidthIncrementStep()),
                              INT_MAX));
        m_SpinWidth.SetPos(GetImportExport()->GetBlockWidth());

        m_SpinHeight.SetAccel( 1, &pHAccel);
        m_SpinHeight.SetRange32(GetImportExport()->GetMinimumBlockHeight(),MIN(GetImportExport()->GetMinimumBlockHeight() +
                              (GetImportExport()->CountBlockHeightIncrementStep() *
                               GetImportExport()->GetBlockHeightIncrementStep()),
                              INT_MAX));
        m_SpinHeight.SetPos  ( GetImportExport()->GetBlockHeight());

        if (GetImportExport()->CountBlockWidthIncrementStep() > 1)
            {
            m_SpinWidth.EnableWindow(true);
            m_EditWidth.EnableWindow(true);
            }
        else
            {
            m_SpinWidth.EnableWindow(false);
            m_EditWidth.EnableWindow(false);
            }

        if (GetImportExport()->CountBlockHeightIncrementStep() > 1)
            {
            m_SpinHeight.EnableWindow(true);
            m_EditHeight.EnableWindow(true);
            }
        else
            {
            m_SpinHeight.EnableWindow(false);
            m_EditHeight.EnableWindow(false);
            }
        }
    else
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_BLOCK_SECTION, false);
        SetShowState(false);
        }
    }

/////////////////////////////////////////////////////////////////////////////

void HUTBlockSection::RefreshConfig()
    {

    }


/////////////////////////////////////////////////////////////////////////////

void HUTBlockSection::OnSelchangeComboBlockType()
    {
    GetImportExport()->SelectBlockTypeByIndex(m_comboBlockType.GetCurSel());
    RefreshDisplaySection(HUTEXPORT_BLOCK_SECTION);
    }

/////////////////////////////////////////////////////////////////////////////

BOOL HUTBlockSection::OnInitDialog()
    {
    HUTSectionDialog::OnInitDialog();

    m_SpinHeight.SetBuddy(&m_EditHeight);
    m_SpinHeight.SetBase (10);

    m_SpinWidth.SetBuddy(&m_EditWidth);
    m_SpinWidth.SetBase (10);

    RefreshControls();

    return true;
    }


/////////////////////////////////////////////////////////////////////////////

void HUTBlockSection::OnChangeEditHeight()
    {
//    GetImportExport()->SetBlockHeight(m_SpinHeight.GetPos());
//    m_SpinWidth.SetPos(GetImportExport()->GetBlockWidth());
    }

/////////////////////////////////////////////////////////////////////////////

void HUTBlockSection::OnChangeEditWidth()
    {
//    GetImportExport()->SetBlockWidth(m_SpinWidth.GetPos());
//    m_SpinHeight.SetPos(GetImportExport()->GetBlockHeight());
    }

/////////////////////////////////////////////////////////////////////////////

void HUTBlockSection::OnKillFocusEditWidth()
    {
    CString EditPostion;
    INT Position;

    m_EditWidth.GetWindowText(EditPostion);
    Position = BeStringUtilities::Wtoi(EditPostion);

    GetImportExport()->SetBlockWidth(Position);
    m_SpinWidth.SetPos(GetImportExport()->GetBlockWidth());
    RefreshControlsTo(HUTEXPORT_BLOCK_SECTION);
    }

/////////////////////////////////////////////////////////////////////////////

void HUTBlockSection::OnKillFocusEditHeight()
    {
    CString EditPostion;
    INT Position;

    m_EditHeight.GetWindowText(EditPostion);
    Position = BeStringUtilities::Wtoi(EditPostion);

    GetImportExport()->SetBlockHeight(Position);
    m_SpinHeight.SetPos(GetImportExport()->GetBlockHeight());
    RefreshControlsTo(HUTEXPORT_BLOCK_SECTION);
    }

