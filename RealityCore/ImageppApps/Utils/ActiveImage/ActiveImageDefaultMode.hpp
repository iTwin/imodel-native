/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageDefaultMode.hpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageDefaultMode.hpp,v 1.5 2011/07/18 21:10:37 Donald.Morissette Exp $
//
// Class: ActiveImageDefaultMode
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Private
// 
//-----------------------------------------------------------------------------
inline void CActiveImageDefaultMode::OnUpdateOrder(CCmdUI* pCmdUI) 
{
    HTREEITEM        SelObject;
    BOOL             State = true;

    // Enable the command only if there is a selection
    State = !(m_Selection.empty());

    // get the currently selected object
    SelObject = m_pDoc->GetSelectedObject();

    // Force to disabled if the current object is not a mosaic
    if (SelObject != NULL && SelObject != m_pDoc->GetDocumentObject() && m_pDoc->GetRaster(SelObject) != 0)
    {
        HFCPtr<HRARaster> pRaster = m_pDoc->GetRaster(SelObject);
        HASSERT(pRaster != 0);
        if (pRaster->GetClassID() != HIMMosaic::CLASS_ID)
            State = false;
    }
    else // if there is no document selected, disable the controls
    {
        State = false;
    }

    

    pCmdUI->Enable(State);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
inline void CActiveImageDefaultMode::OnUpdateRotate(CCmdUI* pCmdUI) 
{
    BOOL State;

    // Enable the command only if there is one item in the selection
    State = m_Selection.size() == 1;

    pCmdUI->Enable(State);
}

inline void CActiveImageDefaultMode::OnUpdateRemove(CCmdUI* pCmdUI) 
    {
    CTreeCtrl* pTree = m_pDoc->GetTreeCtrl();
    HTREEITEM SelObject = m_pDoc->GetSelectedObject();

    if (SelObject != NULL && SelObject != m_pDoc->GetDocumentObject() && pTree != NULL)
        {
        HTREEITEM SelParent = pTree->GetParentItem(SelObject);

        if (SelParent != NULL && SelParent != m_pDoc->GetDocumentObject())
            {
            HFCPtr<HRARaster> pRaster = m_pDoc->GetRaster(SelParent);
            pCmdUI->Enable(pRaster->GetClassID() == HIMMosaic::CLASS_ID);
            }
        else 
            pCmdUI->Enable(false);
        }
    else
        pCmdUI->Enable(false);
    }
