/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageDefaultMode.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageDefaultMode.h,v 1.2 2005/02/08 18:35:28 SebastienGosselin Exp $
//-----------------------------------------------------------------------------
// Class : CActiveImageDefaultMode
//-----------------------------------------------------------------------------

#ifndef __CACTIVEIMAGEDEFAULTMODE_H__
#define __CACTIVEIMAGEDEFAULTMODE_H__

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "ActiveImageFunctionMode.h"
#include <Imagepp/all/h/HRPFunctionFilters.h>
#include <Imagepp/all/h/HRPDEMFilter.h>

class CActiveImageDoc;
class CActiveImageView;

#define CActiveImageDefaultMode_FilteringLevel  5

//-----------------------------------------------------------------------------
// Class Declaration
//-----------------------------------------------------------------------------
class CActiveImageDefaultMode : public CActiveImageFunctionMode
{
    public:

        ////////////////////////////////////////
        // Construction/Destruction
        ////////////////////////////////////////
                     CActiveImageDefaultMode();
        virtual      ~CActiveImageDefaultMode();


        typedef vector<HFCPtr<HRARaster>, allocator<HFCPtr<HRARaster> > > 
            RasterSelection;
        ////////////////////////////////////////
        // Toolbar and menu Methods
        ////////////////////////////////////////

        // These static functions will modify the
        // menu and add a toolbar.
        static bool    SetupMenu(CMenu* pi_pEditMenu, CMenu* pi_pViewMenu);
        static CToolBar* 
                        SetupToolbar(CActiveImageFrame* pi_pFrame,
                                     uint32_t            pi_ID);

        
        ////////////////////////////////////////
        // Basic Methods
        ////////////////////////////////////////
        virtual uint32_t GetType() const;
        virtual void    OnDraw(CDC* pDC);
        virtual void    OnUndraw(CDC* pDC);
        virtual void    Setup();
        void RemoveFromSelection(const HFCPtr<HRARaster>& pi_pRaster);
        vector<HFCPtr<HRARaster>, allocator<HFCPtr<HRARaster> > >  GetSelection();


        ////////////////////////////////////////
        // Mouse Commands
        ////////////////////////////////////////
        virtual void    OnLButtonDblClk (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint);
        virtual void    OnLButtonDown   (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint);
        virtual void    OnLButtonUp     (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint);
        virtual void    OnRButtonDblClk (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint);
        virtual void    OnRButtonDown   (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint);
        virtual void    OnRButtonUp     (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint);
        virtual void    OnMouseMove     (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint);

        ////////////////////////////////////////
        // Keyboard Commands
        ////////////////////////////////////////
        virtual void    OnKeyDown(uint32_t pi_Char, 
                                  uint32_t pi_RepeatCount, 
                                  uint32_t pi_Flags);
        virtual void    OnKeyUp  (uint32_t pi_Char, 
                                  uint32_t pi_RepeatCount, 
                                  uint32_t pi_Flags);
    

        ////////////////////////////////////////
        // WindowProc, for trapping messages
        ////////////////////////////////////////

        // The follwing methods respond to commands while
        // the object is active
        virtual bool   OnCommand(uint32_t pi_CommandID,
                                       CActiveImageView* pi_pView,
                                       CActiveImageDoc*  pi_pDoc);
        virtual bool   OnCommandUpdate(CCmdUI* pi_pCmdUI,
                                       CActiveImageView* pi_pView,
                                       CActiveImageDoc*  pi_pDoc);

        HFCPtr<HRABitmap> ColorSelection();





    private:

        ////////////////////////////////////////
        // Private attributes
        ////////////////////////////////////////
        
        // Pointers to the document and its view
        CActiveImageDoc*    m_pDoc;
        CActiveImageView*   m_pView;

        // Raster list for selection
        HFCPtr<HRARaster>        m_pCurrentRaster;

        // Toolbar for default mode
        static CToolBar          m_Toolbar;

        // MENU
        static CMenu             m_Menu;
        static CMenu             m_FilterMenu;
        static CMenu             m_FilterSubMenu[CActiveImageDefaultMode_FilteringLevel];

        // transparence
        static CMenu             m_TransparenceMenu;
        static Byte            m_Transparence;

        // Raster selection
        // Selection vector used for Mosaic operations
        // This vector will contain all the rasters that are selected
        RasterSelection         m_Selection;


        ////////////////////////////////////////
        // Private methods
        ////////////////////////////////////////

        // Selection methods
        
        HGF2DExtent 
             GetSelectionExtent() const;
        HVEShape
             GetSelectionShape() const;

        // Update Command Handlers
        void OnUpdateOrder(CCmdUI* pCmdUI);
        void OnUpdateRotate(CCmdUI* pCmdUI);
        void OnUpdateRemove(CCmdUI* pCmdUI);

        // Command Handlers
        void OnBringToFront();
	    void OnMoveDown();
	    void OnMoveUp();
	    void OnSendToBack();
	    void OnRotate();
        void OnRemove();
        void OnUnselect();
        void OnSelectAll();
        
        // Filter operations
        void OnFilter(const HFCPtr<HRPFilter>& pi_prFilter);
        void OnPixelReplacerFilter();
        void OnAlphaReplacerFilter();
        void OnAlphaComposerFilter();
        void OnRemoveFilters();

        bool OnDEMFilter(HRPDEMFilter::Style style, bool hillShadingState);
        
        // transparence operations
        void OnTransparent(Byte pi_Transparence);

        bool IsRasterTransparent           (const HFCPtr<HRARaster>& pi_rpRaster) const;
        BOOL  IsRasterPixelTypeV16Compatbile(const HFCPtr<HRARaster>& pi_rpRaster) const;
       
        // Drawing methods
        void DrawSelection(CDC* pDC);
        void DrawContour(CDC* pDC, const HVEShape& pi_Shape);

        // Edit methods
        typedef enum
        {
            MS_START,
            MS_MOVE,
            MS_END,
            MS_ABORT
        } MoveStep;
        void MoveRaster  (MoveStep pi_Step, CPoint pi_Point);
        void SelectRaster(UINT nFlags, const HFCPtr<HRARaster>& pi_pRaster);

        // Mosaic operations
        void DoSendToBack   (HFCPtr<HIMMosaic>& pi_pMosaic,
                             HTREEITEM          pi_ParentItem,
                             RasterSelection&   pi_Selection);
        void DoBringToFront (HFCPtr<HIMMosaic>& pi_pMosaic,
                             HTREEITEM          pi_ParentItem,
                             RasterSelection&   pi_Selection);
        void DoMoveDown     (HFCPtr<HIMMosaic>& pi_pMosaic,
                             HTREEITEM          pi_ParentItem,
                             RasterSelection&   pi_Selection);
        void DoMoveUp       (HFCPtr<HIMMosaic>& pi_pMosaic,
                             HTREEITEM          pi_ParentItem,
                             RasterSelection&   pi_Selection);
        void DoRemove       (HFCPtr<HIMMosaic>& pi_pMosaic,
                             HTREEITEM          pi_ParentItem,
                             RasterSelection&   pi_Selection);
        HFCPtr<HRARaster>
            DoRotate       (HTREEITEM          pi_ParentItem,
                             HFCPtr<HRARaster>& pi_pRaster,
                             double            pi_Angle,
                             double            pi_Affinity);
        HFCPtr<HRARaster>
             DoMove         (HFCPtr<HRARaster>& pi_pRaster,
                             HGF2DDisplacement  pi_Displacement);
};

#include "ActiveImageDefaultMode.hpp"

#endif
