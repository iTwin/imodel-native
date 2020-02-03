/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ReprojectDialog.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ReprojectDialog.cpp : implementation file

#include "stdafx.h"
#include "ActiveImage.h"
#include "ReprojectDialog.h"
#include "afxdialogex.h"


// ReprojectDialog dialog

IMPLEMENT_DYNAMIC(ReprojectDialog, CDialog)

ReprojectDialog::ReprojectDialog(CWnd* pParent)
: CDialog(ReprojectDialog::IDD, pParent), m_WktString("")
    {
    }

ReprojectDialog::~ReprojectDialog()
    {
    }

BEGIN_MESSAGE_MAP(ReprojectDialog, CDialog)
    ON_BN_CLICKED(IDC_CLEAR_WKT, &ReprojectDialog::OnBnClickedClear)
END_MESSAGE_MAP()

void ReprojectDialog::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_REPROJECT_WKT, m_WktString);
    }

BOOL ReprojectDialog::DestroyWindow()
    {
    return CDialog::DestroyWindow();
    }


BOOL ReprojectDialog::OnInitDialog()
    {
    CDialog::OnInitDialog();

    CString text(L"You can find ESRI WKT at http://spatialreference.org/ \r\n\r\nExample of ESRI WKT:"
                     " http://spatialreference.org/ref/epsg/3798/esriwkt/ \r\n\r\nWARNING! The reprojection"
                     " source GCS is always based on the LAST SELECTIONED raster!");

    GetDlgItem(IDC_EDIT_TEXT_REPROJECT)->SetWindowText(text);

    return TRUE;  // return TRUE unless you set the focus to a control
    }

CString ReprojectDialog::GetWkt()
    {
    return m_WktString;
    }

void ReprojectDialog::OnBnClickedClear()
    {
    CString emptyString = "";
    SetDlgItemText(IDC_REPROJECT_WKT, emptyString);
    }
