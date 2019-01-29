//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMScatteredPointsFeatureArray.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : IDTMTriangulatedFeatureArray
//-----------------------------------------------------------------------------

#pragma once

#include "HPUCompositeArray.h"


template <typename PointType, typename HeaderType>
class IDTMScatteredPointsFeatureArray;

template <typename FeatureArrayType, typename InputIter>
struct IDTMAppendPointsScatteredPointsFeatureStrategy;


/*---------------------------------------------------------------------------------**//**
* @description  A facade that provides consistent interface to a feature whose points,
*               point indexes and header are separately stored in their respective array.
*               This facade access directly those array without any copying of the data.
* @bsiclass                                                  Raymond.Gauthier   5/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType, typename HeaderType>
class IDTMScatteredPointsFeatureFacade : public HPU::CompositeFacadeBaseWExtCopy<IDTMScatteredPointsFeatureFacade<PointType, HeaderType>,
    HeaderType,
    IDTMScatteredPointsFeatureArray<PointType, HeaderType>>
    {
public:
    typedef typename header_type::index_type
    point_idx_type;
    typedef point_idx_type*                 point_idx_iterator;
    typedef const point_idx_type*           point_idx_const_iterator;

    typedef typename header_type::feature_type
    feature_type;
    typedef typename header_type::group_id_type
    group_id_type;

private:
    typedef HPU::Array<PointType>           point_array_type;
    typedef HPU::Array<point_idx_type>      point_index_array_type;
    typedef typename point_array_type::iterator
    point_iterator;
    typedef typename point_array_type::const_iterator
    point_const_iterator;

public:
    typedef PointType                       value_type;

    typedef BentleyApi::ImagePP::IndexedElementRandomIterator<facade_type, point_iterator, point_idx_const_iterator, value_type>
    iterator;
    typedef BentleyApi::ImagePP::IndexedElementRandomIterator<facade_type, point_const_iterator, point_idx_const_iterator, const value_type>
    const_iterator;

    feature_type                            GetType                            () const;

    group_id_type                           GetGroupID                         () const;

    size_t                                  GetSize                            () const;

    bool                                    operator==                         (const facade_type&      pi_rRight) const;

    const_iterator                          Begin                              () const;
    const_iterator                          End                                () const;

    iterator                                BeginEdit                          ();
    iterator                                EndEdit                            ();

    point_idx_const_iterator                BeginIdx                           () const;
    point_idx_const_iterator                EndIdx                             () const;

    point_idx_iterator                      BeginIdxEdit                       ();
    point_idx_iterator                      EndIdxEdit                         ();

    /*
    template <typename InputIter>
    void                                    Insert                             (const_iterator          pi_Position,
                                                                                InputIter               pi_Begin,
                                                                                InputIter               pi_End);

    template <typename InputIter>
    void                                    Append                             (InputIter               pi_Begin,
                                                                                InputIter               pi_End);
    */

private:
    friend                                  composite_array_type;
    friend                                  array_type;
    friend                                  iterator;
    friend                                  const_iterator;
    friend                                  super_class;

    explicit                                IDTMScatteredPointsFeatureFacade   () {}


    void                                    OnArrayDataChanged                 ();

    static void                             InitHeader                         (header_type&                pio_rHeader,
                                                                                feature_type                pi_Type,
                                                                                size_t                      pi_Offset,
                                                                                size_t                      pi_Size = 0,
                                                                                group_id_type               pi_GroupID = header_type::GetNullID());

    static point_idx_const_iterator         GetIdxIterFor                      (const const_iterator&       pi_rIter) {
        return pi_rIter.GetIndexIterator();
        }
    static point_idx_const_iterator         GetIdxIterFor                      (const iterator&             pi_rIter) {
        return pi_rIter.GetIndexIterator();
        }

    const HPU::Array<point_idx_type>&       GetPointIndexes                    () const;
    HPU::Array<point_idx_type>&             EditPointIndexes                   ();

    const HPU::Array<value_type>&           GetPoints                          () const;
    HPU::Array<value_type>&                 EditPoints                         ();

    bool                                    IsPartOfSameList                   (const facade_type&          pi_rRight) const;

    };


