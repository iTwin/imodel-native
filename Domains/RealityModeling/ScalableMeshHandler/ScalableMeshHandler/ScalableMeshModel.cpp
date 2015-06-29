#include <ScalableMeshHandler/ScalableMeshModel.h>


BEGIN_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE

HANDLER_DEFINE_MEMBERS(ScalableMeshHandler)

/*DgnModelP ScalableMeshHandler::_CreateInstance(DgnPlatform::DgnModel::CreateParams const& params)
    {
    return new ScalableMeshModel(params);
    }*/

void ScalableMeshModel::_AddGraphicsToScene(ViewContextR context)
    {
    if (m_display == nullptr) m_display = new ScalableMeshProgressiveDisplay(*this, *context.GetViewport());
    m_display->DrawView(context);
    }

StatusInt ScalableMeshModel::Load()
    {
    m_scMeshPtr = IMrDTM::GetFor(m_fileName.c_str(), true, true);
    return m_scMeshPtr != 0 ? SUCCESS : ERROR;
    }


AxisAlignedBox3d ScalableMeshModel::_QueryModelRange() const
    {
    DRange3d range;
    m_scMeshPtr->GetRange(range);
    return AxisAlignedBox3d(range);
    }

END_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE