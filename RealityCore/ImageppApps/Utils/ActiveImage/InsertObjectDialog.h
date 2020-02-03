/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/InsertObjectDialog.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef __INSERTOBJECTDIALOG__
#define __INSERTOBJECTDIALOG__

#include <Imagepp/all/h/HRARaster.h>
#include "ActiveImageDoc.h"

class CInsertObjectDialog : public CDialog
{
// Construction
public:
    CInsertObjectDialog(const char*     pi_pTitle,
                        CActiveImageDoc* pi_pDoc, 
                        CWnd* pParent = NULL);
    ~CInsertObjectDialog();

// Dialog Data
	//{{AFX_DATA(CInsertObjectDialog)
	enum { IDD = IDD_INSERT_OBJECT };
	CTreeCtrl	m_ObjectList;
	BOOL	m_Linked;
	//}}AFX_DATA

    HFCPtr<HRARaster> m_pSelectedRaster;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInsertObjectDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    CActiveImageDoc* m_pDoc;
    CString          m_DialogTitle;
    uint32_t          m_NumMosaic;
    uint32_t          m_NumImages;


    // RasterMap is used to store rasters with their associated tree item handle
    CActiveImageDoc::RasterMap m_RasterMap;

    void InsertRasterFromFile(HFCPtr<HRARaster> pi_pRaster, 
                              HTREEITEM         NewItem);
    void GenerateFromDocument();
    void CopyItemfromDocument(HTREEITEM SourceItem, 
                              HTREEITEM DestItem);

	// Generated message map functions
	//{{AFX_MSG(CInsertObjectDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif