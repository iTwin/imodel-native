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

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

//=======================================================================================
//! A ClassificationSystem Class Definition element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ClassificationSystemClassDefinition : Dgn::DefinitionElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystemClassDefinition, Dgn::DefinitionElement);
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
        
    };
END_CLASSIFICATIONSYSTEMS_NAMESPACE
