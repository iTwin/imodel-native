//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTPixelType.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// HUTPixelType.cpp : implementation file
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "HUTPixelType.h"
#include "HUTDialogContainer.h"

#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <ImagePP/all/h/HRPPixelTypeI2R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeI4R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeI4R8G8B8A8.h>
#include <ImagePP/all/h/HRPPixelTypeI8R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <ImagePP/all/h/HRPPixelTypeV1Gray1.h>
#include <ImagePP/all/h/HRPPixelTypeV8Gray8.h>
#include <ImagePP/all/h/HRPPixelTypeV8GrayWhite8.h>
#include <ImagePP/all/h/HRPPixelTypeV16B5G5R5.h>
#include <ImagePP/all/h/HRPPixelTypeV16R5G6B5.h>
#include <ImagePP/all/h/HRPPixelTypeV16PRGray8A8.h>
#include <ImagePP/all/h/HRPPixelTypeV24B8G8R8.h>
#include <ImagePP/all/h/HRPPixelTypeV24PhotoYCC.h>
#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeV32A8R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeV32PRPhotoYCCA8.h>
#include <ImagePP/all/h/HRPPixelTypeV32PR8PG8PB8A8.h>
#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP/all/h/HRPPixelTypeV1GrayWhite1.h>
#include <ImagePP/all/h/HUTClassIDDescriptor.h>

#ifdef __HMR_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
// HUTPixelType dialog
//-----------------------------------------------------------------------------

HUTPixelType::HUTPixelType(HRFImportExport* pi_pImportExport, CWnd* pParent /*=NULL*/)
    : HUTSectionDialog(pi_pImportExport, pParent)
    {
    //{{AFX_DATA_INIT(HUTPixelType)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HUTPixelType::DoDataExchange(CDataExchange* pDX)
    {
    HUTSectionDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(HUTPixelType)
    DDX_Control(pDX, IDC_CMB_PIXEL_TYPE, m_comboPixelType);
    //}}AFX_DATA_MAP
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(HUTPixelType, HUTSectionDialog)
    //{{AFX_MSG_MAP(HUTPixelType)
    ON_CBN_SELCHANGE(IDC_CMB_PIXEL_TYPE, OnSelchangeCmbPixelType)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// HUTPixelType message handlers
//-----------------------------------------------------------------------------

void HUTPixelType::RefreshControls()
    {
    if (!m_HiddenSection && (GetImportExport()->CountPixelType() > 1))
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_PIXEL_TYPE_SECTION, true);
        SetShowState(true);

        m_comboPixelType.ResetContent();

        // Fill with the pixel type
        for (uint32_t Index=0; Index<GetImportExport()->CountPixelType(); Index++)
            {
            WString PixelDescription;
            PixelDescription.AssignUtf8(HUTClassIDDescriptor::GetInstance()->GetClassLabelPixelType(GetImportExport()->GetPixelType(Index)).c_str());
            m_comboPixelType.AddString(PixelDescription.c_str());
            }

        m_comboPixelType.SetCurSel(GetImportExport()->GetSelectedPixelTypeIndex());
        }
    else
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_PIXEL_TYPE_SECTION, false);
        SetShowState(false);
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HUTPixelType::RefreshConfig()
    {

    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

BOOL HUTPixelType::OnInitDialog()
    {
    HUTSectionDialog::OnInitDialog();

    RefreshControls();

    return true;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HUTPixelType::OnSelchangeCmbPixelType()
    {
    GetImportExport()->SelectPixelTypeByIndex(m_comboPixelType.GetCurSel());
    RefreshDisplaySection(HUTEXPORT_PIXEL_TYPE_SECTION);
    }
