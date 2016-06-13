/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/SortingRule.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Sorting rule for defining sorting type of ECInstances. If no rule is defined, ECInstances
will be sorted using natural (alphanumeric) sorting by default.
* @bsiclass                                     Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct SortingRule : public PresentationRule
    {
    private:
        Utf8String m_schemaName;
        Utf8String m_className;
        Utf8String m_propertyName;
        bool    m_sortAscending;
        bool    m_doNotSort;
        bool    m_isPolymorphic;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP   _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool     _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void     _WriteXml (BeXmlNodeP xmlNode) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT SortingRule ();

        //! Constructor.
        ECOBJECTS_EXPORT SortingRule (Utf8StringCR condition, int priority, Utf8StringCR schemaName, Utf8StringCR className, Utf8StringCR propertyName, bool sortAscending, bool doNotSort, bool isPolymorphic);

        //! SchemaName of ECInstances to which sorting configuration should be applied.
        ECOBJECTS_EXPORT Utf8StringCR     GetSchemaName (void) const;

        //! ClassName of ECInstances to which sorting configuration should be applied.
        ECOBJECTS_EXPORT Utf8StringCR     GetClassName (void) const;

        //! PropertyName of ECInstances by which sorting should be executed.
        ECOBJECTS_EXPORT Utf8StringCR     GetPropertyName (void) const;

        //! Direction of sorting. If false, then descending order will be used.
        ECOBJECTS_EXPORT bool             GetSortAscending (void) const;

        //! Identifies whether sort or not ECInstances. If true, then ECInstances will be listed
        //! in the order they were stored, or the order PersistenceProvider returns them.
        ECOBJECTS_EXPORT bool             GetDoNotSort (void) const;

        //! Identifies whether ECClass defined in this rule should be accepted polymorphically.
        ECOBJECTS_EXPORT bool             GetIsPolymorphic (void) const;

    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
