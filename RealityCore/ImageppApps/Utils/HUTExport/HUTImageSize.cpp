//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTImageSize.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// HUTImageSize.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <ImagePP/all/h/HRFTypes.h>
#include "HUTImageSize.h"
#include "HUTDialogContainer.h"
#include <ImagePP/all/h/HFCGrid.h>

#ifdef __HMR_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


HUTImageSize::HUTImageSize(HRFImportExport* pi_pImportExport, CWnd* pParent /*=NULL*/)
    : HUTSectionDialog(pi_pImportExport, pParent)
    {
    m_IsDestroy = false;

    //{{AFX_DATA_INIT(HUTImageSize)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    }

HUTImageSize::~HUTImageSize()
    {
    m_IsDestroy = true;
    }

void HUTImageSize::DoDataExchange(CDataExchange* pDX)
    {
    HUTSectionDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(HUTImageSize)
    DDX_Control(pDX, IDC_CHK_RESAMPLE, m_CheckBoxResample);

    DDX_Control(pDX, IDC_EDIT_WIDTH, m_EditWidth);
    DDX_Control(pDX, IDC_EDIT_HEIGHT, m_EditHeight);
    DDX_Control(pDX, IDC_CHK_FIX_IMAGE_SIZE, m_CheckBoxFixImageSize);

    DDX_Control(pDX, IDC_EDIT_SCALE_X, m_EditScaleX);
    DDX_Control(pDX, IDC_EDIT_SCALE_Y, m_EditScaleY);
    DDX_Control(pDX, IDC_CHK_FIX_IMAGE_SCALE, m_CheckBoxFixScaling);

    DDX_Control(pDX, IDC_CHK_ASPECT_RATIO, m_CheckBoxAspectRatio);
    DDX_Control(pDX, IDC_RESET, m_ButtonReset);
    //}}AFX_DATA_MAP
    }


BEGIN_MESSAGE_MAP(HUTImageSize, HUTSectionDialog)
    //{{AFX_MSG_MAP(HUTImageSize)
    ON_BN_CLICKED(IDC_CHK_RESAMPLE, OnStatusChangeChkResample)

    ON_EN_KILLFOCUS(IDC_EDIT_HEIGHT, OnKillFocusEditHeight)
    ON_EN_KILLFOCUS(IDC_EDIT_WIDTH, OnKillFocusEditWidth)
    ON_BN_CLICKED(IDC_CHK_FIX_IMAGE_SIZE, OnStatusChangeChkFixImageSize)

    ON_EN_KILLFOCUS(IDC_EDIT_SCALE_X, OnKillFocusEditScaleX)
    ON_EN_KILLFOCUS(IDC_EDIT_SCALE_Y, OnKillFocusEditScaleY)
    ON_BN_CLICKED(IDC_CHK_FIX_IMAGE_SCALE, OnStatusChangeChkFixScaling)

    ON_BN_CLICKED(IDC_CHK_ASPECT_RATIO, OnStatusChangeChkAspectRatio)
    ON_BN_CLICKED(IDC_RESET, OnClickedButtonReset)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// HUTImageSize message handlers

/////////////////////////////////////////////////////////////////////////////

void HUTImageSize::RefreshControls()
    {
    if (m_HiddenSection)
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_IMG_SIZE_SECTION, false);
        SetShowState(false);
        }
    else
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_IMG_SIZE_SECTION, true);
        SetShowState(true);

        CString StringBuffer;

        // Resample
        m_CheckBoxResample.SetCheck(GetImportExport()->GetResample());
        m_CheckBoxResample.EnableWindow(!GetImportExport()->GetResampleIsForce());

        // Image size section
        StringBuffer.Format(L"%lu", GetImportExport()->GetImageWidth());
        m_EditWidth.SetWindowText(StringBuffer);

        StringBuffer.Format(L"%lu",  GetImportExport()->GetImageHeight());
        m_EditHeight.SetWindowText(StringBuffer);

        m_CheckBoxFixImageSize.SetCheck(GetImportExport()->ImageSizeIsLock());

        if(GetImportExport()->ImageSizeIsLock())
            {
            m_EditWidth.EnableWindow(false);
            m_EditHeight.EnableWindow(false);
            }
        else
            {
            m_EditWidth.EnableWindow(true);
            m_EditHeight.EnableWindow(true);
            }

        // Scaling section
        if(GetImportExport()->GetResample())
            {
            StringBuffer.Format(L"%.15f", GetImportExport()->GetScaleFactorX());
            m_EditScaleX.SetWindowText(StringBuffer);

            StringBuffer.Format(L"%.15f",  GetImportExport()->GetScaleFactorY());
            m_EditScaleY.SetWindowText(StringBuffer);

            m_CheckBoxFixScaling.SetCheck(GetImportExport()->ScaleFactorIsLock());
            m_CheckBoxFixScaling.EnableWindow(true);

            if(GetImportExport()->ScaleFactorIsLock())
                {
                m_EditScaleX.EnableWindow(false);
                m_EditScaleY.EnableWindow(false);
                }
            else
                {
                m_EditScaleX.EnableWindow(true);
                m_EditScaleY.EnableWindow(true);
                }
            m_CheckBoxFixImageSize.EnableWindow(true);
            m_ButtonReset.EnableWindow(true);

            // Aspect ratio
            if(m_CheckBoxFixImageSize.GetCheck() || m_CheckBoxFixScaling.GetCheck())
                m_CheckBoxAspectRatio.EnableWindow(false);
            else
                m_CheckBoxAspectRatio.EnableWindow(true);

            m_CheckBoxAspectRatio.SetCheck(GetImportExport()->MaintainAspectRatioIsCheck());
            }
        else
            {
            StringBuffer.Format(L" ");
            m_EditScaleX.SetWindowText(StringBuffer);
            m_EditScaleX.EnableWindow(false);

            m_EditScaleY.SetWindowText(StringBuffer);
            m_EditScaleY.EnableWindow(false);

            m_CheckBoxFixScaling.SetCheck(false);
            m_CheckBoxFixScaling.EnableWindow(false);
            m_ButtonReset.EnableWindow(false);

            m_CheckBoxFixImageSize.EnableWindow(false);
            m_CheckBoxAspectRatio.EnableWindow(false);
            }
        }
    }

/////////////////////////////////////////////////////////////////////////////

void HUTImageSize::RefreshConfig()
    {

    }

/////////////////////////////////////////////////////////////////////////////

void HUTImageSize::OnStatusChangeChkResample()
    {
    CString StringBuffer;

    // Save resample option in ImportExport.
    GetImportExport()->SetResample(m_CheckBoxResample.GetCheck() != 0);

    RefreshControlsTo(HUTEXPORT_BLOCK_SECTION);
    RefreshControlsTo(HUTEXPORT_IMG_SIZE_SECTION);
    }


/////////////////////////////////////////////////////////////////////////////

void HUTImageSize::OnKillFocusEditHeight()
    {
    if (!m_IsDestroy)
        {
        HASSERT(!GetImportExport()->ImageSizeIsLock());

        CString EditPostion;
        INT Position;

        m_EditHeight.GetWindowText(EditPostion);
        Position = BeStringUtilities::Wtoi(EditPostion);

        if (Position != GetImportExport()->GetImageHeight())
            {
            GetImportExport()->SetImageHeight(Position);

            RefreshControlsTo(HUTEXPORT_BLOCK_SECTION);
            RefreshControlsTo(HUTEXPORT_IMG_SIZE_SECTION);
            }
        }
    }

/////////////////////////////////////////////////////////////////////////////

void HUTImageSize::OnKillFocusEditWidth()
    {
    if (!m_IsDestroy)
        {
        HASSERT(!GetImportExport()->ImageSizeIsLock());

        CString EditPostion;
        INT Position;

        m_EditWidth.GetWindowText(EditPostion);
        Position = BeStringUtilities::Wtoi(EditPostion);

        if (Position != GetImportExport()->GetImageWidth())
            {
            GetImportExport()->SetImageWidth(Position);

            RefreshControlsTo(HUTEXPORT_BLOCK_SECTION);
            RefreshControlsTo(HUTEXPORT_IMG_SIZE_SECTION);
            }
        }
    }

/////////////////////////////////////////////////////////////////////////////

