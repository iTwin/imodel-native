/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ImageInfo.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ImageInfo dialog

#pragma once

class ImageInfo : public CDialog
{
	DECLARE_DYNAMIC(ImageInfo)

public:
	ImageInfo(CWnd* pParent = NULL);   // standard constructor
	virtual ~ImageInfo();

// Dialog Data
	enum { IDD = IDD_IMAGE_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL DestroyWindow();
    virtual BOOL Create(UINT nID, CWnd* pParentWnd = NULL);
    void SetImageInfo();
    void EmptyValues();
    CString m_Type;
    CString m_Location;
    CString m_Size;
    CString m_SizeUncomp;
    CString m_Dimensions;
    CString m_TypeComp;
    CString m_ColorSpace;
    CString m_Scanline;
    CString m_BlockCount;
    CString m_BlockSize;
    CString m_GcsName;
    CString m_GcsUnits;
    CString m_DatumName;

};
