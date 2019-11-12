/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PendingChangeSetStorage.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

#define CHANGESET_ID_PROPERTY_NAME "ChangeSetId"
#define RELINSQUISH_PROPERTY_NAME "Relinquish"
#define PENDING_CHANGESET_PROPERTY_NAME "PendingChangeSets"

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas            03/2018
//---------------------------------------------------------------------------------------
void GetExistingValuesList(Dgn::DgnDbR db, bvector<Utf8String>& items)
    {
    Utf8String savedValue;
    if (BeSQLite::DbResult::BE_SQLITE_ROW != db.QueryBriefcaseLocalValue(savedValue, PENDING_CHANGESET_PROPERTY_NAME) || Utf8String::IsNullOrEmpty(savedValue.c_str()))
        return;

    Json::Value parsedValue;
    Json::Reader::Parse(savedValue, parsedValue);

    for (Json::ArrayIndex i = 0; i < parsedValue.size(); i++)
        items.push_back(parsedValue[i].asString());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas            03/2018
//---------------------------------------------------------------------------------------
void SaveValuesToBriefcase(Dgn::DgnDbR db, bvector<Utf8String> items)
    {
    Json::Value jsonValues = Json::arrayValue;
    int i = 0;
    for (auto const& item : items)
        jsonValues[i++] = item;

    db.SaveBriefcaseLocalValue(PENDING_CHANGESET_PROPERTY_NAME, jsonValues.ToString());
    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas            03/2018
//---------------------------------------------------------------------------------------
void PendingChangeSetStorage::AddItem(Dgn::DgnDbR db, Utf8String item)
    {
    bvector<Utf8String> items;
    GetExistingValuesList(db, items);

    bool found = false;
    for (auto it = items.begin(); it != items.end(); it++)
        if (item == *it)
            {
            found = true;
            break;
            }

    if (!found)
        items.push_back(item);
        
    SaveValuesToBriefcase(db, items);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas            03/2018
//---------------------------------------------------------------------------------------
void PendingChangeSetStorage::RemoveItem(Dgn::DgnDbR db, Utf8String item)
    {
    bvector<Utf8String> items;
    GetExistingValuesList(db, items);

    for (auto it = items.begin(); it != items.end(); it++)
        if (item == *it)
            {
            items.erase(it);
            break;
            }

    SaveValuesToBriefcase(db, items);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas            03/2018
//---------------------------------------------------------------------------------------
void PendingChangeSetStorage::GetItems(Dgn::DgnDbR db, bvector<Utf8String>& items)
    {
    GetExistingValuesList(db, items);
    }
