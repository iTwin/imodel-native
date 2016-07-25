/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/HiddenPropertiesSpecification.h $
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
* Specification for specifying single property that should not be displayed.
* @bsiclass                                     Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct HiddenPropertiesSpecification
    {
    private:
        Utf8String m_fullClassName;
        Utf8String m_propertyNames;

    public:
        HiddenPropertiesSpecification() {}
        HiddenPropertiesSpecification(Utf8String fullClassName, Utf8String propertyNames) 
            : m_fullClassName(fullClassName), m_propertyNames(propertyNames) 
            {}

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT bool ReadXml(BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT void WriteXml(BeXmlNodeP parentXmlNode) const;
                
        //! Get the full class name of the hidden properties.
        Utf8StringCR GetFullClassName() const {return m_fullClassName;}

        //! Get the hidden property names.
        Utf8StringCR GetPropertyNames() const {return m_propertyNames;}
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
