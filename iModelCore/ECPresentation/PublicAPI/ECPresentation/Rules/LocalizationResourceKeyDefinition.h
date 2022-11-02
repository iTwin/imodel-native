/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Implementation of localization resource key definition. This rule is used to define
additional settings for the resource key.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalizationResourceKeyDefinition : PrioritizedPresentationKey
{
    DEFINE_T_SUPER(PrioritizedPresentationKey)

private:
    Utf8String m_id;
    Utf8String m_key;
    Utf8String m_defaultValue;

protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
    ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    Utf8CP _GetJsonElementType() const override {return "LocalizationResourceKeyDefinition";}
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;

public:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT LocalizationResourceKeyDefinition ();

    //! Constructor.
    ECPRESENTATION_EXPORT LocalizationResourceKeyDefinition (int priority, Utf8StringCR id, Utf8StringCR key, Utf8StringCR defaultValue);

    //! Id of the resource to localize.
    ECPRESENTATION_EXPORT Utf8StringCR    GetId (void) const;

    //! New key of the resource.
    ECPRESENTATION_EXPORT Utf8StringCR    GetKey (void) const;

    //! Default value which is used if the resource is not found.
    ECPRESENTATION_EXPORT Utf8StringCR    GetDefaultValue (void) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
