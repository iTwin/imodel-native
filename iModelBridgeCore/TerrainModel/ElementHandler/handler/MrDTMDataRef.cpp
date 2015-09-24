/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/MrDTMDataRef.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"

// It is important that this is the first file included
#include <TerrainModel/ElementHandler/TerrainModelElementHandler.h>
#include <DgnPlatform\Material.h>

#include <io.h>
#include "time.h"

#include "DTMBinaryData.h"
#include "DTMDrawingInfo.h"

#include "MrDTMDataRef.h"
#include <ScalableTerrainModel/MrDTMUtilityFunctions.h>

#include <RmgrTools/Tools/DataExternalizer.h>

#include <TerrainModel\ElementHandler\IRasterTextureSourceManager.h>
      
using namespace Bentley::GeoCoordinates;


/*----------------------------------------------------------------------+
| Include Rasterlib header files                                        |
+----------------------------------------------------------------------*/
BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE


#define APPROX_NB_METERS_FOR_ONE_DEGREE 111000.62
#define DEFAULT_MINIMUM_PIXEL_SIZE_FOR_DRAPING 0.25 //0.25 meters.
#define DOUBLE_EPSILON 0.00001
#define DOUBLE_EQUAL(v1,v2,precision)  ((v1 <= (v2+precision)) && (v1 >= (v2-precision)))
#define MIN_MAX_POINTS 10000
#define MAX_MAX_POINTS 400000
#define MAX_NB_POINTS_MIDDLE_VALUE 100000
#define MAX_NB_POINTS_FOR_CLIPPED_EXTENT_ESTIMATION 50000


//Define to use with the function SetMaximumNumberOfPointsForLinear
#define MAX_NB_POINTS_FOR_LINEAR 50000

//TR 332668 : During an animation loading to few points for the breakline compared to what is shown 
//            lead to some earthquake like effect since the breakline has no sub-resolution and 
//            the points kept for the breaklines are always changing at each display. 
//            A quick, albeit unperfect solution, is to increase the number of points for the breakline.
#define MAX_NB_POINTS_FOR_LINEAR_IN_ANIMATION 500000 

//Printing definition
#define WIREFRAME_PRINTING_DPI 100
#define MAX_NB_POINTS_DURING_SMOOTH_PRINT 1000000.0

typedef vector<FPNotifyChange> ChangeListeners;
ChangeListeners s_changeListeners;

IMrDTMProgressiveDisplayPtr s_mrDtmProgressiveDisplayPtr = 0;    
IPlotterInfo*               s_plotterInterface = 0;

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
class DTMRWDgnCacheGuard
    {
    public:
        DTMRWDgnCacheGuard(DgnModelP cacheP)
            {
            m_dgnCacheP = cacheP;
            if(m_wasReadOnly = m_dgnCacheP->IsReadOnly())
                {
                m_dgnCacheP->SetReadOnly (false);
                }
            }

        ~DTMRWDgnCacheGuard()
            {
            if(m_wasReadOnly)
                m_dgnCacheP->SetReadOnly (true);
            }
    private:
        DgnModelP   m_dgnCacheP;
        bool        m_wasReadOnly;
    };
    
//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
MrDTMViewData::MrDTMViewData(MrDTMDataRef* dtmDataRef)
    {
    BeAssert(dtmDataRef->GetMrDTM() != 0);

    m_DTMDataRef = dtmDataRef;

    if ((dtmDataRef->GetMrDTM()->GetBaseGCS() == 0) || (dtmDataRef->GetDestinationGCS() == 0))
        {
        m_mrDtmFullResolutionLinearQueryPtr = dtmDataRef->GetMrDTM()->GetQueryInterface(DTM_QUERY_FULL_RESOLUTION, 
                                                                                        DTM_QUERY_DATA_LINEAR);   
           
        m_mrDtmViewDependentPointQueryPtr = dtmDataRef->GetMrDTM()->GetQueryInterface(DTM_QUERY_VIEW_DEPENDENT, 
                                                                                      DTM_QUERY_DATA_POINT);
        }
    else
        {
        // Since we are reprojected, the extent of the element is expressed in the target GCS and may not fit exactly
        // with the original source exent (due to reprojection and the actual extent computing algorithm used)
        // we will thus provide the extent of the element as computed. This extent will be used
        // to limit the clip shapes more preceisely than if such clipping were solely based upon the source data extent.
        DRange3d drange;
        m_DTMDataRef->GetExtents(drange);

        Bentley::GeoCoordinates::BaseGCSPtr destinationGCS(dtmDataRef->GetDestinationGCS());

        m_mrDtmFullResolutionLinearQueryPtr = dtmDataRef->GetMrDTM()->GetQueryInterface(DTM_QUERY_FULL_RESOLUTION, 
                                                                                        DTM_QUERY_DATA_LINEAR,
                                                                                        destinationGCS,
                                                                                        drange);   

        m_mrDtmViewDependentPointQueryPtr = dtmDataRef->GetMrDTM()->GetQueryInterface(DTM_QUERY_VIEW_DEPENDENT, 
                                                                                      DTM_QUERY_DATA_POINT, 
                                                                                      destinationGCS,
                                                                                      drange);        
           
        }
    
    //The MrDTM should contain some data.
    BeAssert((m_mrDtmFullResolutionLinearQueryPtr != 0) || (m_mrDtmViewDependentPointQueryPtr != 0));
        
    int status = m_DTMDataRef->AddClipOnQuery(m_mrDtmViewDependentPointQueryPtr);

    BeAssert(status == 0);

    status = m_DTMDataRef->AddClipOnQuery(m_mrDtmFullResolutionLinearQueryPtr);

    BeAssert(status == 0);
            
    m_latestResolution = 0;    
    m_previousDrawDrapingRequired = false;
    m_latestFenceDeltaX = -1;
    m_latestFenceDeltaY = -1;
    m_latestFenceDeltaZ = -1;         
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
MrDTMViewData::~MrDTMViewData()
    {    
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
DTMPtr MrDTMViewData::GetDTM(DTMDataRefPurpose purpose, ViewContextP context)
    {
    if (context)
        QueryRequiredDTM(context);
            
    return m_dtmPtr;
    }


//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 07/11
//=======================================================================================
int MrDTMViewData::GetProgressiveDataBlockManager(IMrDTMProgressiveDataBlockManagerPtr& progressiveDataBlockManagerPtr,
                                                  BcDTMP                               dtmToLoadProgressively, 
                                                  ViewContextP                          context, 
                                                  bool                                  isDrapingRequired)
    {  

    bool needToCreateManager = false;    
        
    MrDTMDrawingInfoPtr currentMrDTMDrawingInfoPtr(new MrDTMDrawingInfo(context));
    
    if (m_previousMrDTMDrawingInfoForProgressiveDisplayPtr != nullptr)
        {                                
        if ((m_previousMrDTMDrawingInfoForProgressiveDisplayPtr->HasAppearanceChanged(currentMrDTMDrawingInfoPtr) == true) ||
            (m_previousMrDTMDrawingInfoForProgressiveDisplayPtr->GetDrawPurpose() == DrawPurpose::UpdateDynamic) ||
            (m_previousDrawDrapingRequired != isDrapingRequired))
            {       
            needToCreateManager = true;         
            }                    
        }
    else
        {        
        if (m_dataBlockManagerPtr == 0)
            {
            needToCreateManager = true;
            }
        }

    m_previousMrDTMDrawingInfoForProgressiveDisplayPtr = currentMrDTMDrawingInfoPtr;
    m_previousDrawDrapingRequired = isDrapingRequired;

    if ((needToCreateManager == true) /*|| (m_dataBlockManagerPtr2->WithRasterDraping() != isDrapingRequired)*/)
        {    
        int status;

        DRange3d dtmExtent;
        
        if (m_DTMDataRef->_GetClipActivation() == true)
            {            
            status = m_DTMDataRef->GetClippedExtent(dtmExtent);

            BeAssert(status == SUCCESS);

            Transform invertTrsf;
            Transform trsf;            

            getStorageToUORMatrix(trsf, m_DTMDataRef->GetElement().GetModelRef(), m_DTMDataRef->GetElement(), false);
                        
            bool invert = bsiTransform_invertTransform(&invertTrsf, &trsf);

            BeAssert(invert != 0);

            bsiTransform_multiplyDPoint3dArrayInPlace(&invertTrsf, (DPoint3dP)&dtmExtent, 2);            
            }
        else
            {
            m_DTMDataRef->GetExtents(dtmExtent);
            }               

        DTMDrawingInfo drawingInfo;        
        
        DTMElementDisplayHandler::GetDTMDrawingInfo(drawingInfo, m_DTMDataRef->GetElement(), m_DTMDataRef, context);

        IMrDTMProgressiveDataBlockManagerCreatorPtr progressiveDataBlockManagerCreatorPtr;

        
        status = IMrDTMDataRef::GetMrDTMProgressiveDisplayInterface()->GetMrDTMProgressiveDataBlockManagerCreator(progressiveDataBlockManagerCreatorPtr); 

        BeAssert((status == SUCCESS) && (progressiveDataBlockManagerCreatorPtr != 0));

        Transform storageToUOR;
                
        ElementHandle elementHandle(m_DTMDataRef->GetElement().GetElementRef(), m_DTMDataRef->GetElement().GetModelRef());
        
        getStorageToUORMatrix(storageToUOR, m_DTMDataRef->GetElement().GetModelRef(), m_DTMDataRef->GetElement(), false);
                                           
        if (false == isDrapingRequired)                            
            {   
            double screenPixelsPerPoint;
          
            if (false == IsWireframeRendering(*context))
                {
                screenPixelsPerPoint = m_DTMDataRef->_GetPointDensityForShadedView();
                }
            else
                {
                screenPixelsPerPoint = m_DTMDataRef->_GetPointDensityForWireframeView();                    
                }

            IMultiResolutionGridManagerPtr gridDtmManager(m_DTMDataRef->GetMultiResolutionGridDtmManager());

            status = progressiveDataBlockManagerCreatorPtr->CreateDataBlockManagerBasedOnDTM(m_dataBlockManagerPtr,
                                                                                             gridDtmManager,
                                                                                             dtmExtent,
                                                                                             storageToUOR, 
                                                                                             context, 
                                                                                             m_DTMDataRef, 
                                                                                             dtmToLoadProgressively, 
                                                                                             screenPixelsPerPoint);                        
            }
        else
            {
            double screenPixelsPerDrapedPixel;

            if (MrDTMElementDisplayHandler::IsHighQualityDisplayForMrDTM() == true)
                {
                screenPixelsPerDrapedPixel = MRDTM_MIN_SCREEN_PIXELS_PER_DRAPE_PIXEL_IN_HIGH_QUALITY_DISPLAY;
                }
            else
                {
                screenPixelsPerDrapedPixel = MRDTM_MIN_SCREEN_PIXELS_PER_DRAPE_PIXEL_IN_DEFAULT_DISPLAY;
                }

            IMultiResolutionGridMaterialManagerPtr materialManagerPtr(m_DTMDataRef->_GetMultiResGridMaterialManager());
                                                      
            status = progressiveDataBlockManagerCreatorPtr->CreateDataBlockManagerBasedOnMaterial(m_dataBlockManagerPtr,
                                                                                                  dtmToLoadProgressively,
                                                                                                  materialManagerPtr,                                                                                                  
                                                                                                  m_DTMDataRef->GetMrDTM(), 
                                                                                                  dtmExtent,
                                                                                                  storageToUOR, 
                                                                                                  context, 
                                                                                                  m_DTMDataRef, 
                                                                                                  screenPixelsPerDrapedPixel);                                                        
            }

        BeAssert(status == 0 && m_dataBlockManagerPtr != 0);
        }  

    progressiveDataBlockManagerPtr = m_dataBlockManagerPtr;

    return 0;
    }



#ifndef GRAPHICCACHE

//=======================================================================================
// @bsimethod                                                    Mathieu.St-Pierre  04/10
//=======================================================================================
DTMQvCacheDetails* MrDTMDataRef::_GetDTMDetails
(
ElementHandleCR        el,
DTMDataRefPurpose   purpose,
ViewContextR       context,
DTMDrawingInfo&     drawingInfo
)
    {            
    ViewGeomInfoCP viewGeomInfoP = 0;
    bool isWireFrameDisplayMode = true; 

    if (NULL != context.GetViewport())
        {
        viewGeomInfoP = &context.GetViewport()->GetViewInfoCP()->GetGeomInfo();
        if (IsWireframeRendering(context) == false)
            { isWireFrameDisplayMode = false; }
        }

    //ElementHiliteState hiliteState = elementRef_getHiliteState(el.GetElementRef(), el.GetModelRef());    
    return new MrDTMQvCacheDetails(viewGeomInfoP,       
                                   context.GetLocalToView(),
                                   m_pointDensityForShadedView, 
                                   m_pointDensityForWireframeView,
                                   !isWireFrameDisplayMode && CanDrapeRasterTexture(),
                                   context.GetDrawPurpose());
    }

#endif 


//--------------------------------------------------------------------------------------
//IPlotterInfo interface definition
//--------------------------------------------------------------------------------------
DPoint2d IPlotterInfo::GetPrinterResolutionInInches () const
    {
    return _GetPrinterResolutionInInches ();
    }
//--------------------------------------------------------------------------------------
//IPlotterInfo interface definition - END
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
//IMrDTMDataRef interface definition
//--------------------------------------------------------------------------------------
    int IMrDTMDataRef::GetOverviewDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& overviewDTMPtr)
        {
        return _GetOverviewDTM(overviewDTMPtr);
        }   

    int IMrDTMDataRef::GetProgressiveDataBlockManager(IMrDTMProgressiveDataBlockManagerPtr& progressiveDataBlockManagerPtr,
                                                      BcDTMP                               dtmToLoadProgressively, 
                                                      ViewContextP                          context, 
                                                      bool                                  isDrapingRequired)
        {
        return _GetProgressiveDataBlockManager(progressiveDataBlockManagerPtr, dtmToLoadProgressively, context, isDrapingRequired);   
        }
    
    //Creation and data source modification functions
    int IMrDTMDataRef::GetIMrDTMCreator(Bentley::MrDTM::IMrDTMCreatorPtr& iMrDTMCreatorPtr)
        {
        return _GetIMrDTMCreator(iMrDTMCreatorPtr);
        }          

    int IMrDTMDataRef::SaveIMrDTMCreatorToFile()
        {
        return _SaveIMrDTMCreatorToFile();
        }

    //Information functions  
     const WString& IMrDTMDataRef::GetDescription() const
        {
        return _GetDescription();          
        }

    int IMrDTMDataRef::SetDescription(const WString& description)
        {
        return _SetDescription(description);
        }         
    
    const DgnDocumentPtr IMrDTMDataRef::GetMrDTMDocument() const
        {
        return _GetMrDTMDocument();          
        }

    int IMrDTMDataRef::SetMrDTMDocument(DgnDocumentPtr& mrdtmDocumentPtr)
        {
        return _SetMrDTMDocument(mrdtmDocumentPtr);
        }         
                   
     const DgnDocumentMonikerPtr IMrDTMDataRef::GetFileMoniker() const
        {
        return _GetFileMoniker();          
        }

     bool IMrDTMDataRef::CanLocate() const
        {
        return _CanLocate();
        }

    int IMrDTMDataRef::SetLocate(bool canLocate)
        {
        return _SetLocate(canLocate);
        }     

     bool IMrDTMDataRef::IsAnchored() const
        {
        return _IsAnchored();
        }

    int IMrDTMDataRef::SetAnchored(bool isAnchored)
        {
        return _SetAnchored(isAnchored);
        }

    MrDTMStatus IMrDTMDataRef::GetIDTMFileLastLoadStatus()
        {
        return _GetIDTMFileLastLoadStatus();
        }         
             
    bool IMrDTMDataRef::IsOpenInReadOnly()
        {
        return _IsOpenInReadOnly();
        }

    int IMrDTMDataRef::SetReadOnly(bool isReadOnly)
        {
        return _SetReadOnly(isReadOnly);
        }

     Bentley::GeoCoordinates::BaseGCSPtr IMrDTMDataRef::GetGeoCS()
        {
        return _GetGeoCS();         
        }

    int IMrDTMDataRef::SetGeoCS(const Bentley::GeoCoordinates::BaseGCSPtr& baseGCSPtr)
        {
        return _SetGeoCS(baseGCSPtr);
        }         

    int IMrDTMDataRef::GetExtentInGeoCS(DRange3d& extentInGeoCS, bool inUors)
        {
        return _GetExtentInGeoCS(extentInGeoCS, inUors);
        }
            
    //Display control function    

    int IMrDTMDataRef::GetEdgeMethod(int& edgeMethod)
        {
        return _GetEdgeMethod(edgeMethod);
        }

    double IMrDTMDataRef::GetEdgeMethodLengthInStorage()
        {
        return _GetEdgeMethodLengthInStorage();
        }

    int IMrDTMDataRef::GetEdgeMethodLength(double& edgeMethodLength)
        {
        return _GetEdgeMethodLength(edgeMethodLength);
        }

     bool IMrDTMDataRef::GetClipActivation() const
        {
        return _GetClipActivation();
        }

    int IMrDTMDataRef::SetClipActivation(bool isActivated)
        {
        return _SetClipActivation(isActivated);
        }              

    int IMrDTMDataRef::GetNbClips() const
        {
        return _GetNbClips();
        }

    int IMrDTMDataRef::GetClip(DPoint3d*& clipPointsP,
                               int&       numberOfPoints, 
                               bool&      isClipMask,
                               int        clipInd) const
        {
        return _GetClip(clipPointsP, numberOfPoints, isClipMask, clipInd);
        }

    int IMrDTMDataRef::AddClip(DPoint3d* clipPointsP,
                               int       numberOfPoints, 
                               bool      isClipMask)
        {
        return _AddClip(clipPointsP, numberOfPoints, isClipMask);    
        }

    int IMrDTMDataRef::RemoveClip(int toRemoveClipInd)
        {
        return _RemoveClip(toRemoveClipInd);
        }

    int IMrDTMDataRef::RemoveAllClip()
        {
        return _RemoveAllClip();
        }        
    
    double IMrDTMDataRef::GetPointDensityForShadedView()
        {
        return _GetPointDensityForShadedView();
        }                        

    int IMrDTMDataRef::SetPointDensityForShadedView(double pi_minScreenPixelsPerPoint)
        {
        return _SetPointDensityForShadedView(pi_minScreenPixelsPerPoint);
        }
    
    double IMrDTMDataRef::GetPointDensityForWireframeView()
        {
        return _GetPointDensityForWireframeView();
        }                        

    int IMrDTMDataRef::SetPointDensityForWireframeView(double pi_minScreenPixelsPerPoint)
        {
        return _SetPointDensityForWireframeView(pi_minScreenPixelsPerPoint);
        }
         
    //High resolution raster texturing     
    int IMrDTMDataRef::ClearCachedMaterials()
        {
        return _ClearCachedMaterials();
        }

    int IMrDTMDataRef::GetRasterTextureCacheFilePath(WString& cacheFilePath)
        {
        return _GetRasterTextureCacheFilePath(cacheFilePath);
        }

    int IMrDTMDataRef::GetRasterTexturePSSFilePath(WString& pssFilePath)
        {
        return _GetRasterTexturePSSFilePath(pssFilePath);
        }

    IRasterTextureSourcePtr IMrDTMDataRef::GetRasterTextureSource()
        {
        return _GetRasterTextureSource();
        }
     
    int IMrDTMDataRef::SetRasterTextureSource(IRasterTextureSourcePtr& rasterTextureSourcePtr)
        {
        return _SetRasterTextureSource(rasterTextureSourcePtr);
        }

    IMultiResolutionGridMaterialManagerPtr IMrDTMDataRef::GetMultiResGridMaterialManager()
        {
        return _GetMultiResGridMaterialManager();         
        }


    bool IMrDTMDataRef::IsVisibleForView(short viewNumber)
        {
        return _IsVisibleForView(viewNumber);
        }

    int IMrDTMDataRef::SetVisibilityForView(short viewNumber, bool isVisible)
        {
        return _SetVisibilityForView(viewNumber, isVisible);
        }
   
    //Synchronization functions

    //-	State (empty, dirty, up-to-date + time stamp)                        
            
    int IMrDTMDataRef::GetMrDTMState()
        {
        return _GetMrDTMState();
        }                        

    bool IMrDTMDataRef::GetLastUpToDateCheckTime(time_t& last)
        {
        return _GetLastUpToDateCheckTime(last);
        }
    
    int IMrDTMDataRef::Generate()
        {
        return _Generate();
        }   
            
    int IMrDTMDataRef::GenerateRasterTextureCache()
        {
        return _GenerateRasterTextureCache();
        }   

    int IMrDTMDataRef::GetRasterTextureCacheDecimationMethod(IRasterTextureSourceCacheFile::DecimationMethod& decimationMethod) const
        {
        return _GetRasterTextureCacheDecimationMethod(decimationMethod);
        }

    int IMrDTMDataRef::SetRasterTextureCacheDecimationMethod(IRasterTextureSourceCacheFile::DecimationMethod decimationMethod)
        {
        return _SetRasterTextureCacheDecimationMethod(decimationMethod);
        }   

    int IMrDTMDataRef::IsRasterTextureCacheFileUpToDate(bool& isUpToDate, Element::EditElementHandle* elmHandleP)
        {
        return _IsRasterTextureCacheFileUpToDate(isUpToDate, elmHandleP);
        }  

    bool IMrDTMDataRef::IsUpToDate()
        {
        return _IsUpToDate();
        }       

    int64_t IMrDTMDataRef::GetPointCountForResolution(int resolutionIndex)
        {
        return _GetPointCountForResolution(resolutionIndex);
        }  

    DgnGCSP IMrDTMDataRef::GetDestinationGCS()
        {
        return _GetDestinationGCS();
        }


//--------------------------------------------------------------------------------------
//IMrDTMDataRef interface definition - END
//--------------------------------------------------------------------------------------




//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 04/10
//=======================================================================================
void MrDTMDataRef::ScheduleFromDtmFile(EditElementHandleR elem, TransformCR trsf)
    {
    BeAssert(0);
    //MrDTMDataRef::ScheduleFromDtmFile(elem, m_fileName.GetWCharCP(), false, trsf);
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 04/10
//=======================================================================================
void MrDTMDataRef::UpdateFromDtmFile(EditElementHandleR elem)
    {
    MrDTMDataRef::UpdateFromDtmFileImpl(elem);
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 07/11
//=======================================================================================
int MrDTMDataRef::GetClippedExtent(DRange3d& clippedExtent)
    {    
    int result = SUCCESS;

    if ((m_clipContainerPtr != 0) && (m_clipContainerPtr->GetNbClips() > 0) && (m_pGetClippedExtentEstimation == 0))
        {
        BcDTMPtr singleResolutionDtm;

        bool queryResult = GetDtmForSingleResolution(singleResolutionDtm, 
                                                     MAX_NB_POINTS_FOR_CLIPPED_EXTENT_ESTIMATION);

        if (queryResult == true)
            {
            BeAssert((singleResolutionDtm != 0));

            m_pGetClippedExtentEstimation  = new DRange3d;
            
            DTMStatusInt status = singleResolutionDtm->GetRange(*m_pGetClippedExtentEstimation);

            BeAssert(status == DTM_SUCCESS);

            Transform trsf;

            getStorageToUORMatrix(trsf, GetElement().GetModelRef(), GetElement(), false);
            
            bsiTransform_multiplyDPoint3dArrayInPlace(&trsf, (DPoint3dP)m_pGetClippedExtentEstimation, 2);
            }     
        else
            {
            result = ERROR;
            }
        }

    if (result == SUCCESS)
        {
        if (m_pGetClippedExtentEstimation == 0)
            {                
            _GetExtents(clippedExtent);       

            Transform trsf;

            getStorageToUORMatrix(trsf, GetElement().GetModelRef(), GetElement(), false);
            
            bsiTransform_multiplyDPoint3dArrayInPlace(&trsf, (DPoint3dP)&clippedExtent, 2);
            }
        else
            {         
            clippedExtent = *m_pGetClippedExtentEstimation;
            }
        }
  
    return result;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 09/11
//=======================================================================================
int MrDTMDataRef::_GetOverviewDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& overviewDTMPtr)
    {        
    int status = ERROR;

    if (m_overviewDTMPtr == 0)
        {
        BcDTMPtr singleResolutionDtm;
            
        bool status = GetDtmForSingleResolution(singleResolutionDtm, MAX_NB_POINTS_FOR_OVERVIEW);

        if ((status != false) && (singleResolutionDtm != 0))
            {
            m_overviewDTMPtr = singleResolutionDtm;
            }        
        }

    if (m_overviewDTMPtr != 0)
        {
        overviewDTMPtr = m_overviewDTMPtr;
        status = SUCCESS;
        }
       
    return status;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 07/11
//=======================================================================================
int MrDTMDataRef::_GetProgressiveDataBlockManager(IMrDTMProgressiveDataBlockManagerPtr& progressiveDataBlockManagerPtr,
                                                 BcDTMP                               dtmToLoadProgressively, 
                                                 ViewContextP                         context, 
                                                 bool                                  isDrapingRequired)
    {
    BeAssert((context != 0) && (context->GetViewport() != 0));

    int view = context->GetViewport()->GetViewNumber(); 
                        
    if (!m_viewData[view])
        {
        m_viewData[view] = new MrDTMViewData(this);        
        }

    int status = m_viewData[view]->GetProgressiveDataBlockManager(progressiveDataBlockManagerPtr,
                                                                  dtmToLoadProgressively, 
                                                                  context, 
                                                                  isDrapingRequired);
        
    return status;    
    }




//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 09/10
//=======================================================================================
bool MrDTMDataRef::CanDrapeRasterTexture()
    {
    bool canDrapeRasterTexture = false;

    if (MrDTMDataRef::_GetMultiResGridMaterialManager() != 0)
        {                
        canDrapeRasterTexture = true;
        }

    return canDrapeRasterTexture;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void MrDTMDataRef::ScheduleFromDtmFile (DgnModelRefP dgnModelRefP, EditElementHandleR elem, const DgnDocumentPtr& mrdtmDocumentPtr, bool inCreation)
    {    
    Transform inTrfs;
        
    memset(&inTrfs, 0, sizeof(Transform));
        
    //Dummy transformation. Need to be set once the MrDTMDataRef has been created.
    (inTrfs).form3d[0][0] = 1.0;
    (inTrfs).form3d[1][1] = 1.0;
    (inTrfs).form3d[2][2] = 1.0;
       
    DTMElementHandlerManager::CheckAndCreateElementDescr (elem, nullptr, MrDTMDefaultElementHandler::GetElemHandlerId(), inTrfs, *dgnModelRefP);    
                    
    elem.SetModelRef(dgnModelRefP);
    elem.AddToModel();            

    EditElementHandle editElementHandle(elem, false);

    bool scheduling = true;
    
    XAttributeHandlerId xAttrHandlerId(MrDTMElementMajorId, XATTRIBUTES_SUBID_MRDTM_DETAILS);

    editElementHandle.ScheduleWriteXAttribute(xAttrHandlerId, MRDTM_DETAILS_SCHEDULING, (uint32_t)sizeof(scheduling), &scheduling);
    
    //Create the MrDTMDataRef 
    RefCountedPtr<DTMDataRef> dtmDataRef(MrDTMDataRef::FromElemHandle(editElementHandle, mrdtmDocumentPtr, inCreation));    
    if (0 == dtmDataRef.get()) 
        {
        BeAssert(!"Missing error handling! This method should consider returning a status...");
        throw std::runtime_error("Could not access STM data ref!");
        }

    ElementHandle::XAttributeIter xAttrIter(editElementHandle, xAttrHandlerId, MRDTM_DETAILS_MRDTM_CREATED);
    
    if (!xAttrIter.IsValid())
        {
        bool isCreated = true;        
        editElementHandle.ScheduleWriteXAttribute(xAttrHandlerId, MRDTM_DETAILS_MRDTM_CREATED, (uint32_t)sizeof(isCreated), &isCreated);
        }
    
    ((MrDTMDataRef*)dtmDataRef.get())->UpdateStorageToUORMatrix(editElementHandle);

    DTMElementHandlerManager::CreateDefaultDisplayElements (editElementHandle);
               
              
    dtmDataRef = 0; // Release data ref in order not to hamper proper data ref loading that will follows
  
    UpdateFromDtmFileImpl(editElementHandle);     
    }

//=======================================================================================
// @bsimethod                                                     Mathieu.St-Pierre 08/11
//=======================================================================================
int MrDTMDataRef::UpdateStorageToUORMatrix(EditElementHandleR elemHandle)
    {
    // We rebuild the transformation from target GCS to UOR
    double convFactor = dgnModel_getUorPerMeter (m_element.GetModelRef()->GetDgnModelP ()); 

    Bentley::GeoCoordinates::BaseGCSPtr sourceGCSPtr(_GetGeoCS());

    m_destinationGCSP = GetTargetGCS();

    //If the STM has no GCS it is assumed that its data are in meters. 
    if ((m_destinationGCSP != 0) && (m_destinationGCSP->IsValid() == true) &&         
        (sourceGCSPtr != 0) && (sourceGCSPtr->IsValid() == true))
        {
        // A target GCS is set ... we obtain the transformation from this
        // GCS to meter        
        WString tempUnit;
        Bentley::GeoCoordinates::UnitCP theUnit = Unit::FindUnit (m_destinationGCSP->GetUnits(tempUnit));

        convFactor *= theUnit->GetConversionFactor();   
        }   
    else if ((sourceGCSPtr != 0) && (sourceGCSPtr->IsValid() == true))
        {
        WString tempUnit;
        Bentley::GeoCoordinates::UnitCP theUnit = Unit::FindUnit (sourceGCSPtr->GetUnits(tempUnit));

        convFactor *= theUnit->GetConversionFactor();
        }

    Transform inTrfs;
    
    memset(&inTrfs, 0, sizeof(Transform));
    
    (inTrfs).form3d[0][0] = convFactor;
    (inTrfs).form3d[1][1] = convFactor;
    //MST : The same factor should not necessary be used for the Z, but currently 
    //      we have no information to set it properly.
    (inTrfs).form3d[2][2] = convFactor;
    
    const DPoint3d globalOrigin = m_element.GetModelRef()->GetDgnModelP()->GetModelInfo().GetGlobalOrigin(); 

    (inTrfs).form3d[0][3] = globalOrigin.x;
    (inTrfs).form3d[1][3] = globalOrigin.y;
    (inTrfs).form3d[2][3] = globalOrigin.z;
    
    
    Transform currentTrsf;
    memset(&currentTrsf, 0, sizeof(Transform));

    getStorageToUORMatrix(currentTrsf, elemHandle);

    //TR 353153 - Don't update the storage matrix if it hasn't change so as to no change the last modified time of the 
    //            .dgn file.
    if (memcmp(&inTrfs, &currentTrsf, sizeof(Transform)) != 0)
        {
        setStorageToUORMatrix(inTrfs, elemHandle);    
        }
        
    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void MrDTMDataRef::UpdateFromDtmFileImpl (EditElementHandleR elem)
    {    
    RefCountedPtr<DTMDataRef> ref;

    DTMElementHandlerManager::GetDTMDataRef (ref, elem);

    RefCountedPtr<MrDTMDataRef> mrRef = (MrDTMDataRef*)ref.get();
    
    if (elem.GetDisplayHandler ())
        elem.GetDisplayHandler ()->ValidateElementRange (elem, true);
    
    //Need to call replace in model here otherwise the range is not 
    //written correctly to the element.
    elem.ReplaceInModel(elem.GetElementRef());      
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
MrDTMDataRef::MrDTMDataRef(ElementHandleCR el, const DgnDocumentPtr& mrdtmDocumentPtr, bool inCreation)
    {                   
    int status;     

    for (int i = 0; i < NUMBER_OF_PP_VIEWS; i++)
        m_viewData[i] = nullptr;
    
    m_xAttrMrDTMDetailsHandlerId = XAttributeHandlerId(MrDTMElementMajorId, XATTRIBUTES_SUBID_MRDTM_DETAILS);                    
    m_iDTMFileLastLoadStatus = MRDTMSTATUS_FILE_LOADED;

    //TR 322637 - Get an estimation of the clipped extent by doing the clipped query of some lower 
    //            resolution data and taking the extent of the resulting DTM. m_pGetClippedExtentEstimation 
    //            is only initialized if required.
    m_pGetClippedExtentEstimation = 0;

    m_element = ElementHandle(el);
    
    //Create an element descriptor to ensure that XAttribute are written back to the file.
    m_element.GetElementDescrCP();
    
    EditElementHandle editElementHandle(m_element, false);

    ElementHandle::XAttributeIter xAttrIterReadOnly(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_IS_READ_ONLY);
    
    // Verify if the dgn is read only
    if (el.GetModelRef()->IsCacheReadonly())
        m_isReadOnly = true;
    else
        {
        //The read only flag need to be read before the file is read.
        if (xAttrIterReadOnly.IsValid())
            {
            memcpy(&m_isReadOnly, (byte*)xAttrIterReadOnly.PeekData(), sizeof(m_isReadOnly));       
            }
        else 
            {
            //Default value
            m_isReadOnly = !inCreation; 

            editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_IS_READ_ONLY, (uint32_t)sizeof(m_isReadOnly), &m_isReadOnly);        
            }    
        }    
        
    //Read the file moniker
    ElementHandle::XAttributeIter xAttrIterMoniker(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_FILE_MONIKER);
    if (xAttrIterMoniker.IsValid())
        {
        BeAssert(inCreation == false);
        BeAssert(mrdtmDocumentPtr == 0);

        status = ReadMonikerFromSerializationData((byte*)xAttrIterMoniker.PeekData(), xAttrIterMoniker.GetSize(), m_element.GetModelRef());

        BeAssert(status == 0);
        }
    else
        {        
        BeAssert(mrdtmDocumentPtr != 0);
        
        m_mrdtmDocumentPtr = mrdtmDocumentPtr;                
        m_mrdtmMonikerPtr = m_mrdtmDocumentPtr->GetMonikerPtr();                

        DataExternalizer descriptionDataExternalizer;
        
        WString monikerString(m_mrdtmMonikerPtr->Externalize());

        descriptionDataExternalizer.put(monikerString);

        editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_FILE_MONIKER, (uint32_t)descriptionDataExternalizer.getBytesWritten(), descriptionDataExternalizer.getBuf());            
        }   
        
    if (inCreation == true)
        {                    
        WString fileName(m_mrdtmDocumentPtr->GetFileName());
        
        m_mrDTMCreator = IMrDTMCreator::GetFor(fileName.GetWCharCP()); 
      
        memset(&m_minPoint, 0, sizeof(m_minPoint));            
        memset(&m_maxPoint, 0, sizeof(m_maxPoint));                    
        }
    else
        {                       
        //Load the file when the MrDTMDataRef is loaded.     
        ReadFile();

        BeAssert((m_mrDTM != 0) || (m_iDTMFileLastLoadStatus != MRDTMSTATUS_FILE_LOADED));                
        }
       
    ElementHandle::XAttributeIter xAttrIterMinPoint(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_MIN_POINT);
    
    //TR 334171 - Currently the min points and max points are recompute each time the file is 
    //            read if the m_mrDTM is not null (see the ReadFile function call above).
    if (xAttrIterMinPoint.IsValid() && (m_mrDTM == 0))
        {
        memcpy(&m_minPoint, (byte*)xAttrIterMinPoint.PeekData(), sizeof(m_minPoint));
        }
    else
        {                
        editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_MIN_POINT, (uint32_t)sizeof(m_minPoint), &m_minPoint);                   
        }

    ElementHandle::XAttributeIter xAttrIterMaxPoint(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_MAX_POINT);
    
    if (xAttrIterMaxPoint.IsValid() && (m_mrDTM == 0))
        {
        memcpy(&m_maxPoint, (byte*)xAttrIterMaxPoint.PeekData(), sizeof(m_maxPoint));
        }
    else        
        {        
        editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_MAX_POINT, (uint32_t)sizeof(m_maxPoint), &m_maxPoint);        
        }
    
    ElementHandle::XAttributeIter xAttrIterPointDensity(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_POINT_DENSITY);
    if (xAttrIterPointDensity.IsValid())
        {            
        DataInternalizer source ((byte*)xAttrIterPointDensity.PeekData(), xAttrIterPointDensity.GetSize());       
        source.get(&m_pointDensityForShadedView);        
        source.get(&m_pointDensityForWireframeView);           
        }
    else
        {        
        //Default values                
        m_pointDensityForShadedView = MRDTM_GUI_TO_VIEW_POINT_DENSITY(100);
        m_pointDensityForWireframeView = MRDTM_GUI_TO_VIEW_POINT_DENSITY(50);                

        WriteXAttributePointDensityData(false, &editElementHandle);        
        }

    //Read the description
    ElementHandle::XAttributeIter xAttrIterDescription(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_DESCRIPTION);    
    if (xAttrIterDescription.IsValid())
        {
        DataInternalizer source ((byte*)xAttrIterDescription.PeekData(), xAttrIterDescription.GetSize());

        source.get(m_description);        
        }
    else
        {
        DataExternalizer descriptionDataExternalizer;

        descriptionDataExternalizer.put(m_description);
            
        editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_DESCRIPTION, (uint32_t)descriptionDataExternalizer.getBytesWritten(), descriptionDataExternalizer.getBuf());        
        }
    
    ElementHandle::XAttributeIter xAttrIterCanLocate(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_CAN_LOCATE);    
    if (xAttrIterCanLocate.IsValid())
        {
        memcpy(&m_canLocate, (byte*)xAttrIterCanLocate.PeekData(), sizeof(m_canLocate));       
        }
    else
        {
        //Default value
        m_canLocate = true;

        editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_CAN_LOCATE, (uint32_t)sizeof(m_canLocate), &m_canLocate);        
        }

    ElementHandle::XAttributeIter xAttrIterAnchored(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_IS_ANCHORED);    
    if (xAttrIterAnchored.IsValid())
        {
        memcpy(&m_isAnchored, (byte*)xAttrIterAnchored.PeekData(), sizeof(m_isAnchored));       
        }
    else
        {
        //Default value
        m_isAnchored = true;

        editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_IS_ANCHORED, (uint32_t)sizeof(m_isAnchored), &m_isAnchored);        
        }
    
    ElementHandle::XAttributeIter xAttrIterVisibilityPerView(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_VISIBILITY_PER_VIEW);    
    if (xAttrIterVisibilityPerView.IsValid())
        {
        memcpy(&m_visibilityPerView, (byte*)xAttrIterVisibilityPerView.PeekData(), sizeof(m_visibilityPerView));       
        }
    else
        {
        //Default value
        memset(&m_visibilityPerView, 1, sizeof(m_visibilityPerView));                                   
        editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_VISIBILITY_PER_VIEW, (uint32_t)sizeof(m_visibilityPerView), &m_visibilityPerView);        
        }   
    
    ElementHandle::XAttributeIter xAttrIterClip(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_CLIPS);    
    if (xAttrIterClip.IsValid())
        {                                  
        status = ReadClipsFromSerializationData((byte*)xAttrIterClip.PeekData(), xAttrIterClip.GetSize());    
        }
    else
        {
        //Default value.
        m_isClipActivated = true;

        status = SaveClipsInXAttribute(false, &editElementHandle);

        BeAssert(status == 0);
        }   

    ElementHandle::XAttributeIter xAttrIterDrapingRasters(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_DRAPING_RASTERS); 
    ElementHandle::XAttributeIter xAttrIterDrapingRastersUnicode(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_DRAPING_RASTERS_UNICODE); 
    if (xAttrIterDrapingRasters.IsValid() || xAttrIterDrapingRastersUnicode.IsValid())
        {      
        DataInternalizer drapingRasters;
        
        if (xAttrIterDrapingRastersUnicode.IsValid())
            {
            drapingRasters.setBuffer((byte*)xAttrIterDrapingRastersUnicode.PeekData(), xAttrIterDrapingRastersUnicode.GetSize());
            }
        else
            {
            drapingRasters.setBuffer((byte*)xAttrIterDrapingRasters.PeekData(), xAttrIterDrapingRasters.GetSize());
            }

        bool    isRasterTextureSourceExist = true;
        int     nbRasterFiles;        
        WString persistedRasterFileName;
   //     long   index;

        drapingRasters.get(&nbRasterFiles);

        //Only one raster is supported at this time.
        BeAssert(nbRasterFiles == 1);        
                
        StringList* drapingRasterFileNamesP = mdlStringList_create(nbRasterFiles, 0);

        for (long rasterFileInd = 0; rasterFileInd < nbRasterFiles; rasterFileInd++)
            {              
            if (xAttrIterDrapingRastersUnicode.IsValid())
                {                    
                drapingRasters.get(persistedRasterFileName);
                }
            else
                {
                persistedRasterFileName = WString(drapingRasters.getString ());
                }                        
            
            if (_waccess(persistedRasterFileName.c_str(), 00) != 0)
                {
                isRasterTextureSourceExist = false;
                break;
                }
                        
            status = mdlStringList_setMember(drapingRasterFileNamesP,
                                             rasterFileInd, 
                                             persistedRasterFileName.c_str(), 
                                             0);

            BeAssert(status == SUCCESS);                       
            }                  

        if (isRasterTextureSourceExist == true)
            {
            LoadRasterTextureSource(drapingRasterFileNamesP);
            
			if ((m_rasterDrapingSrcPtr == 0) && (DTMElementHandlerManager::GetRasterTextureSourceManager() != 0))
            	{
                SetWarningForLoadStatus(MRDTMSTATUS_MISSING_RASTER_DRAPING_FILE);            
                }
            }
        else
            {
            SetWarningForLoadStatus(MRDTMSTATUS_MISSING_RASTER_DRAPING_INFO);            
            }

        long destroyStatus = mdlStringList_destroy(drapingRasterFileNamesP);

        BeAssert(destroyStatus == 0);
        }
    else
        {
        WString pssFileName;

        int status = _GetRasterTexturePSSFilePath(pssFileName);

        if (_waccess(pssFileName.GetWCharCP(), 00) == 0)
            {            
            StringList* drapingRasterFileNamesP = mdlStringList_create(1, 0);

            string pssFileNameMultiByte;
              
            status = mdlStringList_setMember(drapingRasterFileNamesP,
                                             0, 
                                             pssFileName.c_str(), 
                                             0);

            BeAssert(status == SUCCESS);

            if (LoadRasterTextureSource(drapingRasterFileNamesP) == true)
                {
                DataExternalizer descriptionDataExternalizerUnicode;   
                descriptionDataExternalizerUnicode.put(1); 
                descriptionDataExternalizerUnicode.put(pssFileName.c_str());     

                editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_DRAPING_RASTERS_UNICODE, (uint32_t)descriptionDataExternalizerUnicode.getBytesWritten(), descriptionDataExternalizerUnicode.getBuf());                           

                char* pssFileNameMultiByte = new char[pssFileName.GetMaxLocaleCharBytes()];
                
                pssFileName.ConvertToLocaleChars (pssFileNameMultiByte, pssFileName.GetMaxLocaleCharBytes());                

                DataExternalizer descriptionDataExternalizer;   
                descriptionDataExternalizer.put(1); 
                descriptionDataExternalizer.put(pssFileNameMultiByte);     

                editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_DRAPING_RASTERS, (uint32_t)descriptionDataExternalizer.getBytesWritten(), descriptionDataExternalizer.getBuf());                           

                delete [] pssFileNameMultiByte;
                }

            long destroyStatus = mdlStringList_destroy(drapingRasterFileNamesP);

            BeAssert(destroyStatus == 0);
            }                             
        }

    ElementHandle::XAttributeIter xAttrIterRasterTextureCacheDecimationMethod(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_RASTER_TEXTURE_CACHE_DECIMATION_METHOD); 
    if (xAttrIterRasterTextureCacheDecimationMethod.IsValid())
        {   
        memcpy(&m_rasterTextureCacheDecimationMethod, (byte*)xAttrIterRasterTextureCacheDecimationMethod.PeekData(), sizeof(m_rasterTextureCacheDecimationMethod));                           
        }    
    else
        {   
        IRasterTextureSourceCacheFilePtr rasterTextureSourceCacheFilePtr;

        if (_GetRasterTextureSource() != 0)
            {            
            rasterTextureSourceCacheFilePtr = _GetRasterTextureSource()->GetCacheFileInterface();
            }
                       
        if (rasterTextureSourceCacheFilePtr != 0)
            {
            //Obtained the value from the cTIFF cache file.            
            int status = rasterTextureSourceCacheFilePtr->GetDecimationMethod(m_rasterTextureCacheDecimationMethod);        

            BeAssert(status == SUCCESS);            
            }
        else
            {
            //Default value            
            m_rasterTextureCacheDecimationMethod = IRasterTextureSourceCacheFile::NOT_SPECIFIED_DECIMATION;                
            }
        
        editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_RASTER_TEXTURE_CACHE_DECIMATION_METHOD, (uint32_t)sizeof(m_rasterTextureCacheDecimationMethod), &m_rasterTextureCacheDecimationMethod);
        }


    ElementHandle::XAttributeIter xAttrIterRasterTextureCacheState(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_RASTER_TEXTURE_CACHE_STATE); 

    if (xAttrIterRasterTextureCacheState.IsValid())
        {   
        memcpy(&m_rasterTextureCacheState, (byte*)xAttrIterRasterTextureCacheState.PeekData(), sizeof(m_rasterTextureCacheState));           
        }    
    else
        {        
        bool isUpToDate;

        //Both raster draping related XAttribute must be present in other to correctly state create MRDTM_DETAILS_RASTER_TEXTURE_CACHE_STATE.
        _IsRasterTextureCacheFileUpToDate(isUpToDate, &editElementHandle);                         
        }


    ElementHandle::XAttributeIter xAttrIterTriangulationEdgeOptions(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_TRIANGULATION_EDGE_OPTIONS); 
    if (xAttrIterTriangulationEdgeOptions.IsValid())
        {   
        DataInternalizer source ((byte*)xAttrIterTriangulationEdgeOptions.PeekData(), xAttrIterTriangulationEdgeOptions.GetSize());       
            
        source.get(&m_edgeMethod);        
        source.get(&m_edgeMethodLength);                   
        }    
    else
        {        
        //Default value
        m_edgeMethod = 0;                            
        m_edgeMethodLength = 1000 * dgnModel_getUorPerMeter(editElementHandle.GetDgnModelP());                

        WriteXAttributeTriangulationEdgeOptions(false, &editElementHandle);        
        }
                
    m_GCSEventHandler = new GcoordGeoCoordinateEventHandler(this);
    Bentley::GeoCoordinates::DgnGCS::SetEventHandler(m_GCSEventHandler);

    m_destinationGCSP = GetTargetGCS();      
    
    BeAssert(status == 0);    
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
MrDTMDataRef::~MrDTMDataRef()
    {
    Bentley::GeoCoordinates::DgnGCS::RemoveEventHandler(m_GCSEventHandler);

    FlushAllViewData();         

    //Is currently cleaned up by FlushAllViewData.
    BeAssert(m_pGetClippedExtentEstimation == 0);
    
    int status = -1;

    //Ensure that the sources are saved to the file.
    if (m_mrDTMCreator != 0)
        {
        status = _SaveIMrDTMCreatorToFile();

        BeAssert(status == 0);
        }           
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
void MrDTMDataRef::AdviseGCSChanged()
    {
    m_destinationGCSP = GetTargetGCS();
    ReloadMrDTM(m_destinationGCSP);
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
void MrDTMDataRef::Reproject(DgnGCSP sourceGCS, DgnGCSP targetGCS)
    {   
    // Explanation on the weird cases implemented below.
    // Since clips are stored as UORs in the DGNModel coordinate yet refer to data
    // expressed using an external non-UOR based geographic coordinate system
    // in order for clips to follow the STM upon model GCS deletion, we reproject
    // in such case (where targetGCS is NULL) to the STM source GCS if there is one.
    // In the case the GCS is deleted (targetGCS NULL) and the STM has no GCS then
    // we bypass completely clip reprojection as there were none performed on the STM and 
    // none will be afterwards.
    m_destinationGCSP = targetGCS;

    DgnGCSP effectiveTargetGCS = targetGCS;
    DgnGCSP effectiveSourceGCS = sourceGCS;

    DgnGCSPtr mrdtmGCSPtr;

    // Check if there is a target GCS (If the GCS has not been deleted)
    if (targetGCS == NULL)
    {
        if ((0 != m_mrDTM.get()) &&
            (m_mrDTM->GetBaseGCS() != NULL) && 
            m_mrDTM->GetBaseGCS()->IsValid())
        {
            // The target has been deleted ... We use the MRDTM file source GCS as target 
            // This means that since now no reprojection will occur we move the clips to 
            // the source MRDTM coordinate system
            mrdtmGCSPtr = DgnGCS::CreateGCS(&*m_mrDTM->GetBaseGCS(), GetElement().GetModelRef());
            effectiveTargetGCS = &*mrdtmGCSPtr;
        }
    }

    // Check if there is a target GCS (If the GCS has not been deleted)
    if (sourceGCS == NULL)
    {
        if ((0 != m_mrDTM.get()) &&
            (m_mrDTM->GetBaseGCS() != NULL) && 
            m_mrDTM->GetBaseGCS()->IsValid())
        {
            // The target has been deleted ... We use the MRDTM file source GCS as target 
            // This means that since now no reprojection will occur we move the clips to 
            // the source MRDTM coordinate system
            mrdtmGCSPtr = DgnGCS::CreateGCS(&*m_mrDTM->GetBaseGCS(), GetElement().GetModelRef());
            effectiveSourceGCS = &*mrdtmGCSPtr;
        }
    }

    if ((effectiveTargetGCS != NULL) && (effectiveSourceGCS != NULL))
    {
	    // If there are clip shapes we need to reproject the clip shapes also
        if ((m_clipContainerPtr != 0) && (m_clipContainerPtr->GetNbClips() > 0) )
            {
            // Create a reprojection object
            for (size_t indexClips = 0 ; indexClips < m_clipContainerPtr->GetNbClips(); indexClips++)
                {
                IMrDTMClipInfoPtr clipInfoP;

                int status = m_clipContainerPtr->GetClip(clipInfoP, indexClips);

                assert(status == SUCCESS);
                
                for (size_t indexPoints = 0 ; indexPoints < clipInfoP->GetNbClipPoints(); indexPoints++)
                    {
                    DPoint3d newPoint;
                    effectiveSourceGCS->ReprojectUors(&newPoint, NULL, NULL, &(clipInfoP->GetClipPoints()[indexPoints]), 1, *effectiveTargetGCS);
                    clipInfoP->GetClipPoints()[indexPoints] = newPoint;
                    }

                }
            int status = SaveClipsInXAttribute();

            BeAssert(status == 0);

            FlushAllViewData();    
            }
        }

    ReloadMrDTM(targetGCS);
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/11
//=======================================================================================
void MrDTMDataRef::FlushAllViewData()
    {
    for (int i = 0; i < NUMBER_OF_PP_VIEWS; i++)
        {
        if (m_viewData[i])
            {
            delete m_viewData[i];
            m_viewData[i] = 0;
            }
        }      

    m_overviewDTMPtr = 0;

    //Remove all cached block
    if (IMrDTMDataRef::GetMrDTMProgressiveDisplayInterface() != 0)
        {
        IMrDTMDataRef::GetMrDTMProgressiveDisplayInterface()->ClearCachedBlocks(this, false);
        }
    
    // The next line fixes bug TFS#16393. Maybe it will be possible to delete this line when the previous one (DTMDataRefCachingManager::DeleteCacheElem) is
    // reintegrated into the code; presently it is left out because Descartes is not ported yet.
    if (m_element.IsPersistent())
        {
        DTMDisplayCacheManager::DeleteCacheElem (m_element);
        }

    //Delete the clipped extent estimation. 
    if (m_pGetClippedExtentEstimation != 0) 
        {
        delete m_pGetClippedExtentEstimation;
        m_pGetClippedExtentEstimation = 0;
        }
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 11/10
//=======================================================================================
bool MrDTMDataRef::CanDisplayMrDTM()
    {                  
    BeAssert(m_mrdtmDocumentPtr != 0 || m_mrDTM == 0);

    return (m_mrDTM != 0) && 
           (_waccess(m_mrdtmDocumentPtr->GetFileName().GetWCharCP(), 04) == 0);        
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
RefCountedPtr<DTMDataRef> MrDTMDataRef::FromElemHandle(ElementHandleCR elemHandle, const DgnDocumentPtr& mrdtmDocumentPtr, bool inCreation)
    {
    if (!IsValidMrDTMElement(elemHandle))
        return NULL;
   
    // TR #334173. Do not cache "inCreation" STMs as it prevents further attempts to open
    // same file in read only. As we do not cache it yet, file in create mode won't live
    // long enough to prevent other attempts at opening the same file.
    if (inCreation)
        return new MrDTMDataRef(elemHandle, mrdtmDocumentPtr, inCreation);

    // Do not cache non persistent element.
    if(elemHandle.GetElementRef() == NULL)
        return new MrDTMDataRef(elemHandle, mrdtmDocumentPtr, inCreation);

    DTMDataRef* pDataRef;

    // Don't cache using a DgnAttachment modelref.
    if(elemHandle.GetModelRef()->AsDgnAttachmentCP() != NULL)
        {
        BeAssert(elemHandle.PeekElementDescrCP() == NULL);  // We assumed a non modify elementhandle.
        ElementHandle eh(elemHandle.GetElementRef());
        pDataRef = new MrDTMDataRef(eh, mrdtmDocumentPtr, inCreation);
        DTMDataRef::AddDTMAppData (eh, pDataRef);
        }
    else
        {
        pDataRef = new MrDTMDataRef(elemHandle, mrdtmDocumentPtr, inCreation);
        DTMDataRef::AddDTMAppData (elemHandle, pDataRef);
        }   
    
    return pDataRef;
    }


//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
bool IMrDTMDataRef::IsValidMrDTMElement(ElementHandleCR elmHandle)
    {
    if (dynamic_cast<DTMElementHandler*>(&elmHandle.GetHandler()) != 0)
        {        
        XAttributeHandlerId handlerId (MrDTMElementMajorId, XATTRIBUTES_SUBID_MRDTM_DETAILS);
        ElementHandle::XAttributeIter xAttrIter(elmHandle, handlerId, MRDTM_DETAILS_SCHEDULING);
        
        return xAttrIter.IsValid();
        }

    return false;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 05/11
//=======================================================================================
int MrDTMDataRef::AddClipOnQuery(IMrDTMQueryPtr& queryPtr)
    {
    if ((_GetClipActivation() == true) && (_GetNbClips() > 0) && (queryPtr != 0))
        {
        DPoint3d* clipPointsP = 0;
        int       numberOfClipPoints;
        bool      isClipMask;
        Transform trsf;
        Transform invertTrsf;
        int       status;
        
        getStorageToUORMatrix(trsf, GetElement().GetModelRef(), GetElement(), false);

        bool invert = bsiTransform_invertTransform(&invertTrsf, &trsf);

        BeAssert(invert != 0);
        
        for (int clipInd = 0; clipInd < _GetNbClips(); clipInd++)
            {
            status = GetClip(clipPointsP, numberOfClipPoints, isClipMask, clipInd);
            
            BeAssert(clipPointsP != 0);        

            bsiTransform_multiplyDPoint3dArrayInPlace(&invertTrsf, clipPointsP, numberOfClipPoints);
            
            queryPtr->AddClip(clipPointsP, numberOfClipPoints, isClipMask);    
                                    
            delete clipPointsP;   

            clipPointsP = 0;
            }
        }

    return 0;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 11/11
//=======================================================================================
void MrDTMDataRef::GetDefaultMinimumPixelSizeInUORS(DPoint2d& minimumPixelSizeInUORS,
                                                    BaseGCSP  gcsForDefaultPixelSizeP)
    {  
    WString defaultMinimumPixelSizeForDraping;
    const bool foundCfgVar = BSISUCCESS == ConfigurationManager::GetVariable (defaultMinimumPixelSizeForDraping, 
                                                                              L"STM_DEFAULT_MINIMUM_PIXEL_SIZE_FOR_DRAPING");
    double defaultMinimumPixelSizeInMeters;
    if (!foundCfgVar ||
        0 >= (defaultMinimumPixelSizeInMeters = _wtof(defaultMinimumPixelSizeForDraping.c_str())))
        defaultMinimumPixelSizeInMeters = DEFAULT_MINIMUM_PIXEL_SIZE_FOR_DRAPING;
    
    const double uorPerMeters = dgnModel_getUorPerMeter(m_element.GetModelRef()->GetRoot());

    //Geographic - value should be in latitude/longitude
    if (gcsForDefaultPixelSizeP != 0)
        {
        //Geographic coordinate system.
        if (gcsForDefaultPixelSizeP->GetProjectionCode() == GeoCoordinates::BaseGCS::pcvUnity)
            {
            minimumPixelSizeInUORS.x = defaultMinimumPixelSizeInMeters / APPROX_NB_METERS_FOR_ONE_DEGREE * uorPerMeters;
            minimumPixelSizeInUORS.y = minimumPixelSizeInUORS.x;
            }
        else
            {
            minimumPixelSizeInUORS.x = defaultMinimumPixelSizeInMeters * uorPerMeters;                    
            minimumPixelSizeInUORS.y = minimumPixelSizeInUORS.x;                    
            }                
        }
    else
        {
        //The STM has no GCS, so the GCS is assumed to be in meters.
        minimumPixelSizeInUORS.x = defaultMinimumPixelSizeInMeters * uorPerMeters;
        minimumPixelSizeInUORS.y = defaultMinimumPixelSizeInMeters * uorPerMeters;               
        }           
    }


//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 04/11
//=======================================================================================
void MrDTMDataRef::NotifyChange()
    {
    ChangeListeners::iterator changeListenerIter(s_changeListeners.begin());
    ChangeListeners::iterator changeListenerIterEnd(s_changeListeners.end());
    
    while (changeListenerIter != changeListenerIterEnd)
        {
        (*(*changeListenerIter))(GetElement());
        changeListenerIter++;
        }
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 04/11
//=======================================================================================
void MrDTMDataRef::SetWarningForLoadStatus(MrDTMStatus warningStatus)
    {
    BeAssert((warningStatus > MRDTMSTATUS_WARNING_BEGIN) && (warningStatus < MRDTMSTATUS_WARNING_END));

    if ((m_iDTMFileLastLoadStatus < MRDTMSTATUS_ERROR_BEGIN) || (m_iDTMFileLastLoadStatus > MRDTMSTATUS_ERROR_END))
        {
        m_iDTMFileLastLoadStatus = warningStatus;
        }
    }

//=======================================================================================
// @bsimethod                                                   Chantal.Poulin 06/11
//=======================================================================================
int64_t MrDTMDataRef::_GetPointCountForResolution(int resolutionIndex)
    { 
    IMrDTMFixResolutionIndexQueryParamsPtr queryParamsPtr(IMrDTMFixResolutionIndexQueryParams::CreateParams());
    queryParamsPtr->SetResolutionIndex(resolutionIndex); 

    queryParamsPtr->SetEdgeOptionTriangulationParam(m_edgeMethod);                
    queryParamsPtr->SetMaxSideLengthTriangulationParam(_GetEdgeMethodLengthInStorage());

    //Get query interface
    IMrDTMQueryPtr fixResQueryPtr = m_mrDTM->GetQueryInterface(DTM_QUERY_FIX_RESOLUTION_VIEW, DTM_QUERY_DATA_POINT);   

    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmPtr;

    fixResQueryPtr->Query(dtmPtr, 0, 0, IMrDTMQueryParametersPtr(queryParamsPtr));

    BeAssert(dynamic_cast<IMrDTM*>(dtmPtr.get()) != 0);

    IMrDTMPtr mrDTMsingleResolutionView = (IMrDTM*)dtmPtr.get();

    return mrDTMsingleResolutionView->GetPointCount();
    }

/*===================================================================*/
/*        VIRTUAL FUNCTIONS INHERITED FROM DTMDataRef SECTION - BEGIN*/
/*===================================================================*/

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
IDTM* MrDTMDataRef::_GetDTMStorage(DTMDataRefPurpose purpose)
    {
    DTMPtr pDtm;      
    int    view = 0; // Default View

    if (purpose == GetStatistics || purpose == GetMrDtm)
        {   
        pDtm = (DTMPtr&)GetMrDTM();
        }
    else
        {
        IMrDTMPtr& iMrDTMPtr = GetMrDTM();

        //If the state is not empty the STM should contain some points and/or breaklines.       
        BeAssert((iMrDTMPtr->GetState() == MRDTM_STATE_EMPTY) || 
               (iMrDTMPtr->GetBreaklineCount() != 0) || 
               (iMrDTMPtr->GetPointCount() != 0));
        
        if ((iMrDTMPtr == 0) || 
            (iMrDTMPtr->GetState() == MRDTM_STATE_EMPTY) || 
            ((iMrDTMPtr->GetBreaklineCount() == 0) && (iMrDTMPtr->GetPointCount() == 0)))            
            return nullptr;

        if (!m_viewData[view])
            {
            m_viewData[view] = new MrDTMViewData(this);
            }

        pDtm = m_viewData[view]->GetDTM(purpose, nullptr);
        }

    return pDtm.get();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
IDTM* MrDTMDataRef::_GetDTMStorage(DTMDataRefPurpose purpose, ViewContextR viewContext)
    {
    int   view = 0; // Default View
    IDTM* pDTM = 0;

    if (m_mrDTM != 0)
        {
        if (viewContext.GetViewport())
            view = viewContext.GetViewport()->GetViewNumber();

        if (!m_viewData[view])
            {
            m_viewData[view] = new MrDTMViewData(this);        
            }

        pDTM = m_viewData[view]->GetDTM(purpose, &viewContext).get();
        }

    return pDTM;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
StatusInt MrDTMDataRef::_GetDTMReferenceStorage (RefCountedPtr<IDTM>& outDtm)
    {
    outDtm = GetDTMStorage(None);
    return outDtm.IsValid() ? SUCCESS : ERROR;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/11
//=======================================================================================
/*
double MrDTMDataRef::_GetLastModified() 
    {
    //Should represent the last time the DTM data in the STM file have been modified.
 //   BeAssert(!"Not implemented yet. Should probably be stored at the STM file level");
    return 0;
    }
*/
//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
bool MrDTMDataRef::_GetExtents (DRange3dR range)
    {    
   // BeAssert ((m_mrDTM != 0) || (m_iDTMFileLastLoadStatus != MRDTMSTATUS_LOADEDFILE));                
    range.low = m_minPoint;
    range.high = m_maxPoint;          
    return true;
    }

/*==================================================================*/
/*        VIRTUAL FUNCTIONS INHERITED FROM DTMDataRef SECTION - END */
/*==================================================================*/


/*===================================================================*/
/*        PUBLISHED FUNCTIONS SECTION - BEGIN                        */
/*===================================================================*/

//Progressive Display
IMrDTMProgressiveDisplayPtr IMrDTMDataRef::GetMrDTMProgressiveDisplayInterface()
    {
    return s_mrDtmProgressiveDisplayPtr;
    }

void IMrDTMDataRef::SetMrDTMProgressiveDisplayInterface(IMrDTMProgressiveDisplayPtr mrDtmProgressiveDisplayPtr)
    {
    s_mrDtmProgressiveDisplayPtr = mrDtmProgressiveDisplayPtr;
    }

//Plotter Interface
IPlotterInfo* IMrDTMDataRef::GetPlotterInterface()
    {
    return s_plotterInterface;
    }

void IMrDTMDataRef::SetPlotterInterface(IPlotterInfo* plotterInterface)
    {
    s_plotterInterface = plotterInterface;
    }

//Notification of change 
int IMrDTMDataRef::AddChangeListener(FPNotifyChange fpChangeListener)
    {
    int status = -1;

    ChangeListeners::iterator changeListenerIter(s_changeListeners.begin());
    ChangeListeners::iterator changeListenerIterEnd(s_changeListeners.end());

    while (changeListenerIter != changeListenerIterEnd)
        {        
        if (*changeListenerIter == fpChangeListener)
            break;

        changeListenerIter++;
        }

    if (changeListenerIter == changeListenerIterEnd)
        {
        s_changeListeners.push_back(fpChangeListener);
        status = 0;
        }

    return status;    
    }

int IMrDTMDataRef::RemoveChangeListener(FPNotifyChange fpChangeListener)
    {
    int status = -1;

    ChangeListeners::iterator changeListenerIter(s_changeListeners.begin());
    ChangeListeners::iterator changeListenerIterEnd(s_changeListeners.end());

    while (changeListenerIter != changeListenerIterEnd)
        {        
        if (*changeListenerIter == fpChangeListener)
            {
            s_changeListeners.erase(changeListenerIter);
            status = 0;
            break;
            }

        changeListenerIter++;
        }

    return status;    
    }

int MrDTMDataRef::_GetRasterTextureCacheDecimationMethod(IRasterTextureSourceCacheFile::DecimationMethod& decimationMethod) const
    {    
    decimationMethod = m_rasterTextureCacheDecimationMethod;
    return SUCCESS;   
    }

int MrDTMDataRef::_SetRasterTextureCacheDecimationMethod(IRasterTextureSourceCacheFile::DecimationMethod decimationMethod)  
    {                      
    if (m_rasterTextureCacheDecimationMethod != decimationMethod)
        {
        m_rasterTextureCacheDecimationMethod = decimationMethod;

        EditElementHandle editElementHandle(m_element.GetElementRef());      
                   
        editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_RASTER_TEXTURE_CACHE_DECIMATION_METHOD, (uint32_t)sizeof(m_rasterTextureCacheDecimationMethod), &m_rasterTextureCacheDecimationMethod);
        
        editElementHandle.ReplaceInModel(m_element.GetElementRef());    
        }
        
    return SUCCESS;   
    }




bool MrDTMDataRef::_GetClipActivation() const
    {    
    return m_isClipActivated;   
    }

int MrDTMDataRef::_SetClipActivation(bool isActivated) 
    {                      
    m_isClipActivated = isActivated;

    int status = SaveClipsInXAttribute();

    BeAssert(status == 0);

    FlushAllViewData();
        
    return 0;   
    }

int MrDTMDataRef::_GetNbClips() const
    {
    return m_clipContainerPtr == 0 ? 0 : static_cast<int>(m_clipContainerPtr->GetNbClips());   
    }

int MrDTMDataRef::_GetClip(DPoint3d*& clipPointsP,
                           int&       numberOfPoints, 
                           bool&      isClipMask,
                           int        clipInd) const
    {
    BeAssert(m_clipContainerPtr != 0 && clipPointsP == 0);    
    
    IMrDTMClipInfoPtr clipInfoP;
    
    m_clipContainerPtr->GetClip(clipInfoP, clipInd);

    numberOfPoints = static_cast<int>(clipInfoP->GetNbClipPoints());

    clipPointsP = new DPoint3d[numberOfPoints];
            
    for (size_t pointInd = 0; pointInd < clipInfoP->GetNbClipPoints(); pointInd++)
        {
        clipPointsP[pointInd] =  clipInfoP->GetClipPoints()[pointInd];        
        }

    isClipMask = clipInfoP->IsClipMask();
        
    return 0;
    }

double MrDTMDataRef::_GetEdgeMethodLengthInStorage()
    {
    double    edgeMethodLengthInStorage;
    Transform trsf;    
    
    getStorageToUORMatrix(trsf, GetElement().GetModelRef(), GetElement(), false);

    DPoint3d directionVector;
    DPoint3d fixedPoint;
    double   radians;
    double   scale;
            
    if (bsiTransform_isUniformScaleAndRotateAroundLine(&trsf,
                                                       &fixedPoint,
                                                       &directionVector,
                                                       &radians,
                                                       &scale) != 0)        
        {
        edgeMethodLengthInStorage = m_edgeMethodLength / scale;
        }
    else
    if (bsiTransform_isIdentity(&trsf) != 0)
        {        
        edgeMethodLengthInStorage = m_edgeMethodLength;
        }
    else
        {
        BeAssert(!"Different scale");            
        edgeMethodLengthInStorage = m_edgeMethodLength / trsf.form3d[0][0];
        }

    return edgeMethodLengthInStorage;
    }


int MrDTMDataRef::GenerateMrDTM()
    {   
    StatusInt  status = 0; 
    bool isUpToDate;
    bool needToReload = false;

    status = IsMrDTMUpToDate(isUpToDate);

    BeAssert(status == 0);

    if (isUpToDate == false)
        {    
        // Make sure that the MRDTM is closed
        FlushAllViewData();        
        m_mrDTM = nullptr;
        m_MrDTMlowestResolutionPointView = 0;  		      

        m_multiResolutionGridDtmManagerPtr = 0;
        m_multiResolutionGridMaterialManagerPtr = 0;

        m_mrDTMCreator = 0;
        
        //The creator should have the intelligence of recreating an MrDTM even 
        //if the file pointer is kept by an IMrDTM.        
        WString fileName(m_mrdtmMonikerPtr->ResolveFileName(&status));

        BeAssert(status == 0);
        m_mrDTMCreator = IMrDTMCreator::GetFor(fileName.GetWCharCP());        

        status = (0 != m_mrDTMCreator.get()) ? m_mrDTMCreator->Create() : BSIERROR;

        //TR 330586 - If the generation failed the STM file can still be present, but empty. 
        //            So reload the empty file, which will ensure that MrDTMDataRef 
        //            correctly represents the empty STM.
        if ((status == 0) || (_waccess(fileName.GetWCharCP(), 00) == 0))
            {
            needToReload = true;

            m_mrDTMCreator = 0;  

            const StatusInt reloadStatus = ReloadMrDTM(GetDestinationGCS());
            status = (BSISUCCESS == status) ? reloadStatus : status;
            }
        }    
   
    return status ;
    }   

int MrDTMDataRef::_GenerateRasterTextureCache()
    {
    bool isUpToDate;

    int status = _IsRasterTextureCacheFileUpToDate(isUpToDate);

    BeAssert(status == 0);

    if (isUpToDate == false)
        {
        FlushAllViewData();

        BeAssert(m_rasterDrapingSrcPtr != 0);

        IRasterTextureSourceCacheFilePtr cacheFilePtr(m_rasterDrapingSrcPtr->GetCacheFileInterface());
        
        BeAssert(cacheFilePtr != 0);
            
        WString cacheFilePath;
    
        int status = _GetRasterTextureCacheFilePath(cacheFilePath);

        BeAssert(status == 0);
               
        status = cacheFilePtr->Create(cacheFilePath, m_rasterTextureCacheDecimationMethod);

        if (status == SUCCESS)
            {
            bool isUpToDate;
   
            //Check again if everything is up-to-date.
            status = _IsRasterTextureCacheFileUpToDate(isUpToDate);

            BeAssert((status == 0) && (isUpToDate == true)); 
            }
        }

    return status ;
    }

bool MrDTMDataRef::LoadRasterTextureSource(StringList* drapingRasterFileNamesP)
    {        
    IRasterTextureSourceManager* rasterTextureSourceManagerP = DTMElementHandlerManager::GetRasterTextureSourceManager();

    if (rasterTextureSourceManagerP != 0)
        {                        
        WString cacheFilePath;
    
        int status = _GetRasterTextureCacheFilePath(cacheFilePath);

        BeAssert(status == 0);

        if (_waccess(cacheFilePath.GetWCharCP(), 00) == 0)
            {                        
            m_rasterDrapingSrcPtr = rasterTextureSourceManagerP->GetSource(*drapingRasterFileNamesP, 
                                                                           cacheFilePath.GetWCharCP());         
            }                                                          
        else
            {
            m_rasterDrapingSrcPtr = rasterTextureSourceManagerP->GetSource(*drapingRasterFileNamesP);
            }
        }

    return m_rasterDrapingSrcPtr != 0;    
    }



int MrDTMDataRef::IsMrDTMUpToDate(bool& isUpToDate)
    {
    isUpToDate = false;

    //Need to check the status instead.        
    if (m_mrDTM != 0)
        {
        IMrDTMCreatorPtr creatorPtr = m_mrDTMCreator;

        if (0 == m_mrDTMCreator.get())
            {
            creatorPtr = IMrDTMCreator::GetFor(m_mrDTM);
            
            if (0 == creatorPtr.get())
                {
                BeAssert(!"Bad... Expect a valid creator!");
                return true;
                }
            }


        creatorPtr->UpdateLastModified();
        creatorPtr->SaveToFile();


        isUpToDate = m_mrDTM->InSynchWithDataSources();
        }
    else
        {
        isUpToDate = (m_mrDTMCreator == 0) && (m_iDTMFileLastLoadStatus != MRDTMSTATUS_FILE_LOADED);
        }
    
    return 0;
    }

int MrDTMDataRef::_IsRasterTextureCacheFileUpToDate(bool& isUpToDate, EditElementHandle* elmHandleP)
    {
    isUpToDate = true;

    if (m_rasterDrapingSrcPtr != 0)
        {
        IRasterTextureSourceCacheFilePtr cacheFilePtr(m_rasterDrapingSrcPtr->GetCacheFileInterface());
                
        if (cacheFilePtr != 0)
            {
            BeAssert(cacheFilePtr != 0);

            isUpToDate = cacheFilePtr->IsUpToDate();

            IRasterTextureSourceCacheFile::DecimationMethod decimationMethod;

            int status = cacheFilePtr->GetDecimationMethod(decimationMethod);  

            BeAssert(status == SUCCESS);

            if ((isUpToDate == true) && (decimationMethod == m_rasterTextureCacheDecimationMethod))
                {
                m_rasterTextureCacheState = RASTER_TEXTURE_CACHE_STATE_UP_TO_DATE;
                }
            else
                {
                m_rasterTextureCacheState = RASTER_TEXTURE_CACHE_STATE_DIRTY;
                }
            }
        else
            {
            //Cache file for given raster texture not supported, so no need to update it.
            m_rasterTextureCacheState = RASTER_TEXTURE_CACHE_STATE_UP_TO_DATE;
            }

        if (elmHandleP != 0)
            {
            elmHandleP->ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, 
                                                MRDTM_DETAILS_RASTER_TEXTURE_CACHE_STATE, 
                                                (uint32_t)sizeof(m_rasterTextureCacheState), 
                                                &m_rasterTextureCacheState);   
            }
        else
            {
            EditElementHandle editElementHandle(m_element.GetElementRef());   

            editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, 
                                                      MRDTM_DETAILS_RASTER_TEXTURE_CACHE_STATE, 
                                                      (uint32_t)sizeof(m_rasterTextureCacheState), 
                                                      &m_rasterTextureCacheState);   

            editElementHandle.ReplaceInModel(m_element.GetElementRef());
            }
        }

    return 0;
    }

 

int MrDTMDataRef::PushBackClip(DPoint3d* clipPointsP,
                               int       numberOfPoints, 
                               bool      isClipMask)
    {
    BeAssert(clipPointsP != 0 && numberOfPoints > 0);

    if (m_clipContainerPtr == 0)
        {
        m_clipContainerPtr = IMrDTMClipContainer::Create();
        }

    IMrDTMClipInfoPtr clipInfoPtr(IMrDTMClipInfo::Create(clipPointsP, numberOfPoints, isClipMask));

    m_clipContainerPtr->AddClip(clipInfoPtr);
    
    return 0;
    }
  
int MrDTMDataRef::_AddClip(DPoint3d* clipPointsP,
                          int       numberOfPoints, 
                          bool      isClipMask)
    {    
    int status = SUCCESS;

    if (numberOfPoints > 0)
        {
        status = PushBackClip(clipPointsP, numberOfPoints, isClipMask);

        status = SaveClipsInXAttribute();

        BeAssert(status == 0);

        FlushAllViewData();
        }
    else
        {
        status = ERROR;
        }

    return status;
    }

int MrDTMDataRef::_RemoveClip(int toRemoveClipInd)
    {
    BeAssert((toRemoveClipInd >= 0) && (toRemoveClipInd < (int)m_clipContainerPtr->GetNbClips()));    
    
    m_clipContainerPtr->RemoveClip(toRemoveClipInd);
    
    int status = SaveClipsInXAttribute();

    BeAssert(status == 0);

    FlushAllViewData();

    return 0;
    }

int MrDTMDataRef::_RemoveAllClip()
    {
    m_clipContainerPtr = 0;
   
    int status = SaveClipsInXAttribute();

    BeAssert(status == 0);

    FlushAllViewData();

    return 0;
    }

int MrDTMDataRef::ReadClipsFromSerializationData(const byte* data, size_t dataSize)
    {
    int       status = 0;
    DPoint3d* clipPointsP = 0;
    int       nbOfClips;
    bool      isClipMask;
    int       nbClipPoints;

    m_clipContainerPtr = 0;    

    DataInternalizer source(data, dataSize);                           
          
    source.get((Int8*)&m_isClipActivated);
    source.get(&nbOfClips);

    for (int clipInd = 0; clipInd < nbOfClips; clipInd++)
        {
        source.get((Int8*)&isClipMask);
        source.get(&nbClipPoints);

        clipPointsP = new DPoint3d[nbClipPoints];

        source.get((UInt8*)clipPointsP, sizeof(DPoint3d) * nbClipPoints);
        
        status = PushBackClip(clipPointsP, nbClipPoints, isClipMask);  
        
        BeAssert(status == 0);

        delete clipPointsP;   
        }

    BeAssert(nbOfClips == _GetNbClips());

    return status;    
    }

int MrDTMDataRef::ReadMonikerFromSerializationData(const byte* data, size_t dataSize, DgnModelRefP modelRefP)
    {   
    WString monikerString;

    DataInternalizer source((byte*)data, dataSize);

    source.get(monikerString);           

    WString searchPath;
    // 1- Search in dgnFile dir.
    if(NULL != modelRefP && NULL != modelRefP->GetDgnFileP())
        {
        searchPath += modelRefP->GetDgnFileP()->GetFileName(); 
        searchPath += L";";
        }
        
    // 2- Search in MS_RFDIR
    searchPath += L"MS_RFDIR";

    m_mrdtmMonikerPtr = DgnDocumentMoniker::Create(monikerString.GetWCharCP(), searchPath.c_str(), false);   
     
    BeAssert(m_mrdtmMonikerPtr != 0);
              
    StatusInt statusInt;

    DgnDocument::FetchMode openMode;

    if (m_isReadOnly == true)
        {
        openMode = DgnDocument::FetchMode::Read;
        }
    else
        {
        openMode = DgnDocument::FetchMode::Write;
        }

    m_mrdtmDocumentPtr = DgnDocument::CreateFromMoniker(statusInt, *m_mrdtmMonikerPtr, DEFDGNFILE_ID, openMode);

    return 0;
    }

int MrDTMDataRef::SaveClipsInXAttribute(bool replaceInModel, EditElementHandle* elmHandleP)
    {
    DataExternalizer d;        

    d.put((Int8)m_isClipActivated);
    d.put(_GetNbClips());

    DPoint3d* clipPointsP = 0;
    int       numberOfClipPoints;
    bool      isClipMask;
    int       status;

    for (int clipInd = 0; clipInd < _GetNbClips(); clipInd++)
        {
        status = GetClip(clipPointsP, numberOfClipPoints, isClipMask, clipInd);

        BeAssert(status == 0);

        d.put((Int8)isClipMask);
        d.put(numberOfClipPoints);
        d.put((UInt8*)clipPointsP, sizeof(DPoint3d) * numberOfClipPoints);                                  
        delete clipPointsP;   

        clipPointsP = 0;
        }                  

    if (elmHandleP != NULL)
        {
        elmHandleP->ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_CLIPS, (uint32_t)d.getBytesWritten(), d.getBuf());

        if (replaceInModel == true)
            elmHandleP->ReplaceInModel(elmHandleP->GetElementRef());
        }
    else
        {
        EditElementHandle editElementHandle(m_element.GetElementRef());      

        editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_CLIPS, (uint32_t)d.getBytesWritten(), d.getBuf());

        if (replaceInModel == true)
            {
            editElementHandle.ReplaceInModel(editElementHandle.GetElementRef());
            }
        }

    return 0;
    }

int MrDTMDataRef::_GetIMrDTMCreator(IMrDTMCreatorPtr& iMrDTMCreatorPtr)
    {    
    StatusInt status = BSISUCCESS;

    if ((m_mrDTMCreator == 0) && (m_mrDTM != 0))
        {
        //StatusInt status;
        
        //m_mrDTMCreator = IMrDTMCreator::GetMrDTMCreator(m_mrdtmMonikerPtr->GetFileName(status).GetWCharCP());        


        m_mrDTMCreator = IMrDTMCreator::GetFor(m_mrDTM, status);        

        //BeAssert(status == 0);
        }
        
    iMrDTMCreatorPtr = m_mrDTMCreator;
    return status;
    }

int MrDTMDataRef::_SaveIMrDTMCreatorToFile()
    {
    int status = -1;

     if (m_mrDTMCreator != 0)
        {
        status = m_mrDTMCreator->SaveToFile();
        }

    return status;
    }

const WString& MrDTMDataRef::_GetDescription() const
    {
    return m_description;
    }

int MrDTMDataRef::_SetDescription(const WString& description)
    {
    EditElementHandle editElementHandle(m_element.GetElementRef());
    
    DataExternalizer descriptionDataExternalizer;

    descriptionDataExternalizer.put(description);

    m_description = description;

    editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_DESCRIPTION, (uint32_t)descriptionDataExternalizer.getBytesWritten(), descriptionDataExternalizer.getBuf());
        
    editElementHandle.ReplaceInModel(editElementHandle.GetElementRef());
    
    return 0;
    }

MrDTMStatus MrDTMDataRef::_GetIDTMFileLastLoadStatus()
    {
    return m_iDTMFileLastLoadStatus;
    }
    
bool MrDTMDataRef::_IsOpenInReadOnly()
    {    
    return m_isReadOnly;
    }

int MrDTMDataRef::_SetReadOnly(bool isReadOnly)
    {
    SetReadOnly(isReadOnly, true);   

    return SUCCESS;
    }

int MrDTMDataRef::SetReadOnly(bool isReadOnly, bool updateXAtt)
    {
    //For now MrDTM is only supported in read-only.    
    if (m_isReadOnly != isReadOnly)
        {
        m_isReadOnly = isReadOnly;        
        
        if (updateXAtt)
            {                
            EditElementHandle editElementHandle(m_element.GetElementRef());

            editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_IS_READ_ONLY, (uint32_t)sizeof(isReadOnly), &isReadOnly);
            
            ReloadMrDTM(GetDestinationGCS());
            
            editElementHandle.ReplaceInModel (editElementHandle.GetElementRef());
            }
        else
            ReloadMrDTM(GetDestinationGCS());
        }

    return SUCCESS;
    }
    
bool MrDTMDataRef::_CanLocate() const
    {
    return m_canLocate;
    }

int MrDTMDataRef::_SetLocate(bool canLocate)
    {        
    EditElementHandle editElementHandle(m_element.GetElementRef());

    m_canLocate = canLocate;

    editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_CAN_LOCATE, (uint32_t)sizeof(canLocate), &canLocate);

    editElementHandle.ReplaceInModel(editElementHandle.GetElementRef());    

    return 0;
    }

bool MrDTMDataRef::_IsAnchored() const
    {
    return m_isAnchored;
    }

int MrDTMDataRef::_SetAnchored(bool isAnchored) 
    {
    EditElementHandle editElementHandle(m_element.GetElementRef());

    m_isAnchored = isAnchored;

    editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_IS_ANCHORED, (uint32_t)sizeof(isAnchored), &isAnchored);

    editElementHandle.ReplaceInModel(editElementHandle.GetElementRef());    

    return 0;   
    }

const DgnDocumentPtr MrDTMDataRef::_GetMrDTMDocument() const
    {
    return m_mrdtmDocumentPtr;
    }

int MrDTMDataRef::_SetMrDTMDocument(DgnDocumentPtr& mrdtmDocumentPtr)
    {
    EditElementHandle editElementHandle(m_element.GetElementRef());

    m_mrdtmDocumentPtr = mrdtmDocumentPtr;                
    m_mrdtmMonikerPtr = m_mrdtmDocumentPtr->GetMonikerPtr();                

    DataExternalizer descriptionDataExternalizer;
        
    WString monikerString(m_mrdtmMonikerPtr->Externalize());

    descriptionDataExternalizer.put(monikerString);
       
    editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_FILE_MONIKER, (uint32_t)descriptionDataExternalizer.getBytesWritten(), descriptionDataExternalizer.getBuf());
            
    editElementHandle.ReplaceInModel(editElementHandle.GetElementRef()); 

    return ReloadMrDTM(GetDestinationGCS());
    }
  
const DgnDocumentMonikerPtr MrDTMDataRef::_GetFileMoniker() const
    {
    return m_mrdtmMonikerPtr;
    }

Bentley::GeoCoordinates::BaseGCSPtr MrDTMDataRef::_GetGeoCS()
    {   
    BaseGCSPtr baseGCSPtr;
    
    BeAssert((m_mrDTM != 0) || 
           (m_mrDTMCreator != 0) || 
           (m_iDTMFileLastLoadStatus == MRDTMSTATUS_INVALID_FILE) || 
           (m_iDTMFileLastLoadStatus == MRDTMSTATUS_MISSING_FILE) ||
           (m_iDTMFileLastLoadStatus == MRDTMSTATUS_UNSUPPORTED_VERSION));    

    if (m_mrDTM != 0)
        {
        baseGCSPtr = m_mrDTM->GetBaseGCS();
        }            
    else
    if (m_mrDTMCreator != 0)
        {
        baseGCSPtr = m_mrDTMCreator->GetBaseGCS();
        }

    return baseGCSPtr;
    }

int MrDTMDataRef::_SetGeoCS(const Bentley::GeoCoordinates::BaseGCSPtr& baseGCSPtr)
    {     
    return SetGeoCS(baseGCSPtr, false);
    }    

int MrDTMDataRef::_GetExtentInGeoCS(DRange3d& extentInGeoCS, bool inUors)
    {
    int status = ERROR;

    if (m_mrDTM != 0)
        {                    
        BaseGCSPtr baseGCSPtr(m_mrDTM->GetBaseGCS());
            
        if ((baseGCSPtr != 0) && 
            (SUCCESS == m_mrDTM->GetRangeInSpecificGCS(extentInGeoCS.low, extentInGeoCS.high, baseGCSPtr)))
            {
            if (inUors)
                {
                DgnGCSPtr dgnGcsPtr(DgnGCS::CreateGCS(baseGCSPtr.get(), m_element.GetModelRef()));

                dgnGcsPtr->UorsFromCartesian(extentInGeoCS.low, extentInGeoCS.low);
                dgnGcsPtr->UorsFromCartesian(extentInGeoCS.high, extentInGeoCS.high);
                }

            status = SUCCESS;
            }        
        }        

    return status; 
    }

int MrDTMDataRef::_GetEdgeMethod(int& edgeMethod)
    {
    edgeMethod = m_edgeMethod;
    return 0;
    }

int MrDTMDataRef::_GetEdgeMethodLength(double& edgeMethodLength)
    {
    edgeMethodLength = m_edgeMethodLength;
    return 0;
    }

int MrDTMDataRef::SetEdgeMethod(int edgeMethod)
    {
    m_edgeMethod = edgeMethod;
    
    WriteXAttributeTriangulationEdgeOptions();

    return 0;
    }

int MrDTMDataRef::SetEdgeMethodLength(double edgeMethodLength)
    {
    m_edgeMethodLength = edgeMethodLength;

    WriteXAttributeTriangulationEdgeOptions();
    return 0;
    }

int MrDTMDataRef::SetGeoCS(const Bentley::GeoCoordinates::BaseGCSPtr& baseGCSPtr, bool fromUndoRedo)
    {
    BeAssert((m_mrDTM != 0) || (m_mrDTMCreator != 0) || (m_iDTMFileLastLoadStatus != MRDTMSTATUS_FILE_LOADED));

    int status = 0;
              
    if (((m_mrDTM != 0) || (m_mrDTMCreator != 0)) && (m_isReadOnly == false))
        {       
        BaseGCSPtr oldBaseGCSPtr(_GetGeoCS());

        //Flush the multi-resolution grid managers
        m_multiResolutionGridDtmManagerPtr = 0;
        m_multiResolutionGridMaterialManagerPtr = 0;

        if (m_mrDTM != 0)
            {
            status = m_mrDTM->SetBaseGCS(baseGCSPtr);
            }
        else
            {
            status = m_mrDTMCreator->SetBaseGCS(Bentley::GeoCoordinates::BaseGCSPtr(baseGCSPtr)); 
            }
                       
        if (status == 0)
            {

// Disable the undo for the moment
#if 0
            //Should be fixed when TFS 103749 is fixed.
            if (fromUndoRedo == false)
                {                               
                //Dummy element for undo/redo of GCS                       
                MrDTMGeoCSUndoRedoData gcsUndoRedoData(m_element);

                status = gcsUndoRedoData.SetOldBaseGCS(oldBaseGCSPtr);
                
                BeAssert(status == 0);

                status = gcsUndoRedoData.SetNewBaseGCS(baseGCSPtr);

                BeAssert(status == 0);
                                
                EditElementHandle editElementHandle(m_element.GetElementRef());
                                 
                if (m_mrDTM != 0)
                    {
                    ReloadMrDTM(GetDestinationGCS());
                    m_element.GetDisplayHandler()->ValidateElementRange(editElementHandle, true);                       
                    }
                }
#endif

            FlushAllViewData();
                 
            NotifyChange();        
            }
         }
     else
        {
        status = -1;
        }
        
    return status;
    }

bool MrDTMDataRef::_IsVisibleForView(short viewNumber)
    {
    BeAssert((viewNumber >= 0) && (viewNumber < NUMBER_OF_PP_VIEWS));
    return m_visibilityPerView[viewNumber];
    }
 
int MrDTMDataRef::_SetVisibilityForView(short viewNumber, bool isVisible)
    {
    BeAssert((viewNumber >= 0) && (viewNumber < NUMBER_OF_PP_VIEWS));

    m_visibilityPerView[viewNumber] = isVisible;

    EditElementHandle editElementHandle(m_element.GetElementRef());
          
    editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_VISIBILITY_PER_VIEW, (uint32_t)sizeof(m_visibilityPerView), &m_visibilityPerView);

    editElementHandle.ReplaceInModel(editElementHandle.GetElementRef());

    return 0;
    }
   
int MrDTMDataRef::_GetMrDTMState()
    {
    MrDTMState state(MRDTM_STATE_EMPTY);

    if (m_mrDTM != 0)
        {
        state = m_mrDTM->GetState();
        }

    if ((state == MRDTM_STATE_UP_TO_DATE) && 
        (m_rasterDrapingSrcPtr != 0) && 
        (m_rasterTextureCacheState == RASTER_TEXTURE_CACHE_STATE_DIRTY))
        {
        state = MRDTM_STATE_DIRTY;
        }

    return state;
    }

bool MrDTMDataRef::_GetLastUpToDateCheckTime(time_t& last)
    {
    bool lastCheckTimeAvailable = false;

    if (m_mrDTM != 0)
        {                
        lastCheckTimeAvailable = m_mrDTM->LastSynchronizationCheck(last);        
        }
    
    return lastCheckTimeAvailable;
    }
 
int MrDTMDataRef::_Generate()
    {
    int status;
    
    status = GenerateMrDTM();  

    if (status == 0)
        {
        status = _GenerateRasterTextureCache();    
        }    

    NotifyChange();

    return status;
    }
    
bool MrDTMDataRef::_IsUpToDate()
    {    
    int  status;
    bool isMrDTMUpToDate;
    bool isRasterTextureCacheFileUpToDate;

    status = IsMrDTMUpToDate(isMrDTMUpToDate);
    BeAssert(status == 0);

    status = _IsRasterTextureCacheFileUpToDate(isRasterTextureCacheFileUpToDate);

    BeAssert(status == 0);

    NotifyChange();        

    return (isMrDTMUpToDate == true) && (isRasterTextureCacheFileUpToDate == true);     
    }    

/*===================================================================*/
/*        PUBLISHED FUNCTIONS SECTION - END                          */
/*===================================================================*/



/*===================================================================*/
/*        DISPLAY CONTROL FUNCTIONS SECTION - BEGIN                  */
/*===================================================================*/
double MrDTMDataRef::_GetPointDensityForShadedView()
    {       
    return m_pointDensityForShadedView;
    }

int MrDTMDataRef::_SetPointDensityForShadedView(double pi_minScreenPixelsPerPoint)
    {
    BeAssert(pi_minScreenPixelsPerPoint > 0.5);

    m_pointDensityForShadedView = pi_minScreenPixelsPerPoint;

    WriteXAttributePointDensityData();

    return 0;
    }
    
double MrDTMDataRef::_GetPointDensityForWireframeView()
    {
    return m_pointDensityForWireframeView; 
    }

int MrDTMDataRef::_SetPointDensityForWireframeView(double pi_minScreenPixelsPerPoint)
    {
    BeAssert(pi_minScreenPixelsPerPoint > 0.5);
    m_pointDensityForWireframeView = pi_minScreenPixelsPerPoint;

    WriteXAttributePointDensityData();    
      
    return 0;
    }

int MrDTMDataRef::_ClearCachedMaterials()
    {

    if (m_multiResolutionGridMaterialManagerPtr != 0)
        {
        m_multiResolutionGridMaterialManagerPtr->ClearCachedMaterials(); 
        }

    if (IMrDTMDataRef::GetMrDTMProgressiveDisplayInterface() != 0)
        {
        IMrDTMDataRef::GetMrDTMProgressiveDisplayInterface()->ClearCachedBlocks(this, true);
        }

    FlushAllViewData();

    return 0;
    }

//This function returns the default absolute raster texture cache file name. 
//Currently the default name is the only valid name.
int MrDTMDataRef::_GetRasterTextureCacheFilePath(WString& cacheFilePath)
    {
    int status;

    if (m_mrdtmDocumentPtr != 0) 
        {
        cacheFilePath = m_mrdtmDocumentPtr->GetFileName() + L".pss" + L".ctiff";    
        status = SUCCESS;
        }
    else
    if (m_mrdtmMonikerPtr != 0)
        {
        WString fileName(m_mrdtmMonikerPtr->ResolveFileName(&status));

        if (status == SUCCESS)
            {
            cacheFilePath = fileName + L".pss" + L".ctiff";    
            }
        }
    
    return status;
    }

//This function returns the default absolute raster texture PSS file name. 
//Currently the default name is the only valid path.
int MrDTMDataRef::_GetRasterTexturePSSFilePath(WString& pssFilePath)
    {
    StatusInt status;

    if (m_mrdtmDocumentPtr != 0) 
        {
        pssFilePath = m_mrdtmDocumentPtr->GetFileName() + L".pss";    
        status = SUCCESS;
        }
    else
    if (m_mrdtmMonikerPtr != 0)
        {
        WString fileName(m_mrdtmMonikerPtr->ResolveFileName(&status));

        if (status == SUCCESS)
            {
            pssFilePath = fileName + L".pss";    
            }
        }
    
    return status;
    }

IRasterTextureSourcePtr MrDTMDataRef::_GetRasterTextureSource()
    {           
    return m_rasterDrapingSrcPtr; 
    }

int MrDTMDataRef::_SetRasterTextureSource(IRasterTextureSourcePtr& rasterTextureSourcePtr)
    {
    int status;

    EditElementHandle editElementHandle(m_element.GetElementRef());
              
    if (rasterTextureSourcePtr != 0)
        {   
        m_rasterDrapingSrcPtr = rasterTextureSourcePtr;

        //The raster texture source has changed, recreate the material manager.
        m_multiResolutionGridMaterialManagerPtr = 0;

        const StringList* sourceFileNamesP(rasterTextureSourcePtr->GetSourceFileNames());      

        int              nbFileNames = mdlStringList_size(sourceFileNamesP);    
        WCharCP          fileNameDescP;
        InfoField*       infoFieldP;
        DataExternalizer d;            
        DataExternalizer dUnicode;            

        d.put(nbFileNames); 
        dUnicode.put(nbFileNames); 

        for (long fileNameInd = 0; fileNameInd < nbFileNames; fileNameInd++)
            {      
            status = mdlStringList_getMember(&fileNameDescP, 
                                             &infoFieldP, 
                                             sourceFileNamesP,
                                             fileNameInd);

            BeAssert(fileNameDescP != 0);        
            BeAssert(status == 0);
            
            size_t strLength = wcslen(fileNameDescP) + 1;
            CharP fileNameDescMultiBytes = new char[strLength];

            BeStringUtilities::WCharToCurrentLocaleChar(fileNameDescMultiBytes, fileNameDescP, strLength);            
            d.put(fileNameDescMultiBytes);              
            dUnicode.put(fileNameDescP);

            delete fileNameDescMultiBytes;
            }    

        status = editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_DRAPING_RASTERS_UNICODE, (uint32_t)dUnicode.getBytesWritten(), dUnicode.getBuf());        
                       
        BeAssert(status == 0);

        status = editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_DRAPING_RASTERS, (uint32_t)d.getBytesWritten(), d.getBuf());        
                       
        BeAssert(status == 0);
        }
    else
        {
        wstring rasterTextureSourceFileName;

        if (m_rasterDrapingSrcPtr != 0) 
            {                               
            const StringList* sourceFileNamesP(m_rasterDrapingSrcPtr->GetSourceFileNames());      

            int        nbFileNames = mdlStringList_size(sourceFileNamesP);    
            WCharCP    fileNameDescP;
            InfoField* infoFieldP;
            
            if (nbFileNames == 1)
                {
                status = mdlStringList_getMember(&fileNameDescP, 
                                                 &infoFieldP, 
                                                 sourceFileNamesP,
                                                 0);

                BeAssert(fileNameDescP != 0);        
                BeAssert(status == 0);     

                //Currently the raster texture source if always a .PSS file generated internally that should be delete 
                //if the raster texture source sets for the MrDTM is delete. 
                if ((wcslen(fileNameDescP) > 4) && (wcsncmp(fileNameDescP + wcslen(fileNameDescP) - 4, L".pss", 4) == 0))
                    {
                    rasterTextureSourceFileName = fileNameDescP;                    
                    }                         
                else
                    {
                    //Currently only a .pss file could be set.
                    BeAssert(0);
                    }
                }
            }

        FreeRasterTextureDrapingResources();

        //Remove the PSS and the PSS cache file, if any.
        if (rasterTextureSourceFileName.empty() == false)
            {                
#ifdef DEBUG
            WString filePath;

            _GetRasterTexturePSSFilePath(filePath);

            string filePathMultiByte; 

            StatusInt status = WideCharToMultiByte(filePathMultiByte, wstring(filePath.GetWCharCP()));  

            //The file name should of the raster texture source is currently fixed.
            BeAssert(status == 0 && filePathMultiByte == rasterTextureSourceFileName);            
#endif 

            status = _wremove(rasterTextureSourceFileName.c_str());
            BeAssert(status == 0);
            }        
    
        WString filePath;

        _GetRasterTextureCacheFilePath(filePath);

        if (_waccess(filePath.GetWCharCP(), 00) == 0)
            {             
            int removeFileStatus = _wremove(filePath.GetWCharCP());
            BeAssert(removeFileStatus == 0);
            }

        status = editElementHandle.ScheduleDeleteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_DRAPING_RASTERS);        
                       
        BeAssert(status == 0);

        ElementHandle::XAttributeIter xAttrIterDrapingRasters(editElementHandle, m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_DRAPING_RASTERS_UNICODE); 
        if (xAttrIterDrapingRasters.IsValid())
            {
            status = editElementHandle.ScheduleDeleteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_DRAPING_RASTERS_UNICODE);        
                       
            BeAssert(status == 0);
            }
        }
    
    FlushAllViewData();

    status = editElementHandle.ReplaceInModel(editElementHandle.GetElementRef());

    BeAssert(status == 0);
    
    if (m_iDTMFileLastLoadStatus == MRDTMSTATUS_MISSING_RASTER_DRAPING_INFO)
        m_iDTMFileLastLoadStatus = MRDTMSTATUS_FILE_LOADED;
    
    return status;
    }



int MrDTMDataRef::ReloadMrDTM(DgnGCSP targetGCS, bool updateRange)
    {       
    int status;
    // Close the mrDTM ... this will result in the MRDTM being reloaded ...
    // We need to change the Extent of the element (which is counter DGN element usual behavior)    
    FlushAllViewData();
    m_MrDTMlowestResolutionPointView = 0;  

    m_multiResolutionGridDtmManagerPtr = 0;
    m_multiResolutionGridMaterialManagerPtr = 0;

    m_mrDTM = 0;
    m_mrDTMCreator = 0;
    
    // Relaod right away to update the element range
    status = ReadFile(targetGCS);
        
    BeAssert((m_mrDTM != 0) || (m_iDTMFileLastLoadStatus != MRDTMSTATUS_FILE_LOADED));
  
    if (status == 0 && updateRange)
        {        
        EditElementHandle editElementHandle(m_element.GetElementRef());

        UpdateStorageToUORMatrix(editElementHandle);
        
        //TR 353153 - Don't write the min/max point if those haven't change        
        ElementHandle::XAttributeIter xAttrIterMinPoint(editElementHandle);
        
        if (xAttrIterMinPoint.Search(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_MIN_POINT))
            {
            if (memcmp(&m_minPoint, (byte*)xAttrIterMinPoint.PeekData(), sizeof(m_minPoint)) != 0)
                {
                editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_MIN_POINT, (uint32_t)sizeof(m_minPoint), &m_minPoint);
                }            
            }
        else
            {                
            editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_MIN_POINT, (uint32_t)sizeof(m_minPoint), &m_minPoint);
            }

        ElementHandle::XAttributeIter xAttrIterMaxPoint(editElementHandle);
    
        if (xAttrIterMaxPoint.Search(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_MAX_POINT))
            {
            if (memcmp(&m_maxPoint, (byte*)xAttrIterMaxPoint.PeekData(), sizeof(m_maxPoint)) != 0)
                {
                editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_MAX_POINT, (uint32_t)sizeof(m_maxPoint), &m_maxPoint);
                }                
            }
        else        
            {        
            editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_MAX_POINT, (uint32_t)sizeof(m_maxPoint), &m_maxPoint);
            }       
       
        editElementHandle.GetDisplayHandler()->ValidateElementRange(editElementHandle, true);

        // Make sure cache is RW because we also want to update the georeference of raster in nested reference and R-O model.
        DTMRWDgnCacheGuard DTMRWDgnCacheGuard(m_element.GetDgnModelP());        

        editElementHandle.ReplaceInModel (editElementHandle.GetElementRef());
        }

    return status;
    }

void MrDTMDataRef::WriteXAttributePointDensityData(bool replaceInModel, EditElementHandle* elmHandleP)
    {                
    DataExternalizer d;        
    d.put(m_pointDensityForShadedView);        
    d.put(m_pointDensityForWireframeView);

    if (elmHandleP != NULL)
        {
        elmHandleP->ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_POINT_DENSITY, (uint32_t)d.getBytesWritten(), d.getBuf());

        FlushAllViewData();      

        if (replaceInModel == true)
            elmHandleP->ReplaceInModel(elmHandleP->GetElementRef());    
        }
    else
        { 
        EditElementHandle editElementHandle(m_element.GetElementRef());

        editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_POINT_DENSITY, (uint32_t)d.getBytesWritten(), d.getBuf());

        FlushAllViewData();      

        if (replaceInModel == true)
            {
            editElementHandle.ReplaceInModel(editElementHandle.GetElementRef());    
            }
        }       
    }

void MrDTMDataRef::WriteXAttributeTriangulationEdgeOptions(bool replaceInModel, EditElementHandle* elmHandleP)
    {            
    DataExternalizer d;    

    d.put(m_edgeMethod);        
    d.put(m_edgeMethodLength);

    if (elmHandleP != NULL)
        {
        elmHandleP->ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_TRIANGULATION_EDGE_OPTIONS, (uint32_t)d.getBytesWritten(), d.getBuf());

        FlushAllViewData();      

        if (replaceInModel == true)
            elmHandleP->ReplaceInModel(elmHandleP->GetElementRef());    
        }
    else
        {
        EditElementHandle editElementHandle(m_element.GetElementRef());

        editElementHandle.ScheduleWriteXAttribute(m_xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_TRIANGULATION_EDGE_OPTIONS, (uint32_t)d.getBytesWritten(), d.getBuf());

        FlushAllViewData();      

        if (replaceInModel == true)
            {
            editElementHandle.ReplaceInModel(editElementHandle.GetElementRef());    
            }     
        }        
    }

IMultiResolutionGridMaterialManagerPtr MrDTMDataRef::_GetMultiResGridMaterialManager()
    {

    if ((m_multiResolutionGridMaterialManagerPtr == 0) && 
        (DTMElementHandlerManager::GetMultiResolutionGridManagerCreator() != 0))
        {
        SourceID rasterTextureSourceID;
        
        IRasterTextureSourcePtr rasterTextureSourcePtr = _GetRasterTextureSource();
       
        if (rasterTextureSourcePtr != 0)
            {   
            BaseGCSP gcsForDefaultPixelSizeP;

            if ((_GetGeoCS() != 0) &&  
                (GetDestinationGCS() != 0) &&
                (_GetGeoCS()->IsEquivalent(*GetDestinationGCS()) == false))
                {
                BaseGCSPtr sourceGCSPtr;
                BaseGCSPtr destinationGCSPtr;

                sourceGCSPtr = _GetGeoCS().get();
                destinationGCSPtr = DgnGCS::CreateGCS(*GetDestinationGCS());

                StatusInt status = rasterTextureSourcePtr->SetReprojectionGCS(sourceGCSPtr, destinationGCSPtr);

                BeAssert(status == 0);

                gcsForDefaultPixelSizeP = GetDestinationGCS();
                }  
            else
                {
                BaseGCSPtr nullBaseGCSPtr;

                StatusInt status = rasterTextureSourcePtr->SetReprojectionGCS(nullBaseGCSPtr, nullBaseGCSPtr);

                BeAssert(status == 0);

                gcsForDefaultPixelSizeP = _GetGeoCS().get();
                }

            rasterTextureSourceID = rasterTextureSourcePtr->GetSourceID();

            DPoint2d minimumPixelSizeInUORS; 

            GetDefaultMinimumPixelSizeInUORS(minimumPixelSizeInUORS, gcsForDefaultPixelSizeP);
            
            DRange3d clippedExtent;

            GetClippedExtent(clippedExtent);

            DRange2d dtmRange;

            dtmRange.low.x = clippedExtent.low.x;
            dtmRange.low.y = clippedExtent.low.y;
            dtmRange.high.x = clippedExtent.high.x;
            dtmRange.high.y = clippedExtent.high.y;                   

            const double uorPerMeters = dgnModel_getUorPerMeter(m_element.GetModelRef()->GetRoot());
                       
            m_multiResolutionGridMaterialManagerPtr = DTMElementHandlerManager::GetMultiResolutionGridManagerCreator()->CreateMaterialManager(rasterTextureSourceID,      
                                                                                                                                              dtmRange,           
                                                                                                                                              minimumPixelSizeInUORS, 
                                                                                                                                              uorPerMeters);                 
            }                    
        }
    
    return m_multiResolutionGridMaterialManagerPtr;
    }


IMultiResolutionGridManagerPtr MrDTMDataRef::GetMultiResolutionGridDtmManager()
    {
    if ((m_multiResolutionGridDtmManagerPtr == 0) && 
        (DTMElementHandlerManager::GetMultiResolutionGridManagerCreator() != 0) && 
        (m_mrDTM != 0))
        {            
        DRange2d dtmRange; 
        
		DRange3d clippedExtent;

        int status = GetClippedExtent(clippedExtent);
		
		BeAssert(status == SUCCESS);
						        
        dtmRange.low.x = clippedExtent.low.x;
        dtmRange.low.y = clippedExtent.low.y;
        dtmRange.high.x = clippedExtent.high.x;
        dtmRange.high.y = clippedExtent.high.y;
                                          
        DRange3d mrDtmRange;

        GetExtents(mrDtmRange);

        Transform trsf;

        getStorageToUORMatrix(trsf, GetElement().GetModelRef(), GetElement(), false);
            
        bsiTransform_multiplyDPoint3dArrayInPlace(&trsf, (DPoint3dP)&mrDtmRange, 2);

        double maxDimension = max(mrDtmRange.high.x - mrDtmRange.low.x, mrDtmRange.high.y - mrDtmRange.low.y);        
                
        //Here is some heuristic to recreate a multi-resolution grid similar 
        //(i.e. : same number of resolutions and same scaling) to the point index quadtree.
        DPoint2d pointToPointToleranceInUors;

        int nbLevel = m_mrDTM->GetNbResolutions(DTM_QUERY_DATA_POINT);

        pointToPointToleranceInUors.x = maxDimension / pow(2.0, nbLevel - 1) / 257;
        pointToPointToleranceInUors.y = pointToPointToleranceInUors.x;                                

        m_multiResolutionGridDtmManagerPtr = DTMElementHandlerManager::GetMultiResolutionGridManagerCreator()->CreateSimpleManager(dtmRange, 
                                                                                                                                   pointToPointToleranceInUors);                                          
        }

    return m_multiResolutionGridDtmManagerPtr;
    }



void MrDTMDataRef::FreeRasterTextureDrapingResources()
    {
    m_multiResolutionGridMaterialManagerPtr = 0;
    m_rasterDrapingSrcPtr = 0;
    }

bool MrDTMDataRef::GetDtmForSingleResolution(BcDTMPtr& singleResolutionDtm, 
                                             long                   maximumNumberOfPoints)
    {
    BeAssert(singleResolutionDtm == 0);

    IMrDTMPtr& mrDTMPtr = GetMrDTM();
            
    if (mrDTMPtr != 0)
        {        
        int            status;
        DTMPtr         dtmPtr;          
        IMrDTMQueryPtr fullResLinearQueryPtr;
        
        //Query the linears 
        if ((m_mrDTM->GetBaseGCS() == 0) || (GetDestinationGCS() == 0))
            {            
            //Get the query interfaces
            fullResLinearQueryPtr = m_mrDTM->GetQueryInterface(DTM_QUERY_FULL_RESOLUTION, DTM_QUERY_DATA_LINEAR);
            }
        else
            {                                        
            DRange3d drange;
            GetExtents(drange);

            Bentley::GeoCoordinates::BaseGCSPtr destinationGCS(GetDestinationGCS());

            fullResLinearQueryPtr = m_mrDTM->GetQueryInterface(DTM_QUERY_FULL_RESOLUTION, 
                                                               DTM_QUERY_DATA_LINEAR,
                                                               destinationGCS,
                                                               drange);              
            }

        IMrDTMQueryPtr fixResPointQueryPtr(m_mrDTM->GetQueryInterface(DTM_QUERY_FIX_RESOLUTION_VIEW, 
                                                                      DTM_QUERY_DATA_POINT));    
      
        if (fullResLinearQueryPtr != 0)
            {                                    
            IMrDTMFullResolutionLinearQueryParamsPtr mrDtmQueryParametersPtr(IMrDTMFullResolutionLinearQueryParams::CreateParams());

            mrDtmQueryParametersPtr->SetEdgeOptionTriangulationParam(m_edgeMethod);            
            mrDtmQueryParametersPtr->SetMaxSideLengthTriangulationParam(_GetEdgeMethodLengthInStorage());

            if (fixResPointQueryPtr != 0)
                {                        
                mrDtmQueryParametersPtr->SetTriangulationState(false);                
                }

            //Currently the high quality display mode is not influencing the maximum number of linear 
            //points use to obtain a single resolution representation of the MrDTM.
            mrDtmQueryParametersPtr->SetMaximumNumberOfPointsForLinear(MAX_NB_POINTS_FOR_LINEAR);

            status = AddClipOnQuery(fullResLinearQueryPtr);

            BeAssert(status == SUCCESS);

            status = fullResLinearQueryPtr->Query(dtmPtr, 0, 0, IMrDTMQueryParametersPtr(mrDtmQueryParametersPtr));

            BeAssert(status == SUCCESS);
            }
                                  
        //Query the points       
        if (fixResPointQueryPtr != 0)
            {
            IMrDTMFixResolutionMaxPointsQueryParamsPtr queryParamsPtr(IMrDTMFixResolutionMaxPointsQueryParams::CreateParams());
            
            queryParamsPtr->SetMaximumNumberPoints(maximumNumberOfPoints);                        

            queryParamsPtr->SetEdgeOptionTriangulationParam(m_edgeMethod);            
            queryParamsPtr->SetMaxSideLengthTriangulationParam(_GetEdgeMethodLengthInStorage());
                
            //Query the view
            DTMPtr singleResolutionViewDtmPtr; 

            status = fixResPointQueryPtr->Query(singleResolutionViewDtmPtr, 0, 0, IMrDTMQueryParametersPtr(queryParamsPtr));

            //singleResolutionViewDtmPtr might be null if the maximum number of points requested is less then the number of 
            //points in the lowest resolution. In that case, takes the lowest resolution instead.
            if (singleResolutionViewDtmPtr == 0)
                {          
                IMrDTMFixResolutionIndexQueryParamsPtr queryParamsPtr(IMrDTMFixResolutionIndexQueryParams::CreateParams());                                                           

                queryParamsPtr->SetResolutionIndex(0);                            
                queryParamsPtr->SetEdgeOptionTriangulationParam(m_edgeMethod);            
                queryParamsPtr->SetMaxSideLengthTriangulationParam(_GetEdgeMethodLengthInStorage());                

                status = fixResPointQueryPtr->Query(singleResolutionViewDtmPtr, 0, 0, IMrDTMQueryParametersPtr(queryParamsPtr));
                }

            BeAssert(singleResolutionViewDtmPtr != 0);
                        
            IMrDTMPtr      singleResMrDTMViewPtr = IMrDTMPtr((IMrDTM*)singleResolutionViewDtmPtr.get());
            IMrDTMQueryPtr fullResQueryPtr;

            if ((m_mrDTM->GetBaseGCS() == 0) || (GetDestinationGCS() == 0))
                {
                fullResQueryPtr = singleResMrDTMViewPtr->GetQueryInterface(DTM_QUERY_FULL_RESOLUTION, 
                                                                           DTM_QUERY_DATA_POINT);                
                }
            else
                {
                DRange3d drange;
                GetExtents(drange);

                Bentley::GeoCoordinates::BaseGCSPtr destinationGCS(GetDestinationGCS());

                fullResQueryPtr = singleResMrDTMViewPtr->GetQueryInterface(DTM_QUERY_FULL_RESOLUTION, 
                                                                           DTM_QUERY_DATA_POINT, 
                                                                           destinationGCS,
                                                                           drange);     
                }

            BeAssert(fullResQueryPtr != 0);
                                
            IMrDTMQueryParametersPtr queryParam((const IMrDTMQueryParametersPtr&)IMrDTMFullResolutionQueryParams::CreateParams());

            queryParam->SetEdgeOptionTriangulationParam(m_edgeMethod);            
            queryParam->SetMaxSideLengthTriangulationParam(_GetEdgeMethodLengthInStorage());
           
            status = AddClipOnQuery(fullResQueryPtr);

            BeAssert(status == SUCCESS);
                    
            status = fullResQueryPtr->Query(dtmPtr, 0, 0, queryParam);        

            BeAssert(status == SUCCESS);                
            }

        if (dtmPtr != 0)
            {
            singleResolutionDtm = dtmPtr->GetBcDTM();
            }
        }
    
    return (singleResolutionDtm != 0);
    }


//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
/*
void MrDTMViewData::GetExtents(DPoint3d& low, DPoint3d& high)
    {
    if (!m_mrDTM)
        ReadFile();

    if (!m_mrDTM)
        return;

    BC_DTM_OBJ* pLastSubResDTM = m_mrDTM->GetLastSubResDTM();

    BeAssert(pLastSubResDTM != 0);

    low = m_minPoint;
    high = m_maxPoint;              
    }*/

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
DgnGCSP MrDTMDataRef::GetTargetGCS() 

    // Temporary stuff ... Extracted from MstnGCS::GetReprojectedTarget()
    {
    DgnModelRefP    targetModelRef  = NULL;
    DgnGCSP        tempTargetGCS       = NULL;
    DgnModelRefP    parentModelRef  = this->GetElement().GetModelRef();

    for ( ; (NULL != parentModelRef); parentModelRef = parentModelRef->GetParentModelRefP())
        {
        DgnGCSP    parentGCS;

        if (NULL != (parentGCS = Bentley::GeoCoordinates::DgnGCS::FromModel (parentModelRef, true)))
            {
            targetModelRef = parentModelRef;
            tempTargetGCS      = parentGCS;

            // if this modelRef's cache is not set to reprojected, or it has a non-identity transform, then it has a transform to get back to its parent (or its the root). 
            // (The non-identity transform happens in the case where the reference is attached as Reprojected, but we can calculate a linear transform to accomplish that).
            // We want to reproject our data to the GCS of the parent (which targetModelRef is currently set to).
            DgnAttachmentP  refP = parentModelRef->AsDgnAttachmentP();
            if (NULL == refP) 
                break;
            DPoint3d masterOrigin;
            refP->GetMasterOrigin(masterOrigin);
            const DPoint3d refOrigin(refP->GetRefOrigin());

            if ( (ATTACHMETHOD_GeographicProjected != refP->GetAttachMethod()) || 
                 ( (0.0 != masterOrigin.x) || (0.0 != masterOrigin.y) || (0.0 != masterOrigin.z) ) ||
                 ( (0.0 != refOrigin.x) || (0.0 != refOrigin.y) || (0.0 != refOrigin.z) ) ||
                 (!refP->GetRotMatrix().isIdentity()) ||
                 (1.0 != refP->GetDisplayScale()) )
                break;
            }
        else
            {
            // there is no GCS associated with a parent reference. Thus it can't be attached geographically, and we don't want any
            // of it's children attached geographically either.
            tempTargetGCS      = NULL;
            break;
            }
        }

    return tempTargetGCS;    
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
int MrDTMDataRef::ReadFile()
    {
    return ReadFile(GetTargetGCS());
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
int MrDTMDataRef::ReadFile(DgnGCSP targetGCS)
    {
    StatusInt status=BSISUCCESS;
    
    WString fileName(m_mrdtmMonikerPtr->ResolveFileName(&status));
    
    //Should have some info saved in an XAttribute concerning if 
    //the file has been generated at least once.                        

    //Assume that when MSDocumentMoniker::GetFileName doesn't return success the file is not found.
    if (status != 0)
        {                           
        m_iDTMFileLastLoadStatus = MRDTMSTATUS_MISSING_FILE;

        WString detailedMsg = TerrainModelElementResources::GetString(MSG_TERRAINMODEL_STMFileNotExist);
        detailedMsg.ReplaceAll (L"{1}", fileName.GetWCharCP());
       
        WString msg = TerrainModelElementResources::GetString(MSG_TERRAINMODEL_CannotOpenSTM);
        NotificationManager::OutputMessage(NotifyMessageDetails(OutputMessagePriority::Warning, msg.c_str(), detailedMsg.c_str()));

        return -1;
        }

    //Ensure that the file is closed.
    m_mrDTM = 0;

    m_mrDTM = IMrDTM::GetFor(fileName.GetWCharCP(), m_isReadOnly, true, status);
    
    // If we try to open the file in ReadWrite and we have an error, try to open the file in ReadOnly
    if (!m_isReadOnly && (status != SUCCESS)) 
        {
        //Ensure that the file is closed.
        m_mrDTM = 0;

        m_isReadOnly = true;
        m_mrDTM = IMrDTM::GetFor(fileName.GetWCharCP(), m_isReadOnly, true, status);
        }
    
    if (status != SUCCESS)
        {  
        m_mrDTM = nullptr;
        
        if (status == DTMSTATUS_UNSUPPORTED_VERSION)      
            {
            m_iDTMFileLastLoadStatus = MRDTMSTATUS_UNSUPPORTED_VERSION;
            WString msgW = TerrainModelElementResources::GetString(MSG_TERRAINMODEL_UnsupportedFileVersion);
            NotificationManager::OutputMessage(NotifyMessageDetails(OutputMessagePriority::Error, msgW.c_str(), fileName.c_str()));
            }
        else
            {                     
            m_iDTMFileLastLoadStatus = MRDTMSTATUS_INVALID_FILE;     
           
            WString detailedMsg = TerrainModelElementResources::GetString(MSG_TERRAINMODEL_CannotOpenSTMFile);
            detailedMsg.ReplaceAll (L"{1}", fileName.GetWCharCP());
           
            WString msg = TerrainModelElementResources::GetString(MSG_TERRAINMODEL_CannotOpenSTM);
            NotificationManager::OutputMessage(NotifyMessageDetails(OutputMessagePriority::Error, msg.c_str(), detailedMsg.c_str()));
            }
        return status;
        }
    else
        {
        m_iDTMFileLastLoadStatus = MRDTMSTATUS_FILE_LOADED;
        
        if (m_mrDTM->GetState() != MRDTM_STATE_EMPTY)
            {
            //MS : In the  
            //m_mrDTM->SetMinScreenPixelsPerPoint(m_minScreenPixelsPerPoint);  
            IMrDTMQueryPtr fixResQueryPtr = m_mrDTM->GetQueryInterface(DTM_QUERY_FIX_RESOLUTION_VIEW, DTM_QUERY_DATA_POINT);                

            if (fixResQueryPtr != 0)
                {
                IMrDTMFixResolutionIndexQueryParamsPtr queryParamsPtr(IMrDTMFixResolutionIndexQueryParams::CreateParams());

                queryParamsPtr->SetResolutionIndex(1); 

                //No edge removal for the lowest resolution view.
                queryParamsPtr->SetEdgeOptionTriangulationParam(0);            
                queryParamsPtr->SetMaxSideLengthTriangulationParam(0);

                //Get the lowest resolution view.        
                BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmPtr;
                     
                fixResQueryPtr->Query(dtmPtr, 0, 0, IMrDTMQueryParametersPtr(queryParamsPtr.get()));
                    
                BeAssert(dynamic_cast<IMrDTM*>(dtmPtr.get()) != 0);

                m_MrDTMlowestResolutionPointView = (IMrDTM*)dtmPtr.get();
                }
            }
        }

    BeAssert(status == 0);

    //When opening the DTM the last sub-resolution DTM should be the least precise 
    //sub-resolution of the MrDTM, whose range should represent the range of the whole DTM.         
    DRange3d initialRange;
    status = m_mrDTM->GetRange(initialRange);      

    BeAssert(status == 0);

    Bentley::GeoCoordinates::BaseGCSPtr sourceGCSPtr(m_mrDTM->GetBaseGCS());        

    //Reproject the range to the target GCS
    if ((sourceGCSPtr != 0) && (targetGCS != 0) && (sourceGCSPtr->IsEquivalent(*targetGCS) == false))
        {           
        Bentley::GeoCoordinates::BaseGCSPtr targetGCS2(targetGCS);
        status = m_mrDTM->GetRangeInSpecificGCS(initialRange.low, initialRange.high, targetGCS2);      

        // If the status is different than SUCCESS then most likely the extent of the datasource cannot be reprojected (in any portions)
        // to the target GCS. In this target GCS the extent is thus NULL and we fold min/max one upon the other.
        if (status == ERROR)
            {
            initialRange.low.x = 0.0;
            initialRange.low.y = 0.0;
            initialRange.low.z = 0.0;
            initialRange.high.x = 0.0;
            initialRange.high.y = 0.0;
            initialRange.high.z = 0.0;
            }
        }

    m_minPoint.x = initialRange.low.x - ((initialRange.high.x - initialRange.low.x) / 100.0);
    m_minPoint.y = initialRange.low.y - ((initialRange.high.y - initialRange.low.y) / 100.0);
    m_minPoint.z = initialRange.low.z - ((initialRange.high.z - initialRange.low.z) / 100.0);
    m_maxPoint.x = initialRange.high.x + ((initialRange.high.x - initialRange.low.x) / 100.0);
    m_maxPoint.y = initialRange.high.y + ((initialRange.high.y - initialRange.low.y) / 100.0);
    m_maxPoint.z = initialRange.high.z + ((initialRange.high.z - initialRange.low.z) / 100.0);
 
    m_maxNbPoints = MAX_NB_POINTS_MIDDLE_VALUE;            
    
    return 0;
    }
       


    ViewContextP g_currentViewContextP = 0;

    int CheckTriangulationStopCallback(DTMFeatureType dtmFeatureType)
        {       
        if ((g_currentViewContextP != 0) && (g_currentViewContextP->CheckStop() == true))
            {             
            return DTM_ERROR;
            }  

        return DTM_SUCCESS;
        }

 static bool isTerminationCallbackSet = false;

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void MrDTMViewData::QueryRequiredDTM(ViewContextP context)
    {         
    if (DoProgressiveDraw((RefCountedPtr<DTMDataRef>&)m_DTMDataRef, context) == true)
        {
        if (isTerminationCallbackSet == false) 
            {                
            bool isSet = SetTriangulationTerminationCallback((checkTriangulationStopCallbackFP)CheckTriangulationStopCallback);

            BeAssert(isSet == true);

            isTerminationCallbackSet = true;                                    
            }        
        }
    else
        {
        if (isTerminationCallbackSet == true) 
            {
            bool isSet = SetTriangulationTerminationCallback(0);

            BeAssert(isSet == true);

            isTerminationCallbackSet = false;                                    
            }        
        }   
                    
    if ((context->GetViewport() == 0) || (DTMElementHandlerManager::GetMrDTMActivationRefCount() == 0))        
        {         
        //Return the points from the lowest resolution. 
        //That might not be a wise solution for all cases.               
        //int status;       
        if ((m_previousMrDTMDrawingInfoPtr != nullptr) || (m_dtmPtr == 0))
            {
            BcDTMPtr singleResolutionDtm;

            bool status = m_DTMDataRef->GetDtmForSingleResolution(singleResolutionDtm, 
                                                                  MAX_NB_POINTS_FOR_OVERVIEW);

            if ((status != false) && (singleResolutionDtm != 0))
                {
                m_dtmPtr = singleResolutionDtm;
                }
                           
            m_previousMrDTMDrawingInfoPtr = 0;         
            }
        }
    else
        {  
        MrDTMDrawingInfoPtr currentMrDTMDrawingInfoPtr(new MrDTMDrawingInfo(context));
        
        if ((m_previousMrDTMDrawingInfoPtr != nullptr) && 
            (m_previousMrDTMDrawingInfoPtr->GetDrawPurpose() != DrawPurpose::UpdateDynamic) && 
            (m_dtmPtr != 0))
            {            
            //If the m_dtmPtr equals 0 it could mean that the last data request to the STM was cancelled, so start a new request even 
            //if the view has not changed.                 
            if (m_previousMrDTMDrawingInfoPtr->HasAppearanceChanged(currentMrDTMDrawingInfoPtr) == false)
                return;                            
            }

        g_currentViewContextP = context;

        m_previousMrDTMDrawingInfoPtr = currentMrDTMDrawingInfoPtr;

        // Need to get the fence info.
        DTMDrawingInfo drawingInfo;        
        DTMElementDisplayHandler::GetDTMDrawingInfo(drawingInfo, m_DTMDataRef->GetElement(), m_DTMDataRef, context);

        if (!drawingInfo.IsVisible ())
            {                        
            return;
            }        
                                                                                                              
        DMatrix4d localToView(context->GetLocalToView());

        Transform scaleTransform; 
  
        getStorageToUORMatrix(scaleTransform, m_DTMDataRef->GetElement().GetModelRef(), m_DTMDataRef->GetElement(), false);
                                                 
        DMatrix4d scaleMatrix;   
        bsiDMatrix4d_initFromTransform (&scaleMatrix, &scaleTransform);

        bsiDMatrix4d_multiply(&localToView, &localToView, &scaleMatrix);

        double apparentPlottingDPIScaleDownFactor = 1.0;       

        if (context->GetDrawPurpose() == DrawPurpose::Plot)
            {          
            assert(IMrDTMDataRef::GetPlotterInterface() != 0);
                                   
            //TR 328196 - Ensure that DPI used during a plot in wireframe is the same as some arbitrary DPI 
            //            representing a computer screen.
            if (true == IsWireframeRendering(*context) && IMrDTMDataRef::GetPlotterInterface() != 0)
                {                                                                      
                DPoint2d printerResolution(IMrDTMDataRef::GetPlotterInterface()->GetPrinterResolutionInInches ());
                                                               
                DMatrix4d plotterDpiToViewDpiScaleMatrix;

                memset(plotterDpiToViewDpiScaleMatrix.coff, 0, sizeof(plotterDpiToViewDpiScaleMatrix.coff));

                plotterDpiToViewDpiScaleMatrix.coff[0][0] = (printerResolution.x * WIREFRAME_PRINTING_DPI);
                plotterDpiToViewDpiScaleMatrix.coff[1][1] = (printerResolution.y * WIREFRAME_PRINTING_DPI);
                plotterDpiToViewDpiScaleMatrix.coff[3][3] = 1;

                bsiDMatrix4d_multiply(&localToView, &plotterDpiToViewDpiScaleMatrix, &localToView);              
                }       
            else
                {           
                DPoint3d* tempPoints = new DPoint3d[drawingInfo.GetFence().numPoints];

                memcpy(tempPoints, drawingInfo.GetFence().points, sizeof(DPoint3d) * drawingInfo.GetFence().numPoints);
                                                                                             
                bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray(&localToView, tempPoints, tempPoints, drawingInfo.GetFence().numPoints);
  
                double polygonArea = fabs(bsiGeom_getXYPolygonArea(tempPoints, drawingInfo.GetFence().numPoints));                
                
                delete tempPoints;

                //Worst case estimation based on the assumption that the points are at the same density (e.g. : DEM raster).                                                                              
                double nbOfReturnedPointsEstimation = polygonArea / m_DTMDataRef->_GetPointDensityForShadedView();

                if (nbOfReturnedPointsEstimation > MAX_NB_POINTS_DURING_SMOOTH_PRINT) 
                    {                    
                    apparentPlottingDPIScaleDownFactor = sqrt(MAX_NB_POINTS_DURING_SMOOTH_PRINT / nbOfReturnedPointsEstimation);                    

                    DMatrix4d plotterDpiToViewDpiScaleMatrix;

                    memset(plotterDpiToViewDpiScaleMatrix.coff, 0, sizeof(plotterDpiToViewDpiScaleMatrix.coff));

                    plotterDpiToViewDpiScaleMatrix.coff[0][0] = apparentPlottingDPIScaleDownFactor;
                    plotterDpiToViewDpiScaleMatrix.coff[1][1] = apparentPlottingDPIScaleDownFactor;
                    plotterDpiToViewDpiScaleMatrix.coff[3][3] = 1;

                    bsiDMatrix4d_multiply(&localToView, &plotterDpiToViewDpiScaleMatrix, &localToView);
                    }                                
                }
            }

        DPoint3d viewBox[8];
        
        GetViewBoxFromContext(viewBox, _countof(viewBox), context, drawingInfo);
    
        DMatrix4d rootToStorage;    


        //Convert the view box in storage.        
        bool inverted = bsiDMatrix4d_invertQR(&rootToStorage, &scaleMatrix);
      
        BeAssert(inverted != 0);
        
        bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray(&rootToStorage, viewBox, viewBox, 8);                
                                                                                                                                                                 
        BeAssert((m_mrDtmFullResolutionLinearQueryPtr != 0) || (m_mrDtmViewDependentPointQueryPtr != 0));

        int status = SUCCESS;
        m_dtmPtr = 0;

        if (m_mrDtmFullResolutionLinearQueryPtr != 0)
            {
            IMrDTMFullResolutionLinearQueryParamsPtr mrDtmQueryParametersPtr(IMrDTMFullResolutionLinearQueryParams::CreateParams());

            mrDtmQueryParametersPtr->SetEdgeOptionTriangulationParam(m_DTMDataRef->m_edgeMethod);            
            mrDtmQueryParametersPtr->SetMaxSideLengthTriangulationParam(m_DTMDataRef->_GetEdgeMethodLengthInStorage());

            if (m_mrDtmViewDependentPointQueryPtr != 0)
                {                        
                mrDtmQueryParametersPtr->SetTriangulationState(false);                
                }

            if (MrDTMElementDisplayHandler::IsHighQualityDisplayForMrDTM() == false)
                {
                mrDtmQueryParametersPtr->SetMaximumNumberOfPointsForLinear(MAX_NB_POINTS_FOR_LINEAR);
                }
            else
                {
                mrDtmQueryParametersPtr->SetMaximumNumberOfPointsForLinear(MAX_NB_POINTS_FOR_LINEAR_IN_ANIMATION);
                }

            status = m_mrDtmFullResolutionLinearQueryPtr->Query(m_dtmPtr,                            
                                                                drawingInfo.GetFence().points, 
                                                                drawingInfo.GetFence().numPoints,
                                                                IMrDTMQueryParametersPtr(mrDtmQueryParametersPtr));

            if (status != SUCCESS)
                {
                m_dtmPtr = 0;
                }                                                                         
            }

        if (m_mrDtmViewDependentPointQueryPtr != 0)
            {
            IMrDTMViewDependentQueryParamsPtr viewDependentQueryParams(IMrDTMViewDependentQueryParams::CreateParams()); 

            viewDependentQueryParams->SetEdgeOptionTriangulationParam(m_DTMDataRef->m_edgeMethod);            
            viewDependentQueryParams->SetMaxSideLengthTriangulationParam(m_DTMDataRef->_GetEdgeMethodLengthInStorage());           
                        
            //During an animation preview there is only an DrawPurpose::UpdateDynamic draw, 
            //with no DrawPurpose::Update draw following.
            if ((context->GetDrawPurpose() == DrawPurpose::UpdateDynamic) && 
                (DTMElementHandlerManager::IsDrawForAnimation() == false))
                {
                viewDependentQueryParams->SetMinScreenPixelsPerPoint(MRDTM_GUI_TO_VIEW_POINT_DENSITY(20));                
                }
            else
                {                    
                if (false == IsWireframeRendering(*context))
                    {
                    viewDependentQueryParams->SetMinScreenPixelsPerPoint(m_DTMDataRef->_GetPointDensityForShadedView());
                    }
                else
                    {
                    viewDependentQueryParams->SetMinScreenPixelsPerPoint(m_DTMDataRef->_GetPointDensityForWireframeView());
                    }
                }
                                                                                                                          
            //Inflate the viewbox in 2D so the Z range equals the Z range of the MrDTM.           
            if (context->Is3dView() == false)
                {        

                DRange3d range;      
                m_DTMDataRef->_GetExtents(range);       

                DRange3d viewRange;
                bsiDRange3d_initFromArray(&viewRange, viewBox, 8);
                
                for (int ptInd = 0; ptInd < 8; ptInd++)
                    {
                    if (viewBox[ptInd].z == viewRange.low.z)
                        {
                        viewBox[ptInd].z = range.low.z;
                        }   
                    else
                        {
                        BeAssert(viewBox[ptInd].z == viewRange.high.z);
                        viewBox[ptInd].z = range.high.z;
                        }                        
                    }
                }            

            viewDependentQueryParams->SetViewBox(viewBox);       
            viewDependentQueryParams->SetRootToViewMatrix(localToView.coff);            
            
            //During a plot for a none wireframe view try to honor as closely as possible the maximum number of points 
            //that can safely be loaded on a recent computer. To achieve that an extra query is done to obtain the 
            //number of points returned for the calculated scale down factor, and the scale down factor is then ajusted if 
            //it was too much conservative.
            if (apparentPlottingDPIScaleDownFactor != 1.0)
                {                
                BeAssert(apparentPlottingDPIScaleDownFactor < 1.0);
                BeAssert(context->GetDrawPurpose() == DrawPurpose::Plot);

                BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmPtr;    

                bool oldTriangulationState = viewDependentQueryParams->GetTriangulationState();                

                viewDependentQueryParams->SetTriangulationState(false);
                                   
                //! @return The number of points of the DTM..                    
                status = m_mrDtmViewDependentPointQueryPtr->Query(dtmPtr,                            
                                                                  drawingInfo.GetFence().points, 
                                                                  drawingInfo.GetFence().numPoints,
                                                                  IMrDTMQueryParametersPtr(viewDependentQueryParams));               

                viewDependentQueryParams->SetTriangulationState(oldTriangulationState);                
                                
                double ajustedApparentPlottingDPIScaleDownFactor = sqrt(pow(apparentPlottingDPIScaleDownFactor, 2) * MAX_NB_POINTS_DURING_SMOOTH_PRINT / dtmPtr->GetPointCount());

                //Don't load more points than are required by the DPI specified for the print output device.
                if (ajustedApparentPlottingDPIScaleDownFactor > 1.0)
                    {
                    ajustedApparentPlottingDPIScaleDownFactor = 1.0;
                    }

                DMatrix4d ajustedPlotterDpiToViewDpiScaleMatrix;

                memset(ajustedPlotterDpiToViewDpiScaleMatrix.coff, 0, sizeof(ajustedPlotterDpiToViewDpiScaleMatrix.coff));
                
                ajustedPlotterDpiToViewDpiScaleMatrix.coff[0][0] = ajustedApparentPlottingDPIScaleDownFactor / apparentPlottingDPIScaleDownFactor;
                ajustedPlotterDpiToViewDpiScaleMatrix.coff[1][1] = ajustedApparentPlottingDPIScaleDownFactor / apparentPlottingDPIScaleDownFactor;
                ajustedPlotterDpiToViewDpiScaleMatrix.coff[3][3] = 1;

                bsiDMatrix4d_multiply(&localToView, &ajustedPlotterDpiToViewDpiScaleMatrix, &localToView);                                                

                viewDependentQueryParams->SetRootToViewMatrix(localToView.coff);            
                }            

            //! @return The number of points of the DTM..                       
            status = m_mrDtmViewDependentPointQueryPtr->Query(m_dtmPtr,                            
                                                              drawingInfo.GetFence().points, 
                                                              drawingInfo.GetFence().numPoints,
                                                              IMrDTMQueryParametersPtr(viewDependentQueryParams));
                                                
            if (status != SUCCESS)
                {
                m_dtmPtr = 0;
                }
            }                
        }

    g_currentViewContextP = 0;
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
