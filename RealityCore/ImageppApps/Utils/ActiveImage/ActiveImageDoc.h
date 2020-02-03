/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageDoc.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageDoc.h,v 1.20 2011/07/18 21:10:48 Donald.Morissette Exp $
//
// Class: ActiveImageDoc
// ----------------------------------------------------------------------------

#ifndef __CACTIVEIMAGEDDOC_H__
#define __CACTIVEIMAGEDDOC_H__

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HIMMosaic.h>
#include <Imagepp/all/h/HRAReferenceToRaster.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HPMPool.h>
#include <Imagepp/all/h/HIMFilteredImage.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DWorldCluster.h>
#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>
#include <Imagepp/all/h/HMGThread.h>
#include <Imagepp/all/h/HRFRasterFileCache.h>
#include <Imagepp/all/h/HRFPageDescriptor.h>
#include <Imagepp/all/h/HGF2DTransfoModelAdapter.h>
#include <Imagepp/all/h/HGFPolynomialModelAdapter.h>
#include <Imagepp/all/h/HGF2DProjective.h>

#include <Imagepp/all/h/HCPGCoordModel.h>
#include <Imagepp/all/h/HGF2DShape.h>



#include "DlgInfoReprojection.h"
#include "TextLog.h"
#include "ReprojectDialog.h"
#include "ImageInfo.h"


class CActiveImageDoc : public CDocument
    {
    friend class TreeListOrder;
    friend class CInsertObjectDialog;

    ////////////////////////////////////////
    // Types
    ////////////////////////////////////////

    public:

        // RasterMap is used to store rasters with their associated tree item handle
        typedef map<HTREEITEM, HFCPtr<HRARaster>, less<HTREEITEM>, allocator<HFCPtr<HRARaster> > >
            RasterMap;

        // Raster List for the rasters to clone in Database
        typedef list<HFCPtr<HRARaster> >
            RasterList;

        // ObjectList contains all the object store used to create raster from
        // Raster Files
        typedef list<HPMObjectStore*, allocator<HPMObjectStore*> >
            ObjectList;


        ////////////////////////////////////////
        // Constructor/Destructor
        // Constructor must be Protected (from MFC)
        // create from serialization only
        ////////////////////////////////////////

    protected:
        CActiveImageDoc();
        DECLARE_DYNCREATE(CActiveImageDoc)

    public:
        virtual ~CActiveImageDoc();

        // Overridden from CDocument
        virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = true);
        virtual BOOL SaveModified();


        ////////////////////////////////////////
        // Debug Methods
        ////////////////////////////////////////
