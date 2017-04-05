/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbCodeAdmin.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnPlatform/DgnPlatformLib.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
// @bsiclass                                      Algirdas.Mikoliunas            03/2017
//=======================================================================================
struct DgnDbCodeAdmin : public DgnPlatformLib::Host::CodeAdmin
{
    DEFINE_T_SUPER(DgnPlatformLib::Host::CodeAdmin)
private:
    bmap<Utf8CP, Utf8CP, ECN::less_str> m_classToCodeSpecMap;
	
    DgnDbStatus GenerateCodeValue (CodeSpecCR codeSpec, Utf8StringCR generatedSequenceNumber, Utf8StringR formattedValue) const;
    DgnDbStatus GenerateMask(CodeSpecCR codeSpec, Utf8StringR mask) const;
public:
    DGNDBSERVERCLIENT_EXPORT DgnCode _ReserveNextCodeInSequence(DgnElementCR element, CodeSpecCR codeSpec, Utf8StringCR sequenceMask) const override;
    DGNDBSERVERCLIENT_EXPORT DgnDbStatus _ReserveCode(DgnElementCR element, DgnCodeCR codeToReserve) const override;
    DGNDBSERVERCLIENT_EXPORT DgnDbStatus _RegisterDefaultCodeSpec(Utf8CP className, Utf8CP codeSpecName) override;
    DGNDBSERVERCLIENT_EXPORT CodeSpecId _GetDefaultCodeSpecId(DgnDbR db, ECN::ECClassCR inputClass) const override;
};

END_BENTLEY_DGNDBSERVER_NAMESPACE
