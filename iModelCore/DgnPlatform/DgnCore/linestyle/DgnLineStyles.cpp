/*----------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/DgnLineStyles.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

using namespace std;

//=======================================================================================
// Used in persistence; do not change values.
// These are string defines so that they can be easily concatenated into queries.
// @bsiclass
//=======================================================================================
#define DGN_STYLE_TYPE_Line "1"

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus DgnLineStyles::Insert (DgnStyleId& newStyleId, Utf8CP name, LsComponentId componentId, LsComponentType componentType, uint32_t flags, double unitDefinition)
    {
    // Don't assert to ensure an invalid ID.
    // Consider the case of cloning a style object, modifying, and then inserting it as a new style. The Clone keeps the ID, and I don't think it's worth having an overload of Clone to expose this detail.

    Json::Value jsonObj(Json::objectValue);
    LsDefinition::InitializeJsonObject(jsonObj, componentId, componentType, flags, unitDefinition);
    Utf8String data = Json::FastWriter::ToString(jsonObj);

    PRECONDITION(BE_SQLITE_OK == m_dgndb.GetServerIssuedId(newStyleId, DGN_TABLE(DGN_CLASSNAME_Style), "Id"), ERROR);

    Statement insert;
    insert.Prepare(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Style) " (Id,Type,Name,Data) VALUES (?," DGN_STYLE_TYPE_Line ",?,?)");
    insert.BindId(1, newStyleId);
    insert.BindText(2, name, Statement::MakeCopy::No);
    insert.BindBlob(3, (void const*)&data[0], (int)data.size() + 1, Statement::MakeCopy::No);

    POSTCONDITION(BE_SQLITE_DONE == insert.Step(), ERROR);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus DgnLineStyles::Update (DgnStyleId styleId, Utf8CP name, LsComponentId componentId, LsComponentType componentType, uint32_t flags, double unitDefinition)
    {
    PRECONDITION(styleId.IsValid(), ERROR);

    Json::Value jsonObj(Json::objectValue);
    LsDefinition::InitializeJsonObject(jsonObj, componentId, componentType, flags, unitDefinition);
    Utf8String data = Json::FastWriter::ToString(jsonObj);

    Statement update;
    update.Prepare(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Style) " SET Name=?,Data=? WHERE Type=" DGN_STYLE_TYPE_Line " AND Id=?");
    update.BindText(1, name, Statement::MakeCopy::No);
    update.BindBlob(2, (void const*)&data[0], (int)data.size() + 1, Statement::MakeCopy::No);
    update.BindId(3, styleId);

    POSTCONDITION(BE_SQLITE_DONE == update.Step(), ERROR);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//--------------+------------------------------------------------------------------------
LsCacheR DgnLineStyles::ReloadMap()
    {
    m_lineStyleMap = nullptr;
    return *GetLsCacheP(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsCacheP DgnLineStyles::GetLsCacheP (bool loadIfNull)
    {
    if (m_lineStyleMap.IsNull())
        m_lineStyleMap = LsCache::Create (m_dgndb);

    if (!m_lineStyleMap->IsLoaded() && loadIfNull)
        m_lineStyleMap->Load();

    return m_lineStyleMap.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2015
//---------------------------------------------------------------------------------------
void DgnLineStyles::PrepareToQueryLineStyle(BeSQLite::Statement & stmt, DgnStyleId styleId)
    {
    stmt.Prepare(m_dgndb, "SELECT * FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type = " DGN_STYLE_TYPE_Line " AND Id=?");
    stmt.BindId(1, styleId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
void DgnLineStyles::PrepareToQueryAllLineStyles(BeSQLite::Statement & stmt)
    {
    stmt.Prepare(m_dgndb, "SELECT * FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type = " DGN_STYLE_TYPE_Line );
    }
