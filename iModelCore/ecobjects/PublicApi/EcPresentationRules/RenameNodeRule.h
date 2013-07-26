/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/RenameNodeRule.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Presentation rule for configuring node rename functionality.
* @bsiclass                                    dmitrijus.tiazlovas                11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct RenameNodeRule : public PresentationRule
    {
    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP      _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool        _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void        _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT RenameNodeRule ();

        //! Constructor.
        ECOBJECTS_EXPORT RenameNodeRule (WStringCR condition, int priority);
    };

END_BENTLEY_ECOBJECT_NAMESPACE