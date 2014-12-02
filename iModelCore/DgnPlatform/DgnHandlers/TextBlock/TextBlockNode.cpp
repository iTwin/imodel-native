/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/TextBlockNode.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/04
//---------------------------------------------------------------------------------------
TextBlockNode::TextBlockNode ()
    {
    m_origin.zero ();
    m_nominalRange.init ();
    m_exactRange.init ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2011
//---------------------------------------------------------------------------------------
TextBlockNode::TextBlockNode (TextBlockNodeCR rhs) :
    m_origin        (rhs.m_origin),
    m_nominalRange  (rhs.m_nominalRange),
    m_exactRange    (rhs.m_exactRange)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/04
//---------------------------------------------------------------------------------------
TextBlockNode::~TextBlockNode ()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TextBlockNodeLevel  TextBlockNode::GetUnitLevel                     () const                    { return _GetUnitLevel (); }
DPoint3d            TextBlockNode::GetOrigin                        () const                    { return _GetOrigin (); }
Transform           TextBlockNode::GetTransform                     () const                    { return _GetTransform (); }
DRange3d            TextBlockNode::GetNominalRange                  () const                    { return _GetNominalRange (); }
DRange3d            TextBlockNode::ComputeTransformedNominalRange   () const                    { return _GetTransformedNominalRange (); }
double              TextBlockNode::GetNominalWidth                  () const                    { return _GetNominalWidth (); }
double              TextBlockNode::GetNominalHeight                 () const                    { return _GetNominalHeight (); }
DRange3d            TextBlockNode::GetExactRange                    () const                    { return _GetExactRange (); }
DRange3d            TextBlockNode::ComputeTransformedExactRange     () const                    { return _GetTransformedExactRange (); }
double              TextBlockNode::GetExactWidth                    () const                    { return _GetExactWidth (); }
double              TextBlockNode::GetExactHeight                   () const                    { return _GetExactHeight (); }
void                TextBlockNode::Drop                             (TextBlockNodeArrayR nodes) { _Drop (nodes); }

void                TextBlockNode::SetOrigin                        (DPoint3dCR value)          { m_origin = value; }
void                TextBlockNode::ExtendNominalRange               (DRange3dCR value)          { if (value.IsEmpty ()) { return; } m_nominalRange.Extend (&value.low, 2); }
void                TextBlockNode::ExtendExactRange                 (DRange3dCR value)          { if (value.IsEmpty ()) { return; } m_exactRange.Extend (&value.low, 2); }

DRange3d            TextBlockNode::_GetNominalRange                 () const                    { return m_nominalRange; }
DRange3d            TextBlockNode::_GetExactRange                   () const                    { return m_exactRange; }
DPoint3d            TextBlockNode::_GetOrigin                       () const                    { return m_origin; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
Transform TextBlockNode::_GetTransform () const
    {
    DPoint3d origin = this->GetOrigin ();

    Transform transform;
    transform.initFrom (&origin);

    return transform;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
DRange3d TextBlockNode::_GetTransformedNominalRange () const
    {
    DRange3d nominalRange = this->GetNominalRange ();
    if (nominalRange.isNull ())
        return nominalRange;

    this->GetTransform ().multiply (&nominalRange.low, 2);

    return nominalRange;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
double TextBlockNode::_GetNominalWidth () const
    {
    DRange3d nominalRange = this->GetNominalRange ();
    if (nominalRange.isNull ())
        return 0.0;
    
    return std::max (0.0, nominalRange.high.x - nominalRange.low.x);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
double TextBlockNode::_GetNominalHeight () const
    {
    DRange3d nominalRange = this->GetNominalRange ();
    if (nominalRange.isNull ())
        return 0.0;
    
    return std::max (0.0, nominalRange.high.y - nominalRange.low.y);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
DRange3d TextBlockNode::_GetTransformedExactRange () const
    {
    DRange3d exactRange = this->GetExactRange ();
    if (exactRange.isNull ())
        return exactRange;

    this->GetTransform ().multiply (&exactRange.low, 2);

    return exactRange;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12-05
//---------------------------------------------------------------------------------------
double TextBlockNode::_GetExactWidth () const
    {
    DRange3d exactRange = this->GetExactRange ();
    if (exactRange.isNull ())
        return 0.0;
    
    return std::max (0.0, exactRange.high.x - exactRange.low.x);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
double TextBlockNode::_GetExactHeight () const
    {
    DRange3d exactRange = this->GetExactRange ();
    if (exactRange.isNull ())
        return 0.0;
    
    return std::max (0.0, exactRange.high.y - exactRange.low.y);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/05
//---------------------------------------------------------------------------------------
void TextBlockNode::_Drop (TextBlockNodeArrayR nodes)
    {
    nodes.push_back (this);
    }

//---------------------------------------------------------------------------------------
// The particular range that we use to perform layout has changed from version to version,
// and from font type to font type over the years. We are now looking to ensure we standardize
// on this range, the "justification range," which is generally where versions were tending
// towards. This range includes left and top bearing, but not right or bottom bearings.
// While in theory we always want to use all bearings, we have too much legacy (and AutoCAD)
// to support that does not follow this behavior. Note that AutoCAD MTEXT does not use
// left-side bearing, but this is a display behavior, not strictly a layout behavior.
// Word-wrapping computations should use exact ranges instead of this.
// @bsimethod                                                   Jeff.Marker     03/08
//---------------------------------------------------------------------------------------
DRange3d TextBlockNode::ComputeJustificationRange () const
    {
    return TextBlockUtilities::ComputeJustificationRange (this->GetNominalRange (), this->GetExactRange ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
bool TextBlockNode::Equals (TextBlockNodeCR rhs, TextBlockCompareOptionsCR compareOptions) const
    {
    if (!compareOptions.ShouldIgnoreCachedValues ())
        {
        if (!m_nominalRange.IsEqual (rhs.m_nominalRange, compareOptions.GetTolerance ()))   return false;
        if (!m_exactRange.IsEqual   (rhs.m_exactRange, compareOptions.GetTolerance ()))     return false;
        }
    
    if (!m_origin.IsEqual (rhs.m_origin, compareOptions.GetTolerance ())) return false;
    
    return true;
    }
