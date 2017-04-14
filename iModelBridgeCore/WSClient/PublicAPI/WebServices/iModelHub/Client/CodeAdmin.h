/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/CodeAdmin.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <DgnPlatform/DgnPlatformLib.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
// @bsiclass                                      Algirdas.Mikoliunas            03/2017
//=======================================================================================
struct CodeAdmin : public DgnPlatformLib::Host::CodeAdmin
{
    DEFINE_T_SUPER(DgnPlatformLib::Host::CodeAdmin)
private:
    bmap<Utf8CP, Utf8CP, ECN::less_str> m_classToCodeSpecMap;
	
    DgnDbStatus GenerateCodeValue (CodeSpecCR codeSpec, Utf8StringCR generatedSequenceNumber, Utf8StringR formattedValue) const;
    DgnDbStatus GenerateMask(CodeSpecCR codeSpec, Utf8StringR mask) const;
public:
    IMODELHUBCLIENT_EXPORT DgnCode _ReserveNextCodeInSequence(DgnElementCR element, CodeSpecCR codeSpec, Utf8StringCR sequenceMask) const override;
    IMODELHUBCLIENT_EXPORT DgnDbStatus _ReserveCode(DgnElementCR element, DgnCodeCR codeToReserve) const override;
    IMODELHUBCLIENT_EXPORT DgnDbStatus _RegisterDefaultCodeSpec(Utf8CP className, Utf8CP codeSpecName) override;
    IMODELHUBCLIENT_EXPORT CodeSpecId _GetDefaultCodeSpecId(DgnDbR db, ECN::ECClassCR inputClass) const override;
};

END_BENTLEY_IMODELHUB_NAMESPACE
