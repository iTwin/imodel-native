/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/AssociativePoint.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* Create and query associative points. An associative point defines a location that
* is evaluated from target/root element(s). An element that supports associative points
* has a dependency callback that is responsible for updating the element when the
* root elements change.
* The following types support an associative point for their origin.
* \li CELL_HEADER_ELM
* \li SHARED_CELL_ELM
* <p>
* The following types support associative points for their points/vertices.
* \li DIMENSION_ELM
* \li MULTILINE_ELM
* \li LINE_ELM
* \li LINE_STRING_ELM
* \li SHAPE_ELM
* \li POINT_STRING_ELM
* \li BSPLINE_POLE_ELM
* \li CURVE_ELM
* <p>
* @bsiclass                                                     Sam.Wilson      12/2008
+===============+===============+===============+===============+===============+======*/
struct AssociativePoint
{
enum CreateOptions
    {
    CREATE_ASSOC_Normal              = 0, //! Allow far associations and custom associations.
    CREATE_ASSOC_DisallowFarElm      = 1, //! Disallow the creation of far associations.
    CREATE_ASSOC_DisallowCustom      = 2, //! Disallow the creation of custom associations.
    CREATE_ASSOC_AllowAdjustedPoints = 3, //! Allow association to adjusted snap point (dimensions only).
    };

enum CreateMask
    {
    CREATE_ASSOC_MASK_DIMENSION = (1),
    CREATE_ASSOC_MASK_MLINE     = (1 << 1),
    CREATE_ASSOC_MASK_CELLS     = (1 << 2),
    CREATE_ASSOC_MASK_LINEAR    = (1 << 3),
    CREATE_ASSOC_MASK_NOTE      = (1 << 4),
    CREATE_ASSOC_MASK_TEXT      = (1 << 5),
    };

enum ArcLocation : UShort
    {
    ARC_ANGLE   = 0,
    ARC_CENTER  = 1,
    ARC_START   = 2,
    ARC_END     = 3,
    };

//__PUBLISH_SECTION_END__

DGNPLATFORM_EXPORT static WString        FormatAssocPointString (AssocPoint const& assocPoint, bool appendIds = false);
DGNPLATFORM_EXPORT static void           DropAssociationsToReferences (DgnModelP masterModel, ElementId* attachmentIds, size_t nAttachmentIds);
DGNPLATFORM_EXPORT static void           InValidateRoots (AssocPoint& assoc);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus HandleTopologyChange
(
bool*           changedP,       // <= if assoc point was changed
DPoint3dP       newPointP,      // <= Point to look for
DPoint3dCP      oldPointP,      // => Point to look for
AssocPoint&     assocPoint,     // => Association information
DgnModelP    modelRef        // => source of element
);

/*---------------------------------------------------------------------------------**//**
* Adjust association point indices for the specified element.
* @Param        element  IN  element to add association information to
* @Param        pointNum IN     index of point added/removed
* @Param        maxPoints IN    max assoc pts on element
* @Param        inserted IN     pass true is vertex has been added; false if it's been removed
* @Return       SUCCESS if the information is found, ERROR if an error occurs
* @see    ExtractPoint
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static StatusInt VertexAddedOrRemoved
(
EditElementHandleR element,
int             pointNum,
int             maxPoints,
bool            inserted
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static StatusInt VertexAddedOrRemovedFromMSElement
(
DgnElementR     element,
DgnModelR       model,
int             pointNum,
int             maxPoints,
bool            inserted
);

/*---------------------------------------------------------------------------------**//**
* Uses the element and point number to create a 3D data point that is returned in outPoint. An AssocCreate function must create the information in assocPoint.
* @Param        outPoint OUT    the point created by the function.
* @Param        element    IN     element to get the point from.
* @Param        pointNum IN     point number to get.
* @Param        maxPoints IN    total number of points on element.
* @Param        modelRef IN     the model that contains the element and the association.
* @Return SUCCESS (zero) if a valid point is created in dPoint. If the information in assocPoint is invalid, the function returns a non-zero value.
* @see    GetPoint
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static StatusInt GetPointFromElement
(
DPoint3dP       outPoint,
ElementHandleCR element,
int             pointNum,
int             maxPoints,
DgnModelP    modelRef
);

/*---------------------------------------------------------------------------------**//**
* Define an DEPENDENCY_DATA_TYPE_ASSOC1_I dependency linkage root to capture the state of the specified AssocPoint.
* @param dl             OUT the dependency linkage to write to
* @param iRoot          IN index into the dl roots array to write to
* @param assocPoint     IN the input AssocPoint
* @param assoc1Subtype  IN the assocPoint subtype to define. Should be a DEPENDENCY_ASSOC1_SUBTYPE_ value
* @returns non-zero error code if the assocPoint cannot be captured by an DEPENDENCY_DATA_TYPE_ASSOC1_I root.
* @See DependencyLinkage
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static StatusInt    DefineAssoc1FromAssocAndSubtype
(
DependencyLinkage&  dl,         // <=
int                 iRoot,
AssocPoint const&   assocPoint,
int                 assoc1Subtype
);

/*---------------------------------------------------------------------------------**//**
* Define an DEPENDENCY_DATA_TYPE_ASSOC1_I dependency linkage root to capture the state of the specified AssocPoint.
* @param dl             OUT the dependency linkage to write to
* @param iRoot          IN index into the dl roots array to write to
* @param assocPoint     IN the input AssocPoint
* @returns non-zero error code if the assocPoint cannot be captured by an DEPENDENCY_DATA_TYPE_ASSOC1_I root.
* @See DependencyLinkage
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static StatusInt    DefineAssoc1FromAssoc
(
DependencyLinkage&  dl,         // <=
int                 iRoot,
AssocPoint const&   assocPoint
);

/*---------------------------------------------------------------------------------**//**
* Recreate an AssocPoint from an DEPENDENCY_DATA_TYPE_ASSOC1_I root.
* @param assocPoint     OUT the AssocPoint to write to
* @param dl             IN the input dependency linkage
* @param iRoot          IN index into the dl roots array
* @returns non-zero error code if linkage does not contain DEPENDENCY_DATA_TYPE_ASSOC1_I roots.
* @See DependencyLinkage
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static StatusInt GetAssocFromAssoc1
(
AssocPoint&                 assocPoint,     // <=
DependencyLinkage const&    dl,
int                         iRoot
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus CustomGetDataFromPathElement
(
byte*&              data,
UInt32&             size,
ElementHandleCR        pathEh,
AssocPoint const&   assoc
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus GetAssocPointSegment
(
DSegment3dR         dVec,
AssocPoint&         assocPoint,
DgnModelP        parentModel
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus GetTargetGeometry
(
EditElementHandleR     eeh,
TransformP          pathTransP,
DisplayPathCP        path,
DgnModelP        modelRef
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus GetTargetGeometry
(
EditElementHandleR     eeh,
TransformP          pathTransP,
AssocPoint&         assoc,
int                 iRoot,
DgnModelP        modelRef
);

/*---------------------------------------------------------------------------------**//**
* Returns the roots of an assoc point.
* @Param        elemRefP        OUT extracted root element ref.
* @Param        elemDgnModelP   OUT model ref for the extracted root elem ref.
* @Param        elemTransformP  OUT transform applied to root element ref.
* @Param        nRootsP         OUT number of roots in this assoc, 1 or 2.
* @Param        assoc       IN  assoc point to extract the root from.
* @Param        modelRef    IN  the model that contains the assoc point.
* @Param        rootIndex   IN  which root to extract, 0 or 1 (intersect assoc has 2 roots)
* @Return SUCCESS (zero) if the requested root is extracted.
* @see    GetPoint
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static StatusInt GetRoot
(
ElementRefP*     elemRefP,
DgnModelP*   elemDgnModelP,
TransformP      elemTransformP,
int*            nRootsP,
AssocPoint&     assoc,
DgnModelP    modelRef,
int             rootIndex = 0
);

//__PUBLISH_SECTION_START__

//! Test if the associative point evaluates to a valid location.
//! @param assocPoint IN The associative point to check.
//! @param pathRoot IN The model that contains an associative point root element (can use either root for intersection).
//! @param parentModel IN The model that contains the associative point dependent element.
//! @return SUCCESS if assoc point data and roots are valid.
DGNPLATFORM_EXPORT static BentleyStatus IsValid
(
AssocPoint&     assocPoint,
DgnModelP    pathRoot,
DgnModelP    parentModel
);

//! Complete setup of a new associative point by setting the target/root element for the dependency by a pair of element ids.
//! @param assocPoint IN The associative point being created.
//! @param elemId IN ElementId of the target/root of the associative point dependency.
//! @param rootIndex IN Which root to set, an intersection association requires 2 roots.
//! @return SUCCESS if assoc point root can be set.
//! @note Must call one of the various Init methods before setting roots, ex. InitKeypoint.
DGNPLATFORM_EXPORT static BentleyStatus SetRoot
(
AssocPoint&     assocPoint,
ElementId       elemId,
int             rootIndex = 0
);

//! Complete setup of a new associative point by setting the target/root element for the dependency from a DisplayPath.
//! @param assocPoint IN The associative point being created.
//! @param path IN A DisplayPath to the target/root element to use for the associative point dependency.
//! @param parentModel IN The model that contains the associative point dependent element.
//! @param allowFarElm IN Allow the creation of associative points to nested references and nested shared cells.
//! @param rootIndex IN Which root to set, an intersection association requires 2 roots.
//! @return SUCCESS if assoc point root can be set.
//! @note Must call one of the various Init methods before setting roots, ex. InitKeypoint.
//! @see DisplayPath::GetHitElem
DGNPLATFORM_EXPORT static BentleyStatus SetRoot
(
AssocPoint&     assocPoint,
DisplayPathCP   path,
DgnModelP    parentModel,
bool            allowFarElm,
int             rootIndex = 0
);

//! Populate a seed DisplayPath for a root of an associative point dependency.
//! @param path IN OUT The seed DisplayPath that is to be populated.
//! @param nRootsP OUT The number of roots for this associative point, an intersection association will return 2, all other types return 1.
//! @param assocPoint IN The associative point to query.
//! @param modelRef IN The model that contains the associative point dependent element.
//! @param rootIndex IN Which root to get, an intersection association has 2 roots.
//! @return A DisplayPath to associative point root or NULL. Caller needs to call Release on path.
DGNPLATFORM_EXPORT static BentleyStatus GetRoot
(
DisplayPathP        path,           // <=> seed display path
int*                nRootsP,        // <=  intersect assoc will return 2, all others return 1
AssocPoint const&   assocPoint,     //  => assoc point to build a displaypath for
DgnModelP        modelRef,       //  => model ref of element this assoc is from
int                 rootIndex = 0   //  => which root to get (Use 0 for single elem assoc
);

//! Initialize the data in the associative point for an association that represents a point on a linear element.
//! @param assocPoint OUT The associative point to setup.
//! @param vertex IN The index of the vertex directly preceding the association point.
//! @param nVertex IN The total number of vertices of the linear element. This is optional, but if supplied the association can be adjusted for insert/delete vertex on the root/target.
//! @param numerator IN The distance from vertex number vertex in units of divisor as described below. Its range must be between 0 and 32767.
//! @param divisor IN The number of units (segments) to be considered between the points at vertex and vertex+1. The values of numerator and divisor are
//!     used together as the fraction of the distance between the points at vertex and vertex+1, where the association point will be located. The
//!     denominator must be between 1 and 32767.
DGNPLATFORM_EXPORT static void InitKeypoint
(
AssocPoint&     assocPoint,
UShort          vertex,
UShort          nVertex,
UShort          numerator,
UShort          divisor
);

//! Initialize the data in the associative point for an association that represents the origin of an element.
//! @param assocPoint OUT The associative point to setup.
//! @param option IN Origin association option flag.
//! @see DisplayHandler::GetSnapOrigin
DGNPLATFORM_EXPORT static void InitOrigin
(
AssocPoint&     assocPoint,
UShort          option
);

//! Initialize the data in the associative point for an association that represents a point on an arc or ellipse element.
//! @param assocPoint OUT The associative point to setup.
//! @param keyPoint IN The location on the arc to create the association to.
//! \li ARC_ANGLE angle radians from the primary axis
//! \li ARC_CENTER center of the arc or ellipse
//! \li ARC_START  arc start point
//! \li ARC_END arc end point
//! <p>
//! @param angle IN The angle (radians) from the primary axis of the arc or ellipse to the association point. This argument is used only when the value of <em>keyPoint</em> is <PRE>ARC_ANGLE</PRE>.
DGNPLATFORM_EXPORT static void InitArc
(
AssocPoint&     assocPoint,
ArcLocation     keyPoint,
double          angle = 0.0
);

//! Initialize the data in the associative point for an association that represents a point on a linear element.
//! @param assocPoint OUT The associative point to setup.
//! @param vertex IN The index of the vertex directly preceding the association point.
//! @param nVertex IN The total number of vertices of the linear element. This is optional, but if supplied the association can be adjusted for insert/delete vertex on the root/target.
//! @param ratio IN Fraction parameter along segment defined by vertex number.
DGNPLATFORM_EXPORT static void InitProjection
(
AssocPoint&     assocPoint,
UShort          vertex,
UShort          nVertex,
double          ratio
);

//! Initialize the data in the associative point for an association that represents a point on a multiline element.
//! @param assocPoint OUT The associative point to setup.
//! @param vertex IN The index of the vertex directly preceding the association point.
//! @param nVertex IN The total number of vertices of the multiline element. This is optional, but if supplied the association can be adjusted for insert/delete vertex on the root/target multiline.
//! @param lineNo IN Indicates which profile line in the multiline the association is to.
//! @param offset IN The distance from the specified vertex to the association point, measured along the work line and divided by the work line length. Not used if joint is true.
//! @param joint IN If true, the association point is at the intersection of the line specified by lineNo and the joint bvector at vertex. In other words, the association point will always be on the joint.
DGNPLATFORM_EXPORT static void InitMline
(
AssocPoint&     assocPoint,
UShort          vertex,
UShort          nVertex,
UShort          lineNo,
double          offset,
bool            joint
);

//! Initialize the data in the associative point for an association that represents a point on a bspline curve.
//! @param assocPoint OUT The associative point to setup.
//! @param uParam IN The parameter along the B-spline curve representing the associative point.
DGNPLATFORM_EXPORT static void InitBCurve
(
AssocPoint&     assocPoint,
double          uParam
);

//! Initialize the data in the associative point for an association that represents a point on a bspline surface.
//! @param assocPoint OUT The associative point to setup.
//! @param uParam IN The u parameter along the B-spline surface representing the associative point.
//! @param vParam IN The v parameter along the B-spline surface representing the associative point.
DGNPLATFORM_EXPORT static void InitBSurface
(
AssocPoint&     assocPoint,
double          uParam,
double          vParam
);

//! Initialize the data in the associative point for an association that represents a point on a mesh edge.
//! @param assocPoint OUT The associative point to setup.
//! @param edgeIndex IN The index of the edge for the association point.
//! @param nEdge IN The total number of edges of the mesh element.
//! @param uParam IN The parameter along the mesh edge representing the associative point.
DGNPLATFORM_EXPORT static void InitMeshEdge
(
AssocPoint&     assocPoint,
int             edgeIndex,
int             nEdge,
double          uParam
);

//! Initialize the data in the associative point for an association that represents a mesh vertex.
//! @param assocPoint OUT The associative point to setup.
//! @param vertexIndex IN he index of the vertex for the association point.
//! @param nVertex IN The total number of vertices of the mesh element.
DGNPLATFORM_EXPORT static void InitMeshVertex
(
AssocPoint&     assocPoint,
int             vertexIndex,
int             nVertex
);

//! Initialize the data in the associative point for an association that represents the intersection of 2 curve paths.
//! @param assocPoint OUT The associative point to setup.
//! @param index IN The index of the intersection between the two elements where the association will be created. The number of intersections between two elements can be found via the mdlIntersect_allBetweenElms function.
//! @param seg1 IN The index of the segment of element 1 where the intersection occurs.
//! @param seg2 IN The index of the segment of element 2 where the intersection occurs.
//! @param nSeg1 IN The number of vertices of element 1. This parameter is optional, you can pass zero. If non-zero this value is used to avoid the association jumping as vertices are inserted/deleted.
//! @param nSeg2 IN The number of vertices of element 2. This parameter is optional, you can pass zero. If non-zero this value is used to avoid the association jumping as vertices are inserted/deleted.
DGNPLATFORM_EXPORT static void InitIntersection
(
AssocPoint&     assocPoint,
byte            index,
UShort          seg1,
UShort          seg2,
int             nSeg1,
int             nSeg2
);

//! Extracts the association information from the specified point on the given element.
//! @param assocPoint OUT The associative point to query.
//! @param element IN The element to extract the point from.
//! @param pointNum IN The index of point to extract.
//! @param maxPoints IN The maximum number of associative points on the element.
//! @return SUCCESS if associative point was extracted.
DGNPLATFORM_EXPORT static StatusInt ExtractPoint
(
AssocPoint&     assocPoint,
ElementHandleCR    element,
int             pointNum,
int             maxPoints
);

//! Insert an association point to the specified element.
//! @param element IN The element to add the associative point to.
//! @param assocPoint IN The associative point to insert.
//! @param pointNum IN The index of point to insert.
//! @param maxPoints IN The maximum number of associative points on the element.
//! @return SUCCESS if associative point was inserted.
//! @note An element that supports associative points has a dependency callback that is responsible for updating the element when the root elements change.
DGNPLATFORM_EXPORT static StatusInt InsertPoint
(
EditElementHandleR     element,
AssocPoint const&   assocPoint,
int                 pointNum,
int                 maxPoints
);

//! Uses the information in assocPoint to create a 3D data point that is returned in outPoint.
//! @param outPoint OUT The point location of the evaluated associative point.
//! @param assocPoint IN The associative point to query.
//! @param modelRef IN The model that contains the associative point dependent element.
//! @return SUCCESS if associative point was extracted.
DGNPLATFORM_EXPORT static BentleyStatus GetPoint
(
DPoint3dP           outPoint,
AssocPoint const&   assocPoint,
DgnModelP        modelRef
);

//! Remove a single associative point from an element.
//! @param element IN The element to remove the associative point on.
//! @param pointNum IN The point number to remove.
//! @param maxPoints IN The total number of points on element.
//! @return SUCCESS if associative point was removed.
DGNPLATFORM_EXPORT static StatusInt RemovePoint
(
EditElementHandleR element,
int             pointNum,
int             maxPoints
);

//! Removes all association points from an element.
//! @param element IN The element to remove the associative points on.
//! @return SUCCESS if associative points were removed.
DGNPLATFORM_EXPORT static StatusInt RemoveAllAssociations
(
EditElementHandleR element
);

//! Initialize an associative point from a snap path.
//! @param assocPoint OUT The associative point to setup.
//! @param pathIn IN The snap path.
//! @param modifierMask IN Mask of allowed associative point types.
//! @param parentModel IN The model that contains the associative point dependent element.
//! @param options IN Create options for associative point.
//! @return SUCCESS if associative point was setup
DGNPLATFORM_EXPORT static StatusInt CreateFromSnap
(
AssocPoint&         assocPoint,
SnapPathCP          pathIn,
int                 modifierMask,
DgnModelP        parentModel,
CreateOptions       options
);

//! Register roots changed extension for known handlers.
DGNPLATFORM_EXPORT static void RegisterExtensions ();

}; // AssociativePoint

//__PUBLISH_SECTION_END__

/*================================================================================**//**
* Extension implemented by elements to update associative point dependencies.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE IAssocPointRootsChangedExtension : Handler::Extension
{
DECLARE_KEY_METHOD

protected:

struct RootsChangedState
    {
    ElementHandleCP     m_depEh;            //!  => Dependent element.
    DependencyLinkageP  m_depData;          //! <=> Dependency data. This is a local copy and can be modified in place to invalidate roots, etc.
    UInt8*              m_rootStatus;       //!  => Array of root changes status for this dependency linkage/callback.
    bool                m_allowWrite;       //!  => Whether updating the element in the file is allowed (ex. false for undo/redo).

    bool                m_selfStatus;       //!  => Whether dependent element has been modified. May also have root changes.
    bool                m_onlySelf;         //!  => Whether dependent has been modified by itself without any root changes (value derived from m_rootStatus array).
    
    bool                m_failed;           //! <=  Set when the association has failed, ex. root has been deleted.
    bool                m_changed;          //! <=  Set when the dependent element has been modified and should be replaced in the model.
    bool                m_deleteSelf;       //! <=  Set to request deletion of the dependent element.
    bool                m_replaceLinkage;   //! <=  Set to request update of dependency linkage on dependent when local copy has been modified.
    };

//! Get last evaluated location for association at the given index.
//! @note For assoc_i roots index != iRoot.
virtual StatusInt _GetPoint (ElementHandleCR eh, DPoint3dR point, int index) = 0;

//! Set new location of updated association for the given index.
//! @note For assoc_i roots index != iRoot.
virtual StatusInt _SetPoint (EditElementHandleR eeh, DPoint3dCR point, int index) = 0;

//! Allow a complex component with root changes to update and rewrite the outermost complex header.
//! @note Almost always a bad idea to return true. A complex component should be treated as "frozen" to avoid multiple components updating and each rewriting the entire complex!
virtual bool _AllowComplexComponentUpdate () {return false;}

//! Evaluate current assoc point location for comparision with current persistent element state.
//! @note The default implementation calls AssociativePoint::GetPoint.
DGNPLATFORM_EXPORT virtual StatusInt _GetRootComparePoint (DPoint3dR point, RootsChangedState& stateInfo, int iRoot);

//! Allow special processing when root element has changed and location set from assoc point needs to be updated.
DGNPLATFORM_EXPORT virtual StatusInt _OnRootChanged (EditElementHandleR eeh, DPoint3dCR point, RootsChangedState& stateInfo, int iRoot);

//! Allow special processing when dependent element itself has changed without any changes to its roots.
DGNPLATFORM_EXPORT virtual StatusInt _OnDependentChanged (EditElementHandleR eeh, DPoint3dCR point, RootsChangedState& stateInfo, int iRoot);

//! Allow extension with specialized root or root status processing to override default processing per-root.
//! @note The default implementation calls _GetPoint/_SetPoint in order to query and update the element data.
DGNPLATFORM_EXPORT virtual StatusInt _OnProcessRoot (EditElementHandleR eeh, RootsChangedState& stateInfo, int iRoot);

//! Allow extension to update/validate element after successfully processing all roots.
//! @note The default implementation calls DisplayHandler::ValidElementRange if stateInfo.m_changed is set to update a modified dependent's range.
DGNPLATFORM_EXPORT virtual StatusInt _OnUpdateElementRootsFinish (EditElementHandleR eeh, RootsChangedState& stateInfo);

//! Allow extension with specialized behavior to update itself by directly accessing the dependency linkage data and status for each root.
//! @note The default implementation calls _OnProcessRoot for each root and _OnUpdateElementRootsFinish afterwards.
DGNPLATFORM_EXPORT virtual StatusInt _OnUpdateElementRoots (EditElementHandleR eeh, RootsChangedState& stateInfo);

public:

//! Allow extension to completely handle the dependency roots changed event.
//! @note The default implementation of this method is what calls the other virtual methods of this extension. Overriding this method is not recommended.
//! @remarks Returning ERROR will cause an entire transaction to rolled back which is almost always a bad idea! Preferable to return SUCCESS and set failed roots invalid.
DGNPLATFORM_EXPORT virtual StatusInt _OnProcessRootsChanged (ElementHandleCR depEh, DependencyLinkageCR depData, UInt8* rootStatus, UInt8 selfStatus, bool allowWrite);

ELEMENTHANDLER_EXTENSION_DECLARE_MEMBERS (IAssocPointRootsChangedExtension, DGNPLATFORM_EXPORT)

}; // IAssocPointRootsChangedExtension

//__PUBLISH_SECTION_START__

END_BENTLEY_DGNPLATFORM_NAMESPACE

BEGIN_BENTLEY_API_NAMESPACE

// graphite moved this function here from AssocFrPath.h and removed that file.
DGNPLATFORM_EXPORT void         assoc_getSnapPoint
(
DPoint3dR           snapPoint,
SnapPathCP          snapPathP,
DgnModelP        parentModel,
bool                localCoords
);

END_BENTLEY_API_NAMESPACE

/** @endcond */
