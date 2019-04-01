/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/PublicApi/Classification.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatformApi.h>
#include <ClassificationSystems/Domain/ClassificationSystemsMacros.h>
#include <ClassificationSystems/Elements/ForwardDeclarations.h>

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

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
    
END_CLASSIFICATIONSYSTEMS_NAMESPACE