/*---------------------------------------------------------------------------------**//**
* @description  The array provide consistent interface to 3 underlying arrays which are:
*               feature headers array, feature point indexes array and feature points array.
*               It actually refers to virtual features which in fact, are facades to
*               elements of the underlying arrays. This enable us to avoid copying data to
*               another form, keeping locality of reference and providing secure
*               access/edition of feature data. This is a copy on write(COW) array,
*               so that when you wrap on the data of another array, this array only refers
*               to the data of the wrapped array (read only). When this array happens to
*               write the data, all sub arrays are reallocated/copied at another memory
*               location so that this array can edit data without corrupting wrapped array's
*               data. After first edit, array is now said to be owner of the data.
*
* @see HPU::CompositeArrayBase
* @see IDTMFeatureArray for use examples
*
* @bsiclass                                                  Raymond.Gauthier   5/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType, typename HeaderType = FeatureHeader>
class IDTMScatteredPointsFeatureArray : public HPU::CompositeArrayBase<IDTMScatteredPointsFeatureFacade<PointType, HeaderType>,
    HeaderType,
    IDTMScatteredPointsFeatureArray<PointType, HeaderType>>
    {
private:
    typedef typename header_type::feature_type
    feature_type;
    typedef typename header_type::group_id_type
    group_id_type;

public:
    typedef HPU::Array<HeaderType>          HeaderArray;
    typedef HPU::Array<typename header_type::index_type>
    PointIdxArray;
    typedef HPU::Array<PointType>           PointArray;

    typedef typename facade_type::point_idx_type
    point_idx_type;

    typedef typename facade_type::point_array_type
    point_array_type;
    typedef typename facade_type::point_index_array_type
    point_index_array_type;

    explicit                                IDTMScatteredPointsFeatureArray    (size_t                      pi_Capacity = 0,
                                                                                size_t                      pi_TotalPointIdxCapacity = 0,
                                                                                size_t                      pi_TotalPointCapacity = 0);

    explicit                                IDTMScatteredPointsFeatureArray    (size_t                      pi_Capacity,
                                                                                size_t                      pi_TotalPointIdxCapacity,
                                                                                PointType*                  pi_pPointBuffer,
                                                                                size_t                      pi_PointBufferSize,
                                                                                size_t                      pi_PointBufferCapacity = 0);

    void                                    Wrap                               (const IDTMScatteredPointsFeatureArray&
                                                                                pi_Right);

    void                                    Wrap                               (PointType*                  pi_pPointBuffer,
                                                                                size_t                      pi_PointBufferSize,
                                                                                size_t                      pi_PointBufferCapacity = 0);

    size_t                                  GetTotalPointIdxQty                () const;
    size_t                                  GetTotalPointIdxCapacity           () const;

    size_t                                  GetTotalPointQty                   () const;
    size_t                                  GetTotalPointCapacity              () const;


    void                                    Reserve                            (size_t                      pi_Capacity,
                                                                                size_t                      pi_TotalPointIdxCapacity = 0,
                                                                                size_t                      pi_TotalPointCapacity = 0);


    void                                    Append                             (const_reference             pi_rFeature);

    iterator                                Append                             (feature_type                pi_FeatureType,
                                                                                group_id_type               pi_GroupId = header_type::GetNullID());

    template <typename InputIter>
    void                                    Append                             (feature_type                pi_FeatureType,
                                                                                InputIter                   pi_PtBegin,
                                                                                InputIter                   pi_PtEnd,
                                                                                group_id_type               pi_GroupId = header_type::GetNullID());

    void                                    Append                             (const_iterator              pi_Begin,
                                                                                const_iterator              pi_End);

    void                                    Clear                              ();


    /*---------------------------------------------------------------------------------**//**
    * This is an alias for Append. Was created in order to be able to reuse stl vector
    * existing tools such as back_inserter & inserter.
    +---------------+---------------+---------------+---------------+---------------+------*/
    void                                    push_back                          (const_reference             pi_rFeature);

    /*---------------------------------------------------------------------------------**//**
    * If the user decided to manually sort or rearrange directly the underlying header array,
    * he must use this method before doing anything else so that points are aligned again
    * with their respective headers.
    +---------------+---------------+---------------+---------------+---------------+------*/
    void                                    AlignPointsWithHeaders             ();


    bool                                    IsValidIterator                    (typename const facade_type::const_iterator&
                                                                                pi_Iterator);
    bool                                    IsValidIterator                    (typename const facade_type::iterator&
                                                                                pi_Iterator);
    bool                                    IsValidIterator                    (typename const facade_type::point_idx_const_iterator&
                                                                                pi_Iterator);

private:
    friend                                  facade_type;

    template <typename FeatureArrayType, typename InputIter>
    friend struct IDTMAppendPointsScatteredPointsFeatureStrategy;


    struct IncrementHeaderOffset : public binary_function<header_type, size_t, void>
        {
        void        operator()         (header_type& pi_rHeader, size_t pi_OffsetIncrement) const
            {
            pi_rHeader.offset += static_cast<typename header_type::index_type>(pi_OffsetIncrement);
            }
        };

    struct DecrementHeaderOffset : public binary_function<header_type, size_t, void>
        {
        void        operator()         (header_type& pi_rHeader, size_t pi_OffsetDecrement) const
            {
            pi_rHeader.offset -= static_cast<typename header_type::index_type>(pi_OffsetDecrement);
            }
        };

    struct RealignHeaderOffsets : public unary_function<header_type, void>
        {
        explicit    RealignHeaderOffsets   () : m_CurrentOffset(0) {}

        void        operator()             (header_type&     pi_rHeader)
            {
            pi_rHeader.offset = static_cast<typename header_type::index_type>(m_CurrentOffset);
            m_CurrentOffset += pi_rHeader.size;
            }

    private:
        size_t      m_CurrentOffset;
        };


    struct DecrementPtIndex
        {
        explicit DecrementPtIndex(size_t                        pi_Decrement,
                                  const pair<size_t, size_t>&   pi_IndexRange)
            :   m_Decrement(pi_Decrement),
                m_IndexRange(pi_IndexRange) {}

        void operator() (const point_idx_type& pio_rIndex)
            {
            if (m_IndexRange.first <= pio_rIndex && pio_rIndex < m_IndexRange.second)
                {
                HASSERT(pio_rIndex >= m_Decrement);
                pio_rIndex -= m_Decrement;
                }
            }
        size_t                      m_Decrement;
        const pair<size_t, size_t>& m_IndexRange;
        };

    struct LinearGenerator
        {
        explicit LinearGenerator (size_t  pi_StartValue = 0) : m_Value(static_cast<point_idx_type>(pi_StartValue)) {}

        point_idx_type operator() ()
            {
            return m_Value++;
            }

    private:
        point_idx_type m_Value;
        };

    struct IsPartOfThisFeature
        {
        explicit IsPartOfThisFeature(const facade_type& pi_rFeature) : m_rFeature(pi_rFeature) {}

        bool operator () (const value_type& pi_rRight)
            {
            return m_rFeature.End() != find(m_rFeature.Begin(), m_rFeature.End(), pi_rRight);
            }

        const facade_type& m_rFeature;
        };

    struct CopyFeaturePointIndexes : public unary_function<value_type, void>
        {
        explicit    CopyFeaturePointIndexes(typename PointIdxArray::iterator po_OutputPointIdxIt) : m_OutputPointIdxIt(po_OutputPointIdxIt) {}

        void        operator()             (const facade_type&       pi_rFeature)
            {
            m_OutputPointIdxIt = copy(pi_rFeature.BeginIdx(), pi_rFeature.EndIdx(), m_OutputPointIdxIt);
            }
    private:
        typename PointIdxArray::iterator m_OutputPointIdxIt;
        };

    template <typename InputIter>
    void                                    AppendFeaturePoints                (InputIter                   pi_PtBegin,
                                                                                InputIter                   pi_PtEnd,
                                                                                size_t                      pi_PtQty);

    void                                    IncrementHeaderPointOffsets        (header_type*                pi_From,
                                                                                size_t                      pi_OffsetIncrement);

    void                                    DecrementHeaderPointOffsets        (header_type*                pi_From,
                                                                                size_t                      pi_OffsetDecrement);

    void                                    DecrementPointIndexes              (size_t                      pi_Decrement,
                                                                                const std::pair<size_t, size_t>&
                                                                                pi_IndexRange);

    void                                    RecomputePointIdxOffsets           ();

    bool                                    Contains                           (const facade_type&          pi_rRight) const;

    typename facade_type::point_idx_const_iterator
    GetPointIdxIterFor                 (typename const facade_type::const_iterator&
                                        pi_rIterator) const;

    typename facade_type::point_idx_const_iterator
    GetPointIdxIterFor                 (typename const facade_type::iterator&
                                        pi_rIterator) const;


    // Internal methods
    const HeaderArray&                      GetHeaders                         () const {
        return super_class::GetHeaders();
        }
    HeaderArray&                            EditHeaders                        () {
        return super_class::EditHeaders();
        }

    const PointArray&                       GetPoints                          () const {
        return m_Points;
        }
    PointArray&                             EditPoints                         () {
        return m_Points;
        }

    const PointIdxArray&                    GetPointIndexes                    () const {
        return m_PointIndexes;
        }
    PointIdxArray&                          EditPointIndexes                   () {
        return m_PointIndexes;
        }


    point_index_array_type                  m_PointIndexes;
    point_array_type                        m_Points;
    bool                                    m_PointsAlignedWithHeader;
    };


template <typename PointType, typename HeaderType>
inline void                                 swap                               (IDTMScatteredPointsFeatureFacade<PointType, HeaderType>& pi_rLeft,
                                                                                IDTMScatteredPointsFeatureFacade<PointType, HeaderType>& pi_rRight)
    {
    pi_rLeft.Swap(pi_rRight);
    }


#include "IDTMScatteredPointsFeatureArray.hpp"


// TDORAY: Dirty Microsoft fix/hack for their bad use of swap in std algorithms. In VC2010 the problem is fixed so remove.
#include "IDTMFileDirectories/PointTypes.h"
#include "IDTMFileDirectories/FeatureHeaderTypes.h"

namespace std
{

inline void                             swap                               (IDTMScatteredPointsFeatureFacade<IDTMFile::Point3d64f, IDTMFile::FeatureHeader>& pi_rLeft,
                                                                            IDTMScatteredPointsFeatureFacade<IDTMFile::Point3d64f, IDTMFile::FeatureHeader>& pi_rRight)
    {
    pi_rLeft.Swap(pi_rRight);
    }

inline void                             iter_swap                          (IDTMScatteredPointsFeatureArray<IDTMFile::Point3d64f, IDTMFile::FeatureHeader>::iterator pi_rLeft,
                                                                            IDTMScatteredPointsFeatureArray<IDTMFile::Point3d64f, IDTMFile::FeatureHeader>::iterator pi_rRight)
    {
    (*pi_rLeft).Swap(*pi_rRight);
    }
};
