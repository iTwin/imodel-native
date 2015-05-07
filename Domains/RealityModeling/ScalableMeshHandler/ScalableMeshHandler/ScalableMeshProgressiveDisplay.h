/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshHandler/ScalableMeshHandler/ScalableMeshProgressiveDisplay.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include "ScalableMeshModel.h"
#include <DgnPlatform/DgnCore/DgnViewport.h>

BEGIN_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE

class ScalableMeshDrawingInfo;

typedef RefCountedPtr<ScalableMeshDrawingInfo> ScalableMeshDrawingInfoPtr;
class ScalableMeshProgressiveDisplay : public IProgressiveDisplay
    {
    friend struct ScalableMeshModel;
    private:
        ScalableMeshModel& m_model;
        ScalableMeshDrawingInfoPtr m_previousDrawingInfoForMeshPtr;
        IMrDTMMeshQueryPtr m_mrDtmViewDependentMeshQueryPtr;
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
            if (!m_viewInfo.IsValid() != scalableMeshDrawingInfoP->m_viewInfo.IsValid())
                return  false;

            if (m_viewInfo.IsValid())
                return  true;
            const bool areEquivalent = m_viewInfo.GetEyePoint().IsEqual(scalableMeshDrawingInfoP->m_viewInfo.GetEyePoint()) && m_viewInfo.GetLensAngle() == scalableMeshDrawingInfoP->m_viewInfo.GetLensAngle() && m_viewInfo.GetFocusDistance() == scalableMeshDrawingInfoP->m_viewInfo.GetFocusDistance();

            return !areEquivalent ||
                (0 != memcmp(&scalableMeshDrawingInfoP->m_localToViewTransformation, &m_localToViewTransformation, sizeof(DMatrix4d)));
            }
    };
END_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE