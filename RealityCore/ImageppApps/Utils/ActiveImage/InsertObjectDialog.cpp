/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/InsertObjectDialog.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/InsertObjectDialog.cpp,v 1.3 2011/07/18 21:10:30 Donald.Morissette Exp $
//
// Class: InsertObjectDialog
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "ActiveImage.h"
#include "InsertObjectDialog.h"


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
// Message Map and other MFC Macros
//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CInsertObjectDialog, CDialog)
	//{{AFX_MSG_MAP(CInsertObjectDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
CInsertObjectDialog::CInsertObjectDialog(const char* pi_pTitle,
                                         CActiveImageDoc* pi_pDoc, 
                                         CWnd* pParent /*=NULL*/)
	: CDialog(CInsertObjectDialog::IDD, pParent)
{
    HPRECONDITION(pi_pTitle != 0);

	//{{AFX_DATA_INIT(CInsertObjectDialog)
	m_Linked = false;
	//}}AFX_DATA_INIT

    m_pDoc        = pi_pDoc;
    m_DialogTitle = pi_pTitle;
    m_NumMosaic = 0;
    m_NumImages = 0;
}



//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
CInsertObjectDialog::~CInsertObjectDialog()
{
    m_RasterMap.clear();
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CInsertObjectDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsertObjectDialog)
	DDX_Control(pDX, IDC_OBJECT_LIST, m_ObjectList);
	DDX_Check(pDX, IDC_LINK, m_Linked);
	//}}AFX_DATA_MAP
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
BOOL CInsertObjectDialog::OnInitDialog() 
{
    HPRECONDITION(!m_DialogTitle.IsEmpty());

    // call the base class
	CDialog::OnInitDialog();

    // set the window title
    SetWindowText(m_DialogTitle);

    // Prepare the tree list
    m_ObjectList.SetImageList(m_pDoc->GetTreeCtrl()->GetImageList(TVSIL_NORMAL), TVSIL_NORMAL);

    // Hide the link box (Hey, its already in the document!)
    CRect LinkRect;
    GetDlgItem(IDC_LINK)->GetWindowRect(&LinkRect);
    GetDlgItem(IDC_LINK)->ShowWindow(SW_HIDE);
    CRect DialogRect;
    GetWindowRect(&DialogRect);
    DialogRect.bottom -= LinkRect.Height();
    MoveWindow(DialogRect);

    // Generate the list tree
    GenerateFromDocument();
    
	return true;  // return true unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return false
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CInsertObjectDialog::OnOK() 
{
    HTREEITEM SelectedItem;

    // get the selected raster and verify that it is not the document
    SelectedItem =  m_ObjectList.GetSelectedItem();
    if (SelectedItem != m_ObjectList.GetRootItem())
    {
        // Obtain the iterator on the selected item
        CActiveImageDoc::RasterMap::iterator Itr = m_RasterMap.find(SelectedItem);
        HASSERT(Itr != m_RasterMap.end());

        // get the raster from the found iterator
        m_pSelectedRaster = (*Itr).second;
        HASSERT(m_pSelectedRaster != 0);

        // End the dialog
    	CDialog::OnOK();
    }
    else
        AfxMessageBox(_TEXT("Cannot insert the document item"));
}



//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CInsertObjectDialog::InsertRasterFromFile(HFCPtr<HRARaster> pi_pRaster, 
                                               HTREEITEM         pi_ParentItem)
{
    WChar     TreeLabel[255];
    HTREEITEM  NewItem;
    HPRECONDITION(pi_pRaster != 0);
    HPRECONDITION(pi_ParentItem!= NULL);

    // Generate the label for the tree item
    if (pi_pRaster->GetClassID() == HIMMosaic::CLASS_ID)
        _stprintf(TreeLabel, _TEXT("Mosaic %d"), m_NumMosaic++);
    else
        _stprintf(TreeLabel, _TEXT("Image %d"), m_NumImages++);

    // Add the new item in the tree list
    if (pi_pRaster->GetClassID() == HIMMosaic::CLASS_ID)
        NewItem = m_ObjectList.InsertItem(TreeLabel, 8, 9, pi_ParentItem);
    else
        NewItem = m_ObjectList.InsertItem(TreeLabel, 4, 5, pi_ParentItem);
    
    HASSERT(NewItem != NULL);
       
    // Add the raster in the raster map
    m_RasterMap.insert(CActiveImageDoc::RasterMap::value_type(NewItem, pi_pRaster));

    // if the raster is a mosaic, insert also its source
    if (pi_pRaster->GetClassID() == HIMMosaic::CLASS_ID)
    {
        HFCPtr<HIMMosaic>& pMosaic = (HFCPtr<HIMMosaic>&) pi_pRaster;
        HIMMosaic::IteratorHandle Iterator = pMosaic->StartIteration();
        for (uint32_t i = 0; i < pMosaic->CountElements(Iterator); i++)
        {
            // get the current element
            HFCPtr<HRARaster> pRaster = pMosaic->GetElement(Iterator);
            HASSERT(pRaster != 0);

            // Inser the current source
            InsertRasterFromFile(pRaster, NewItem);

            // get the next raster
            pMosaic->Iterate(Iterator);
        }
        pMosaic->StopIteration(Iterator);
    }

    // expand the parent
    m_ObjectList.Expand(pi_ParentItem, TVE_EXPAND);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CInsertObjectDialog::GenerateFromDocument()
{
    CTreeCtrl* pTree;
    HTREEITEM  DocItem;
    HTREEITEM  NewItem;
    CString    ItemLabel;
    int32_t     nImage;
    int32_t     nSelectedImage;

    // Get the tree control from the document item
    pTree = m_pDoc->GetTreeCtrl();
    HASSERT(pTree != 0);
    
    // copy the document item
    DocItem = pTree->GetRootItem();
    ItemLabel = pTree->GetItemText(DocItem);
    pTree->GetItemImage(DocItem, nImage, nSelectedImage);

    // create a new item in our tree
    NewItem = m_ObjectList.InsertItem(ItemLabel, nImage, nSelectedImage);
    HASSERT(NewItem != NULL);

    // copy all the children of the source item
    if (pTree->ItemHasChildren(DocItem))
    {
        for (HTREEITEM CurrentItem = pTree->GetChildItem(DocItem);
             CurrentItem != NULL;
             CurrentItem = pTree->GetNextSiblingItem(CurrentItem))
        {
            CopyItemfromDocument(CurrentItem, NewItem);
        }

        // expand the parent
        m_ObjectList.Expand(NewItem, TVE_EXPAND);
    }
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CInsertObjectDialog::CopyItemfromDocument(HTREEITEM pi_SourceItem, 
                                               HTREEITEM pi_DestItem)
{
    CTreeCtrl* pTree;
    CString           ItemLabel;
    HFCPtr<HRARaster> pRaster;
    HTREEITEM         NewItem;
    int32_t            nImage;
    int32_t            nSelectedImage;

    // get the source tree
    pTree = m_pDoc->GetTreeCtrl();
    HASSERT(pTree != 0);

    // Get the source item's information
    //
    //  1. The Label
    //  2. The associated raster
    //  3. Tree list icons
    ItemLabel = pTree->GetItemText(pi_SourceItem);
    pTree->GetItemImage(pi_SourceItem, nImage, nSelectedImage);
    pRaster = m_pDoc->GetRaster(pi_SourceItem);
    HASSERT(pRaster != 0);

    // Create the destination
    //
    NewItem = m_ObjectList.InsertItem(ItemLabel, nImage, nSelectedImage, pi_DestItem);
    HASSERT(NewItem != NULL);
    
    // insert the raster in the raster map
    m_RasterMap.insert(CActiveImageDoc::RasterMap::value_type(NewItem, pRaster));

    // do the same for its children
    if (pTree->ItemHasChildren(pi_SourceItem))
    {
        for (HTREEITEM CurrentItem = pTree->GetChildItem(pi_SourceItem);
             CurrentItem != NULL;
             CurrentItem = pTree->GetNextSiblingItem(CurrentItem))
        {
            CopyItemfromDocument(CurrentItem, NewItem);
        }

        // expand the parent
        m_ObjectList.Expand(NewItem, TVE_EXPAND);
    }
}
