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

//=======================================================================================
// @bsiclass                                      Algirdas.Mikoliunas            03/2017
//=======================================================================================
struct CodeAdmin : public Dgn::DgnPlatformLib::Host::CodeAdmin
{
    DEFINE_T_SUPER(Dgn::DgnPlatformLib::Host::CodeAdmin)
private:
    bmap<Utf8CP, Utf8CP, ECN::less_str> m_classToCodeSpecMap;
	
    Dgn::DgnDbStatus GenerateCodeValue (Dgn::CodeSpecCR codeSpec, Utf8StringCR generatedSequenceNumber, Utf8StringR formattedValue) const;
    Dgn::DgnDbStatus GenerateMask(Dgn::CodeSpecCR codeSpec, Utf8StringR mask) const;
public:
    IMODELHUBCLIENT_EXPORT Dgn::DgnCode _ReserveNextCodeInSequence(Dgn::DgnElementCR element, Dgn::CodeSpecCR codeSpec, Utf8StringCR sequenceMask) const override;
    IMODELHUBCLIENT_EXPORT Dgn::DgnDbStatus _ReserveCode(Dgn::DgnElementCR element, Dgn::DgnCodeCR codeToReserve) const override;
    IMODELHUBCLIENT_EXPORT Dgn::DgnDbStatus _RegisterDefaultCodeSpec(Utf8CP className, Utf8CP codeSpecName) override;
    IMODELHUBCLIENT_EXPORT Dgn::CodeSpecId _GetDefaultCodeSpecId(Dgn::DgnDbR db, ECN::ECClassCR inputClass) const override;
};

END_BENTLEY_IMODELHUB_NAMESPACE
