/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshHandler/PublicAPI/ScalableMeshModel.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnCore/DgnDb.h>
#include <ScalableMesh/IMrDTM.h>
#include <ScalableMesh/IMrDTMQuery.h>
#include "ScalableMeshProgressiveDisplay.h"

#ifdef __SCM_HANDLERS_BUILD__
#define SCALABLEMESH_HANDLERS_EXPORT EXPORT_ATTRIBUTE
#else
#define SCALABLEMESH_HANDLERS_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace ScalableMesh { namespace Model {
#define END_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE          }} END_BENTLEY_NAMESPACE
#define BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE              BENTLEY_NAMESPACE_NAME::ScalableMesh::Model
#define USING_NAMESPACE_BENTLEY_SCALABLE_MESH_MODEL        using namespace BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE;

USING_NAMESPACE_BENTLEY_MRDTM
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE
struct ScalableMeshModel : PhysicalModel
    {
    DEFINE_T_SUPER(PhysicalModel)

        friend class ScalableMeshProgressiveDisplay;
    private:
        BeFileName      m_fileName;
        IMrDTMPtr       m_scMeshPtr;
        ScalableMeshProgressiveDisplay* m_display;

    protected:
        SCALABLEMESH_HANDLERS_EXPORT virtual AxisAlignedBox3d _QueryModelRange() const override;
        virtual DgnModels::Model::CoordinateSpace _GetCoordinateSpace() const override { return DgnModels::Model::CoordinateSpace::World; }
        SCALABLEMESH_HANDLERS_EXPORT virtual void _AddGraphicsToScene(ViewContextR) override;

    public:
        explicit ScalableMeshModel(DgnModel::CreateParams const& params) : T_Super(params) { m_display = nullptr; }
        virtual ~ScalableMeshModel() { if (m_display != nullptr) delete m_display; }
        BeFileName GetFileName() const { return m_fileName; }
        SCALABLEMESH_HANDLERS_EXPORT void SetFileName(BeFileName const& fileName) { m_fileName = fileName; }
        SCALABLEMESH_HANDLERS_EXPORT StatusInt Load();
    }; 

typedef RefCountedPtr<ScalableMeshModel> ScalableMeshModelPtr;

struct ScalableMeshHandler : dgn_ModelHandler::Model
    {
    MODELHANDLER_DECLARE_MEMBERS("ScalableMesh", ScalableMeshModel, ScalableMeshHandler, dgn_ModelHandler::Model, SCALABLEMESH_HANDLERS_EXPORT)

    //public:
    //    SCALABLEMESH_HANDLERS_EXPORT virtual DgnModelP _CreateInstance(DgnPlatform::DgnModel::CreateParams const& params) override;
    };
END_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE
//__PUBLISH_SECTION_END__
