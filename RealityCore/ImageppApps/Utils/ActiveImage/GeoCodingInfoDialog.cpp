/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/GeoCodingInfoDialog.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-------------------------------------------------------------------
// GeoCodingInfoDialog.cpp : implementation file
//-------------------------------------------------------------------

#include "stdafx.h"
#include "activeimage.h"
#include "GeoCodingInfoDialog.h"

#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HFCURLFile.h>

#include <Imagepp/all/h/HRFUtility.h>
#include <ImagePP/all/h/HCPGeoTiffKeys.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-------------------------------------------------------------------
// 
//-------------------------------------------------------------------

GeoCodingInfoDialog::GeoCodingInfoDialog(HFCPtr<HRFRasterFile >& pi_prRasterFile)
	: CDialog(GeoCodingInfoDialog::IDD, 0),
      m_prRasterFile(pi_prRasterFile)
{
	//{{AFX_DATA_INIT(GeoCodingInfoDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void GeoCodingInfoDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GeoCodingInfoDialog)
	DDX_Control(pDX, IDC_BT_SAVE, m_BtSave);
	DDX_Control(pDX, IDC_EDIT_GEOKEYS, m_EditGeoKeys);
	DDX_Control(pDX, IDC_EDIT_TAG_VALUE, m_TagValue);
	DDX_Control(pDX, IDC_CMB_TAG_LIST, m_CmbSupportedTag);
	DDX_Control(pDX, IDC_GEOCODING_INFO_LIST, m_GeoCodingInfoList);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------
// GeoCodingInfoDialog message handlers
//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(GeoCodingInfoDialog, CDialog)
	//{{AFX_MSG_MAP(GeoCodingInfoDialog)
	ON_CBN_SELCHANGE(IDC_CMB_TAG_LIST, OnSelchangeCmbTagList)
	ON_BN_CLICKED(IDC_BT_SAVE, OnBtSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------


BOOL GeoCodingInfoDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	FeedTagListCtrl();

    m_EditGeoKeys.SetReadOnly (true);
    m_EditGeoKeys.EnableWindow(false);

    if (!m_CmbSupportedTag.GetCount())
    {
        m_CmbSupportedTag.AddString(_TEXT("Not available"));
        m_CmbSupportedTag.EnableWindow(false);
        m_BtSave.EnableWindow(false);
    }
    
    GeoCoordinates::BaseGCSCP pFileGeocoding = m_prRasterFile->GetPageDescriptor(0)->GetGeocodingCP();

    if (pFileGeocoding != NULL)
        {
        HCPGeoTiffKeys geoKeyList;
        pFileGeocoding->GetGeoTiffKeys(&geoKeyList, true);

        WString geoKeyString;

        GeoCoordinates::IGeoTiffKeysList::GeoKeyItem KeyItem;
        if (geoKeyList.GetFirstKey(&KeyItem))
        {
            TCHAR Msg[4096];
            do {
                if (KeyItem.KeyDataType == GeoCoordinates::IGeoTiffKeysList::LONG)
                    _stprintf(Msg, _TEXT("[%6d : long  : %lu] - "), KeyItem.KeyID, KeyItem.KeyValue.LongVal);
                else if (KeyItem.KeyDataType == GeoCoordinates::IGeoTiffKeysList::DOUBLE)
                    _stprintf(Msg, _TEXT("[%6d : double : %lf] -" ), KeyItem.KeyID, KeyItem.KeyValue.DoubleVal);
                else if (KeyItem.KeyDataType == GeoCoordinates::IGeoTiffKeysList::ASCII)
                {
                    WString TmpStr;
                    BeStringUtilities::CurrentLocaleCharToWChar(TmpStr, KeyItem.KeyValue.StringVal);

                    _stprintf(Msg,
                              _TEXT("[%6d : ascii  : \"%s\"] - "),
                              KeyItem.KeyID,
                              TmpStr.c_str());
                }
                else
                {
                    Msg[0] = 0;
                    HASSERT(false);
                }
                geoKeyString += Msg;
            }
            while(geoKeyList.GetNextKey(&KeyItem));
        }
        m_EditGeoKeys.SetWindowText(geoKeyString.c_str());
        m_EditGeoKeys.EnableWindow(true);
        m_BtSave.EnableWindow(true);
    }
    else
    {
        m_EditGeoKeys.SetWindowText(_TEXT("IT'S NOT GEOCODED!"));
        m_BtSave.EnableWindow(false);
    }

    m_CmbSupportedTag.SetCurSel(0);
    OnSelchangeCmbTagList();
	
	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}


//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void GeoCodingInfoDialog::FeedTagListCtrl()
    {
    m_GeoCodingInfoList.InsertColumn( 1 ,_TEXT("Tag")   , LVCFMT_LEFT,  140);
    m_GeoCodingInfoList.InsertColumn( 2 ,_TEXT("Access"), LVCFMT_LEFT,   60);
    m_GeoCodingInfoList.InsertColumn( 3 ,_TEXT("Value") , LVCFMT_LEFT,  200);

    // Don't take chance!
    m_GeoCodingInfoList.DeleteAllItems();

    // Fill list with capabilities, if a tag is present in file, add its value
    HPMAttributeSet TagList;

    HFCPtr<HRFCapability> pTagCap;
    HFCPtr<HPMGenericAttribute> pTag;

    HFCPtr<HRFRasterFileCapabilities>  TagCapabilities; 
    TagCapabilities = m_prRasterFile->GetCapabilities()->GetCapabilitiesOfType(HRFTagCapability::CLASS_ID, HFC_NO_ACCESS);                
    // If some tags are supported
    if (TagCapabilities)
        {
        // Loop in all supported tags
        for (uint32_t IndexTag=0; IndexTag < TagCapabilities->CountCapabilities(); IndexTag++)
            {
            HFCPtr<HRFTagCapability> pTagCapability;
            pTagCapability = ((HFCPtr<HRFTagCapability>&)TagCapabilities->GetCapability(IndexTag));
            Utf8String Tag = pTagCapability->GetTag()->GetName();

            // Add tag access mode(s)
            WString TagAccessMode;

            HFCPtr<HRFCapability> pReadCapability   = new HRFTagCapability(HFC_READ_ONLY, pTagCapability->GetTag());
            HFCPtr<HRFCapability> pWriteCapability  = new HRFTagCapability(HFC_WRITE_ONLY, pTagCapability->GetTag());
            HFCPtr<HRFCapability> pCreateCapability = new HRFTagCapability(HFC_CREATE_ONLY, pTagCapability->GetTag());

            if (TagCapabilities->Supports(pReadCapability))
                TagAccessMode = _TEXT("R");

            if (TagCapabilities->Supports(pWriteCapability))
                TagAccessMode = TagAccessMode + _TEXT("W");

            if (TagCapabilities->Supports(pCreateCapability))
                TagAccessMode = TagAccessMode + _TEXT("C");

            // Check if tag is present in current raster file instance (scan page tags)
            HPMAttributeSet  TagList(m_prRasterFile->GetPageDescriptor(0)->GetTags());
            HPMAttributeSet::HPMASiterator TagIterator;

            bool FoundTagInFile = false;
            CString TagValue;

            for (TagIterator  = TagList.begin(); TagIterator != TagList.end(); TagIterator++)
                {   
                if (((*TagIterator)->GetName()) == Tag) 
                    {
                    TagValue = (*TagIterator)->GetDataAsString().c_str();
                    FoundTagInFile = true;
                    break;
                    }
                }

            WString TagW; TagW.AssignUtf8(Tag.c_str());

            if (FoundTagInFile)
                {
                // Add tag label to list of supported tags
                int NewItemIndex = m_GeoCodingInfoList.InsertItem(m_GeoCodingInfoList.GetItemCount(), TagW.c_str());

                m_GeoCodingInfoList.SetItem( NewItemIndex,  1, LVIF_TEXT, TagAccessMode.c_str(), 0, 0, 0, 0);
                m_GeoCodingInfoList.SetItem( NewItemIndex,  2, LVIF_TEXT, TagValue, 0, 0, 0, 0);
                }
            else
                {
                BOOL MaybeAdded = TagCapabilities->Supports(pWriteCapability) || TagCapabilities->Supports(pCreateCapability);

                m_CmbSupportedTag.SetItemData(m_CmbSupportedTag.AddString(TagW.c_str()), MaybeAdded);
                // m_CmbSupportedTag.AddString(Tag.c_str());
                }

            }
        }
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void GeoCodingInfoDialog::OnSelchangeCmbTagList() 
{
	 DWORD_PTR MaybeAdded = m_CmbSupportedTag.GetItemData(m_CmbSupportedTag.GetCurSel());
     m_TagValue.EnableWindow(MaybeAdded != 0);

     if (!MaybeAdded)
        m_TagValue.SetWindowText(_TEXT("Sorry, this is a read-only tag."));
     else
        m_TagValue.SetWindowText(_TEXT(""));
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void GeoCodingInfoDialog::OnBtSave() 
{
    WString FileName;
    FileName.AssignUtf8(((HFCPtr <HFCURLFile>&)(m_prRasterFile->GetURL()))->GetFilename().c_str());
    FileName += _TEXT(".GeoKey.Txt");
    
    CFileDialog MySaveDialog( false, _TEXT("GeoKey Txt"), FileName.c_str(), OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT, _TEXT("GeoKey Txt (*.GeoKey.Txt)||"), this );
    if (MySaveDialog.DoModal() == IDOK)
    {
        CString DumpFileName = MySaveDialog.GetPathName();
        CString GeoKeys;

        FILE* pDumpFile = _tfopen(DumpFileName, _TEXT("wt"));

        if (pDumpFile)
        {
            m_EditGeoKeys.GetWindowText(GeoKeys);

            if (!GeoKeys.IsEmpty())
                fwrite((LPCTSTR)GeoKeys, sizeof(TCHAR), GeoKeys.GetLength(), pDumpFile);

            fclose(pDumpFile);
        }
    }
}
