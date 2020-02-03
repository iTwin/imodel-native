//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTSmallCompressionSection.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include "stdafx.h"

#include "HUTSmallCompressionSection.h"
#include "HUTDialogContainer.h"

#include <ImagePP/all/h/HUTClassIDDescriptor.h>

#include <ImagePP/all/h/HCDCodecBMPRLE8.h>
#include <ImagePP/all/h/HCDCodecCCITT.h>
#include <ImagePP/all/h/HCDCodecDeflate.h>
#include <ImagePP/all/h/HCDCodecFlashpix.h>
#include <ImagePP/all/h/HCDCodecFPXSingleColor.h>
#include <ImagePP/all/h/HCDCodecGIF.h>
#include <ImagePP/all/h/HCDCodecHMRCCITT.h>
#include <ImagePP/all/h/HCDCodecHMRGIF.h>
#include <ImagePP/all/h/HCDCodecHMRPackBits.h>
#include <ImagePP/all/h/HCDCodecHMRRLE1.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HCDCodecIJG.h>
#include <ImagePP/all/h/HCDCodecJPEG.h>
#include <ImagePP/all/h/HCDCodecLzw.h>
#include <ImagePP/all/h/HCDCodecPackBits.h>
#include <ImagePP/all/h/HCDCodecPCX.h>
#include <ImagePP/all/h/HCDCodecRLE1.h>
#include <ImagePP/all/h/HCDCodecRLE8.h>
#include <ImagePP/all/h/HCDCodecSingleColor.h>
#include <ImagePP/all/h/HCDCodecZlib.h>

#ifdef __HMR_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// HUTSmallCompressionSection dialog


HUTSmallCompressionSection::HUTSmallCompressionSection(HRFImportExport* pi_pImportExport, CWnd* pParent /*=NULL*/)
    : HUTSectionDialog(pi_pImportExport, pParent)
    {
    //{{AFX_DATA_INIT(HUTSmallCompressionSection)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    }


void HUTSmallCompressionSection::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(HUTSmallCompressionSection)
    DDX_Control(pDX, IDC_CMB_COMPRESSION, m_ComboCompression);
    //}}AFX_DATA_MAP
    }


BEGIN_MESSAGE_MAP(HUTSmallCompressionSection, CDialog)
    //{{AFX_MSG_MAP(HUTSmallCompressionSection)
    ON_CBN_SELCHANGE(IDC_CMB_COMPRESSION, OnSelchangeCmbCompression)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// HUTSmallCompressionSection message handlers

void HUTSmallCompressionSection::OnSelchangeCmbCompression()
    {
    GetImportExport()->SelectCodecByIndex(m_ComboCompression.GetCurSel());
    RefreshDisplaySection(HUTEXPORT_SMALL_COMPRESSION_SECTION);
    }

/////////////////////////////////////////////////////////////////////////////

BOOL HUTSmallCompressionSection::OnInitDialog()
    {
    HUTSectionDialog::OnInitDialog();

    RefreshControls();

    return true;  // return true unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return false
    }

/////////////////////////////////////////////////////////////////////////////

void HUTSmallCompressionSection::RefreshConfig()
    {

    }

/////////////////////////////////////////////////////////////////////////////

void HUTSmallCompressionSection::RefreshControls(void)
    {
    if (!m_HiddenSection && ((GetImportExport()->CountCodecs() > 1) && (GetImportExport()->CountCompressionStep() <=  1)))
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_SMALL_COMPRESSION_SECTION, true);
        SetShowState(true);

        m_ComboCompression.ResetContent();

        // Fill with the codecs list
        HCLASS_ID Codec;
        HCLASS_ID SelectedCodec = GetImportExport()->GetSelectedCodec();
        unsigned short ItemIndex;
        unsigned short SelectCodecIndex = 0;
        for (uint32_t Index=0; Index<GetImportExport()->CountCodecs(); Index++)
            {
            Codec = GetImportExport()->GetCodec(Index);

            WString CodecDescription;
            CodecDescription.AssignUtf8(HUTClassIDDescriptor::GetInstance()->GetClassLabelCodec(Codec).c_str());
            ItemIndex = (unsigned short)m_ComboCompression.AddString(CodecDescription.c_str());

            m_ComboCompression.SetItemData(ItemIndex, Codec);

            if (Codec == SelectedCodec)
                SelectCodecIndex = ItemIndex;

            }

        m_ComboCompression.SetCurSel(SelectCodecIndex);
        }
    else
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_SMALL_COMPRESSION_SECTION, false);
        SetShowState(false);
        }
    }

