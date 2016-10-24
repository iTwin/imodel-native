#include "ScalableMeshPCH.h"
#include "ScalableMeshEdit.h"

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
    clipPlaneSet->GetRange(range, nullptr);
    if (!range.IntersectsWith(m_smIndex->GetContentExtent())) return SMStatus::S_ERROR;
    
    return m_smIndex->RemoveWithin(clipPlaneSet, priorityNodes);
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

ScalableMeshEdit* ScalableMeshEdit::Create(SMMeshIndex<DPoint3d, DRange3d>* smIndex)
     { return new ScalableMeshEdit(smIndex); }

END_BENTLEY_SCALABLEMESH_NAMESPACE
