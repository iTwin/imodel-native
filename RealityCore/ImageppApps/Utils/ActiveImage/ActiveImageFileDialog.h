/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageFileDialog.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageFileDialog.h,v 1.3 2011/07/18 21:10:40 Donald.Morissette Exp $
//
// Class: ActiveImageFileDialog
// ----------------------------------------------------------------------------

#ifndef __ACTIVEIMAGEFILEDIALOG_h__
#define __ACTIVEIMAGEFILEDIALOG_h__

class CActiveImageFileDialog : public CFileDialog
{
	DECLARE_DYNAMIC(CActiveImageFileDialog)

public:
	CActiveImageFileDialog(BOOL bOpenFileDialog, // true for FileOpen, false for FileSaveAs
                           BOOL bShowLinkBox,
		                   LPCTSTR lpszDefExt = NULL,
		                   LPCTSTR lpszFileName = NULL,
     	                   DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
	                       LPCTSTR lpszFilter = NULL,
		                   CWnd* pParentWnd = NULL);

    // function to obtain the link (HOD interim) status of the file
    BOOL IsLinkBoxChecked() const;

protected:

    BOOL                    m_bLink;        // indicates if the file must be used as link
    BOOL                    m_bShowLinkBox; // indicates if the link box must be dsiplayed
    CButton                 m_LinkBox;
    // Buffer to store multi-selection
    HArrayAutoPtr<WChar>   m_pBufferFileNames;

    afx_msg void OnLinkButtonClicked();
	//{{AFX_MSG(CActiveImageFileDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif