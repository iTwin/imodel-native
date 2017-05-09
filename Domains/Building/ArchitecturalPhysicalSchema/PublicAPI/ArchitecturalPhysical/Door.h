/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/PublicAPI/ArchitecturalPhysical/Door.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ArchitecturalPhysical\ArchitecturalPhysicalApi.h>
#include <BuildingPhysical\BuildingPhysicalApi.h>

BEGIN_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE


//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ArchitecturalBaseElement : Dgn::PhysicalElement
    {
    //    DEFINE_T_SUPER(Dgn::PhysicalElement)
    DGNELEMENT_DECLARE_MEMBERS(AP_CLASS_ArchitecturalBaseElement, Dgn::PhysicalElement);
    friend struct ArchitecturalBaseElementHandler;

    ////! Enum used to indicate the condition of a tile
    //enum class Condition : int
    //    {
    //    Unknown=0,  //!< Tile condition is unknown
    //    New,        //!< Tile condition is new or like new
    //    Scratched,  //!< Tile is scratched
    //    Cracked     //!< Tile is cracked (supersedes Scratched)
    //    };

    ////! Enum that describes material choices for tiles
    //enum class CasingMaterialType : int
    //    {
    //    Invalid=0,
    //    RedPlastic,
    //    GreenPlastic,
    //    BluePlastic,
    //    OrangePlastic,
    //    PurplePlastic,
    //    YellowPlastic
    //    };

    protected:
        explicit ArchitecturalBaseElement(CreateParams const& params) : T_Super(params) {}

        //BUILDING_PHYSICAL_DOMAIN_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Dgn::ElementECPropertyAccessor&, ECN::ECValueCR value, Dgn::PropertyArrayIndex const&) override;

        // static Dgn::Render::GeometryParams GetCasingMaterialParams(Dgn::DgnDbR, Dgn::DgnCategoryId, CasingMaterialType);
        // static Dgn::Render::GeometryParams GetMagnetMaterialParams(Dgn::DgnDbR, Dgn::DgnCategoryId);

    public:
        ARCHITECTURAL_PHYSICAL_EXPORT static ArchitecturalBaseElementPtr Create(Utf8CP,  BuildingPhysical::BuildingPhysicalModelR);
        ARCHITECTURAL_PHYSICAL_EXPORT static ArchitecturalBaseElementPtr Create(CreateParams const& params);

        //BUILDING_PHYSICAL_DOMAIN_EXPORT static CasingMaterialType ParseCasingMaterial(Utf8CP);

        // BUILDING_PHYSICAL_DOMAIN_EXPORT Condition GetCondition() const;
        // BUILDING_PHYSICAL_DOMAIN_EXPORT void SetCondition(Condition condition);
        // BUILDING_PHYSICAL_DOMAIN_EXPORT void SetCondition(Utf8CP);

        //! Move the placement for this element by adding the specified vector to its current placement
        //  BUILDING_PHYSICAL_DOMAIN_EXPORT void MoveRelative(DVec3dCR);
    };

//=======================================================================================
//! The ElementHandler for SmallSquareTile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ArchitecturalBaseElementHandler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(AP_CLASS_ArchitecturalBaseElement, ArchitecturalBaseElement, ArchitecturalBaseElementHandler, Dgn::dgn_ElementHandler::Physical, ARCHITECTURAL_PHYSICAL_EXPORT)
    };


//=======================================================================================
//! The SmallSquare tile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Door : ArchitecturalBaseElement
    {
    DGNELEMENT_DECLARE_MEMBERS(AP_CLASS_Door, ArchitecturalBaseElement);
    friend struct DoorHandler;

    protected:
        explicit Door(CreateParams const& params) : T_Super(params) {}

    public:
        ARCHITECTURAL_PHYSICAL_EXPORT static DoorPtr Create(BuildingPhysical::BuildingPhysicalModelR);
    };

//__PUBLISH_EXTRACT_END__
//__PUBLISH_EXTRACT_START__ ToyTilePhysicalElement_DeclareHandler.sampleCode
//=======================================================================================
//! The ElementHandler for SmallSquareTile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DoorHandler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(AP_CLASS_Door, Door, DoorHandler, ArchitecturalBaseElementHandler, ARCHITECTURAL_PHYSICAL_EXPORT)
    };


//=======================================================================================
//! The SmallSquare tile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DoorType : Dgn::PhysicalType
    {
    DGNELEMENT_DECLARE_MEMBERS(AP_CLASS_DoorType, Dgn::PhysicalType);
    friend struct DoorTypeHandler;

    protected:
        explicit DoorType(CreateParams const& params) : T_Super(params) {}

    public:
        ARCHITECTURAL_PHYSICAL_EXPORT static DoorTypePtr Create(BuildingPhysical::BuildingTypeDefinitionModelR);
    };

//=======================================================================================
//! The ElementHandler for SmallSquareTile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DoorTypeHandler : Dgn::dgn_ElementHandler::PhysicalType
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(AP_CLASS_DoorType, DoorType, DoorTypeHandler, Dgn::dgn_ElementHandler::PhysicalType, ARCHITECTURAL_PHYSICAL_EXPORT)
    };

END_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE





