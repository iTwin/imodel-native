/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECPresentationRules/PropertyEditorsSpecification.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Specification for specifying editor for a single property.
* @bsiclass                                     Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyEditorsSpecification
    {
    private:
        Utf8String m_propertyName;
        Utf8String m_editorName;

    public:
        PropertyEditorsSpecification() {}
        PropertyEditorsSpecification(Utf8String propertyName, Utf8String editorName)
            : m_propertyName(propertyName), m_editorName(editorName)
            {}

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT bool ReadXml(BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT void WriteXml(BeXmlNodeP parentXmlNode) const;

        //! Get property name.
        Utf8StringCR GetPropertyName() const {return m_propertyName;}

        //! Get editor name.
        Utf8StringCR GetEditorName() const {return m_editorName;}
    };

END_BENTLEY_ECOBJECT_NAMESPACE