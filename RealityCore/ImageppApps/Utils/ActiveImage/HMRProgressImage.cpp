/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/HMRProgressImage.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/HMRProgressImage.cpp,v 1.4 2011/07/18 21:10:28 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Class HRFRasterFile
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "ActiveImage.h"
#include "ActiveImageDoc.h"
#include "ActiveImageView.h"

#include "HMRProgressImage.h"
#include <Imagepp/all/h/HRAMessages.h>

#include <time.h>

#define ELAPSE_TIME_SECOND    ((clock_t)1)


HMG_BEGIN_RECEIVER_MESSAGE_MAP(HMRProgressImage, HMGMessageReceiver, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HMRProgressImage, HRAProgressImageChangedMsg, NotifyProgressImageChanged)
HMG_END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// public
// Constructor (Creation)
//
// Take a raw pointer on the Raster only, because problem when we
// destroy the Store, --> CleanUp load the Raster completly in memory.
//-----------------------------------------------------------------------------
HMRProgressImage::HMRProgressImage(HRARaster*        pi_pRaster,
                                   CWnd*             pio_pWindow)
{
    m_Elapse  = CLOCKS_PER_SEC * ELAPSE_TIME_SECOND;
    m_LastClock = clock();

    m_pRaster = pi_pRaster;
    m_pWindow = pio_pWindow;

    LinkTo (pi_pRaster, true);
}


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HMRProgressImage::~HMRProgressImage()
{
    while (m_RefreshList.size() > 0)
    {
        delete m_RefreshList.front();
        m_RefreshList.pop_front();
    }
    UnlinkFrom(m_pRaster);
}


//-----------------------------------------------------------------------------
// public
// NotifyContentChanged - Message Handler
//-----------------------------------------------------------------------------       
bool HMRProgressImage::NotifyProgressImageChanged (const HMGMessage& pi_rMessage)
{
    // add the current message to the refresh list

// Visual C++ 6.0   Compiler Bugs	//DMx
#if (defined(_WIN32) || defined(WIN32)) && (_MSC_VER == 1200)
	HGF2DExtent Ptr = ((HRAProgressImageChangedMsg&)pi_rMessage).GetShape().GetExtent(); 
    HGF2DExtent* pNew = new HGF2DExtent(Ptr);  
#else
    HGF2DExtent* pNew = new HGF2DExtent(((HRAProgressImageChangedMsg&)pi_rMessage).GetShape().GetExtent());  
#endif

    m_RefreshList.push_back(pNew);

    return false;
}


//-----------------------------------------------------------------------------
// public
// Redraw - 
//-----------------------------------------------------------------------------       
void HMRProgressImage::Redraw ()
{
    // If tile pending, drawing it
    if (m_RefreshList.size() > 0)
    {
        CActiveImageView* pView = (CActiveImageView*)m_pWindow;

        CRgn InvalidRgn;
        InvalidRgn.CreateRectRgn(0, 0, 1, 1);

        while (m_RefreshList.size() > 0)
        {
            // get the current extent
            HGF2DExtent* pExtent = m_RefreshList.front();
            pExtent->ChangeCoordSys(pView->GetCoordSys());
        
            // Create a region from that extent
            CRgn Current;
            Current.CreateRectRgn((int)pExtent->GetXMin(),
                                  (int)pExtent->GetYMin(),
                                  (int)pExtent->GetXMax(),
                                  (int)pExtent->GetYMax());
        
            // Get a copy of the invalid region
            CRgn Copy;
            Copy.CreateRectRgn(0, 0, 1, 1);
            Copy.CopyRgn(&InvalidRgn);
        
            // Combine to the invalidate region
            InvalidRgn.CombineRgn(&Copy, &Current, RGN_OR);
        
            // delete the current extent
            delete pExtent;
            m_RefreshList.erase(m_RefreshList.begin());
        }

        // redraw the view
#ifdef _DEBUG
        m_pWindow->RedrawWindow(NULL, &InvalidRgn, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
#else
        m_pWindow->RedrawWindow(NULL, &InvalidRgn, RDW_INVALIDATE | RDW_UPDATENOW);
#endif
    }
}

//---------------------------------------------------------- Privates

