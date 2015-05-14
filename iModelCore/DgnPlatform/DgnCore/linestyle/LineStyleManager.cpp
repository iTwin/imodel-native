/*----------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LineStyleManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

using namespace std;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleManager::InitializeSystemMap ()
    {
    LsSystemMap*   systemMap = LsSystemMap::GetSystemMapP (false);   //  initialize structures
    if (systemMap->IsLoaded ())
        return;
        
    // Preset this; if not, it will recurse looking for names.
    systemMap->TreeLoaded();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleManager::TerminateSystemMap ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleManager::Initialize ()
    {
    InitializeSystemMap ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleManager::Terminate ()
    {
    LineStyleCacheManager::CacheFree ();
    LineStyleCacheManager::FreeDgnFileMaps ();
    GetManager().TerminateSystemMap ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LineStyleManager::Reinitialize ()
    {
    LineStyleManager::GetManager().Terminate ();
    LineStyleManager::GetManager().Initialize ();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleManagerR LineStyleManager::GetManager() {return T_HOST.GetLineStyleManager();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinitionP   LineStyleManager::ResolveLineStyle (int32_t styleNumber, DgnDbP dgnFile)
    {
    if (NULL == dgnFile)
        return NULL;

    LsDgnProjectMapP map = dgnFile->Styles().LineStyles().GetMapP();
    if (NULL == map)
        return NULL;
        
    return map->Find (styleNumber);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinitionP   LineStyleManager::FindSystemLineStyle (Utf8CP lineStyleName)
    {
    LsSystemMapP map = &LsSystemMap::GetSystemMapR ();
    
    return map->Find (lineStyleName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
WString         LineStyleManager::GetNameFromNumber (int32_t styleNumber, DgnDbP dgnFile)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP_LINESTYLES
    if (styleNumber < 0L)
        {
        LsIdNodeP idRec = NULL;
        LsDgnFileMapCP map = GetLineStyleMapCP (dgnFile);

        if (NULL == map)
            return WString (L"");
            
        idRec = map->FindId (styleNumber);
        if (NULL != idRec)
            return WString (idRec->GetName ());

        return WString (L"");
        }

    //  styleNumber > 0 means that the number-to-style map is in
    //  the resoruce file. This is rarely used.
    LsSystemMap* info = LsSystemMap::GetSystemMapP (true);
    BeAssert(info);
    if (NULL == info)
        return WString (L"");

    LsIdNodeP idRec = NULL;

    idRec = info->FindId (styleNumber);
    if (NULL != idRec)
        return WString (idRec->GetName ());

    LsDefinitionP   def = ResolveLineStyle (styleNumber, dgnFile);
    if (NULL == def)
        return WString(L"");

    return def->_GetName ();
#else
        return WString (L"");
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
WString         LineStyleManager::GetStringFromNumber (int32_t styleNumber, DgnDbP dgnFile)
    {
    if (IS_LINECODE (styleNumber))
        {
        WChar    buffer [100];
        BeStringUtilities::Snwprintf (buffer, _countof(buffer), L"%d", styleNumber);
        return WString (buffer);
        }
        
    if (STYLE_ByLevel == styleNumber)
        return WString (L"STYLE_ByLevel");

    if (STYLE_ByCell == styleNumber)
        return WString (L"STYLE_ByCell");

    if (STYLE_Invalid == styleNumber)
        return WString (L"STYLE_Invalid");

    return GetNameFromNumber (styleNumber, dgnFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t           LineStyleManager::GetNumberFromName (Utf8CP name, DgnDbP dgnFile)
    {
    Utf8CP specials [] = 
        {
        "STYLE_ByLevel",
        "STYLE_ByCell",
        "STYLE_Invalid",
        "0",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        NULL
        };

    for (Utf8CP *curr = specials; NULL != *curr; curr++)
        {
        if (0 == strcmp (*curr, name))
            {
            int32_t specialsIndex [] = 
                {
                STYLE_ByLevel,
                STYLE_ByCell,
                STYLE_Invalid,
                0,
                1,
                2,
                3,
                4,
                5,
                6,
                7
                };

            size_t  index = curr - specials;
            return specialsIndex [index];
            }
        }

    LsDgnProjectMapP map = LsMap::GetMapPtr (*dgnFile);
    if (NULL == map)
        return INVALID_STYLE;

    LsIdNodeP   node = map->SearchIdsForName (name);
    if (NULL == node)
        return INVALID_STYLE;

    return node->GetKey ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t           LineStyleManager::GetNumberFromName (WCharCP name, DgnDbP dgnFile)
    {
    Utf8String utf8;
    BeStringUtilities::WCharToUtf8 (utf8, name);
    return GetNumberFromName(utf8.c_str(), dgnFile);
    }

//=======================================================================================
// Used in persistence; do not change values.
// These are string defines so that they can be easily concatenated into queries.
// @bsiclass
//=======================================================================================
#define DGN_STYLE_TYPE_Line "1"

//-------------------------------------------------------------------------------   --------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus DgnLineStyles::Insert (DgnStyleId& newStyleId, Utf8CP name, uint32_t componentId, uint16_t componentType, uint32_t flags, double unitDefinition)
    {
    // Don't assert to ensure an invalid ID.
    // Consider the case of cloning a style object, modifying, and then inserting it as a new style. The Clone keeps the ID, and I don't think it's worth having an overload of Clone to expose this detail.

    Json::Value jsonObj(Json::objectValue);
    LsDefinition::InitializeJsonObject(jsonObj, componentId, componentType, flags, unitDefinition);
    Utf8String data = Json::FastWriter::ToString(jsonObj);

    PRECONDITION(BE_SQLITE_OK == m_dgndb.GetNextRepositoryBasedId(newStyleId, DGN_TABLE(DGN_CLASSNAME_Style), "Id"), ERROR);

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
BentleyStatus DgnLineStyles::Update (DgnStyleId styleId, Utf8CP name, uint32_t componentId, uint16_t componentType, uint32_t flags, double unitDefinition)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnLineStyles::~DgnLineStyles()
    {
    RELEASE_AND_CLEAR (m_lineStyleMap);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//--------------+------------------------------------------------------------------------
LsDgnProjectMapR DgnLineStyles::ReloadMap()
    {
    RELEASE_AND_CLEAR (m_lineStyleMap);
    return *GetMapP(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsDgnProjectMapP DgnLineStyles::GetMapP (bool loadIfNull)
    {
    if (NULL == m_lineStyleMap)
        {
        LsDgnProjectMapPtr lsmap = LsDgnProjectMap::Create (m_dgndb);
        m_lineStyleMap = lsmap.get();
        m_lineStyleMap->AddRef();
        }

    if (!m_lineStyleMap->IsLoaded() && loadIfNull)
        m_lineStyleMap->Load();

    return m_lineStyleMap;
    }
