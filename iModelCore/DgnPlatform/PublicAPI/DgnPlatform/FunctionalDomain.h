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

DGNPLATFORM_REF_COUNTED_PTR(FunctionalModel)

#define FUNCTIONAL_DOMAIN_ECSCHEMA_PATH             L"ECSchemas/Domain/Functional.01.00.ecschema.xml"
#define FUNCTIONAL_DOMAIN_NAME                      "Functional"
#define FUNCTIONAL_SCHEMA(className)                FUNCTIONAL_DOMAIN_NAME "." className

#define FUNC_CLASSNAME_FunctionalModel              "FunctionalModel"
#define FUNC_CLASSNAME_FunctionalElement            "FunctionalElement"
#define FUNC_CLASSNAME_FunctionalBreakdownElement   "FunctionalBreakdownElement"
#define FUNC_CLASSNAME_FunctionalComponentElement   "FunctionalComponentElement"

BEGIN_BENTLEY_DGN_NAMESPACE

namespace func_ModelHandler {struct Functional;};

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
    DGNMODEL_DECLARE_MEMBERS(FUNC_CLASSNAME_FunctionalModel, DgnModel);
    friend struct func_ModelHandler::Functional;

protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element) override;
    explicit FunctionalModel(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static FunctionalModelPtr Create(DgnDbR db, DgnCode const& code = DgnCode());
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
    DEFINE_T_SUPER(FunctionalElement);
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
    DEFINE_T_SUPER(FunctionalElement);
protected:
    explicit FunctionalComponentElement(CreateParams const& params) : T_Super(params) {}
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
        MODELHANDLER_DECLARE_MEMBERS(FUNC_CLASSNAME_FunctionalModel, FunctionalModel, Functional, Model, DGNPLATFORM_EXPORT)
    };
}

END_BENTLEY_DGN_NAMESPACE
