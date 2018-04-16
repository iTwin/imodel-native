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
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ClassificationSystemClassDefinition)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ClassificationSystemClassDefinitionGroup)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(CIBSEClassDefinition)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(OmniClassClassDefinition)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ASHRAEClassDefinition)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ASHRAE2004ClassDefinition)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ASHRAE2010ClassDefinition)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(MasterFormatClassDefinition)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(UniFormatClassDefinition)

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

//=======================================================================================
//! A Classification System element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ClassificationSystem : Dgn::DefinitionElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem, Dgn::DefinitionElement);
    private:
        BE_PROP_NAME(Name)

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationSystem(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationSystem(CreateParams const& params, Utf8CP name);
        friend struct ClassificationSystemHandler;
        friend struct ClassificationSystemsDomain;

        //! Sets the name of this ClassificationSystem
        //! @param[in]  name   new name for this ClassificationSystem
        void SetName(Utf8CP name) { SetPropertyValue(prop_Name(), name); }

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
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetPropertyValueString(prop_Name()).c_str(); }

    };

//=======================================================================================
//! A ClassificationSystem Class Definition element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ClassificationSystemClassDefinition : Dgn::DefinitionElement
    {
    DEFINE_T_SUPER(Dgn::DefinitionElement);
    private:
        BE_PROP_NAME(Group)

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationSystemClassDefinition(CreateParams const& params) : T_Super(params) {}

        //! Sets ClassificationSystemClassDefinition group Id
        //! @param[in] groupId to set
        void SetGroupId(Dgn::DgnElementId groupId);

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationSystemClassDefinition, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        //!Returns id of group that has this ClassificationSystemClassDefinition
        //! @return group id of the ClassificationSystemClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Dgn::DgnElementId GetGroupId() const;

    };
//=======================================================================================
//! A ClassificationSystem Class Definition Group element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ClassificationSystemClassDefinitionGroup : Dgn::GroupInformationElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystemClassDefinitionGroup, Dgn::GroupInformationElement);
    private:
        BE_PROP_NAME(Name)
    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationSystemClassDefinitionGroup(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationSystemClassDefinitionGroup(CreateParams const& params, Utf8CP name);
        friend struct ClassificationSystemClassDefinitionGroupHandler;
        friend struct ClassificationSystemsDomain;

        //! Sets the name of this ClassificationSystemClassDefinitionGroup
        //! @param[in]  name   new name for this ClassificationSystemClassDefinitionGroup
        void SetName(Utf8CP name) { SetPropertyValue(prop_Name(), name); }

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationSystemClassDefinitionGroup, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates a ClassificationSystemClassDefinitionGroup
        //! @param[in]  db          db to insert class definition group in
        //! @param[in]  name        name of the ClassificationSystem class definition group
        //! @return     a ptr to created ClassificationSystemClassDefinitionGroup
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationSystemClassDefinitionGroupPtr Create(Dgn::DgnDbR db, Utf8CP name);
        
        //! Gets the name of this ClassificationSystemClassDefinitionGroup
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetPropertyValueString(prop_Name()).c_str(); }
    };
    
    
