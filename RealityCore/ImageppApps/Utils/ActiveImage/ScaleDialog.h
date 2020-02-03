/*--------------------------------------------------------------------------------------+
    |
    |     $Source: Utils/ActiveImage/ScaleDialog.h $
    |
    |  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
    |
    +--------------------------------------------------------------------------------------*/
    // ScaleDialog.h : header file
    //

    /////////////////////////////////////////////////////////////////////////////


// ScaleDialog dialog

class ScaleDialog : public CDialog
{

public:
	ScaleDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_SCALE };
    double	m_scale;

    protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support


protected:
	DECLARE_MESSAGE_MAP()
};
