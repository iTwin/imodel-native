/*----------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LineStyleManager.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
LsDefinitionP   LineStyleManager::ResolveLineStyle (Int32 styleNumber, DgnProjectP dgnFile)
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
WString         LineStyleManager::GetNameFromNumber (Int32 styleNumber, DgnProjectP dgnFile)
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
WString         LineStyleManager::GetStringFromNumber (Int32 styleNumber, DgnProjectP dgnFile)
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
Int32           LineStyleManager::GetNumberFromName (Utf8CP name, DgnProjectP dgnFile)
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
            Int32 specialsIndex [] = 
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
Int32           LineStyleManager::GetNumberFromName (WCharCP name, DgnProjectP dgnFile)
    {
    Utf8String utf8;
    BeStringUtilities::WCharToUtf8 (utf8, name);
    return GetNumberFromName(utf8.c_str(), dgnFile);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2012
//---------------------------------------------------------------------------------------
DbResult DgnLineStyles::InsertWithId (DgnStyleId newStyleId, Utf8CP name, UInt32 componentId, UInt16 componentType, UInt32 flags, double unitDefinition)
    {
    Json::Value jsonObj (Json::objectValue);

    LsDefinition::InitializeJsonObject (jsonObj, componentId, componentType, flags, unitDefinition);
    Utf8String  blob = Json::FastWriter::ToString (jsonObj);

    DgnStyles::Style styleRow (newStyleId, DgnStyleType::Line, name, NULL, blob.c_str(), blob.size() + 1);
    DbResult result = m_project.Styles().InsertStyleWithId (styleRow);
    //  We use this method when copying all of the line styles from a file.  If the file has 2 line styles with the
    //  same name we get BE_SQLITE_CONSTRAINT.  Assume the caller can deal with it.
    BeAssert(BE_SQLITE_DONE == result || IsConstraintDbResult(result));

    return result;
    }

//-------------------------------------------------------------------------------   --------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
DbResult DgnLineStyles::Insert (DgnStyleId& newStyleId, Utf8CP name, UInt32 componentId, UInt16 componentType, UInt32 flags, double unitDefinition)
    {
    Json::Value jsonObj (Json::objectValue);

    LsDefinition::InitializeJsonObject (jsonObj, componentId, componentType, flags, unitDefinition);
    Utf8String  blob = Json::FastWriter::ToString (jsonObj);

    DgnStyles::Style styleRow (newStyleId, DgnStyleType::Line, name, NULL, blob.c_str(), blob.size() + 1);
    DbResult result = m_project.Styles().InsertStyle (styleRow, STYLE_MaxLineCode + 1);  //  This also should protect against STYLE_ByLevel, STYLE_ByCell, and STYLE_Invalid

    BeAssert(BE_SQLITE_DONE == result);

    if (BE_SQLITE_DONE == result)
        newStyleId = styleRow.GetId();

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus DgnLineStyles::Update (DgnStyleId newStyleId, Utf8CP name, UInt32 componentId, UInt16 componentType, UInt32 flags, double unitDefinition)
    {
    Json::Value jsonObj (Json::objectValue);

    LsDefinition::InitializeJsonObject (jsonObj, componentId, componentType, flags, unitDefinition);
    Utf8String  blob = Json::FastWriter::ToString (jsonObj);

    DgnStyles::Style styleRow (newStyleId, DgnStyleType::Line, name, NULL, blob.c_str(), blob.size() + 1);
    if (BE_SQLITE_DONE != m_project.Styles().UpdateStyle (styleRow))
        {
        BeAssert (false);
        return BSIERROR;
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//---------------------------------------------------------------------------------------
DgnStyles::Iterator DgnLineStyles::MakeIterator (DgnStyleSort sortOrder) const
    {
    Utf8String queryModifierClauses;
    queryModifierClauses.Sprintf ("WHERE Type=%d", DgnStyleType::Line);

    switch (sortOrder)
        {
        case DgnStyleSort::None:       break;
        case DgnStyleSort::NameAsc:    queryModifierClauses += " ORDER BY Name ASC";   break;
        case DgnStyleSort::NameDsc:    queryModifierClauses += " ORDER BY Name DESC";   break;

        default:
            BeAssert (false);// && L"Unknown DgnStyleSort");
            break;
        }

    DgnStyles::Iterator it (m_project);
    it.Params().SetWhere(queryModifierClauses.c_str());
    return it;
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
        LsDgnProjectMapPtr lsmap = LsDgnProjectMap::Create (m_project);
        m_lineStyleMap = lsmap.get();
        m_lineStyleMap->AddRef();
        }

    if (!m_lineStyleMap->IsLoaded() && loadIfNull)
        m_lineStyleMap->Load();

    return m_lineStyleMap;
    }