//=======================================================================================
//! A CIBSE Class Definition element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CIBSEClassDefinition : ClassificationSystemClassDefinition
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_CIBSEClassDefinition, ClassificationSystemClassDefinition);
    private:
       BE_PROP_NAME(Name)
       BE_PROP_NAME(Category)

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT CIBSEClassDefinition(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT CIBSEClassDefinition(CreateParams const& params, Utf8CP name, ClassificationSystemClassDefinitionGroupCR group);
        friend struct CIBSEClassDefinitionHandler;
        friend struct ClassificationSystemsDomain;

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates a CIBSEClassDefinition
        //! @param[in]  db          db to insert class definition in
        //! @param[in]  name        name of the CIBSE Class definition
        //! @param[in]  Category    Category of the CIBSE Class definition
        //! @return     a ptr to created CIBSEClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static CIBSEClassDefinitionPtr Create(Dgn::DgnDbR db, Utf8CP name, ClassificationSystemClassDefinitionGroupCR group);

        //! Sets the name of this CIBSEClassDefinition
        //! @param[in]  name   new name for this CIBSEClassDefinition
        void SetName(Utf8CP name) { SetPropertyValue(prop_Name(), name); }

        //! Sets the Category of this CIBSEClassDefinition
        //! @param[in]  Category   new Category for this CIBSEClassDefinition
        void SetCategory(Utf8CP Category) { SetPropertyValue(prop_Category(), Category); }

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(CIBSEClassDefinition, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        //! Gets the name of this CIBSEClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetPropertyValueString(prop_Name()).c_str(); }

        //! Gets the Category of this CIBSEClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetCategory() const { return GetPropertyValueString(prop_Category()).c_str(); }
        
    };

//=======================================================================================
//! A Omni Class Definition element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OmniClassClassDefinition : ClassificationSystemClassDefinition
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_OmniClassClassDefinition, ClassificationSystemClassDefinition);
    private:
        BE_PROP_NAME(Name)
        BE_PROP_NAME(OmniClassID)
        BE_PROP_NAME(Description)

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT OmniClassClassDefinition(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT OmniClassClassDefinition(CreateParams const& params, Utf8CP name, Utf8CP omniClassID, Utf8CP description);
        friend struct OmniClassClassDefinitionHandler;
        friend struct ClassificationSystemsDomain;

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates a OmniClassClassDefinition
        //! @param[in]  db              db to insert class definition in
        //! @param[in]  name            name of the OmniClass Class definition
        //! @param[in]  omniClassID     ID of the OmniClass Class definition
        //! @param[in]  description     Description of the OmniClass Class definition
        //! @return     a ptr to created OmniClassClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static OmniClassClassDefinitionPtr Create(Dgn::DgnDbR db, Utf8CP name, Utf8CP omniClassID, Utf8CP description);

        //! Sets the name of this OmniClassClassDefinition
        //! @param[in]  name            new name for this OmniClassClassDefinition
        void SetName(Utf8CP name) { SetPropertyValue(prop_Name(), name); }

        //! Sets the ID of this OmniClassClassDefinition
        //! @param[in]  omniClassID     new ID for this OmniClassClassDefinition
        void SetOmniClassID(Utf8CP omniClassID) { SetPropertyValue(prop_OmniClassID(), omniClassID); }

        //! Sets the description of this OmniClassClassDefinition
        //! @param[in]  description     new description for this OmniClassClassDefinition
        void SetDescription(Utf8CP description) { SetPropertyValue(prop_Description(), description); }

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(OmniClassClassDefinition, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        //! Gets the name of this OmniClassClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetPropertyValueString(prop_Name()).c_str(); }

        //! Gets the ID of this OmniClassClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetOmniClassID() const { return GetPropertyValueString(prop_OmniClassID()).c_str(); }

        //! Gets the description of this OmniClassClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetDescription() const { return GetPropertyValueString(prop_Description()).c_str(); }

    };

//=======================================================================================
//! A ASHRAE Class Definition element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ASHRAEClassDefinition : ClassificationSystemClassDefinition
    {
    DEFINE_T_SUPER(ClassificationSystemClassDefinition);
    private:
        BE_PROP_NAME(Name)
        BE_PROP_NAME(Category)
    protected:
        
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ASHRAEClassDefinition(CreateParams const& params) : T_Super(params) {}

        //! Sets the name of this ASHRAEClassDefinition
        //! @param[in]  name   new name for this ASHRAEClassDefinition
        void SetName(Utf8CP name) { SetPropertyValue(prop_Name(), name); }

        //! Sets the Category of this ASHRAEClassDefinition
        //! @param[in]  Category   new Category for this ASHRAEClassDefinition
        void SetCategory(Utf8CP Category) { SetPropertyValue(prop_Category(), Category); }

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ASHRAEClassDefinition, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        //! Gets the name of this ASHRAEClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetPropertyValueString(prop_Name()).c_str(); }

        //! Gets the Category of this ASHRAEClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetCategory() const { return GetPropertyValueString(prop_Category()).c_str(); }
    };
//=======================================================================================
//! A ASHRAE2004 Class Definition element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ASHRAE2004ClassDefinition : ASHRAEClassDefinition
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ASHRAE2004ClassDefinition, ASHRAEClassDefinition);
    private:

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ASHRAE2004ClassDefinition(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ASHRAE2004ClassDefinition(CreateParams const& params, Utf8CP name, Utf8CP Category);
        friend struct ASHRAE2004ClassDefinitionHandler;
        friend struct ClassificationSystemsDomain;

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates a ASHRAE2004ClassDefinition
        //! @param[in]  db          db to insert class definition in
        //! @param[in]  name        name of the ASHRAE2004 Class Definition
        //! @param[in]  Category    Category of the ASHRAE2004 Class Definition
        //! @return     a ptr to created ASHRAE2004ClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ASHRAE2004ClassDefinitionPtr Create(Dgn::DgnDbR db, Utf8CP name, Utf8CP Category);
    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ASHRAE2004ClassDefinition, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)
    };
