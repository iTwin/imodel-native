//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMMeshNodeArray.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : IDTMMeshNodeArray
//-----------------------------------------------------------------------------


#pragma once


#include "IDTMCompositeArray.h"
#include "IDTMScatteredPointsFeatureArray.h"
#include "IDTMArray.h"
#include "IDTMUtils.h"


/*---------------------------------------------------------------------------------**//**
* @description Native IDTM feature header
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMMeshNodeHeader
    {
    typedef uint32_t                   index_type;
    typedef uint32_t                   size_type;
    index_type                          linkedNodeOffset;
    size_type                           linkedNodeQty;
    index_type                          attachedFeatureOffset;
    size_type                           attachedFeatureQty;
    index_type                          attachedHullPtOffset;
    size_type                           attachedHullPtQty;
    };


template <typename PointType>
class IDTMMeshNodeArray;

template <typename PointType>
class IDTMMeshNodeFacade : public IDTMCompositeFacadeBase<IDTMMeshNodeFacade<PointType>,
    IDTMMeshNodeHeader,
    IDTMMeshNodeArray<PointType>>
    {
private:
    typedef IDTMMeshNodeHeader              header_type;
    typedef IDTMScatteredPointsFeatureArray<PointType>
    feature_array_type;
    typedef typename feature_array_type::value_type
    feature_type;
public:
    typedef typename header_type::index_type
    sub_elem_idx_type;
    typedef const sub_elem_idx_type*        sub_elem_idx_const_iterator;
    typedef sub_elem_idx_type*              sub_elem_idx_iterator;

    typedef BentleyApi::ImagePP::IndexedElementRandomIterator<IDTMMeshNodeFacade,
            typename composite_array_type::const_iterator,
            sub_elem_idx_const_iterator,
            typename const composite_array_type::value_type>
            linked_node_const_iterator;
    typedef BentleyApi::ImagePP::IndexedElementRandomIterator<IDTMMeshNodeFacade,
            typename composite_array_type::iterator,
            sub_elem_idx_const_iterator,
            typename composite_array_type::value_type>
            linked_node_iterator;

    typedef BentleyApi::ImagePP::IndexedElementRandomIterator<IDTMMeshNodeFacade,
            typename feature_array_type::const_iterator,
            sub_elem_idx_const_iterator,
            typename feature_array_type::value_type>
            attached_feature_const_iterator;




    linked_node_const_iterator              BeginLinkedNode                    () const;
    linked_node_const_iterator              EndLinkedNode                      () const;

    linked_node_iterator                    BeginLinkedNodeEdit                ();
    linked_node_iterator                    EndLinkedNodeEdit                  ();

    attached_feature_const_iterator         BeginAttatchedFeature              () const;
    attached_feature_const_iterator         EndAttachedFeature                 () const;

    sub_elem_idx_const_iterator             BeginLinkedNodeIdx                 () const;
    sub_elem_idx_const_iterator             EndLinkedNodeIdx                   () const;

    sub_elem_idx_const_iterator             BeginAttatchedFeatureIdx           () const;
    sub_elem_idx_const_iterator             EndAttatchedFeatureIdx             () const;

    };


template <typename PointType>
class IDTMMeshNodeArray : public IDTMCompositeArrayBase<IDTMMeshNodeFacade<PointType>,
    IDTMMeshNodeHeader,
    IDTMMeshNodeArray<PointType>>
    {
private:
    friend                                  facade_type;

    typedef IDTMMeshNodeHeader              header_type;
    typedef IDTMScatteredPointsFeatureArray<PointType>
    feature_type;

    typedef Array<typename header_type::index_type>
    LinkedNodeIdxArray;
    typedef Array<typename header_type::index_type>
    AttachedFeatureIdxArray;
    typedef Array<typename header_type::index_type>
    AttachedHullPtIdxArray;
    typedef IDTMScatteredPointsFeatureArray<PointType>
    FeatureArray;




    LinkedNodeIdxArray                      m_LinkedNodeIndexes;
    AttachedFeatureIdxArray                 m_AttachedFeatureIndexes;
    FeatureArray                            m_Features;
    AttachedHullPtIdxArray                  m_AttachedHullPtIndexes;

public:
    typedef typename header_type::index_type
    sub_elem_idx_type;
    typedef const sub_elem_idx_type*        sub_elem_idx_const_iterator;
    typedef sub_elem_idx_type*              sub_elem_idx_iterator;

    explicit                                IDTMMeshNodeArray                  (size_t                      pi_Capacity = 0,
                                                                                size_t                      pi_TotalLinkedNodeIdxCapacity = 0,
                                                                                size_t                      pi_TotalAttachedFeatureIdxCapacity = 0,
                                                                                size_t                      pi_TotalAttachedHullPtIdxCapacity = 0);



    void                                    Wrap                               (FeatureArray&               pi_rFeatureArray);


    template <typename LinkedNodeIdxIter, typename AttachedFeatureIdxIter, typename AttachedHullPtIdxIter>
    void                                    Append                             (LinkedNodeIdxIter           pi_LinkedNodeIdxBegin,
                                                                                LinkedNodeIdxIter           pi_LinkedNodeIdxEnd,
                                                                                AttachedFeatureIdxIter      pi_AttachedFeatureIdxBegin ,
                                                                                AttachedFeatureIdxIter      pi_AttachedFeatureIdxEnd ,
                                                                                AttachedHullPtIdxIter       pi_AttachedHullPtIdxBegin,
                                                                                AttachedHullPtIdxIter       pi_AttachedHullPtIdxEnd);

    template <typename LinkedNodeIdxIter, typename AttachedFeatureIdxIter>
    void                                    Append                             (LinkedNodeIdxIter           pi_LinkedNodeIdxBegin,
                                                                                LinkedNodeIdxIter           pi_LinkedNodeIdxEnd,
                                                                                AttachedFeatureIdxIter      pi_AttachedFeatureIdxBegin,
                                                                                AttachedFeatureIdxIter      pi_AttachedFeatureIdxEnd);

    template <typename LinkedNodeIdxIter>
    void                                    Append                             (LinkedNodeIdxIter           pi_LinkedNodeIdxBegin,
                                                                                LinkedNodeIdxIter           pi_LinkedNodeIdxEnd);


    };

#include "IDTMMeshNodeArray.hpp"
