/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRulesTypes.h>
#include <ECPresentation/RulesDriven/Rules/RelationshipPathSpecification.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelatedInstanceSpecification : HashableBase
{
private:
    RelationshipPathSpecification m_relationshipPath;
    Utf8String m_alias;
    bool m_isRequired;
        
protected:
    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

public:
    RelatedInstanceSpecification() : m_isRequired(false) {}
    RelatedInstanceSpecification(RequiredRelationDirection direction, Utf8String relationshipName, Utf8String className, Utf8String alias, bool isRequired = false)
        : m_alias(alias), m_isRequired(isRequired)
        {
        m_relationshipPath.AddStep(*new RelationshipStepSpecification(relationshipName, direction, className));
        }
    RelatedInstanceSpecification(RelationshipPathSpecification relationshipPath, Utf8String alias, bool isRequired = false)
        : m_relationshipPath(relationshipPath), m_alias(alias), m_isRequired(isRequired)
        {}

    //! Does shallow comparison between this specification and other specification
    ECPRESENTATION_EXPORT bool ShallowEqual(RelatedInstanceSpecificationCR other) const;

    //! Reads specification from XML.
    ECPRESENTATION_EXPORT bool ReadXml(BeXmlNodeP xmlNode);

    //! Writes the specification to xml node.
    ECPRESENTATION_EXPORT void WriteXml(BeXmlNodeP parentXmlNode) const;

    //! Reads specification from Json.
    ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);

    //! Reads specification from Json.
    ECPRESENTATION_EXPORT Json::Value WriteJson() const;

    //! Get specification of the relationship path to the related instance
    RelationshipPathSpecification const& GetRelationshipPath() const {return m_relationshipPath;}
    RelationshipPathSpecification& GetRelationshipPath() {return m_relationshipPath;}

    //! Related instance alias which will be used to access properties of this instance. Must be unique per parent specification.
    Utf8StringCR GetAlias() const {return m_alias;}
        
    //! @see GetAlias
    void SetAlias(Utf8String alias) {m_alias = alias;}

    //! Get whether related instance is required
    bool IsRequired() const {return m_isRequired;}

    //! @see IsRequired
    void SetIsRequired(bool value) {m_isRequired = value;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
