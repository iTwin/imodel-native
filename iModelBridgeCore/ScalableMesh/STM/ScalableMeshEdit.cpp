#include "ScalableMeshPCH.h"
#include "ScalableMeshEdit.h"
#include "ScalableMeshQuery.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 08/16
//=======================================================================================
ScalableMeshEdit::ScalableMeshEdit(SMMeshIndex<DPoint3d, DRange3d>* smIndex)
: m_smIndex(smIndex)
    {

    }

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 08/16
//=======================================================================================
ScalableMeshEdit::~ScalableMeshEdit()
    {

    }

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 08/16
//=======================================================================================
int ScalableMeshEdit::_RemoveWithin(ClipVectorCP clipPlaneSet, const bvector<IScalableMeshNodePtr>& priorityNodes)
    {
    if (m_smIndex == nullptr) return SMStatus::S_ERROR;

    DRange3d range;
	range.Init();

	for (ClipPrimitivePtr const& primitive : *clipPlaneSet)
	{
		DRange3d        thisRange;

		if (primitive->GetRange(thisRange, nullptr, true))
		{
			if (range.IsEmpty())
				range = thisRange;
			else
				range.IntersectionOf(range, thisRange);
		}
	}

    if (!range.IntersectsWith(m_smIndex->GetContentExtent())) return SMStatus::S_ERROR;
    
    return m_smIndex->RemoveWithin(clipPlaneSet, priorityNodes);
    }


//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 09/18
//=======================================================================================
void ScalableMeshEdit::_Smooth(const DPlane3d& sourceGeometry)
{

}

static double s_smoothness = 1.0; //higher -- more aggressive smoothing. lower-- less smoothing

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 09/18
//=======================================================================================
void ScalableMeshEdit::_SmoothNode(const DPlane3d& sourceGeometry, const bvector<size_t>& targetedIndices, IScalableMeshNodePtr& node)
{
    if (!node->IsHeaderLoaded())
        node->LoadNodeHeader();

    bvector<size_t> changedIndices;
    if (!targetedIndices.empty())
        changedIndices = targetedIndices;
    
    bvector<DPoint3d> newCoordinates;

    if (node->GetPointCount() < 4)
        return;
    IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create(false, true);
    auto meshP = node->GetMesh(flags);
    if (meshP == nullptr)
        return;

    auto geom = GeometryGuide(sourceGeometry);
    dynamic_cast<ScalableMeshMeshWithGraph*>(meshP.get())->SmoothToGeometry(geom, changedIndices, newCoordinates, s_smoothness);

    node->EditNode()->ReplaceIndices(changedIndices, newCoordinates);
}

void ScalableMeshEdit::_SmoothNode(const DPoint3d& center, double radius, const DVec3d& direction, double height, const bvector<size_t>& targetedIndices, IScalableMeshNodePtr& node)
{
    if (!node->IsHeaderLoaded())
        node->LoadNodeHeader();

    bvector<size_t> changedIndices;
    if (!targetedIndices.empty())
        changedIndices = targetedIndices;

    bvector<DPoint3d> newCoordinates;

    if (node->GetPointCount() < 4)
        return;
    IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create(false, true);
    auto meshP = node->GetMesh(flags);
    if (meshP == nullptr)
        return;

    auto geom = GeometryGuide(center,direction, radius, height);
    dynamic_cast<ScalableMeshMeshWithGraph*>(meshP.get())->SmoothToGeometry(geom, changedIndices, newCoordinates, s_smoothness*2);

    node->EditNode()->ReplaceIndices(changedIndices, newCoordinates);
}

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 08/16
//=======================================================================================
int IScalableMeshEdit::RemoveWithin(ClipVectorCP clipPlaneSet) 
    {
    bvector<IScalableMeshNodePtr> emptyNodes;
    return _RemoveWithin(clipPlaneSet, emptyNodes);
    }


//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 08/16
//=======================================================================================
int IScalableMeshEdit::RemoveWithin(ClipVectorCP clipPlaneSet, const bvector<IScalableMeshNodePtr>& priorityNodes)
    {
    return _RemoveWithin(clipPlaneSet, priorityNodes);
    }

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 09/18
//=======================================================================================
void IScalableMeshEdit::Smooth(const DPlane3d& sourceGeometry)
{
    return _Smooth(sourceGeometry);
}

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 09/18
//=======================================================================================
void IScalableMeshEdit::SmoothNode(const DPlane3d& sourceGeometry, IScalableMeshNodePtr& node)
{
    bvector<size_t> targetedIndices;
    return _SmoothNode(sourceGeometry, targetedIndices, node);
}

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 09/18
//=======================================================================================
void IScalableMeshEdit::SmoothNode(const DPlane3d& sourceGeometry, const bvector<size_t>& targetedIndices, IScalableMeshNodePtr& node)
{
    return _SmoothNode(sourceGeometry, targetedIndices, node);
}

void IScalableMeshEdit::SmoothNode(const DPoint3d& center, double radius, const DVec3d& direction, double height, IScalableMeshNodePtr& node)
{
    bvector<size_t> targetedIndices;
    return _SmoothNode(center, radius, direction, height, targetedIndices, node);
}

ScalableMeshEdit* ScalableMeshEdit::Create(SMMeshIndex<DPoint3d, DRange3d>* smIndex)
     { return new ScalableMeshEdit(smIndex); }

END_BENTLEY_SCALABLEMESH_NAMESPACE
