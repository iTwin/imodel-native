/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnDomain.h>

DGNPLATFORM_TYPEDEFS(FunctionalComposite)
DGNPLATFORM_TYPEDEFS(FunctionalModel)
DGNPLATFORM_TYPEDEFS(FunctionalPartition)
DGNPLATFORM_TYPEDEFS(FunctionalPortion)
DGNPLATFORM_TYPEDEFS(FunctionalType)

DGNPLATFORM_REF_COUNTED_PTR(FunctionalComposite)
DGNPLATFORM_REF_COUNTED_PTR(FunctionalModel)
DGNPLATFORM_REF_COUNTED_PTR(FunctionalPartition)
DGNPLATFORM_REF_COUNTED_PTR(FunctionalPortion)
DGNPLATFORM_REF_COUNTED_PTR(FunctionalType)

#define FUNCTIONAL_DOMAIN_ECSCHEMA_PATH         L"ECSchemas/Domain/Functional.ecschema.xml"
#define FUNCTIONAL_DOMAIN_NAME                  "Functional"
#define FUNCTIONAL_SCHEMA(className)            FUNCTIONAL_DOMAIN_NAME "." className

#define FUNC_CLASS_FunctionalBreakdownElement   "FunctionalBreakdownElement"
#define FUNC_CLASS_FunctionalComponentElement   "FunctionalComponentElement"
#define FUNC_CLASS_FunctionalComposite          "FunctionalComposite"
#define FUNC_CLASS_FunctionalElement            "FunctionalElement"
#define FUNC_CLASS_FunctionalModel              "FunctionalModel"
#define FUNC_CLASS_FunctionalPartition          "FunctionalPartition"
#define FUNC_CLASS_FunctionalPortion            "FunctionalPortion"
#define FUNC_CLASS_FunctionalType               "FunctionalType"

#define FUNC_REL_FunctionalElementIsOfType      "FunctionalElementIsOfType"

BEGIN_BENTLEY_DGN_NAMESPACE

namespace func_ModelHandler 
    {
    struct Functional;
    };

namespace func_ElementHandler
    {
    struct FunctionalBreakdownElementHandler; 
    struct FunctionalComponentElementHandler; 
    struct FunctionalCompositeHandler; 
    struct FunctionalPartitionHandler; 
    struct FunctionalPortionHandler; 
    struct FunctionalTypeHandler;
    };

//=======================================================================================
//! The Functional DgnDomain
//! @ingroup GROUP_DgnDomain
// @bsiclass                                                    Shaun.Sewall    06/2016
//=======================================================================================
struct FunctionalDomain : DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(FunctionalDomain, DGNPLATFORM_EXPORT)

private:
    WCharCP _GetSchemaRelativePath() const override { return FUNCTIONAL_DOMAIN_ECSCHEMA_PATH; }

public:
    FunctionalDomain();
    ~FunctionalDomain();
};

//=======================================================================================
//! A FunctionalPartition provides a starting point for a FunctionalModel hierarchy
//! @note FunctionalPartition elements only reside in the RepositoryModel
// @bsiclass                                                    Shaun.Sewall    10/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE FunctionalPartition : InformationPartitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(FUNC_CLASS_FunctionalPartition, InformationPartitionElement);
    friend struct func_ElementHandler::FunctionalPartitionHandler;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnSubModelInsert(DgnModelCR model) const override;
    explicit FunctionalPartition(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a new FunctionalPartition
    //! @param[in] parentSubject The new FunctionalPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this FunctionalPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static FunctionalPartitionPtr Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
    //! Create and insert a new FunctionalPartition
    //! @param[in] parentSubject The new FunctionalPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this FunctionalPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static FunctionalPartitionCPtr CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
};

//=======================================================================================
//! A model which contains only FunctionalElements.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Shaun.Sewall    05/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE FunctionalModel : RoleModel
{
    DGNMODEL_DECLARE_MEMBERS(FUNC_CLASS_FunctionalModel, RoleModel);
    friend struct func_ModelHandler::Functional;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsertElement(DgnElementR element) override;
    explicit FunctionalModel(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static FunctionalModelPtr Create(FunctionalPartitionCR modeledElement);
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

private:
    BE_PROP_NAME(TypeDefinition)

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    explicit FunctionalElement(CreateParams const& params) : T_Super(params) {}

public:
    //! Set the FunctionalType for this FunctionalElement
    //! @param[in] functionalTypeId The DgnElementId of the FunctionalType to be associated with this FunctionalElement
    //! @param[in] relClassId The ECClassId of the ECRelationshipClass that must be a subclass of FunctionalElementIsOfType
    DgnDbStatus SetFunctionalType(DgnElementId functionalTypeId, ECN::ECClassId relClassId) {return SetPropertyValue(prop_TypeDefinition(), functionalTypeId, relClassId);}
    //! Get the DgnElementId of the FunctionalType for this FunctionalElement
    //! @return Will be invalid if there is no FunctionalType associated with this FunctionalElement
    DgnElementId GetFunctionalTypeId() const {return GetPropertyValueId<DgnElementId>(prop_TypeDefinition());}
    //! Get the FunctionalType for this FunctionalElement
    //! @return Will be invalid if there is no FunctionalType associated with this FunctionalElement
    DGNPLATFORM_EXPORT FunctionalTypeCPtr GetFunctionalType() const;
};

//=======================================================================================
//! Abstract base class for objects that break down the functional structure of something.
//! For example, a facility, like a building or plant, may be broken down into functional <i>systems</i>.
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
//! Concrete/generic class for objects that break down the functional structure of something.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    11/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE FunctionalComposite : FunctionalBreakdownElement
{
    DGNELEMENT_DECLARE_MEMBERS(FUNC_CLASS_FunctionalComposite, FunctionalBreakdownElement)
    friend struct func_ElementHandler::FunctionalCompositeHandler;

protected:
    explicit FunctionalComposite(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static FunctionalCompositePtr Create(FunctionalModelR);
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
};

//=======================================================================================
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    11/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE FunctionalPortion : FunctionalComponentElement
{
    DGNELEMENT_DECLARE_MEMBERS(FUNC_CLASS_FunctionalPortion, FunctionalComponentElement)
    friend struct func_ElementHandler::FunctionalPortionHandler;

protected:
    explicit FunctionalPortion(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static FunctionalPortionPtr Create(FunctionalModelR);
};

//=======================================================================================
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    08/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE FunctionalType : TypeDefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(FUNC_CLASS_FunctionalType, TypeDefinitionElement)
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
    struct EXPORT_VTABLE_ATTRIBUTE Functional : dgn_ModelHandler::Role
    {
        MODELHANDLER_DECLARE_MEMBERS(FUNC_CLASS_FunctionalModel, FunctionalModel, Functional, dgn_ModelHandler::Role, DGNPLATFORM_EXPORT)
    };
}

//=======================================================================================
//! The namespace that only contains ElementHandlers for the FunctionalDomain
//! @private
//=======================================================================================
namespace func_ElementHandler
{
    //! The ElementHandler for FunctionalPartition
    //! @private
    struct FunctionalPartitionHandler : Dgn::dgn_ElementHandler::InformationPartition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(FUNC_CLASS_FunctionalPartition, FunctionalPartition, FunctionalPartitionHandler, Dgn::dgn_ElementHandler::InformationPartition, DGNPLATFORM_EXPORT)
    };
    
    //! The ElementHandler for FunctionalBreakdownElement
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE FunctionalBreakdownElementHandler : dgn_ElementHandler::Role
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(FUNC_CLASS_FunctionalBreakdownElement, FunctionalBreakdownElement, FunctionalBreakdownElementHandler, dgn_ElementHandler::Role, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for FunctionalComposite
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE FunctionalCompositeHandler : FunctionalBreakdownElementHandler
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(FUNC_CLASS_FunctionalComposite, FunctionalComposite, FunctionalCompositeHandler, FunctionalBreakdownElementHandler, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for FunctionalComponentElement
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE FunctionalComponentElementHandler : dgn_ElementHandler::Role
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(FUNC_CLASS_FunctionalComponentElement, FunctionalComponentElement, FunctionalComponentElementHandler, dgn_ElementHandler::Role, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for FunctionalPortion
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE FunctionalPortionHandler : FunctionalComponentElementHandler
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(FUNC_CLASS_FunctionalPortion, FunctionalPortion, FunctionalPortionHandler, FunctionalComponentElementHandler, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for FunctionalType
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE FunctionalTypeHandler : dgn_ElementHandler::Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(FUNC_CLASS_FunctionalType, FunctionalType, FunctionalTypeHandler, dgn_ElementHandler::Definition, DGNPLATFORM_EXPORT)
    };
}

END_BENTLEY_DGN_NAMESPACE
