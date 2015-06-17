#include <ScalableMeshHandler/ScalableMeshModel.h>
#include "QvCachedNodeManager.h"


BEGIN_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE

static ::DPoint3d const s_npcViewBox[8] =
    {
        { 0, 0, 0 },
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 1, 1, 0 },
        { 0, 0, 1 },
        { 1, 0, 1 },
        { 0, 1, 1 },
        { 1, 1, 1 }
    };

#define MRDTM_GUI_TO_VIEW_POINT_DENSITY(guiValue) exp((guiValue - 99.81) / -14.32)

bool GetVisibleAreaForView(DPoint3d** areaPt, int& nbPts, const DPoint3d viewBox[], DRange3d& dtmRange, DRange3d& dtmIntersectionRange)
    {
    // Work out which bit of the triangulation is displayed on screen.    
    DRange3d dtmViewRange;
    bool isVisible = false;

    dtmViewRange = dtmRange;

    //The BoxBoxIntersectionRange function needs a box to perform correctly, so add a very 
    //small artificial range for any coordinate whose real range equals zero.    
    if (dtmViewRange.low.x == dtmViewRange.high.x)
        {
        dtmViewRange.low.x -= 0.0000001;
        }

    if (dtmViewRange.low.y == dtmViewRange.high.y)
        {
        dtmViewRange.low.y -= 0.0000001;
        }

    if (dtmViewRange.low.z == dtmViewRange.high.z)
        {
        dtmViewRange.low.z -= 0.0000001;
        }

    //bsiDRange3d_box2Points(&dtmViewRange, dtmBox);

    //BoxBoxIntersectionRange(dtmBox, viewBox, dtmIntersectionRange);
    DRange3d boxRange = DRange3d::From(viewBox, 8);
    dtmIntersectionRange.IntersectionOf(dtmViewRange, boxRange);

    if (dtmIntersectionRange.high.x > dtmIntersectionRange.low.x &&
        dtmIntersectionRange.high.y > dtmIntersectionRange.low.y)
        {

        if (areaPt != 0)
            {
            nbPts = 5;
            *areaPt = new DPoint3d[nbPts];

            DPoint3d lowPt(dtmIntersectionRange.low);
            DPoint3d highPt(dtmIntersectionRange.high);

            (*areaPt)[0].x = lowPt.x;
            (*areaPt)[0].y = lowPt.y;
            (*areaPt)[0].z = lowPt.z;

            (*areaPt)[1].x = (*areaPt)[0].x;
            (*areaPt)[1].y = highPt.y;
            (*areaPt)[1].z = (*areaPt)[0].z;

            (*areaPt)[2].x = highPt.x;
            (*areaPt)[2].y = (*areaPt)[1].y;
            (*areaPt)[2].z = (*areaPt)[0].z;

            (*areaPt)[3].x = (*areaPt)[2].x;
            (*areaPt)[3].y = (*areaPt)[0].y;
            (*areaPt)[3].z = (*areaPt)[0].z;

            (*areaPt)[4].x = (*areaPt)[0].x;
            (*areaPt)[4].y = (*areaPt)[0].y;
            (*areaPt)[4].z = (*areaPt)[0].z;
            }

        isVisible = true;
        }

    return isVisible;
    }

struct MeshStrokeForCache : public IStrokeForCache
    {
    IMrDTMMeshPtr m_meshPtr;
    unsigned int m_nbPointsDrawn;
    DgnDbR          m_dgnDb;
    virtual void _StrokeForCache(ViewContextR context, double pixelSize = 0.0)
        {
        m_nbPointsDrawn = 0;

        if (context.CheckStop())
            return;

        if (m_meshPtr->GetPolyfaceQuery() != 0)
            {
            // Push the transformation matrix to transform the coordinates to UORS.
            //DrawSentinel    sentinel(context, m_drawingInfo);
            context.GetIDrawGeom().DrawPolyface(*m_meshPtr->GetPolyfaceQuery());

            m_nbPointsDrawn = (unsigned int)m_meshPtr->GetPolyfaceQuery()->GetPointCount();

            assert(m_nbPointsDrawn > 2);
            }
        }

    virtual int32_t _GetQvIndex(void) const override { return 0; }
    virtual QvElemP _GetQvElem(double) const override { return nullptr; }
    virtual void _SaveQvElem(QvElemP, double, double) const override {}
    virtual DgnDbR _GetDgnDb() const override { return m_dgnDb; }

    MeshStrokeForCache(IMrDTMMeshPtr mesh, DgnDbR dgnDb) : m_meshPtr(mesh), m_dgnDb(dgnDb) {}
    };

