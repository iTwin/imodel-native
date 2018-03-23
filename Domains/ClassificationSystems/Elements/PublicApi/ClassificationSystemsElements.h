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
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_CIBSEClassDefinition, Dgn::DefinitionElement);
    private:

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT CIBSEClassDefinition(CreateParams const& params) : T_Super(params) {}
        friend struct CIBSEClassDefinitionHandler;
        friend struct ClassificationSystemsDomain;

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates an egress path from known egress sections.
        //! @param[in]  model       model to create ClassificationSystems path in
        //! @param[in]  sections    ClassificationSystems sections as a sequence
        //! @param[in]  isPrimary   true to make this egress path primary. NOTE: This will also make all the given egress sequences primary
        //! @return     a ptr to created egress path
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationSystemsPathPtr Create(Dgn::DgnModelP model, ClassificationSystemsSectionSequence const& sections, bool isPrimary = true);
    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(CIBSEClassDefinition, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)
        
    };
END_CLASSIFICATIONSYSTEMS_NAMESPACE
