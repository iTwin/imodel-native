/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/IMrDTMProgressiveDisplay.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"

//#include <interface\IMrDTMProgressiveDisplay.h>

#include <TerrainModel\ElementHandler\IMrDTMProgressiveDisplay.h>
#include <TerrainModel\ElementHandler\IMrDTMDataRef.h>

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

unsigned int IMrDTMProgressiveStroker::GetNbPointsStroken()
    {
    return _GetNbPointsStroken();
    }

int IMrDTMProgressiveStrokerManager::CreateStroker(IMrDTMProgressiveStrokerPtr& strokerForCache, BcDTMP dtm,  struct DTMDrawingInfo* dtmDrawingInfo, DPoint3d* strokingExtentPts, int nbStrokingExtentPts, DTMFeatureId textureRegionFeatureId)
    {
    return _CreateStroker(strokerForCache, dtm, dtmDrawingInfo, strokingExtentPts, nbStrokingExtentPts, textureRegionFeatureId);
    }

int IMrDTMProgressiveDataBlockManagerCreator::CreateDataBlockManagerBasedOnMaterial(IMrDTMProgressiveDataBlockManagerPtr&                          dataBlockManagerP,
                                                                                    BcDTMP                                                        dtm, 
                                                                                    IMultiResolutionGridMaterialManagerPtr& multiResolutionGridMatPtr,                                                                                                                                                             
                                                                                    Bentley::MrDTM::IMrDTMPtr&                                     mrDTMPtr,
                                                                                    DRange3d&                                                      dtmRange,
                                                                                    Transform&                                                     storageToUors,                                                           
                                                                                    ViewContextP                                                  context, 
                                                                                    IMrDTMDataRef*                                                  dataSource, 
                                                                                    double                                                         minScreenPixelsPerDrapePixel)
    {
    return _CreateDataBlockManagerBasedOnMaterial(dataBlockManagerP,
                                                  dtm, 
                                                  multiResolutionGridMatPtr,                                                                                                                           
                                                  mrDTMPtr,
                                                  dtmRange,
                                                  storageToUors,
                                                  context, 
                                                  dataSource, 
                                                  minScreenPixelsPerDrapePixel);
    }

int IMrDTMProgressiveDataBlockManagerCreator::CreateDataBlockManagerBasedOnDTM(IMrDTMProgressiveDataBlockManagerPtr&                  dataBlockManagerP,                                                         
                                                                               IMultiResolutionGridManagerPtr& multiResolutionGridDTMManagerPtr,
                                                                               DRange3d&                                              dtmRange,
                                                                               Transform&                                             storageToUors,                                                           
                                                                               ViewContextP                                          context, 
                                                                               IMrDTMDataRef*                                          dataSource, 
                                                                               BcDTMP                                                dtm, 
                                                                               double                                                 minScreenPixelsPerPoint)
    {
    return _CreateDataBlockManagerBasedOnDTM(dataBlockManagerP,                                                         
                                             multiResolutionGridDTMManagerPtr,
                                             dtmRange,
                                             storageToUors,
                                             context, 
                                             dataSource, 
                                             dtm, 
                                             minScreenPixelsPerPoint);
    }   

int IMrDTMProgressiveDisplay::ClearCachedBlocks(IMrDTMDataRef* mrdtmSource, bool onlyBlockWithMaterial)
    {
    return _ClearCachedBlocks(mrdtmSource, onlyBlockWithMaterial);
    }

int IMrDTMProgressiveDisplay::Draw(ElementHandleCR el, ViewContextR context, RefCountedPtr<DTMDataRef>& DTMDataRef, const ElementHandle::XAttributeIter& xAttr, Transform& uorToStorage, DTMDrawingInfo* dtmDrawingInfo, bool isDrapingRequired)
    {
    return _Draw(el, context, DTMDataRef, xAttr, uorToStorage, dtmDrawingInfo, isDrapingRequired);
    }

int IMrDTMProgressiveDisplay::GetMrDTMProgressiveDataBlockManagerCreator(IMrDTMProgressiveDataBlockManagerCreatorPtr& progressiveDataBlockManagerCreator)
    {
    return _GetMrDTMProgressiveDataBlockManagerCreator(progressiveDataBlockManagerCreator);
    }

int IMrDTMProgressiveDisplay::SetMrDTMProgressiveStrokerManager(IMrDTMProgressiveStrokerManagerPtr& progressiveStroker)
    {
    return _SetMrDTMProgressiveStrokerManager(progressiveStroker);
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
