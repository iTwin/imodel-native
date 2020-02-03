/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ColorReplacerDlg.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//----------------------------------------------------------------------------
// ColorReplacerDlg dialog
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

#ifndef __ColorReplacerDlg_H_
#define __ColorReplacerDlg_H_

#if _MSC_VER > 1000
    #pragma once
#endif // _MSC_VER > 1000

#include <Imagepp/all/h/HGFRGBSet.h>

class CActiveImageView;
class CActiveImageDoc;

class ColorReplacerDlg : public CDialog
{
    public:
	    ColorReplacerDlg(CActiveImageDefaultMode* pi_pParent, CActiveImageView* pi_pView, CActiveImageDoc* pi_pDoc);   // standard constructor
        virtual ~ColorReplacerDlg();

        const HGFRGBSet& GetSelectedRGBSet() const;
        const HGFRGBSet& GetSelectedRemoveRGBSet() const;

	    //{{AFX_DATA(ColorReplacerDlg)
	    enum { IDD = IDD_DLG_COLOR_REPLACEMENT };
		    // NOTE: the ClassWizard will add data members here
	    //}}AFX_DATA


	    // ClassWizard generated virtual function overrides
	    //{{AFX_VIRTUAL(ColorReplacerDlg)
	protected:
	    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

    protected:

	    // Generated message map functions
	    //{{AFX_MSG(ColorReplacerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtClose();
	afx_msg void OnBtCubeSelect();
	afx_msg void OnBtNewColor();
	afx_msg void OnBtCubeRemove();
	//}}AFX_MSG
	    DECLARE_MESSAGE_MAP()

    private:

        CActiveImageView*        m_pView;
        CActiveImageDoc*         m_pDoc;
        CActiveImageDefaultMode* m_pParent;

        HGFRGBSet m_RGBSet;
        HGFRGBSet m_RGBRemoveSet;

        bool IsColorPicking;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // __ColorReplacerDlg_H_
