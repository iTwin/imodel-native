//----------------------------------------------------------------------------
//
// metadataDlg.cpp : implementation file
//
// Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "metadata.h"
#include "metadataDlg.h"
#include "MetaDataGrid.h"

#include <PointoolsVortexAPI_DLL/PTAPI/PointoolsVortexAPI_import.h>
//#include "../lic/vortexLicense.c"

#include <shlwapi.h>

// for Pointools Internal Use, please comment out
#ifdef _DEBUG
#define PT_DEBUG
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMetaDataDlg dialog

CMetaDataDlg::CMetaDataDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMetaDataDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMetaDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDT_FILENAME, m_edtFilename);
	DDX_Control(pDX, IDC_GRID, m_grid);
}

BEGIN_MESSAGE_MAP(CMetaDataDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_OPEN_FILE, &CMetaDataDlg::OnBnClickedBtnOpenFile)
	ON_BN_CLICKED(IDC_SAVE, &CMetaDataDlg::OnBnClickedSave)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CMetaDataDlg::OnBnClickedBtnSave)
END_MESSAGE_MAP()


// CMetaDataDlg message handlers

BOOL CMetaDataDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMetaDataDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMetaDataDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
/********************************************/
/*   Display Meta data in Tree				*/
/********************************************/
void CMetaDataDlg::DisplayPODMetaData( CString filename )
{	
	if (InitializeVortexAPI())
	{
		// Read the Metadata from an open POD file
		/*

		PThandle h = ptOpenPOD( filename.GetBuffer() );
		PThandle metaHandle = ptGetMetaDataHandle( h );

		// populate the grid
		m_grid.Populate( metaHandle );

		ptFreeMetaData( h );

		*/

		// Read meta data from a file directly (uncomment below)		
		PThandle h = ptReadPODMeta( filename.GetBuffer() );

		m_grid.Populate( h );

		// Free the meta data store in the Vortex Engine		
		//ptFreeMetaData( h ); - don't free because we want to edit it and resave
	}
}
/********************************************/
/*   Initialize Vortex API				    */
/********************************************/
void CMetaDataDlg::OnBnClickedBtnOpenFile()
{
	/* pick file with browser  */ 
	CFileDialog fileDialog(TRUE,NULL,NULL,OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		_T("Pointools POD file(*.pod)|*.pod||"));

	if (IDOK == fileDialog.DoModal())
	{
		CString fileName = fileDialog.GetPathName();
		m_edtFilename.SetWindowText( fileName.GetBuffer() );
		
		DisplayPODMetaData( fileName );
	}
}

/********************************************/
/*   Initialize Vortex API				    */
/********************************************/
bool CMetaDataDlg::InitializeVortexAPI()
{
	extern bool LoadPointoolsDLL(const wchar_t*filepath);

	wchar_t folder[260];
	wchar_t apiFile[260];
	GetModuleFileName(NULL, folder, 260);
	::PathRemoveFileSpecW(folder);

#ifdef PT_DEBUG
	swprintf_s(apiFile, 260, L"%s\\PointoolsVortexAPId.dll", folder);
#else
	swprintf_s(apiFile, 260, L"%s\\PointoolsVortexAPI.dll", folder);
#endif

	static bool isVortexLoaded = false;

	if (isVortexLoaded) return true;

	if (LoadPointoolsDLL( apiFile ))
	{
		PTubyte version[4];
		ptGetVersionNum(version);

		if (ptInitialize( vortexLicCode ) == PT_FALSE)
		{
			MessageBox( L"Failed to initialise Vortex API", L"API Error", MB_OK );
			MessageBox( ptGetLastErrorString(), L"Load Error", MB_OK );
		}
	
		ptSetWorkingFolder( folder );
		ptSetAutoBaseMethod( PT_AUTO_BASE_CENTER );
		ptFlipMouseYCoords();
		
		/* single viewport, no multiple viewport handling in this example */ 
		ptSetViewport( 0 );
		isVortexLoaded = true;
	}
	else
	{
		extern const char*DLLLoadErrorMessage();

		MessageBox( L"Error loading PointoolsVortexAPI.dll", L"API Error", MB_OK );

		const char* err = DLLLoadErrorMessage();
		if (err)
		MessageBoxA( 0, err, "API Load Error", MB_OK );
	}
	return isVortexLoaded;
}

void CMetaDataDlg::OnBnClickedSave()
{
	// read data from grid and update meta tags
	//m_grid.get

	// save changes back to file

}

void CMetaDataDlg::OnBnClickedBtnSave()
{
	// meta data already updated, just need to save
	ptWriteMetaTags( m_grid.m_metaHandle );
}
