/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/CodeAdmin.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/CodeAdmin.h>
#include <WebServices/iModelHub/Client/iModelManager.h>

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
// @bsimethod                                   Algirdas.Mikoliunas             03/2017
//---------------------------------------------------------------------------------------
DgnDbStatus CodeAdmin::GenerateCodeValue
(
CodeSpecCR codeSpec,
Utf8StringCR generatedSequenceNumber,
Utf8StringR formattedValue
) const
    {
    for (CodeFragmentSpecCR fragmentSpec : codeSpec.GetFragmentSpecs())
        {
        switch (fragmentSpec.GetType())
            {
            case CodeFragmentSpec::Type::FixedString:
                formattedValue.append(fragmentSpec.GetFixedString());
                break;
            case CodeFragmentSpec::Type::Sequence:
                formattedValue.append(generatedSequenceNumber);
                break;
            default:
                return DgnDbStatus::BadRequest;
            }
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Algirdas.Mikoliunas             03/2017
//---------------------------------------------------------------------------------------
DgnDbStatus CodeAdmin::GenerateMask
(
CodeSpecCR codeSpec,
Utf8StringR mask
) const
    {
    Utf8String placeholder;
    for (CodeFragmentSpecCR fragmentSpec : codeSpec.GetFragmentSpecs())
        {
        switch (fragmentSpec.GetType())
            {
            case CodeFragmentSpec::Type::FixedString:
                mask.append(fragmentSpec.GetFixedString());
                break;
            case CodeFragmentSpec::Type::Sequence:
                placeholder = Utf8String(fragmentSpec.GetMinChars(), '#');
                mask.append(placeholder);
                break;
            default:
                return DgnDbStatus::BadRequest;
            }
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Algirdas.Mikoliunas             03/2017
//---------------------------------------------------------------------------------------
DgnCode CodeAdmin::_ReserveNextCodeInSequence(DgnElementCR element, CodeSpecCR codeSpec, Utf8StringCR sequenceMask) const
    {
    DgnDbR dgndb = element.GetDgnDb();
    iModelManager* imodelManager = static_cast<iModelManager*>(T_HOST.GetRepositoryAdmin()._GetRepositoryManager(dgndb));
    auto imodelConnection = imodelManager->GetiModelConnectionPtr();
    
    // Query next available code 
    auto queryResult = imodelConnection->QueryCodeNextAvailable(codeSpec)->GetResult();
    if (!queryResult.IsSuccess())
        {
        BeAssert(false);
        return DgnCode();
        }
    auto templateResult = queryResult.GetValue();
    if (Utf8String::IsNullOrEmpty(templateResult.GetValue().c_str()))
        {
        BeAssert(false);
        return DgnCode();
        }

    // Generate next available code locally
    Utf8String generatedValue;
    if (DgnDbStatus::Success != GenerateCodeValue(codeSpec, templateResult.GetValue(), generatedValue))
        {
        BeAssert(false);
        return DgnCode();
        }

    auto generatedCode = DgnCode(templateResult.GetCodeSpecId(), generatedValue, templateResult.GetScope());

    // Reserve code
    DgnDbStatus result = this->_ReserveCode(element, generatedCode);
    if (DgnDbStatus::Success != result)
        {
        BeAssert(false);
        return DgnCode();
        }
        
    return generatedCode;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Algirdas.Mikoliunas             03/2017
//---------------------------------------------------------------------------------------
DgnDbStatus CodeAdmin::_ReserveCode(DgnElementCR element, DgnCodeCR codeToReserve) const
    {
    DgnDbR dgndb = element.GetDgnDb();
    iModelManager* imodelManager = static_cast<iModelManager*>(T_HOST.GetRepositoryAdmin()._GetRepositoryManager(dgndb));

    IBriefcaseManager::Request request;
    request.Codes().insert(codeToReserve);

    RepositoryStatus result = imodelManager->Acquire(request, dgndb).Result();
    if (RepositoryStatus::Success != result)
        {
        return DgnDbStatus::CodeNotReserved;
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Algirdas.Mikoliunas             03/2017
//---------------------------------------------------------------------------------------
DgnDbStatus CodeAdmin::_RegisterDefaultCodeSpec(Utf8CP className, Utf8CP codeSpecName)
    {
    m_classToCodeSpecMap[className] = codeSpecName;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Algirdas.Mikoliunas             03/2017
//---------------------------------------------------------------------------------------
CodeSpecId CodeAdmin::_GetDefaultCodeSpecId(DgnDbR db, ECN::ECClassCR inputClass) const
    {
    if (!inputClass.Is(BIS_ECSCHEMA_NAME, BIS_CLASS_Element))
        return CodeSpecId();

    Utf8PrintfString className("%s.%s", inputClass.GetSchema().GetName().c_str(), inputClass.GetName().c_str());
    auto found = m_classToCodeSpecMap.find(className.c_str());
    if (m_classToCodeSpecMap.end() != found)
        return db.CodeSpecs().QueryCodeSpecId(found->second);

    if (inputClass.HasBaseClasses())
        return _GetDefaultCodeSpecId(db, *inputClass.GetBaseClasses()[0]);

    return T_Super::_GetDefaultCodeSpecId(db, inputClass);
    }
