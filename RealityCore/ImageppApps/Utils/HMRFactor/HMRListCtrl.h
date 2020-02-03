/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HMRFactor/HMRListCtrl.h $
|    $RCSfile: HMRListCtrl.h,v $
|   $Revision: 1.2 $
|       $Date: 2006/02/16 21:24:34 $
|     $Author: BENTLEY\Simon.Normand $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

class HMRListCtrl : public CListCtrl
{
    public:
        HMRListCtrl ();
        virtual ~HMRListCtrl();

        void AddFilename (CString& filename);
        void RemoveSelected ();

// Implementation
    protected:
        //{{AFX_MSG(HMRListCtrl)
        afx_msg void OnDropFiles(HDROP hDropInfo);
        afx_msg void OnKeyDown (UINT nChar, UINT nRepCnt, UINT nFlags); 

        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

    private:

};