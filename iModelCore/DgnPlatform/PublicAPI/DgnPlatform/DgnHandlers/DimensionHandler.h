/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/DimensionHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */
#if defined (NEEDS_WORK_DGNITEM)

#include "TextHandlers.h"
#include <DgnPlatform/DgnHandlers/DimensionElem.h>
#include <DgnPlatform/DgnCore/ValueFormat.h>
#include <DgnPlatform/DgnCore/ElementGeometry.h>

#ifdef WIP_ECENABLERS
//__PUBLISH_SECTION_END__
#include "DelegatedElementECEnabler.h"
//__PUBLISH_SECTION_START__
#endif

DGNPLATFORM_TYPEDEFS (IDimCreateData);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct  DimFormattedText;

/*=================================================================================**//**
* An object of this type is required to supply necessary information to DimensionHandler::CreateDimensionElement.
* The intended use is for applications to implement their own subclass of IDimCreateData, overriding
* each of the pure virtual methods.
* @remarks ustation.dll supplies its own implementation which can be used by applications
*          that have access to it.  To use it, call MstnDimensionUtils::CreateDimension which
*          is not part of DgnPlatform.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct      IDimCreateData
    {
    protected:
        DirectionFormatterPtr  m_dirFormatter;

        IDimCreateData ()
            { m_dirFormatter = DirectionFormatter::Create(); }

    public:

    virtual ~IDimCreateData () {}
    virtual DimensionStyleCR                _GetDimStyle()      const = 0;                          //!< Supplies style information for the new dimension.
    virtual DgnTextStyleCR                  _GetTextStyle()     const = 0;                          //!< Supplies text style information for the new dimension.
    virtual Symbology                       _GetSymbology()     const = 0;                          //!< Supplies color, linestyle and weight information for the new dimension.
    virtual LevelId                         _GetLevelID()       const = 0;                          //!< Supplies the level for the new dimension.
    virtual DirectionFormatterCR            _GetDirFormat()     const {return *m_dirFormatter;}     //!< Supplies direction formatting information for the new dimension.  Only called when creating label lines.
    virtual int                             _GetViewNumber()    const = 0;                          //!< Supplies a view number which is stored on the dimension.  The value will be ignored.
    virtual RotMatrixCR                     _GetDimRMatrix()    const = 0;                          //!< Supplies a rotation matrix that determines the orientation of the dimension.
    virtual RotMatrixCR                     _GetViewRMatrix()   const = 0;                          //!< Supplies a rotation matrix that determines the orientation of text within the dimension.
#if defined (NEEDS_WORK_DGNITEM)
    virtual ElementRefP                     _GetSharedCell (WCharCP name) const
        {return ISharedCellQuery::FindDefinitionByName (name, *_GetDimStyle().GetDgnProject()).get();}
#endif
    };

/*__PUBLISH_SECTION_END__*/
/*------------------------------------------------------------------------*//**
* This enum describes the valid values for orientation of a dimension.
* @Group        "Dimension Functions"
+--------------+--------------+---------------+----------------+-------------*/
enum DimensionAlignment
    {
    View                = 0,
    Drawing             = 1,
    TrueOrientation     = 2,
    ArbitaryOrientation = 3
    };

/*__PUBLISH_SECTION_START__*/
/*=================================================================================**//**
* Query an element for dimension specific properties. 
* In order to query the properties of a dimension, get the dimensionstyle object representing
* the element using the function IDimensionQuery::GetDimensionStyle. Then query the required 
* values using DimensionStyle API.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE IDimensionQuery
{
//__PUBLISH_SECTION_END__
protected:
virtual DimensionStylePtr               _GetDimensionStyle (ElementHandleCR eh) const = 0;
virtual DimStylePropMaskPtr             _GetOverrideFlags (ElementHandleCR eh) const = 0;
virtual int                             _GetNumPoints (ElementHandleCR) const = 0;
virtual int                             _GetNumSegments (ElementHandleCR) const = 0;
virtual BentleyStatus                   _ExtractPoint (ElementHandleCR, DPoint3dR, int iPoint) const = 0;
virtual BentleyStatus                   _GetHeight (ElementHandleCR eh, double& height) const = 0;
virtual BentleyStatus                   _GetJustification (ElementHandleCR eh, int segmentNo, DimStyleProp_Text_Justification& just) const= 0;
virtual BentleyStatus                   _GetWitnessVisibility (ElementHandleCR eh, int pointNo, bool& value) const = 0;
virtual DimensionType                  _GetDimensionType (ElementHandleCR eh) const = 0;
virtual BentleyStatus                   _GetStackHeight (ElementHandleCR eh, int segmentNo, double& height) const = 0;
virtual BentleyStatus                   _GetRotationMatrix (ElementHandleCR eh, RotMatrixR rmatrix) const = 0;
virtual bool                            _IsTextOutside (ElementHandleCR eh) const = 0;
virtual BentleyStatus                   _IsPointAssociative (ElementHandleCR eh, int pointNo, bool& flag) const = 0;
virtual bool                            _IsVertexInsertable (ElementHandleCR dim) const = 0;
virtual bool                            _IsVertexDeletable (ElementHandleCR dim, HitPathCR hitPath) const = 0;
virtual BentleyStatus                   _GetProxyCell (ElementHandleCR dimElement, DgnPlatform::ElementId& proxyCellId, DPoint3dP origin, RotMatrixP rotMatrix) const = 0;
virtual BentleyStatus                   _GetTextOffset (ElementHandleCR dimElement, int segmentNo, DPoint2dR offset) const = 0;
virtual BentleyStatus                   _GetViewRotation (ElementHandleCR dimElement, RotMatrixR rmatrix) const = 0;
virtual bool                            _GetAngularDimensionClockWiseSweep (ElementHandleCR dimElement) const = 0;
virtual BentleyStatus                   _GetWitnessUseAltSymbology (ElementHandleCR eh, int pointNo, bool& value) const = 0;
DECLARE_KEY_METHOD

public:

//! Compute the stacking height at a particular segment
//! @param eh IN dimension element
//! @param height OUT stacked height
//! @return SUCCESS if stacking height was computed.
DGNPLATFORM_EXPORT BentleyStatus       GetStackHeight (ElementHandleCR eh, int segmentNo, double& height) const;

//! Compute whether the location of the text is outside the dimension
//! @param eh IN dimension element
//! @return true if text location is outside.
DGNPLATFORM_EXPORT bool                IsTextOutside (ElementHandleCR eh) const;

//! Determine whether the association flag for a given point is set
//! @param eh IN dimension element
//! @param pointNo IN query point;
//! @param flag OUT status.
//! @return SUCCESS if flag status was determined.
DGNPLATFORM_EXPORT BentleyStatus       IsPointAssociative (ElementHandleCR eh, int pointNo, bool& flag) const;

//!  Test whether we can insert a vertex at htipath on a dimension element.
//! @param        dim     IN OUT Dimension element
//! @param        hitPath IN     location
//! @return       status
//! @bsimethod
DGNPLATFORM_EXPORT bool    IsVertexInsertable (ElementHandleCR dim) const;

//!  Test whether we can delete a vertex at htipath on a dimension element.
//! @param        dim     IN OUT Dimension element
//! @param        hitPath IN     location
//! @return       status
//! @bsimethod
DGNPLATFORM_EXPORT bool    IsVertexDeletable (ElementHandleCR dim, HitPathCR hitPath) const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Query the type of a dimension element.
//! @param eh IN dimension element
//! @return dimension type enum
DGNPLATFORM_EXPORT DimensionType       GetDimensionType (ElementHandleCR eh) const;

//! Get an object representing the style of a dimension element.  In general, the properties of
//! the returned style object will differ from the file's version of the same style.
//! @param eh IN dimension element
//! @return dimension style
DGNPLATFORM_EXPORT DimensionStylePtr   GetDimensionStyle (ElementHandleCR eh) const;

//! Query the local overrides from the dimension's style.  If a bit in the returned mask
//! is OFF, then that property is controlled by the dimension's style, so that if the
//! style changes, that change will be propagated to the dimension.  If a bit is ON,
//! then the property is locally controlled by the dimension, and the property will
//! not be effected by style changes.
//! @param eh IN dimension element
//! @return dimension style
DGNPLATFORM_EXPORT DimStylePropMaskPtr GetOverrideFlags (ElementHandleCR eh) const;

//! Query the number of points in dimension element.
//! @param eh IN dimension element
//! @return dimension point count
DGNPLATFORM_EXPORT int                            GetNumPoints (ElementHandleCR eh) const;

//! Query the number of segements present in a dimension element.
//! @param eh IN dimension element
//! @return dimension segment count
DGNPLATFORM_EXPORT int                            GetNumSegments (ElementHandleCR eh) const;

//! Query a point from a dimension element.
//! @param eh       IN    dimension element
//! @param point    OUT   the coordinates of the requested point
//! @param iPoint   IN    index of requested point
//! @return SUCCESS if the point was extracted
DGNPLATFORM_EXPORT BentleyStatus                  ExtractPoint (ElementHandleCR eh, DPoint3dR point, int iPoint) const;

//! Query the height of a dimension element.  The height is the distance along the extension line from the 
//! first measurement point to the dimension line.  All dimensions must contain at least one point in order
//! for the height to be stored, however angular dimensions require at least two points.
//! @param eh       IN    dimension element
//! @param height   OUT   the height
//! @return SUCCESS if the height was extracted
DGNPLATFORM_EXPORT BentleyStatus    GetHeight (ElementHandleCR eh, double& height) const;

//! Query the justification of a text in a dimension element.  All dimensions must contain at least two points
//! for the justification values to be valid.
//! @param eh        IN    dimension element
//! @param segmentNo IN    segment to be queried
//! @param just      OUT   the justification
//! @return SUCCESS if the justification was extracted.
DGNPLATFORM_EXPORT BentleyStatus    GetJustification (ElementHandleCR eh, int segmentNo, DimStyleProp_Text_Justification& just) const;

//! Query the visibility of a witness line in a dimension element.
//! @param eh       IN      dimension element
//! @param pointNo  IN      point Number.
//! @param value    OUT     bool visibilty on or off.
//! @return SUCCESS if the witness line visibility was found.
DGNPLATFORM_EXPORT BentleyStatus    GetWitnessVisibility (ElementHandleCR eh, int pointNo, bool& value) const;

//! Query whether the witness line uses an alternate symbology.
//! @param eh       IN      dimension element
//! @param pointNo  IN      point Number.
//! @param value    OUT     bool use alt symbology on or off.
//! @return SUCCESS if the flag value can be determined.
DGNPLATFORM_EXPORT BentleyStatus    GetWitnessUseAltSymbology (ElementHandleCR eh, int pointNo, bool& value) const;

//! Query for the rotation matrix of a dimension element.
//! @param eh IN dimension element
//! @param rmatrix OUT rotation of the dimension elemnent.
//! @return SUCCESS if the rotation matrix was extracted.
DGNPLATFORM_EXPORT BentleyStatus    GetRotationMatrix (ElementHandleCR eh, RotMatrixR rmatrix) const;

//! Query for existence of a proxy cell element used in a dimension element.
//! @param dimElement IN dimension element
//! @param rotMatrix OUT rotation of the cell element.
//! @param proxyCellId OUT Proxy cell element id.
//! @param origin OUT Origin of the cell element.
//! @return SUCCESS if the proxy cell was found.
DGNPLATFORM_EXPORT BentleyStatus    GetProxyCell (ElementHandleCR dimElement, DgnPlatform::ElementId& proxyCellId, DPoint3dP origin, RotMatrixP rotMatrix) const;

//! Query for the text offset in a dimension element.
//! @param dimElement IN dimension element
//! @param segmentNo IN segment Number element
//! @param offset OUT offset values
DGNPLATFORM_EXPORT BentleyStatus    GetTextOffset (ElementHandleCR dimElement, int segmentNo, DPoint2dR offset) const;

//! Query for the view rotation stored in a dimension element.
//! @param dimElement IN dimension element
//! @param rmatrix OUT view rotation of the dimension element.
DGNPLATFORM_EXPORT BentleyStatus    GetViewRotation (ElementHandleCR dimElement, RotMatrixR rmatrix) const;

//! Get the clockwise sweep flag stored in an angular dimension.
//! @param dimElement IN dimension element
DGNPLATFORM_EXPORT bool             GetAngularDimensionClockWiseSweep (ElementHandleCR dimElement) const;
};

/*=================================================================================**//**
* Modify and Query dimension specific properties of an element.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE IDimensionEdit : IDimensionQuery
{
//__PUBLISH_SECTION_END__
protected:
virtual void                            _ApplyDimensionStyle (EditElementHandleR eeh, DimensionStyleCR dimStyle, bool retainOverrides) = 0;
virtual BentleyStatus                   _SetPoint (EditElementHandleR eeh, DPoint3dCP point, AssocPoint const* assocPt, int pointNo) = 0;
virtual BentleyStatus                   _InsertPoint (EditElementHandleR eeh, DPoint3dCP point, AssocPoint const* assocPt, DimensionStyleCR dimStyle, int pointNo) = 0;
virtual BentleyStatus                   _SetHeight (EditElementHandleR eeh, double height) = 0;
virtual BentleyStatus                   _SetJustification (EditElementHandleR eeh, int segmentNo, DimStyleProp_Text_Justification just) = 0;
virtual BentleyStatus                   _SetWitnessVisibility (EditElementHandleR eeh, int pointNo, bool value) = 0;
virtual BentleyStatus                   _SetRotationMatrix (EditElementHandleR eh, RotMatrixCR rmatrix) = 0;
virtual BentleyStatus                   _DeletePoint (EditElementHandleR dim, int pointNo) = 0;
virtual BentleyStatus                   _InsertVertex (EditElementHandleR dim, HitPathCR hitPath, DPoint3dCR point) = 0;
virtual BentleyStatus                   _DeleteVertex (EditElementHandleR dim, HitPathCR hitPath) = 0;
virtual DgnHandlersStatus               _SetProxyCell (EditElementHandleR dimElement, DgnPlatform::ElementId const& proxyCellId, DPoint3dCR origin, RotMatrixCR rotMatrix) = 0;
virtual BentleyStatus                   _SetTextOffset (EditElementHandleR dimElement, int segmentNo, DPoint2dCR offset) = 0;
virtual BentleyStatus                   _SetViewRotation (EditElementHandleR dimElement, RotMatrixCR rMatrix) = 0;
virtual void                            _SetAngularDimensionClockWiseSweep (EditElementHandleR dimElement, bool value) = 0;
virtual BentleyStatus                   _SetWitnessUseAltSymbology (EditElementHandleR eh, int pointNo, bool value) = 0;
virtual BentleyStatus                   _SetPointsForLabelLine (EditElementHandleR eeh, DSegment3dCP segment, HitPathP hitPath, double offset, RotMatrixCR viewRMatrix, DimensionStyleCR dimStyle) =0;
DECLARE_KEY_METHOD

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Modify the properties of a dimension element to conform to the provided dimension style object.
//! To remove overrides, get the dimension style using DimensionStyle::GetByName and pass false for
//! retainOverrides.  To add new overrides, get the dimension style using IDimensionQuery::GetDimensionStyle
//! and pass true for retain overrides.
//! @param eeh           INOUT dimension element
//! @param dimStyle         IN dimension style
//! @param retainOverrides  IN if false, all existing overrides are cleared from the dimension.
//! @return SUCCESS or ERROR
DGNPLATFORM_EXPORT void              ApplyDimensionStyle (EditElementHandleR eeh, DimensionStyleCR dimStyle, bool retainOverrides);

//! Insert a new point into a dimension element.
//! @param eeh        INOUT dimension element
//! @param point         IN point coordinates
//! @param assocPt       IN if assocPt is provided then point is optional
//! @param iPoint        IN point index
//! @return SUCCESS if the point was successfully changed.
DGNPLATFORM_EXPORT BentleyStatus     SetPoint (EditElementHandleR eeh, DPoint3dCP point, AssocPoint const* assocPt, int iPoint);

//! Insert a new point into a dimension element.
//! @param eeh        INOUT dimension element
//! @param point         IN point coordinates
//! @param assocPt       IN if assocPt is provided then point is optional
//! @param dimStyle      IN the style is used specify point specific properties like extension line visibility and justification
//! @param iPoint        IN insertion index, pass -1 to append at the end
//! @return SUCCESS if the point was successfully inserted.
DGNPLATFORM_EXPORT BentleyStatus     InsertPoint (EditElementHandleR eeh, DPoint3dCP point, AssocPoint const* assocPt, DimensionStyleCR dimStyle, int iPoint);

//! Modify the height of a dimension element.  The height is the distance along the extension line from the 
//! first measurement point to the dimension line.  All dimensions must contain at least one point in order
//! for the height to be stored, however angular dimensions require at least two points.
//! @param eeh    INOUT   dimension element
//! @param height   OUT   the height
//! @return SUCCESS if the height could be changed
DGNPLATFORM_EXPORT BentleyStatus     SetHeight (EditElementHandleR eeh, double height);

//!  Modify the justification of a segment in a dimension element.
//! @param eeh      INOUT    dimension element
//! @param segmentNo IN    segment Number
//! @param just      IN   the justification
//! @return SUCCESS if the justification was set.
DGNPLATFORM_EXPORT BentleyStatus    SetJustification (EditElementHandleR eeh, int segmentNo, DimStyleProp_Text_Justification just);

//! Modify the visibility of a witness line in a dimension element.
//! @param eeh      INOUT   dimension element
//! @param pointNo  IN      point Number.
//! @param value    IN      bool visibilty on or off.
//! @return SUCCESS if the witness line visibility was changed.
DGNPLATFORM_EXPORT BentleyStatus    SetWitnessVisibility (EditElementHandleR eeh, int pointNo, bool value);

//! Modify whether the witness line uses an alternate symbology.
//! @param eh      INOUT    dimension element
//! @param pointNo  IN      point Number.
//! @param value    IN      bool use alt symbology on or off.
//! @return SUCCESS if the flag value can be determined.
DGNPLATFORM_EXPORT BentleyStatus    SetWitnessUseAltSymbology (EditElementHandleR eh, int pointNo, bool value);

//! Modify the rotation matrix of a dimension element.
//! @param eeh IN dimension element
//! @param rmatrix IN orientation of the dimension elemnent.
//! @return SUCCESS if the rotation matrix was extracted.
DGNPLATFORM_EXPORT BentleyStatus    SetRotationMatrix (EditElementHandleR eeh, RotMatrixCR rmatrix);

//!  Deletes the vertex at pointNo from a dimension element.
//! @param        dim     IN OUT Dimension element
//! @param        pointNo IN     Point to delete
//! @return       SUCCESS if the point is deleted
//! @bsimethod
DGNPLATFORM_EXPORT BentleyStatus    DeletePoint (EditElementHandleR dim, int pointNo);

//! Set the proxy cell element used in a dimension element.
//! @param dimElement IN dimension element
//! @param rotMatrix IN rotation of the cell element.
//! @param proxyCellId IN element ID of the cell element.
//! @param origin IN origin of the cell element.
//! @return SUCCESS if the proxy cell was set.
DGNPLATFORM_EXPORT DgnHandlersStatus SetProxyCell (EditElementHandleR dimElement, DgnPlatform::ElementId const& proxyCellId, DPoint3dCR origin, RotMatrixCR rotMatrix);

//! Set text offset in a dimension element.
//! @param dimElement IN dimension element
//! @param segmentNo IN segment Number element
//! @param offset IN offset values
DGNPLATFORM_EXPORT BentleyStatus    SetTextOffset (EditElementHandleR dimElement, int segmentNo, DPoint2dCR offset);

//! Set view rotation stored in a dimension element.
//! @param dimElement IN dimension element
//! @param rMatrix IN view rotation of the dimension element.
DGNPLATFORM_EXPORT BentleyStatus    SetViewRotation (EditElementHandleR dimElement, RotMatrixCR rMatrix);

//! Set the clockwise sweep flag stored in an angular dimension.
//! @param dimElement IN dimension element
//! @param value IN value to set 
DGNPLATFORM_EXPORT void             SetAngularDimensionClockWiseSweep (EditElementHandleR dimElement, bool value);

//! Insert a new point into a dimension element.
//! @param eeh        INOUT dimension element
//! @param segment       IN Segment points.  Optional if hitPath is non-NULL.
//! @param hitPath       IN Used to create an association on the dimension.  Only provide if you will be calling AddToModel.
//! @param offset        IN Distance along the line at which to position the label.
//! @param viewRMatrix   IN View orientation
//! @param dimStyle      IN Used to specify point specific properties like extension line visibility and justification.
//! @return SUCCESS if the points were successfully set.
DGNPLATFORM_EXPORT BentleyStatus     SetPointsForLabelLine (EditElementHandleR eeh, DSegment3dCP segment, HitPathP hitPath, double offset, RotMatrixCR viewRMatrix, DimensionStyleCR dimStyle);

//__PUBLISH_SECTION_END__

//!  Insert a vertex at htipath on a dimension element.
//! @param        dim     IN OUT Dimension element
//! @param        hitPath IN     location
//! @return       SUCCESS if the point is inserted
//! @bsimethod
DGNPLATFORM_EXPORT BentleyStatus    InsertVertex (EditElementHandleR dim, HitPathCR hitPath, DPoint3dCR point);

//!  Deletes the vertex at hitpath on a dimension element.
//! @param        dim     IN OUT Dimension element
//! @param        pointNo IN     location
//! @return       SUCCESS if the point is deleted
//! @bsimethod
DGNPLATFORM_EXPORT BentleyStatus    DeleteVertex (EditElementHandleR dim, HitPathCR hitPath);

//__PUBLISH_SECTION_START__

};

#if defined (NEEDS_WORK_DGNITEM)
/*=================================================================================**//**
//!  While querying a dimension using ITextEdit and ITextQuery the returned
//! object is a DimensionTextPartId.
* @bsiclass                                                     Jeff.Marker     08/2009
+===============+===============+===============+===============+===============+======*/
struct DimensionTextPartId : public ITextPartId
    {
    //__PUBLISH_SECTION_END__
    private:
    UInt32                   m_partSegment;
    DimensionTextPartType    m_partType;
    DimensionTextPartSubType m_partSubType;
    
    DimensionTextPartId (UInt32 partSegment, DimensionTextPartType partType, DimensionTextPartSubType partSubType);

    public:
    //__PUBLISH_SECTION_START__

        //!  Queries the segment associated with this part.
        DGNPLATFORM_EXPORT UInt32                       GetPartSegment  () const;
        //!  Queries the part type associated with this part.
        DGNPLATFORM_EXPORT DimensionTextPartType        GetPartType     () const;
        //!  Queries the part sub type associated with this part.
        DGNPLATFORM_EXPORT DimensionTextPartSubType     GetPartSubType  () const;

    public: 
        //!  Create a new dimension text part.  Will fail if partType or partSubType are not valid text parts.
        //! @param        partSegment IN  segment number
        //! @param        partType    IN  part type
        //! @param        partSubType IN  part subtype
        DGNPLATFORM_EXPORT static ITextPartIdPtr Create (UInt32 partSegment, DimensionPartType partType, DimensionPartSubType partSubType);

        //!  Create a new dimension text part.
        //! @param        partSegment IN  segment number
        //! @param        partType    IN  part type
        //! @param        partSubType IN  part subtype
        DGNPLATFORM_EXPORT static ITextPartIdPtr Create (UInt32 partSegment, DimensionTextPartType partType, DimensionTextPartSubType partSubType);
    
    }; // DimensionTextPartId