#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif


        ////////////////////////////////////////
        // Selection Tree Methods
        ////////////////////////////////////////

        // selection operation
        void       SelectedObjectChanged();
        HTREEITEM  GetSelectedObject() const;
        HTREEITEM  GetDocumentObject() const;
        bool IsRasterSelected() const;

        // Tree Control methods
        CTreeCtrl*  GetTreeCtrl() const;
        void        MoveTreeItem(HTREEITEM  pi_Destination,
                                 HTREEITEM  pi_Source,
                                 HTREEITEM  pi_InsertAfterInDestination);
        void        RemoveTreeItem(HTREEITEM pi_ItemToDelete);

        HFCPtr<HRARaster> PlaceInTransparentImage(const HFCPtr<HRARaster>& pi_pRaster,
                                                  Byte pi_Transparence);


        ////////////////////////////////////////
        // Utility Methods
        ////////////////////////////////////////

        // CoordSys System        
        const HFCPtr<HGF2DWorldCluster> GetWorldCluster() const;
        HPMPool*    GetPool() const;


        bool                 IsReadOnly() const;
        bool                 FitImageOnView() const;

        // Raster Map methods
        HFCPtr<HRARaster> GetRaster(HTREEITEM pi_TreeItem) const;
        HTREEITEM         GetTreeItem(HTREEITEM&        pi_Parent,  // Ref (see code)
                                      HFCPtr<HRARaster> pi_pRaster);

        HFCPtr<HGF2DTransfoModel>   GetReprojectionModel();

        // Raster Operation
        HFCPtr<HRAReferenceToRaster>
            PlaceInReference(const HFCPtr<HRARaster>& pi_pRaster);
        bool       IsRasterLinked(HFCPtr<HRARaster>& pi_rpRaster) const;
        void        ReplaceRaster(HTREEITEM pi_hItem, HFCPtr<HRARaster>& pi_pNewRaster);
        void        ReplaceChildRaster(HTREEITEM pi_hItem, HFCPtr<HRARaster>& pi_pRaster, HFCPtr<HRARaster>& pi_pNewRaster);


        // Remote File Operations
        BOOL OnOpenInternetDocument(LPCTSTR lpszPathName);

        // Image view operations
        HFCPtr<HRARaster>
            RemoveImageView(const HFCPtr<HRAImageView>& pi_pRaster);

        // Filter Operations
        HFCPtr<HIMFilteredImage>
            PlaceInFilteredImage(const HFCPtr<HRARaster>& pi_pRaster,
                                 const HFCPtr<HRPFilter>& pi_rpFilter);

        // View operation
        void        RedrawView(HFCPtr<HRARaster>& pi_pRaster);

        // Idle Operation
        virtual     void OnIdle();

        HFCPtr<HRFRasterFile>
            MyImprove(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                      const HRFCacheFileCreator*  pi_pCreator);

        void ExportRaster(const HFCPtr<HVEShape>& pi_rpShape);

        void    AddStringToLog(const CString& pi_String);

        CString GetFilename() const { return m_FileName; };

        ////////////////////////////////////////
        // Message Map Methods
        ////////////////////////////////////////

    protected:

        // Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CActiveImageDoc)
    public:
        virtual BOOL OnNewDocument();
        virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
        virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
        virtual void DeleteContents();
        //}}AFX_VIRTUAL

        //{{AFX_MSG(CActiveImageDoc)
        afx_msg void OnInsertFromExternalDocument();
        afx_msg void OnUpdateInsertFromExternalDocument(CCmdUI* pCmdUI);
        afx_msg void OnInsertNewObject();
        afx_msg void OnUpdateInsertNewObject(CCmdUI* pCmdUI);
        afx_msg void OnInsertFromthisdocument();
        afx_msg void OnUpdateInsertFromthisdocument(CCmdUI* pCmdUI);
        afx_msg void OnFileExportHmr();
        afx_msg void OnFileExportChannels();
        afx_msg void OnUpdateViewRemovetransparency(CCmdUI* pCmdUI);
        afx_msg void OnViewRemovetransparency();
        afx_msg void OnUpdateFileReadonly(CCmdUI* pCmdUI);
        afx_msg void OnFileReadonly();
        afx_msg void OnFilePreferencesCachecompressions();
        afx_msg void OnUpdateFileExport(CCmdUI* pCmdUI);
        afx_msg void OnUpdateFileExportRegion(CCmdUI* pCmdUI);
        afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
        afx_msg void OnFileOverwritehgrtwf();
        afx_msg void OnUpdateFileOverwritehgrtwf(CCmdUI* pCmdUI);
        afx_msg void OnUpdateFileInfoTime(CCmdUI* pCmdUI);
        afx_msg void OnFileInfoTime();
        afx_msg void OnUpdateFilePreferenceFitImageOnView(CCmdUI* pCmdUI);
        afx_msg void OnFilePreferenceFitImageOnView();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()



#ifdef __AI_INTERNET__
        afx_msg void OnInternetAdd();
#endif
#ifdef STRESS_APPS	
        afx_msg void InternetTest();
