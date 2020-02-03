//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTSubresPixelType.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include "stdafx.h"

#include "HUTSubresPixelType.h"
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
// HUTSubresPixelType dialog
//-----------------------------------------------------------------------------

HUTSubresPixelType::HUTSubresPixelType (HRFImportExport* pi_pImportExport, CWnd* pParent)
    : HUTSectionDialog(pi_pImportExport, pParent)
    {
    //{{AFX_DATA_INIT(HUTSubresPixelType)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HUTSubresPixelType::DoDataExchange(CDataExchange* pDX)
    {
    HUTSectionDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(HUTSubresPixelType)
    DDX_Control(pDX, IDC_CMB_PIXEL_TYPE, m_comboPixelType);
    //}}AFX_DATA_MAP
    }

//-----------------------------------------------------------------------------
// HUTSubresPixelType message handlers
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(HUTSubresPixelType, HUTSectionDialog)
    //{{AFX_MSG_MAP(HUTSubresPixelType)
    ON_CBN_SELCHANGE(IDC_CMB_PIXEL_TYPE, OnSelchangeCmbPixelType)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HUTSubresPixelType::RefreshConfig()
    {

    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------


BOOL HUTSubresPixelType::OnInitDialog()
    {
    HUTSectionDialog::OnInitDialog();

    RefreshControls();

    return true;
    }

//-----------------------------------------------------------------------------
// HUTPixelType message handlers
//
// Display this section only if the pixeltype selected for the image is 1bit
//-----------------------------------------------------------------------------

void HUTSubresPixelType::RefreshControls()
    {
    HFCPtr<HRPPixelType> pPixelType(GetImportExport()->GetPixelType());
    HCLASS_ID SelectedPixelType = GetImportExport()->GetSelectedPixelType();

    if ((SelectedPixelType == HRPPixelTypeV1Gray1::CLASS_ID ||
         SelectedPixelType == HRPPixelTypeV1GrayWhite1::CLASS_ID) &&
        (!m_HiddenSection && (GetImportExport()->CountSubResPixelType() > 1)))
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_SUBRES_PIXEL_TYPE_SECTION, true);
        SetShowState(true);

        HCLASS_ID SubResPixelType;

        if (SelectedPixelType == HRPPixelTypeV1Gray1::CLASS_ID)
            SubResPixelType = HRPPixelTypeV8Gray8::CLASS_ID;
        else
            SubResPixelType = HRPPixelTypeV8GrayWhite8::CLASS_ID;

        string PixelDescription;

        // Fill with the pixel type
        HCLASS_ID PixelType;
        m_comboPixelType.ResetContent();
        uint32_t Index;
        for (Index=0; Index<GetImportExport()->CountPixelType(); Index++)
            {
            PixelType = GetImportExport()->GetPixelType(Index);

            if (PixelType == SelectedPixelType || PixelType == SubResPixelType)
                {
                WString PixelDescription(HUTClassIDDescriptor::GetInstance()->GetClassLabelPixelType(PixelType).c_str(),
                                         BentleyCharEncoding::Utf8);

                if (PixelType == SelectedPixelType && m_comboPixelType.GetCount() > 0)
                    {
                    m_comboPixelType.InsertString(0, PixelDescription.c_str());
                    m_comboPixelType.SetItemData(0, Index);
                    }
                else
                    {
                    int ItemIndex = m_comboPixelType.AddString(PixelDescription.c_str());
                    m_comboPixelType.SetItemData(ItemIndex, Index);
                    }
                }
            }

        uint32_t CurSubResPixelTypeIndex = GetImportExport()->GetSelectedSubResPixelTypeIndex();
        for (int Index2 = 0; Index2 < m_comboPixelType.GetCount(); Index2++)
            if (m_comboPixelType.GetItemData(Index2) == CurSubResPixelTypeIndex)
                m_comboPixelType.SetCurSel(Index2);
        }
    else
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_SUBRES_PIXEL_TYPE_SECTION, false);
        SetShowState(false);
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HUTSubresPixelType::OnSelchangeCmbPixelType()
    {
    GetImportExport()->SelectSubResPixelType(m_comboPixelType.GetCurSel());
    RefreshDisplaySection(HUTEXPORT_SUBRES_PIXEL_TYPE_SECTION);
    }
