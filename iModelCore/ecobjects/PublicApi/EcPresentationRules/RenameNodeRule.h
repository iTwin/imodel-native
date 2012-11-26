/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/RenameNodeRule.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/
/*---------------------------------------------------------------------------------**//**
Presentation rule for configuring node rename functionality.
* @bsiclass                                    dmitrijus.tiazlovas                11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct RenameNodeRule : public PresentationRule
    {
    /*__PUBLISH_SECTION_END__*/

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual CharCP      _GetXmlElementName ();
        ECOBJECTS_EXPORT virtual bool        _ReadXml (BeXmlNodeP xmlNode) override;
        ECOBJECTS_EXPORT virtual void        _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        ECOBJECTS_EXPORT RenameNodeRule ()
            : PresentationRule ()
            {
            }

        ECOBJECTS_EXPORT RenameNodeRule (WStringCR condition, int priority)
            : PresentationRule (condition, priority, false)
            {
            }
    };

END_BENTLEY_ECOBJECT_NAMESPACE