//=======================================================================================
//! A ASHRAE2010 Class Definition element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ASHRAE2010ClassDefinition : ASHRAEClassDefinition
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ASHRAE2010ClassDefinition, ASHRAEClassDefinition);
    private:

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ASHRAE2010ClassDefinition(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ASHRAE2010ClassDefinition(CreateParams const& params, Utf8CP name, Utf8CP Category);
        friend struct ASHRAE2010ClassDefinitionHandler;
        friend struct ClassificationSystemsDomain;

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates a ASHRAE2010ClassDefinition
        //! @param[in]  db          db to insert class definition in
        //! @param[in]  name        name of the ASHRAE2010 Class Definition
        //! @param[in]  Category    Category of the ASHRAE2010 Class Definition
        //! @return     a ptr to created CIBSEClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ASHRAE2010ClassDefinitionPtr Create(Dgn::DgnDbR db, Utf8CP name, Utf8CP Category);
    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ASHRAE2010ClassDefinition, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)
    };

//=======================================================================================
//! A MasterFormat Class Definition element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE MasterFormatClassDefinition : ClassificationSystemClassDefinition
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_MasterFormatClassDefinition, ClassificationSystemClassDefinition);
    private:
        BE_PROP_NAME(Name)
        BE_PROP_NAME(Description)

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT MasterFormatClassDefinition(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT MasterFormatClassDefinition(CreateParams const& params, Utf8CP name, Utf8CP description);
        friend struct MasterFormatClassDefinitionHandler;
        friend struct ClassificationSystemsDomain;

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates a MasterFormatClassDefinition
        //! @param[in]  db          db to insert class definition in
        //! @param[in]  name        name of the MasterFormat Class definition
        //! @param[in]  description description of the MasterFormat Class definition
        //! @return     a ptr to created MasterFormatClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static MasterFormatClassDefinitionPtr Create(Dgn::DgnDbR db, Utf8CP name, Utf8CP description);

        //! Sets the name of this MasterFormatClassDefinition
        //! @param[in]  name   new name for this MasterFormatClassDefinition
        void SetName(Utf8CP name) { SetPropertyValue(prop_Name(), name); }

        //! Sets the Category of this MasterFormatClassDefinition
        //! @param[in]  description   new description for this MasterFormatClassDefinition
        void SetDescription(Utf8CP description) { SetPropertyValue(prop_Description(), description); }

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(MasterFormatClassDefinition, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        //! Gets the name of this MasterFormatClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetPropertyValueString(prop_Name()).c_str(); }

        //! Gets the description of this MasterFormatClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetDescription() const { return GetPropertyValueString(prop_Description()).c_str(); }

    };

//=======================================================================================
//! A UniFormat Class Definition element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE UniFormatClassDefinition : ClassificationSystemClassDefinition
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_UniFormatClassDefinition, ClassificationSystemClassDefinition);
    private:
        BE_PROP_NAME(Name)
        BE_PROP_NAME(Description)

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT UniFormatClassDefinition(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT UniFormatClassDefinition(CreateParams const& params, Utf8CP name, Utf8CP description);
        friend struct UniFormatClassDefinitionHandler;
        friend struct ClassificationSystemsDomain;

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates a CIBSEClassDefinition
        //! @param[in]  db          db to insert class definition in
        //! @param[in]  name        name of the UniFormat Class definition
        //! @param[in]  description description of the UniFormat Class definition
        //! @return     a ptr to created UniFormatClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static UniFormatClassDefinitionPtr Create(Dgn::DgnDbR db, Utf8CP name, Utf8CP description);

        //! Sets the name of this UniFormatClassDefinition
        //! @param[in]  name   new name for this UniFormatClassDefinition
        void SetName(Utf8CP name) { SetPropertyValue(prop_Name(), name); }

        //! Sets the description of this UniFormatClassDefinition
        //! @param[in]  description   new Category for this UniFormatClassDefinition
        void SetDescription(Utf8CP description) { SetPropertyValue(prop_Description(), description); }

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(UniFormatClassDefinition, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        //! Gets the name of this UniFormatClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetPropertyValueString(prop_Name()).c_str(); }

        //! Gets the description of this UniFormatClassDefinition
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetDescription() const { return GetPropertyValueString(prop_Description()).c_str(); }

    };
END_CLASSIFICATIONSYSTEMS_NAMESPACE
