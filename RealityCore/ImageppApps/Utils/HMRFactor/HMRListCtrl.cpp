/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HMRFactor/HMRListCtrl.cpp $
|    $RCSfile: HMRListCtrl.cpp,v $
|   $Revision: 1.2 $
|       $Date: 2006/02/16 21:24:33 $
|     $Author: BENTLEY\Simon.Normand $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "HMRListCtrl.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
HMRListCtrl::HMRListCtrl ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
HMRListCtrl::~HMRListCtrl ()
    {
    }

BEGIN_MESSAGE_MAP(HMRListCtrl, CListCtrl)
    //{{AFX_MSG_MAP(HMRListCtrl)
        ON_WM_DROPFILES()
        ON_WM_KEYDOWN()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRListCtrl::OnKeyDown (UINT nChar, UINT nRepCnt, UINT nFlags)
    {
    switch (nChar)
        {
        case VK_DECIMAL:
        case VK_DELETE:
            RemoveSelected ();
            break;
        case 'A':
            {
            // Select all items in the list if Ctrl+A is pressed
            if(GetKeyState(VK_CONTROL) < 0)
                {
                SetItemState (-1, LVIS_SELECTED, LVIS_SELECTED);
                }
            }
            break;
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRListCtrl::OnDropFiles(HDROP hDropInfo)
    {
    UINT  uNumFiles;
    TCHAR szNextFile [MAX_PATH];

    // Get the # of files being dropped.
    uNumFiles = DragQueryFile ( hDropInfo, -1, NULL, 0 );

    for ( UINT uFile = 0; uFile < uNumFiles; uFile++ )
        {
        // Get the next filename from the HDROP info.
        if ( DragQueryFile ( hDropInfo, uFile, szNextFile, MAX_PATH ) > 0 )
            {
            CString filename (szNextFile);

            AddFilename (filename);
            }
        }

    // Free up memory.
    DragFinish ( hDropInfo );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRListCtrl::AddFilename (CString& filename)
    {
    LVFINDINFO info;

    info.flags = LVFI_STRING;
    info.psz = filename;

    if (-1 == FindItem (&info))
        {
        InsertItem (GetItemCount (), filename, 0);
        SetItemData (GetItemCount ()-1, NEWFILE);
        // Auto size the width of the column
        SetColumnWidth (0,LVSCW_AUTOSIZE);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRListCtrl::RemoveSelected ()
    {
    UINT uSelectedCount = GetSelectedCount();

    UINT* pSelectionArray = new UINT [uSelectedCount];
    int  nItem = -1;

    // Get selected items.
    for (UINT i = 0; i < uSelectedCount; i++)
        {
        nItem = GetNextItem(nItem, LVNI_SELECTED);
        pSelectionArray [i] = nItem;
        }

    // delete the items in inverse order
    for (UINT i = uSelectedCount; i > 0 ; i--)
        {
        DeleteItem (pSelectionArray [i - 1]);
        }

    delete[] pSelectionArray;

    // Auto size the width of the column
    SetColumnWidth (0,LVSCW_AUTOSIZE);
    }
