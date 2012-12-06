/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/SortingRule.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/
/*---------------------------------------------------------------------------------**//**
Sorting rule for defining sorting type of ECInstances. If no rule is defined, ECInstances
will be sorted using natural (alphanumeric) sorting by default.
* @bsiclass                                     Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct SortingRule : public PresentationRule
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString m_schemaName;
        WString m_className;
        WString m_propertyName;
        bool    m_sortAscending;
        bool    m_doNotSort;

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual CharCP   _GetXmlElementName ();
        ECOBJECTS_EXPORT virtual bool     _ReadXml (BeXmlNodeP xmlNode) override;
        ECOBJECTS_EXPORT virtual void     _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        ECOBJECTS_EXPORT SortingRule ()
            : PresentationRule (), m_schemaName (L""), m_className (L""), m_propertyName (L""), m_sortAscending (true), m_doNotSort (false)
            {
            }

        ECOBJECTS_EXPORT SortingRule (WStringCR condition, int priority, WStringCR schemaName, WStringCR className, WStringCR propertyName, bool sortAscending, bool doNotSort)
            : PresentationRule (condition, priority, false), 
              m_schemaName (schemaName), m_className (className), m_propertyName (propertyName), m_sortAscending (sortAscending), m_doNotSort (doNotSort)
            {
            }

        //! SchemaName of ECInstances to which sorting configuration should be applied.
        ECOBJECTS_EXPORT WStringCR        GetSchemaName (void) const      { return m_schemaName; }

        //! ClassName of ECInstances to which sorting configuration should be applied.
        ECOBJECTS_EXPORT WStringCR        GetClassName (void) const       { return m_className; }

        //! PropertyName of ECInstances by which sorting should be executed.
        ECOBJECTS_EXPORT WStringCR        GetPropertyName (void) const    { return m_propertyName; }

        //! Direction of sorting. If false, then descending order will be used.
        ECOBJECTS_EXPORT bool             GetSortAscending (void) const   { return m_sortAscending; }

        //! Identifies whether sort or not ECInstances. If true, then ECInstances will be listed
        //! in the order they were stored, or the order PersistenceProvider returns them.
        ECOBJECTS_EXPORT bool             GetDoNotSort (void) const       { return m_doNotSort; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE