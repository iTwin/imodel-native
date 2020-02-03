//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTGeoReferenceSection.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HUTGeoReferenceSection
//-----------------------------------------------------------------------------
// These are the class used to select the GeoReference format.
//-----------------------------------------------------------------------------
#if !defined(AFX_HUTGEOREFERENCESECTION_H__0DCC1526_A98B_11D2_8400_0060082DE76F__INCLUDED_)
#define AFX_HUTGEOREFERENCESECTION_H__0DCC1526_A98B_11D2_8400_0060082DE76F__INCLUDED_

#pragma once

#include "HUTSectionDialog.h"
#include "HUTClassID.h"

class HUTEXPORT_LINK_MODE HUTGeoReferenceSection : public HUTSectionDialog
    {
public:
    HUTGeoReferenceSection(HRFImportExport* pi_pImportExport, CWnd* pParent = NULL);   // standard constructor

    //{{AFX_DATA(HUTGeoReferenceSection)
    enum { IDD = IDD_DLG_GEO_REF_FORMAT };
    CComboBox        m_ComboGeoRefFormat;
    //}}AFX_DATA

    virtual void RefreshControls();
    virtual void RefreshConfig();

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HUTGeoReferenceSection)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:

    // Generated message map functions
    //{{AFX_MSG(HUTGeoReferenceSection)
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg void OnSelchangeCmbGeoRefFormat();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    };

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUTGEOREFERENCESECTION_H__0DCC1526_A98B_11D2_8400_0060082DE76F__INCLUDED_)
