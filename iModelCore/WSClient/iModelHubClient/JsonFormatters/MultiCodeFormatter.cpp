/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "MultiCodeFormatter.h"
#include "../Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
void EncodeIdString
(
Utf8StringR value
)
    {
    if (value.empty())
        return;

    Utf8String reservedChar("-");
    Utf8String replacement;
    replacement.Sprintf("_%2X_", (int)reservedChar[0]);

    value.ReplaceAll(reservedChar.c_str(), replacement.c_str());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
Utf8String UriEncode(Utf8String input)
    {
    Utf8String result = BeStringUtilities::UriEncode(input.c_str());

    Utf8String charsNotEncode = ",!'()";
    for (char character : charsNotEncode)
        {
        Utf8String charString(&character, 1);
        result.ReplaceAll(BeStringUtilities::UriEncode(charString.c_str()).c_str(), charString.c_str());
        }

    result.ReplaceAll("~", "~7E");
    result.ReplaceAll("*", "~2A");
    result.ReplaceAll("%", "~");

    return result;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
Utf8String FormatCodeId
(
BeInt64Id                        codeSpecId,
Utf8StringCR                     scope,
Utf8StringCR                     value
)
    {
    Utf8String idString;

    Utf8String encodedScope(scope.c_str());
    EncodeIdString(encodedScope);
    encodedScope = UriEncode(encodedScope);

    Utf8String encodedValue(value.c_str());
    EncodeIdString(encodedValue);
    encodedValue = UriEncode(encodedValue);

    idString.Sprintf("%s-%s-%s", FormatBeInt64Id(codeSpecId).c_str(), encodedScope.c_str(), encodedValue.c_str());

    return idString;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
Utf8String FormatCodeId
(
BeInt64Id                        codeSpecId,
Utf8StringCR                     scope,
Utf8StringCR                     value,
BeBriefcaseId                    briefcaseId
)
    {
    Utf8String idString;
    idString.Sprintf("%s-%d", FormatCodeId(codeSpecId, scope, value).c_str(), briefcaseId.GetValue());

    return idString;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
ObjectId MultiCodeFormatter::GetCodeId
(
DgnCodeCR code,
const BeBriefcaseId briefcaseId
)
    {
    Utf8String idString;
    if (briefcaseId.IsValid())
        idString.Sprintf("%s", FormatCodeId(code.GetCodeSpecId(), code.GetScopeString(), code.GetValueUtf8(), briefcaseId).c_str());
    else
        idString.Sprintf("%s", FormatCodeId(code.GetCodeSpecId(), code.GetScopeString(), code.GetValueUtf8()).c_str());

    return ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::Code, idString);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
uint8_t DgnCodeStateToInt(DgnCodeStateCR state)
    {
    //NEEDSWORK: Make DgnCodeState::Type public
    if (state.IsReserved())
        return 1;
    if (state.IsUsed())
        return 2;

    return 0;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
Json::Value CreateCodeInstanceJson
(
bvector<DgnCode> const&      codes,
DgnCodeStateCR               state,
BeBriefcaseId                briefcaseId,
bool                         queryOnly
)
    {
    Json::Value properties;
    DgnCode const* firstCode = codes.begin();

    properties[ServerSchema::Property::CodeSpecId]   = FormatBeInt64Id(firstCode->GetCodeSpecId());
    properties[ServerSchema::Property::CodeScope]    = firstCode->GetScopeString();
    properties[ServerSchema::Property::BriefcaseId]  = briefcaseId.GetValue();
    properties[ServerSchema::Property::State]        = DgnCodeStateToInt(state);
    properties[ServerSchema::Property::QueryOnly]    = queryOnly;

    properties[ServerSchema::Property::Values] = Json::arrayValue;
    int i = 0;
    for (auto const& code : codes)
        {
        properties[ServerSchema::Property::Values][i++] = code.GetValueUtf8();
        }

    return properties;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
void AddCodeToInstance
(
WSChangeset&                     changeset,
WSChangeset::ChangeState const&  changeState,
bvector<DgnCode> const&          codes,
DgnCodeStateCR                   state,
BeBriefcaseId                    briefcaseId,
bool                             queryOnly
)
    {
    if (codes.empty())
        return;

    ObjectId codeObject(ServerSchema::Schema::iModel, ServerSchema::Class::MultiCode, "MultiCode");
    JsonValueCR codeJson = CreateCodeInstanceJson(codes, state, briefcaseId, queryOnly);
    changeset.AddInstance(codeObject, changeState, std::make_shared<Json::Value>(codeJson));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             03/2018
//---------------------------------------------------------------------------------------
void FlushCodesToChangeset
(
bmap<Utf8String, bvector<DgnCode>>& groupedCodes,
DgnCodeState                        state,
BeBriefcaseId                       briefcaseId,
std::shared_ptr<WSChangeset>        changeset,
const WSChangeset::ChangeState&     changeState,
bool                                queryOnly = false
)
    {
    for (auto& group : groupedCodes)
        {
        AddCodeToInstance(*changeset, changeState, group.second, state, briefcaseId, queryOnly);
        }

    groupedCodes.clear();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
void GroupCode
(
bmap<Utf8String, bvector<DgnCode>>* groupedCodes,
DgnCode searchCode
)
    {
    Utf8String searchKey = FormatCodeId(searchCode.GetCodeSpecId(), searchCode.GetScopeString(), "");
    auto it = groupedCodes->find(searchKey);
    if (it == groupedCodes->end())
        {
        bvector<DgnCode> codes;
        codes.push_back(searchCode);
        groupedCodes->insert({searchKey, codes});
        return;
        }
    it->second.push_back(searchCode);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             03/2018
//---------------------------------------------------------------------------------------
void MultiCodeFormatter::SetToChunkedChangeset
    (
    const DgnCodeSet                codes,
    DgnCodeState                    state,
    BeBriefcaseId                   briefcaseId,
    ChunkedWSChangeset&             chunkedChangeset,
    const WSChangeset::ChangeState& changeState,
    bool                            queryOnly
    )
    {
    bmap<Utf8String, bvector<DgnCode>> groupedCodes;
    for (auto& code : codes)
        {
        if (!chunkedChangeset.AddInstance())
            {
            FlushCodesToChangeset(groupedCodes, state, briefcaseId, chunkedChangeset.GetCurrentChangeset(), changeState, queryOnly);
            chunkedChangeset.StartNewChangeset();
            }

        GroupCode(&groupedCodes, code);
        }

    FlushCodesToChangeset(groupedCodes, state, briefcaseId, chunkedChangeset.GetCurrentChangeset(), changeState, queryOnly);
    }
