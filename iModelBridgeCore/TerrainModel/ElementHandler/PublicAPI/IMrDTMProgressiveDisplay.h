/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/PublicAPI/IMrDTMProgressiveDisplay.h $
|    $RCSfile: IMrDTMProgressiveDisplay.h,v $
|   $Revision: 1.4 $
|       $Date: 2012/02/08 18:22:46 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

/*--------------------------------------------------------------------------------------+
|   Header File Dependencies
+--------------------------------------------------------------------------------------*/
// MicroStation Includes
/*
#include <bentley.h>
#include <mstypes.h>
#include <GeoCoord\BaseGeoCoord.h>
#include <RasterlibExport.h>
*/
#include <TerrainModel\ElementHandler\IMultiResolutionGridMaterialManager.h>

//DTM Shared Part
#include <ScalableTerrainModel/IMrDTM.h>




BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

class IMrDTMDataRef;

DTMELEMENT_EXPORT bool IsWireframeRendering(ViewContextCR viewContextCP);

// Forward declarations

//
// Class Declarations

/*--------------------------------------------------------------------------------------+
|class IRasterTextureSourceCacheFile
|      This interface is supported by any kind of complex raster texture source 
|      (e.g. : source involving multiple raster files) who supports caching data 
|      (e.g. : lowest resolution data) in a cache file. 
+--------------------------------------------------------------------------------------*/  
struct DTMDrawingInfo;

struct IMrDTMProgressiveStroker : public Bentley::DgnPlatform::IStrokeForCache, 
                                  public RefCounted <IRefCounted>
    {
    protected : 

        virtual unsigned int _GetNbPointsStroken() = 0;

    public : 

        DTMELEMENT_EXPORT unsigned int GetNbPointsStroken();

    };

typedef RefCountedPtr<IMrDTMProgressiveStroker> IMrDTMProgressiveStrokerPtr;


struct IMrDTMProgressiveStrokerManager : RefCounted <IRefCounted>
    {
    protected : 

        virtual int _CreateStroker(IMrDTMProgressiveStrokerPtr& strokerForCache, BcDTMP dtm,  struct DTMDrawingInfo* dtmDrawingIfo, DPoint3d* strokingExtentPts, int nbStrokingExtentPts, DTMFeatureId textureRegionFeatureId) = 0;
                        
    public : 

        DTMELEMENT_EXPORT int CreateStroker(IMrDTMProgressiveStrokerPtr& strokerForCache, BcDTMP dtm,  struct DTMDrawingInfo* dtmDrawingIfo, DPoint3d* strokingExtentPts, int nbStrokingExtentPts, DTMFeatureId textureRegionFeatureId);
    };

typedef RefCountedPtr<IMrDTMProgressiveStrokerManager> IMrDTMProgressiveStrokerManagerPtr;
       

struct IMrDTMProgressiveDataBlockManager : RefCounted <IRefCounted>
    {
    protected : 
                        
    public : 

    };

typedef RefCountedPtr<IMrDTMProgressiveDataBlockManager> IMrDTMProgressiveDataBlockManagerPtr;


