/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageView.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageView.h,v 1.14 2011/07/18 21:10:44 Donald.Morissette Exp $
//
// Class: ActiveImageView
// ----------------------------------------------------------------------------

#pragma once

#include "ActiveImageDoc.h"
#include "ActiveImageFunctionMode.h"
#include "ActiveImageDefaultMode.h"
#include "ActiveImageEditMode.h"
#include "ActiveImageViewMode.h"
#include <Imagepp/all/h/HFCProgressIndicator.h>

#define UPDATE_REDRAW       0x00000001
#define UPDATE_FIRST        0x00000002
#define UPDATE_KEEPZOOM     0x00000004
#define UPDATE_KEEPSCROLL   0x00000008

class CActiveImageFrame;
class HMRProgressImage;
class HDSPage;

class CActiveImageView : public CView, public HFCProgressListener
{
////////////////////////////////////////
// Constructor / Destructor
////////////////////////////////////////

protected: // create from serialization only
	CActiveImageView();
	DECLARE_DYNCREATE(CActiveImageView)

public:
	virtual ~CActiveImageView();


////////////////////////////////////////
// Public Methods
////////////////////////////////////////

public:
	virtual void Progression(HFCProgressIndicator* pi_pProgressIndicator, // Indicator
                             uint64_t             pi_Processed,		  // Total processed items count.                              
                             uint64_t             pi_CountProgression);  // number of items.

	CActiveImageFrame*	GetFrame();
	CProgressCtrl*		GetProgressBar();

    // Things to do before and after operations
    void  BeginOperation();
    void  EndOperation();

    // Function mode Methods
    const CActiveImageFunctionMode* 
                            GetFunctionMode() const;	

    void  SetFunctionMode(CActiveImageFunctionMode* pi_pFunction);

    HFCPtr<HRARaster>     GetSelectedRaster();

    // Method to redraw a portion of the display
    void RedrawImage(HVEShape& pi_Source, HVEShape& pi_Destination);

    // View's CoordSys, bitmap and raster methods
    const HFCPtr<HGF2DCoordSys>& GetCoordSys() const;

    void SetCoordSys(HFCPtr<HGF2DCoordSys>& newCoordSys);

    // Local CoordSys, the raster file coordSys
    const HFCPtr<HGF2DCoordSys>& GetLocalCoordSys() const;

    HGF2DLocation           GetLocation(const CPoint& pi_Point) const;

    COLORREF                GetDefaultColor() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CActiveImageView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo );
	//}}AFX_VIRTUAL

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    void    OnHMRPrint();

////////////////////////////////////////
// Private Attributes
////////////////////////////////////////
private:
    // Command class
//    CActiveImageFunctionMode    m_CommandMode;
    CActiveImageDefaultMode     m_CommandDefaultMode;
    CActiveImageEditMode        m_CommandEditMode;
    CActiveImageViewMode        m_CommandViewMode;

    uint32_t                      m_EyePos;
    bool                       m_LeftDir;

    // Export information
    bool                       m_ExportFence;

    // Utility functions    
	CActiveImageDoc*      GetDocument();
    void                  UpdateMousePosition();
    void                  DrawBitmapToGDI(CDC* pDC, HRABitmapBase& bitmap);
    HFCPtr<HRABitmap>     GenerateDiffBitmap(HRABitmap& src1, HRABitmap& src2, unsigned short threshold1, unsigned short threshold2);

     // Buffered image Tile Size
    uint32_t                m_TileSizeX;
    uint32_t                m_TileSizeY;

    // Pointer to the display raster and its log
    COLORREF                 m_DefaultColorGrayscale;
    COLORREF                 m_DefaultColor;
    HAutoPtr<CBrush>         m_pBkgBrush;
    HPMPool*                 m_pObjectLog;
    uint32_t                  m_ViewOrigin;      // Origin of the view Top_left or Bottom_Left

    // Mouse position point used to display in the status bar
    CPoint                   m_MousePosition;

    // Pointer to the frame window
    CActiveImageFrame*       m_pFrame;

    CProgressCtrl	        m_WndProgress;
	
    bool                    m_DisplayAnnotationIcon;


    // function object
    CActiveImageFunctionMode* m_pFunction;

    // Operation Extent
    HGF2DExtent              m_OperationExtent;

    CPoint                   m_lastMButtonDown;

    HFCPtr<HGF2DCoordSys>    m_pViewCoordSys;  
    HFCPtr<HGF2DCoordSys>    m_pLocalCoordSys;
    HFCPtr<HGF2DCoordSys>    m_pReprojectedCoordSys;


    // Progressive Image support
    UINT_PTR                  m_ProgressTimer;
    UINT_PTR                 m_OnIdleTimer;	

    // Pre-calculate display
    bool                    m_PrecalculateDisplay;

    // Redesign menu bar settings
    uint32_t                m_redesignSettings;

    HGSResampling           m_Resampling;

    uint32_t                m_ColorConversionID;    // ID_CHANGECOLORSPACE_***

    uint32_t RGB2BGR(uint32_t rgb);
   

    void Export(const HFCPtr<HVEShape>& pi_rpShape);

