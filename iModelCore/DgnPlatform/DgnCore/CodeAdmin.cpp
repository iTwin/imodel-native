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

    return NullAuthority::GetNullAuthorityId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    12/2016
//---------------------------------------------------------------------------------------
DgnDbStatus DgnElement::GenerateCode(bool replaceExistingCode)
    {
    if (GetCode().IsValid() && !replaceExistingCode)
        return DgnDbStatus::BadRequest;

    CodeSpecId codeSpecId = T_HOST.GetCodeAdmin()._GetDefaultCodeSpecId(GetDgnDb(), *GetElementClass());
    CodeSpecCPtr codeSpec = GetDgnDb().Authorities().GetAuthority(codeSpecId);
    if (!codeSpec.IsValid())
        return DgnDbStatus::InvalidCodeAuthority;

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

    Utf8String codeValue;
    for (CodeFragmentSpecCR fragmentSpec : codeSpec.GetFragmentSpecs())
        {
        Utf8String fragmentString;
        if (DgnDbStatus::Success != GenerateCodeFragment(fragmentString, element, codeSpec, fragmentSpec))
            return DgnCode();

        codeValue.append(fragmentString);
        }

    DgnElementId scopeElementId = GetCodeScopeElementId(element, codeSpec.GetScope());
    if (scopeElementId.IsValid())
        return DgnCode(codeSpec.GetAuthorityId(), codeValue, scopeElementId);

    BeAssert(false);
    return DgnCode();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus DgnPlatformLib::Host::CodeAdmin::GenerateCodeFragment(Utf8StringR fragmentString, DgnElementCR element, CodeSpecCR codeSpec, CodeFragmentSpecCR codeFragmentSpec) const
    {
    switch (codeFragmentSpec.GetType())
        {
        case CodeFragmentSpec::Type::FixedString:
            fragmentString = codeFragmentSpec.GetFixedString();
            return DgnDbStatus::Success;

        case CodeFragmentSpec::Type::SequenceNumber:
            return _GetNextSequenceNumber(fragmentString, element, codeSpec, codeFragmentSpec);

        case CodeFragmentSpec::Type::ElementClass:
            return _GetElementClassName(fragmentString, element, codeFragmentSpec);

        case CodeFragmentSpec::Type::PropertyValue:
            return _GetPropertyValue(fragmentString, element, codeFragmentSpec);

        default:
            BeAssert(false);
            return DgnDbStatus::BadRequest;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus DgnPlatformLib::Host::CodeAdmin::_GetNextSequenceNumber(Utf8StringR fragmentString, DgnElementCR element, CodeSpecCR codeSpec, CodeFragmentSpecCR fragmentSpec) const
    {
    BeAssert(false);
    return DgnDbStatus::BadRequest; // proper implementation requires an external service
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus DgnPlatformLib::Host::CodeAdmin::_GetElementClassName(Utf8StringR className, DgnElementCR element, CodeFragmentSpecCR fragmentSpec) const
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
DgnElementId DgnPlatformLib::Host::CodeAdmin::GetCodeScopeElementId(DgnElementCR element, CodeScopeSpecCR codeScopeSpec) const
    {
    switch (codeScopeSpec.GetType())
        {
        case CodeScopeSpec::Type::DgnDb:            
            return element.GetDgnDb().Elements().GetRootSubjectId();          

        case CodeScopeSpec::Type::Model:            
            return element.GetModel()->GetModeledElementId(); 

        case CodeScopeSpec::Type::ParentElement:    
            return element.GetParentId();                     

        default:
            BeAssert(false);                                            
            return DgnElementId();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus DgnElement::ValidateCode() const
    {
    CodeSpecCPtr codeSpec = GetCodeAuthority();
    if (codeSpec.IsNull() || !SupportsCodeAuthority(*codeSpec))
        return DgnDbStatus::InvalidCodeAuthority;

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