struct IMrDTMProgressiveDataBlockManagerCreator : RefCounted <IRefCounted>
    {
    protected : 
        
        virtual int _CreateDataBlockManagerBasedOnMaterial(IMrDTMProgressiveDataBlockManagerPtr&   dataBlockManagerP,
                                                           BcDTMP 
                                                           dtm, 
                                                           Bentley::TerrainModel::Element::IMultiResolutionGridMaterialManagerPtr& multiResolutionGridMatPtr,
                                                           Bentley::MrDTM::IMrDTMPtr&                              mrDTMPtr,
                                                           DRange3d&                                               dtmRange,
                                                           Transform&                                              storageToUors,                                                           
                                                           ViewContextP                                            context, 
                                                           IMrDTMDataRef*                                          dataSource, 
                                                           double                                                  minScreenPixelsPerDrapePixel) = 0;

        virtual int _CreateDataBlockManagerBasedOnDTM(IMrDTMProgressiveDataBlockManagerPtr& dataBlockManagerP,                                                         
                                                      IMultiResolutionGridManagerPtr&       multiResolutionGridDTMManagerPtr,                                                                                                                                    
                                                      DRange3d&                             dtmRange,
                                                      Transform&                            storageToUors,                                                           
                                                      ViewContextP                          context, 
                                                      IMrDTMDataRef*                        dataSource, 
                                                      BcDTMP                                dtm, 
                                                      double                                minScreenPixelsPerPoint) = 0;        
        
    public : 
                       
        DTMELEMENT_EXPORT int CreateDataBlockManagerBasedOnMaterial(IMrDTMProgressiveDataBlockManagerPtr&                          dataBlockManagerP,
                                                                    BcDTMP                                                        dtm, 
                                                                    Bentley::TerrainModel::Element::IMultiResolutionGridMaterialManagerPtr& multiResolutionGridMatPtr,                                                                                                                                          
                                                                    Bentley::MrDTM::IMrDTMPtr&                                     mrDTMPtr,
                                                                    DRange3d&                                                      dtmRange,
                                                                    Transform&                                                     storageToUors,
                                                                    ViewContextP                                                   context, 
                                                                    IMrDTMDataRef*                                                  dataSource, 
                                                                    double                                                         minScreenPixelsPerDrapePixel);
                                                                                    
        DTMELEMENT_EXPORT int CreateDataBlockManagerBasedOnDTM(IMrDTMProgressiveDataBlockManagerPtr& dataBlockManagerP,  
                                                               IMultiResolutionGridManagerPtr&       multiResolutionGridDTMManagerPtr,
                                                               DRange3d&                             dtmRange,
                                                               Transform&                            storageToUors,
                                                               ViewContextP                          context, 
                                                               IMrDTMDataRef*                        dataSource, 
                                                               BcDTMP                                dtm, 
                                                               double                                minScreenPixelsPerPoint);        
    };

typedef RefCountedPtr<IMrDTMProgressiveDataBlockManagerCreator> IMrDTMProgressiveDataBlockManagerCreatorPtr;


struct IMrDTMProgressiveDisplay : RefCounted <IRefCounted>
    {
    protected : 

        virtual int _ClearCachedBlocks(IMrDTMDataRef* mrdtmSource, bool onlyBlockWithMaterial) = 0;

        virtual int _Draw(ElementHandleCR el, ViewContextR context, RefCountedPtr<DTMDataRef>& DTMDataRef, const Bentley::DgnPlatform::ElementHandle::XAttributeIter& xAttr, Transform& uorToStorage, DTMDrawingInfo* dtmDrawingInfo, bool isDrapingRequired) = 0;        

        virtual int _GetMrDTMProgressiveDataBlockManagerCreator(IMrDTMProgressiveDataBlockManagerCreatorPtr& progressiveDataBlockManagerCreator) = 0; 

        virtual int _SetMrDTMProgressiveStrokerManager(IMrDTMProgressiveStrokerManagerPtr& progressiveStroker) = 0;        
               
    public : 
                       
        DTMELEMENT_EXPORT int ClearCachedBlocks(IMrDTMDataRef* mrdtmSource, bool onlyBlockWithMaterial);

        DTMELEMENT_EXPORT int Draw(ElementHandleCR el, ViewContextR context, RefCountedPtr<DTMDataRef>& DTMDataRef, const Bentley::DgnPlatform::ElementHandle::XAttributeIter& xAttr, Transform& uorToStorage, DTMDrawingInfo* dtmDrawingInfo, bool isDrapingRequired);

        DTMELEMENT_EXPORT int GetMrDTMProgressiveDataBlockManagerCreator(IMrDTMProgressiveDataBlockManagerCreatorPtr& progressiveDataBlockManagerCreator); 

        DTMELEMENT_EXPORT int SetMrDTMProgressiveStrokerManager(IMrDTMProgressiveStrokerManagerPtr& progressiveStroker);            
    };

typedef RefCountedPtr<IMrDTMProgressiveDisplay> IMrDTMProgressiveDisplayPtr;

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE