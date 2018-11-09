//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMScatteredPointsFeatureArray.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


template <typename PointType, typename HeaderType>
void IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::OnArrayDataChanged ()
    {
    if(GetArrayP()->m_PointsAlignedWithHeader)
        EditArrayP()->m_PointsAlignedWithHeader = false;
    }

template <typename PointType, typename HeaderType>
typename IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::feature_type
IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::GetType () const
    {
    HPRECONDITION(0 != GetArrayP());
    return GetHeader().type;
    }


template <typename PointType, typename HeaderType>
typename IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::group_id_type
IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::GetGroupID () const
    {
    HPRECONDITION(0 != GetArrayP());
    return GetHeader().groupId;
    }


template <typename PointType, typename HeaderType>
size_t IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::GetSize () const
    {
    HPRECONDITION(0 != GetArrayP());
    return GetHeader().size;
    }

template <typename PointType, typename HeaderType>
bool IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::operator== (const facade_type& pi_rRight) const
    {
    if (GetType() != pi_rRight.GetType())
        return false;
    if (GetGroupID() != pi_rRight.GetGroupID())
        return false;
    if (GetSize() != pi_rRight.GetSize())
        return false;

    return std::equal(Begin(), End(), pi_rRight.Begin());
    }

template <typename PointType, typename HeaderType>
typename IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::const_iterator IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::Begin () const
    {
    return const_iterator(BeginIdx(),
                          GetPoints().Begin(),
                          GetPoints().End(),
                          BeginIdx(),
                          EndIdx());
    }

template <typename PointType, typename HeaderType>
typename IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::const_iterator IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::End () const
    {
    return Begin() + GetHeader().size;
    }

template <typename PointType, typename HeaderType>
typename IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::iterator IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::BeginEdit ()
    {
    return iterator(BeginIdx(),
                    EditPoints().BeginEdit(),
                    EditPoints().EndEdit(),
                    BeginIdx(),
                    EndIdx());
    }

template <typename PointType, typename HeaderType>
typename IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::iterator IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::EndEdit ()
    {
    return BeginEdit() + GetHeader().size;
    }

template <typename PointType, typename HeaderType>
typename IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::point_idx_const_iterator
IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::BeginIdx () const
    {
    HPRECONDITION(0 != GetArrayP());
    HPRECONDITION(GetHeaderIter() < GetHeaders().End());
    HPRECONDITION((GetHeader().offset + GetHeader().size) <= GetPointIndexes().GetSize());
    return GetPointIndexes().Begin() + GetHeader().offset;
    }

template <typename PointType, typename HeaderType>
typename IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::point_idx_const_iterator
IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::EndIdx () const
    {
    return BeginIdx() + GetHeader().size;
    }

template <typename PointType, typename HeaderType>
typename IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::point_idx_iterator
IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::BeginIdxEdit ()
    {
    HPRECONDITION(0 != GetArrayP());
    HPRECONDITION(GetHeaderIter() < GetHeaders().End());
    HPRECONDITION((GetHeader().offset + GetHeader().size) <= GetPointIndexes().GetSize());
    return EditPointIndexes().BeginEdit() + GetHeader().offset;
    }

template <typename PointType, typename HeaderType>
typename IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::point_idx_iterator
IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::EndIdxEdit ()
    {
    return EndIdxEdit() + GetHeader().size;
    }


template <typename PointType, typename HeaderType>
bool IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::IsPartOfSameList (const facade_type& pi_rRight) const
    {
    return GetArrayP()->Contains(pi_rRight);
    }


/*
template <typename PointType, typename HeaderType> template <typename InputIter>
void IDTMTriangulatedFeatureFacade<PointType, HeaderType>::Insert  (const_iterator  pi_Position,
                                                        InputIter       pi_Begin,
                                                        InputIter       pi_End)
    {
    HPRECONDITION(pi_Begin <= pi_End);
    const size_t AddedPointQty = distance(pi_Begin, pi_End);

    EditArrayP()->IncrementHeaderPointOffsets(EditHeaderIter() + 1, AddedPointQty);

    std::generate_n(EditPointIndexes().Insert(pi_Position.m_PointIndexIter), AddedPointQty, LinearGenerator(GetHeader().size));
    EditPoints().Append(pi_Begin, pi_End);

    EditHeader().size += static_cast<typename header_type::size_type>(AddedPointQty);
    }

template <typename PointType, typename HeaderType> template <typename InputIter>
void IDTMTriangulatedFeatureFacade<PointType, HeaderType>::Append  (InputIter   pi_Begin,
                                                        InputIter   pi_End)
    {
    if (GetHeaders().End() == (GetHeaderIter() + 1))
        {
        const size_t AddedPointQty = distance(pi_Begin, pi_End);
        std::generate_n(EditPointIndexes().Append(AddedPointQty), AddedPointQty, LinearGenerator(GetHeader().size));
        EditPoints().Append(pi_Begin, pi_End);

        EditHeader().size += static_cast<typename header_type::size_type>(AddedPointQty);
        return;
        }

    Insert(End(), pi_Begin, pi_End);
    }
*/

template <typename PointType, typename HeaderType>
const HPU::Array<typename IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::point_idx_type>&
IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::GetPointIndexes () const
    {
    HPRECONDITION(0 != GetArrayP());
    return GetArrayP()->m_PointIndexes;
    }

template <typename PointType, typename HeaderType>
HPU::Array<typename IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::point_idx_type>&
IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::EditPointIndexes ()
    {
    HPRECONDITION(0 != GetArrayP());
    return EditArrayP()->m_PointIndexes;
    }

template <typename PointType, typename HeaderType>
const HPU::Array<typename IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::value_type>&
IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::GetPoints () const
    {
    HPRECONDITION(0 != GetArrayP());
    return GetArrayP()->m_Points;
    }

template <typename PointType, typename HeaderType>
HPU::Array<typename IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::value_type>&
IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::EditPoints ()
    {
    HPRECONDITION(0 != GetArrayP());
    return EditArrayP()->m_Points;
    }


template <typename PointType, typename HeaderType>
void IDTMScatteredPointsFeatureFacade<PointType, HeaderType>::InitHeader   (header_type&        pio_rHeader,
                                                                            feature_type        pi_Type,
                                                                            size_t              pi_Offset,
                                                                            size_t              pi_Size,
                                                                            group_id_type       pi_GroupID)
    {
    pio_rHeader.type = pi_Type;
    pio_rHeader.offset = static_cast<typename header_type::index_type>(pi_Offset);
    pio_rHeader.size = static_cast<typename header_type::size_type>(pi_Size);
    pio_rHeader.groupId = pi_GroupID;
    }

template <typename PointType, typename HeaderType>
IDTMScatteredPointsFeatureArray<PointType, HeaderType>::IDTMScatteredPointsFeatureArray(size_t  pi_Capacity,
        size_t  pi_TotalPointIdxCapacity,
        size_t  pi_TotalPointCapacity)
    :   super_class(pi_Capacity),
        m_PointIndexes(pi_TotalPointIdxCapacity),
        m_Points(pi_TotalPointCapacity),
        m_PointsAlignedWithHeader(true)
    {
    }

template <typename PointType, typename HeaderType>
IDTMScatteredPointsFeatureArray<PointType, HeaderType>::IDTMScatteredPointsFeatureArray(size_t      pi_Capacity,
        size_t      pi_TotalPointIdxCapacity,
        PointType*  pi_pPointBuffer,
        size_t      pi_PointBufferSize,
        size_t      pi_PointBufferCapacity)
    :   super_class(pi_Capacity),
        m_PointIndexes(pi_TotalPointIdxCapacity),
        m_PointsAlignedWithHeader(true)
    {
    m_Points.WrapEditable(pi_pPointBuffer, pi_PointBufferSize, pi_PointBufferCapacity);
    }

template <typename PointType, typename HeaderType>
void IDTMScatteredPointsFeatureArray<PointType, HeaderType>::Wrap (const IDTMScatteredPointsFeatureArray& pi_Right)
    {
    m_PointIndexes.Wrap(pi_Right.m_PointIndexes);
    m_Points.Wrap(pi_Right.m_Points);
    m_PointsAlignedWithHeader = pi_Right.m_PointsAlignedWithHeader;
    }

template <typename PointType, typename HeaderType>
void IDTMScatteredPointsFeatureArray<PointType, HeaderType>::Wrap  (PointType*  pi_pPointBuffer,
                                                                    size_t      pi_PointBufferSize,
                                                                    size_t      pi_PointBufferCapacity)
    {
    // Ensure that this action won't invalidate existing indexes to points.
    HPRECONDITION(0 == m_PointIndexes.GetSize());

    m_Points.WrapEditable(pi_pPointBuffer, pi_PointBufferSize, pi_PointBufferCapacity);
    }


/*

template <typename PointType, typename HeaderType>
void IDTMTriangulatedFeatureFacade<PointType, HeaderType>::Erase ()
    {
    static const value_type REMOVED_POINT_VALUE;

    // TDORAY: This could be done at static time...
    memset(REMOVED_POINT_VALUE, 0xFF, size_of(REMOVED_POINT_VALUE));

    replace_if(EditPoints().BeginEdit(), EditPoints().EndEdit(), IsPartOfThisFeature(*this), REMOVED_POINT_VALUE);


    const size_t PtIndexDecrementCount = count_if(GetPoints().Begin(), GetPoints().End(),
                                                  std::bind1st(std::equal_to<value_type>(), REMOVED_POINT_VALUE));

    typedef reverse_iterator<point_array_type::const_iterator> ReversePtIter;

    ReversePtIter PtFoundRevIt(GetPoints().End());
    ReversePtIter PtEndRevIt(GetPoints().Begin());

    std::pair<size_t, size_t> PtIndexRange(GetPoints().GetSize(), GetPoints().GetSize());

    do
        {
        PtFoundRevIt = find_if(PtFoundRevIt, PtEndRevIt, std::bind1st(std::equal_to<value_type>(), REMOVED_POINT_VALUE));
        PtIndexRange.first = distance (PtFoundRevIt, PtEndRevIt);
        DecrementPointIndexes(PtIndexDecrementCount, PtIndexRange);
        PtIndexRange.second = PtIndexRange.first;

        } while (PtFoundRevIt != PtEndRevIt);

    EditPoints().Erase(remove_if(EditPoints().BeginEdit(),
                                 EditPoints().EndEdit(),
                                 std::bind1st(std::equal_to<value_type>(), REMOVED_POINT_VALUE)),
                       EditPoints().End());

    EditPointIndexes()->Erase(Begin(), End());
    DecrementPointOffsets(EditHeaderIter() + 1, GetSize());
    }
*/

template <typename PointType, typename HeaderType>
size_t IDTMScatteredPointsFeatureArray<PointType, HeaderType>::GetTotalPointIdxQty () const
    {
    return m_PointIndexes.GetSize();
    }

template <typename PointType, typename HeaderType>
size_t IDTMScatteredPointsFeatureArray<PointType, HeaderType>::GetTotalPointIdxCapacity () const
    {
    return m_PointIndexes.GetCapacity();
    }


template <typename PointType, typename HeaderType>
size_t IDTMScatteredPointsFeatureArray<PointType, HeaderType>::GetTotalPointQty () const
    {
    return m_Points.GetSize();
    }

template <typename PointType, typename HeaderType>
size_t IDTMScatteredPointsFeatureArray<PointType, HeaderType>::GetTotalPointCapacity () const
    {
    return m_Points.GetCapacity();
    }

template <typename PointType, typename HeaderType>
void IDTMScatteredPointsFeatureArray<PointType, HeaderType>::Reserve   (size_t  pi_Capacity,
                                                                        size_t  pi_TotalPointIdxCapacity,
                                                                        size_t  pi_TotalPointCapacity)
    {
    super_class::Reserve(pi_Capacity);
    m_PointIndexes.Reserve(pi_TotalPointIdxCapacity);

    // TDORAY: Do I reserve for these???
    m_Points.Reserve(pi_TotalPointCapacity);
    }



template <typename PointType, typename HeaderType>
void IDTMScatteredPointsFeatureArray<PointType, HeaderType>::Append (const_reference pi_rFeature)
    {
    Append(pi_rFeature.GetType(), pi_rFeature.Begin(), pi_rFeature.End(), pi_rFeature.GetGroupID());
    }

template <typename PointType, typename HeaderType>
typename IDTMScatteredPointsFeatureArray<PointType, HeaderType>::iterator
IDTMScatteredPointsFeatureArray<PointType, HeaderType>::Append (feature_type pi_FeatureType,
                                                                group_id_type     pi_GroupId)
    {
    HeaderArray::iterator AddedHeaderIt = EditHeaders().Append();
    facade_type::InitHeader(*AddedHeaderIt, pi_FeatureType, GetPointIndexes().GetSize(), 0, pi_GroupId);

    return CreateIterator(AddedHeaderIt);
    }

template <typename PointType, typename HeaderType> template <typename InputIter>
void IDTMScatteredPointsFeatureArray<PointType, HeaderType>::Append    (feature_type    pi_FeatureType,
                                                                        InputIter       pi_PtBegin,
                                                                        InputIter       pi_PtEnd,
                                                                        group_id_type   pi_GroupId)
    {
    HPRECONDITION(pi_PtBegin <= pi_PtEnd);

    HeaderArray::iterator AddedHeaderIt = EditHeaders().Append();
    facade_type::InitHeader(*AddedHeaderIt, pi_FeatureType, GetPointIndexes().GetSize(), distance(pi_PtBegin, pi_PtEnd), pi_GroupId);

    AppendFeaturePoints(pi_PtBegin, pi_PtEnd, AddedHeaderIt->size);
    }


template <typename PointType, typename HeaderType>
void IDTMScatteredPointsFeatureArray<PointType, HeaderType>:: Append   (const_iterator  pi_Begin,
                                                                        const_iterator  pi_End)
    {
    HPRECONDITION(IsValidIterator(pi_Begin));
    HPRECONDITION(IsValidIterator(pi_End));
    HPRECONDITION(pi_Begin <= pi_End);

    copy(pi_Begin, pi_End, back_inserter(*this));
    }

template <typename PointType, typename HeaderType>
void IDTMScatteredPointsFeatureArray<PointType, HeaderType>::Clear ()
    {
    EditHeaders().Clear();
    EditPointIndexes().Clear();
    EditPoints().Clear();
    }


template <typename PointType, typename HeaderType>
void IDTMScatteredPointsFeatureArray<PointType, HeaderType>::push_back (const_reference pi_rFeature)
    {
    Append(pi_rFeature);
    }


template <typename PointType, typename HeaderType>
void IDTMScatteredPointsFeatureArray<PointType, HeaderType>::AlignPointsWithHeaders ()
    {
    if (m_PointsAlignedWithHeader)
        return;

    // Allocate a new point array that has the same capacity as the previous
    PointIdxArray PointIdxArray(GetPointIndexes().GetCapacity());

    // Allocate required space at the end of the point array and copy feature points in order (so that
    // points are sequentially aligned with their respective headers)
    for_each(Begin(), End(), CopyFeaturePointIndexes(PointIdxArray.Append(GetPointIndexes().GetSize())));

    // Ensure that headers refers to its points
    RecomputePointIdxOffsets();

    m_PointIndexes.Swap(PointIdxArray);
    m_PointsAlignedWithHeader = true; // Now, points are aligned
    }

template <typename PointType, typename HeaderType>
bool IDTMScatteredPointsFeatureArray<PointType, HeaderType>::IsValidIterator (typename const facade_type::const_iterator& pi_Iterator)
    {
    return IsValidIterator(facade_type::GetIdxIterFor(pi_Iterator));
    }

template <typename PointType, typename HeaderType>
bool IDTMScatteredPointsFeatureArray<PointType, HeaderType>::IsValidIterator (typename const facade_type::iterator& pi_Iterator)
    {
    return IsValidIterator(facade_type::GetIdxIterFor(pi_Iterator));
    }

template <typename PointType, typename HeaderType>
bool IDTMScatteredPointsFeatureArray<PointType, HeaderType>::IsValidIterator (typename const facade_type::point_idx_const_iterator& pi_Iterator)
    {
    return GetPointIndexes().IsValidIterator(pi_Iterator);
    }


template <typename FeatureArrayType, typename InputIter>
struct IDTMAppendPointsScatteredPointsFeatureStrategy
    {
private:
    friend FeatureArrayType;
    static void Append (FeatureArrayType&  pi_rArray,
                        InputIter          pi_PtBegin,
                        InputIter          pi_PtEnd,
                        size_t             pi_PtQty)
        {
        std::generate_n(pi_rArray.EditPointIndexes().Append(pi_PtQty),
                        pi_PtQty,
                        typename FeatureArrayType::LinearGenerator(pi_rArray.GetPoints().GetSize()));
        pi_rArray.EditPoints().Append(pi_PtBegin, pi_PtEnd);
        }
    };

template <typename FeatureArrayType>
struct IDTMAppendPointsScatteredPointsFeatureStrategy<FeatureArrayType, typename FeatureArrayType::facade_type::const_iterator>
    {
private:
    friend FeatureArrayType;
    static void Append (FeatureArrayType&                                       pi_rArray,
                        typename FeatureArrayType::facade_type::const_iterator  pi_PtBegin,
                        typename FeatureArrayType::facade_type::const_iterator  pi_PtEnd,
                        size_t                                                  pi_PtQty)
        {
        if (pi_rArray.IsValidIterator(pi_PtBegin))
            {
            pi_rArray.AppendFeaturePoints(pi_rArray.GetPointIdxIterFor(pi_PtBegin), pi_rArray.GetPointIdxIterFor(pi_PtEnd), pi_PtQty);
            }
        else
            {
            std::generate_n(pi_rArray.EditPointIndexes().Append(pi_PtQty),
                            pi_PtQty,
                            typename FeatureArrayType::LinearGenerator(pi_rArray.GetPoints().GetSize()));
            pi_rArray.EditPoints().Append(pi_PtBegin, pi_PtEnd);
            }
        }
    };


template <typename FeatureArrayType>
struct IDTMAppendPointsScatteredPointsFeatureStrategy<FeatureArrayType, typename FeatureArrayType::facade_type::iterator>
    {
private:
    friend FeatureArrayType;
    static void Append (FeatureArrayType&                                   pi_rArray,
                        typename FeatureArrayType::facade_type::iterator    pi_PtBegin,
                        typename FeatureArrayType::facade_type::iterator    pi_PtEnd,
                        size_t                                              pi_PtQty)
        {
        pi_rArray.AppendFeaturePoints<typename facade_type::const_iterator>(pi_PtBegin, pi_PtEnd, pi_PtQty);
        }
    };

template <typename FeatureArrayType>
struct IDTMAppendPointsScatteredPointsFeatureStrategy<FeatureArrayType, typename FeatureArrayType::facade_type::point_idx_const_iterator>
    {
private:
    friend FeatureArrayType;
    static void Append (FeatureArrayType&                                                   pi_rArray,
                        typename FeatureArrayType::facade_type::point_idx_const_iterator    pi_PtBegin,
                        typename FeatureArrayType::facade_type::point_idx_const_iterator    pi_PtEnd,
                        size_t                                                              pi_PtQty)
        {
        HPRECONDITION(pi_rArray.IsValidIterator(pi_PtBegin));
        HPRECONDITION(pi_rArray.IsValidIterator(pi_PtEnd));
        // Those are valid indexes of existing points of this array. Do not add them again.
        pi_rArray.EditPointIndexes().Append(pi_PtBegin, pi_PtEnd);
        }
    };

template <typename PointType, typename HeaderType> template <typename InputIter>
void IDTMScatteredPointsFeatureArray<PointType, HeaderType>::AppendFeaturePoints  (InputIter  pi_PtBegin,
        InputIter  pi_PtEnd,
        size_t     pi_PtQty)
    {
    return IDTMAppendPointsScatteredPointsFeatureStrategy<array_type, InputIter>::Append(*this, pi_PtBegin, pi_PtEnd, pi_PtQty);
    }


template <typename PointType, typename HeaderType>
void IDTMScatteredPointsFeatureArray<PointType, HeaderType>::IncrementHeaderPointOffsets   (header_type*    pi_From,
        size_t          pi_OffsetIncrement)
    {
    //HPRECONDITION(m_PointsAlignedWithHeader);

    HDEBUGCODE(EditHeaders().BeginEdit()); // Trigger a reallocation if needed
    HASSERT(GetHeaders().IsValidIterator(pi_From));

    for_each<header_type*>(pi_From, EditHeaders().EndEdit(), bind2nd(IncrementHeaderOffset(), pi_OffsetIncrement));
    }

template <typename PointType, typename HeaderType>
void IDTMScatteredPointsFeatureArray<PointType, HeaderType>::DecrementHeaderPointOffsets   (header_type*                pi_From,
        size_t                      pi_OffsetDecrement)
    {
    //HPRECONDITION(m_PointsAlignedWithHeader);

    HDEBUGCODE(EditHeaders().BeginEdit()); // Trigger a reallocation if needed
    HASSERT(GetHeaders().IsValidIterator(pi_From));

    for_each<header_type*>(pi_From, EditHeaders().EndEdit(), bind2nd(DecrementHeaderOffset(), pi_OffsetDecrement));
    }

template <typename PointType, typename HeaderType>
void IDTMScatteredPointsFeatureArray<PointType, HeaderType>::DecrementPointIndexes     (size_t                      pi_Decrement,
        const pair<size_t, size_t>& pi_IndexRange)
    {
    for_each(EditPointIndexes().BeginEdit(), EditPointIndexes().EndEdit(), DecrementPtIndex(pi_Decrement, pi_IndexRange));
    }

template <typename PointType, typename HeaderType>
void IDTMScatteredPointsFeatureArray<PointType, HeaderType>::RecomputePointIdxOffsets ()
    {
    for_each(EditHeaders().BeginEdit(), EditHeaders().EndEdit(), RealignHeaderOffsets());
    }

template <typename PointType, typename HeaderType>
bool IDTMScatteredPointsFeatureArray<PointType, HeaderType>::Contains (const facade_type& pi_rRight) const
    {
    return this == pi_rRight.GetArrayP();
    }

template <typename PointType, typename HeaderType>
typename IDTMScatteredPointsFeatureArray<PointType, HeaderType>::facade_type::point_idx_const_iterator
IDTMScatteredPointsFeatureArray<PointType, HeaderType>::GetPointIdxIterFor (typename const facade_type::const_iterator& pi_rIterator) const
    {
    return facade_type::GetIdxIterFor(pi_rIterator);
    }

template <typename PointType, typename HeaderType>
typename IDTMScatteredPointsFeatureArray<PointType, HeaderType>::facade_type::point_idx_const_iterator
IDTMScatteredPointsFeatureArray<PointType, HeaderType>::GetPointIdxIterFor (typename const facade_type::iterator& pi_rIterator) const
    {
    return facade_type::GetIdxIterFor(pi_rIterator);
    }

