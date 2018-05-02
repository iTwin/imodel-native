/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/RenameNodeRule.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Presentation rule for configuring node rename functionality.
* @bsiclass                                    dmitrijus.tiazlovas                11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct RenameNodeRule : public ConditionalCustomizationRule
    {
    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECPRESENTATION_EXPORT virtual CharCP      _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT virtual bool        _ReadXml (BeXmlNodeP xmlNode) override;

        //! Reads rule information from Json, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT virtual bool        _ReadJson(JsonValueCR json) override;

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT virtual void        _WriteXml (BeXmlNodeP xmlNode) const override;

        //! Accept nested cutomization rule visitor
        ECPRESENTATION_EXPORT void _Accept(CustomizationRuleVisitor& visitor) const override;

        //! Compute rule hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

        //! Clones rule.
        ECPRESENTATION_EXPORT virtual CustomizationRule* _Clone() const override {return new RenameNodeRule(*this);}

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        RenameNodeRule() {}

        //! Constructor.
        ECPRESENTATION_EXPORT RenameNodeRule (Utf8StringCR condition, int priority);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
