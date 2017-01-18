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

    CodeFragmentSpecList fragmentSpecs = codeSpec.GetFragmentSpecs();
    CodeFragmentStringList fragmentStrings;
    fragmentStrings.resize(fragmentSpecs.size());
    int i = 0;

    for (CodeFragmentSpecCR fragmentSpec : codeSpec.GetFragmentSpecs())
        {
        DgnDbStatus status = DgnDbStatus::BadRequest;
        Utf8String fragmentString;

        switch (fragmentSpec.GetType())
            {
            case CodeFragmentSpec::Type::FixedString:
                fragmentString = fragmentSpec.GetFixedString();
                status = DgnDbStatus::Success;
                break;

            case CodeFragmentSpec::Type::SequenceNumber:
                {
                Utf8String sequenceMask;
                status = _BuildSequenceMask(sequenceMask, codeSpec, fragmentStrings);
                if (DgnDbStatus::Success == status)
                    status = _GetNextSequenceNumber(fragmentString, element, fragmentSpec, codeSpec.GetScope(), sequenceMask);
                break;
                }

            case CodeFragmentSpec::Type::ElementTypeCode:
                status = _GetElementTypeCode(fragmentString, element, fragmentSpec);
                break;

            case CodeFragmentSpec::Type::PropertyValue:
                status = _GetPropertyValue(fragmentString, element, fragmentSpec);
                break;

            default:
                BeAssert(false);
                break;
            }

        if (DgnDbStatus::Success != status)
            return DgnCode();

        fragmentStrings[i] = fragmentString;
        i++;
        }

    Utf8String codeValue;
    for (Utf8String fragmentString : fragmentStrings)
        codeValue.append(fragmentString);

    DgnElementId scopeElementId = GetCodeScopeElementId(element, codeSpec.GetScope());
    if (scopeElementId.IsValid())
        return DgnCode(codeSpec.GetCodeSpecId(), codeValue, scopeElementId);

    BeAssert(false);
    return DgnCode();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus DgnPlatformLib::Host::CodeAdmin::_BuildSequenceMask(Utf8StringR sequenceMask, CodeSpecCR codeSpec, CodeFragmentStringListCR fragmentStrings) const
    {
    int i = 0;
    for (CodeFragmentSpecCR fragmentSpec : codeSpec.GetFragmentSpecs())
        {
        if (fragmentSpec.IsInSequenceMask())
            {
            sequenceMask.append(fragmentStrings[i]);
            }
        else
            {
            for (int j=0; j<fragmentSpec.GetMinChars(); j++)
                sequenceMask.append("?");

            if (fragmentSpec.GetMinChars() < fragmentSpec.GetMaxChars())
                sequenceMask.append("*");
            }

        i++;
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus DgnPlatformLib::Host::CodeAdmin::_GetNextSequenceNumber(Utf8StringR, DgnElementCR, CodeFragmentSpecCR, CodeScopeSpecCR, Utf8StringCR) const
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
DgnElementId DgnPlatformLib::Host::CodeAdmin::GetCodeScopeElementId(DgnElementCR element, CodeScopeSpecCR codeScopeSpec) const
    {
    switch (codeScopeSpec.GetType())
        {
        case CodeScopeSpec::Type::Repository:            
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
