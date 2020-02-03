/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageInsider/MyPropertyPage1.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// MyPropertyPage1.h : header file
//

#ifndef __MYPROPERTYPAGE1_H__
#define __MYPROPERTYPAGE1_H__

#include <Imagepp/all/h/HUTImportFromFileExportToFile.h>
#include <Imagepp/all/h/HFCProgressIndicator.h>

//-------------------------------------------------------------------------------------------------
// CMyPropertyPage1 dialog
//-------------------------------------------------------------------------------------------------

class CMyPropertyPage1 : public CPropertyPage, public HFCProgressDurationListener
{
	DECLARE_DYNCREATE(CMyPropertyPage1)

// Construction
public:
	CMyPropertyPage1();
	~CMyPropertyPage1();


// Dialog Data
	//{{AFX_DATA(CMyPropertyPage1)
	enum { IDD = IDD_PROPPAGE1 };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMyPropertyPage1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

    virtual void Progression(HFCProgressIndicator* pi_pProgressIndicator, 
                             uint32_t              pi_Processed,		 
                             uint32_t              pi_CountProgression);  

// Implementation
protected:
    bool  m_IsThumbnailCanceled;

    // Generated message map functions
	//{{AFX_MSG(CMyPropertyPage1)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnCheckQualityThumb();
	afx_msg void OnCheckGeoThumb();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    void DrawGDIThumbnail(Byte* pSrcPixels, uint32_t PreferedWidth, uint32_t PreferedHeight);
};


//-------------------------------------------------------------------------------------------------
// class ResourceListCtrl
//-------------------------------------------------------------------------------------------------
class ResourceListCtrl : public CListCtrl
{
public:
    ResourceListCtrl(){};
    virtual ~ResourceListCtrl(){};

protected:
    // Generated message map functions
    //{{AFX_MSG(ResourceListCtrl)
    afx_msg void OnKeyDown( UINT pi_Char,
                            UINT pi_RepCnt,
                            UINT pi_Flags );
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    void OnEditCopy();
    
};

//-------------------------------------------------------------------------------------------------
// CMyPropertyPage2 dialog
//-------------------------------------------------------------------------------------------------
class CMyPropertyPage2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CMyPropertyPage2)

// Construction
public:
	CMyPropertyPage2();
	~CMyPropertyPage2();

// Dialog Data
	//{{AFX_DATA(CMyPropertyPage2)
	enum { IDD = IDD_PROPPAGE2 };
	ResourceListCtrl	m_Resource;
	//}}AFX_DATA
  
// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMyPropertyPage2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
    
// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMyPropertyPage2)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

};


//-------------------------------------------------------------------------------------------------
// CMyPropertyPage3 dialog
//-------------------------------------------------------------------------------------------------

class CMyPropertyPage3 : public CPropertyPage
{
	DECLARE_DYNCREATE(CMyPropertyPage3)

// Construction
public:
	CMyPropertyPage3();
	~CMyPropertyPage3();

// Dialog Data
	//{{AFX_DATA(CMyPropertyPage3)
	enum { IDD = IDD_PROPPAGE3 };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMyPropertyPage3)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMyPropertyPage3)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnStartBench();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//-------------------------------------------------------------------------------------------------

#endif // __MYPROPERTYPAGE1_H__
