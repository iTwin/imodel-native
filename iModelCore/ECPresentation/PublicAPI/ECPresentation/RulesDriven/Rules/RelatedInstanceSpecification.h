/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRulesTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelatedInstanceSpecification : HashableBase
    {
    private:
        RequiredRelationDirection          m_direction;
        Utf8String                         m_relationshipName;
        Utf8String                         m_className;
        Utf8String                         m_alias;
        bool                               m_isRequired;
        
    protected:
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Constructor.
        RelatedInstanceSpecification() : m_isRequired(false), m_direction(RequiredRelationDirection_Both) {}
        
        //! Constructor.
        RelatedInstanceSpecification(RequiredRelationDirection direction, Utf8String relationshipName, Utf8String className, Utf8String alias, bool isRequired = false)
            : m_direction(direction), m_relationshipName(relationshipName), m_className(className), m_alias(alias), m_isRequired(isRequired)
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

        //! Returns direction of relationship that should be selected in the query.
        RequiredRelationDirection GetRelationshipDirection() const {return m_direction;}

        //! Relationship class name. Format: "SchemaName:ClassName"
        Utf8StringCR GetRelationshipName() const {return m_relationshipName;}

        //! Class name. Format: "SchemaName:ClassName"
        Utf8StringCR GetClassName() const {return m_className;}

        //! Set class name. Format: "SchemaName:ClassName"
        void SetClassName(Utf8String className) {m_className = className;}

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
