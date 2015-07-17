/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/MrDTMDataRef.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__PUBLISH_SECTION_START__*/
#pragma once
/*__PUBLISH_SECTION_END__*/

#include <DgnPlatform/DgnDocumentManager.h>
#include <TerrainModel\ElementHandler\IRasterTextureSource.h>
#include <TerrainModel\ElementHandler\IMultiResolutionGridMaterialManager.h>

#include <DgnPlatform/XAttributeHandler.h>



/*__PUBLISH_SECTION_START__*/

#include <TerrainModel\ElementHandler\DTMDataRef.h>

/*__PUBLISH_SECTION_END__*/

#include <DgnGeoCoord/DgnGeoCoord.h>

#include "DTMDataRefCachingManager.h"
#include <ScalableTerrainModel/IMrDTM.h>
#include <ScalableTerrainModel/IMrDTMCreator.h>
#include <ScalableTerrainModel/MrDTMDefs.h>

#include <ScalableTerrainModel/IMrDTMClipContainer.h>
#include "MrDTMXAttributeHandler.h"

#include "DTMDisplayUtils.h"
#include <TerrainModel\ElementHandler\IMrDTMDataRef.h>
#include <TerrainModel\ElementHandler\IMrDTMProgressiveDisplay.h>

USING_NAMESPACE_RASTER
BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

USING_NAMESPACE_BENTLEY_MRDTM


enum RasterTextureCacheState
    {
    RASTER_TEXTURE_CACHE_STATE_DIRTY = 0,
    RASTER_TEXTURE_CACHE_STATE_UP_TO_DATE,
    };

class MrDTMDrawingInfo;

typedef RefCountedPtr<MrDTMDrawingInfo> MrDTMDrawingInfoPtr;     

class MrDTMDrawingInfo : public RefCountedBase
    {
private : 

    DrawPurpose m_drawPurpose;        
    bool        m_isHighQualityDisplayOn;
    DMatrix4d   m_localToViewTransformation;
    ViewInfoPtr m_viewInfo;
        
public : 

    MrDTMDrawingInfo(ViewContextP viewContext)
        :   m_viewInfo(ViewInfo::CopyFrom(*viewContext->GetViewport()->GetViewInfoCP(), true, true, true))
        {
        m_drawPurpose = viewContext->GetDrawPurpose();   
        const DMatrix4d localToView(viewContext->GetLocalToView());     
        memcpy(&m_localToViewTransformation, &localToView, sizeof(DMatrix4d));

        m_isHighQualityDisplayOn = MrDTMElementDisplayHandler::IsHighQualityDisplayForMrDTM();
        }

    ~MrDTMDrawingInfo()
        {
        }

    DrawPurpose GetDrawPurpose()
        {
        return m_drawPurpose;
        }
    
    bool HasAppearanceChanged(const MrDTMDrawingInfoPtr& mrdtmDrawingInfoPtr)
        {        
        if ( (NULL == &*m_viewInfo) != (NULL == &*mrdtmDrawingInfoPtr->m_viewInfo) )
            return  false;

        if (NULL == &*m_viewInfo)
            return  true;
           
        ViewChangeType  changeType;
        const bool areEquivalent = m_viewInfo->IsEqual (&*mrdtmDrawingInfoPtr->m_viewInfo, changeType);
                 
        return !areEquivalent ||  
               (0 != memcmp(&mrdtmDrawingInfoPtr->m_localToViewTransformation, &m_localToViewTransformation, sizeof(DMatrix4d))) ||
               (m_isHighQualityDisplayOn != mrdtmDrawingInfoPtr->m_isHighQualityDisplayOn);        
        }
    };

class MrDTMDataRef;

class MrDTMViewData 
    {
    friend class MrDTMDataRef;

    private:
    long                                 m_latestResolution;
    double                               m_latestFenceDeltaX;
    double                               m_latestFenceDeltaY;
    double                               m_latestFenceDeltaZ;    
    Bentley::TerrainModel::DTMPtr                 m_dtmPtr;    

    IMrDTMProgressiveDataBlockManagerPtr m_dataBlockManagerPtr;

    
    //Should not be put in a smart pointer because MrDTMDataRef 
    //has pointers to MrDTMViewData (circular dependency).
    MrDTMDataRef*   m_DTMDataRef;
    IMrDTMQueryPtr  m_mrDtmFullResolutionLinearQueryPtr;
    IMrDTMQueryPtr  m_mrDtmViewDependentPointQueryPtr;
    
    MrDTMDrawingInfoPtr m_previousMrDTMDrawingInfoPtr;
    MrDTMDrawingInfoPtr m_previousMrDTMDrawingInfoForProgressiveDisplayPtr;
        
    bool            m_previousDrawDrapingRequired;
    
    void QueryRequiredDTM(ViewContextP context);        

    protected: 
        //Only MrDTMDataRef can create MrDTMViewData object.
        MrDTMViewData(MrDTMDataRef* dtmDataRef);
        ~MrDTMViewData();

    public:
        
    DTMPtr GetDTM(DTMDataRefPurpose purpose, ViewContextP context);


    int    GetProgressiveDataBlockManager(IMrDTMProgressiveDataBlockManagerPtr& progressiveDataBlockManagerPtr,
                                          BcDTMP                               dtmToLoadProgressively, 
                                          ViewContextP                         context, 
                                          bool                                  isDrapingRequired);                

    };

#define MAX_NB_POINTS_FOR_OVERVIEW 50000

/*__PUBLISH_SECTION_START__*/  
class MrDTMDataRef;

typedef void (*FPNotifyChange)(ElementHandleCR);                        

