/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/SpecificationVisitor.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Interface for presentation rule specification visitor.
* @bsiclass                                     Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct PresentationRuleSpecificationVisitor
{
    friend struct SelectedNodeInstancesSpecification;
    friend struct ContentInstancesOfSpecificClassesSpecification;
    friend struct ContentRelatedInstancesSpecification;
    friend struct AllInstanceNodesSpecification;
    friend struct AllRelatedInstanceNodesSpecification;
    friend struct CustomNodeSpecification;
    friend struct InstanceNodesOfSpecificClassesSpecification;
    friend struct RelatedInstanceNodesSpecification;
    friend struct SearchResultInstanceNodesSpecification;

protected:
    virtual ~PresentationRuleSpecificationVisitor() {}

/** @name Content rule specifications */
/** @{ */
    virtual void _Visit(SelectedNodeInstancesSpecification const& specification) {}
    virtual void _Visit(ContentInstancesOfSpecificClassesSpecification const& specification) {}
    virtual void _Visit(ContentRelatedInstancesSpecification const& specification) {}
/** @} */

/** @name Navigation rule specifications */
/** @{ */
    virtual void _Visit(AllInstanceNodesSpecification const& specification) {}
    virtual void _Visit(AllRelatedInstanceNodesSpecification const& specification) {}
    virtual void _Visit(CustomNodeSpecification const& specification) {}
    virtual void _Visit(InstanceNodesOfSpecificClassesSpecification const& specification) {}
    virtual void _Visit(RelatedInstanceNodesSpecification const& specification) {}
    virtual void _Visit(SearchResultInstanceNodesSpecification const& specification) {}
/** @} */
};

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
