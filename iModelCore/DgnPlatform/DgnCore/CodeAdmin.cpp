/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/CodeAdmin.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus DgnPlatformLib::Host::CodeAdmin::_RegisterDefaultCodeSpec(Utf8CP className, Utf8CP codeSpecName)
    {
    return DgnDbStatus::NotEnabled;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    12/2016
//---------------------------------------------------------------------------------------
CodeSpecId DgnPlatformLib::Host::CodeAdmin::_GetDefaultCodeSpecId(DgnDbR db, ECClassCR inputClass) const
    {
    if (!inputClass.Is(BIS_ECSCHEMA_NAME, BIS_CLASS_Element))
        return CodeSpecId();

    return CodeSpec::GetNullCodeSpecId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    12/2016
//---------------------------------------------------------------------------------------
DgnDbStatus DgnElement::GenerateCode(bool replaceExistingCode)
    {
    if (GetCode().IsValid() && !replaceExistingCode)
        return DgnDbStatus::BadRequest;

    CodeSpecId codeSpecId = T_HOST.GetCodeAdmin()._GetDefaultCodeSpecId(GetDgnDb(), *GetElementClass());
    CodeSpecCPtr codeSpec = GetDgnDb().CodeSpecs().GetCodeSpec(codeSpecId);
    if (!codeSpec.IsValid())
        return DgnDbStatus::InvalidCodeSpec;

    DgnCode code = T_HOST.GetCodeAdmin()._GenerateCode(*this, *codeSpec);
    if (!code.IsValid())
        return DgnDbStatus::ValidationFailed;

    return SetCode(code);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    12/2016
//---------------------------------------------------------------------------------------
DgnCode DgnPlatformLib::Host::CodeAdmin::_GenerateCode(DgnElementCR element, CodeSpecCR codeSpec) const
    {
    if (!codeSpec.CanGenerateCode())
        return DgnCode();

    Utf8String sequenceMask;
    bool sequenceMaskContainsWildcard = false;

    for (CodeFragmentSpecCR fragmentSpec : codeSpec.GetFragmentSpecs())
        {
        Utf8String fragmentString;

        if (!fragmentSpec.IsInSequenceMask())
            {
            sequenceMask.append("*");
            sequenceMaskContainsWildcard = true;
            continue;
            }

        switch (fragmentSpec.GetType())
            {
            case CodeFragmentSpec::Type::FixedString:
                sequenceMask.append(fragmentSpec.GetFixedString());
                break;

            case CodeFragmentSpec::Type::ElementTypeCode:
                if (DgnDbStatus::Success != _GetElementTypeCode(fragmentString, element, fragmentSpec))
                    return DgnCode();

                sequenceMask.append(fragmentString);
                break;

            case CodeFragmentSpec::Type::PropertyValue:
                if (DgnDbStatus::Success != _GetPropertyValue(fragmentString, element, fragmentSpec))
                    return DgnCode();

                sequenceMask.append(fragmentString);
                break;

            default:
                return DgnCode();
            }
        }

    if (!sequenceMaskContainsWildcard)
        return DgnCode(codeSpec.GetCodeSpecId(), codeSpec.GetScopeElementId(element), sequenceMask);

    return _ReserveNextCodeInSequence(element, codeSpec, sequenceMask);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    03/2017
//---------------------------------------------------------------------------------------
DgnCode DgnPlatformLib::Host::CodeAdmin::_ReserveNextCodeInSequence(DgnElementCR element, CodeSpecCR codeSpec, Utf8StringCR sequenceMask) const
    {
    BeAssert(false);
    return DgnCode(); // proper implementation requires an external service
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    03/2017
//---------------------------------------------------------------------------------------
DgnDbStatus DgnPlatformLib::Host::CodeAdmin::_ReserveCode(DgnElementCR element, DgnCodeCR code) const
    {
    BeAssert(false);
    return DgnDbStatus::BadRequest; // proper implementation requires an external service
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus DgnPlatformLib::Host::CodeAdmin::_GetElementTypeCode(Utf8StringR className, DgnElementCR element, CodeFragmentSpecCR fragmentSpec) const
    {
    className = element.GetElementClass()->GetName();
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus DgnPlatformLib::Host::CodeAdmin::_GetPropertyValue(Utf8StringR valueAsString, DgnElementCR element, CodeFragmentSpecCR fragmentSpec) const
    {
    ECValue value;
    if (DgnDbStatus::Success != element.GetPropertyValue(value, fragmentSpec.GetPropertyName().c_str()))
        return DgnDbStatus::InvalidName;

    if (value.IsNull() || !value.ConvertPrimitiveToString(valueAsString))
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus DgnElement::ValidateCode() const
    {
    CodeSpecCPtr codeSpec = GetCodeSpec();
    if (codeSpec.IsNull() || !SupportsCodeSpec(*codeSpec))
        return DgnDbStatus::InvalidCodeSpec;

    DgnDbStatus status = codeSpec->ValidateCode(*this); // WIP: do we need this any longer?
    if (DgnDbStatus::Success != status)
        return status;

    return T_HOST.GetCodeAdmin()._ValidateCode(GetCode(), *codeSpec);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus DgnPlatformLib::Host::CodeAdmin::_ValidateCode(DgnCodeCR code, CodeSpecCR codeSpec) const
    {
    return DgnDbStatus::Success;
    }