class MrDTMDataRef : public IMrDTMDataRef
    {
    friend class MrDTMViewData;
    friend class MrDTMXAttributeHandler;        

/*__PUBLISH_SECTION_END__*/

private:    
    MrDTMDataRef(); // disabled
    
    //Pointer
   
    //Range information
    DPoint3d        m_minPoint;
    DPoint3d        m_maxPoint;   

    DRange3d* m_pGetClippedExtentEstimation;

    //XAttribute information
    XAttributeHandlerId m_xAttrMrDTMDetailsHandlerId;
    
    long            m_maxNbPoints;    

    //Interface pointers to the MrDTM.
    IMrDTMPtr             m_mrDTM;
    IMrDTMPtr             m_MrDTMlowestResolutionPointView;
    Bentley::TerrainModel::DTMPtr  m_overviewDTMPtr;
    
    MrDTMViewData*       m_viewData[NUMBER_OF_PP_VIEWS];     
    DgnDocumentPtr        m_mrdtmDocumentPtr;    
    DgnDocumentMonikerPtr m_mrdtmMonikerPtr;

    //Display control information
    double           m_pointDensityForShadedView;    //In minimum screen pixels per point.
    double           m_pointDensityForWireframeView; //In minimum screen pixels per point.

    int              m_edgeMethod;
    double           m_edgeMethodLength;

              
    RasterTextureCacheState                                                m_rasterTextureCacheState;
    IRasterTextureSourceCacheFile::DecimationMethod m_rasterTextureCacheDecimationMethod;


    IMrDTMClipContainerPtr m_clipContainerPtr;    
    bool                   m_isClipActivated;
    bool                   m_visibilityPerView[NUMBER_OF_PP_VIEWS];    
        
    //Global information on the MrDTM.
    WString          m_description;
    bool             m_isAnchored;
    bool             m_canLocate;    
    bool             m_isReadOnly;
    IMrDTMCreatorPtr m_mrDTMCreator;   
    MrDTMStatus      m_iDTMFileLastLoadStatus;        

    //Information on

    DgnGCSP         m_destinationGCSP;    


    //Draping objects
    IRasterTextureSourcePtr                m_rasterDrapingSrcPtr;    
    IMultiResolutionGridMaterialManagerPtr m_multiResolutionGridMaterialManagerPtr;

    //Multi-resolution grid manager to use for tiling the terrain data when no texturing is done.
    IMultiResolutionGridManagerPtr         m_multiResolutionGridDtmManagerPtr;


    static MrDTMDataRef*                           CreateP (ElementHandleCR el, const DgnDocumentPtr& mrdtmDocumentPtr = 0, bool inCreation = true)
        {
        return new MrDTMDataRef(el, mrdtmDocumentPtr, inCreation);
        }

    int                                            AddClipOnQuery(IMrDTMQueryPtr& queryPtr);

    void                                           GetDefaultMinimumPixelSizeInUORS(DPoint2d&                         minimumPixelSizeInUORS,
                                                                                    Bentley::GeoCoordinates::BaseGCSP gcsForDefaultPixelSizeP);

    void                                           NotifyChange();
                
    int                                            GenerateMrDTM();    


    bool                                           LoadRasterTextureSource(StringList* drapingRasterFileNamesP);
                
    int                                            IsMrDTMUpToDate(bool& isUpToDate);      

    int                                            PushBackClip(DPoint3d* clipPointsP,
                                                                int       numberOfPoints, 
                                                                bool      isClipMask);   

    int                                            ReloadMrDTM(DgnGCSP targetGCS, bool updateRange = true);
        
    int                                            ReadClipsFromSerializationData(const byte* data, size_t dataSize);

    int                                            ReadMonikerFromSerializationData(const byte* data, size_t dataSize, DgnModelRefP modelRefP);

    int                                            SaveClipsInXAttribute(bool replaceInModel = true, EditElementHandle* elmHandleP = 0);        
       
    void                                           SetWarningForLoadStatus(MrDTMStatus warningStatus);

    int                                            UpdateStorageToUORMatrix(EditElementHandleR elemHandle);
        
    void                                           WriteXAttributePointDensityData(bool replaceInModel = true, EditElementHandle* elmHandleP = 0);

    void                                           WriteXAttributeTriangulationEdgeOptions(bool replaceInModel = true, EditElementHandle* elmHandleP = 0);

    struct GcoordGeoCoordinateEventHandler : public Bentley::GeoCoordinates::IGeoCoordinateEventHandler
        {
        GcoordGeoCoordinateEventHandler (MrDTMDataRef* dataRef)
            {
                m_dataRef = dataRef;
            }
        virtual StatusInt   BeforeCoordinateSystemChanged (DgnGCSP oldGCS, DgnGCSP newGCS, DgnModelRefP modelRef, bool primaryCoordSys, bool writingToFile, bool reprojectData)
            {
            return SUCCESS;
            };
        virtual void        AfterCoordinateSystemChanged (DgnGCSP oldGCS, DgnGCSP newGCS, DgnModelRefP modelRef, bool primaryCoordSys, bool writtenToFile, bool reprojectData)
            {
            // In the case a new coordinate system is assigned (there were none before) then the clips must be reprojected.
            // Reprojection is automatically called in other cases.
            if (oldGCS == NULL)
                m_dataRef->Reproject(NULL, newGCS);

            m_dataRef->AdviseGCSChanged();
            return;
            };

        virtual StatusInt   BeforeCoordinateSystemDeleted (DgnGCSP currentGCS, DgnModelRefP modelRef, bool primaryCoordSys)
            {
            return SUCCESS;
            };

        virtual void        AfterCoordinateSystemDeleted (DgnGCSP currentGCS, DgnModelRefP modelRef, bool primaryCoordSys)
            {
            // Calling this insures that clips are projected back to the mrdtm source GCS prior to reloading
            m_dataRef->Reproject(currentGCS, NULL);

            m_dataRef->AdviseGCSChanged();

            return;
            };

        virtual void        BeforeReferenceGeoCoordinationChanged (DgnModelRefP modelRef, Bentley::GeoCoordinates::GeoCoordinationState oldState, Bentley::GeoCoordinates::GeoCoordinationState newState)
            {
            return;
            };

        virtual void        AfterReferenceGeoCoordinationChanged (DgnModelRefP modelRef, Bentley::GeoCoordinates::GeoCoordinationState oldState, Bentley::GeoCoordinates::GeoCoordinationState newState)
            {
            return;
            }

        private:
            MrDTMDataRef* m_dataRef;
        };

    GcoordGeoCoordinateEventHandler* m_GCSEventHandler;

protected: 

        virtual int _GetOverviewDTM(Bentley::TerrainModel::DTMPtr& overviewDTMPtr) override;


    
        virtual int         _GetProgressiveDataBlockManager(IMrDTMProgressiveDataBlockManagerPtr& progressiveDataBlockManagerPtr,
                                                                          BcDTMP                               dtmToLoadProgressively, 
                                                                          ViewContextP                         context, 
                                                                          bool                                  isDrapingRequired) override;   

         //Creation and data source modification functions
        virtual int            _GetIMrDTMCreator(Bentley::MrDTM::IMrDTMCreatorPtr& iMrDTMCreatorPtr) override;          

        virtual int            _SaveIMrDTMCreatorToFile() override;

        //Information functions  
        virtual const WString& _GetDescription() const override;          

        virtual int            _SetDescription(const WString& description) override;         
    
        virtual const DgnDocumentPtr       _GetMrDTMDocument() const override;          

        virtual int                        _SetMrDTMDocument(DgnDocumentPtr& mrdtmDocumentPtr) override;         
                   
        virtual const DgnDocumentMonikerPtr _GetFileMoniker() const override;          
    /*           
        virtual int                          _SetFileMoniker(DgnDocumentMonikerPtr& fileMonikerPtr) override;         
      */                                      
        virtual bool           _CanLocate() const override;          

        virtual int            _SetLocate(bool canLocate) override;     

        virtual bool           _IsAnchored() const override;

        virtual int            _SetAnchored(bool isAnchored) override;

        virtual MrDTMStatus    _GetIDTMFileLastLoadStatus() override;         
             
        virtual bool           _IsOpenInReadOnly() override;

        virtual int            _SetReadOnly(bool isReadOnly) override;

        virtual Bentley::GeoCoordinates::BaseGCSPtr _GetGeoCS() override;         

        virtual int                                 _SetGeoCS(const Bentley::GeoCoordinates::BaseGCSPtr& baseGCSPtr) override;                 

        virtual int                                 _GetExtentInGeoCS(DRange3d& extentInGeoCS, bool inUors) override;
            
        //Display control function    

        virtual int            _GetEdgeMethod(int& edgeMethod) override;

        virtual double         _GetEdgeMethodLengthInStorage() override;

        virtual int            _GetEdgeMethodLength(double& edgeMethodLength) override;

        virtual bool           _GetClipActivation() const override;

        virtual int            _SetClipActivation(bool isActivated) override;              

        virtual int            _GetNbClips() const override;

        virtual int            _GetClip(DPoint3d*& clipPointsP,
                                                      int&       numberOfPoints, 
                                                      bool&      isClipMask,
                                                      int        clipInd) const override;

        virtual int            _AddClip(DPoint3d* clipPointsP,
                                                      int       numberOfPoints, 
                                                      bool      isClipMask) override;    

        virtual int            _RemoveClip(int toRemoveClipInd) override;

        virtual int            _RemoveAllClip() override;        
    
        virtual double         _GetPointDensityForShadedView() override;                        

        virtual int            _SetPointDensityForShadedView(double pi_minScreenPixelsPerPoint) override;
    
        virtual double         _GetPointDensityForWireframeView() override;                        

        virtual int            _SetPointDensityForWireframeView(double pi_minScreenPixelsPerPoint) override;
         
        //High resolution raster texturing     
        virtual int            _ClearCachedMaterials() override;

        virtual int            _GetRasterTextureCacheFilePath(WString& cacheFilePath) override;

        virtual int            _GetRasterTexturePSSFilePath(WString& pssFilePath) override;
                 
        virtual IRasterTextureSourcePtr _GetRasterTextureSource() override;

        virtual int                             _SetRasterTextureSource(IRasterTextureSourcePtr& rasterTextureSourcePtr) override;
        
    
        virtual bool           _IsVisibleForView(short viewNumber) override;

        virtual int            _SetVisibilityForView(short viewNumber, bool isVisible) override;
   
        //Synchronization functions

        //-	State (empty, dirty, up-to-date + time stamp)                        
            
        virtual int            _GetMrDTMState() override;                        

        virtual bool           _GetLastUpToDateCheckTime(time_t& last) override;
    
        virtual int            _Generate() override;   
            
        virtual int            _GenerateRasterTextureCache() override;   
    
        virtual int            _GetRasterTextureCacheDecimationMethod(IRasterTextureSourceCacheFile::DecimationMethod& decimationMethod) const override;   

        virtual int            _SetRasterTextureCacheDecimationMethod(IRasterTextureSourceCacheFile::DecimationMethod decimationMethod) override;   

        virtual int            _IsRasterTextureCacheFileUpToDate(bool& isUpToDate, Element::EditElementHandle* elmHandleP = 0) override;  
    
        virtual bool           _IsUpToDate() override;       

        virtual Int64          _GetPointCountForResolution(int resolutionIndex) override;    


          //MS : Get GetDestinationGCS and GetTargetGCS are quite similar. 
     //     Their differences should be made clearer.
        virtual DgnGCSP        _GetDestinationGCS() override 
            {
            return m_destinationGCSP;
            }              

          
public:    
    
    static void ScheduleFromDtmFile (DgnModelRefP dgnModelRefP, EditElementHandleR elem, const DgnDocumentPtr& mrdtmDocumentPtr, bool inCreation);
    static void UpdateFromDtmFileImpl (EditElementHandleR elem);
           void ScheduleFromDtmFile (EditElementHandleR elem, TransformCR trsf);
           void UpdateFromDtmFile (EditElementHandleR elem);

    MrDTMDataRef(ElementHandleCR el, const DgnDocumentPtr& mrdtmDocumentPtr = 0, bool inCreation = true);

    virtual ~MrDTMDataRef(); 

    int    GetClippedExtent(DRange3d& clippedExtent);

    IMultiResolutionGridManagerPtr GetMultiResolutionGridDtmManager();

    //Functions which can be used to determine if the MrDTM can be displayed 
    //and drape with high resolution rasters.
    virtual bool CanDrapeRasterTexture();   
            bool CanDisplayMrDTM();
    
    public:
    static RefCountedPtr<DTMDataRef> FromElemHandle(ElementHandleCR el, const DgnDocumentPtr& mrdtmDocumentPtr = 0, bool inCreation = false);
    
    //Inherited from DTMDataRef
    protected: virtual IDTM* _GetDTMStorage(DTMDataRefPurpose purpose) override;
    protected: virtual IDTM* _GetDTMStorage(DTMDataRefPurpose purpose, ViewContextR context) override;
    protected: virtual StatusInt _GetDTMReferenceStorage(RefCountedPtr<IDTM>& outDtm) override;
   // protected: virtual double _GetLastModified() override;
#ifndef GRAPHICCACHE
    public: virtual DTMQvCacheDetails* _GetDTMDetails(ElementHandleCR el, DTMDataRefPurpose purpose, ViewContextR context, DTMDrawingInfo& drawingInfo) override;   
#endif 
    public: virtual bool _GetExtents(DRange3dR range) override;       

    public: virtual IMultiResolutionGridMaterialManagerPtr _GetMultiResGridMaterialManager() override;

    public: virtual bool _IsMrDTM() override
                {
                return true;
                }    

    public: int SetGeoCS(const Bentley::GeoCoordinates::BaseGCSPtr& baseGCSPtr, bool fromUndoRedo);         
    
    public: int SetEdgeMethod(int edgeMethod);

    public: int SetEdgeMethodLength(double edgeMethodLength);

    public: virtual void UpdateAfterModelUnitDefinitionChange() override
        {
        ReloadMrDTM(GetDestinationGCS());
        }

    public: int SetReadOnly(bool isReadOnly, bool updateXAtt);
                                                            
    public : void FreeRasterTextureDrapingResources();

    private: int ReadFile();
    private: int ReadFile(DgnGCSP targetGCS);          

    public: const IMrDTMPtr& GetMrDTMPtr() const
                {                
                return m_mrDTM;
                }

    public: const DgnDocumentMoniker& GetMoniker() const
                {     
                BeAssert(0 != m_mrdtmMonikerPtr.get());
                return *m_mrdtmMonikerPtr;
                }



    // NTERAY: This is wrong returning a non-const shared ptr. It actually compromise this class's integrity. 
    // Code owner: Change this as soon as possible...
    public: IMrDTMPtr& GetMrDTM()
                {                
                return m_mrDTM;
                }

    // NTERAY: This is wrong returning a non-const shared ptr. It actually compromise this class's integrity. 
    // Code owner: Change this as soon as possible...
    public: IMrDTMPtr& GetMrDTMLowestResolution()
                {                
                return m_MrDTMlowestResolutionPointView;
                }

    //MS Should be changed to get range
    public: DPoint3d GetMinPoint() {return m_minPoint;}
    public: DPoint3d GetMaxPoint() {return m_maxPoint;}
   
    public: long GetMaxNbPoints()
                {
                return m_maxNbPoints;
                }          
    
    public: DgnGCSP GetTargetGCS();

    public: void AdviseGCSChanged();

    public: void Reproject(DgnGCSP sourceGCS, DgnGCSP targetGCS); 


    public: void FlushAllViewData();
         
    public: bool GetDtmForSingleResolution(BcDTMPtr& singleResolutionDtm, 
                                           long                   maximumNumberOfPoints);              
               
/*__PUBLISH_SECTION_START__*/
    };

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
/*__PUBLISH_SECTION_END__*/