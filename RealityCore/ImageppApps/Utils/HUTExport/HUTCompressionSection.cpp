//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTCompressionSection.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// HUTCompressionSection.cpp : implementation file
//
#include "stdafx.h"

#include "HUTCompressionSection.h"
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
#include <ImagePP/all/h/HCDCodecPackBits.h>
#include <ImagePP/all/h/HCDCodecPCX.h>
#include <ImagePP/all/h/HCDCodecRLE1.h>
#include <ImagePP/all/h/HCDCodecRLE8.h>
#include <ImagePP/all/h/HCDCodecSingleColor.h>
#include <ImagePP/all/h/HCDCodecZlib.h>
#include <ImagePP/all/h/HCDCodecLZW.h>
#include <ImagePP/all/h/HCDCodecTgaRLE.h>

#ifdef __HMR_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// HUTCompressionSection dialog


HUTCompressionSection::HUTCompressionSection(HRFImportExport* pi_pImportExport, CWnd* pParent /*=NULL*/)
    : HUTSectionDialog(pi_pImportExport, pParent)
    {
    //{{AFX_DATA_INIT(HUTCompressionSection)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    }

void HUTCompressionSection::DoDataExchange(CDataExchange* pDX)
    {
    HUTSectionDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(HUTCompressionSection)
    DDX_Control(pDX, IDC_SPIN_SET_QUALITY, m_SpinQuality);
    DDX_Control(pDX, IDC_COMPRESSION_QUALITY_SLIDER, m_QualitySlider);
    DDX_Control(pDX, IDC_EDIT_QUALITY, m_EditQuality);
    DDX_Control(pDX, IDC_CMB_COMPRESSION, m_ComboCompression);
    //}}AFX_DATA_MAP
    }


BEGIN_MESSAGE_MAP(HUTCompressionSection, HUTSectionDialog)
    //{{AFX_MSG_MAP(HUTCompressionSection)
    ON_EN_CHANGE(IDC_EDIT_QUALITY, OnChangeEditQuality)
    ON_WM_TIMER()
    ON_WM_DESTROY()
    ON_CBN_SELCHANGE(IDC_CMB_COMPRESSION, OnSelchangeCmbCompression)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// HUTCompressionSection message handlers

BOOL HUTCompressionSection::OnInitDialog()
    {
    HUTSectionDialog::OnInitDialog();

    m_SpinQuality.SetBuddy(&m_EditQuality);
    m_SpinQuality.SetBase (10);
    RefreshControls();

    m_uTimerID = SetTimer(ID_HUT_TIMER,500,NULL);

    return true;  // return true unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return false
    }

/////////////////////////////////////////////////////////////////////////////

void HUTCompressionSection::RefreshConfig()
    {

    }

/////////////////////////////////////////////////////////////////////////////

void HUTCompressionSection::OnChangeEditQuality()
    {
    // TODO: If this is a RICHEDIT control, the control will not
    // send this notification unless you override the HUTSectionDialog::OnInitDialog()
    // function to send the EM_SETEVENTMASK message to the control
    // with the ENM_CHANGE flag ORed into the lParam mask.

    // TODO: Add your control notification handler code here
    CString EditPostion;
    uint32_t Position;

    m_EditQuality.GetWindowText(EditPostion);
    Position = BeStringUtilities::Wtoi(EditPostion);
    m_QualitySlider.SetPos(Position);

    if (Position <= CountCompressionStep())
        SelectCompressionLevel(Position);
    }

/////////////////////////////////////////////////////////////////////////////

void HUTCompressionSection::OnTimer(UINT_PTR nIDEvent)
    {
    CDialog::OnTimer(nIDEvent);

    if (IsShowSection())
        m_SpinQuality.SetPos(m_QualitySlider.GetPos());

    // Eat spurious WM_TIMER messages
    MSG msg;
    while(::PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
    }

/////////////////////////////////////////////////////////////////////////////

void HUTCompressionSection::OnDestroy()
    {
    CDialog::OnDestroy();

    KillTimer(m_uTimerID);
    }

/////////////////////////////////////////////////////////////////////////////

void HUTCompressionSection::RefreshControls(void)
    {
    if (!m_HiddenSection && ((GetImportExport()->CountCodecs() >= 1) && (CountCompressionStep() >  1)))
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_COMPRESSION_SECTION, true);
        SetShowState(true);

        UDACCEL pAccel;

        pAccel.nSec =  1;
        pAccel.nInc =  1;       // Step Increment

        UINT MinimumRange    =  0;
        UINT MaximumRange    =  CountCompressionStep();
        UINT defaultPosition =  GetSelectedCompressionLevel();

        m_ComboCompression.ResetContent();

        // Fill with the codecs list
        unsigned short ItemIndex;
        unsigned short SelectedCodecIndex = 0;
        HCLASS_ID Codec;
        HCLASS_ID SelectedCodec = GetImportExport()->GetSelectedCodec();

        for (uint32_t Index=0; Index<GetImportExport()->CountCodecs(); Index++)
            {
            Codec = GetImportExport()->GetCodec(Index);

            WString CodecDescription;
            CodecDescription.AssignUtf8(HUTClassIDDescriptor::GetInstance()->GetClassLabelCodec(Codec).c_str());
            ItemIndex = (unsigned short)m_ComboCompression.AddString(CodecDescription.c_str());
            m_ComboCompression.SetItemData(ItemIndex, Codec);

            if (Codec == SelectedCodec)
                SelectedCodecIndex = ItemIndex;
            }

        m_ComboCompression.SetCurSel(SelectedCodecIndex);

        m_SpinQuality.SetRange((short)MinimumRange, (short)MaximumRange);
        m_SpinQuality.SetPos  (defaultPosition);
        m_SpinQuality.SetAccel(1, &pAccel);

        m_QualitySlider.SetRange( MinimumRange, MaximumRange, true);
        m_QualitySlider.SetTicFreq(10);
        m_QualitySlider.SetPos(defaultPosition);
        }
    else
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_COMPRESSION_SECTION, false);
        SetShowState(false);
        }
    }

void HUTCompressionSection::OnSelchangeCmbCompression()
    {
    GetImportExport()->SelectCodec((HCLASS_ID)m_ComboCompression.GetItemData(m_ComboCompression.GetCurSel()));
//    GetImportExport()->SelectCodec((uint32_t) m_ComboCompression.GetCurSel());
    RefreshDisplaySection(HUTEXPORT_COMPRESSION_SECTION);
    }

uint32_t HUTCompressionSection::CountCompressionStep()
    {
    uint32_t CompressionSteps;

    if (GetImportExport()->CountCompressionStep() != 0)
        {
        HASSERT(GetImportExport()->CountCompressionRatioStep() == 0);

        CompressionSteps = GetImportExport()->CountCompressionStep();
        }
    else
        {
        CompressionSteps = GetImportExport()->CountCompressionRatioStep();
        }

    return CompressionSteps;
    }

void HUTCompressionSection::SelectCompressionLevel(uint32_t pi_Position)
    {
    if (GetImportExport()->CountCompressionStep() != 0)
        {
        GetImportExport()->SelectCompressionQuality(pi_Position);
        }
    else
        {
        GetImportExport()->SelectCompressionRatio(pi_Position);
        }
    }

uint32_t HUTCompressionSection::GetSelectedCompressionLevel() const
    {
    uint32_t CompressionLevel;

    if (GetImportExport()->CountCompressionStep() != 0)
        {
        CompressionLevel = GetImportExport()->GetSelectedCompressionQuality();
        }
    else
        {
        CompressionLevel = GetImportExport()->GetSelectedCompressionRatio();
        }

    return CompressionLevel;
    }