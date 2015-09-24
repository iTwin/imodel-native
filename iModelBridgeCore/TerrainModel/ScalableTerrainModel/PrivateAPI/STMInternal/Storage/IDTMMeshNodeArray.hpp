//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMMeshNodeArray.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


template <typename PointType>
typename IDTMMeshNodeFacade<PointType>::linked_node_const_iterator IDTMMeshNodeFacade<PointType>::BeginLinkedNode () const
    {
    HPRECONDITION(0 != GetArrayP());
    return linked_node_const_iterator(BeginLinkedNodeIdx(),
                                      GetArrayP()->Begin(),
                                      GetArrayP()->End(),
                                      BeginLinkedNodeIdx(),
                                      BeginLinkedNodeIdx());
    }

template <typename PointType>
typename IDTMMeshNodeFacade<PointType>::linked_node_const_iterator IDTMMeshNodeFacade<PointType>::EndLinkedNode () const
    {
    return BeginLinkedNode() + GetHeader().linkedNodeQty;
    }

template <typename PointType>
typename IDTMMeshNodeFacade<PointType>::linked_node_iterator IDTMMeshNodeFacade<PointType>::BeginLinkedNodeEdit ()
    {
    HPRECONDITION(0 != GetArrayP());
    return linked_node_iterator(BeginLinkedNodeIdx(),
                                EditArrayP()->BeginEdit(),
                                EditArrayP()->EndEdit(),
                                BeginLinkedNodeIdx(),
                                BeginLinkedNodeIdx());
    }

template <typename PointType>
typename IDTMMeshNodeFacade<PointType>::linked_node_iterator IDTMMeshNodeFacade<PointType>::EndLinkedNodeEdit ()
    {
    return BeginLinkedNodeEdit() + GetHeader().linkedNodeQty;
    }

template <typename PointType>
typename IDTMMeshNodeFacade<PointType>::attached_feature_const_iterator IDTMMeshNodeFacade<PointType>::BeginAttatchedFeature () const
    {
    HPRECONDITION(0 != GetArrayP());
    return attached_feature_const_iterator(BeginAttatchedFeatureIdx(),
                                           GetArrayP()->m_Features.Begin(),
                                           GetArrayP()->m_Features.End(),
                                           BeginAttatchedFeatureIdx(),
                                           BeginAttatchedFeatureIdx());
    }

template <typename PointType>
typename IDTMMeshNodeFacade<PointType>::attached_feature_const_iterator IDTMMeshNodeFacade<PointType>::EndAttachedFeature () const
    {
    return BeginAttatchedFeature() + GetHeader().attachedFeatureQty;
    }

template <typename PointType>
typename IDTMMeshNodeFacade<PointType>::sub_elem_idx_const_iterator IDTMMeshNodeFacade<PointType>::BeginLinkedNodeIdx () const
    {
    HPRECONDITION(0 != GetArrayP());
    HPRECONDITION(GetHeaderIter() < GetHeaders().End());
    HPRECONDITION((GetHeader().linkedNodeOffset + GetHeader().linkedNodeQty) <= GetArrayP()->m_LinkedNodeIndexes.GetSize());
    return GetArrayP()->m_LinkedNodeIndexes.Begin() + GetHeader().linkedNodeOffset;
    }

template <typename PointType>
typename IDTMMeshNodeFacade<PointType>::sub_elem_idx_const_iterator IDTMMeshNodeFacade<PointType>::EndLinkedNodeIdx () const
    {
    return BeginAttatchedFeatureIdx() + GetHeader().linkedNodeQty;
    }

template <typename PointType>
typename IDTMMeshNodeFacade<PointType>::sub_elem_idx_const_iterator IDTMMeshNodeFacade<PointType>::BeginAttatchedFeatureIdx () const
    {
    HPRECONDITION(0 != GetArrayP());
    HPRECONDITION(GetHeaderIter() < GetHeaders().End());
    HPRECONDITION((GetHeader().attachedFeatureOffset + GetHeader().attachedFeatureQty) <= GetArrayP()->m_AttachedFeatureIndexes.GetSize());
    return GetArrayP()->m_AttachedFeatureIndexes.Begin() + GetHeader().attachedFeatureOffset;
    }

