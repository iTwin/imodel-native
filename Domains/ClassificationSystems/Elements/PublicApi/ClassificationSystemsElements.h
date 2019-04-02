/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/PublicApi/ClassificationSystemsElements.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatformApi.h>
#include <ClassificationSystems/Domain/ClassificationSystemsMacros.h>
#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>
#include <BuildingShared/interfaces.h>

CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ClassificationSystem)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(Classification)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ClassificationGroup)

#include "IClassified.h"

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

//=======================================================================================
//! A Classification System element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ClassificationSystem final : Dgn::DefinitionElement, BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::IBCSSerializable
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem, Dgn::DefinitionElement);
    private:
        static Dgn::DgnCode GetSystemCode(Dgn::DgnDbR db, Utf8CP name);
    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationSystem(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationSystem(CreateParams const& params, Utf8CP name);
        friend struct ClassificationSystemHandler;
        friend struct ClassificationSystemsDomain;

        virtual void _SerializeProperties(Json::Value& elementData) const override;
        virtual void _FormatSerializedProperties(Json::Value& elementData) const override;

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationSystem, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates a ClassificationSystem
        //! @param[in]  db          db to insert class definition in
        //! @param[in]  name        name of the Classification System
        //! @return     a ptr to created ClassificationSystem
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationSystemPtr Create(Dgn::DgnDbR db, Utf8CP name);

        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationSystemCPtr TryGet(Dgn::DgnDbR db, Utf8CP name);

        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static Dgn::ElementIterator MakeIterator(Dgn::DgnDbR dgnDbR);

        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Dgn::ElementIterator MakeClassificationIterator() const;

        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Dgn::ElementIterator MakeClassificationGroupIterator() const;

        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT void GetClassificationDataVerbose (Json::Value& elementData) const;

        //! Gets the name of this ClassificationSystem
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const;
    };

//=======================================================================================
//! A ClassificationSystem Class Definition element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Classification final : Dgn::DefinitionElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_Classification, Dgn::DefinitionElement);
    private:
        BE_PROP_NAME(Description)
        
        Dgn::DgnCode GetClassificationCode(Dgn::DgnDbR db, Utf8CP name, Dgn::DgnElementId id) const;

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Classification(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Classification(CreateParams const& params, ClassificationSystemCR system, Utf8CP name, Utf8CP id, Utf8CP description, ClassificationGroupCP group, ClassificationCP specializes);
        friend struct ClassificationHandler;
        friend struct ClassificationSystemsDomain;

        //! Sets the Description of this Classification
        //! @param[in] description   new Description for this Classificatio
        void SetDescription(Utf8CP description) { SetPropertyValue(prop_Description(), description); }

        //! Sets Classification group Id
        //! @param[in] groupId to set
        void SetGroupId(Dgn::DgnElementId groupId);

        //! Sets Specialization Classification Id
        //! @param[in] specializationId to set
        void SetSpecializationId(Dgn::DgnElementId specializationId);

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(Classification, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates and inserts a Classification
        //! @param[in]  system      db to insert Classification in
        //! @param[in]  name        name of this Classification
        //! @param[in]  description description of this Classification
        //! @param[in]  group       Group this Classification is in
        //! @param[in]  specializes What Classification this Classification specializes in
        //! @return     a ptr to created Classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationPtr CreateAndInsert(ClassificationSystemCR system, Utf8CP name, Utf8CP id, Utf8CP description, ClassificationGroupCP group, ClassificationCP specializes);

        //!Returns this Classification Name property
        //! @return Name property of the Classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetUserLabel(); }
        
        //!Returns this Classification Id property
        //! @return Id property of the Classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetClassificationId() const { return GetCode().GetValueUtf8CP(); }

        //!Returns Id property of the parent ClassificationSystem
        //! @return Id property of the parent ClassificationSystem
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Dgn::DgnElementId GetClassificationSystemId() const;

        //!Returns this Classification Description property
        //! @return Description property of the Classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetDescription() const { return GetPropertyValueString(prop_Description()).c_str(); }

        //!Returns id of group that has this Classification
        //! @return group id of the Classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Dgn::DgnElementId GetGroupId() const;

        //!Returns id of Classification that this Classification Specializes in
        //! @return group id of the Classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Dgn::DgnElementId GetSpecializationId() const;


    };
//=======================================================================================
//! A ClassificationSystem Class Definition Group element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ClassificationGroup final : Dgn::GroupInformationElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationGroup, Dgn::GroupInformationElement);

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationGroup(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationGroup(CreateParams const& params, Utf8CP name);
        friend struct ClassificationGroupHandler;
        friend struct ClassificationSystemsDomain;

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationGroup, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)
        
        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates a ClassificationGroup
        //! @param[in]  db          db to insert class definition group in
        //! @param[in]  name        name of the ClassificationSystem class definition group
        //! @return     a ptr to created ClassificationGroup
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationGroupPtr Create(ClassificationSystemCR system, Utf8CP name);

        //!Returns Id property of the parent ClassificationSystem
        //! @return Id property of the parent ClassificationSystem
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Dgn::DgnElementId GetClassificationSystemId() const;

        //! Gets the name of this ClassificationGroup
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetUserLabel(); }
    };
    
END_CLASSIFICATIONSYSTEMS_NAMESPACE
