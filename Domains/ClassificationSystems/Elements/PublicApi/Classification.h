/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

        static Dgn::DgnModelPtr GetOrCreateTableSubModel(ClassificationTableCR table);

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Classification(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Classification(CreateParams const& params, ClassificationTableCR system, Utf8CP name, Utf8CP id, Utf8CP description, ClassificationGroupCP group, ClassificationCP specializes);
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
        //! @param[in]  table       Classification Table in which this Classification Group will be inserted
        //! @param[in]  name        name of this Classification
        //! @param[in]  id          id for code generation
        //! @param[in]  description description of this Classification
        //! @param[in]  group       Group this Classification is in
        //! @param[in]  specializes What Classification this Classification specializes in
        //! @return     a ptr to created Classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationPtr CreateAndInsert(ClassificationTableCR table, Utf8CP name, Utf8CP id, Utf8CP description, ClassificationGroupCP group, ClassificationCP specializes);

        //! Tries to get a Classification from the database using provided name and table id
        //! @param[in]  db       db that containes ClassificationSystem
        //! @param[in]  id       Classification System's name
        //! @param[in]  tableId  Classification Table's id which the Classification belongs to
        //! @return     a ptr to Classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationCPtr TryGet(Dgn::DgnDbR db, Utf8StringCR id, Dgn::DgnElementId tableId);

        //! Returns a classification which belongs to System <- Table hierarchy. 
        //! If any of the pieces are missing, they are created and inserted into the database.
        //! @param[in]  db           db that should contain the hierarchy
        //! @param[in]  model        model which should contain the classificationsystem
        //! @param[in]  name         name of the Classification
        //! @param[in]  id           id for code generation
        //! @param[in]  description  description of the Classification
        //! @param[in]  systemName   name of the Classification System that the Classification belongs to
        //! @param[in]  tableName    name of the Classification Table that the Classification belongs to
        //! @return     a ptr to created or queried Classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationPtr GetOrCreateBySystemTableNames(
            Dgn::DgnDbR db, Dgn::DgnModelCR model, Utf8StringCR name, Utf8StringCR id, Utf8StringCR description, Utf8StringCR systemName, Utf8StringCR systemEdition, Utf8StringCR tableName);

        //!Returns this Classification Name property
        //! @return Name property of the Classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetUserLabel(); }
        
        //! Gets Classification code generated from given parameters
        //! @param[in]  db       db that contains code specs
        //! @param[in]  name     name of the Classification that will be used for code generation
        //! @param[in]  id       id of ClassificationTable that contains this Classification
        //! @return     generated code
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static Dgn::DgnCode GetClassificationCode(Dgn::DgnDbR db, Utf8StringCR name, Dgn::DgnElementId id);

        //!Returns this Classification Id property
        //! @return Id property of the Classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetClassificationId() const { return GetCode().GetValueUtf8CP(); }

        //!Returns Id property of the parent Classification Table
        //! @return Id property of the parent Classification Table
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Dgn::DgnElementId GetClassificationTableId() const;

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
