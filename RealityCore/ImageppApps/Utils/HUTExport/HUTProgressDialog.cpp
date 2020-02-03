//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTProgressDialog.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// HUTProgressDialog.cpp : implementation file
//

#include "stdafx.h"

#include "HUTProgressDialog.h"
#include "HUTresource.h"

#ifdef __HMR_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// HUTProgressDialog dialog


HUTProgressDialog::HUTProgressDialog(CWnd* pParent /*=NULL*/)
    : CDialog(HUTProgressDialog::IDD, pParent),
      HFCProgressDurationListener()
    {
    //{{AFX_DATA_INIT(HUTProgressDialog)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    m_pParent = pParent;
    m_pProgressIndicator = 0;
    m_pLifeProgressIndicator = 0;
    }

/////////////////////////////////////////////////////////////////////////////
HUTProgressDialog::~HUTProgressDialog()
    {
    if(m_hWnd!=NULL)
        DestroyWindow();
    }

void HUTProgressDialog::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(HUTProgressDialog)
    DDX_Control(pDX, IDC_STOP_PROGRESSION, m_StopButton);
    DDX_Control(pDX, IDC_PROGRESS_BAR, m_ProgressBar);
    //}}AFX_DATA_MAP
    }


BEGIN_MESSAGE_MAP(HUTProgressDialog, CDialog)
    //{{AFX_MSG_MAP(HUTProgressDialog)
    ON_BN_CLICKED(IDC_STOP_PROGRESSION, OnButtonStop)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// HUTProgressDialog message handlers

BOOL HUTProgressDialog::OnInitDialog()
    {
    CDialog::OnInitDialog();

    // TODO: Add extra initialization here

    return true;  // return true unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return false
    }

void HUTProgressDialog::PumpMessages()
    {
    // Must call Create() before using the dialog
    ASSERT(m_hWnd!=NULL);

    MSG msg;
    // Handle dialog messages
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
        if(!IsDialogMessage(&msg))
            {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            }
        }
    }

void HUTProgressDialog::OnButtonStop()
    {
    PumpMessages();

    m_pProgressIndicator->StopIteration();
    if (m_pLifeProgressIndicator)
        m_pLifeProgressIndicator->StopIteration();
    }

void HUTProgressDialog::Progression(HFCProgressIndicator* pi_pProgressIndicator,// Indicator
                                    uint32_t                pi_Processed,            // Total processed items count.
                                    uint32_t                pi_CountProgression)  // number of items.
    {
    if (!pi_pProgressIndicator->IsLifeSignal())
        HFCProgressDurationListener::Progression(pi_pProgressIndicator, pi_Processed, pi_CountProgression);

    // open the dialog only when the export take more of 5 seconds
    if ((m_pProgressIndicator == 0) && !pi_pProgressIndicator->IsLifeSignal() && ((pi_CountProgression * GetAverageDuration()) > 5))
        {
        m_EyePos  =  10;
        m_LeftDir = false;

        CDialog::Create(HUTProgressDialog::IDD, m_pParent);

        m_ProgressBar.SetRange(0, (short)pi_CountProgression);
        m_ProgressBar.SetPos((int)pi_Processed);
        m_ProgressBar.SetStep(1);

        CenterWindow();
        ShowWindow(SW_SHOW);

        m_StopButton.ShowWindow(SW_SHOW);
        m_pProgressIndicator = pi_pProgressIndicator;
        }

    if (m_pProgressIndicator != 0)
        {
        if (!pi_pProgressIndicator->IsLifeSignal())
            {
            WString TimeString;
            WChar  buffer[200];

            PumpMessages();
            m_ProgressBar.StepIt();

            // Rest number of items to be processed approx duration.
            TimeString += L"Approx time to be processed: ";

            double TimeToDo = (pi_CountProgression - pi_Processed) * GetAverageDuration();
            uint32_t  Hours    = ((uint32_t)TimeToDo / 3600);
            uint32_t  Minutes  = ((uint32_t)TimeToDo % 3600) / 60;
            uint32_t  Seconds  = ((uint32_t)TimeToDo % 3600) % 60;

            if (Hours > 0)
                {
                BeStringUtilities::Snwprintf(buffer, L"%02ldh", Hours);
                TimeString += buffer;
                }
            else
                TimeString += L"   ";

            if ((Hours > 0) || (Minutes > 0))
                {
                BeStringUtilities::Snwprintf(buffer, L"%02ldm", Minutes);
                TimeString += buffer;
                }
            else
                TimeString += L"   ";

            BeStringUtilities::Snwprintf(buffer, L"%02lds", Seconds);
            TimeString += buffer;

            GetDlgItem(IDC_DURATION)->SetWindowText(TimeString.c_str());

            if (pi_Processed == pi_CountProgression - 1)
                {
                // Delay of 1.0 seconds to see the last progression.
                clock_t wait = (clock_t)1.0 * CLOCKS_PER_SEC;
                clock_t goal;
                goal = wait + clock();
                while( goal > clock() )
                    ;
                CDialog::DestroyWindow();
                m_pProgressIndicator = 0;
                }
            }
        else
            {
            if (!m_pLifeProgressIndicator)
                m_pLifeProgressIndicator = pi_pProgressIndicator;

            PumpMessages();
            // Draw the life signal
            WChar  LifeString[23] = L" .................... ";

            if (m_LeftDir)
                m_EyePos--;
            else
                m_EyePos++;

            if (m_EyePos == 20)
                m_LeftDir = true;

            if (m_EyePos == 1)
                m_LeftDir = false;

            LifeString[m_EyePos] = L'';

            WString lString;
            lString += LifeString;
            GetDlgItem(IDC_DRAWSIGNAL)->SetWindowText(lString.c_str());
            }
        }
    }