void HUTImageSize::OnStatusChangeChkFixImageSize()
    {
    // Save option in ImportExport.
    GetImportExport()->SetImageSizeIsLock(m_CheckBoxFixImageSize.GetCheck() != 0);
    if(m_CheckBoxFixImageSize.GetCheck())
        GetImportExport()->SetMaintainAspectRatio(false);

    if(!m_CheckBoxFixImageSize.GetCheck() && !m_CheckBoxFixScaling.GetCheck())
        {
        GetImportExport()->SetScaleFactorX(GetImportExport()->GetScaleFactorX());
        GetImportExport()->SetScaleFactorY(GetImportExport()->GetScaleFactorY());
        }

    RefreshControlsTo(HUTEXPORT_IMG_SIZE_SECTION);
    }


/////////////////////////////////////////////////////////////////////////////

void HUTImageSize::OnKillFocusEditScaleX()
    {
    if (!m_IsDestroy)
        {
        HASSERT(!GetImportExport()->ScaleFactorIsLock());

        CString EditPostion;
        double Position;

        m_EditScaleX.GetWindowText(EditPostion);

        Position = BeStringUtilities::Wtof((LPCWSTR)EditPostion);

        GetImportExport()->SetScaleFactorX(Position);

        RefreshControlsTo(HUTEXPORT_BLOCK_SECTION);
        RefreshControlsTo(HUTEXPORT_IMG_SIZE_SECTION);
        }
    }

/////////////////////////////////////////////////////////////////////////////

void HUTImageSize::OnKillFocusEditScaleY()
    {
    if (!m_IsDestroy)
        {
        HASSERT(!GetImportExport()->ScaleFactorIsLock());

        CString EditPostion;
        double Position;

        m_EditScaleY.GetWindowText(EditPostion);

        Position = BeStringUtilities::Wtof(EditPostion);

        GetImportExport()->SetScaleFactorY(Position);

        RefreshControlsTo(HUTEXPORT_BLOCK_SECTION);
        RefreshControlsTo(HUTEXPORT_IMG_SIZE_SECTION);
        }
    }

/////////////////////////////////////////////////////////////////////////////

void HUTImageSize::OnStatusChangeChkFixScaling()
    {
    // Save option in ImportExport.
    GetImportExport()->SetScaleFactorIsLock(m_CheckBoxFixScaling.GetCheck() != 0);
    if(m_CheckBoxFixScaling.GetCheck())
        GetImportExport()->SetMaintainAspectRatio(false);


    if(!m_CheckBoxFixScaling.GetCheck() && !m_CheckBoxFixImageSize.GetCheck())
        {
        GetImportExport()->SetImageHeight(GetImportExport()->GetImageHeight());
        GetImportExport()->SetImageWidth(GetImportExport()->GetImageWidth());
        }

    RefreshDisplaySection(HUTEXPORT_IMG_SIZE_SECTION);
    }

/////////////////////////////////////////////////////////////////////////////

void HUTImageSize::OnStatusChangeChkAspectRatio()
    {
    // Save option in ImportExport.
    GetImportExport()->SetMaintainAspectRatio(m_CheckBoxAspectRatio.GetCheck() != 0);

    // Use the Scale X to reset the scale and height.
    if(m_CheckBoxAspectRatio.GetCheck() && !m_CheckBoxFixScaling.GetCheck() && !m_CheckBoxFixImageSize.GetCheck())
        GetImportExport()->SetScaleFactorX(GetImportExport()->GetScaleFactorX());

    RefreshControlsTo(HUTEXPORT_IMG_SIZE_SECTION);
    }


/////////////////////////////////////////////////////////////////////////////

void HUTImageSize::OnClickedButtonReset()
    {
    // Reset button should be available only when resample
    // option is set.
    HASSERT(GetImportExport()->GetResample());

    CString StringBuffer;

    // Image size section (Default resample size).
    if(!GetImportExport()->ImageSizeIsLock())
        {
        GetImportExport()->SetImageWidth((uint32_t)GetImportExport()->GetDefaultResampleSize().GetX());
        GetImportExport()->SetImageHeight((uint32_t)GetImportExport()->GetDefaultResampleSize().GetY());
        }

    // Scaling Section (Default resample scaling).
    if(!GetImportExport()->ScaleFactorIsLock())
        {
        GetImportExport()->SetScaleFactorX(GetImportExport()->GetDefaultResampleScaleFactorX());
        GetImportExport()->SetScaleFactorY(GetImportExport()->GetDefaultResampleScaleFactorY());
        }

    RefreshControlsTo(HUTEXPORT_IMG_SIZE_SECTION);
    }
