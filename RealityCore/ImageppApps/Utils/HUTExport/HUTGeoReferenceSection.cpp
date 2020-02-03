//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTGeoReferenceSection.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HUTGeoReferenceSection
//-----------------------------------------------------------------------------
// These are the class used to select the GeoReference format.
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "HUTGeoReferenceSection.h"
#include "HUTDialogContainer.h"
#include <ImagePP/all/h/HUTClassIDDescriptor.h>

#ifdef __HMR_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


HUTGeoReferenceSection::HUTGeoReferenceSection(HRFImportExport* pi_pImportExport, CWnd* pParent /*=NULL*/)
    : HUTSectionDialog(pi_pImportExport, pParent)
    {
    //{{AFX_DATA_INIT(HUTCompressionSection)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    }


void HUTGeoReferenceSection::DoDataExchange(CDataExchange* pDX)
    {
    HUTSectionDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(HUTGeoReferenceSection)
    DDX_Control(pDX, IDC_CMB_GEO_REF_FORMAT, m_ComboGeoRefFormat);
    //}}AFX_DATA_MAP
    }


BEGIN_MESSAGE_MAP(HUTGeoReferenceSection, HUTSectionDialog)
    //{{AFX_MSG_MAP(HUTGeoReferenceSection)
    ON_WM_DESTROY()
    ON_CBN_SELCHANGE(IDC_CMB_GEO_REF_FORMAT, OnSelchangeCmbGeoRefFormat)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// HUTCompressionSection message handlers

BOOL HUTGeoReferenceSection::OnInitDialog()
    {
    HUTSectionDialog::OnInitDialog();

    RefreshControls();

    return true;  // return true unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return false
    }

/////////////////////////////////////////////////////////////////////////////

void HUTGeoReferenceSection::RefreshConfig()
    {

    }


/////////////////////////////////////////////////////////////////////////////

void HUTGeoReferenceSection::OnDestroy()
    {
    CDialog::OnDestroy();
    }

/////////////////////////////////////////////////////////////////////////////

void HUTGeoReferenceSection::RefreshControls(void)
    {
    uint32_t Count = GetImportExport()->CountGeoreferenceFormats();
    if (!m_HiddenSection &&
        (Count >= 1 && !(Count == 1 && GetImportExport()->GetGeoreferenceFormat(0) == HRFGeoreferenceFormat::GEOREFERENCE_IN_IMAGE)))
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_GEO_REF_SECTION, true);
        SetShowState(true);

        m_ComboGeoRefFormat.ResetContent();

        // Fill with the codecs list
        unsigned short ItemIndex;
        unsigned short SelectedGeoRefFormatIndex = 0;

        HRFGeoreferenceFormat SelectedGeoRefFormat = GetImportExport()->GetSelectedGeoreferenceFormat();
        HRFGeoreferenceFormat GeoRefFormat;

        for (uint32_t Index=0; Index<Count; Index++)
            {
            GeoRefFormat = GetImportExport()->GetGeoreferenceFormat(Index);
            WString ClassLabelGeoRefW(HUTClassIDDescriptor::GetInstance()->GetClassLabelGeoRef(GeoRefFormat).c_str(), BentleyCharEncoding::Utf8);            
            ItemIndex    = (unsigned short)m_ComboGeoRefFormat.AddString(ClassLabelGeoRefW.c_str());

            m_ComboGeoRefFormat.SetItemData(ItemIndex, GetImportExport()->GetSelectedGeoreferenceFormatIndex());

            if (GeoRefFormat == SelectedGeoRefFormat)
                SelectedGeoRefFormatIndex = ItemIndex;
            }

        m_ComboGeoRefFormat.SetCurSel(SelectedGeoRefFormatIndex);
        }
    else
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_GEO_REF_SECTION, false);
        SetShowState(false);
        }
    }


void HUTGeoReferenceSection::OnSelchangeCmbGeoRefFormat()
    {
    GetImportExport()->SelectGeoreferenceFormatByIndex(m_ComboGeoRefFormat.GetCurSel());

    RefreshDisplaySection(HUTEXPORT_IMG_SIZE_SECTION);
    RefreshDisplaySection(HUTEXPORT_GEO_REF_SECTION);
    }
