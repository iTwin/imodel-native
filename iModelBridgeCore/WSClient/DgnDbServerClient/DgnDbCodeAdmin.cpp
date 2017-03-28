/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbCodeAdmin.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbCodeAdmin.h>
#include <DgnDbServer/Client/DgnDbRepositoryManager.h>

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
// @bsimethod                                   Algirdas.Mikoliunas             03/2017
//---------------------------------------------------------------------------------------
DgnDbStatus DgnDbCodeAdmin::GenerateCodeValue
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
DgnCode DgnDbCodeAdmin::_ReserveNextCodeInSequence(DgnElementCR element, CodeSpecCR codeSpec, Utf8StringCR sequenceMask) const
    {
#if 0
    DgnDbR dgndb = element.GetDgnDb();
    DgnDbRepositoryManager* repositoryManager = static_cast<DgnDbRepositoryManager*>(T_HOST.GetRepositoryAdmin()._GetRepositoryManager(dgndb));
    auto repositoryConnection = repositoryManager->GetRepositoryConnectionPtr();
    
    // TODO: remove when DgnPlatform will use #
    Utf8String mask(sequenceMask);
    //mask.ReplaceAll("?", "#");

    auto queryResult = repositoryConnection->QueryCodeNextAvailable(mask, *element.GetCodeSpec())->GetResult();
    if (!queryResult.IsSuccess())
        return DgnDbStatus::BadRequest;

    auto templateResult = *queryResult.GetValue().GetTemplates().begin();
    Utf8String generatedValue;

    auto elementClass = element.GetElementClass();
    CodeSpecId codeSpecId = _GetDefaultCodeSpecId(dgndb, *elementClass);
    auto codeSpec = dgndb.CodeSpecs().GetCodeSpec(codeSpecId);

    GenerateCodeValue(*codeSpec, templateResult.GetValue(), generatedValue);
    auto generatedCode = DgnCode(templateResult.GetCodeSpecId(), generatedValue, templateResult.GetScope());

    // Reserve code
    IBriefcaseManager::Request request;
    request.Codes().insert(generatedCode);
    RepositoryStatus result = repositoryManager->Acquire(request, dgndb).Result();

    if (RepositoryStatus::Success != result)
        return DgnDbStatus::BadRequest;

    fragmentString = templateResult.GetValue();
    if (Utf8String::IsNullOrEmpty(fragmentString.c_str()))
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
#else
    BeAssert(false);
    return DgnCode();
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Algirdas.Mikoliunas             03/2017
//---------------------------------------------------------------------------------------
DgnDbStatus DgnDbCodeAdmin::_RegisterDefaultCodeSpec(Utf8CP className, Utf8CP codeSpecName)
    {
    m_classToCodeSpecMap[className] = codeSpecName;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Algirdas.Mikoliunas             03/2017
//---------------------------------------------------------------------------------------
CodeSpecId DgnDbCodeAdmin::_GetDefaultCodeSpecId(DgnDbR db, ECN::ECClassCR inputClass) const
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
