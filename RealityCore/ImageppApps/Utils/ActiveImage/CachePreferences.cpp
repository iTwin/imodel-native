/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/CachePreferences.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// CachePreferences.cpp : implementation file
//

#include "stdafx.h"
#include "activeimage.h"
#include "CachePreferences.h"
#include <Imagepp/all/h/HRFCacheFileCreator.h>
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HUTClassIDDescriptor.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CCachePreferencesDlg dialog


CCachePreferencesDlg::CCachePreferencesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCachePreferencesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCachePreferencesDlg)
	//}}AFX_DATA_INIT
}


void CCachePreferencesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCachePreferencesDlg)
	DDX_Control(pDX, IDC_COMBO_CACHEFORMAT, m_CacheFormatCombo);
	DDX_Control(pDX, IDC_COMBO_CODEC, m_CodecCombo);
	DDX_Control(pDX, IDC_COMBO_PIXELTYPE, m_PixelTypeCombo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCachePreferencesDlg, CDialog)
	//{{AFX_MSG_MAP(CCachePreferencesDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_PIXELTYPE, OnSelchangeComboPixeltype)
	ON_CBN_SELCHANGE(IDC_COMBO_CODEC, OnSelchangeComboCodec)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


#define  MEMORY_OBJECTSTORE_MEM     "ObjectStoreLog_Mem"
#define  DEF_OBJECTSTORE_MEM        16192
#define  MEMORY_BUFFERIMG_MEM       "BufferImgLog_Mem"
#define  DEF_BUFFERIMG_MEM          8096

/////////////////////////////////////////////////////////////////////////////
BOOL CCachePreferencesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
    m_pCacheFileCreator = HRFiTiffCacheFileCreator::GetInstance();

    // Reload the Profile ...
    CWinApp* pApp = AfxGetApp();
    
    for (uint32_t Index=0; Index<m_pCacheFileCreator->CountPixelType(); Index++)
    {
        WChar VarName[1024];

         _stprintf(VarName, _TEXT("PixelType(%ld)"), m_pCacheFileCreator->GetPixelType(Index));

        uint32_t CodecID = pApp->GetProfileInt (_TEXT("CacheSetting"), 
                                             VarName, 
                                             m_pCacheFileCreator->GetSelectedCodecFor(m_pCacheFileCreator->GetPixelType(Index)));
               
        m_pCacheFileCreator->SelectCodecFor(m_pCacheFileCreator->GetPixelType(Index), 
                                            CodecID);

        pApp->WriteProfileInt (_TEXT("CacheSetting"), 
                               VarName, 
                               m_pCacheFileCreator->GetSelectedCodecFor(m_pCacheFileCreator->GetPixelType(Index)));
    }
    m_CacheFormatCombo.SetCurSel(0);

    int     ItemIndex;

    // Color Space
    m_PixelTypeCombo.ResetContent();

    // Fill with the pixel type
    for (uint32_t Index=0; Index<m_pCacheFileCreator->CountPixelType(); Index++)
    {
        WString labelW;
        labelW.AssignUtf8(HUTClassIDDescriptor::GetInstance()->GetClassLabelPixelType(m_pCacheFileCreator->GetPixelType(Index)).c_str());
        ItemIndex = m_PixelTypeCombo.AddString(labelW.c_str());
        
        m_PixelTypeCombo.SetItemData(ItemIndex, m_pCacheFileCreator->GetPixelType(Index));
    }

    m_PixelTypeCombo.SetCurSel(0);


    // Fill with the codecs list
    m_CodecCombo.ResetContent();

    HCLASS_ID Codec;
    HCLASS_ID SelCodec = m_pCacheFileCreator->GetSelectedCodecFor(m_pCacheFileCreator->GetPixelType(m_PixelTypeCombo.GetCurSel()));

    for (uint32_t Index=0; Index<m_pCacheFileCreator->CountCodecsFor(m_pCacheFileCreator->GetPixelType(m_PixelTypeCombo.GetCurSel())); Index++)
    {
        // Get the current codec for this pixel type
        Codec = m_pCacheFileCreator->GetCodecFor(m_pCacheFileCreator->GetPixelType(m_PixelTypeCombo.GetCurSel()), Index);

        WString labelW;
        labelW.AssignUtf8(HUTClassIDDescriptor::GetInstance()->GetClassLabelCodec(Codec).c_str());
        ItemIndex = m_CodecCombo.AddString(labelW.c_str());

        m_CodecCombo.SetItemData(ItemIndex, Codec);

        if (SelCodec == Codec)
            m_CodecCombo.SetCurSel(Index);
    }	

    return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}


// CCachePreferencesDlg message handlers

void CCachePreferencesDlg::OnSelchangeComboPixeltype() 
{
    // Fill with the codecs list
    m_CodecCombo.ResetContent();

    int     ItemIndex;
    HCLASS_ID Codec;
    HCLASS_ID SelCodec = m_pCacheFileCreator->GetSelectedCodecFor(m_pCacheFileCreator->GetPixelType(m_PixelTypeCombo.GetCurSel()));

    for (uint32_t Index=0; Index<m_pCacheFileCreator->CountCodecsFor(m_pCacheFileCreator->GetPixelType(m_PixelTypeCombo.GetCurSel())); Index++)
    {
        Codec = m_pCacheFileCreator->GetCodecFor(m_pCacheFileCreator->GetPixelType(m_PixelTypeCombo.GetCurSel()), Index);
        WString labelW;
        labelW.AssignUtf8(HUTClassIDDescriptor::GetInstance()->GetClassLabelCodec(Codec).c_str());

        ItemIndex = m_CodecCombo.AddString(labelW.c_str());
        m_CodecCombo.SetItemData(ItemIndex, Codec);
        if (SelCodec == Codec)
            m_CodecCombo.SetCurSel(Index);
    }	
}

void CCachePreferencesDlg::OnSelchangeComboCodec() 
{
    m_pCacheFileCreator->SelectCodecFor((HCLASS_ID)m_PixelTypeCombo.GetItemData(m_PixelTypeCombo.GetCurSel()), 
                                        (HCLASS_ID)m_CodecCombo.GetItemData(m_CodecCombo.GetCurSel()));
    // Update Profile ...
    CWinApp* pApp = AfxGetApp();
    WChar VarName[1024];
    _stprintf(VarName, _TEXT("PixelType(%lld)"), (int64_t)m_PixelTypeCombo.GetItemData(m_PixelTypeCombo.GetCurSel()));
    pApp->WriteProfileInt (_TEXT("CacheSetting"), 
                           VarName, 
                           (int)m_CodecCombo.GetItemData(m_CodecCombo.GetCurSel()));

	
}