/** @addtogroup DisplayHandler */
/** @beginGroup */


/*=================================================================================**//**
* The default type handler for Dimension elements (type: DIMENSION_ELM).
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE DimensionHandler :  DisplayHandler,
                                    IDimensionEdit,
                                    //ITransactionHandler,  removed in Graphite
                                    IAnnotationHandler, 
                                    ITextEdit
{
    DEFINE_T_SUPER(DisplayHandler)    
    ELEMENTHANDLER_DECLARE_MEMBERS (DimensionHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
private:
friend struct  DimFormattedText;


bool    _ProcessPropertiesHelper (EditElementHandleP elmP, ElementHandleCR , PropertyContextR context);
void    _ProcessProperties (PropertyContextR, ElementHandleCR, EditElementHandleP);

int     TransformDimensionEx
(
EditElementHandleR  elemHandle,     /* <=> Dimension element to be transformed    */
Transform const*    ct,             /*  => composite transform                    */
DgnModelP        sourceDgnModel,
DgnModelP        modelRef,
bool                dontScaleSize,
bool                scaleValue,
bool                rotateDimView
);

BentleyStatus   UpdateModelAnnotationScale (EditElementHandleR  element, ChangeAnnotationScale& changeContextIn);

protected:

//virtual ITransactionHandlerP                        _GetITransactionHandler() override {return this;} removed in Graphite
virtual IAnnotationHandlerP                         _GetIAnnotationHandler (ElementHandleCR)  override {return this;}

DGNPLATFORM_EXPORT virtual void                     _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual void                     _QueryProperties (ElementHandleCR eh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void                     _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;

DGNPLATFORM_EXPORT virtual bool                     _IsTransformGraphics (ElementHandleCR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt                _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt                _OnChangeOfUnits (EditElementHandleR, DgnModelP source, DgnModelP dest) override;

DGNPLATFORM_EXPORT virtual StatusInt                _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual StatusInt                _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;
DGNPLATFORM_EXPORT virtual StatusInt                _OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) override;

DGNPLATFORM_EXPORT virtual void                     _Draw (ElementHandleCR, ViewContextR) override;
DGNPLATFORM_EXPORT virtual void                     _DrawFiltered (ElementHandleCR, ViewContextR, DPoint3dCP, double size) override;
DGNPLATFORM_EXPORT virtual bool                     _IsPlanar (ElementHandleCR, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal) override;
DGNPLATFORM_EXPORT virtual void                     _GetElemDisplayParams (ElementHandleCR, ElemDisplayParams&, bool wantMaterials = false) override;

DGNPLATFORM_EXPORT virtual void                     _GetOrientation (ElementHandleCR elHandle, RotMatrixR orientation) override;

//DGNPLATFORM_EXPORT virtual PreActionStatus          _OnReplace (EditElementHandleR eeh, ElementHandleCR eh) override; removed in Graphite

// ITextEdit
DGNPLATFORM_EXPORT virtual ITextPartIdPtr           _GetTextPartId      (ElementHandleCR, HitPathCR) const override;
DGNPLATFORM_EXPORT virtual void                     _GetTextPartIds     (ElementHandleCR, ITextQueryOptionsCR, T_ITextPartIdPtrVectorR) const override;
DGNPLATFORM_EXPORT virtual TextBlockPtr             _GetTextPart        (ElementHandleCR, ITextPartIdCR) const override;
DGNPLATFORM_EXPORT virtual ReplaceStatus            _ReplaceTextPart    (EditElementHandleR, ITextPartIdCR, TextBlockCR) override;

DGNPLATFORM_EXPORT virtual ReprojectStatus          _OnGeoCoordinateReprojection (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;

//DGNPLATFORM_EXPORT virtual StatusInt                _OnPreprocessCopy (EditElementHandleR symbolEH, ElementCopyContextP ccP) override; removed in graphite
DGNPLATFORM_EXPORT virtual void                     _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void                     _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;
DGNPLATFORM_EXPORT virtual bool                     _GetAnnotationScale (double* annotationScale, ElementHandleCR element) const override;
DGNPLATFORM_EXPORT virtual StatusInt                _ApplyAnnotationScaleDifferential (EditElementHandleR, double scale) override;

// IDimensionQuery
DGNPLATFORM_EXPORT virtual int                      _GetNumPoints (ElementHandleCR) const override;
DGNPLATFORM_EXPORT virtual int                      _GetNumSegments (ElementHandleCR eh) const override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _ExtractPoint (ElementHandleCR, DPoint3dR, int iPoint) const override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _GetHeight (ElementHandleCR eh, double& height) const override;
DGNPLATFORM_EXPORT virtual DimensionStylePtr        _GetDimensionStyle (ElementHandleCR eh) const override;
DGNPLATFORM_EXPORT virtual DimStylePropMaskPtr      _GetOverrideFlags (ElementHandleCR eh) const override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _GetJustification (ElementHandleCR eh, int segmentNo, DimStyleProp_Text_Justification& just) const override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _GetWitnessVisibility (ElementHandleCR eh, int pointNo, bool& value) const override;
DGNPLATFORM_EXPORT virtual DimensionType           _GetDimensionType (ElementHandleCR eh) const override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _GetStackHeight (ElementHandleCR eh, int segmentNo, double& height) const override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _GetRotationMatrix (ElementHandleCR eh, RotMatrixR rmatrix) const override;
DGNPLATFORM_EXPORT virtual bool                     _IsTextOutside (ElementHandleCR eh) const override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _IsPointAssociative (ElementHandleCR eh, int pointNo, bool& flag) const override;
DGNPLATFORM_EXPORT virtual bool                     _IsVertexInsertable (ElementHandleCR dim) const override;
DGNPLATFORM_EXPORT virtual bool                     _IsVertexDeletable (ElementHandleCR dim, HitPathCR hitPath) const override;
// IDimensionEdit
DGNPLATFORM_EXPORT virtual void                     _ApplyDimensionStyle (EditElementHandleR eeh, DimensionStyleCR dimStyle, bool retainOverrides) override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _SetPoint (EditElementHandleR eeh, DPoint3dCP point, AssocPoint const* assocPt, int pointNo) override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _InsertPoint (EditElementHandleR eeh, DPoint3dCP point, AssocPoint const* assocPt, DimensionStyleCR dimStyle, int pointNo) override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _SetHeight (EditElementHandleR eeh, double height) override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _SetJustification (EditElementHandleR eeh, int segmentNo, DimStyleProp_Text_Justification just) override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _SetWitnessVisibility (EditElementHandleR eeh, int pointNo, bool value) override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _SetRotationMatrix (EditElementHandleR eh, RotMatrixCR rmatrix) override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _DeletePoint (EditElementHandleR dim, int pointNo) override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _SetPointsForLabelLine (EditElementHandleR eeh, DSegment3dCP segment, HitPathP hitPath, double offset, RotMatrixCR viewRMatrix, DimensionStyleCR dimStyle) override;

DGNPLATFORM_EXPORT virtual BentleyStatus            _InsertVertex (EditElementHandleR dim, HitPathCR hitPath, DPoint3dCR point) override;
DGNPLATFORM_EXPORT virtual BentleyStatus            _DeleteVertex (EditElementHandleR dim, HitPathCR hitPath) override;

DGNPLATFORM_EXPORT virtual BentleyStatus            _GetProxyCell (ElementHandleCR dimElement, DgnPlatform::ElementId& proxyCellId, DPoint3dP origin, RotMatrixP rotMatrix) const override;
DGNPLATFORM_EXPORT virtual DgnHandlersStatus        _SetProxyCell (EditElementHandleR dimElement, DgnPlatform::ElementId const& proxyCellId, DPoint3dCR origin, RotMatrixCR rotMatrix) override;

DGNPLATFORM_EXPORT virtual BentleyStatus            _GetTextOffset (ElementHandleCR dimElement, int segmentNo, DPoint2dR offset) const;
DGNPLATFORM_EXPORT virtual BentleyStatus            _SetTextOffset (EditElementHandleR dimElement, int segmentNo, DPoint2dCR offset);

DGNPLATFORM_EXPORT virtual BentleyStatus            _GetViewRotation (ElementHandleCR dimElement, RotMatrixR rmatrix) const;
DGNPLATFORM_EXPORT virtual BentleyStatus            _SetViewRotation (EditElementHandleR dimElement, RotMatrixCR rMatrix);

DGNPLATFORM_EXPORT virtual bool                     _GetAngularDimensionClockWiseSweep (ElementHandleCR dimElement) const;
DGNPLATFORM_EXPORT virtual void                     _SetAngularDimensionClockWiseSweep (EditElementHandleR dimElement, bool value);

DGNPLATFORM_EXPORT virtual BentleyStatus            _GetWitnessUseAltSymbology (ElementHandleCR eh, int pointNo, bool& value) const;
DGNPLATFORM_EXPORT virtual BentleyStatus            _SetWitnessUseAltSymbology (EditElementHandleR eh, int pointNo, bool value);

public:
BentleyStatus   DropAssociation (EditElementHandleR dimElement, int pointNo);
StatusInt       ScaleElement (EditElementHandleR element, double scale, bool allowModify, bool setSheilds, bool updateRange);

DGNPLATFORM_EXPORT void                             SetShieldsFromStyle (EditElementHandleR eeh, DimensionStyleCR dimStyle);//For mdlDim_setShieldsFromStyle
DGNPLATFORM_EXPORT void                             SaveShieldsDirect (EditElementHandleR eeh, DimStylePropMaskR propMask);

DGNPLATFORM_EXPORT void                             ExtractDimStyle (DimensionStyleR style, ElementHandleCR eh) const;
DGNPLATFORM_EXPORT void                             UpdateFromDimStyle (EditElementHandleR eh, DimensionStyleCR style, IDimCreateDataCP createData, int option);
DGNPLATFORM_EXPORT BentleyStatus                    InsertPointDirect (EditElementHandleR eeh, DPoint3dCP point, AssocPoint const* assocPt, DimText const& dimText, int pointNo);

DGNPLATFORM_EXPORT void             SetAlignment (EditElementHandleR eeh, DimensionAlignment alignment);
DGNPLATFORM_EXPORT DimensionAlignment GetAlignment (ElementHandleCR eh) const;

DGNPLATFORM_EXPORT StatusInt        DropDimensionToSegments (ElementAgendaR droppedDimension, ElementHandleCR dimension);
DGNPLATFORM_EXPORT StatusInt        InsertStrings (EditElementHandleR eeh, DimStrings const&, int pointNo);
DGNPLATFORM_EXPORT StatusInt        GetStrings (ElementHandleCR eeh, DimStrings&, int pointNo, DimStringConfig* stringConfig);
DGNPLATFORM_EXPORT StatusInt        OverallSetRefScale (EditElementHandleR dimElement, double* value);
DGNPLATFORM_EXPORT StatusInt        OverallSetAngleQuadrant (EditElementHandleR dimElement, UInt16* value);
DGNPLATFORM_EXPORT void             SetRadialDimensionCrossCenterFlag (EditElementHandleR dimElement, bool value);

DGNPLATFORM_EXPORT StatusInt        SetTextFromDescr (EditElementHandleR dimElement, ElementHandleCR textElementIn, bool bHasValuePlaceHolder, int iSegmentNo, int iPartType, int iSubType);
DGNPLATFORM_EXPORT StatusInt        GetTextDescr (ElementHandleCR dimElement, EditElementHandleR textElement, int iSegmentNo, int iPartType, int iSubType) const;
DGNPLATFORM_EXPORT StatusInt        GetTextStyle (ElementHandleCR dimElement, MdlTextStyle* pTextStyleOut) const;
DGNPLATFORM_EXPORT StatusInt        SetTextStyle  (EditElementHandleR dimElm, const MdlTextStyle*  pTextStyle, bool setOverrideFlags, bool sizeChangeAllowed);
DGNPLATFORM_EXPORT StatusInt        GetTextStyleID (ElementHandleCR dimElement, UInt32* pTextStyleID) const;
DGNPLATFORM_EXPORT StatusInt        SetTextStyleID (EditElementHandleR eh, UInt32 textStyleID);

DGNPLATFORM_EXPORT void             ChangeFont (EditElementHandleR dimElement, UInt32 font, UInt32 bigFont);

//__PUBLISH_SECTION_START__

//! Utility method to get the point number from a given segment number.
DGNPLATFORM_EXPORT static BentleyStatus GetTextPointNo(int& pointNo, ElementHandleCR element, int segmentNo);

//! Create a new dimension element with the supplied properties.
//! @param eeh          OUT   the new element
//! @param createData   IN    properties used to create the element
//! @param dimType      IN    the type of dimension to create
//! @param is3d         IN    pass true to create a 3d element
//! @param modelRef     IN    model with which to associate this element
//! @return SUCCESS if the element was created
DGNPLATFORM_EXPORT  static BentleyStatus CreateDimensionElement (EditElementHandleR eeh, IDimCreateDataCR createData, DimensionType dimType, bool is3d, DgnModelR modelRef);

};
#endif
/** @endGroup */

END_BENTLEY_DGNPLATFORM_NAMESPACE
#endif

/** @endcond */
