/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageInsider/MyPropertyPage4.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// MyPropertyPage4.cpp : implementation file
//

#include "stdafx.h"
#include "toolbox.h"
#include "ImageInsider.h"
#include "MyPropertyPage4.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
// CMyPropertyPage4 property page
//-----------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CMyPropertyPage4, CPropertyPage)

//-----------------------------------------------------------------------------

CMyPropertyPage4::CMyPropertyPage4() : CPropertyPage(CMyPropertyPage4::IDD)
{
	//{{AFX_DATA_INIT(CMyPropertyPage4)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//-----------------------------------------------------------------------------

CMyPropertyPage4::~CMyPropertyPage4()
{
}

//-----------------------------------------------------------------------------

void CMyPropertyPage4::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMyPropertyPage4)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CMyPropertyPage4, CPropertyPage)
	//{{AFX_MSG_MAP(CMyPropertyPage4)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// CMyPropertyPage4 message handlers
//-----------------------------------------------------------------------------

BOOL CMyPropertyPage4::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}

//-----------------------------------------------------------------------------

void CMyPropertyPage4::OnPaint() 
{	
   CPaintDC   dc(this); // device context for painting
   CDC        memdc;
   GCIPalette l_palette;
   HANDLE     hImage;
   HANDLE     hOldImage;
   
   // Get ressource bitmap
   hImage = LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP3), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_SHARED);            

   //l_palette.CreateOptimizedPaletteFromHandle(hImage, 8);

   // Create a compatible memory DC
   memdc.CreateCompatibleDC( &dc );
   
   // Set and Realize the pallete in memory dc
   memdc.SelectPalette(&l_palette, false);
   memdc.RealizePalette();

   // Set and Realize the pallete in screen dc
   dc.SelectPalette(&l_palette, false);
   dc.RealizePalette();
   
   // set the bitmap inside the memory dc 
   hOldImage = memdc.SelectObject(hImage);

   // Copy (BitBlt) bitmap from memory DC to screen DC
   dc.BitBlt( 10, 10, 390, 493, &memdc, 0, 0, SRCCOPY);
   memdc.SelectObject(hOldImage);      

   if (hImage)
      DeleteObject(hImage);

   // TODO: Add your message handler code here
	
	// Do not call CPropertyPage::OnPaint() for painting messages
}

//-----------------------------------------------------------------------------
