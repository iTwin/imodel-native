/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "Utils.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <WebServices/Cache/Util/JsonUtil.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_SQLITE

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

//=======================================================================================
//@bsiclass                                      Karolis.Dziedzelis             11/2015
//=======================================================================================
struct LocationsAdmin : public Dgn::DgnPlatformLib::Host::IKnownLocationsAdmin
    {
    private:
        BeFileName m_temp;
        BeFileName m_assets;
    protected:
        virtual BeFileNameCR _GetLocalTempDirectoryBaseName() override { return m_temp; };
        virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() override { return m_assets; };
    public:
        LocationsAdmin(BeFileNameCR temp, BeFileNameCR assets) : m_temp(temp), m_assets(assets) {}
        virtual ~LocationsAdmin() {}
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2017
//---------------------------------------------------------------------------------------
Utf8String FormatBeInt64Id(BeInt64Id int64Value)
    {
    return int64Value.ToString(BeInt64Id::UseHex::Yes);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
bool BeInt64IdFromJson(BeInt64Id& id, RapidJsonValueCR value)
    {
    if (value.IsNull())
        return false;

    if (value.IsString())
        {
        uint64_t idParsed;
        if (SUCCESS != BeStringUtilities::ParseUInt64(idParsed, value.GetString()))
            return false;
        id = BeInt64Id(idParsed);
        }
    else
        id = BeInt64Id((uint64_t)value.GetInt64());

    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
bool StringFromJson(Utf8StringR result, RapidJsonValueCR value)
    {
    if (value.IsNull())
        return false;

    result = value.GetString();
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas              10/2019
//---------------------------------------------------------------------------------------
bool TryParseStringProperty(Utf8StringR result, RapidJsonValueCR propertiesInstance, Utf8CP propertyName)
    {
    if (!propertiesInstance.HasMember(propertyName) ||
        !propertiesInstance[propertyName].IsString())
        return false;

    result = propertiesInstance[propertyName].GetString();
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas              10/2019
//---------------------------------------------------------------------------------------
bool TryParseGuidProperty(BeSQLite::BeGuid& result, RapidJsonValueCR propertiesInstance, Utf8CP propertyName)
    {
    Utf8String resultString;
    if (!TryParseStringProperty(resultString, propertiesInstance, propertyName))
        return false;

    return result.FromString(resultString.c_str()) == BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas              10/2019
//---------------------------------------------------------------------------------------
bool TryParseStringArrayProperty(bvector<Utf8String>& result, RapidJsonValueCR propertiesInstance, Utf8CP propertyName)
    {
    if (!propertiesInstance.HasMember(propertyName) || !propertiesInstance[propertyName].IsArray())
        return false;

    auto valuesArray = propertiesInstance[propertyName].GetArray();
    for (auto const& value : valuesArray)
        {
        if (value.IsString())
            result.push_back(value.GetString());
        }

    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas              10/2019
//---------------------------------------------------------------------------------------
bool TryGetRelatedInstance(WSObjectsReader::Instance& result, WSObjectsReader::Instance mainInstance, Utf8CP className)
    {
    auto relationshipInstances = mainInstance.GetRelationshipInstances();
    if (0 == relationshipInstances.Size())
        return false;

    for (auto relationshipInstance : relationshipInstances)
        {
        auto relatedInstance = relationshipInstance.GetRelatedInstance();
        if (!relatedInstance.IsValid())
            continue;

        if (relatedInstance.GetObjectId().className == className)
            {
            result = relatedInstance;
            return true;
            }
        }

    return false;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            10/2019
//---------------------------------------------------------------------------------------
void FillArrayPropertyJson
(
JsonValueR            propertiesJson,
Utf8CP                propertyName,
bvector<Utf8String>   arrayValues
)
    {
    if (arrayValues.empty())
        return;

    propertiesJson[propertyName] = Json::arrayValue;
    int i = 0;
    for (auto const& arrayValue : arrayValues)
        propertiesJson[propertyName][i++] = arrayValue.c_str();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              julius.cepukenas   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool LockLevelFromJson(LockLevel& level, RapidJsonValueCR value)
    {
    if (value.IsUint())
        {
        if (RepositoryJson::LockLevelFromUInt(level, value.GetUint()))
            return true;
        }

    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             julius.cepukenas   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool LockableTypeFromJson(LockableType& type, RapidJsonValueCR value)
    {
    if (value.IsUint())
        {
        if (RepositoryJson::LockableTypeFromUInt(type, value.GetUint()))
            return true;
        }

    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool BriefcaseIdFromJson(BeBriefcaseId& bcId, RapidJsonValueCR value)
    {
    if (!value.IsUint())
        return false;

    bcId = BeBriefcaseId(value.GetUint());
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                12/2016
//---------------------------------------------------------------------------------------
bool GetMultiLockFromServerJson(RapidJsonValueCR serverJson, DgnLockSet& lockSet, BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringR changeSetId)
    {
    LockLevel               level;
    LockableType            type;

    if (!serverJson.HasMember(ServerSchema::Property::ObjectIds) || !serverJson.HasMember(ServerSchema::Property::LockLevel) ||
        !serverJson.HasMember(ServerSchema::Property::LockType) || !serverJson.HasMember(ServerSchema::Property::BriefcaseId))
        return false;

    if (!LockLevelFromJson(level, serverJson[ServerSchema::Property::LockLevel]) ||
        !LockableTypeFromJson(type, serverJson[ServerSchema::Property::LockType]) ||
        !BriefcaseIdFromJson(briefcaseId, serverJson[ServerSchema::Property::BriefcaseId]))
        return false;

    auto values = serverJson[ServerSchema::Property::ObjectIds].GetArray();
    if (0 == values.Size())
        return false;

    for (auto it = values.begin(); it != values.end(); ++it)
        {
        BeInt64Id id;
        BeInt64IdFromJson(id, *it);
        lockSet.insert(DgnLock(LockableId(type, id), level));
        }

    if (!serverJson.HasMember(ServerSchema::Property::ReleasedWithChangeSet) || 
        !StringFromJson(changeSetId, serverJson[ServerSchema::Property::ReleasedWithChangeSet]))
        changeSetId = nullptr;

    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
bool GetLockFromServerJson(RapidJsonValueCR serverJson, DgnLockR lock, BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringR changeSetId)
    {
    BeInt64Id           id;
    LockLevel           level;
    LockableType        type;

    if (!serverJson.HasMember(ServerSchema::Property::ObjectId) || !serverJson.HasMember(ServerSchema::Property::LockLevel) ||
        !serverJson.HasMember(ServerSchema::Property::LockType) || !serverJson.HasMember(ServerSchema::Property::BriefcaseId))
        return false;

    if (!BeInt64IdFromJson(id, serverJson[ServerSchema::Property::ObjectId]) ||
        !LockLevelFromJson(level, serverJson[ServerSchema::Property::LockLevel]) ||
        !LockableTypeFromJson(type, serverJson[ServerSchema::Property::LockType]) ||
        !BriefcaseIdFromJson(briefcaseId, serverJson[ServerSchema::Property::BriefcaseId]))
        return false;

    if (!serverJson.HasMember(ServerSchema::Property::ReleasedWithChangeSet) || !StringFromJson(changeSetId, serverJson[ServerSchema::Property::ReleasedWithChangeSet]))
        changeSetId = nullptr;

    lock = DgnLock(LockableId(type, id), level);

    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite         03/2017
//---------------------------------------------------------------------------------------
bool AddLockInfoToListFromErrorJson(DgnLockInfoSet& lockInfos, RapidJsonValueCR serverJson)
    {
    if (!serverJson.HasMember(ServerSchema::Property::LockLevel) || !serverJson.HasMember(ServerSchema::Property::LockType) || !serverJson.HasMember(ServerSchema::Property::LockOwners))
        return false;

    LockLevel    level;
    LockableType type;
    if (!LockLevelFromJson(level, serverJson[ServerSchema::Property::LockLevel]) ||
        !LockableTypeFromJson(type, serverJson[ServerSchema::Property::LockType]))
        return false;

    for (auto& lockInfo : serverJson[ServerSchema::Property::LockOwners].GetArray())
        {

        if (!lockInfo.HasMember(ServerSchema::Property::ObjectId) || !lockInfo.HasMember(ServerSchema::Property::BriefcaseIds))
            return false;

        BeInt64Id id;
        if (!BeInt64IdFromJson(id, lockInfo[ServerSchema::Property::ObjectId]))
            return false;

        for (auto& briefcaseInfo : lockInfo[ServerSchema::Property::BriefcaseIds].GetArray())
            {
            BeSQLite::BeBriefcaseId briefcaseId;
            if (BriefcaseIdFromJson(briefcaseId, briefcaseInfo))
                AddLockInfoToList(lockInfos, DgnLock(LockableId(type, id), level), briefcaseId, nullptr);
            }
        }

    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas              08/2016
//---------------------------------------------------------------------------------------
bool GetCodeSequenceFromServerJson(RapidJsonValueCR serverJson, CodeSequence& codeSequence)
    {
    BeInt64Id      codeSpecId;
    Utf8String     scope = "";
    Utf8String     value = "";
    Utf8String     valuePattern = "";

    if (!BeInt64IdFromJson(codeSpecId, serverJson[ServerSchema::Property::CodeSpecId]) ||
        !StringFromJson(scope, serverJson[ServerSchema::Property::CodeScope]))
        return false;

    if (!StringFromJson(value, serverJson[ServerSchema::Property::Value]) &&
        !StringFromJson(valuePattern, serverJson[ServerSchema::Property::ValuePattern]))
        return false;

    codeSequence = CodeSequence(CodeSpecId(codeSpecId.GetValue()), scope, value, valuePattern);

    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
void AddLockInfoToList(DgnLockInfoSet& lockInfos, const DgnLock& dgnLock, const BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR changeSetId)
    {
    DgnLockInfo&      info = *lockInfos.insert(DgnLockInfo(dgnLock.GetLockableId())).first;
    DgnLockOwnershipR ownership = info.GetOwnership();

    if (LockLevel::Exclusive == dgnLock.GetLevel())
        ownership.SetExclusiveOwner(briefcaseId);
    else if (LockLevel::Shared == dgnLock.GetLevel())
        ownership.AddSharedOwner(briefcaseId);

    if (!changeSetId.empty())
        info.SetRevisionId(changeSetId);

    if (info.IsOwned() || !changeSetId.empty())
        info.SetTracked();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
bool CodeStateFromJson(DgnCodeStateR codeState, RapidJsonValueCR stateValue, BeSQLite::BeBriefcaseId& briefcaseId)
    {
    if (stateValue.IsNull())
        return false;

    int parsedState;
    parsedState = stateValue.GetInt();

    if (1 == parsedState)
        {
        codeState.SetReserved(briefcaseId);
        return true;
        }
    if (2 == parsedState)
        {
        // TODO what state changeSet???
        codeState.SetUsed("");
        return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                12/2016
//---------------------------------------------------------------------------------------
bool GetMultiCodeFromServerJson(RapidJsonValueCR serverJson, DgnCodeSet& codeSet, DgnCodeStateR codeState, BeSQLite::BeBriefcaseId& briefcaseId)
    {
    CodeSpecId codeSpecId;
    Utf8String scopeString;

    if (!serverJson.HasMember(ServerSchema::Property::CodeSpecId) || !serverJson.HasMember(ServerSchema::Property::CodeScope) ||
        !serverJson.HasMember(ServerSchema::Property::Values) || !serverJson.HasMember(ServerSchema::Property::BriefcaseId) ||
        !serverJson.HasMember(ServerSchema::Property::State))
        return false;

    if (!BeInt64IdFromJson(codeSpecId, serverJson[ServerSchema::Property::CodeSpecId]) ||
        !StringFromJson(scopeString, serverJson[ServerSchema::Property::CodeScope]) ||
        !BriefcaseIdFromJson(briefcaseId, serverJson[ServerSchema::Property::BriefcaseId]) ||
        !CodeStateFromJson(codeState, serverJson[ServerSchema::Property::State], briefcaseId))
        return false;

    auto values = serverJson[ServerSchema::Property::Values].GetArray();
    if (0 == values.Size())
        return false;

    for (auto it = values.begin(); it != values.end(); ++it)
        {
        Utf8String value = "";
        StringFromJson(value, *it);
        DgnCode code = DgnCode::From(codeSpecId, scopeString, value);
        if (code.IsValid())
            codeSet.insert(code);
        }

    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
bool GetCodeFromServerJson(RapidJsonValueCR serverJson, DgnCodeR code, DgnCodeStateR codeState, BeSQLite::BeBriefcaseId& briefcaseId)
    {
    CodeSpecId codeSpecId;
    Utf8String scopeString;
    Utf8String value;

    if (!serverJson.HasMember(ServerSchema::Property::CodeSpecId) || !serverJson.HasMember(ServerSchema::Property::CodeScope) ||
        !serverJson.HasMember(ServerSchema::Property::Value) || !serverJson.HasMember(ServerSchema::Property::BriefcaseId) ||
        !serverJson.HasMember(ServerSchema::Property::State))
        return false;

    if (!BeInt64IdFromJson(codeSpecId, serverJson[ServerSchema::Property::CodeSpecId]) ||
        !StringFromJson(scopeString, serverJson[ServerSchema::Property::CodeScope]) ||
        !StringFromJson(value, serverJson[ServerSchema::Property::Value]) ||
        !BriefcaseIdFromJson(briefcaseId, serverJson[ServerSchema::Property::BriefcaseId]) ||
        !CodeStateFromJson(codeState, serverJson[ServerSchema::Property::State], briefcaseId))
        return false;

    code = DgnCode::From(codeSpecId, scopeString, value);
    return code.IsValid();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas              10/2016
//---------------------------------------------------------------------------------------
rapidjson::Document ToRapidJson(JsonValueCR source)
    {
    rapidjson::Document target;
    auto sourceStr = Json::FastWriter::ToString(source);
    target.Parse<0>(sourceStr.c_str());
    BeAssert(!target.HasParseError());
    return target;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             07/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus GetErrorResponseStatus(Result<void> result)
    {
    static bmap<Error::Id, RepositoryStatus> map;
    if (map.empty())
        {
        map[Error::Id::LockOwnedByAnotherBriefcase]    = RepositoryStatus::LockAlreadyHeld;
        map[Error::Id::PullIsRequired]                 = RepositoryStatus::RevisionRequired;
        map[Error::Id::ChangeSetDoesNotExist]          = RepositoryStatus::InvalidRequest;
        map[Error::Id::CodeStateInvalid]               = RepositoryStatus::InvalidRequest;
        map[Error::Id::CodeReservedByAnotherBriefcase] = RepositoryStatus::CodeUnavailable;
        map[Error::Id::CodeDoesNotExist]               = RepositoryStatus::CodeNotReserved;
        map[Error::Id::InvalidPropertiesValues]        = RepositoryStatus::InvalidRequest;
        map[Error::Id::iModelIsLocked]                 = RepositoryStatus::RepositoryIsLocked;
        }

    auto it = map.find(result.GetError().GetId());
    if (it != map.end())
        {
        return it->second;
        }

    return RepositoryStatus::ServerUnavailable;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SetCodesLocksStates(IBriefcaseManager::Response& response, IBriefcaseManager::ResponseOptions options, JsonValueCR deniedLocks, 
                         JsonValueCR deniedCodes)
    {
    if (IBriefcaseManager::ResponseOptions::None != (IBriefcaseManager::ResponseOptions::LockState & options))
        {
        for (auto const& lockJson : deniedLocks)
            {
            auto rapidJson = ToRapidJson(lockJson);
            if (!AddLockInfoToListFromErrorJson(response.LockStates(), rapidJson))
                continue;//NEEDSWORK: log an error
            }
        }
    if (IBriefcaseManager::ResponseOptions::None != (IBriefcaseManager::ResponseOptions::CodeState & options))
        {
        for (auto const& codeJson : deniedCodes)
            {
            DgnCode                  code;
            DgnCodeState             codeState;
            BeSQLite::BeBriefcaseId  briefcaseId;
            auto rapidJson = ToRapidJson(codeJson);
            if (!GetCodeFromServerJson(rapidJson, code, codeState, briefcaseId))
                continue;//NEEDSWORK: log an error

            AddCodeInfoToList(response.CodeStates(), code, codeState, briefcaseId);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             07/16
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManager::Response ConvertErrorResponse(IBriefcaseManager::Request const& request, Result<void> result, IBriefcaseManager::RequestPurpose purpose)
    {
    IBriefcaseManager::Response response(purpose, request.Options(), RepositoryStatus::ServerUnavailable);
    Error&  error = result.GetError();

    RepositoryStatus responseStatus = GetErrorResponseStatus(result);
    if (RepositoryStatus::LockAlreadyHeld == responseStatus)
        {
        response.SetResult(responseStatus);
        }
    else if (RepositoryStatus::RevisionRequired == responseStatus)
        {
        response.SetResult(responseStatus);
        }
    else if (RepositoryStatus::InvalidRequest == responseStatus || RepositoryStatus::RepositoryIsLocked == responseStatus)
        {
        response.SetResult(responseStatus);
        }
    else if (RepositoryStatus::CodeUnavailable == responseStatus)
        {
        response.SetResult(responseStatus);
        }
    else if (RepositoryStatus::CodeNotReserved == responseStatus)
        {
        response.SetResult(responseStatus);
        }

    JsonValueCR errorData = error.GetExtendedData();
    if (errorData.isMember(ServerSchema::Property::ConflictingLocks))
        SetCodesLocksStates(response, request.Options(), errorData[ServerSchema::Property::ConflictingLocks], nullptr);
    if (errorData.isMember(ServerSchema::Property::LocksRequiresPull) || errorData.isMember(ServerSchema::Property::CodesRequiresPull))
        SetCodesLocksStates(response, request.Options(), errorData[ServerSchema::Property::LocksRequiresPull], errorData[ServerSchema::Property::CodesRequiresPull]);
    if (errorData.isMember(ServerSchema::Property::CodeStateInvalid))
        SetCodesLocksStates(response, request.Options(), nullptr, errorData[ServerSchema::Property::CodeStateInvalid]);
    if (errorData.isMember(ServerSchema::Property::ConflictingCodes))
        SetCodesLocksStates(response, request.Options(), nullptr, errorData[ServerSchema::Property::ConflictingCodes]);
    if (errorData.isMember(ServerSchema::Property::CodesNotFound))
        SetCodesLocksStates(response, request.Options(), nullptr, errorData[ServerSchema::Property::CodesNotFound]);

    return response;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
void AddCodeInfoToList(DgnCodeInfoSet& codeInfos, DgnCodeCR dgnCode, DgnCodeState codeState, const BeSQLite::BeBriefcaseId briefcaseId)
    {
    DgnCodeInfo&      info = *codeInfos.insert(DgnCodeInfo(dgnCode)).first;

    if (codeState.IsAvailable())
        {
        info.SetAvailable();
        }
    else if (codeState.IsReserved())
        {
        info.SetReserved(briefcaseId);
        }
    else if (codeState.IsUsed())
        {
        //TODO changeSet
        info.SetUsed("");
        }
    else if (codeState.IsDiscarded())
        {
        //TODO changeSet
        info.SetDiscarded("");
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas           06/2017
//---------------------------------------------------------------------------------------
bool ContainsSchemaChanges(ChangeSets const& changeSets, DgnDbR db)
    {
    if (!changeSets.empty())
        {
        for (auto changeSet : changeSets)
            {
            if (changeSet->ContainsSchemaChanges(db))
                return true;
            }
        }
    return false;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2017
//---------------------------------------------------------------------------------------
void ConvertToChangeSetPointersVector(ChangeSets changeSets, bvector<DgnRevisionCP>& pointersVector)
    {
    pointersVector.clear();
    for (auto changeSetPtr : changeSets)
        {
        pointersVector.push_back(changeSetPtr.get());
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
bool IsErrorForRetry(Error::Id errorId)
    {
    static bset<Error::Id> set;
    if (set.empty())
        {
        set.insert(Error::Id::InternalServerError);
        set.insert(Error::Id::WebServicesError);
        set.insert(Error::Id::ConnectionError);
        set.insert(Error::Id::AzureError);
        }

    return set.find(errorId) != set.end();
    }

END_BENTLEY_IMODELHUB_NAMESPACE
