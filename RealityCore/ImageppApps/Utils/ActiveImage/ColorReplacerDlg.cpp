//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/ActiveImage/ColorReplacerDlg.cpp $
//:>    $RCSfile: ColorReplacerDlg.cpp,v $
//:>   $Revision: 1.3 $
//:>       $Date: 2011/07/18 21:10:25 $
//:>     $Author: Donald.Morissette $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "activeimage.h"
#include "ActiveImageDoc.h"
#include "ActiveImageView.h"
#include "ActiveImageFunctionMode.h"
#include "ColorReplacerDlg.h"
#include <Imagepp/all/h/HCDPacket.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------

ColorReplacerDlg::ColorReplacerDlg(CActiveImageDefaultMode* pi_pParent, CActiveImageView* pi_pView, CActiveImageDoc* pi_pDoc)
	: CDialog(ColorReplacerDlg::IDD, NULL),
      m_pParent(pi_pParent),
      m_pView(pi_pView),
      m_pDoc (pi_pDoc)
{
	//{{AFX_DATA_INIT(ColorReplacerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------

ColorReplacerDlg::~ColorReplacerDlg()
{

}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void ColorReplacerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ColorReplacerDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

//----------------------------------------------------------------------------
// ColorReplacerDlg message handlers
//----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(ColorReplacerDlg, CDialog)
	//{{AFX_MSG_MAP(ColorReplacerDlg)
	ON_BN_CLICKED(IDC_BT_CLOSE, OnBtClose)
	ON_BN_CLICKED(IDC_BT_CUBE_SELECT, OnBtCubeSelect)
	ON_BN_CLICKED(IDC_BT_NEW_COLOR, OnBtNewColor)
	ON_BN_CLICKED(IDC_BT_CUBE_REMOVE, OnBtCubeRemove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------

BOOL ColorReplacerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	IsColorPicking = false;
	
	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------

void ColorReplacerDlg::OnBtClose() 
{
	OnOK();
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------

void ColorReplacerDlg::PostNcDestroy() 
{
	//delete this;
	CDialog::PostNcDestroy();
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------

void ColorReplacerDlg::OnBtCubeSelect() 
{
    HFCPtr<HRABitmap> ColorSelection         = m_pParent->ColorSelection();
    if(ColorSelection)
        {
        HFCPtr<HCDPacket> BitmapPacket           = ColorSelection->GetPacket();

        uint64_t width64, height64;
        ColorSelection->GetSize(&width64, &height64);
        
        Byte *pData;
        size_t DataSize = BitmapPacket->GetDataSize();

        uint32_t pWidth  = (uint32_t)width64;
        uint32_t pHeight = (uint32_t)height64;

        uint32_t PixelItr;

        // Build an RGBCube here!
        for (uint32_t LineItr = 0; LineItr < pHeight; LineItr++)
        {
            pData = BitmapPacket->GetBufferAddress() + (LineItr * (DataSize / pHeight));

            for (PixelItr = 0; PixelItr < pWidth; PixelItr++)
            {
                m_RGBSet.Add(*(pData + 2), *(pData + 1), *(pData));
                pData += 3;
            }
        }
    }
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------

void ColorReplacerDlg::OnBtNewColor() 
{
	
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------

const HGFRGBSet& ColorReplacerDlg::GetSelectedRGBSet() const
{
    return m_RGBSet;
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------

const HGFRGBSet& ColorReplacerDlg::GetSelectedRemoveRGBSet() const
{
    return m_RGBRemoveSet;
}


//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------

void ColorReplacerDlg::OnBtCubeRemove() 
{
	HFCPtr<HRABitmap> ColorSelection         = m_pParent->ColorSelection();
    HFCPtr<HCDPacket> BitmapPacket           = ColorSelection->GetPacket();
    
    uint64_t width64, height64;
    ColorSelection->GetSize(&width64, &height64);

    Byte *pData;
    size_t DataSize = BitmapPacket->GetDataSize();

    uint32_t pWidth  = (uint32_t)width64;
    uint32_t pHeight = (uint32_t)height64;

    uint32_t PixelItr;

    // Build an RGBCube here!
    for (uint32_t LineItr = 0; LineItr < pHeight; LineItr++)
    {
        pData = BitmapPacket->GetBufferAddress() + (LineItr * (DataSize / pHeight));

        for (PixelItr = 0; PixelItr < pWidth; PixelItr++)
        {
            m_RGBRemoveSet.Add(*(pData + 2), *(pData + 1), *(pData));
            pData += 3;
        }
    }
}