static QvCache* s_qvCache = 0;
void CreateQvElemForMesh(IMrDTMMeshPtr& mrdtmMeshPtr,
                         PhysicalModel& modelRef,
                         ViewContextR context,
                         __int64 nodeId)
    {
    QvElemP qvCachedElem = 0;

    MeshStrokeForCache trianglesStroker(mrdtmMeshPtr, modelRef.GetDgnDb());

    qvCachedElem = context.CreateCacheElem(trianglesStroker, s_qvCache);

    if (!qvCachedElem)
        {
        assert(context.CheckStop() == true);
        }
    if (qvCachedElem != 0)
        {
        context.GetIViewDraw().DrawQvElem(qvCachedElem);
        QvCachedNodeManager::GetManager().AddCachedNode(nodeId, qvCachedElem, &modelRef, mrdtmMeshPtr->GetPolyfaceQuery()->GetPointCount());
        }
    }

IProgressiveDisplay::Completion ScalableMeshProgressiveDisplay::_Process(ViewContextR context)
    {
    return IProgressiveDisplay::Completion::Finished;
    }

static double s_guiPointDensity = 60.0;

void ScalableMeshProgressiveDisplay::_GetMeshNodes(bvector<IMrDTMNodePtr>& meshNodes, ViewContextR context)
    {
    ScalableMeshDrawingInfoPtr currentDrawingInfoPtr(new ScalableMeshDrawingInfo(&context));
    
    if ((m_previousDrawingInfoForMeshPtr != nullptr) &&
        (m_previousDrawingInfoForMeshPtr->GetDrawPurpose() != DrawPurpose::UpdateDynamic) &&
        (m_meshNodes.size() > 0))
        {
        //If the m_dtmPtr equals 0 it could mean that the last data request to the STM was cancelled, so start a new request even
        //if the view has not changed.
        if (m_previousDrawingInfoForMeshPtr->HasAppearanceChanged(currentDrawingInfoPtr) == false)
            {
            meshNodes.insert(meshNodes.begin(), m_meshNodes.begin(), m_meshNodes.end());
            return;
            }        
        }
       
    m_previousDrawingInfoForMeshPtr = currentDrawingInfoPtr;

    //NEEDS_WORK_SM : When camera is on the matrix doesn't behave as expected.
    DMatrix4d localToView(context.GetLocalToView());
    
    DPoint3d viewBox[8];
    Transform    ltf, ftl;
    context.NpcToWorld(viewBox, s_npcViewBox, 8);

    context.GetCurrLocalToWorldTrans(ltf);
    ftl.InverseOf(ltf);
    ftl.Multiply(viewBox, 8);

    //DMatrix4d rootToStorage;


    //Convert the view box in storage.
    //bool inverted = rootToStorage.qrInverseOf(&localToView);

    //BeAssert(inverted != 0);

    //bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray(&rootToStorage, viewBox, viewBox, 8);

    // Need to get the fence info.
    // DTMDrawingInfo drawingInfo;
    //DTMElementDisplayHandler::GetDTMDrawingInfo(drawingInfo, m_DTMDataRef->GetElement(), m_DTMDataRef, context);
    DPoint3d* fencePts = NULL;
    int nFencePts = 0;
    DRange3d range, intersectionRange;
    m_model.m_scMeshPtr->GetRange(range);
    if (!GetVisibleAreaForView(&fencePts, nFencePts, viewBox, range, intersectionRange))
        {
        return;
        }
    
    BeAssert(m_mrDtmViewDependentMeshQueryPtr != 0);

    int status = SUCCESS;

    if (m_mrDtmViewDependentMeshQueryPtr != 0)
        {
        IMrDTMViewDependentMeshQueryParamsPtr viewDependentQueryParams(IMrDTMViewDependentMeshQueryParams::CreateParams());

        //During an animation preview there is only an DrawPurpose::UpdateDynamic draw,
        //with no DrawPurpose::Update draw following.
        if ((context.GetDrawPurpose() == DrawPurpose::UpdateDynamic))
            {
            viewDependentQueryParams->SetMinScreenPixelsPerPoint(MRDTM_GUI_TO_VIEW_POINT_DENSITY(20));
            }
        else
            {
            viewDependentQueryParams->SetMinScreenPixelsPerPoint(MRDTM_GUI_TO_VIEW_POINT_DENSITY(s_guiPointDensity));
            }

        //Inflate the viewbox in 2D so the Z range equals the Z range of the MrDTM.
        if (context.Is3dView() == false)
            {

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



        status = m_mrDtmViewDependentMeshQueryPtr->Query(meshNodes,
                                                         fencePts,
                                                         nFencePts,
                                                         viewDependentQueryParams);
        if (fencePts != NULL) free(fencePts);

        assert(status != SUCCESS || meshNodes.size() > 0);

        m_meshNodes.clear();

        if (meshNodes.size() > 0)
            {
            m_meshNodes.insert(m_meshNodes.begin(), meshNodes.begin(), meshNodes.end());
            }        
        }
    }


void ScalableMeshProgressiveDisplay::DrawView(ViewContextR context)
    {
    if (NULL == context.GetViewport() || NULL == m_model.m_scMeshPtr.get()) return;
    bvector<IMrDTMNodePtr> meshNodes;
    _GetMeshNodes(meshNodes, context);

    if (!s_qvCache)
        {
        s_qvCache = T_HOST.GetGraphicsAdmin()._CreateQvCache();
        }

    bvector<IMrDTMNodePtr> unchachedMeshNodes;

    //NEEDS_WORK_SM : If kept needs clean up
    for (size_t nodeInd = 0; nodeInd < meshNodes.size(); nodeInd++)
        {
        if (context.CheckStop())
            break;

        QvElem* qvElem = QvCachedNodeManager::GetManager().FindQvElem(meshNodes[nodeInd]->GetNodeId(), &m_model);

        if (qvElem != 0)
            {
            context.GetIViewDraw().DrawQvElem(qvElem);
            }
        else   
            {
            unchachedMeshNodes.push_back(meshNodes[nodeInd]);            
            }
        }

    if (unchachedMeshNodes.size() > 0)
        {
        auto meshNodeIter(unchachedMeshNodes.begin());
        auto meshNodeIterEnd(unchachedMeshNodes.end());

        while (meshNodeIter != meshNodeIterEnd)
            {
            IMrDTMMeshPtr mrdtmMeshPtr((*meshNodeIter)->GetMesh(false));

            if (mrdtmMeshPtr != 0)
                {
                CreateQvElemForMesh(mrdtmMeshPtr, m_model, context, (*meshNodeIter)->GetNodeId());
                }

            meshNodeIter++;
            }
        }
    }

ScalableMeshProgressiveDisplay::ScalableMeshProgressiveDisplay(ScalableMeshModel& model, DgnViewportR vp) : m_model(model)
    {
    DgnGCS* gcsP = vp.GetViewController().GetDgnDb().Units().GetDgnGCS();
    if ((model.m_scMeshPtr->GetBaseGCS() == NULL) || (gcsP == NULL) || model.m_scMeshPtr->GetBaseGCS()->IsEquivalent(*(BentleyApi::GeoCoordinates::BaseGCS*)gcsP))
        {

        m_mrDtmViewDependentMeshQueryPtr = model.m_scMeshPtr->GetMeshQueryInterface(MESH_QUERY_VIEW_DEPENDENT);
        }
    else
        {
        // Since we are reprojected, the extent of the element is expressed in the target GCS and may not fit exactly
        // with the original source exent (due to reprojection and the actual extent computing algorithm used)
        // we will thus provide the extent of the element as computed. This extent will be used
        // to limit the clip shapes more preceisely than if such clipping were solely based upon the source data extent.
        DRange3d drange;
        model.m_scMeshPtr->GetRange(drange);

        BentleyApi::GeoCoordinates::BaseGCSPtr destinationGCS((BentleyApi::GeoCoordinates::BaseGCS*)gcsP);

        m_mrDtmViewDependentMeshQueryPtr = model.m_scMeshPtr->GetMeshQueryInterface(MESH_QUERY_VIEW_DEPENDENT,
                                                                                         destinationGCS,
                                                                                         drange);

        // assert(!"No GetMeshQueryInterface with GCS");
        }
    }

END_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE