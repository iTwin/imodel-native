/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/Indentation.cpp $
|
|   $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace std;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- IndentationData ------------------------------------------------------------------------------------------------------------- IndentationData --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/04
//---------------------------------------------------------------------------------------
double              IndentationData::GetFirstLineIndent () const                    { return m_firstLineIndent; }
void                IndentationData::SetFirstLineIndent (double value)              { m_firstLineIndent = value; }
double              IndentationData::GetHangingIndent () const                    { return m_paragraphIndent; }
void                IndentationData::SetHangingIndent (double value)              { m_paragraphIndent = value; }
T_DoubleVectorCR    IndentationData::GetTabStops        () const                    { return m_tabStops; }
void                IndentationData::SetTabStops        (T_DoubleVectorCR value)    { m_tabStops = value; }

IndentationDataPtr  IndentationData::Create             ()                          { return new IndentationData (); }
IndentationDataPtr  IndentationData::Clone              () const                    { return new IndentationData (*this); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/05
//---------------------------------------------------------------------------------------
IndentationData::IndentationData () :
    m_firstLineIndent (0.0),
    m_paragraphIndent (0.0)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/04
//---------------------------------------------------------------------------------------
IndentationData::IndentationData (double firstLineIndent, double paragraphIndent, size_t nTabStops, double const* pTabStops) :
    m_firstLineIndent (firstLineIndent),
    m_paragraphIndent (paragraphIndent)
    {
    for (size_t i = 0; i < nTabStops; ++i)
        m_tabStops.push_back (pTabStops [i]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2009
//---------------------------------------------------------------------------------------
IndentationData::IndentationData (IndentationDataCR rhs) :
    RefCountedBase (),
    m_firstLineIndent   (rhs.m_firstLineIndent),
    m_paragraphIndent   (rhs.m_paragraphIndent),
    m_tabStops          (rhs.m_tabStops)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
void IndentationData::SetTabStops (size_t nTabStops, double const * tabStops)
    {
    m_tabStops.clear ();
    
    for (size_t i = 0; i < nTabStops; ++i)
        m_tabStops.push_back (tabStops[i]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
bool IndentationData::IsDefault () const
    {
    return (0.0 == GetFirstLineIndent () && 0.0 == GetHangingIndent () && 0 == m_tabStops.size ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
double IndentationData::GetNextTabStop (double currentOffset, double textHeight, double lineBreakLength) const
    {
    T_DoubleVector adjustedTabStops = m_tabStops;

    // Based on AutoCAD: for tab processing only (e.g. we don't want this written to the file, which is why we aren't writing it into the raw m_tabStops collection) paragraph-indent is effectively a tab stop. Note that if paragraph-indent < first-line-indent, adding it as a tab stop is harmless because incoming currentOffset will have been adjusted past the tab stop. Related TR's are 105609, 225687, and 250208.

    if (m_paragraphIndent > mgds_fc_epsilon)
        {
        T_DoubleVector::iterator    paragraphIndentInsertionPoint;
        bool                        skipInsert                      = false;

        for (paragraphIndentInsertionPoint = adjustedTabStops.begin (); paragraphIndentInsertionPoint != adjustedTabStops.end (); paragraphIndentInsertionPoint++)
            {
            if (fabs (m_paragraphIndent - *paragraphIndentInsertionPoint) < mgds_fc_epsilon)
                {
                skipInsert = true;
                break;
                }
            
            if (m_paragraphIndent < *paragraphIndentInsertionPoint)
                break;
            }
        
        if (!skipInsert)
            adjustedTabStops.insert (paragraphIndentInsertionPoint, m_paragraphIndent);
        }

    size_t  nTabStops   = adjustedTabStops.size ();
    double  tabOffset   = 0.0;

    for (size_t i = 0; i < nTabStops; i++)
        {
        tabOffset = adjustedTabStops[i];

        if (tabOffset > currentOffset && ((tabOffset < lineBreakLength) || (0.0 == lineBreakLength)))
            return tabOffset;
        }

    int val = (int) (currentOffset / (DEFAULTTABSTOPHEIGHTFACTOR * textHeight)) + 1;
    tabOffset = val * DEFAULTTABSTOPHEIGHTFACTOR * textHeight;

    if (fabs (currentOffset - tabOffset) < mgds_fc_epsilon)
        tabOffset = (val + 1) * DEFAULTTABSTOPHEIGHTFACTOR * textHeight;

    return tabOffset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/06
//---------------------------------------------------------------------------------------
void IndentationData::Scale (double scale)
    {
    if (IsDefault ())
        return;

    m_firstLineIndent *= scale;
    m_paragraphIndent *= scale;

    size_t nTabStops = m_tabStops.size ();
    
    for (size_t i = 0; i < nTabStops; ++i)
        m_tabStops[i] *= scale;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
void IndentationData::Clear ()
    {
    m_firstLineIndent = 0.0;
    m_paragraphIndent = 0.0;
    m_tabStops.clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
bool IndentationData::Equals (IndentationDataCR rhs) const { return this->Equals (rhs, 0.0); }
bool IndentationData::Equals (IndentationDataCR rhs, double tolerance) const
    {
    if (fabs (m_firstLineIndent - rhs.m_firstLineIndent) > tolerance)   return false;
    if (fabs (m_paragraphIndent - rhs.m_paragraphIndent) > tolerance)   return false;
    
    bvector<double>::size_type numTabStops = m_tabStops.size ();
    
    if (numTabStops != rhs.m_tabStops.size ())
        return false;
    
    if (numTabStops > 0)
        {
        for (bvector<double>::size_type i = 0; i < numTabStops; ++i)
            if (fabs (m_tabStops[i] - rhs.m_tabStops[i]) > tolerance)
                return false;
        }
    
    return true;
    }
