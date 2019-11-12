/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/GlobalRequestOptions.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            07/2018
//---------------------------------------------------------------------------------------
void GlobalRequestOptions::InsertRequestOptions(std::shared_ptr<WebServices::WSChangeset> changeset) const
    {
    if (nullptr == m_requestOptionsPtr)
        return;

    for (auto iterator = m_requestOptionsPtr->begin(); iterator != m_requestOptionsPtr->end(); ++iterator)
        {
        changeset->GetRequestOptions().SetCustomOption(iterator->first, iterator->second);
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            07/2018
//---------------------------------------------------------------------------------------
void GlobalRequestOptions::InsertRequestOptions(JsonValueR jsonValue) const
    {
    if (nullptr == m_requestOptionsPtr)
        return;

    if (Json::nullValue == jsonValue["requestOptions"].type())
        jsonValue["requestOptions"] = Json::objectValue;

    if (Json::nullValue == jsonValue["requestOptions"]["CustomOptions"].type())
        jsonValue["requestOptions"]["CustomOptions"] = Json::objectValue;

    for (auto iterator = m_requestOptionsPtr->begin(); iterator != m_requestOptionsPtr->end(); ++iterator)
        {
        jsonValue["requestOptions"]["CustomOptions"][iterator->first] = iterator->second;
        }
    }

END_BENTLEY_IMODELHUB_NAMESPACE