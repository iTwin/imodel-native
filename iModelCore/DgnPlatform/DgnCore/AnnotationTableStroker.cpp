//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/AnnotationTableStroker.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/AnnotationTable.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

static DVec2d s_xVec = DVec2d::From (1.0,  0.0);
static DVec2d s_yVec = DVec2d::From (0.0, -1.0);

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template <typename T_MemberType>
struct TableSegment
{
double                          m_length;
bvector<T_MemberType const*>    m_members;

TableSegment () : m_length (0.0) {}

}; // TableSegment

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template <typename T_MemberType>
struct SegmentBuilder
{
private:
bool                            m_repeatHeaders;
bool                            m_repeatFooters;
double                          m_breakLength;

TableSegment<T_MemberType>      m_headers;
TableSegment<T_MemberType>      m_footers;
TableSegment<T_MemberType>      m_body;

typedef typename  bvector<T_MemberType const*>::const_iterator  MemberIter;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassifyInputs (bvector <T_MemberType> const& inputs)
    {
    for (T_MemberType const& member : inputs)
        {
        TableSegment<T_MemberType>* group = NULL;

        switch (member.GetHeaderFooterType ())
            {
            case TableHeaderFooterType::Body:    group = &m_body;                                   break;

            case TableHeaderFooterType::Title:   group = m_repeatHeaders  ? &m_headers : &m_body;   break;
            case TableHeaderFooterType::Header:  group = m_repeatHeaders  ? &m_headers : &m_body;   break;
            case TableHeaderFooterType::Footer:  group = m_repeatFooters  ? &m_footers : &m_body;   break;
            }

        group->m_length += member.GetLength();
        group->m_members.push_back (&member);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AddBodyMemberToSegment (TableSegment<T_MemberType>& segment, MemberIter& memberIter, bool force)
    {
    /*-------------------------------------------------------------------------
       In order to add a body row (or column) we need to add all the rows
       that are joined to it by merged cells.  The method FindMergedLength
       looks for merged cells in this row and later rows if needed and
       gives back the height of the whole group.  If they fit we add them
       all.  Otherwise they whole group goes into the next segment.
    -------------------------------------------------------------------------*/

    uint32_t    mergedCount  = 0;
    double      mergedLength = 0.0;

    (*memberIter)->FindMergedLength (mergedLength, mergedCount);

    if ( ! force && m_breakLength < segment.m_length + mergedLength)
        return ERROR;   // doesn't fit

    for (uint32_t iMember = 0; iMember < mergedCount; iMember++)
        {
        segment.m_members.push_back (*(memberIter));
        ++memberIter;
        }

    segment.m_length += mergedLength;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void CreateSegments (bvector <TableSegment <T_MemberType>>& segments)
    {
    bool        startNewSegment  = true;
    bool        addedLastSegment = false;

    TableSegment<T_MemberType>    segment;

    MemberIter  memberIter     = m_body.m_members.begin();
    MemberIter  memberIterEnd  = m_body.m_members.end();

    while (memberIter < memberIterEnd)
        {
        if ( ! startNewSegment)
            {
            // Typical body member just add it to the current segment
            if (SUCCESS == (AddBodyMemberToSegment (segment, memberIter, false)))
                continue;

            // We finished the segment add the footers and put it into the output vector
            for (auto footer : m_footers.m_members)
                {
                // footer length is already baked into segment.m_length
                segment.m_members.push_back (footer);
                }

            segments.push_back (segment);

            addedLastSegment = true;

            // this member will be added next time around
            // don't increment the iter
            startNewSegment = true;
            continue;
            }

        /*-------------------------------------------------------------
            Starting a new segment
        -------------------------------------------------------------*/
        addedLastSegment = false;

        segment.m_members.clear();
        segment.m_length = 0.0;

        // Pre-add the header and footer lengths because we know we will include them
        segment.m_length += m_headers.m_length;
        segment.m_length += m_footers.m_length;

        for (auto header : m_headers.m_members)
            segment.m_members.push_back (header);

        // Every segment has at least one body member
        AddBodyMemberToSegment (segment, memberIter, true);

        startNewSegment = false;
        }

    // This is unusual
    if (m_body.m_members.empty())
        {
        // If there are no body members, then the table must consist of just headers and/or footers
        BeAssert ( ! m_headers.m_members.empty() || ! m_footers.m_members.empty());
        BeAssert (segments.empty());

        // Add the headers
        segment.m_length += m_headers.m_length;
        segment.m_length += m_footers.m_length;

        for (auto header : m_headers.m_members)
            segment.m_members.push_back (header);
        }

    if ( ! addedLastSegment)
        {
        for (auto footer : m_footers.m_members)
            segment.m_members.push_back (footer);

        segments.push_back (segment);
        }
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SetBreakingInfo (bool repeatHeaders, bool repeatFooters, double breakLength)
    {
    m_repeatHeaders = repeatHeaders;
    m_repeatFooters = repeatFooters;
    m_breakLength   = breakLength;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */  SegmentBuilder ()
    {
    m_repeatHeaders = false;
    m_repeatFooters = false;
    m_breakLength   = -1.0; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildSegmentsWithBreaks (bvector <TableSegment <T_MemberType>>& segments, bvector <T_MemberType> const& inputs)
    {
    BeAssert (m_breakLength >= 0.0);

    ClassifyInputs (inputs);
    CreateSegments (segments);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildSegmentNoBreak (TableSegment <T_MemberType>& segment, bvector <T_MemberType> const& inputs)
    {
    for (T_MemberType const& member : inputs)
        {
        segment.m_length += member.GetLength();
        segment.m_members.push_back (&member);
        }
    }

};  // SegmentBuilder

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SubTable::AddRow    (AnnotationTableRowCP row)       { m_rows.push_back (row);    m_size.y += row->GetHeight(); }
void SubTable::AddColumn (AnnotationTableColumnCP col)    { m_columns.push_back (col); m_size.x += col->GetWidth();  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableStroker::LayoutSubTables (SubTablesR subTables) const
    {
    subTables.clear();

    DPoint2d        tableOrigin     = DPoint2d::FromZero();
    double          breakLength     = m_table.GetBreakLength();
    TableBreakType  breakType       = m_table.GetBreakType ();
    bool            repeatHeaders   = m_table.GetRepeatHeaders();
    bool            repeatFooters   = m_table.GetRepeatFooters();

    switch (breakType)
        {
        case TableBreakType::None:
            {
            TableSegment <AnnotationTableRow>   allVisibleRows;
            SegmentBuilder<AnnotationTableRow>  rowBuilder;
            rowBuilder.BuildSegmentNoBreak (allVisibleRows, m_table.GetRowVector());

            TableSegment <AnnotationTableColumn> allVisibleColumns;
            SegmentBuilder<AnnotationTableColumn> colBuilder;
            colBuilder.BuildSegmentNoBreak (allVisibleColumns, m_table.GetColumnVector());

            SubTable        subTable;

            for (AnnotationTableRowCP row : allVisibleRows.m_members)
                subTable.AddRow (row);

            for (AnnotationTableColumnCP column : allVisibleColumns.m_members)
                subTable.AddColumn (column);

            subTables.push_back (subTable);

            break;
            }
        case TableBreakType::Horizontal:
            {
            SegmentBuilder<AnnotationTableRow>    builder;
            builder.SetBreakingInfo (repeatHeaders, repeatFooters, breakLength);

            bvector <TableSegment <AnnotationTableRow>> segments;
            builder.BuildSegmentsWithBreaks (segments, m_table.GetRowVector());

            TableSegment <AnnotationTableColumn>    allVisibleColumns;
            SegmentBuilder<AnnotationTableColumn>   colBuilder;
            colBuilder.BuildSegmentNoBreak (allVisibleColumns, m_table.GetColumnVector());

            for (TableSegment <AnnotationTableRow> rowSegment : segments)
                {
                SubTable    subTable;

                for (AnnotationTableRowCP const& row : rowSegment.m_members)
                    subTable.AddRow (row);

                for (AnnotationTableColumnCP column : allVisibleColumns.m_members)
                    subTable.AddColumn (column);

                subTables.push_back (subTable);
                }

            break;
            }

        case TableBreakType::Vertical:
            {
            TableSegment <AnnotationTableRow> allVisibleRows;
            SegmentBuilder<AnnotationTableRow>    rowBuilder;
            rowBuilder.BuildSegmentNoBreak (allVisibleRows, m_table.GetRowVector());

            SegmentBuilder<AnnotationTableColumn>    builder;
            builder.SetBreakingInfo (repeatHeaders, repeatFooters, breakLength);

            bvector <TableSegment <AnnotationTableColumn>> segments;
            builder.BuildSegmentsWithBreaks (segments, m_table.GetColumnVector());

            for (TableSegment <AnnotationTableColumn> colSegment : segments)
                {
                SubTable    subTable;

                for (AnnotationTableRowCP row : allVisibleRows.m_members)
                    subTable.AddRow (row);

                for (AnnotationTableColumnCP const& column : colSegment.m_members)
                    subTable.AddColumn (column);

                subTables.push_back (subTable);
                }

            break;
            }
        }

    /*-------------------------------------------------------------------------
        Position the subTables
    -------------------------------------------------------------------------*/
    DPoint2d    origin   = tableOrigin;
    double      breakGap = m_table.GetBreakGap();

    switch (m_table.GetBreakPosition())
        {
        default:
        case TableBreakPosition::Right:
            {
            for (SubTable& subTable : subTables)
                {
                subTable.m_origin = origin;
                subTable.m_offset = DVec2d::FromStartEnd (tableOrigin, subTable.m_origin);

                DVec2d  offset = DVec2d::FromScale (s_xVec, subTable.m_size.x + breakGap);

                origin.SumOf (origin, offset);
                }

            break;
            }

        case TableBreakPosition::Left:
            {
            bool        isNotFirst  = false;

            for (SubTable& subTable : subTables)
                {
                if (isNotFirst)
                    {
                    DVec2d  offset = DVec2d::FromScale (s_xVec, -(subTable.m_size.x + breakGap));

                    origin.SumOf (origin, offset);
                    }

                subTable.m_origin = origin;
                subTable.m_offset = DVec2d::FromStartEnd (tableOrigin, subTable.m_origin);

                isNotFirst = true;
                }

            break;
            }

        case TableBreakPosition::Below:
            {
            for (SubTable& subTable : subTables)
                {
                subTable.m_origin = origin;
                subTable.m_offset = DVec2d::FromStartEnd (tableOrigin, subTable.m_origin);

                DVec2d  offset = DVec2d::FromScale (s_yVec, subTable.m_size.y + breakGap);

                origin.SumOf (origin, offset);
                }

            break;
            }

        case TableBreakPosition::Above:
            {
            bool        isNotFirst  = false;

            for (SubTable& subTable : subTables)
                {
                if (isNotFirst)
                    {
                    DVec2d  offset = DVec2d::FromScale (s_yVec, -(subTable.m_size.y + breakGap));

                    origin.SumOf (origin, offset);
                    }

                subTable.m_origin = origin;
                subTable.m_offset = DVec2d::FromStartEnd (tableOrigin, subTable.m_origin);

                isNotFirst = true;
                }

            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
TablePositionedCellIterator::TablePositionedCellIterator (TablePositionedCells const& collection, bool begin)
    :
    m_cellCollection (&collection)
    {
    m_positionedCell.m_cell = NULL;

    if ( ! begin)
        return;

    m_runningRowHeight  = 0.0;

    m_subTableIter    = m_cellCollection->m_subTables->begin();
    m_subTableIterEnd = m_cellCollection->m_subTables->end();

    InitForNewSubTable();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableCellP  TablePositionedCellIterator::CellFromIterators ()
    {
    AnnotationTableElementCR  table = *m_cellCollection->m_table;
    AnnotationTableCellIndex  cellIndex ((*m_rowIter)->GetIndex(), (*m_columnIter)->GetIndex());

    return table.GetCell (cellIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            TablePositionedCellIterator::InitForNewSubTable ()
    {
    SubTable const&  subTable = *m_subTableIter;

    m_rowIter                   = subTable.m_rows.begin();
    m_rowIterEnd                = subTable.m_rows.end();

    m_columnIter                = subTable.m_columns.begin();
    m_columnIterEnd             = subTable.m_columns.end();

    m_positionedCell.m_origin   = subTable.m_origin;
    m_positionedCell.m_cell     = CellFromIterators();

    BeAssert (NULL != m_positionedCell.m_cell);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            TablePositionedCellIterator::MoveToNext ()
    {
    while (true)
        {
        if (m_columnIter != m_columnIterEnd)
            {
            // Move the origin by the width of the previous column
            AnnotationTableColumnCP column = *m_columnIter;
            double                  width  = column->GetWidth();

            m_positionedCell.m_origin.SumOf (m_positionedCell.m_origin, s_xVec, width);

            // Move to the next column
            ++m_columnIter;

            if (m_columnIter != m_columnIterEnd)
                {
                // If we are at a real cell return it
                if (NULL != (m_positionedCell.m_cell = CellFromIterators()))
                    return;

                // current position is a merged cell interior, keep going
                continue;
                }
            }

        if (m_rowIter != m_rowIterEnd)
            {
            SubTable const&  subTable = *m_subTableIter;

            // Increment the runningRowHeight by the height of the previous row
            AnnotationTableRowCP  row = *m_rowIter;
            m_runningRowHeight += row->GetHeight();

            // Compute the row's origin based on the current subTable
            DPoint2d subTableOrigin = subTable.m_origin;
            m_positionedCell.m_origin.SumOf (subTableOrigin, s_yVec, m_runningRowHeight);

            // Move to the next row
            ++m_rowIter;

            if (m_rowIter != m_rowIterEnd)
                {
                // Start over at the first column
                m_columnIter = subTable.m_columns.begin();

                // If we are at a real cell return it
                if (NULL != (m_positionedCell.m_cell = CellFromIterators()))
                    return;

                // current position is a merged cell interior, keep going
                continue;
                }
            }

        // Move to the next SubTable
        ++m_subTableIter;
        m_runningRowHeight = 0;

        if (m_subTableIter != m_subTableIterEnd)
            {
            InitForNewSubTable();
            return;
            }

        // We have reached the end
        m_positionedCell.m_cell = NULL;
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool  TablePositionedCellIterator::IsDifferent (TablePositionedCellIterator const& rhs) const
    {
    if (m_cellCollection != rhs.m_cellCollection)
        return true;

    return (m_positionedCell.m_cell != rhs.m_positionedCell.m_cell);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
TablePositionedCell const& TablePositionedCellIterator::GetCurrent () const
    {
    return m_positionedCell;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */ TablePositionedCells::TablePositionedCells (AnnotationTableElementCR table, SubTablesCP subTables, bool ownSubTables)
    {
    m_table        = const_cast <AnnotationTableElementP> (&table);
    m_subTables    = subTables;
    m_ownSubTables = ownSubTables;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
/* dtor */ TablePositionedCells::~TablePositionedCells ()
    {
    if (NULL != m_subTables && m_ownSubTables)
        delete (m_subTables);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
TablePositionedCells::const_iterator  TablePositionedCells::begin() const { return new TablePositionedCellIterator (*this, true);  }
TablePositionedCells::const_iterator  TablePositionedCells::end() const   { return new TablePositionedCellIterator (*this, false); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
TableFillBoxIterator::TableFillBoxIterator (TableFillBoxes const& collection, bool begin)
    :
    m_fillCollection (&collection)
    {
    m_atEnd = true;

    if ( ! begin)
        return;

    m_atEnd = false;

    m_subTableIter    = m_fillCollection->m_subTables.begin();
    m_subTableIterEnd = m_fillCollection->m_subTables.end();

    InitForNewSubTable();
    MoveToNext();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            TableFillBoxIterator::InitForNewSubTable ()
    {
    SubTable const&  subTable = *m_subTableIter;

    m_rowIter           = subTable.m_rows.begin();
    m_rowIterEnd        = subTable.m_rows.end();
    m_rowOrigin         = subTable.m_origin;

    InitForNewRow();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            TableFillBoxIterator::InitForNewRow ()
    {
    m_columnIter        = (*m_subTableIter).m_columns.begin();
    m_columnIterEnd     = (*m_subTableIter).m_columns.end();

    m_fillBox.m_origin  = m_rowOrigin;
    m_fillBox.m_width   = 0.0;
    m_fillBox.m_rowIndex= (*m_rowIter)->GetIndex();

    FillRunsCR fillRuns = m_fillCollection->GetFillRuns ((*m_rowIter)->GetIndex());

    m_fillRunIter       = fillRuns.begin();
    m_fillRunIterEnd    = fillRuns.end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   TableFillBoxIterator::MoveToNextRow ()
    {
    DVec2d  offset = DVec2d::FromScale (s_yVec, (*m_rowIter)->GetHeight());
    m_rowOrigin.SumOf (m_rowOrigin, offset);

    if (++m_rowIter == m_rowIterEnd)
        return ERROR;

    InitForNewRow();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            TableFillBoxIterator::GetHeightFromFillRun ()
    {
    AnnotationTableElementCR table = m_fillCollection->GetTable();

    m_fillBox.m_height = 0.0;

    uint32_t      hostIndex       = (*m_fillRunIter).GetHostIndex();
    uint32_t      verticalSpan    = (*m_fillRunIter).GetVerticalSpan();

    // Merged cells are always contiguous even with breaks and repeated headers
    // so we don't need to ask the subTable for the order of the next rows.
    for (uint32_t iRow = 0; iRow < verticalSpan; iRow++)
        {
        uint32_t                rowIndex = hostIndex + iRow;
        AnnotationTableRowCP    row = table.GetRow (rowIndex);

        m_fillBox.m_height += row->GetHeight();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            TableFillBoxIterator::MoveToNext ()
    {
    if (m_atEnd)
        return;

    while (true)
        {
        if (m_fillRunIter != m_fillRunIterEnd && m_rowIter != m_rowIterEnd)
            {
            bool reachedEndOfRow = false;

            // Move the origin by the width of the old fill
            m_fillBox.m_origin.SumOf (m_fillBox.m_origin, s_xVec, m_fillBox.m_width);

            // Advance along the columns until we hit the beginning of a fill run
            while ((*m_columnIter)->GetIndex() < (*m_fillRunIter).GetStartIndex())
                {
                m_fillBox.m_origin.SumOf (m_fillBox.m_origin, s_xVec, (*m_columnIter)->GetWidth());

                if (++m_columnIter == m_columnIterEnd)
                    {
                    // reached the end of the row before we hit a run.
                    reachedEndOfRow = true;
                    break;
                    }
                }

            if (reachedEndOfRow)
                continue;

            // We are going to make a fill based on this fill run
            m_fillBox.m_fillKey    = (*m_fillRunIter).GetFillKey();
            m_fillBox.m_width      = 0.0;

            // Advance the width along the columns until we either get the end of
            // the fill run or the end of the row
            while ((*m_columnIter)->GetIndex() < (*m_fillRunIter).GetEndIndex())
                {
                m_fillBox.m_width += (*m_columnIter)->GetWidth();

                if (++m_columnIter == m_columnIterEnd)
                    {
                    // Got to the end of the span before the end of the run.
                    // This will happen due to a break.  No problem we are done.
                    break;
                    }
                }

            GetHeightFromFillRun();

            // Move to the next run on the same row
            ++m_fillRunIter;

            // We've built a good fill time to stop.
            return;
            }

        // Move to the next row on the same subtable
        if (SUCCESS == MoveToNextRow ())
            continue;

        // Move to the next SubTable
        ++m_subTableIter;

        if (m_subTableIter != m_subTableIterEnd)
            {
            InitForNewSubTable();
            continue;
            }

        // We have reached the end
        m_atEnd = true;
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool  TableFillBoxIterator::IsDifferent (TableFillBoxIterator const& rhs) const
    {
    if (m_atEnd && rhs.m_atEnd)
        return false;

    if (m_atEnd != rhs.m_atEnd)
        return true;

    if (m_fillCollection != rhs.m_fillCollection)
        return true;

    if (m_fillRunIter != rhs.m_fillRunIter)
        return true;

    if (m_subTableIter != rhs.m_subTableIter)
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
TableFillBox const& TableFillBoxIterator::GetCurrent () const
    {
    return m_fillBox;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */ TableFillBoxes::TableFillBoxes (AnnotationTableStroker const& stroker, SubTablesCR subTables)
    :
    m_stroker (stroker),
    m_subTables (subTables)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
TableFillBoxes::const_iterator  TableFillBoxes::begin() const { return new TableFillBoxIterator (*this, true);  }
TableFillBoxes::const_iterator  TableFillBoxes::end() const   { return new TableFillBoxIterator (*this, false); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableElementCR        TableFillBoxes::GetTable() const { return m_stroker.GetTable(); }
FillRunsCR                      TableFillBoxes::GetFillRuns (uint32_t rowIndex) const { return m_stroker.GetFillRuns (rowIndex); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
TableEdgeStrokeIterator::TableEdgeStrokeIterator (TableEdgeStrokes const& collection, bool begin)
    :
    m_strokeCollection (&collection)
    {
    m_atEnd = true;

    if ( ! begin)
        return;

    m_atEnd = false;

    m_subTableIter    = m_strokeCollection->m_subTables.begin();
    m_subTableIterEnd = m_strokeCollection->m_subTables.end();

    InitForNewSubTable();
    MoveToNext();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
DVec2dCP        TableEdgeStrokeIterator::GetComponentDirection (bool span) const
    {
    // For horizontal edges, the spans are columns, the hosts are rows
    if (m_strokeCollection->m_horizontal)
        return span ? &s_xVec : &s_yVec;

    // For vertical lines, the spans are rows, the hosts are columns
    return span ? &s_yVec : &s_xVec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
double          TableEdgeStrokeIterator::GetComponentLength (uint32_t index, bool span) const
    {
    AnnotationTableElementR  table = *(m_strokeCollection->m_table);

    // For horizontal edges, the spans are columns, the hosts are rows
    if (m_strokeCollection->m_horizontal)
        return span ? table.GetColumn (index)->GetWidth() : table.GetRow (index)->GetHeight();

    // For vertical lines, the spans are rows, the hosts are columns
    return span ? table.GetRow (index)->GetHeight(): table.GetColumn (index)->GetWidth();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
DVec2d          TableEdgeStrokeIterator::GetHostVector (uint32_t index, bool span) const
    {
    DVec2dCP    dir = GetComponentDirection (span);
    double      len = GetComponentLength (index, span);

    return DVec2d::FromScale (*dir, len);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
DVec2d          TableEdgeStrokeIterator::GetHostLengthVector (uint32_t index) const { return GetHostVector (index, false); }
DVec2d          TableEdgeStrokeIterator::GetHostSpanVector   (uint32_t index) const { return GetHostVector (index, true); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            TableEdgeStrokeIterator::InitRunIterFromStart()
    {
    if (m_strokeCollection->m_horizontal)
        {
        m_runIter    = m_strokeCollection->m_table->GetTopEdgeRuns().begin();
        m_runIterEnd = m_strokeCollection->m_table->GetTopEdgeRuns().end();
        }
    else
        {
        m_runIter    = m_strokeCollection->m_table->GetLeftEdgeRuns().begin();
        m_runIterEnd = m_strokeCollection->m_table->GetLeftEdgeRuns().end();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            TableEdgeStrokeIterator::InitRunIterFromHost(uint32_t index)
    {
    AnnotationTableElementR  table = *(m_strokeCollection->m_table);

    if (m_strokeCollection->m_horizontal)
        {
        EdgeRunsCR edgeRuns = table.GetRow(index)->GetEdgeRuns();

        m_runIter    = edgeRuns.begin();
        m_runIterEnd = edgeRuns.end();
        }
    else
        {
        EdgeRunsCR edgeRuns = table.GetColumn(index)->GetEdgeRuns();

        m_runIter    = edgeRuns.begin();
        m_runIterEnd = edgeRuns.end();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool            TableEdgeStrokeIterator::AtEndOfSpan ()
    {
    if (m_strokeCollection->m_horizontal)
        return m_columnIter == m_columnIterEnd;
    else
        return m_rowIter == m_rowIterEnd;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t          TableEdgeStrokeIterator::GetSpanIndex ()
    {
    if (m_strokeCollection->m_horizontal)
        return (*m_columnIter)->GetIndex();
    else
        return (*m_rowIter)->GetIndex();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool            TableEdgeStrokeIterator::AdvanceAlongSpan (StrokePoint whichPoint)
    {
    DPoint2dR   point = (StrokePoint::Origin == whichPoint) ? m_stroke.m_origin : m_stroke.m_end;
    uint32_t      spanIndex = GetSpanIndex();
    DVec2d      offset = GetHostSpanVector (spanIndex);

    point.SumOf (point, offset);

    if (m_strokeCollection->m_horizontal)
        return ++m_columnIter == m_columnIterEnd;
    else
        return ++m_rowIter == m_rowIterEnd;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   TableEdgeStrokeIterator::IncrementHostIter (uint32_t& newIndex)
    {
    if (m_strokeCollection->m_horizontal)
        {
        // The first edge is *before* the first row.  Don't skip to the second row yet.
        if (! m_firstEdge && ++m_rowIter == m_rowIterEnd)
            return ERROR;

        InitColumnIter();

        newIndex = (*m_rowIter)->GetIndex();
        }
    else
        {
        // The first edge is *before* the first column.  Don't skip to the second column yet.
        if (! m_firstEdge && ++m_columnIter == m_columnIterEnd)
            return ERROR;

        InitRowIter();

        newIndex = (*m_columnIter)->GetIndex();
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   TableEdgeStrokeIterator::MoveToNextHost ()
    {
    uint32_t      newIndex;

    if (SUCCESS != IncrementHostIter (newIndex))
        return ERROR;

    DVec2d offset = GetHostLengthVector (newIndex);

    m_hostOrigin.SumOf (m_hostOrigin, offset);
    m_stroke.m_origin = m_hostOrigin;
    m_stroke.m_end    = m_hostOrigin;

    InitRunIterFromHost (newIndex);
    m_firstEdge = false;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            TableEdgeStrokeIterator::InitRowIter ()
    {
    m_rowIter        = (*m_subTableIter).m_rows.begin();
    m_rowIterEnd     = (*m_subTableIter).m_rows.end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            TableEdgeStrokeIterator::InitColumnIter ()
    {
    m_columnIter        = (*m_subTableIter).m_columns.begin();
    m_columnIterEnd     = (*m_subTableIter).m_columns.end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            TableEdgeStrokeIterator::InitForNewSubTable ()
    {
    InitRowIter();
    InitColumnIter();

    m_firstEdge         = true;

    m_hostOrigin        = (*m_subTableIter).m_origin;
    m_stroke.m_origin   = m_hostOrigin;
    m_stroke.m_end      = m_hostOrigin;

    InitRunIterFromStart();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            TableEdgeStrokeIterator::MoveToNext ()
    {
    if (m_atEnd)
        return;

    while (true)
        {
        if (m_runIter != m_runIterEnd && ! AtEndOfSpan())
            {
            bool reachedEndOfSpan = false;

            m_stroke.m_origin = m_stroke.m_end;

            // Advance along the span until we hit the beginning of the edge run
            while (GetSpanIndex () < (*m_runIter).GetStartIndex())
                {
                if (AdvanceAlongSpan (StrokePoint::Origin))
                    {
                    // reached the end of the span before we hit the run.
                    reachedEndOfSpan = true;
                    break;
                    }
                }

            if (reachedEndOfSpan)
                continue;

            // We are going to make a stroke based on this edge run
            m_stroke.m_symbologyKey = (*m_runIter).GetSymbologyKey();
            m_stroke.m_end          = m_stroke.m_origin;

            // Advance the endpoint along the span until we either get the end of
            // the edge run or the end of the edge
            while (GetSpanIndex () < (*m_runIter).GetEndIndex())
                {
                if (AdvanceAlongSpan (StrokePoint::End))
                    {
                    // Got to the end of the span before the end of the run.
                    // This will happen due to a break.  No problem we are done.
                    break;
                    }
                }

            // Move to the next run on the same host
            ++m_runIter;

            // We've built a good stroke time to stop.
            return;
            }

        // Move to the next host on the same subtable
        if (SUCCESS == MoveToNextHost ())
            continue;

        // Move to the next SubTable
        ++m_subTableIter;

        if (m_subTableIter != m_subTableIterEnd)
            {
            InitForNewSubTable();
            continue;
            }

        // We have reached the end
        m_atEnd = true;
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
TableEdgeStroke const& TableEdgeStrokeIterator::GetCurrent () const
    {
    return m_stroke;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool  TableEdgeStrokeIterator::IsDifferent (TableEdgeStrokeIterator const& rhs) const
    {
    if (m_atEnd && rhs.m_atEnd)
        return false;

    if (m_atEnd != rhs.m_atEnd)
        return true;

    if (m_strokeCollection != rhs.m_strokeCollection)
        return true;

    if (m_runIter != rhs.m_runIter)
        return true;

    if (m_subTableIter != rhs.m_subTableIter)
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */ TableEdgeStrokes::TableEdgeStrokes (AnnotationTableElementCR table, bool horizontal, SubTablesCR subTables)
    :
    m_horizontal (horizontal),
    m_table (const_cast <AnnotationTableElementP> (&table)),
    m_subTables (subTables)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
TableEdgeStrokes::const_iterator  TableEdgeStrokes::begin() const { return new TableEdgeStrokeIterator (*this, true);  }
TableEdgeStrokes::const_iterator  TableEdgeStrokes::end() const   { return new TableEdgeStrokeIterator (*this, false); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */  AnnotationTableStroker::AnnotationTableStroker (AnnotationTableElementCR table, ElementGeometryBuilderR builder)
    :
    m_table (table),
    m_geomBuilder (builder),
    m_addFills (true),
    m_addTextBlocks (true)
    {
    LayoutSubTables (m_subTables);

    m_allFillRuns.reserve (table.GetRowCount());

    for (uint32_t iRow = 0; iRow < table.GetRowCount(); ++iRow)
        {
        m_allFillRuns.push_back (FillRuns());

        AnnotationTableFillRun fillRun;
        fillRun.Initialize (m_table, iRow);
        m_allFillRuns[iRow].push_back (fillRun);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
TableEdgeStrokes   AnnotationTableStroker::ComputeEdgeStrokes (bool horizontal, SubTablesCR subTables) const
    {
    return TableEdgeStrokes (m_table, horizontal, subTables);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
TableFillBoxes   AnnotationTableStroker::ComputeFillBoxes (SubTablesCR subTables) const
    {
    return TableFillBoxes (*this, subTables);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
TablePositionedCells   AnnotationTableStroker::ComputePositionedCells (SubTablesCR subTables) const
    {
    return TablePositionedCells (m_table, &subTables, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableStroker::AppendRectangle (DPoint2dCR origin, double width, double height)
    {
    DPoint2d corner = DPoint2d::From (origin.x + width, origin.y - height);

    ICurvePrimitivePtr  rectangle = ICurvePrimitive::CreateRectangle(origin.x, origin.y, corner.x, corner.y, 0, 1);
    CurveVectorPtr      curveVector = CurveVector::Create (CurveVector::BoundaryType::BOUNDARY_TYPE_Outer, rectangle);
    m_geomBuilder.Append (*curveVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableStroker::AppendFillBox (TableFillBox const& fillBox)
    {
    uint32_t  fillKey = fillBox.m_fillKey;

    if (0 == fillKey)
        fillKey = m_table.GetFillSymbologyForRow (fillBox.m_rowIndex);

    if (0 == fillKey)
        return;

    SymbologyDictionary const&   dictionary  = m_table.GetSymbologyDictionary();
    SymbologyEntryCP             fillSymb    = dictionary.GetSymbology (fillKey);

    if (UNEXPECTED_CONDITION ( ! fillSymb->HasFillColor()))
        return;

    GeometryParams displayParams;
    displayParams.SetCategoryId (m_table.GetCategoryId());
    displayParams.SetFillDisplay (FillDisplay::Blanking);

    displayParams.SetFillColor (fillSymb->GetFillColor());
    m_geomBuilder.Append (displayParams);

    AppendRectangle (fillBox.m_origin, fillBox.m_width, fillBox.m_height);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void   AnnotationTableStroker::AppendEdgeStroke (TableEdgeStroke const& stroke, bool horizontal)
    {
    SymbologyDictionary const&  dictionary  = m_table.GetSymbologyDictionary();
    SymbologyEntryCP            symbology   = dictionary.GetSymbology (stroke.m_symbologyKey);

    if (UNEXPECTED_CONDITION (nullptr == symbology))
        {
        if (nullptr == (symbology = dictionary.GetSymbology (0)))
            return;
        }

    if ( ! symbology->GetVisible())
        return;

    GeometryParams displayParams;
    displayParams.SetCategoryId (m_table.GetCategoryId());
    displayParams.SetFillDisplay (FillDisplay::Never);

    if (symbology->HasColor())
        displayParams.SetLineColor (symbology->GetColor());

    if (symbology->HasWeight())
        displayParams.SetWeight (symbology->GetWeight());

    if (symbology->HasLineStyle())
        {
        LineStyleParams lineStyleParams;
        lineStyleParams.SetScale (symbology->GetLineStyleScale());

        LineStyleInfoPtr lineStyleInfo = LineStyleInfo::Create (symbology->GetLineStyleId(), &lineStyleParams);

        displayParams.SetLineStyle (lineStyleInfo.get());
        }

    m_geomBuilder.Append (displayParams);

    DPoint3d    points[2];
    points[0] = DPoint3d::From (stroke.m_origin);
    points[1] = DPoint3d::From (stroke.m_end);

    ICurvePrimitivePtr curve = ICurvePrimitive::CreateLineString(points, 2);
    m_geomBuilder.Append (*curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableStroker::AppendCell (AnnotationTableCellCR cell, DPoint2dCR cellOrigin)
    {
    cell.AppendContentsGeometry (cellOrigin, s_xVec, s_yVec, m_geomBuilder);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void   AnnotationTableStroker::ApplyCellToFillRuns (AnnotationTableCellCR cell)
    {
    AnnotationTableCellIndex    cellIndex       = cell.GetIndex();
    FillRunsR                   fillRuns        = m_allFillRuns[cellIndex.row];
    uint32_t                    verticalSpan    = cell.GetRowSpan();

    if (cell.HasFillKey() && 0 == cell.GetFillKey())
        {
        fillRuns.CreateGap (nullptr, cellIndex.col, cell.GetColumnSpan());
        }
    else
        {
        uint32_t  runKey = cell.HasFillKey() ? cell.GetFillKey() : 0;

        fillRuns.ApplyRun (runKey, verticalSpan, cellIndex.col, cell.GetColumnSpan());
        fillRuns.MergeRedundantRuns (nullptr);
        }

    for (uint32_t iRow = 1; iRow < verticalSpan; iRow++)
        {
        uint32_t                spannedRow      = cellIndex.row + iRow;
        FillRunsR               spannedFillRuns = m_allFillRuns[spannedRow];

        spannedFillRuns.CreateGap (NULL, cellIndex.col, cell.GetColumnSpan());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void   AnnotationTableStroker::AppendTableGeometry ()
    {
    // Cell Contents
    if (m_addTextBlocks)
        {
        for (TablePositionedCell const& positionedCell : ComputePositionedCells (m_subTables))
            {
            AppendCell (*positionedCell.m_cell, positionedCell.m_origin);

            if (m_addFills)
                ApplyCellToFillRuns (*positionedCell.m_cell);
            }
        }

    // Fills
    if (m_addFills)
        {
        for (TableFillBox const& fillBox : ComputeFillBoxes (m_subTables))
            AppendFillBox (fillBox);
        }

    // Horizontal edges
    for (TableEdgeStroke const& stroke: ComputeEdgeStrokes (true, m_subTables))
        AppendEdgeStroke (stroke, true);

    // Vertical edges
    for (TableEdgeStroke const& stroke: ComputeEdgeStrokes (false, m_subTables))
        AppendEdgeStroke (stroke, false);
    }


END_BENTLEY_DGNPLATFORM_NAMESPACE
