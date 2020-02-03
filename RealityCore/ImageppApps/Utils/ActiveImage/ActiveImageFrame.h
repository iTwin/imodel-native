/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageFrame.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ActiveImageFrame.h : interface of the CActiveImageFrame class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __ACTIVEIMAGEFRAME_H__
#define __ACTIVEIMAGEFRAME_H__


#define INDEX_INDICATOR_DRAWTIME         1
#define INDEX_INDICATOR_MOUSEPOSITION    2
#define INDEX_INDICATOR_PROGRESSION      3
#define INDEX_INDICATOR_EXPORTEDFILESIZE 4 //Estimation


class CActiveImageFrame : public CFrameWnd
{
protected: // create from serialization only
	CActiveImageFrame();
	DECLARE_DYNCREATE(CActiveImageFrame)

// Attributes
public:
    CImageList  m_TreeListIcons;
    CToolBar    m_FileToolBar;
    CDialogBar  m_ObjectToolBar;
    CStatusBar  m_wndStatusBar;

// Operations
public:
    CTreeCtrl* GetSelector();
    void       DockControlBarLeftOf(CToolBar* Bar,CToolBar* LeftOf);
    void       GetMessageString(UINT nID, CString& rMessage) const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CActiveImageFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CActiveImageFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
    afx_msg void OnSelectedObjectChanged(NMHDR * pNotifyStruct, LRESULT * result);
    afx_msg BOOL OnToolTip(UINT nID, NMHDR* pNMHDR, LRESULT* pResult);

	//{{AFX_MSG(CActiveImageFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewFiletoolbar();
	afx_msg void OnUpdateViewFiletoolbar(CCmdUI* pCmdUI);
	afx_msg void OnViewObjecttoolbar();
	afx_msg void OnUpdateViewObjecttoolbar(CCmdUI* pCmdUI);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif