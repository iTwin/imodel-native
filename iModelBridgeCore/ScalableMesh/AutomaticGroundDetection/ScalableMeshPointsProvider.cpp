/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#undef static_assert
#include "ScalableMeshPointsProvider.h"
#ifdef VANCOUVER_API
#include <DgnGeoCoord\DgnGeoCoord.h>
#else
#include <DgnPlatform\DgnGeoCoord.h>
#endif

#include "..\GeoCoords\ReprojectionUtils.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL
using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

//BeCriticalSection ScalableMeshPointsProvider::s_MRMEshQueryCS;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IPointsProviderPtr ScalableMeshPointsProvider::CreateFrom(ScalableMesh::IScalableMeshPtr& smesh, DRange3dCR boundingBoxInUors)
    {
    return new ScalableMeshPointsProvider(smesh, boundingBoxInUors);
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MarcBedard      11/01
+---------------+---------------+---------------+---------------+---------------+------*/
IPointsProviderPtr ScalableMeshPointsProvider::_Clone() const
    {
    return new ScalableMeshPointsProvider(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshPointsProvider::ScalableMeshPointsProvider(IScalableMeshPtr& smesh, 
                                                       DRange3dCR        boundingBoxInUors)
:IPointsProvider(boundingBoxInUors), 
 m_smesh(smesh)
    {
    m_transform = Transform::FromIdentity();
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshPointsProvider::ScalableMeshPointsProvider(ScalableMeshPointsProvider const& object)
:IPointsProvider(object)
    {
    /*
    BeCriticalSectionHolder lock(s_MRMEshQueryCS);

    IMRMeshAttachment* pIMRMeshQuery = dynamic_cast<IMRMeshAttachment*>(&m_eh.GetHandler());
    if (pIMRMeshQuery != NULL)
        pIMRMeshQuery->_GetAttachmentInfo(m_info, m_eh);
        */
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshPointsProvider::~ScalableMeshPointsProvider()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshPointsProvider::_PrefetchPoints()
    {
    //NEEDS_WORK_MST : Crash in multithread with that, might not be required, remove all the _Prefetch logic if so.
    return; 
    #if 0 
    InternalQueryPoints();

    //if we have a threshold on the number of points if it's goes past that clear the vector
    if (m_maxPointsToPreFetch != -1 && m_prefetchedPoints.size() > m_maxPointsToPreFetch)
        {
        m_prefetchedPoints.clear();
        m_prefetchPoints = false;
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double ScalableMeshPointsProvider::_GetExportResolution() const
    {
    /*
    double uorPerMeter(mdlModelRef_getUorPerMeter(mdlModelRef_getActive()));

    double resolution((m_info.m_exportResolution / uorPerMeter));

    //Use smallest resolution between expected from caller and what is set in MrMesh info
    if (m_exportResolution>0.0)
        resolution = m_exportResolution < resolution ? m_exportResolution : resolution;
        -*/
    //assert(!"Not done yet");
    //GDZERO

    double resolution = 1;

    return resolution;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis    08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    ScalableMeshPointsProvider::GetPoints(bvector<DPoint3d>& points, double* resolution, ClipVectorCP clip) const
    {    
    assert(m_smesh.IsValid());
    assert(clip != nullptr);
	assert(m_prefetchPoints == false);
/*
    if (m_prefetchPoints)
        {        
        points.insert(points.end(), m_prefetchedPoints.begin(), m_prefetchedPoints.end());
        }
		*/
    
    ScalableMesh::IScalableMeshMeshQueryPtr meshQueryInterface = m_smesh->GetMeshQueryInterface(ScalableMesh::MESH_QUERY_FULL_RESOLUTION);
    bvector<ScalableMesh::IScalableMeshNodePtr> returnedNodes;
    ScalableMesh::IScalableMeshMeshQueryParamsPtr params = ScalableMesh::IScalableMeshMeshQueryParams::CreateParams();
    params->SetLevel(m_smesh->GetNbResolutions() - 1);

    DPoint3d box[8];
    size_t nPts = 0;
    DRange3d queryRange;    
    DRange3d queryRangeInSrcGCS;
    ClipVectorPtr queryClip;
            
    if (clip != nullptr)
        {     
        clip->GetRange(queryRange, &m_transform);

        if (m_smesh->IsCesium3DTiles())
            {
			//assert(!"Deactivated for now. Need ECEF clipping capability and correct UORs to meters transfo");
//#if 0 			
            Transform uorTransfo(Transform::FromRowValues(1 / 10000.0, 0, 0, 0,
                                                            0, 1 / 10000.0, 0, 0,
                                                            0, 0, 1 / 10000.0, 0));

            Transform smToDestTrans(m_smesh->GetReprojectionTransform());

            smToDestTrans = Transform::FromProduct(uorTransfo, smToDestTrans);            
            Transform destToSmTrans; 
            bool resultInv = destToSmTrans.InverseOf(smToDestTrans);
            assert(resultInv);                                                

            queryClip = ClipVector::CreateCopy(*clip);
            queryClip->TransformInPlace(destToSmTrans);            
//#endif			
            }
        else
            {                
            if (m_destinationGcs.IsValid())
                ReprojectRange(queryRangeInSrcGCS, queryRange, m_destinationGcs, m_sourceGcs, GeoCoordInterpretation::Cartesian, m_geocoordInterpretation);
            else
                queryRangeInSrcGCS = queryRange;

            if (queryRangeInSrcGCS.XLength() == 0 || queryRangeInSrcGCS.YLength() == 0)
                return SUCCESS; 

            queryRangeInSrcGCS.Get8Corners(box);
            nPts = 8;
            }
        }
      
    bvector<ScalableMesh::IScalableMeshNodePtr> nodes;

    if (queryClip.IsValid())
        {
        meshQueryInterface->Query(nodes, queryClip.get(), params);
        }
    else
        { 
        meshQueryInterface->Query(nodes, nPts> 0 ? box : nullptr, (int)nPts, params);
        }
    
    //meshQueryInterface->Query(nodes, ClipVectorCP                                       queryExtent3d, params);


    ScalableMesh::IScalableMeshMeshFlagsPtr flags = ScalableMesh::IScalableMeshMeshFlags::Create();
    flags->SetLoadGraph(false);
    flags->SetLoadTexture(false);
    flags->SetLoadIndices(false);

    for (auto& node: nodes)
        {                
        if (!queryRangeInSrcGCS.IntersectsWith(node->GetContentExtent()))
            continue;
        
        auto mesh = node->GetMesh(flags);
                
        for (size_t ptInd = 0; ptInd <  mesh->GetNbPoints(); ptInd++)
            {        
            DPoint3d pt(mesh->EditPoints()[ptInd]);
        
            if (m_destinationGcs.IsValid())
                ReprojectPt(pt, pt, m_sourceGcs, m_destinationGcs, m_geocoordInterpretation, GeoCoordInterpretation::Cartesian);
                            
            if (queryRange.IsContained(pt))
                {
                points.push_back(pt);
                }
            }
                
        if (points.size() > 5000000) 
            {
            assert(!"Do many points covering grid");
            break;
            }        
        }

    if (!m_transform.IsIdentity())
        { 
        Transform inverseTrans;
        inverseTrans.InverseOf(m_transform);
        if (!points.empty())
            inverseTrans.Multiply(&points[0], (int)points.size());
        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshPointsProvider::InternalQueryPoints() const
    {
    if (!m_prefetchPoints)
        {
        //BeCriticalSectionHolder lock(s_MRMEshQueryCS);
        DRange3d range(m_boundingBoxInUors);
        Transform meterToNative(GetMeterToNativeTransform());
        Transform rootToNativeTransform(GetRootToNativeTransform());
        Transform nativeToMeter;
        nativeToMeter.InverseOf(meterToNative);

        //Transform range to native
        DRange3d rangeNative;   //in SRS
        rootToNativeTransform.Multiply(rangeNative, range);

        //use finer resolution or default                
        double meterToSRSFactor(GetMeterToSRSFactor());

        //Use smallest resolution between expected from caller and what is set in MrMesh info
        double resolution(GetExportResolution()* meterToSRSFactor);
        
        ClipVectorPtr clip = ClipVector::CreateFromPrimitive(ClipPrimitive::CreateFromBlock(rangeNative.low, rangeNative.high, false, ClipMask::All, NULL).get());

        GetPoints(m_prefetchedPoints, &resolution, clip.get());

        m_prefetchPoints = true;
        Transform transformToApply(nativeToMeter);
        if (!GetUseMeterUnit())
            {
            Transform nativeToUor;
            nativeToUor.InverseOf(rootToNativeTransform);
            transformToApply.Copy(nativeToUor);
            }
        if (!transformToApply.IsIdentity())
            {
            transformToApply.Multiply(&m_prefetchedPoints[0], (int)m_prefetchedPoints.size());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshPointsProvider::_ClearPrefetchedPoints()
    {
    m_prefetchedPoints.clear();
    m_prefetchPoints=false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double ScalableMeshPointsProvider::GetMeterToSRSFactor() const
    {
    double meterToSRSFactor(1.0);
    //Find meter to SRS transform
      
    GeoCoordinates::BaseGCSCPtr baseGcs(m_smesh->GetBaseGCS());
    
    if (baseGcs.IsValid())
        {
        WString unitName;        
        GeoCoordinates::Unit const* gcsUnit = GeoCoordinates::Unit::FindUnit(baseGcs->GetUnits(unitName));

        if (gcsUnit == nullptr)
            return meterToSRSFactor;

        meterToSRSFactor = gcsUnit->GetConversionFactor();
        }

    return meterToSRSFactor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Transform ScalableMeshPointsProvider::GetRootToNativeTransform() const 
    {
    // Get the transformation from the model to the root
    /**
    Transform toRoot;
    toRoot.initIdentity();
    DgnModelRefP model = m_eh.GetModelRef();
    DgnModelRefP rootModel = mdlModelRef_getActive();

    Transform displayTransform(m_info.m_transform);//Transform pushed during a draw
    if (model != rootModel)
        {
        //When it's a reference, must also apply transform between modelRef
        mdlRefFile_getTransformToParent(&toRoot, model->AsDgnAttachmentP(), rootModel->AsDgnAttachmentP());
        }

    Transform nativeToRoot(Transform::FromProduct(toRoot, displayTransform));

    Transform rootToNative;
    rootToNative.inverseOf(&nativeToRoot);
    */
    
    //GDZERO
    Transform rootToNative(Transform::FromIdentity());

    return rootToNative;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu St-Pierre               06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshPointsProvider::SetReprojectionInfo(GeoCoordInterpretation geocoordInterpretation,
                                                     BaseGCSPtr&            sourceGcs,
                                                     BaseGCSPtr&            destinationGcs)
    {
    m_geocoordInterpretation = geocoordInterpretation;
    m_sourceGcs = sourceGcs;
    m_destinationGcs = destinationGcs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Transform ScalableMeshPointsProvider::_GetMeterToNativeTransform() const 
    {
    // Get the transformation from the model to the root
    Transform uorToMeters(GetUorToMeterTransform(true));
    Transform rootToNative(GetRootToNativeTransform());

    Transform meterToUor(Transform::FromIdentity());
    meterToUor.InverseOf(uorToMeters);

    Transform meterToNative;
    meterToNative.InitProduct(rootToNative, meterToUor);
    //meterToNative.ProductOf(rootToNative, meterToUor);

    return meterToNative;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ScalableMeshPointsProvider::_GetFileName() const
    {/*
    BeFileName fileName(m_info.m_fileName.c_str());
    return fileName;
    */
    BeFileName fileName(L"");
    return fileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshPointsProvider::ScalableMeshPointsProviderPrefetchedIteratorImpl::ScalableMeshPointsProviderPrefetchedIteratorImpl(bvector<ReturnType>& prefetchedPoints, bool isAtEnd)
    {
    m_endItr = prefetchedPoints.end();
    if (!isAtEnd)
        {
        m_currentItr = prefetchedPoints.begin();
        }
    else
        {
        m_currentItr = m_endItr;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshPointsProvider::ScalableMeshPointsProviderPrefetchedIteratorImpl::~ScalableMeshPointsProviderPrefetchedIteratorImpl()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScalableMeshPointsProvider::ScalableMeshPointsProviderPrefetchedIteratorImpl::_IsDifferent(IPointsProvider::IPointsProviderIteratorImpl const& rhs)  const
    {
    ScalableMeshPointsProvider::ScalableMeshPointsProviderPrefetchedIteratorImpl const* pRhs = dynamic_cast<ScalableMeshPointsProvider::ScalableMeshPointsProviderPrefetchedIteratorImpl const*>(&rhs);
    if (pRhs == nullptr)
        return true;
    return (pRhs->m_currentItr != m_currentItr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshPointsProvider::ScalableMeshPointsProviderPrefetchedIteratorImpl::_MoveToNext()
    {
    m_currentItr++;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IPointsProvider::IPointsProviderIteratorImpl::ReturnType& ScalableMeshPointsProvider::ScalableMeshPointsProviderPrefetchedIteratorImpl::_GetCurrent() const
    {
    return *m_currentItr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScalableMeshPointsProvider::ScalableMeshPointsProviderPrefetchedIteratorImpl::_IsAtEnd() const
    {
    return m_currentItr == m_endItr;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ScalableMeshPointsProvider::_ComputeMemorySize()  const
    {
    size_t  memorySize(sizeof(*this));
    memorySize += m_prefetchedPoints.size() * sizeof(DPoint3d);
    return memorySize;
    }

//=======================================================================================
//! @bsiclass
//===============+===============+===============+===============+===============+=======
struct      ScalableMeshAttachmentPointsIteratorImpl : IPointsProvider::IPointsProviderIteratorImpl
    {
    public:
        friend struct ScalableMeshPointsProvider;
        friend IPointsProvider;

    private:

        bvector<DPoint3d>                 m_points;
        bvector<DPoint3d>::const_iterator m_endItr;
        bvector<DPoint3d>::const_iterator m_currentItr;
        mutable DPoint3d                  m_currentPtTransformed;
        Transform                         m_transformToApply;
        
        ScalableMeshAttachmentPointsIteratorImpl(ScalableMeshPointsProvider const & scalableMeshPointProvider, bool isAtEnd)
            {
            //GDZERO
            assert(isAtEnd == false);

           // BeCriticalSectionHolder lock(scalableMeshPointProvider.s_MRMEshQueryCS);
            DRange3d range(scalableMeshPointProvider.GetBoundingBox());
            Transform meterToNative(scalableMeshPointProvider.GetMeterToNativeTransform());
            Transform rootToNativeTransform(scalableMeshPointProvider.GetRootToNativeTransform());
            Transform nativeToMeter;
            nativeToMeter.InverseOf(meterToNative);

            //Transform range to native
            DRange3d rangeNative;   //in SRS
            rootToNativeTransform.Multiply(rangeNative, range);

            //use finer resolution or default            
            double meterToSRSFactor(scalableMeshPointProvider.GetMeterToSRSFactor());

            //Use smallest resolution between expected from caller and what is set in MrMesh info
            double resolution(scalableMeshPointProvider.GetExportResolution()* meterToSRSFactor);

            /*
            IMRMeshAttachment* pIMRMeshQuery = dynamic_cast<IMRMeshAttachment*>(&scalableMeshPointProvider.m_eh.GetHandler());
            BeAssert(pIMRMeshQuery != NULL);
            if (pIMRMeshQuery != NULL)
                {
                */                

                ClipVectorPtr clip = ClipVector::CreateFromPrimitive(ClipPrimitive::CreateFromBlock(rangeNative.low, rangeNative.high, false, ClipMask::All, NULL).get());
                                            
                BentleyStatus status = scalableMeshPointProvider.GetPoints(m_points, &resolution, clip.get());                
                assert(status == SUCCESS);
                m_endItr = m_points.end();
                                                
                if (isAtEnd)
                    m_currentItr =  m_endItr;
                else
                    m_currentItr = m_points.begin();
                //}

            m_transformToApply.Copy(nativeToMeter);
            if (!scalableMeshPointProvider.GetUseMeterUnit())
                {
                Transform nativeToUor;
                nativeToUor.InverseOf(rootToNativeTransform);
                m_transformToApply.Copy(nativeToUor);
                }
            }
        ~ScalableMeshAttachmentPointsIteratorImpl() {}

        virtual bool          _IsDifferent(IPointsProvider::IPointsProviderIteratorImpl const& rhs) const override
            {
                /*
            ScalableMeshAttachmentPointsIteratorImpl const* pRhs = dynamic_cast<ScalableMeshAttachmentPointsIteratorImpl const*>(&rhs);
            if (pRhs == nullptr)
                return true;
                */
            //GDZERO

            return (m_currentItr != m_endItr);
            }

        virtual void          _MoveToNext() override
            {
            ++m_currentItr;
            }
        virtual ReturnType&   _GetCurrent() const override
            {
            DPoint3d currentPt(*m_currentItr);
            m_transformToApply.Multiply(&m_currentPtTransformed, &currentPt, 1);
            return m_currentPtTransformed;
            }
        virtual bool          _IsAtEnd() const override
            {
            return m_currentItr == m_endItr;
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IPointsProvider::const_iterator ScalableMeshPointsProvider::_begin() const
    {
    m_currentIterator = new ScalableMeshAttachmentPointsIteratorImpl(*this, false/*isAtEnd*/);
    return IPointsProvider::const_iterator(*m_currentIterator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IPointsProvider::const_iterator ScalableMeshPointsProvider::_end() const
    {        
    return IPointsProvider::const_iterator(*m_currentIterator);
    }


/*---------------------------------------------------------------------------------**//**
* ScalableMeshPointsProvider
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshPointsProviderCreatorPtr ScalableMeshPointsProviderCreator::Create(IScalableMeshPtr&      smesh, 
                                                                               BaseGCSPtr             sourceGcs,
                                                                               BaseGCSPtr             destinationGcs,
                                                                               GeoCoordInterpretation geocoordInterpretation)
    {
    return new ScalableMeshPointsProviderCreator(smesh, sourceGcs, destinationGcs, geocoordInterpretation);
    }

ScalableMeshPointsProviderCreator::ScalableMeshPointsProviderCreator(IScalableMeshPtr&      smesh, 
                                                                     BaseGCSPtr&            sourceGcs,
                                                                     BaseGCSPtr&            destinationGcs,
                                                                     GeoCoordInterpretation geocoordInterpretation)
    {
    m_smesh = smesh;    
    m_sourceGcs = sourceGcs;
    m_destinationGcs = destinationGcs;
    m_geocoordInterpretation = geocoordInterpretation;
    }

ScalableMeshPointsProviderCreator::~ScalableMeshPointsProviderCreator()
    {    
    }

IPointsProviderPtr ScalableMeshPointsProviderCreator::_CreatePointProvider(DRange3d const& boundingBoxInUors) 
    {
    IPointsProviderPtr pointProvider(ScalableMeshPointsProvider::CreateFrom(m_smesh, boundingBoxInUors));

#ifdef VANCOUVER_API
    ((ScalableMeshPointsProvider*)pointProvider.get())->SetReprojectionInfo(m_geocoordInterpretation,
                                                                            m_sourceGcs,
                                                                            m_destinationGcs);
#endif
    return pointProvider;
    }

IPointsProviderPtr ScalableMeshPointsProviderCreator::_CreatePointProvider() 
    {    
    assert(!"Not supported yet");
    DRange3d range; 
    return ScalableMeshPointsProvider::CreateFrom(m_smesh, range);
    }

void ScalableMeshPointsProviderCreator::_GetAvailableRange(DRange3d& availableRange) 
    {   
    DRange3d smRange = DRange3d::NullRange();

    if (m_destinationGcs.IsValid())
        {
        DRange3d smRangeLocal;
        DTMStatusInt status = m_smesh->GetRange(smRangeLocal);
#ifdef VANCOUVER_API
        ReprojectRange(smRange, smRangeLocal, m_sourceGcs, m_destinationGcs, m_geocoordInterpretation, GeoCoordInterpretation::Cartesian);
#endif
        assert(status == SUCCESS);
        }
    else
        {
        DTMStatusInt status = m_smesh->GetRange(smRange);        
        assert(status == SUCCESS);
        }            

    if (m_extractionArea.size() > 0)
        {
        DRange3d areaRange(DRange3d::From(&m_extractionArea[0], (int)m_extractionArea.size()));
        areaRange.low.z = smRange.low.z;
        areaRange.high.z = smRange.high.z;
        availableRange.IntersectionOf(smRange, areaRange);

        assert(!availableRange.IsEmpty());
        }    
    else
        {
        availableRange = smRange;
        }
    }

void ScalableMeshPointsProviderCreator::SetExtractionArea(const bvector<DPoint3d>& area)
    {    
    if (m_destinationGcs.IsValid())
        { 
#ifdef VANCOUVER_API
        Reproject(m_extractionArea, area, m_sourceGcs, m_destinationGcs, m_geocoordInterpretation, GeoCoordInterpretation::Cartesian);
#endif 
        }
    else
        {
        m_extractionArea.insert(m_extractionArea.end(), area.begin(), area.end());
        }    
    }
            
END_BENTLEY_SCALABLEMESH_NAMESPACE