////////////////////////////////////////
// MFC Message Map
////////////////////////////////////////

protected:

    afx_msg void OnFunctionModeCommand(UINT pi_CommandID);
    afx_msg void OnFunctionModeUpdate(CCmdUI* pCmdUI);

    afx_msg void OnTileSize(UINT nID);
    afx_msg void OnTileSizeUpdate(CCmdUI* pCmdUI);

    afx_msg void OnResampling(UINT nID);
    afx_msg void OnResamplingUpdate(CCmdUI* pCmdUI);
    
    afx_msg void OnRedesignMenuBar(UINT nID);
	afx_msg void OnUpdateRedesignMenuBar(CCmdUI* pCmdUI);

    afx_msg void OnColorSpaceConversion(UINT nID);
    afx_msg void OnColorSpaceConversionUpdate(CCmdUI* pCmdUI);

    afx_msg void OnReprojectionModelUpdate(UINT nID);

// Generated message map functions
	//{{AFX_MSG(CActiveImageView)
    afx_msg void OnViewRepaint();
	afx_msg void OnUpdateViewRepaint(CCmdUI* pCmdUI);    
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnViewBackground();
	afx_msg void OnViewRecenterXY();
	afx_msg void OnUpdateViewRecenterXY(CCmdUI* pCmdUI);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnMButtonDown(UINT nFlags,CPoint point);
    afx_msg void OnMButtonUp(UINT nFlags,CPoint point);
    afx_msg void OnMButtonDblClk (UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnUpdateViewVieworiginBottomleft(CCmdUI* pCmdUI);
	afx_msg void OnViewVieworiginBottomleft();
	afx_msg void OnUpdateViewVieworiginTopleft(CCmdUI* pCmdUI);
	afx_msg void OnViewVieworiginTopleft();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnHelpTestdisplay();
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPrecalculateDisplay();
	afx_msg void OnUpdatePrecalculateDisplay(CCmdUI* pCmdUI);
	afx_msg void OnFileExportRegion();
	afx_msg void OnFileExport();
	afx_msg void OnComputeHistogram();
	afx_msg void OnInfoDisplayExtent();
    afx_msg void OnGeocodingInfo();
    afx_msg void OnViewLayers();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnDisplayAnnotationIcon();
    afx_msg void OnUpdateDisplayAnnotationIcons(CCmdUI *pCmdUI);
    afx_msg void OnMenuEditSubsamplingNearest();
    afx_msg void OnUpdateMenuEditSubsamplingNearest(CCmdUI *pCmdUI);
    afx_msg void OnMenuEditSubsamplingAverage();
    afx_msg void OnUpdateMenuEditSubsamplingAverage(CCmdUI *pCmdUI);
    afx_msg void OnMenuEditSubsamplingBilinear();
    afx_msg void OnUpdateMenuEditSubsamplingBilinear(CCmdUI *pCmdUI);
    afx_msg void OnMenuEditSubsamplingBicubic();
    afx_msg void OnUpdateMenuEditSubsamplingBicubic(CCmdUI *pCmdUI);
    afx_msg void OnViewScale();
    afx_msg void OnUpdateViewScale(CCmdUI *pCmdUI);
    afx_msg void OnUpdateReprojectionModel(CCmdUI *pCmdUI);
    afx_msg void OnItemChangedObjectTree(NMHDR *pNMHDR, LRESULT *pResult);
};
