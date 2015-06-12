/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshHandler/PublicAPI/ScalableMeshProgressiveDisplay.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <BentleyApi/BentleyApi.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnCore/DgnDb.h>
#include <ScalableMesh/IMrDTM.h>
#include <ScalableMesh/IMrDTMQuery.h>
#include <DgnPlatform/DgnCore/DgnViewport.h>

#define BEGIN_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE        BEGIN_BENTLEY_API_NAMESPACE namespace ScalableMesh { namespace Model {
#define END_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE          }} END_BENTLEY_API_NAMESPACE
#define BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE              BENTLEY_API_NAMESPACE_NAME::ScalableMesh::Model
#define USING_NAMESPACE_BENTLEY_SCALABLE_MESH_MODEL        using namespace BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE;

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_MRDTM

BEGIN_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE

class ScalableMeshDrawingInfo;
struct ScalableMeshModel;

typedef RefCountedPtr<ScalableMeshDrawingInfo> ScalableMeshDrawingInfoPtr;
class ScalableMeshProgressiveDisplay : IProgressiveDisplay
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS

        friend struct ScalableMeshModel;
    private:
        ScalableMeshModel& m_model;
        ScalableMeshDrawingInfoPtr m_previousDrawingInfoForMeshPtr;
        IMrDTMMeshQueryPtr m_mrDtmViewDependentMeshQueryPtr;
        bvector<IMrDTMNodePtr> m_meshNodes;                                       

        void _GetMeshNodes(bvector<IMrDTMNodePtr>& meshNodes, ViewContextR context);
    protected:
        virtual Completion _Process(ViewContextR) override;

        virtual bool _WantTimeoutSet(uint32_t& limit) override { return false; }

    public:
        void DrawView(ViewContextR);

        ScalableMeshProgressiveDisplay(ScalableMeshModel& model, DgnViewportR vp);
    };


class ScalableMeshDrawingInfo : public RefCountedBase
    {
    private:

        DrawPurpose m_drawPurpose;
        bool        m_isHighQualityDisplayOn;
        DMatrix4d   m_localToViewTransformation;
        CameraInfo m_viewInfo;

    public:

        ScalableMeshDrawingInfo(ViewContextP viewContext)
            : m_viewInfo(viewContext->GetViewport()->GetCamera())
            {
            m_drawPurpose = viewContext->GetDrawPurpose();
            const DMatrix4d localToView(viewContext->GetLocalToView());
            memcpy(&m_localToViewTransformation, &localToView, sizeof(DMatrix4d));
            }

        ~ScalableMeshDrawingInfo()
            {}

        DrawPurpose GetDrawPurpose()
            {
            return m_drawPurpose;
            }

        bool HasAppearanceChanged(const ScalableMeshDrawingInfoPtr& scalableMeshDrawingInfoP)
            {
            /*NEEDS_WORK_SM : CameraInfo is very different than ViewInfo in Vancouver.
            if (!m_viewInfo.IsValid() != scalableMeshDrawingInfoP->m_viewInfo.IsValid())
                return  false;

            if (m_viewInfo.IsValid())
                return  true;
                */
            const bool areEquivalent = m_viewInfo.GetEyePoint().IsEqual(scalableMeshDrawingInfoP->m_viewInfo.GetEyePoint()) && m_viewInfo.GetLensAngle() == scalableMeshDrawingInfoP->m_viewInfo.GetLensAngle() && m_viewInfo.GetFocusDistance() == scalableMeshDrawingInfoP->m_viewInfo.GetFocusDistance();

            return !areEquivalent ||
                (0 != memcmp(&scalableMeshDrawingInfoP->m_localToViewTransformation, &m_localToViewTransformation, sizeof(DMatrix4d)));
            }
    };
END_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE