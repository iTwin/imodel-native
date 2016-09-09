/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/FunctionalDomain.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnDomain.h>

DGNPLATFORM_TYPEDEFS(FunctionalModel)
DGNPLATFORM_TYPEDEFS(FunctionalType)

DGNPLATFORM_REF_COUNTED_PTR(FunctionalModel)
DGNPLATFORM_REF_COUNTED_PTR(FunctionalType)

#define FUNCTIONAL_DOMAIN_ECSCHEMA_PATH         L"ECSchemas/Domain/Functional.01.00.ecschema.xml"
#define FUNCTIONAL_DOMAIN_NAME                  "Functional"
#define FUNCTIONAL_SCHEMA(className)            FUNCTIONAL_DOMAIN_NAME "." className

#define FUNC_CLASS_FunctionalModel              "FunctionalModel"
#define FUNC_CLASS_FunctionalElement            "FunctionalElement"
#define FUNC_CLASS_FunctionalBreakdownElement   "FunctionalBreakdownElement"
#define FUNC_CLASS_FunctionalComponentElement   "FunctionalComponentElement"
#define FUNC_CLASS_FunctionalType               "FunctionalType"

BEGIN_BENTLEY_DGN_NAMESPACE

namespace func_ModelHandler {struct Functional;};
namespace func_ElementHandler {struct FunctionalBreakdownElementHandler; struct FunctionalComponentElementHandler; struct FunctionalTypeHandler;};

//=======================================================================================
//! The Functional DgnDomain
//! @ingroup GROUP_DgnDomain
// @bsiclass                                                    Shaun.Sewall    06/2016
//=======================================================================================
struct FunctionalDomain : DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(FunctionalDomain, DGNPLATFORM_EXPORT)

public:
    FunctionalDomain();
    ~FunctionalDomain();
    
    //! Import the ECSchema for the FunctionalDomain into the specified DgnDb
    DGNPLATFORM_EXPORT static DgnDbStatus ImportSchema(DgnDbR);
};

//=======================================================================================
//! A model which contains only FunctionalElements.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Shaun.Sewall    05/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE FunctionalModel : DgnModel
{
    DGNMODEL_DECLARE_MEMBERS(FUNC_CLASS_FunctionalModel, DgnModel);
    friend struct func_ModelHandler::Functional;

protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element) override;
    explicit FunctionalModel(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static FunctionalModelPtr Create(DgnElementCR modeledElement);
};

//=======================================================================================
//! Abstract base class for functions performed by by other (typically physical) elements.
//! For example, the <i>function of pumping</i> is performed by a Pump (PhysicalElement).
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    05/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE FunctionalElement : RoleElement
{
    DEFINE_T_SUPER(RoleElement);
protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert() override;
    explicit FunctionalElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! Abstract base class for objects that break down the functional structure of something.
//! For example, a faciliy, like a building or plant, may be broken down into functional <i>systems</i>.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    05/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE FunctionalBreakdownElement : FunctionalElement
{
    DGNELEMENT_DECLARE_MEMBERS(FUNC_CLASS_FunctionalBreakdownElement, FunctionalElement)
    friend struct func_ElementHandler::FunctionalBreakdownElementHandler;

protected:
    explicit FunctionalBreakdownElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! Abstract base class for functional components.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    06/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE FunctionalComponentElement : FunctionalElement
{
    DGNELEMENT_DECLARE_MEMBERS(FUNC_CLASS_FunctionalComponentElement, FunctionalElement)
    friend struct func_ElementHandler::FunctionalComponentElementHandler;

protected:
    explicit FunctionalComponentElement(CreateParams const& params) : T_Super(params) {}

public:
    //! Set the FunctionalType for this FunctionalComponentElement
    DgnDbStatus SetFunctionalType(DgnElementId functionalTypeId) {return SetPropertyValue("FunctionalType", functionalTypeId);}
    //! Get the DgnElementId of the FunctionalType for this FunctionalComponentElement
    //! @return Will be invalid if there is no FunctionalType associated with this FunctionalComponentElement
    DgnElementId GetFunctionalTypeId() const {return GetPropertyValueId<DgnElementId>("FunctionalType");}
    //! Get the FunctionalType for this FunctionalComponentElement
    //! @return Will be invalid if there is no FunctionalType associated with this FunctionalComponentElement
    DGNPLATFORM_EXPORT FunctionalTypeCPtr GetFunctionalType() const;
};

//=======================================================================================
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    08/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE FunctionalType : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(FUNC_CLASS_FunctionalType, DefinitionElement)
    friend struct func_ElementHandler::FunctionalTypeHandler;

protected:
    explicit FunctionalType(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! The namespace that only contains ModelHandlers for the FunctionalDomain
//! @private
//=======================================================================================
namespace func_ModelHandler
{
    //! The ModelHandler for FunctionalModel
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE Functional : dgn_ModelHandler::Model
    {
        MODELHANDLER_DECLARE_MEMBERS(FUNC_CLASS_FunctionalModel, FunctionalModel, Functional, Model, DGNPLATFORM_EXPORT)
    };
}

//=======================================================================================
//! The namespace that only contains ElementHandlers for the FunctionalDomain
//! @private
//=======================================================================================
namespace func_ElementHandler
{
    //! The ElementHandler for FunctionalBreakdownElement
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE FunctionalBreakdownElementHandler : dgn_ElementHandler::Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(FUNC_CLASS_FunctionalBreakdownElement, FunctionalBreakdownElement, FunctionalBreakdownElementHandler, dgn_ElementHandler::Element, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for FunctionalComponentElement
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE FunctionalComponentElementHandler : dgn_ElementHandler::Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(FUNC_CLASS_FunctionalComponentElement, FunctionalComponentElement, FunctionalComponentElementHandler, dgn_ElementHandler::Element, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for FunctionalType
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE FunctionalTypeHandler : dgn_ElementHandler::Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(FUNC_CLASS_FunctionalType, FunctionalType, FunctionalTypeHandler, dgn_ElementHandler::Definition, DGNPLATFORM_EXPORT)
    };
}

END_BENTLEY_DGN_NAMESPACE
