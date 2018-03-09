//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/IDTMFeatureArray.hpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

template <typename PointType, typename HeaderType>
void IDTMFeatureFacade<PointType, HeaderType>::OnArrayDataChanged ()
    {
    if(GetArrayP()->m_PointsAlignedWithHeader)
        EditArrayP()->m_PointsAlignedWithHeader = false;
    }


template <typename PointType, typename HeaderType>
typename IDTMFeatureFacade<PointType, HeaderType>::feature_type IDTMFeatureFacade<PointType, HeaderType>::GetType () const
    {
    HPRECONDITION(0 != GetArrayP());
    return GetHeader().type;
    }


template <typename PointType, typename HeaderType>
typename IDTMFeatureFacade<PointType, HeaderType>::group_id_type IDTMFeatureFacade<PointType, HeaderType>::GetGroupID () const
    {
    HPRECONDITION(0 != GetArrayP());
    return GetHeader().groupId;
    }


template <typename PointType, typename HeaderType>
size_t IDTMFeatureFacade<PointType, HeaderType>::GetSize () const
    {
    HPRECONDITION(0 != GetArrayP());
    return GetHeader().size;
    }


template <typename PointType, typename HeaderType>
typename IDTMFeatureFacade<PointType, HeaderType>::const_iterator IDTMFeatureFacade<PointType, HeaderType>::Begin () const
    {
    HPRECONDITION(0 != GetArrayP());
    HPRECONDITION(IsOutOfArray() ? true : (GetHeaderIter() < GetArrayP()->GetHeaders().End()));
    HPRECONDITION((GetHeader().offset + GetHeader().size) <= GetArrayP()->GetPoints().GetSize());
    return GetArrayP()->GetPoints().Begin() + GetHeader().offset;
    }


template <typename PointType, typename HeaderType>
typename IDTMFeatureFacade<PointType, HeaderType>::const_iterator IDTMFeatureFacade<PointType, HeaderType>::End () const
    {
    return Begin() + GetHeader().size;
    }


template <typename PointType, typename HeaderType>
typename IDTMFeatureFacade<PointType, HeaderType>::iterator IDTMFeatureFacade<PointType, HeaderType>::BeginEdit ()
    {
    HPRECONDITION(0 != GetArrayP());
    HPRECONDITION(IsOutOfArray() ? true : (GetHeaderIter() < GetArrayP()->GetHeaders().End()));
    HPRECONDITION((GetHeader().offset + GetHeader().size) <= GetArrayP()->GetPoints().GetSize());
    return EditArrayP()->EditPoints().BeginEdit() + GetHeader().offset;
    }


template <typename PointType, typename HeaderType>
typename IDTMFeatureFacade<PointType, HeaderType>::iterator IDTMFeatureFacade<PointType, HeaderType>::EndEdit ()
    {
    return BeginEdit() + GetHeader().size;
    }

template <typename PointType, typename HeaderType>
bool IDTMFeatureFacade<PointType, HeaderType>::IsPartOfSameList (const facade_type& pi_rRight) const
    {
    HPRECONDITION(GetArrayP()->GetPoints().IsValidIterator(pi_rRight.Begin()));
    return GetArrayP() == pi_rRight.GetArrayP();
    }


template <typename PointType, typename HeaderType>
bool IDTMFeatureFacade<PointType, HeaderType>::operator== (const IDTMFeatureFacade& pi_rRight) const
    {
    if (GetType() != pi_rRight.GetType())
        return false;
    if (GetGroupID() != pi_rRight.GetGroupID())
        return false;
    if (GetSize() != pi_rRight.GetSize())
        return false;

    return equal(Begin(), End(), pi_rRight.Begin());
    }


template <typename PointType, typename HeaderType>
template <typename InputIter>
void IDTMFeatureFacade<PointType, HeaderType>::Insert  (const_iterator  pi_Position,
                                                        InputIter       pi_Begin,
                                                        InputIter       pi_End)
    {
    HPRECONDITION(pi_Begin <= pi_End);
    const size_t AddedPointQty = distance(pi_Begin, pi_End);

    EditArrayP()->IncrementPointOffsets(EditHeaderIter() + 1, AddedPointQty);

    EditHeader().size += static_cast<typename header_type::size_type>(AddedPointQty);
    EditArrayP()->EditPoints().Insert(pi_Position, pi_Begin, pi_End);
    }


template <typename PointType, typename HeaderType>
template <typename InputIter>
void IDTMFeatureFacade<PointType, HeaderType>::Append  (InputIter   pi_Begin,
                                                        InputIter   pi_End)
    {
    Insert(End(), pi_Begin, pi_End);
    }


template <typename PointType, typename HeaderType>
typename IDTMFeatureFacade<PointType, HeaderType>::iterator
IDTMFeatureFacade<PointType, HeaderType>::Erase(const_iterator  pi_Begin,
                                                const_iterator  pi_End)
    {
    HPRECONDITION(pi_Begin <= pi_End);
    const size_t RemovedPointQty = distance(pi_Begin, pi_End);

    EditArrayP()->DecrementPointOffsets(EditHeaderIter() + 1, RemovedPointQty);

    HASSERT(GetHeader().size >= RemovedPointQty);
    EditHeader().size -= RemovedPointQty;

    return EditArrayP()->EditPoints().Erase(pi_Begin, pi_End);
    }

template <typename PointType, typename HeaderType>
void IDTMFeatureFacade<PointType, HeaderType>::InitHeader  (header_type&        pio_rHeader,
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
IDTMFeatureArray<PointType, HeaderType>::IDTMFeatureArray  (size_t  pi_Capacity,
                                                            size_t  pi_TotalPointCapacity)
    :   super_class(pi_Capacity),
        m_Points(pi_TotalPointCapacity),
        m_PointsAlignedWithHeader(true)
    {

    }


template <typename PointType, typename HeaderType>
size_t IDTMFeatureArray<PointType, HeaderType>::GetTotalPointQty  () const
    {
    return m_Points.GetSize();
    }


template <typename PointType, typename HeaderType>
size_t IDTMFeatureArray<PointType, HeaderType>::GetTotalPointCapacity () const
    {
    return m_Points.GetCapacity();
    }


template <typename PointType, typename HeaderType>
void IDTMFeatureArray<PointType, HeaderType>::Reserve  (size_t  pi_Capacity,
                                                        size_t  pi_TotalPointCapacity)
    {
    super_class::Reserve(pi_Capacity);
    m_Points.Reserve(pi_TotalPointCapacity);
    }


template <typename PointType, typename HeaderType>
typename IDTMFeatureArray<PointType, HeaderType>::iterator IDTMFeatureArray<PointType, HeaderType>::Insert (const_iterator      pi_Position,
        const_reference     pi_rFeature)
    {
    HPRECONDITION(IsValidIterator(pi_Position));
    HPRECONDITION(m_PointsAlignedWithHeader);

    HeaderArray::iterator AddedHeaderIt = EditHeaders().Insert(pi_Position->GetHeaderIter());
    facade_type::InitHeader(*AddedHeaderIt,
                            pi_rFeature.GetType(),
                            distance(m_Points.Begin(), pi_Position->Begin()),
                            pi_rFeature.GetSize(),
                            pi_rFeature.GetGroupID());

    copy(pi_rFeature.Begin(), pi_rFeature.End(), m_Points.Insert(pi_Position->Begin(), AddedHeaderIt->size));
    IncrementPointOffsets(pi_Position->EditHeaderIter() + 1, AddedHeaderIt->size);

    return iterator(AddedHeaderIt, this);
    }


template <typename PointType, typename HeaderType>
typename IDTMFeatureArray<PointType, HeaderType>::iterator IDTMFeatureArray<PointType, HeaderType>::Insert (const_iterator  pi_Position,
        feature_type    pi_FeatureType,
        group_id_type   pi_GroupId)
    {
    HPRECONDITION(IsValidIterator(pi_Position));
    HPRECONDITION(m_PointsAlignedWithHeader);

    HeaderArray::iterator AddedHeaderIt = EditHeaders().Insert(pi_Position->GetHeaderIter());
    facade_type::InitHeader(*AddedHeaderIt,
                            pi_FeatureType,
                            distance(m_Points.Begin(), pi_Position->Begin()),
                            0,
                            pi_GroupId);

    return iterator(AddedHeaderIt, this);
    }


template <typename PointType, typename HeaderType>
void IDTMFeatureArray<PointType, HeaderType>::Insert   (const_iterator  pi_Position,
                                                        const_iterator  pi_Begin,
                                                        const_iterator  pi_End)
    {
    HPRECONDITION(IsValidIterator(pi_Position));
    HPRECONDITION(m_PointsAlignedWithHeader);
    HPRECONDITION(pi_Begin <= pi_End);

    size_t          InsertedPointsQty   = 0;
    const_iterator  start = pi_Begin;

    while (start != pi_End)
        {
        InsertedPointsQty += pi_Begin->GetSize();
        start++;
        }    

    PointArray::iterator    InsertedPointIt  = m_Points.Insert(pi_Position->Begin(), InsertedPointsQty);
    HeaderArray::iterator   InsertedHeaderit = EditHeaders().Insert(pi_Position->GetHeaderIter(), distance(pi_Begin, pi_End));
    for (const_iterator FeatureIter = pi_Begin; FeatureIter != pi_End; ++FeatureIter)
        {
        const value_type& Feature = *FeatureIter;
        facade_type::InitHeader(*InsertedHeaderit,
                                Feature.GetType(),
                                distance<PointArray::const_iterator>(m_Points.Begin(), InsertedPointIt),
                                Feature.GetSize(),
                                Feature.GetGroupID());

        ++InsertedHeaderit;

        InsertedPointIt = copy(Feature.Begin(), Feature.End(), InsertedPointIt);
        }

    IncrementPointOffsets(InsertedHeaderit, InsertedPointsQty);
    }


template <typename PointType, typename HeaderType>
void IDTMFeatureArray<PointType, HeaderType>::Append    (const_reference pi_rFeature)
    {
    HeaderArray::iterator AddedHeaderIt = EditHeaders().Append();
    facade_type::InitHeader(*AddedHeaderIt, pi_rFeature.GetType(), m_Points.GetSize(), pi_rFeature.GetSize(), pi_rFeature.GetGroupID());

    copy(pi_rFeature.Begin(), pi_rFeature.End(), m_Points.Append(AddedHeaderIt->size));
    }


template <typename PointType, typename HeaderType>
typename IDTMFeatureArray<PointType, HeaderType>::iterator IDTMFeatureArray<PointType, HeaderType>::Append (feature_type    pi_FeatureType,
        group_id_type   pi_GroupId)
    {
    HeaderArray::iterator AddedHeaderIt = EditHeaders().Append();
    facade_type::InitHeader(*AddedHeaderIt, pi_FeatureType, m_Points.GetSize(), 0, pi_GroupId);

    return CreateIterator(AddedHeaderIt);
    }

template <typename PointType, typename HeaderType> template <typename InputIter>
void IDTMFeatureArray<PointType, HeaderType>::Append   (feature_type    pi_FeatureType,
                                                        InputIter       pi_PtBegin,
                                                        InputIter       pi_PtEnd,
                                                        group_id_type   pi_GroupId)
    {
    HPRECONDITION(pi_PtBegin <= pi_PtEnd);

    HeaderArray::iterator AddedHeaderIt = EditHeaders().Append();
    facade_type::InitHeader(*AddedHeaderIt, pi_FeatureType, m_Points.GetSize(), distance(pi_PtBegin, pi_PtEnd), pi_GroupId);

    copy(pi_PtBegin, pi_PtEnd, m_Points.Append(AddedHeaderIt->size));
    }


template <typename PointType, typename HeaderType>
void IDTMFeatureArray<PointType, HeaderType>::Append   (const_iterator  pi_Begin,
                                                        const_iterator  pi_End)
    {
    HPRECONDITION(pi_Begin <= pi_End);

    copy(pi_Begin, pi_End, back_inserter(*this));
    }


template <typename PointType, typename HeaderType>
void IDTMFeatureArray<PointType, HeaderType>::Clear ()
    {
    EditHeaders().Clear();
    m_Points.Clear();
    }


template <typename PointType, typename HeaderType>
typename IDTMFeatureArray<PointType, HeaderType>::iterator IDTMFeatureArray<PointType, HeaderType>::Erase    (const_iterator pi_Position)
    {
    HPRECONDITION(IsValidIterator(pi_Position));
    HPRECONDITION(m_PointsAlignedWithHeader);

    m_Points.Erase(pi_Position->Begin(), pi_Position->End());
    DecrementPointOffsets(pi_Position->EditHeaderIter() + 1, pi_Position->GetSize());

    return CreateIterator(EditHeaders().Erase(pi_Position->GetHeaderIter()));
    }


template <typename PointType, typename HeaderType>
typename IDTMFeatureArray<PointType, HeaderType>::iterator IDTMFeatureArray<PointType, HeaderType>::Erase  (const_iterator pi_Begin,
        const_iterator pi_End)
    {
    HPRECONDITION(IsValidIterator(pi_Begin));
    HPRECONDITION(IsValidIterator(pi_End));
    HPRECONDITION(m_PointsAlignedWithHeader);
    HPRECONDITION(pi_Begin <= pi_End);

    const const_iterator& LastFeatureIt(pi_End - 1);

    const size_t RemovedPtQty = distance(pi_Begin->Begin(), LastFeatureIt->End());
    DecrementPointOffsets(pi_End->EditHeaderIter(), RemovedPtQty);
    m_Points.Erase(pi_Begin->Begin(), LastFeatureIt->End());

    return CreateIterator(EditHeaders().Erase(pi_Begin->GetHeaderIter(), pi_End->GetHeaderIter()));
    }


template <typename PointType, typename HeaderType>
void IDTMFeatureArray<PointType, HeaderType>::push_back (const_reference pi_rFeature)
    {
    Append(pi_rFeature);
    }

template <typename PointType, typename HeaderType>
typename IDTMFeatureArray<PointType, HeaderType>::iterator IDTMFeatureArray<PointType, HeaderType>::insert (const_iterator  pi_Position,
        const_reference pi_rFeature)
    {
    return Insert(pi_Position, pi_rFeature);
    }

template <typename PointType, typename HeaderType>
void IDTMFeatureArray<PointType, HeaderType>::AlignPointsWithHeaders ()
    {
    if (m_PointsAlignedWithHeader)
        return;

    // Allocate a new point array that has the same capacity as the previous
    PointArray PointsArray(m_Points.GetCapacity());

    // Allocate required space at the end of the point array and copy feature points in order (so that
    // points are sequentially aligned with their respective headers)
    for_each(Begin(), End(), CopyFeaturePoints(PointsArray.Append(m_Points.GetSize())));

    // Ensure that headers refers to its points
    RecomputePointOffsets();

    m_Points.Swap(PointsArray);
    m_PointsAlignedWithHeader = true; // Now, points are aligned
    }

template <typename PointType, typename HeaderType>
void IDTMFeatureArray<PointType, HeaderType>::IncrementPointOffsets(typename HeaderArray::iterator  pi_From,
                                                                    size_t                          pi_OffsetIncrement)
    {
    HPRECONDITION(m_PointsAlignedWithHeader);

    HDEBUGCODE(EditHeaders().BeginEdit()); // Trigger a reallocation if needed
    HASSERT(GetHeaders().IsValidIterator(pi_From));

    for_each<HeaderArray::iterator>(pi_From, EditHeaders().EndEdit(), bind2nd(IncrementHeaderOffset(), pi_OffsetIncrement));
    }


template <typename PointType, typename HeaderType>
void IDTMFeatureArray<PointType, HeaderType>::DecrementPointOffsets(typename HeaderArray::iterator  pi_From,
                                                                    size_t                          pi_OffsetDecrement)
    {
    HPRECONDITION(m_PointsAlignedWithHeader);

    HDEBUGCODE(EditHeaders().BeginEdit()); // Trigger a reallocation if needed
    HASSERT(GetHeaders().IsValidIterator(pi_From));

    for_each<HeaderArray::iterator>(pi_From, EditHeaders().EndEdit(), bind2nd(DecrementHeaderOffset(), pi_OffsetDecrement));
    }


template <typename PointType, typename HeaderType>
void IDTMFeatureArray<PointType, HeaderType>::RecomputePointOffsets ()
    {
    for_each(EditHeaders().BeginEdit(), EditHeaders().EndEdit(), RealignHeaderOffsets());
    }