#endif
        ////////////////////////////////////////
        // Internal Methods
        ////////////////////////////////////////

    private:

        // Document access methods
        void        OpenRasterFile(Utf8CP                  pi_pFileName,
                                   list<HFCPtr<HRARaster> >*      po_pPagesList);
        BOOL        SaveRasterFile(Utf8CP                  pi_pFileName);

        // This function insert a raster in the document
        void        InsertRaster(HFCPtr<HRARaster>&             pi_pRaster,
                                 bool                          pi_InsertAsLink,
                                 HTREEITEM                      pi_ParentItem,
                                 const WChar*                  pi_pFileName = 0);

        void        InsertMultiPageRaster(list<HFCPtr<HRARaster> >&      pi_rPageList,
                                          bool                          pi_InsertAsLink,
                                          HTREEITEM                      pi_ParentItem,
                                          const WChar*                  pi_pFileName);

        //Reprojection methods
        HFCPtr<HGF2DTransfoModel>           ChooseModelWithPrecision(const HGF2DTransfoModel& pExactModel,
                                                                     const HGF2DLiteExtent& pRectangle,
                                                                     double DimensionMeanPixel);
        HFCPtr<HGFPolynomialModelAdapter>   ComputePolynomialModel(const HGF2DTransfoModel& pExactModel,
                                                                   const HGF2DLiteExtent& pRectangle,
                                                                   double DimensionMeanPixel);
        HFCPtr<HGF2DProjective>     ComputeProjectiveModel(const HGF2DTransfoModel& pExactModel,
                                                                   const HGF2DLiteExtent& pRectangle,
                                                                   double DimensionMeanPixel);

        void UpdateReprojectionInfoDlg();



        // Tree List operations
        HTREEITEM   InsertRasterInTree(HFCPtr<HRARaster>              pi_pRaster,
                                       HTREEITEM                      pi_ParentItem,
                                       const WChar*                  pi_pRasterName = 0,
                                       HTREEITEM                      pi_InsertAfter = TVI_FIRST);

        void        LoadRasterFile(HFCPtr<HRFRasterFile>&         pi_rpRasterFile,
                                   list<HFCPtr<HRARaster> >*      po_pRasterList);

        void        LoadPSS(const HFCPtr<HFCURL>&          pi_rpFileName,
                            list<HFCPtr<HRARaster> >*      po_pRasterList);
        void        InitializeRaster(HFCPtr<HRARaster>&             pio_rpRaster) const;

        HFCPtr<HRFRasterFile>
            ImproveRasterFile(HFCPtr<HRFRasterFile>&         pi_rpRasterFile) const;


        // Mosaic Operations
        bool       VerifyLoopForward(HFCPtr<HRARaster>&             pi_pRaster1,
                                     HFCPtr<HRARaster>&             pi_pRaster2);
        bool       VerifyLoopBackward(HFCPtr<HRARaster>&             pi_pRaster1,
                                      HFCPtr<HRARaster>&             pi_pRaster2);


        void        Test_DumpGeoKeysString(HFCPtr<HRFRasterFile>&         pi_rpTrueRasterFile,
                                           const WChar*                  pi_pFileName) const;

        void        CloseAllFiles();


        ////////////////////////////////////////
        // Document attributes
        ////////////////////////////////////////

    private:

        // CoordSys System        
        //    HFCPtr<HGFHMRStdWorldCluster> m_pWorldCluster;
        HFCPtr<HGF2DWorldCluster>   m_pWorldCluster;

        // Raster map and raster list to clone upon save
        RasterMap                   m_RasterMap;
        RasterList                  m_RasterToClone;
        //map<HFCPtr<HRARaster>, GeoCoordinates::BaseGCSCP>    m_GcsCPList;

        //Model used to reproject to another gcs, for instance
        HFCPtr<HGF2DTransfoModel>       m_ReprojectionModel;
        GeoCoordinates::BaseGCSPtr    m_GcsPtrFromWellKnownText;

        // Objects
        HAutoPtr<HPMPool>           m_pLog;
        CString                     m_FileName;
        HFCAccessMode               m_FileAccessMode;

        // Images counter for labels
        uint32_t                     m_NumMosaic;
        uint32_t                     m_NumImages;
        HTREEITEM                   m_DocItem;

        bool                       m_RemoveTransparency;
        bool                       m_ReadOnlyMode;
        bool                       m_PageFileOverwrite;
        bool                       m_FitImageOnView;

        // options
        bool                       m_DisableCache;
        int                        m_LoadInMemoryID;
        bool                       m_BestContrast;
        int                        m_3band16bittorgbID;
        bool                       m_3band16bitSameValue;

        // Cached resolutions
        bool                       m_NormalCache;
        bool                       m_Cache16384;
        bool                       m_Cache8192;
        bool                       m_Cache4096;
        bool                       m_Cache2048;
        bool                       m_Cache1024;
        bool                       m_Cache512;
        bool                       m_Cache256;
        bool                       m_Cache128;

        // Helper object to receive HMG messages from other threads
        HMGThread                   m_HMGThread;

        uint32_t                   m_reprojectionType;
        CString                    m_stringInfoReprojection;
        bool                       m_HMRProjectionInitialized;
        unsigned short             m_HMRProjection;
        int                        m_1BitForegroundMode;

        // Oracle Support
        HINSTANCE                   m_OracleWrapperDll;

        TextLog*                    m_pTextLogDlg;
        ReprojectDialog*            m_pReprojectDlg;
        ImageInfo*                  m_pImageInfoDlg;
        CDialog*                    m_pInfoReprojectionDlg;

    public:
        afx_msg void OnView1bitForeground(UINT nID);
        afx_msg void OnUpdateView1bitForeground(CCmdUI *pCmdUI);
        afx_msg void OnOptionsLoadInMemory(UINT nID);
        afx_msg void OnOptionsBestContrast();
        afx_msg void OnOptionsDisableCache();
        afx_msg void OnUpdateCachedResolutions16384(CCmdUI *pCmdUI);
        afx_msg void OnUpdateCachedResolutions8192(CCmdUI *pCmdUI);
        afx_msg void OnUpdateCachedResolutions4096(CCmdUI *pCmdUI);
        afx_msg void OnUpdateCachedResolutions2048(CCmdUI *pCmdUI);
        afx_msg void OnUpdateCachedResolutions1024(CCmdUI *pCmdUI);
        afx_msg void OnUpdateCachedResolutions512(CCmdUI *pCmdUI);
        afx_msg void OnUpdateCachedResolutions256(CCmdUI *pCmdUI);
        afx_msg void OnUpdateCachedResolutions128(CCmdUI *pCmdUI);
        afx_msg void OnUpdateCachedResolutionsNormalCache(CCmdUI *pCmdUI);
        afx_msg void OnUpdateOptionsLoadInMemory(CCmdUI *pCmdUI);
        afx_msg void OnUpdateOptionsBestContrast(CCmdUI *pCmdUI);
        afx_msg void OnUpdateOptionsDisableCache(CCmdUI *pCmdUI);
        afx_msg void OnCachedResolutions16384();
        afx_msg void OnCachedResolutions8192();
        afx_msg void OnCachedResolutions4096();
        afx_msg void OnCachedResolutions2048();
        afx_msg void OnCachedResolutions1024();
        afx_msg void OnCachedResolutions512();
        afx_msg void OnCachedResolutions256();
        afx_msg void OnCachedResolutions128();
        afx_msg void OnCachedResolutionsNormalCache();
        afx_msg void OnInfoTextlog();
        virtual void OnCloseDocument();
        afx_msg void OnInfoImageinfo();
        afx_msg void OnReproject();
        void UpdateWkt();
        HFCPtr<HGF2DTransfoModel> UpdateReprojectionModel();
        void OnReprojectionModelUpdate(UINT nID);
        void OnReprojectionModelUpdateCheckBox(CCmdUI* pCmdUI);
        afx_msg void OnInfoReprojection();
        afx_msg void On3band16bittorgb(UINT nID);
        afx_msg void OnUpdate3band16bittorgb(CCmdUI *pCmdUI);
        afx_msg void On3band16bittorgbUsedsamevalue();
        afx_msg void OnUpdate3band16bittorgbUsedsamevalue(CCmdUI *pCmdUI);
};

