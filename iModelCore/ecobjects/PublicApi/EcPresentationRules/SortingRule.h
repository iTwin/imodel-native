/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/SortingRule.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Sorting rule for defining sorting type of ECInstances. If no rule is defined, ECInstances
will be sorted using natural (alphanumeric) sorting by default.
* @bsiclass                                     Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct SortingRule : public PresentationRule
    {
    private:
        WString m_schemaName;
        WString m_className;
        WString m_propertyName;
        bool    m_sortAscending;
        bool    m_doNotSort;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP   _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool     _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void     _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT SortingRule ();

        //! Constructor.
        ECOBJECTS_EXPORT SortingRule (WStringCR condition, int priority, WStringCR schemaName, WStringCR className, WStringCR propertyName, bool sortAscending, bool doNotSort);

        //! SchemaName of ECInstances to which sorting configuration should be applied.
        ECOBJECTS_EXPORT WStringCR        GetSchemaName (void) const;

        //! ClassName of ECInstances to which sorting configuration should be applied.
        ECOBJECTS_EXPORT WStringCR        GetClassName (void) const;

        //! PropertyName of ECInstances by which sorting should be executed.
        ECOBJECTS_EXPORT WStringCR        GetPropertyName (void) const;

        //! Direction of sorting. If false, then descending order will be used.
        ECOBJECTS_EXPORT bool             GetSortAscending (void) const;

        //! Identifies whether sort or not ECInstances. If true, then ECInstances will be listed
        //! in the order they were stored, or the order PersistenceProvider returns them.
        ECOBJECTS_EXPORT bool             GetDoNotSort (void) const;

    };

END_BENTLEY_ECOBJECT_NAMESPACE