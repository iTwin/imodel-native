/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Sorting rule for defining sorting type of ECInstances. If no rule is defined, ECInstances
will be sorted using natural (alphanumeric) sorting by default.
* @bsiclass                                     Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct SortingRule : public ConditionalCustomizationRule
    {
    private:
        Utf8String m_schemaName;
        Utf8String m_className;
        Utf8String m_propertyName;
        bool    m_sortAscending;
        bool    m_doNotSort;
        bool    m_isPolymorphic;

    protected:
        ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
        ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

        ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
        ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

        //! Accepts customization rule visitor
        ECPRESENTATION_EXPORT void _Accept(CustomizationRuleVisitor& visitor) const override;

        //! Computes rule hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

        //! Clones rule.
        CustomizationRule* _Clone() const override {return new SortingRule(*this);}

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT SortingRule ();

        //! Constructor.
        ECPRESENTATION_EXPORT SortingRule (Utf8StringCR condition, int priority, Utf8StringCR schemaName, Utf8StringCR className, Utf8StringCR propertyName, bool sortAscending, bool doNotSort, bool isPolymorphic);

        //! SchemaName of ECInstances to which sorting configuration should be applied.
        ECPRESENTATION_EXPORT Utf8StringCR     GetSchemaName (void) const;

        //! ClassName of ECInstances to which sorting configuration should be applied.
        ECPRESENTATION_EXPORT Utf8StringCR     GetClassName (void) const;

        //! PropertyName of ECInstances by which sorting should be executed.
        ECPRESENTATION_EXPORT Utf8StringCR     GetPropertyName (void) const;

        //! Direction of sorting. If false, then descending order will be used.
        ECPRESENTATION_EXPORT bool             GetSortAscending (void) const;

        //! Identifies whether sort or not ECInstances. If true, then ECInstances will be listed
        //! in the order they were stored, or the order PersistenceProvider returns them.
        ECPRESENTATION_EXPORT bool             GetDoNotSort (void) const;

        //! Identifies whether ECClass defined in this rule should be accepted polymorphically.
        ECPRESENTATION_EXPORT bool             GetIsPolymorphic (void) const;

    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
