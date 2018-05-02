/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/PublicApi/ClassificationSystemsElements.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__

CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ClassificationSystem)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(Classification)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ClassificationGroup)

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

//=======================================================================================
//! A Classification System element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ClassificationSystem : Dgn::DefinitionElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem, Dgn::DefinitionElement);

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationSystem(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationSystem(CreateParams const& params, Utf8CP name);
        friend struct ClassificationSystemHandler;
        friend struct ClassificationSystemsDomain;

        virtual void _OnInserted(Dgn::DgnElementP copiedFrom) const override;

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

        //! Gets the name of this ClassificationSystem
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetCode().GetValueUtf8(); }

    };

//=======================================================================================
//! A ClassificationSystem Class Definition element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Classification : Dgn::DefinitionElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_Classification, Dgn::DefinitionElement);
    private:
        BE_PROP_NAME(Name)
        BE_PROP_NAME(ClassificationId)
        BE_PROP_NAME(Description)
        BE_PROP_NAME(Group)
        BE_PROP_NAME(Specialization)

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Classification(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Classification(CreateParams const& params, Utf8CP name, Utf8CP id, Utf8CP description, ClassificationGroupCP group, ClassificationCP specializes);
        friend struct ClassificationHandler;
        friend struct ClassificationSystemsDomain;

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates a Classification
        //! @param[in]  system      db to insert Classification in
        //! @param[in]  name        name of this Classification
        //! @param[in]  description description of this Classification
        //! @param[in]  group       Group this Classification is in
        //! @param[in]  specializes What Classification this Classification specializes in
        //! @return     a ptr to created Classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationPtr Create(ClassificationSystemCR system, Utf8CP name, Utf8CP id, Utf8CP description, ClassificationGroupCP group, ClassificationCP specializes);

        //! Sets the Name of this Classification
        //! @param[in] name   new Name for this Classification
        void SetName(Utf8CP name) { SetPropertyValue(prop_Name(), name); }

        //! Sets the Id of this Classification
        //! @param[in] id   new Id for this Classification
        void SetClassificationId(Utf8CP id) { SetPropertyValue(prop_ClassificationId(), id); }

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

        //!Returns this Classification Name property
        //! @return Name property of the Classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetPropertyValueString(prop_Name()).c_str(); }
        
        //!Returns this Classification Id property
        //! @return Id property of the Classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetClassificationId() const { return GetPropertyValueString(prop_ClassificationId()).c_str(); }

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
struct EXPORT_VTABLE_ATTRIBUTE ClassificationGroup : Dgn::GroupInformationElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationGroup, Dgn::GroupInformationElement);
    private:
        BE_PROP_NAME(Name)
    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationGroup(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationGroup(CreateParams const& params, Utf8CP name);
        friend struct ClassificationGroupHandler;
        friend struct ClassificationSystemsDomain;

        //! Sets the name of this ClassificationGroup
        //! @param[in]  name   new name for this ClassificationGroup
        void SetName(Utf8CP name) { SetPropertyValue(prop_Name(), name); }

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
        
        //! Gets the name of this ClassificationGroup
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetPropertyValueString(prop_Name()).c_str(); }
    };
    
END_CLASSIFICATIONSYSTEMS_NAMESPACE
