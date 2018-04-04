/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/PublicApi/ClassificationSystemsElements.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__

CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ClassificationSystemClassDefinition)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(CIBSEClassDefinition)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(OmniClassClassDefinition)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ASHRAEClassDefinition)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ASHRAE2004ClassDefinition)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ASHRAE2010ClassDefinition)

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

//=======================================================================================
//! A ClassificationSystem Class Definition element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ClassificationSystemClassDefinition : Dgn::DefinitionElement
    {
    DEFINE_T_SUPER(Dgn::DefinitionElement);
    private:

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationSystemClassDefinition(CreateParams const& params) : T_Super(params) {}

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationSystemClassDefinition, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)
        
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
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT CIBSEClassDefinition(CreateParams const& params, Utf8CP name, Utf8CP Category);
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
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static CIBSEClassDefinitionPtr Create(Dgn::DgnDbR db, Utf8CP name, Utf8CP Category);
    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(CIBSEClassDefinition, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        //! Sets the name of this CIBSEClassDefinition
        //! @param[in]  name   new name for this CIBSEClassDefinition
        void SetName (Utf8CP name) { SetPropertyValue(prop_Name(), name); }

        //! Sets the Category of this CIBSEClassDefinition
        //! @param[in]  Category   new Category for this CIBSEClassDefinition
        void SetCategory (Utf8CP Category) { SetPropertyValue(prop_Category(), Category); }   

        //! Gets the name of this CIBSEClassDefinition
        Utf8CP GetName() const { return GetPropertyValueString(prop_Name()).c_str(); }

        //! Gets the Category of this CIBSEClassDefinition
        Utf8CP GetCategory() const { return GetPropertyValueString(prop_Category()).c_str(); }
        
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
    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(OmniClassClassDefinition, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        //! Sets the name of this OmniClassClassDefinition
        //! @param[in]  name            new name for this OmniClassClassDefinition
        void SetName(Utf8CP name) { SetPropertyValue(prop_Name(), name); }

        //! Sets the ID of this OmniClassClassDefinition
        //! @param[in]  omniClassID     new ID for this OmniClassClassDefinition
        void SetOmniClassID(Utf8CP omniClassID) { SetPropertyValue(prop_OmniClassID(), omniClassID); }

        //! Sets the description of this OmniClassClassDefinition
        //! @param[in]  description     new description for this OmniClassClassDefinition
        void SetDescription(Utf8CP description) { SetPropertyValue(prop_Description(), description); }

        //! Gets the name of this OmniClassClassDefinition
        Utf8CP GetName() const { return GetPropertyValueString(prop_Name()).c_str(); }

        //! Gets the ID of this OmniClassClassDefinition
        Utf8CP GetOmniClassID() const { return GetPropertyValueString(prop_OmniClassID()).c_str(); }

        //! Gets the description of this OmniClassClassDefinition
        Utf8CP GetDescription() const { return GetPropertyValueString(prop_Description()).c_str(); }

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

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ASHRAEClassDefinition, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)
        //! Sets the name of this ASHRAEClassDefinition
        //! @param[in]  name   new name for this ASHRAEClassDefinition
        void SetName(Utf8CP name) { SetPropertyValue(prop_Name(), name); }

        //! Sets the Category of this ASHRAEClassDefinition
        //! @param[in]  Category   new Category for this ASHRAEClassDefinition
        void SetCategory(Utf8CP Category) { SetPropertyValue(prop_Category(), Category); }

        //! Gets the name of this ASHRAEClassDefinition
        Utf8CP GetName() const { return GetPropertyValueString(prop_Name()).c_str(); }

        //! Gets the Category of this ASHRAEClassDefinition
        Utf8CP GetCategory() const { return GetPropertyValueString(prop_Category()).c_str(); }
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
END_CLASSIFICATIONSYSTEMS_NAMESPACE
