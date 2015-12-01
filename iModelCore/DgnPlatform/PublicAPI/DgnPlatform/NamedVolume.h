/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/NamedVolume.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

DGNPLATFORM_REF_COUNTED_PTR(NamedVolume)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

using BentleyApi::BeSQLite::EC::ECInstanceKey;
using BentleyApi::BeSQLite::EC::ECSqlStatement;
using BentleyApi::BeSQLite::Statement;

namespace dgn_ElementHandler { struct NamedVolumeHandler; }

//=======================================================================================
//! API to setup user defined regions that can be shaded, clipped, queried & serialized
// @bsiclass                                                 Ramanujam.Raman      01/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE NamedVolume : PhysicalElement
{
    friend struct dgn_ElementHandler::NamedVolumeHandler;
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_NamedVolume, PhysicalElement)
public:
    struct CreateParams : PhysicalElement::CreateParams
    {
        friend struct dgn_ElementHandler::NamedVolumeHandler;
        DEFINE_T_SUPER(PhysicalElement::CreateParams);

        DPoint3d m_origin;
        bvector<DPoint2d> m_shape;
        double m_height;

    protected:
        CreateParams(DgnDbR db, DgnModelId modelId, DPoint3dCR origin, bvector<DPoint2d> const& shape, double height, DgnClassId classId, DgnCategoryId category = DgnCategoryId(), Code const& code = Code(), Utf8CP label = nullptr, DgnElementId parent = DgnElementId()) :
            T_Super(db, modelId, classId, category.IsValid() ? category : NamedVolume::GetDefaultCategoryId(db), Placement3d(), code, label, parent), m_origin(origin), m_shape(shape), m_height(height) {}

        explicit CreateParams(Dgn::DgnElement::CreateParams const& params) : T_Super(params, DgnCategoryId(), Placement3d()) {}

    public:
        //! Parameters to create a NamedVolume
        //! @param db DgnDb
        //! @param modelId Model Id
        //! @param category Category Id
        //! @param origin Origin of the volume, described from the Project Coordinate System in storage units (this translated origin, and the Project Coordinate System directions together define a Local Coordinate System for the volume)
        //! @param shape Closed loop of 2D points representing the boundary of the extruded volume, described from the Local Coordinate System in storage units.
        //! @param height Height of the extruded volume in storage units
        //! @param code Code
        //! @param id ElementId
        //! @param parent Parent ElementId
        CreateParams(DgnDbR db, DgnModelId modelId, DPoint3dCR origin, bvector<DPoint2d> const& shape, double height, DgnCategoryId category = DgnCategoryId(), Code const& code = Code(), DgnElementId id = DgnElementId(), DgnElementId parent = DgnElementId()) :
            T_Super(db, modelId, NamedVolume::QueryClassId(db), category.IsValid() ? category : NamedVolume::GetDefaultCategoryId(db), Placement3d(), code, "", parent), m_origin(origin), m_shape(shape), m_height(height)
            {}

        CreateParams(CreateParams const& params) : T_Super(params), m_origin(params.m_origin), m_shape(params.m_shape), m_height(params.m_height) {}
    };

private:
    mutable ViewContextP m_viewContext = nullptr;
    mutable ClipPlaneSet* m_clipPlaneSet = nullptr;

    ClipVectorPtr CreateClipVector() const;
    std::unique_ptr<FenceParams> CreateFence (DgnViewportP viewport, bool allowPartialOverlaps) const;
    static std::unique_ptr<DgnViewport> CreateNonVisibleViewport (DgnDbR dgnDb);

    BentleyStatus GetRange(DRange3d& range) const; // Gets the range of the volume, described from the Project Coordinate System in storage units 
    
    void FindElements(DgnElementIdSet& elementIds, FenceParamsR fence, Statement& stmt, DgnDbR dgnDb) const;
    BentleyStatus ExtractExtrusionDetail(DgnExtrusionDetail& extrusionDetail) const;

public:
    /*
    * CRUD
    */

    //! Constructor
    DGNPLATFORM_EXPORT explicit NamedVolume(CreateParams const& params);

    //! Creates a new NamedVolume element
    static NamedVolumePtr Create(PhysicalModelCR model, DPoint3dCR origin, bvector<DPoint2d> const& shape, double height) { return new NamedVolume(CreateParams(model.GetDgnDb(), model.GetModelId(), origin, shape, height)); }

    //! Creates a new NamedVolume element
    static NamedVolumePtr Create(CreateParams const& params) { return new NamedVolume(params); }

    //! Destructor
    ~NamedVolume() { ClearClip(); }

    //! Setup the geom stream
    //! @param origin Origin of the volume, described from the Project Coordinate System in storage units (this translated origin, and the Project Coordinate System directions together define a Local Coordinate System for the volume)
    //! @param shape Closed loop of 2D points representing the boundary of the extruded volume, described from the Local Coordinate System in storage units.
    //! @param height Height of the extruded volume in storage units
    DGNPLATFORM_EXPORT void SetupGeomStream(DPoint3dCR origin, bvector<DPoint2d> const& shape, double height);

    //! Extract geometry details
    DGNPLATFORM_EXPORT BentleyStatus ExtractGeomStream(bvector<DPoint3d>& shape, DVec3d& direction, double& height) const;

    //! Inserts the volume into the Db
    DGNPLATFORM_EXPORT NamedVolumeCPtr Insert();
    
    //! Updates the volume in the Db
    DGNPLATFORM_EXPORT NamedVolumeCPtr Update();

    //! Get a read only copy of the NamedVolume from the DgnDb
    DGNPLATFORM_EXPORT static NamedVolumeCPtr Get(DgnDbCR dgndb, Dgn::DgnElementId elementId);
    
    //! Get an editable copy of the NamedVolume from the DgnDb
    DGNPLATFORM_EXPORT static NamedVolumePtr GetForEdit(DgnDbCR dgndb, Dgn::DgnElementId elementId);

    /*
     * Setup views with the Volume
     */

    //! Setup view clips to the boundary of the volume. 
    //! @param context ViewContext to setup the clips in 
    DGNPLATFORM_EXPORT BentleyStatus SetClip(ViewContextR context) const;

    //! Clear any view clips previously setup
    DGNPLATFORM_EXPORT void ClearClip() const;
    
    //! Fit the view to just show the volume, keeping the view's current rotation.
    //! @param[in] viewport  Viewport to fit the volume
    //! @param[in] aspectRatio The X/Y aspect ratio of the view into which the result will be displayed. If the aspect ratio of the volume does not
    //! match aspectRatio, the shorter axis is lengthened and the volume is centered. If aspectRatio is NULL, no adjustment is made.
    //! @param[in] margin The amount of "white space" to leave around the view volume (which essentially increases the volume
    //! of space shown in the view.) If NULL, no additional white space is added.
    //! @note For 3d views, the camera is centered on the new volume and moved along the view z axis using the default lens angle
    //! such that the entire volume is visible.
    //! @note, for 2d views, only the X and Y values of volume are used.
    //! @see ViewController::LookAtVolume()
    DGNPLATFORM_EXPORT void Fit (DgnViewport& viewport, double const* aspectRatio=nullptr, ViewController::MarginPercent const* margin=nullptr) const;

    //! Hide the volume in all views
    void Hide() const { SetUndisplayed(true); }

    //! Un-hide the volume in all views
    //! @remarks The view should display the category and model of the volume to be shown
    void UnHide() const { SetUndisplayed(false); }


    /*
     * Query contained elements in the Volume
     */

    //! Find all elements in the project within the volume
    //! @param[out] elementIds Element ids found. Any existing entries are not cleared. 
    //! @param[in] dgnDb DgnDb containing the elements
    //! @param[in] allowPartialOverlaps Pass false to find only elements that are strictly contained. Pass true 
    //! to include elements that partially overlap the volume (i.e., at the boundary). 
    DGNPLATFORM_EXPORT void FindElements (DgnElementIdSet& elementIds, DgnDbR dgnDb, bool allowPartialOverlaps = true) const;

    //! Find all elements in the specified view within the volume
    //! @param[out] elementIds Element ids found. Any existing entries are not cleared. 
    //! @param[in] viewport Viewport that's used to find only the elements displayed. 
    //! @param[in] allowPartialOverlaps Pass false to find only elements that are strictly contained. Pass true 
    //! to include elements that partially overlap the volume (i.e., at the boundary). 
    DGNPLATFORM_EXPORT void FindElements (DgnElementIdSet& elementIds, DgnViewportR viewport, bool allowPartialOverlaps = true) const;

    //! Determines if the volume contains the element
    //! @param element Element to check
    //! @param allowPartialOverlaps Pass false to check strict containment. Pass true to allow elements that partially
    //! overlap the volume (i.e., at the boundary). 
    //! @return true if volume contains element. false otherwise. 
    DGNPLATFORM_EXPORT bool ContainsElement(DgnElementCR element, bool allowPartialOverlaps = true) const;

    /*
    * Misc
    */

    //! Query the DgnClassId of the dgn.NamedVolume ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the dgn.NamedVolume class - it does @em not return the class of a specific instance.
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_NamedVolume)); }

    //! Gets the default category id for the vaolumes
    DGNPLATFORM_EXPORT static DgnCategoryId GetDefaultCategoryId(DgnDbR db);
};

namespace dgn_ElementHandler
{
    //! The ElementHandler for NamedVolume
    struct EXPORT_VTABLE_ATTRIBUTE NamedVolumeHandler : Physical
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_NamedVolume, NamedVolume, NamedVolumeHandler, Physical, DGNPLATFORM_EXPORT)
    };
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__