template <typename PointType>
typename IDTMMeshNodeFacade<PointType>::sub_elem_idx_const_iterator IDTMMeshNodeFacade<PointType>::EndAttatchedFeatureIdx () const
    {
    return BeginAttatchedFeatureIdx() + GetHeader().attachedFeatureQty;
    }



template <typename PointType>
IDTMMeshNodeArray<PointType>::IDTMMeshNodeArray(size_t pi_Capacity,
                                                size_t pi_TotalLinkedNodeIdxCapacity,
                                                size_t pi_TotalAttachedFeatureIdxCapacity,
                                                size_t pi_TotalAttachedHullPtIdxCapacity)
    :   super_class(pi_Capacity),
        m_LinkedNodeIndexes(pi_TotalLinkedNodeIdxCapacity),
        m_AttachedFeatureIndexes(pi_TotalAttachedFeatureIdxCapacity),
        m_AttachedHullPtIndexes(pi_TotalAttachedHullPtIdxCapacity)
    {

    }


template <typename PointType>
void IDTMMeshNodeArray<PointType>::Wrap (FeatureArray& pi_rFeatureArray)
    {
    //m_Features.
    }

template <typename PointType>
template <typename LinkedNodeIdxIter, typename AttachedFeatureIdxIter, typename AttachedHullPtIdxIter>
void IDTMMeshNodeArray<PointType>::Append  (LinkedNodeIdxIter       pi_LinkedNodeIdxBegin,
                                            LinkedNodeIdxIter       pi_LinkedNodeIdxEnd,
                                            AttachedFeatureIdxIter  pi_AttachedFeatureIdxBegin,
                                            AttachedFeatureIdxIter  pi_AttachedFeatureIdxEnd,
                                            AttachedHullPtIdxIter   pi_AttachedHullPtIdxBegin,
                                            AttachedHullPtIdxIter   pi_AttachedHullPtIdxEnd)
    {
    m_LinkedNodeIndexes.Append(pi_LinkedNodeIdxBegin, pi_LinkedNodeIdxEnd);
    m_AttachedFeatureIndexes.Append(pi_AttachedFeatureIdxBegin, pi_AttachedFeatureIdxEnd);
    m_AttachedHullPtIndexes.Append(pi_AttachedHullPtIdxBegin, pi_AttachedHullPtIdxEnd);
    }

template <typename PointType>
template <typename LinkedNodeIdxIter, typename AttachedFeatureIdxIter>
void IDTMMeshNodeArray<PointType>::Append  (LinkedNodeIdxIter       pi_LinkedNodeIdxBegin,
                                            LinkedNodeIdxIter       pi_LinkedNodeIdxEnd,
                                            AttachedFeatureIdxIter  pi_AttachedFeatureIdxBegin,
                                            AttachedFeatureIdxIter  pi_AttachedFeatureIdxEnd)
    {
    m_LinkedNodeIndexes.Append(pi_LinkedNodeIdxBegin, pi_LinkedNodeIdxEnd);
    m_AttachedFeatureIndexes.Append(pi_AttachedFeatureIdxBegin, pi_AttachedFeatureIdxEnd);
    }

template <typename PointType>
template <typename LinkedNodeIdxIter>
void IDTMMeshNodeArray<PointType>::Append  (LinkedNodeIdxIter   pi_LinkedNodeIdxBegin,
                                            LinkedNodeIdxIter   pi_LinkedNodeIdxEnd)
    {
    m_LinkedNodeIndexes.Append(pi_LinkedNodeIdxBegin, pi_LinkedNodeIdxEnd);
    }
