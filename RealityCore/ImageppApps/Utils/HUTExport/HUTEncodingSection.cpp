//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTEncodingSection.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// HUTEncodingSection.cpp : implementation file
//
#include "stdafx.h"

#include "HUTEncodingSection.h"
#include "HUTDialogContainer.h"
#include <ImagePP/all/h/HUTClassIDDescriptor.h>

#ifdef __HMR_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// HUTEncodingSection dialog


HUTEncodingSection::HUTEncodingSection(HRFImportExport* pi_pImportExport, CWnd* pParent /*=NULL*/)
    : HUTSectionDialog(pi_pImportExport, pParent)
    {
    //{{AFX_DATA_INIT(HUTEncodingSection)
    //}}AFX_DATA_INIT
    }


void HUTEncodingSection::DoDataExchange(CDataExchange* pDX)
    {
    HUTSectionDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(HUTEncodingSection)
    DDX_Control(pDX, IDC_CMB_RESAMPLING, m_comboResampling);
    DDX_Control(pDX, IDC_CMB_ENCODING, m_comboEncoding);
    //}}AFX_DATA_MAP
    }


BEGIN_MESSAGE_MAP(HUTEncodingSection, HUTSectionDialog)
    //{{AFX_MSG_MAP(HUTEncodingSection)
    ON_CBN_SELCHANGE(IDC_CMB_ENCODING, OnSelchangeCmbEncoding)
    ON_CBN_SELCHANGE(IDC_CMB_RESAMPLING, OnSelchangeCmbResampling)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// HUTEncodingSection message handlers

/////////////////////////////////////////////////////////////////////////////

void HUTEncodingSection::RefreshControls()
    {
    if (!m_HiddenSection && ((GetImportExport()->CountEncoding() > 1) || (GetImportExport()->CountDownSamplingMethod() > 1)))
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_ENCODING_SECTION, true);
        SetShowState(true);

        // Fill with the pixel type
        m_comboEncoding.ResetContent();
        for (uint32_t Index = 0; Index < GetImportExport()->CountEncoding(); Index++)
            {
            WString EncodingW(HUTClassIDDescriptor::GetInstance()->GetClassLabelEncodingType(GetImportExport()->GetEncoding(Index)).c_str(),
                               BentleyCharEncoding::Utf8);
            m_comboEncoding.AddString(EncodingW.c_str());
            }            

        m_comboEncoding.SetCurSel(GetImportExport()->GetSelectedEncodingIndex());

        if (GetImportExport()->CountDownSamplingMethod() != 0)
            {
            m_comboResampling.ResetContent();
            for (uint32_t Index = 0; Index < GetImportExport()->CountDownSamplingMethod(); Index++)
                {
                WString SamplingW(HUTClassIDDescriptor::GetInstance()->GetClassLabelResampling(GetImportExport()->GetDownSamplingMethod(Index)).c_str(),
                                 BentleyCharEncoding::Utf8);
                m_comboResampling.AddString(SamplingW.c_str());
                }

            m_comboResampling.SetCurSel(0);
            OnSelchangeCmbResampling();
            m_comboResampling.EnableWindow(true);
            }
        else
            {
            m_comboResampling.SetCurSel(0);
            m_comboResampling.SetWindowText(L"Not available");
            m_comboResampling.EnableWindow(false);
            }
        }
    else
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_ENCODING_SECTION, false);
        SetShowState(false);
        }
    }

/////////////////////////////////////////////////////////////////////////////

void HUTEncodingSection::RefreshConfig()
    {

    }

/////////////////////////////////////////////////////////////////////////////

BOOL HUTEncodingSection::OnInitDialog()
    {
    HUTSectionDialog::OnInitDialog();

    RefreshControls();

    return true;  // return true unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return false
    }

/////////////////////////////////////////////////////////////////////////////

void HUTEncodingSection::OnSelchangeCmbEncoding()
    {
    GetImportExport()->SelectEncodingByIndex(m_comboEncoding.GetCurSel());
    RefreshDisplaySection(HUTEXPORT_ENCODING_SECTION);
    }


/////////////////////////////////////////////////////////////////////////////

void HUTEncodingSection::OnSelchangeCmbResampling()
    {
    GetImportExport()->SelectDownSamplingMethodByIndex(m_comboResampling.GetCurSel());
    }