//-----------------------------------------------------------------------------
// TreeListOrder
// Sort object for raster containers
//-----------------------------------------------------------------------------
class TreeListOrder
    {
    public:
        TreeListOrder(CActiveImageDoc* pi_pDoc,
                      CTreeCtrl*       pi_pTree,
                      HTREEITEM        pi_ParentItem);
        bool operator()(const HFCPtr<HRARaster>& pi_rRaster1,
                        const HFCPtr<HRARaster>& pi_rRaster2);

    private:
        CActiveImageDoc*                     m_pDoc;
        CTreeCtrl*                           m_pTree;
        HTREEITEM                            m_ParentItem;
    };


//-----------------------------------------------------------------------------
// ExportBlockAccessListener
// Listen to error opening during block access
//-----------------------------------------------------------------------------
class ExportBlockAccessListener : public HRSBlockAccessListener
    {
    public:
        ExportBlockAccessListener();

        virtual ~ExportBlockAccessListener();

        virtual void OnReadBlockError(HSTATUS               pi_Error,
                                      HFCPtr<HFCURL> const& pi_pURL);

        virtual void OnWriteBlockError(HSTATUS               pi_Error,
                                       HFCPtr<HFCURL> const& pi_pURL);

        HSTATUS      GetExportStatus();

    private:

        HSTATUS m_Status;
    };



#endif
