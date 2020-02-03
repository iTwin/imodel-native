/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/HMRProgressImage.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/HMRProgressImage.h,v 1.1 2003/02/26 20:01:03 SebastienGosselin Exp $
//-----------------------------------------------------------------------------
// Class : HMRProgressImage
//-----------------------------------------------------------------------------

#ifndef __HMRProgressImage_H__
#define __HMRProgressImage_H__

#include <Imagepp/all/h/HMGMessageReceiver.h>
#include <Imagepp/all/h/HRARaster.h>
#include <Imagepp/all/h/HGF2DExtent.h>


class HMRProgressImage : public HMGMessageReceiver
{
    public:

        // Create new file (overwrites any existing file)
                HMRProgressImage(HRARaster*     pi_pRaster,
                                 CWnd*          pio_pWindow);

        // Destructor
                ~HMRProgressImage  ();


        bool NotifyProgressImageChanged (const HMGMessage& pi_rMessage);
        void  Redraw ();


    protected:

        // members

    private:

        HRARaster*          m_pRaster;
        CWnd*               m_pWindow;

        clock_t             m_LastClock;
        clock_t             m_Elapse;

        typedef list<HGF2DExtent*, allocator<HGF2DExtent*> >
                RefreshList;
        RefreshList         m_RefreshList;

        HMG_DECLARE_MESSAGE_MAP()
};


#endif  // __HMRProgressImage_H__
