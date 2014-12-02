/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/MultilineHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

/*=================================================================================**//**
  @addtogroup MultilineElements Multi-line Elements

  @brief Multi-lines are a complex element that displays multiple profiles. 

  Multi-lines are a complex element that consists of a work line, one or more profiles that
  are parallel to that work line, caps at the orgin and end, and breaks through one or 
  more profiles.
  
  The work line is a series of points that define the placement of the multi-line.  These
  correspond to the points you pick when placing a multi-line, and define the zero offset
  for profiles.
  
  There can be up to MULTILINE_MAX (16) profiles in a multi-line.  Each profile consists of
  an offset from the work line and the symbology for that profile.  The symbology is actually
  set up on two levels.  First, there is symbology on the element itself, set up in the normal
  way.  Often this is the active symbology at the time of placement.  Then each profile can 
  choose to override any given piece of that symbology.  For example,
  the element may have a color of 7.  The first profile may have a color of 3, the second
  profile may have a color of 12, and the third profile may choose to just use the header color.
  
  There are three caps.  The origin cap and end cap are at the start and end of the multi-line
  respectively.  Mid or joint caps appear at every point in the work line.  Caps have symbology
  that works the same as profiles.  The origin and end caps can be set to show lines, arcs
  that go only between the outer-most profiles (outer arcs) and/or arcs that go between each
  pair of interior multlines, concentrically inside the outer arcs (inner arcs).  Furthermore
  the angle of the start and end caps can be set.
  
  @bsiclass 
+===============+===============+===============+===============+===============+======*/

#define     MLINE_MATCH_ENDCAPS     0x01
#define     MLINE_BREAKS_ALLLINES   0xffffffff

#include <DgnPlatform/DgnCore/DisplayHandler.h>
#include <DgnPlatform/DgnCore/ElementGeometry.h>
#include <DgnPlatform/DgnHandlers/MultilineStyle.h>
#include <DgnPlatform/DgnHandlers/IAreaFillProperties.h>

DGNPLATFORM_TYPEDEFS (MultilinePoint)
DGNPLATFORM_TYPEDEFS (MultilineBreak)
     
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (NEEDS_WORK_DGNITEM)

struct MultilineChangeMergingExtension;

//! @ingroup MultilineElements
//! Smart pointer wrapper for MultilinePoint
typedef RefCountedPtr<MultilinePoint>           MultilinePointPtr;

//! @ingroup MultilineElements
//! Smart pointer wrapper for MultilineBreak
typedef RefCountedPtr<MultilineBreak>           MultilineBreakPtr;

/*=================================================================================**//**
Stores a point for a multiline
@ingroup MultilineElements
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultilinePoint : RefCountedBase
{
//__PUBLISH_SECTION_END__

friend struct MultilineHandler;

private:
    DPoint3d    m_point;
    bool        m_associative;
    
    UInt16      m_breakNo;    // Starting point in break array for this segment
    UInt16      m_numBreaks;  // Number of breaks in this segment

    void            Init ();
    void            FromElementData (MlinePoint const & mlPoint);
    BentleyStatus   FromElementData (ElementHandleCR eh, UInt32 pointNum);
    UInt32          GetBreakNumber  () const;

protected:
    static MultilinePointPtr CreateFromPoint (MlinePoint const & mlPoint);
    void            SetAssociative (bool value);

public:
    void            ToElementData (MlinePoint& mlPoint) const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Create a multi-line point from X,Y,Z coordinates
//! @param[in] point    The coordinates of the point
//! @return a MultilinePoint
DGNPLATFORM_EXPORT    static MultilinePointPtr CreateFromPoint (DPoint3dCR point);

//! Get the X,Y,Z coordinates from a MultilinePoint
//! @return The X,Y,Z coordinates
DGNPLATFORM_EXPORT    DPoint3d        GetPoint () const;

//! Set the X,Y,Z coordinates for a MultilinePoint
//! @param[in] point    The coordinates of the point
DGNPLATFORM_EXPORT    void            SetPoint (DPoint3dCR point);
    
//! Determine if a MultilinePoint is associative
//! @return True if the point is associative.
DGNPLATFORM_EXPORT    bool            IsAssociative () const;

//! Get the number of breaks on the segment that begins witn this point
//! @return The number of breaks between this point and the subsequent point
DGNPLATFORM_EXPORT    UInt32          GetNumBreaks () const;  


}; // MultilinePoint


/*=================================================================================**//**
Stores breaks for a multiline.
@ingroup MultilineElements
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultilineBreak : RefCountedBase
{
//__PUBLISH_SECTION_END__
friend struct MultilineHandler;

private:
    UInt32                  m_lineMask;
    MlineBreakLengthType    m_lengthFlags;
    double                  m_offset;
    double                  m_length;
    double                  m_angle;

    void            ToElementData (MlineBreak& mlbreak) const;
    void            FromElementData (MlineBreak const & mlbreak);
    void            AdjustOffset (double delta);
    void            AdjustLength (double delta);
    
    static MultilineBreakPtr Create (MlineBreak const & elmBreak);

public:
// Angle was not visible from MDL; I don't know if we use it yet.
//! Get the angle for the multi-line break.  The angle is measured in radians where 0 is perpindicular to the workline at the point of the break.
//! @return The the offset along the segment of the break
DGNPLATFORM_EXPORT double GetAngle () const;

// Angle was not visible from MDL; I don't know if we use it yet.
//! Set the angle for the multi-line break.  The angle is measured in radians where 0 is perpindicular to the workline at the point of the break.
//! @param[in] type    The new offset for the break.
DGNPLATFORM_EXPORT void SetAngle (double angle);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
//! Create a multi-line break on a specific segment.
//! @param[in] offset    The offset along the segment work line.  It will be igored if lengthType is MLBREAK_FROM_JOINT or MLBREAK_BETWEEN_JOINTS.
//! @param[in] lengthType    Qualifier for the break length; can specify whether it starts at previous point and/or runs to next point.
//! @param[in] length    The length of the break; ignored if lengthType is MLBREAK_TO_JOINT or MLBREAK_BETWEEN_JOINTS.
//! @param[in] profileMask  A bit mask of profiles to break.  Each bit corresponds to the profiles in profile order, not spatial order.  Use MLINE_BREAKS_ALLLINES for all lines.
//! @return a MultilinePoint
DGNPLATFORM_EXPORT static MultilineBreakPtr Create (double offset, MlineBreakLengthType lengthType, double length, UInt32 profileMask);

//! Get the length flags from the multi-line break.  This flag determines whether the break extends from one or both of the points, or just has a fixed length.
//! @return The current type
DGNPLATFORM_EXPORT MlineBreakLengthType GetLengthType () const;

//! Set the length flags from the multi-line break.  This flag determines whether the break extends from one or both of the points, or just has a fixed length.
//! @param[in] type    The new type.
DGNPLATFORM_EXPORT void SetLengthType (MlineBreakLengthType type);

//! Get the length from the multi-line break.  This value is ignored if the the MlineBreakLengthType is MLBREAK_TO_JOINT or MLBREAK_BETWEEN_JOINTS.
//! @return The length of the break
DGNPLATFORM_EXPORT double GetLength () const;

//! Set the length from the multi-line break.  This value is ignored if the the MlineBreakLengthType is MLBREAK_TO_JOINT or MLBREAK_BETWEEN_JOINTS.
//! @param[in] length    The new length of the break.
DGNPLATFORM_EXPORT void SetLength (double length);

//! Get the offset from the first point in the segment.  The offset is always along the workline, regardless of angle.
//!   This value is ignored if the the MlineBreakLengthType is MLBREAK_FROM_JOINT or MLBREAK_BETWEEN_JOINTS.
//! @return The offset along the segment of the break
DGNPLATFORM_EXPORT double GetOffset () const;

//! Get the offset from the first point in the segment.  The offset is always along the workline, regardless of angle.
//!   This value is ignored if the the MlineBreakLengthType is MLBREAK_FROM_JOINT or MLBREAK_BETWEEN_JOINTS.
//! @param[in] offset    The new offset for the break.
DGNPLATFORM_EXPORT void SetOffset (double offset);

//! Get the profile mask for the breaks.  Each bit represents a profile.  Bits are in profile order, not in any spatial order.
//! @return The profile mask of the break
DGNPLATFORM_EXPORT UInt32 GetProfileMask () const;

//! Get the profile mask for the breaks.  Each bit represents a profile.  Bits are in profile order, not in any spatial order.
//! @param[in] lineMask    The new mask for the break.
DGNPLATFORM_EXPORT void SetProfileMask (UInt32 lineMask);

//! Determine if a given profile is affected by this break.  This is just a convenience routine that does the bit comparisons.
//!  Profiles are 0-based.
//! @param[in] profileNum    Profile to test.
DGNPLATFORM_EXPORT bool ProfileIsMasked (UInt32 profileNum);

}; // MultilineBreak

/*=================================================================================**//**
Query an element for multi-line specific properties.
@ingroup MultilineElements
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct IMultilineQuery
{
//__PUBLISH_SECTION_END__
protected:

virtual bool                    _IsClosed (ElementHandleCR source) const = 0;
virtual UInt32                  _GetProfileCount (ElementHandleCR source) const = 0;
virtual UInt32                  _GetPointCount (ElementHandleCR source) const = 0;
virtual MultilinePointPtr       _GetPoint (ElementHandleCR source, UInt32 pointNumber) const = 0;
virtual UInt32                  _GetBreakCount (ElementHandleCR source) const = 0;
virtual MultilineBreakPtr       _GetBreak (ElementHandleCR source, UInt32 segmentNumber, UInt32 segBreakNumber) const = 0;
virtual JointDef                _ExtractJointDefinition (ElementHandleCR source, DPoint3dCP pts, int pointNo) const = 0;
virtual JointDef                _ExtractCapJointDefinition (ElementHandleCR source, DPoint3dCP pts, int pointNo) const = 0;
virtual BentleyStatus           _ExtractPoints (ElementHandleCR source, DPoint3dP pXYZBuffer, size_t& numPoints, size_t maxOut) const = 0;
virtual MultilineStylePtr       _GetStyle (ElementHandleCR eh, MultilineStyleCP seedStyle, UInt32 options) const = 0;
virtual double                  _GetStyleScale (ElementHandleCR element) const = 0;
virtual MultilineProfilePtr     _GetProfile (ElementHandleCR source, int index) const = 0;
virtual MultilineSymbologyPtr   _GetOriginCap (ElementHandleCR source) const = 0;
virtual MultilineSymbologyPtr   _GetEndCap (ElementHandleCR source) const = 0;
virtual MultilineSymbologyPtr   _GetMidCap (ElementHandleCR source) const = 0;
virtual MlineOffsetMode         _GetOffsetMode (ElementHandleCR source) const = 0;
virtual double                  _GetPlacementOffset (ElementHandleCR source) const = 0;
virtual double                  _GetOriginAngle (ElementHandleCR source) const = 0;
virtual double                  _GetEndAngle (ElementHandleCR source) const = 0;
virtual LevelId                 _GetEffectiveSymbologyLevel (ElementHandleCR source, UInt32 index, bool isCap) const = 0;
virtual UInt32                  _GetEffectiveSymbologyColor (ElementHandleCR source, UInt32 index, bool isCap) const = 0;
virtual UInt32                  _GetEffectiveSymbologyWeight (ElementHandleCR source, UInt32 index, bool isCap) const = 0;
virtual Int32                   _GetEffectiveSymbologyStyle (ElementHandleCR source, UInt32 index, bool isCap) const = 0;

public:                         // currently unpublished

DGNPLATFORM_EXPORT LevelId      GetEffectiveSymbologyLevel (ElementHandleCR source, UInt32 index, bool isCap) const;
DGNPLATFORM_EXPORT UInt32       GetEffectiveSymbologyColor (ElementHandleCR source, UInt32 index, bool isCap) const;
DGNPLATFORM_EXPORT UInt32       GetEffectiveSymbologyWeight (ElementHandleCR source, UInt32 index, bool isCap) const;
DGNPLATFORM_EXPORT Int32        GetEffectiveSymbologyStyle (ElementHandleCR source, UInt32 index, bool isCap) const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Query if the multiline element represents a closed path.
//! @param[in] source  source element
//! @return true for a closed multiline
DGNPLATFORM_EXPORT bool                IsClosed (ElementHandleCR source) const;

//! Query the number of profile lines in the multiline element.
//! @param[in] source  source element
//! @return multiline profile count
DGNPLATFORM_EXPORT UInt32              GetProfileCount (ElementHandleCR source) const;

//! Query a profile line definition. The profile definition defines the offset from the base profile
//! line as well as the symbology of the profile line.
//! @param[in] source source element
//! @param[in] index number of profile line to extract
//! @return A pointer to the MlineProfile definition.
DGNPLATFORM_EXPORT MultilineProfilePtr GetProfile (ElementHandleCR source, int index) const;

//! Query the symbology of the multiline origin cap.
//! @param[in] source source element
//! @return A copy of the cap symbology
DGNPLATFORM_EXPORT   MultilineSymbologyPtr  GetOriginCap (ElementHandleCR source) const;

//! Query the symbology of the multiline end cap.
//! @param[in] source source element
//! @return A copy of the cap symbology
DGNPLATFORM_EXPORT   MultilineSymbologyPtr  GetEndCap (ElementHandleCR source) const;

//! Query the symbology of the multiline middle or joint cap.
//! @param[in] source source element
//! @return A copy of the cap symbology
DGNPLATFORM_EXPORT   MultilineSymbologyPtr  GetMidCap (ElementHandleCR source) const;

//! Get the origin or start cap angle for the multiline
//! @param[in] source source element
//! @return The angle in radians
DGNPLATFORM_EXPORT  double  GetOriginAngle (ElementHandleCR source) const;

//! Get the end cap angle for the multiline
//! @param[in] source source element
//! @return The angle in radians
DGNPLATFORM_EXPORT  double  GetEndAngle (ElementHandleCR source) const;

//! Get a pointer to the first MlinePoint definition.
//! @param[in] source source element
//! @param[in] pointNumber point index to return
//! @return A pointer to the first MlinePoint definition
DGNPLATFORM_EXPORT  MultilinePointPtr   GetPoint (ElementHandleCR source, UInt32 pointNumber) const;

//! Query the number of points in the multiline element.
//! @param[in] source source element
//! @return multiline point count
DGNPLATFORM_EXPORT UInt32              GetPointCount (ElementHandleCR source) const;

//! Get the number of breaks on a multline.
//! @param[in] source source element
//! @return The number of breaks
DGNPLATFORM_EXPORT UInt32                  GetBreakCount (ElementHandleCR source) const;

//! Get a pointer to a multiline break based on the segment of the multiline.
//! @param[in] source source element
//! @param[in] segmentNumber Segment to find break
//! @param[in] segBreakNumber Which break on that segment
//! @return A pointer to the a copy of the break.  Be sure to check isValid() on the return in case the break doesn't exist.
DGNPLATFORM_EXPORT MultilineBreakPtr       GetBreak (ElementHandleCR source, UInt32 segmentNumber, UInt32 segBreakNumber) const;

//! Extract a multiline joint definition for a given segment of the multiline.
//! @param[in] source source element
//! @param[in] pts base point array from ExtractPoints.
//! @param[in] pointNo index of first point.
//! @return A joint definition
DGNPLATFORM_EXPORT JointDef             ExtractJointDefinition (ElementHandleCR source, DPoint3dCP pts, int pointNo) const;

//! Extract a multiline cap joint definition for a given segment of the multiline.
//! @param[in] source source element
//! @param[in] pts start and end points of line segment.
//! @param[in] pointNo cap joint, 0 for org and 1 for end.
//! @return A joint definition
DGNPLATFORM_EXPORT JointDef             ExtractCapJointDefinition (ElementHandleCR source, DPoint3dCP pts, int pointNo) const;

//! Extract the array of points from a multi-line. These represent the workline of the multi-line, and may not 
//!   correspond to anything displayed.
//! @param[in] source source element
//! @param[out] pXYZBuffer (optional) buffer for points.
//! @param[out] numPoints number of points
//! @param[in] maxOut buffer size. If numPoints is larger, the large number is returned but only maxOut are copied.
//! @return SUCCESS if the element is a multiline.
DGNPLATFORM_EXPORT BentleyStatus        ExtractPoints (ElementHandleCR source, DPoint3dP pXYZBuffer, size_t& numPoints, size_t maxOut) const;

//! Get an object representing the style of a multiline element.  In general, the properties of
//!   the returned style object may differ from the file's version of the same style.
//! @param[in] source multiline element
//! @param[in] seedStyle multiline style to use as a seed.  This is copied into the new style, and then all salient properties
//!   from the multi-line are applied on top.  For example if the multi-line doesn't use a color for profile 1 but there is
//!   a color in the seed style, then that color will appear in the resulting style.
//! @param[in] options multiline style options - currently 0 or MLINE_MATCH_ENDCAPS.
//! @return multiline style
DGNPLATFORM_EXPORT MultilineStylePtr   GetStyle (ElementHandleCR source, MultilineStyleCP seedStyle, UInt32 options) const;

//! Get the style scale stored on the multi-line element
//! @param[in] source multiline element
//! @return multiline style scale
DGNPLATFORM_EXPORT double          GetStyleScale (ElementHandleCR source) const;

//! Get the Offset Mode stored on the multi-line element
//! @param[in] source multiline element
//! @return multiline offset mode
DGNPLATFORM_EXPORT MlineOffsetMode  GetOffsetMode (ElementHandleCR source) const;

//! Get the Offset Distance stored on the multi-line element.  Note that this is only valid for a Offset Mode of MlineOffsetMode::Custom.
//! @param[in] source multiline element
//! @return multiline offset distance
DGNPLATFORM_EXPORT  double  GetPlacementOffset (ElementHandleCR source) const;

}; // IMultilineQuery


/*=================================================================================**//**
Set specific properties on a multi-line
@ingroup MultilineElements
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct IMultilineEdit : IMultilineQuery
{
//__PUBLISH_SECTION_END__
protected:

virtual BentleyStatus           _ReplacePoint (EditElementHandleR source, DPoint3dCR newPoint, UInt32 pointNum, MlineModifyPoint options) = 0;
virtual BentleyStatus           _InsertPoint (EditElementHandleR source, DPoint3dCR newPoint, AssocPoint const * assocPt, UInt32 pointNum) = 0;
virtual BentleyStatus           _DeletePoint (EditElementHandleR source, UInt32 pointNum) = 0;
virtual BentleyStatus           _SetZVector (EditElementHandleR element, DVec3dCR normal) = 0;
virtual BentleyStatus           _SetClosed (EditElementHandleR element, bool isClosed) = 0;
virtual BentleyStatus           _ApplyStyle (EditElementHandleR element, MultilineStyleCR mlineStyle, double newStyleScale) = 0;
virtual BentleyStatus           _SetProfile (EditElementHandleR element, UInt32 index, MultilineProfileCR profile) = 0;
virtual BentleyStatus           _SetOriginCap (EditElementHandleR element, MultilineSymbologyCR capSymbology) = 0;
virtual BentleyStatus           _SetEndCap (EditElementHandleR element, MultilineSymbologyCR capSymbology) = 0;
virtual BentleyStatus           _SetMidCap (EditElementHandleR element, MultilineSymbologyCR capSymbology) = 0;
virtual BentleyStatus           _SetOffsetMode (EditElementHandleR element, MlineOffsetMode offsetMode) = 0;
virtual BentleyStatus           _SetPlacementOffset (EditElementHandleR element, double placementOffset) = 0;
virtual BentleyStatus           _InsertBreak (EditElementHandleR source, MultilineBreakCR mlbreak, UInt32 segment) = 0;
virtual BentleyStatus           _DeleteBreak (EditElementHandleR source, UInt32 segment, UInt32 breakNo) = 0;
virtual BentleyStatus           _SetOriginAngle (EditElementHandleR element, double angle) = 0;
virtual BentleyStatus           _SetEndAngle (EditElementHandleR element, double angle) = 0;


//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Replace point in the multiline definition.  If you want to replace a point and drop Association, then specify the MlineModifyPoint::RemoveAssociations option;
//!   otherwise you cannot replace an Associative Point.  To make a point Associative, use DeletePoint followed by InsertPoint.
//! @param[in] element   source element
//! @param[in] newPoint  point to replace
//! @param[in] pointNum  index to replace
//! @param[in] options   replace option flags - see enum MlineModifyPoint
//! @return SUCCESS if the element is a multiline and the point currently exists.
DGNPLATFORM_EXPORT BentleyStatus ReplacePoint (EditElementHandleR element, DPoint3dCR newPoint, UInt32 pointNum, MlineModifyPoint options);

//! Insert point in the multiline definition.
//! @param[in] element      source element
//! @param[in] newPoint     point to add
//! @param[in] assocPointP  associative point for this mline point.  NULL to use a normal point from newPoint.
//! @param[in] pointNum     index to insert point.  -1 means at the end.
//! @return SUCCESS if the element is a multiline and the point was inserted.
DGNPLATFORM_EXPORT BentleyStatus InsertPoint (EditElementHandleR element, DPoint3dCR newPoint, AssocPoint const * assocPointP, UInt32 pointNum);

//! Delete point in the multiline definition.
//! @param[in] element      source element
//! @param[in] pointNum     index to delete point.
//! @return SUCCESS if the element is a multiline and the point was deleted.
DGNPLATFORM_EXPORT BentleyStatus DeletePoint (EditElementHandleR element, UInt32 pointNum);

//! Set the origin or start cap angle for the multiline.  PI/2 is perpindicular. 
//! @param[in] element      source element
//! @param[in] angle        New angle in radians
//! @return SUCCESS if the element is a multiline.
DGNPLATFORM_EXPORT BentleyStatus SetOriginAngle (EditElementHandleR element, double angle);

//! Set the end cap angle for the multiline.  PI/2 is perpindicular. 
//! @param[in] element      source element
//! @param[in] angle        New angle in radians
//! @return SUCCESS if the element is a multiline.
DGNPLATFORM_EXPORT  BentleyStatus SetEndAngle (EditElementHandleR element, double angle);

//! Replace a break in the multiline definition.
//! @param[in] element  source element
//! @param[in] mlbreak  break to replace
//! @param[in] segment  which segment the break is on
//! @return SUCCESS if the element is a multiline and the break currently exists.
DGNPLATFORM_EXPORT BentleyStatus InsertBreak (EditElementHandleR element, MultilineBreakCR mlbreak, UInt32 segment);

//! Delete a break from the multiline definition.
//! @param[in] element  source element
//! @param[in] segment  which segment the break is on
//! @param[in] breakNo  which break on the segment it is
//! @return SUCCESS if the element is a multiline and the break currently exists.
DGNPLATFORM_EXPORT BentleyStatus DeleteBreak (EditElementHandleR element, UInt32 segment, UInt32 breakNo);

//! Set the z axis of a 3d multiline element. The element is planar, and this bvector defines that plane.
//! @param[out] element The element to update.
//! @param[in]  normal  new z axis of multiline element.
//! @return SUCCESS if element is the correct type and the element was updated.
//! @note To query the multiline element's Z bvector use DisplayHandler::IsPlanar
DGNPLATFORM_EXPORT BentleyStatus SetZVector (EditElementHandleR element, DVec3dCR normal);

//! Set the closed property of a multiline element.
//! @param[out] element  The element to update.
//! @param[in]  isClosed new closure status.
//! @return SUCCESS if element's closure status was updated.
DGNPLATFORM_EXPORT BentleyStatus SetClosed (EditElementHandleR element, bool isClosed);

//! Replace the symbology of the multiline origin cap geometry.
//! @param[out] element     The element to update.
//! @param[in] capSymbology The new symbology
//! @return SUCCESS if element is the correct type and the element was updated.
DGNPLATFORM_EXPORT BentleyStatus SetOriginCap (EditElementHandleR element, MultilineSymbologyCR capSymbology);

//! Replace the symbology of the multiline end cap geometry.
//! @param[out] element         The element to update.
//! @param[in] capSymbology     The new symbology
//! @return SUCCESS if element is the correct type and the element was updated.
DGNPLATFORM_EXPORT BentleyStatus SetEndCap (EditElementHandleR element, MultilineSymbologyCR capSymbology);

//! Replace the symbology of the multiline middle or joint cap geometry.
//! @param[out] element      The element to update.
//! @param[in] capSymbology The new symbology
//! @return SUCCESS if element is the correct type and the element was updated.
DGNPLATFORM_EXPORT BentleyStatus SetMidCap (EditElementHandleR element, MultilineSymbologyCR capSymbology);

//! Replace a multiline profile
//! @param[out] element     The element to update.
//! @param[in] index        The index of the profile to replace.  The multiline must already have a profile at this index.
//! @param[in] profile      The new profile
//! @return SUCCESS if element is the correct type, the profile index is within range, and the element was updated.
DGNPLATFORM_EXPORT BentleyStatus SetProfile (EditElementHandleR element, UInt32 index, MultilineProfileCR profile);

//! Apply a multiline style to a given multiline element.
//! @param[out] element    The element to update.
//! @param[in]  mlineStyle The style to apply
//! @param[in]  styleScale The scale for the style.  Scale applies to the multi-line profile offsets.
//! @return true if eeh is the correct type and the element was updated.
//! @note To query the multiline element's Z bvector use DisplayHandler::IsPlanar
DGNPLATFORM_EXPORT BentleyStatus    ApplyStyle (EditElementHandleR element, MultilineStyleCR mlineStyle, double styleScale);

//! Set the Offset Mode stored on the multi-line element.  This mode will be used if the style is reapplied.  Note that this does not
//!   modify the current profile locations.
//! @param[in] element multiline element
//! @param[in] offsetMode for multiline
DGNPLATFORM_EXPORT BentleyStatus SetOffsetMode (EditElementHandleR element, MlineOffsetMode offsetMode);

//! Set the offset distance stored on the multi-line element.  It represents the distance between the Work Line (profile offset of zero)
//!   and the line that the user drew to place the multi-line.  This value is only used if the Offset Mode is MlineOffsetMode::Custom.
//!   This distance will be used if the style is reapplied; it does not modify the current profile locations.
//! @param[in] element multiline element
//! @param[in] placementOffset distance for multiline
DGNPLATFORM_EXPORT BentleyStatus SetPlacementOffset (EditElementHandleR element, double placementOffset);
}; // IMultilineEdit

/// @addtogroup DisplayHandler
/// @beginGroup

/*=================================================================================**//**
The default type handler for the MULTILINE_ELM type that corresponds to the 
MlineElm structure.
@ingroup MultilineElements
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE MultilineHandler : DisplayHandler,
                                   IMultilineEdit,
                                   IAreaFillPropertiesEdit
{
DEFINE_T_SUPER(DisplayHandler)
ELEMENTHANDLER_DECLARE_MEMBERS (MultilineHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
private:
friend struct MultilineChangeMergingExtension;

static void                             CalculatePerpVec (DPoint3dP perpvec, ElementHandleCR eh, DPoint3dCP lineseg);   // linseg is ARRAY OF 2 POINTS

static BentleyStatus                    InsertPoint (DgnElementP elm, DPoint3dCP inPoint);
static MultilinePointPtr                GetMlinePointStatic (ElementHandleCR source, UInt32 pointNumber);

BentleyStatus                           EvaluatePoint (DPoint3dP point, DVec3dP tangent, ElementHandleCR eh, double fraction) const;
BentleyStatus                           TransformMline (EditElementHandleR eeh, TransformCP ct, bool mirrorOffsets, bool scaleOffsets);
BentleyStatus                           DrawLineString (ElementHandleCR thisElm, ViewContextR context, DPoint3dP inPoints, JointDef* joints, int nPoints, MlineSymbology const* symb, int lineNo, struct MlineTopology& topology);
BentleyStatus                           StrokeMline (ElementHandleCR thisElm, ViewContextR context);
BentleyStatus                           DrawEndCap (ElementHandleCR thisElm, ViewContextR context, DPoint3dP origin, JointDef* joint, int end);
BentleyStatus                           DrawJointLine (ElementHandleCR thisElm, ViewContextR context, DSegment3dP pJointLine, MlineSymbology const* pSymbology, int lsIndex);
BentleyStatus                           OutputLine (ElementHandleCR thisElm, ViewContextR context, DSegment3dCP line, MlineSymbology const*   symb, int lsIndex);
BentleyStatus                           OutputLineString (ElementHandleCR thisElm, ViewContextR context, DPoint3dP inPoints, int nPoints, MlineSymbology const* symb, int lsIndex, bool isCap);
void                                    DrawMline (ElementHandleCR thisElm, ViewContextR context, UInt32 info);
void                                    DrawPatterns (ElementHandleCR thisElm, ViewContextR context);
void                                    StrokePolygons (ElementHandleCR thisElm, ViewContextR context, int minLine, int maxLine);
void                                    GetMaskedProfile (MlineProfile* mlineProfile, MultilineStyleCR mlineStyle, int profileNum, MultilineStylePropMaskCR styleShields, double styleScale);
void                                    AppendLineStyleData (DgnElementR pMline, bool& wasAdded, int lineNo, int isCap, LineStyleParamsCR lsParams);
void                                    AppendCapLineStyle (DgnElementR pMline, bool& wasAdded, int lineNo, MultilineSymbologyCR pCap);
static bool                             GetCloseDef (DPoint3dP capDir, ElementHandleCR eh);
void                                    SetClosePoint (EditElementHandleR eeh);
int                                     OffsetLineBuffer (DPoint3dP pPoints, JointDef* pJoint, double dist, int nPoints);
void                                    SetCurrentDisplayParams (ElementHandleCR thisElm, ViewContextR context, MlineSymbology const* symb, int lsIndex, bool isCap) const;
bool                                    TestLevelAndClass (ElementHandleCR thisElm, ViewContextR context, MlineSymbology const* symb) const;
BentleyStatus                           OutputArc (ElementHandleCR thisElm, ViewContextR context, DPoint3dP points, MlineSymbology const* symb, MlineSymbology const* symb2, int lsIndex) const;
BentleyStatus                           DrawArcEndCap (ElementHandleCR thisElm, ViewContextR context, DPoint3dP min, double minDist, DPoint3dP max, double maxDist, DPoint3dP perpVec, int end) const;
void                                    ApplyMaskedSymbology (MlineSymbology* destSymb, MultilineSymbologyCR sourceSymb, MultilineStylePropMaskCR pStyleShields, bool isCap, int pcIndex);

BentleyStatus                           PointAtSignedDistance (DPoint3dP pXYZOut, double* pFractionOut, ElementHandleCR eh, double fractionA, double distanceAB, TransformCP pTransform) const;
BentleyStatus                           SignedDistanceAlong (double* pLength, ElementHandleCR eh, double parameterA, double parameterB, TransformCP pTransform) const;
BentleyStatus                           ExtractTransformedPartial (DPoint3dP pXYZOut, int* pNumOut, UInt32 maxOut, ElementHandleCR eh, double parameterA, double parameterB, TransformCP pTransform) const;
BentleyStatus                           ClosestCenterlinePointXY (DPoint3dP pXYZOut, double* pParamOut, double* pXYDistanceOut, ElementHandleCR eh, bool bExtend, DPoint3dCP pXYZIn, TransformCP pElementTransform, DMatrix4dCP  pViewTransform) const;
BentleyStatus                           ExtractTransformedPointsExt (DPoint3dP outPoints, UInt32* outCount, ElementHandleCR eh, UInt32 index, UInt32 maxOut, TransformCP pTransform) const; 
BentleyStatus                           ClosestCenterlinePoint (DPoint3dP pXYZOut, double* pParamOut, ElementHandleCR eh, bool bExtend, DPoint3dCP pXYZIn, TransformCP pTransform) const;
void                                    TransformMlineNormal (DVec3dP pNormal1, DVec3dP pNormal0, TransformCP pTransform) const;
MlineBreak const*                       GetFirstBreakCP (ElementHandleCR eh) const;
MlineBreak*                             GetFirstBreak (EditElementHandleR eh);
MlineProfile const*                     _GetMlineProfileDefCP (ElementHandleCR source, UInt32 index) const;
MlineSymbology const*                   _GetMlineCapSymbologyCP (ElementHandleCR source, MultilineCapType capType) const;
bool                                    IsIndexValid (ElementHandleCP source, UInt32 index) const;
void                                    GetZVector (DVec3dR normal, ElementHandleCR eh) const;
static UInt32                           GetPointCountStatic (ElementHandleCR eh);
DGNPLATFORM_EXPORT BentleyStatus        ReplaceMultilinePoint (EditElementHandleR element, MultilinePointCR newPoint, UInt32 pointNum);
void                                    ClipBadBreaks (EditElementHandleR element, UInt32 pointNo);
void                                    GetSegmentOffsets (double* orgOff, double* endOff, ElementHandleCR element, UInt32 pointNo);
UInt32                                  GetSegmentBreakStart (ElementHandleCR eh, UInt32 pointNum);

bool                                    ProcessLineStyleIDs (PropertyContextR, ElementHandleCR, EditElementHandleP, Int32* outStyleP, const MlineSymbology&  inSymb, bool isCap, int profileIndex);
bool                                    ProcessSymbologyProperties (PropertyContextR, ElementHandleCR, EditElementHandleP, MlineSymbology* out, const MlineSymbology& in, bool isCap, int profileIndex);
ScanTestResult                          TestScanLevelMask (ElementHandleCR eh, BitMaskCP levelsOn) ;
ScanTestResult                          TestScanClassMask (ElementHandleCR eh, int classMask);
bool                                    ProcessPropertiesHelper (PropertyContextR proc, ElementHandleCR eh, EditElementHandleP eehP);
void                                    ProcessProperties (PropertyContextR, ElementHandleCR, EditElementHandleP);
bool                                    UpdateShieldsByCompareWithStyle (EditElementHandleR element);
bool                                    HasMultilineStyle (ElementHandleCR element);

void                                    SetLinestyle (EditElementHandleR element, UInt32 styleNo, bool isCap, UInt32 lsIndex);
BentleyStatus                           SetLinestyleParams (EditElementHandleR element, LineStyleParamsCR params, bool isCap, UInt32 lsIndex);

WString                                 GetStyleName (ElementHandleCR element) const;
void                                    SetStyleID (EditElementHandleR element, ElementId styleId);
void                                    SetStyleScale (EditElementHandleR element, double scale);
bool                                    IsOffsetModeValid (ElementHandleCR element) const;
void                                    SetOffsetModeValid (EditElementHandleR element, bool offsetModeValid);

double                                  GetProfileDistance (ElementHandleCR element, UInt32 profileNum) const;
void                                    SetProfileDistance (EditElementHandleR element, UInt32 profileNum, double profileDistance);

bool                                    GetEffectiveMlineSymbologyProperties (LevelId* level, UInt32* color, UInt32* weight, Int32* style, ElementHandleCR element, UInt32 index, bool isCap) const;

protected:

// Handler
DGNPLATFORM_EXPORT virtual void                _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual bool                _IsTransformGraphics (ElementHandleCR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual ReprojectStatus     _OnGeoCoordinateReprojection (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;
DGNPLATFORM_EXPORT virtual void                _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void                _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;
DGNPLATFORM_EXPORT virtual void                _QueryProperties (ElementHandleCR eh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void                _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual bool                _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

DGNPLATFORM_EXPORT virtual ScanTestResult      _DoScannerTests (ElementHandleCR eh, BitMaskCP levelsOn, UInt32 const*, ViewContextP) override;

// DisplayHandler
virtual bool                                   _IsRenderable (ElementHandleCR) override {return true;}
DGNPLATFORM_EXPORT virtual void                _Draw (ElementHandleCR, ViewContextR) override;
DGNPLATFORM_EXPORT virtual bool                _IsPlanar (ElementHandleCR, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal) override;
DGNPLATFORM_EXPORT virtual bool                _IsVisible (ElementHandleCR, ViewContextR, bool testRange, bool testLevel, bool testClass) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) override;

// IMultilineQuery
DGNPLATFORM_EXPORT virtual bool                _IsClosed (ElementHandleCR source) const override;
DGNPLATFORM_EXPORT virtual UInt32              _GetProfileCount (ElementHandleCR source) const override;
DGNPLATFORM_EXPORT virtual UInt32              _GetPointCount (ElementHandleCR source) const override;
DGNPLATFORM_EXPORT virtual MultilinePointPtr   _GetPoint (ElementHandleCR source, UInt32 pointNumber) const override;
DGNPLATFORM_EXPORT virtual JointDef            _ExtractJointDefinition (ElementHandleCR source, DPoint3dCP pts, int pointNo) const override;
DGNPLATFORM_EXPORT virtual JointDef            _ExtractCapJointDefinition (ElementHandleCR source, DPoint3dCP pts, int pointNo) const override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _ExtractPoints (ElementHandleCR source, DPoint3dP pXYZBuffer, size_t& numPoints, size_t maxOut) const override;
DGNPLATFORM_EXPORT virtual MultilineStylePtr       _GetStyle (ElementHandleCR eh, MultilineStyleCP seedStyle, UInt32 options) const override;
DGNPLATFORM_EXPORT virtual double                  _GetStyleScale (ElementHandleCR element) const override;
DGNPLATFORM_EXPORT virtual MultilineProfilePtr     _GetProfile (ElementHandleCR source, int index) const override;
DGNPLATFORM_EXPORT virtual MultilineSymbologyPtr   _GetOriginCap (ElementHandleCR source) const override;
DGNPLATFORM_EXPORT virtual MultilineSymbologyPtr   _GetEndCap (ElementHandleCR source) const override;
DGNPLATFORM_EXPORT virtual MultilineSymbologyPtr   _GetMidCap (ElementHandleCR source) const override;
DGNPLATFORM_EXPORT virtual MlineOffsetMode         _GetOffsetMode (ElementHandleCR source) const override;
DGNPLATFORM_EXPORT virtual double                  _GetPlacementOffset (ElementHandleCR source) const override;
DGNPLATFORM_EXPORT virtual UInt32                  _GetBreakCount (ElementHandleCR source) const override;
DGNPLATFORM_EXPORT virtual MultilineBreakPtr       _GetBreak (ElementHandleCR source, UInt32 segmentNumber, UInt32 segBreakNumber) const override;
DGNPLATFORM_EXPORT virtual double                  _GetOriginAngle (ElementHandleCR source) const override;
DGNPLATFORM_EXPORT virtual double                  _GetEndAngle (ElementHandleCR source) const override;
DGNPLATFORM_EXPORT virtual LevelId                 _GetEffectiveSymbologyLevel (ElementHandleCR source, UInt32 index, bool isCap) const override;
DGNPLATFORM_EXPORT virtual UInt32                  _GetEffectiveSymbologyColor (ElementHandleCR source, UInt32 index, bool isCap) const override;
DGNPLATFORM_EXPORT virtual UInt32                  _GetEffectiveSymbologyWeight (ElementHandleCR source, UInt32 index, bool isCap) const override;
DGNPLATFORM_EXPORT virtual Int32                   _GetEffectiveSymbologyStyle (ElementHandleCR source, UInt32 index, bool isCap) const override;

// IMultilineEdit
DGNPLATFORM_EXPORT virtual BentleyStatus       _ReplacePoint (EditElementHandleR source, DPoint3dCR newPoint, UInt32 pointNum, MlineModifyPoint options) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _InsertPoint (EditElementHandleR source, DPoint3dCR newPoint, AssocPoint const * assocPt, UInt32 pointNum) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _DeletePoint (EditElementHandleR source, UInt32 pointNum) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _SetZVector (EditElementHandleR element, DVec3dCR normal) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _SetClosed (EditElementHandleR element, bool isClosed) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _ApplyStyle (EditElementHandleR element, MultilineStyleCR mlineStyle, double newStyleScale) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _SetProfile (EditElementHandleR element, UInt32 index, MultilineProfileCR profile) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _SetOriginCap (EditElementHandleR element, MultilineSymbologyCR capSymbology) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _SetEndCap (EditElementHandleR element, MultilineSymbologyCR capSymbology) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _SetMidCap (EditElementHandleR element, MultilineSymbologyCR capSymbology) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _SetOffsetMode (EditElementHandleR element, MlineOffsetMode offsetMode) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _SetPlacementOffset (EditElementHandleR element, double placementOffset) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _InsertBreak (EditElementHandleR source, MultilineBreakCR mlbreak, UInt32 segment) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _DeleteBreak (EditElementHandleR source, UInt32 segment, UInt32 breakNo) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _SetOriginAngle (EditElementHandleR element, double angle) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _SetEndAngle (EditElementHandleR element, double angle) override;

// IAreaFillPropertiesEdit
DGNPLATFORM_EXPORT virtual bool                _GetAreaType (ElementHandleCR eh, bool* isHoleP) const override {return false;}
DGNPLATFORM_EXPORT virtual bool                _SetAreaType (EditElementHandleR eeh, bool isHole) override {return false;}
DGNPLATFORM_EXPORT virtual bool                _AddGradientFill (EditElementHandleR eeh, GradientSymbCR symb) override {return false;} // not supported!
DGNPLATFORM_EXPORT virtual bool                _AddPattern (EditElementHandleR eeh, PatternParamsR params, DwgHatchDefLineP hatchDefLinesP, int index) override;

public:

DGNPLATFORM_EXPORT ElementId            GetStyleID (ElementHandleCR element) const;
DGNPLATFORM_EXPORT Int32                GetLinestyle (ElementHandleCR element, bool isCap, UInt32 lsIndex) const;
DGNPLATFORM_EXPORT BentleyStatus        GetLinestyleParams (ElementHandleCR element, LineStyleParamsR params, bool isCap, UInt32 lsIndex, bool allowSizeChange) const;
DGNPLATFORM_EXPORT double               GetOffsetDistance (ElementHandleR element, MlineOffsetMode offsetMode) const;

DGNPLATFORM_EXPORT BentleyStatus SetOverrides (EditElementHandleR element, MultilineStylePropMaskCP shieldFlags, bool applyStyle);
DGNPLATFORM_EXPORT static bool IsMultilineElement (ElementHandleCR pCandidate);

DGNPLATFORM_EXPORT static BentleyStatus ExtractPointArray (ElementHandleCR eh, DPoint3dP outPoints, UInt32 start, size_t numPoints);

//! Return an open or closed curve vector representing the workline of the multi-line. This may not correspond to anything displayed and is to
//! support code where it is sufficient to treat multilines as simple linestrings or shapes.
DGNPLATFORM_EXPORT static CurveVectorPtr WorklineToCurveVector (ElementHandleCR eh);

//! Return a closed curve vector representing the boundary shape of the multi-line between the specified min/max profile lines.
//! Boundary shape does not include area from end caps.
DGNPLATFORM_EXPORT static CurveVectorPtr BoundaryToCurveVector (ElementHandleCR eh, int minLine = -1, int maxLine = -1);

//! Get a pointer into the multline element to the first MlinePoint.
//! @param[in]  eh        The element to extract from.
//! @return A pointer to the first multiline point.
//! @bsimethod
// Used in depcallback.cpp still 
MlinePoint const* GetFirstPoint (ElementHandleCR eh) const;

static void GetAnyJointDef (ElementHandleCR eh, JointDef* jointDef, DSegment3dCP linePoints, int pointNo, int pointAtOrg);
UInt32 GetNumBreaksOnSegment (ElementHandleCR eh, UInt32 pointNum);

//! Get a bvector perpindicular to the segment in the plane of the multi-line
DGNPLATFORM_EXPORT static BentleyStatus GetPerpVector (DVec3dR perpVec, ElementHandleCR eh, DVec3dR segment);

//! Get the profile index for the minimum and maximum profile line offset
//! @param[out] minProfile    The index of the profile line with the minimum offset distance from the base line.
//! @param[out] maxProfile    The index of the profile line with the maximum offset distance from the base line.
//! @param[in]  eh            The element to extract from.
//! @bsimethod
DGNPLATFORM_EXPORT static void GetLimitProfiles (int& minProfile, int& maxProfile, ElementHandleCR eh);

//! Get the joint defintion for the multiline mid cap.
//! @param[out] jointDef  joint direction and scale.
//! @param[in]  eh        The element to extract from.
//! @param[in]  pts       base point array.
//! @param[in]  pointNo   index of first point.
//! @bsimethod
DGNPLATFORM_EXPORT static void GetJointDef (JointDef& jointDef, ElementHandleCR eh, DPoint3dCP pts, int pointNo);

//! Get the joint defintion for the multiline origin or end cap.
//! @param[out] jointDef  end cap joint direction and scale.
//! @param[in]  eh        The element to extract from.
//! @param[in]  pts       start and end points of line segment.
//! @param[in]  pointNo   cap joint, 0 for org and 1 for end.
DGNPLATFORM_EXPORT static void GetCapJointDef (JointDef& jointDef, ElementHandleCR eh, DPoint3dCP pts, int pointNo);

//! Create an assoc point to an mline
DGNPLATFORM_EXPORT static BentleyStatus CreateMlineAssoc (ElementHandleCR, AssocPoint&, SnapPathCR, int modifierMask, bool createFarPathElems, DgnModelP parentModel);

//! Evaluate an assoc point for a multi-line
DGNPLATFORM_EXPORT static StatusInt EvaluateMlineAssoc (ElementHandleCR, AssocPoint&, DPoint3dR point);

//! Query the symbology of the multiline origin, end, or mid cap geometry.  This differs from 
//!   GetOriginCap, GetEndCap, and GetMidCap in that it takes an index, which is convenient
//!   for looping.
//! @param source IN source element
//! @param capType IN Requested cap information: MULTILINE_ORG_CAP, MULTILINE_END_CAP, MULTILINE_MID_CAP
//! @return A pointer to the origin, end, mid cap symbology.
DGNPLATFORM_EXPORT MultilineSymbologyPtr GetMlineCapByIndex (ElementHandleCR source, MultilineCapType capType) const;

//! Replace the symbology of the multiline origin, end, or mid cap geometry.
//! @param element OUT The element to update.
//! @param capType IN Requested cap information: MULTILINE_ORG_CAP, MULTILINE_END_CAP, MULTILINE_MID_CAP
//! @param capSymbology IN The new symbology
//! @return SUCCESS if element is the correct type and the element was updated.
DGNPLATFORM_EXPORT BentleyStatus SetMlineCapByIndex (EditElementHandleR element, MultilineCapType capType, MultilineSymbologyCR capSymbology);

//! Create a new MULTILINE_ELM with the supplied parameters.  This one is usually used when copying the mline.
//! @param[out] eeh           The new element.
//! @param[in]  templateEh    Template element used to set the header symbology
//! @param[in]  normal        z axis of multiline element.
//! @param[in]  points        input point buffer.
//! @param[in]  numVerts      number of points.
//! @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
//! @param[in]  modelRef      Model to associate this element with. Required to compute range.
//! @return SUCCESS if a valid element is created and range was sucessfully calculated.
DGNPLATFORM_EXPORT static BentleyStatus CreateMultilineElement (EditElementHandleR eeh, ElementHandleCP templateEh, DVec3dCR normal, DPoint3dCP points, int numVerts, bool is3d, DgnModelR modelRef);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

//! Create a new MULTILINE_ELM with the supplied parameters.
//! @param[out] eeh           The new element.
//! @param[in]  templateEh    Template element used to set the header symbology
//! @param[in]  mlineStyle    Multi-line style to apply to the element
//! @param[in]  styleScale    Scale to use when applying the mline style
//! @param[in]  normal        z axis of multiline element.
//! @param[in]  points        input point buffer.
//! @param[in]  numVerts      number of points.
//! @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
//! @param[in]  modelRef      Model to associate this element with. Required to compute range.
//! @return SUCCESS if a valid element is created and range was sucessfully calculated.
DGNPLATFORM_EXPORT static BentleyStatus  CreateMultilineElement (EditElementHandleR eeh, ElementHandleCP templateEh, MultilineStyleCR mlineStyle, double styleScale, DVec3dCR normal, DPoint3dCP points, int numVerts, bool is3d, DgnModelR modelRef);

}; // MultilineHandler

/// @endGroup
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
