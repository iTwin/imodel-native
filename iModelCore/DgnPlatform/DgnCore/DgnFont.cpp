/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnFont.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnFontData.h>
#include <DgnPlatform/DgnRscFontStructures.h>   // For HMASK_RSC_HOLE


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::FontAdmin::~FontAdmin()
    {
    DELETE_AND_CLEAR(m_dbFonts);
    DELETE_AND_CLEAR(m_lastResortFontDb);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BeFileName DgnPlatformLib::Host::FontAdmin::_GetLastResortFontDbPath()
    {
    BeFileName path = DgnPlatformLib::GetHost().GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    path.AppendToPath(L"lastresortfonts");

    return path;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnPlatformLib::Host::FontAdmin::_EnsureLastResortFontDb()
    {
    if (m_isInitialized)
        return ((nullptr != m_dbFonts) ? SUCCESS : ERROR);

    m_isInitialized = true;
    
    BeFileName path = _GetLastResortFontDbPath();
    if (path.empty())
        {
        BeAssert(false);
        return ERROR;
        }
    
    unique_ptr<Db> lastResortDb(new Db());
    DbResult openResult = lastResortDb->OpenBeSQLiteDb(path, Db::OpenParams(Db::OpenMode::Readonly));
    if (BE_SQLITE_OK != openResult)
        {
        BeAssert(false);
        return ERROR;
        }

    m_lastResortFontDb = lastResortDb.release();

    m_dbFonts = new DgnFonts(*m_lastResortFontDb, "lastresortfont");

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontPtr DgnPlatformLib::Host::FontAdmin::_CreateLastResortFont(DgnFontType type)
    {
    if (SUCCESS != _EnsureLastResortFontDb())
        return nullptr;
    
    Utf8String name;
    switch (type)
        {
        case DgnFontType::TrueType: name = "LastResortTrueTypeFont"; break;
        case DgnFontType::Rsc: name = "LastResortRscFont"; break;
        case DgnFontType::Shx: name = "LastResortShxFont"; break;

        default:
            BeAssert(false);
            return nullptr;
        }
    
    DgnFontPtr foundFont = m_dbFonts->DbFontMap().QueryByTypeAndName(type, name.c_str());
    if (!foundFont.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return foundFont;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontCR DgnPlatformLib::Host::FontAdmin::_ResolveFont(DgnFontCP font)
    {
    if ((nullptr != font) && font->IsResolved())
        return *font;
    
    if (nullptr != font)
        {
        switch (font->GetType())
            {
            case DgnFontType::TrueType: return _GetLastResortTrueTypeFont();
            case DgnFontType::Rsc: return _GetLastResortRscFont();
            case DgnFontType::Shx: return _GetLastResortShxFont();
        
            default:
                BeAssert(false);
                // Fall through
            }
        }
    
    return _GetAnyLastResortFont();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2016
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::FontAdmin::Suspend()
    {
    if (!m_isInitialized)
        return;

    m_lastResortFontDb->CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2016
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::FontAdmin::Resume()
    {
    if (!m_isInitialized)
        return;

    BeFileName path = _GetLastResortFontDbPath();
    if (path.empty())
        return;
    
    DbResult openResult = m_lastResortFontDb->OpenBeSQLiteDb(path, Db::OpenParams(Db::OpenMode::Readonly));
    if (BE_SQLITE_OK != openResult)
        return;
    }

/*---------------------------------------------------------------------------------**//**
* This method exists for multi-threaded code which wants to adopt the host. Because
* The FontAdmin lazily-initializes various data members, it cannot be safely used
* on multiple threads until all of those data members are initialized.
* So code that is otherwise very confident in its thread-safety can invoke this method
* before spawning new threads in order to ensure they will not encounter race conditions
* when accessing the FontAdmin.
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::FontAdmin::EnsureInitialized()
    {
    GetLastResortTrueTypeFont();
    GetLastResortRscFont();
    GetLastResortShxFont();
    GetAnyLastResortFont();
    GetDecoratorFont();
    GetFreeTypeLibrary();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontCR DgnFontManager::GetLastResortTrueTypeFont() { return T_HOST.GetFontAdmin()._GetLastResortTrueTypeFont(); }
DgnFontCR DgnFontManager::GetLastResortRscFont() { return T_HOST.GetFontAdmin()._GetLastResortRscFont(); }
DgnFontCR DgnFontManager::GetLastResortShxFont() { return T_HOST.GetFontAdmin()._GetLastResortShxFont(); }
DgnFontCR DgnFontManager::GetAnyLastResortFont() { return T_HOST.GetFontAdmin()._GetAnyLastResortFont(); }
DgnFontCR DgnFontManager::GetDecoratorFont() { return T_HOST.GetFontAdmin()._GetDecoratorFont(); }
DgnFontCR DgnFontManager::ResolveFont(DgnFontCP font) { return T_HOST.GetFontAdmin()._ResolveFont(font); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontId DgnFonts::DbFontMapDirect::Iterator::Entry::GetId() const { Verify(); return m_sql->GetValueId<DgnFontId>(0); }
DgnFontType DgnFonts::DbFontMapDirect::Iterator::Entry::GetType() const { Verify(); return (DgnFontType)m_sql->GetValueInt(1); }
Utf8CP DgnFonts::DbFontMapDirect::Iterator::Entry::GetName() const { Verify(); return m_sql->GetValueText(2); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFonts::DbFontMapDirect::Iterator::const_iterator DgnFonts::DbFontMapDirect::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sql = MakeSqlString(SqlPrintfString("SELECT Id,Type,Name FROM %s", m_dbFonts.m_tableName.c_str()), false);
        m_db->GetCachedStatement(m_stmt, sql.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), (BE_SQLITE_ROW == m_stmt->Step()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
size_t DgnFonts::DbFontMapDirect::Iterator::QueryCount() const
    {
    Utf8String sql = MakeSqlString(SqlPrintfString("SELECT COUNT(*) FROM %s", m_dbFonts.m_tableName.c_str()), false);

    Statement query;
    query.Prepare(*m_db, sql.c_str());
    m_params.Bind(query);

    if (BE_SQLITE_ROW != query.Step())
        {
        BeAssert(false);
        return 0;
        }

    return (size_t)query.GetValueInt(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnFonts::DbFontMapDirect::CreateFontTable()
    {
    DbResult createStatus = m_dbFonts.m_db.CreateTable(m_dbFonts.m_tableName.c_str(),
        "Id INTEGER PRIMARY KEY,"
        "Type INT,"
        "Name CHAR NOT NULL COLLATE NOCASE,"
        "Metadata BLOB,"
        "CONSTRAINT names UNIQUE(Type,Name)");
    
    if (BE_SQLITE_OK != createStatus)
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontPtr DgnFonts::DbFontMapDirect::QueryById(DgnFontId id) const
    {
    PRECONDITION(id.IsValid(), nullptr);

    Statement query;
    query.Prepare(m_dbFonts.m_db, SqlPrintfString("SELECT Type,Name,Metadata FROM %s WHERE Id=?", m_dbFonts.m_tableName.c_str()));
    query.BindId(1, id);

    if (BE_SQLITE_ROW != query.Step())
        return nullptr;

    return DgnFontPersistence::Db::FromDb(m_dbFonts, id, (DgnFontType)query.GetValueInt(0), query.GetValueText(1), (Byte const*)query.GetValueBlob(2), (size_t)query.GetColumnBytes(2));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontPtr DgnFonts::DbFontMapDirect::QueryByTypeAndName(DgnFontType type, Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), nullptr);

    Statement query;
    query.Prepare(m_dbFonts.m_db, SqlPrintfString("SELECT Id,Metadata FROM %s WHERE Type=? AND Name=?", m_dbFonts.m_tableName.c_str()));
    query.BindInt(1, (int)type);
    query.BindText(2, name, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != query.Step())
        return nullptr;

    return DgnFontPersistence::Db::FromDb(m_dbFonts, query.GetValueId<DgnFontId>(0), type, name, (Byte const*)query.GetValueBlob(1), (size_t)query.GetColumnBytes(1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontId DgnFonts::DbFontMapDirect::QueryIdByTypeAndName(DgnFontType type, Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), DgnFontId());

    Statement query;
    query.Prepare(m_dbFonts.m_db, SqlPrintfString("SELECT Id FROM %s WHERE Type=? AND Name=?", m_dbFonts.m_tableName.c_str()));
    query.BindInt(1, (int)type);
    query.BindText(2, name, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != query.Step())
        return DgnFontId();

    return query.GetValueId<DgnFontId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
bool DgnFonts::DbFontMapDirect::ExistsById(DgnFontId id) const
    {
    PRECONDITION(id.IsValid(), false);

    Statement query;
    query.Prepare(m_dbFonts.m_db, SqlPrintfString("SELECT COUNT(*) FROM %s WHERE Id=?", m_dbFonts.m_tableName.c_str()));
    query.BindId(1, id);

    if (BE_SQLITE_ROW != query.Step())
        {
        BeAssert(false);
        return false;
        }

    return (query.GetValueInt(0) > 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
bool DgnFonts::DbFontMapDirect::ExistsByTypeAndName(DgnFontType type, Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), false);

    Statement query;
    query.Prepare(m_dbFonts.m_db, SqlPrintfString("SELECT COUNT(*) FROM %s WHERE Type=? AND Name=?", m_dbFonts.m_tableName.c_str()));
    query.BindInt(1, (int)type);
    query.BindText(2, name, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != query.Step())
        {
        BeAssert(false);
        return false;
        }

    return (query.GetValueInt(0) > 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnFonts::DbFontMapDirect::Insert(DgnFontCR font, DgnFontId& id)
    {
    bvector<Byte> metadata;
    if (SUCCESS != DgnFontPersistence::Db::MetadataToDb(metadata, font))
        {
        BeAssert(false);
        return ERROR;
        }
    
    id.Invalidate();
    DbResult rc= m_dbFonts.m_db.GetServerIssuedId(id, m_dbFonts.m_tableName.c_str(), "Id");

    if (BE_SQLITE_OK != rc)
        {
        BeAssert(false);
        return ERROR;
        }

    Statement insert;
    insert.Prepare(m_dbFonts.m_db, SqlPrintfString("INSERT INTO %s (Id,Type,Name,Metadata) VALUES (?,?,?,?)", m_dbFonts.m_tableName.c_str()));
    insert.BindInt(1, static_cast<int>(id.GetValue()));
    insert.BindInt(2, static_cast<int>(font.GetType()));
    insert.BindText(3, font.GetName(), Statement::MakeCopy::No);
    insert.BindBlob(4, &metadata[0], (int)metadata.size(), Statement::MakeCopy::No);

    if (BE_SQLITE_DONE != insert.Step())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnFonts::DbFontMapDirect::Update(DgnFontCR font, DgnFontId id)
    {
    PRECONDITION(id.IsValid(), ERROR);
    
    bvector<Byte> metadata;
    if (SUCCESS != DgnFontPersistence::Db::MetadataToDb(metadata, font))
        {
        BeAssert(false);
        return ERROR;
        }

    Statement update;
    update.Prepare(m_dbFonts.m_db, SqlPrintfString("UPDATE %s SET Type=?,Name=?,Metadata=? WHERE Id=?", m_dbFonts.m_tableName.c_str()));
    update.BindInt(1, static_cast<int>(font.GetType()));
    update.BindText(2, font.GetName(), Statement::MakeCopy::No);
    update.BindBlob(3, &metadata[0], (int)metadata.size(), Statement::MakeCopy::No);

    update.BindInt(5, (int)id.GetValue());

    if (BE_SQLITE_DONE != update.Step())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnFonts::DbFontMapDirect::Delete(DgnFontId id)
    {
    PRECONDITION(id.IsValid(), ERROR);

    Statement del;
    del.Prepare(m_dbFonts.m_db, SqlPrintfString("DELETE FROM %s WHERE Id=?", m_dbFonts.m_tableName.c_str()));
    del.BindId(1, id);

    if (BE_SQLITE_DONE != del.Step())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

const Utf8CP DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Regular = "regular";
const Utf8CP DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Bold = "bold";
const Utf8CP DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Italic = "italic";
const Utf8CP DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_BoldItalic = "bolditalic";

#define EMBEDDED_FACE_DATA_PROP_NS "dgn_Font"
#define EMBEDDED_FACE_DATA_PROP_NAME "EmbeddedFaceData"
static const PropertySpec EMBEDDED_FACE_DATA_PROPERTY_SPEC(EMBEDDED_FACE_DATA_PROP_NAME, EMBEDDED_FACE_DATA_PROP_NS);

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     04/2015
//=======================================================================================
struct EmbeddedFaceDataIterator : public BeSQLite::DbTableIterator
{
    EmbeddedFaceDataIterator(DbR db) : BeSQLite::DbTableIterator(db) {}

    struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
    {
    private:
        friend struct EmbeddedFaceDataIterator;
        Entry(StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid) {}

    public:
        DgnFonts::DbFaceDataDirect::DataId GetId() const { Verify(); return m_sql->GetValueInt64(0); }
        Utf8CP GetToc() const { Verify(); return m_sql->GetValueText(1); }
        size_t GetTocSize() const { Verify(); return (size_t)m_sql->GetColumnBytes(1); }
        size_t GetRawSize() const { Verify(); return (size_t)m_sql->GetValueInt(2); }
        Entry const& operator*() const { return *this; }
    };

    typedef Entry const_iterator;
    const_iterator begin() const;
    const_iterator end() const { return Entry(NULL, false); }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
EmbeddedFaceDataIterator::const_iterator EmbeddedFaceDataIterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sql = MakeSqlString("SELECT Id,StrData,RawSize FROM " BEDB_TABLE_Property " WHERE Namespace='" EMBEDDED_FACE_DATA_PROP_NS "' AND Name='" EMBEDDED_FACE_DATA_PROP_NAME "'", true);
        m_db->GetCachedStatement(m_stmt, sql.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), (BE_SQLITE_ROW == m_stmt->Step()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
static Json::Value jsonFromFaceMap(CharCP tocStart, CharCP tocEnd, DgnFonts::DbFaceDataDirect::DataId id)
    {
    Json::Value toc;
    Json::Reader reader;
    if (!reader.parse(tocStart, tocEnd, toc, false) || !toc.isArray())
        { BeAssert(false); }

    return toc;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
static Json::Value jsonFromFaceMap(EmbeddedFaceDataIterator::Entry const& faceDataEntry)
    {
    CharCP tocStart = faceDataEntry.GetToc();
    CharCP tocEnd = (tocStart + faceDataEntry.GetTocSize());
    return jsonFromFaceMap(tocStart, tocEnd, faceDataEntry.GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
static DgnFonts::DbFaceDataDirect::FaceKey faceKeyFromJson(JsonValueCR json) { return DgnFonts::DbFaceDataDirect::FaceKey((DgnFontType)json["type"].asUInt(), json["familyName"].asCString(), json["faceName"].asCString()); }
static DgnFonts::DbFaceDataDirect::FaceSubId faceSubIdFromJson(JsonValueCR json) { return (DgnFonts::DbFaceDataDirect::FaceSubId)json["subId"].asUInt(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
uint64_t DgnFonts::DbFaceDataDirect::Iterator::Entry::GetId() const { Verify(); return m_sql->GetValueInt64(0); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
DgnFonts::DbFaceDataDirect::T_FaceMap DgnFonts::DbFaceDataDirect::Iterator::Entry::GenerateFaceMap() const
    {
    Verify();
    
    T_FaceMap faces;
    CharCP tocStart = m_sql->GetValueText(1);
    CharCP tocEnd = (tocStart + m_sql->GetColumnBytes(1));
    Json::Value toc = jsonFromFaceMap(tocStart, tocEnd, m_sql->GetValueInt64(0));
    for (Json::Value::const_iterator faceKeyIter = ((JsonValueCR)toc).begin(); ((JsonValueCR)toc).end() != faceKeyIter; ++faceKeyIter)
        faces[faceSubIdFromJson(*faceKeyIter)] = faceKeyFromJson(*faceKeyIter);
    
    return faces;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
DgnFonts::DbFaceDataDirect::Iterator::const_iterator DgnFonts::DbFaceDataDirect::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sql = MakeSqlString("SELECT Id,StrData FROM " BEDB_TABLE_Property " WHERE Namespace='" EMBEDDED_FACE_DATA_PROP_NS "' AND Name='" EMBEDDED_FACE_DATA_PROP_NAME "'", true);
        m_db->GetCachedStatement(m_stmt, sql.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), (BE_SQLITE_ROW == m_stmt->Step()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
size_t DgnFonts::DbFaceDataDirect::Iterator::QueryCount() const
    {
    Utf8String sql = MakeSqlString("SELECT COUNT(*) FROM " BEDB_TABLE_Property " WHERE Namespace='" EMBEDDED_FACE_DATA_PROP_NS "' AND Name='" EMBEDDED_FACE_DATA_PROP_NAME "'", true);

    Statement query;
    query.Prepare(*m_db, sql.c_str());
    m_params.Bind(query);

    if (BE_SQLITE_ROW != query.Step())
        {
        BeAssert(false);
        return 0;
        }

    return (size_t)query.GetValueInt(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnFonts::DbFaceDataDirect::QueryById(bvector<Byte>& fontData, DataId id)
    {
    uint32_t fontDataSize;
    if (BE_SQLITE_ROW != m_dbFonts.m_db.QueryPropertySize(fontDataSize, EMBEDDED_FACE_DATA_PROPERTY_SPEC, id, 0))
        return ERROR;
    
    fontData.resize(fontDataSize);
    if (BE_SQLITE_ROW != m_dbFonts.m_db.QueryProperty(&fontData[0], fontDataSize, EMBEDDED_FACE_DATA_PROPERTY_SPEC, id, 0))
        return ERROR;
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnFonts::DbFaceDataDirect::QueryByFace(bvector<Byte>& data, FaceSubId& subId, FaceKeyCR key)
    {
    EmbeddedFaceDataIterator allFaceData(m_dbFonts.m_db);
    for (EmbeddedFaceDataIterator::Entry const& faceDataEntry : allFaceData)
        {
        Json::Value toc = jsonFromFaceMap(faceDataEntry);
        for (Json::Value::const_iterator faceKeyIter = ((JsonValueCR)toc).begin(); ((JsonValueCR)toc).end() != faceKeyIter; ++faceKeyIter)
            {
            FaceKey currFaceKey = faceKeyFromJson(*faceKeyIter);
            if (!key.Equals(currFaceKey))
                continue;
            
            subId = faceSubIdFromJson(*faceKeyIter);
            
            data.resize(faceDataEntry.GetRawSize());
            if (BE_SQLITE_ROW != m_dbFonts.m_db.QueryProperty(&data[0], (uint32_t)data.size(), EMBEDDED_FACE_DATA_PROPERTY_SPEC, faceDataEntry.GetId(), 0))
                return ERROR;
            
            return SUCCESS;
            }
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
bool DgnFonts::DbFaceDataDirect::Exists(FaceKeyCR key)
    {
    EmbeddedFaceDataIterator allFaceData(m_dbFonts.m_db);
    for (EmbeddedFaceDataIterator::Entry const& faceDataEntry : allFaceData)
        {
        Json::Value toc = jsonFromFaceMap(faceDataEntry);
        for (Json::ValueConstIterator faceKeyIter = ((JsonValueCR)toc).begin(); ((JsonValueCR)toc).end() != faceKeyIter; ++faceKeyIter)
            {
            FaceKey currFaceKey = faceKeyFromJson(*faceKeyIter);
            if (!key.Equals(currFaceKey))
                continue;

            return true;
            }
        }
    
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnFonts::DbFaceDataDirect::Insert(Byte const* data, size_t size, T_FaceMapCR keys)
    {
    Statement idQuery;
    idQuery.Prepare(m_dbFonts.m_db, "SELECT MAX(Id) FROM " BEDB_TABLE_Property " WHERE Namespace='" EMBEDDED_FACE_DATA_PROP_NS "' AND Name='" EMBEDDED_FACE_DATA_PROP_NAME "'");
    if (BE_SQLITE_ROW != idQuery.Step())
        {
        BeAssert(false);
        return ERROR;
        }

    DataId id = (idQuery.GetValueInt64(0) + 1);

    // Db does not expose a SaveProperty that lets us save a string and blob at the same time.
    // Use the API for the blob so we get compression; manually insert the table of contents string.
    
    if (BE_SQLITE_OK != m_dbFonts.m_db.SaveProperty(EMBEDDED_FACE_DATA_PROPERTY_SPEC, data, (uint32_t)size, id, 0))
        return ERROR;

    Json::Value toc(Json::ValueType::arrayValue);
    for (T_FaceMap::value_type const& keyEntry : keys)
        {
        FaceKeyCR faceKey = keyEntry.second;
        
        Json::Value entry(Json::ValueType::objectValue);
        entry["subId"] = (Json::UInt)keyEntry.first;
        entry["type"] = (Json::UInt)faceKey.m_type;
        entry["familyName"] = faceKey.m_familyName.c_str();
        entry["faceName"] = faceKey.m_faceName.c_str();
        
        toc.append(entry);
        }
    
    Utf8String tocString = Json::FastWriter::ToString(toc);

    Statement update;
    update.Prepare(m_dbFonts.m_db, "UPDATE " BEDB_TABLE_Property " SET StrData=? WHERE Namespace='" EMBEDDED_FACE_DATA_PROP_NS "' AND Name='" EMBEDDED_FACE_DATA_PROP_NAME "' AND Id=?");
    update.BindText(1, tocString.c_str(), Statement::MakeCopy::No);
    update.BindInt64(2, id);

    if (BE_SQLITE_DONE != update.Step())
        {
        BeAssert(false);
        return ERROR;
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnFonts::DbFaceDataDirect::Delete(FaceKeyCR key)
    {
    EmbeddedFaceDataIterator allFaceData(m_dbFonts.m_db);
    for (EmbeddedFaceDataIterator::Entry const& faceDataEntry : allFaceData)
        {
        Json::Value toc = jsonFromFaceMap(faceDataEntry);
        Json::ArrayIndex iFoundEntry = 0;
        for (; iFoundEntry < toc.size(); ++iFoundEntry)
            {
            FaceKey currFaceKey = faceKeyFromJson(toc[iFoundEntry]);
            if (!key.Equals(currFaceKey))
                continue;

            break;
            }
        
        if (toc.size() == iFoundEntry)
            continue;
        
        toc.removeIndex(iFoundEntry);
        
        // No more faces for this entry? Delete it entirely.
        if (toc.empty())
            {
            if (BE_SQLITE_DONE != m_dbFonts.m_db.DeleteProperty(EMBEDDED_FACE_DATA_PROPERTY_SPEC, faceDataEntry.GetId(), 0))
                return ERROR;

            return SUCCESS;
            }

        // Otherwise persist the trimmed table of contents.
        Utf8String tocString = Json::FastWriter::ToString(toc);

        Statement update;
        update.Prepare(m_dbFonts.m_db, "UPDATE " BEDB_TABLE_Property " SET StrData=? WHERE Namespace='" EMBEDDED_FACE_DATA_PROP_NS "' AND Name='" EMBEDDED_FACE_DATA_PROP_NAME "' AND Id=?");
        update.BindText(1, tocString.c_str(), Statement::MakeCopy::No);
        update.BindInt64(2, faceDataEntry.GetId());

        if (BE_SQLITE_DONE != update.Step())
            {
            BeAssert(false);
            return ERROR;
            }

        return SUCCESS;
        }
    
    return ERROR;
    }

BEGIN_UNNAMED_NAMESPACE
// NB: This is static, rather than being a member of DgnFonts, because DgnFont, DgnFontDataSession, IDgnFontData, et al have no backpointer to the DgnFonts or DgnDb to which they belong.
// It is assumed that acquirers of this mutex will not need to acquire additional mutexes, or will order their locks appropriately.
static BeMutex s_fontsMutex;
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeMutex& DgnFonts::GetMutex()
    {
    return s_fontsMutex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
void DgnFonts::Update()
    {
    if (m_isFontMapLoaded)
        return;
    
    BeMutexHolder lock(GetMutex());
    if (m_isFontMapLoaded)
        return;

    for (DbFontMapDirect::Iterator::Entry const& entry : DbFontMap().MakeIterator())
        {
        DgnFontId id = entry.GetId();
        DgnFontPtr font = DbFontMap().QueryById(id);
        if (!font.IsValid())
            font = DgnFontManager::GetAnyLastResortFont().Clone();
        
        m_fontMap[id] = font;
        }

    m_isFontMapLoaded = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontCP DgnFonts::FindFontById(DgnFontId id) const
    {
    const_cast<DgnFonts*>(this)->Update();
    T_FontMap::const_iterator iter = m_fontMap.find(id);
    return ((m_fontMap.end() != iter) ? iter->second.get() : nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontCP DgnFonts::FindFontByTypeAndName(DgnFontType type, Utf8CP name) const
    {
    const_cast<DgnFonts*>(this)->Update();

    for (T_FontMap::value_type const& entry : m_fontMap)
        {
        DgnFontCR existingFont = *entry.second;
        if ((existingFont.GetType() == type) && existingFont.GetName().EqualsI(name))
            return entry.second.get();
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontId DgnFonts::FindId(DgnFontCR font) const
    {
    const_cast<DgnFonts*>(this)->Update();
    
    for (T_FontMap::value_type const& entry : m_fontMap)
        {
        DgnFontCR existingFont = *entry.second;
        if ((font.GetType() == existingFont.GetType()) && font.GetName().EqualsI(existingFont.GetName()))
            return entry.first;
        }

    return DgnFontId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontId DgnFonts::AcquireId(DgnFontCR font)
    {
    // This calls Update for us.
    DgnFontId existingId = FindId(font);
    if (existingId.IsValid())
        return existingId;

    DgnFontId newId;
    if (SUCCESS != DbFontMap().Insert(font, newId))
        return DgnFontId();

    m_fontMap[newId] = DbFontMap().QueryById(newId);

    return newId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
uint32_t const* DgnFont::Utf8ToUcs4(bvector<Byte>& ucs4CharsBuffer, size_t& numUcs4Chars, Utf8StringCR str)
    {
    if (SUCCESS != BeStringUtilities::TranscodeStringDirect(ucs4CharsBuffer, "UCS-4", (Byte const*)str.c_str(), sizeof(Utf8Char) * (str.size() + 1), "UTF-8"))
        return 0;

    // ICU for UCS-4 injects a BOM on the front which we don't want.
    BeAssert(0xfeff == *(uint32_t const*)&ucs4CharsBuffer[0]);
    uint32_t const* ucs4Chars = (uint32_t const*)&ucs4CharsBuffer[sizeof(uint32_t)];
    size_t maxNumUcs4Chars = ((ucs4CharsBuffer.size() - 1) / 4);
    numUcs4Chars = 0;
    while ((0 != ucs4Chars[numUcs4Chars]) && (numUcs4Chars < maxNumUcs4Chars))
        ++numUcs4Chars;

    return ucs4Chars;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
void DgnFont::ScaleAndOffsetGlyphRange(DRange2dR range, DPoint2dCR scale, DPoint2d offset)
    {
    range.low.x = (range.low.x * scale.x) + offset.x;
    range.low.y = (range.low.y * scale.y) + offset.y;
    range.high.x = (range.high.x * scale.x) + offset.x;
    range.high.y = (range.high.y * scale.y) + offset.y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
bool DgnFontDataSession::Start()
    {
    if (!m_hasStarted)
        {
        BeMutexHolder lock(DgnFonts::GetMutex());
        if (!m_hasStarted)
            {
            m_isValid = ((nullptr != m_data) && (SUCCESS == m_data->_AddDataRef()));
            m_hasStarted = true;
            }
        }
    
    return m_isValid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
void DgnFontDataSession::Stop()
    {
    BeMutexHolder lock(DgnFonts::GetMutex());

    if (m_isValid)
        m_data->_ReleaseDataRef();
    
    m_hasStarted = false;
    m_isValid = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFont::~DgnFont() { m_dataSession.Stop(); DELETE_AND_CLEAR(m_data); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
void DgnFont::CopyFrom(DgnFontCR rhs)
    {
    m_type = rhs.m_type;
    m_name = rhs.m_name;
    m_data = ((nullptr != rhs.m_data) ? rhs.m_data->_CloneWithoutData() : nullptr);
    m_dataSession.Reset(m_data);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
bool DgnFont::IsResolved() const
    {
    if (nullptr == m_data)
        return false;
    
    return m_dataSession.Start();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2015
//---------------------------------------------------------------------------------------
DgnFontStyle DgnFont::FontStyleFromBoldItalic(bool isBold, bool isItalic)
    {
    DgnFontStyle fontStyle = DgnFontStyle::Regular;
    if (isBold && isItalic)
        fontStyle = DgnFontStyle::BoldItalic;
    else if (isBold)
        fontStyle = DgnFontStyle::Bold;
    else if (isItalic)
        fontStyle = DgnFontStyle::Italic;
    
    return fontStyle;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2015
//---------------------------------------------------------------------------------------
void DgnFont::FontStyleToBoldItalic(bool& isBold, bool& isItalic, DgnFontStyle fontStyle)
    {
    isBold = ((DgnFontStyle::Bold == fontStyle) || (DgnFontStyle::BoldItalic == fontStyle));
    isItalic = ((DgnFontStyle::Italic == fontStyle) || (DgnFontStyle::BoldItalic == fontStyle));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontPtr DgnFontPersistence::Db::FromDb(DgnFonts& dbFonts, DgnFontId id, DgnFontType type, Utf8CP name, Byte const* metadata, size_t metadataSize)
    {
    switch (type)
        {
        case DgnFontType::TrueType: return DgnTrueTypeFontFromDb(dbFonts, id, name, metadata, metadataSize);
        case DgnFontType::Rsc: return DgnRscFontFromDb(dbFonts, id, name, metadata, metadataSize);
        case DgnFontType::Shx: return DgnShxFontFromDb(dbFonts, id, name, metadata, metadataSize);
        }
    
    BeAssert(false);
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnFontPersistence::Db::MetadataToDb(bvector<Byte>& metadata, DgnFontCR font)
    {
    switch (font.GetType())
        {
        case DgnFontType::TrueType: return SUCCESS;
        case DgnFontType::Rsc: return DgnRscFontMetadataToDb(metadata, static_cast<DgnRscFontCR>(font));
        case DgnFontType::Shx: return DgnShxFontMetadataToDb(metadata, static_cast<DgnShxFontCR>(font));
        }

    BeAssert(false);
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnFontPersistence::Db::Embed(DgnFonts::DbFaceDataDirect& faceData, DgnFontCR font)
    {
    IDgnFontDataP data = font.m_data;
    if (nullptr == data)
        return ERROR;
    
    return data->_Embed(faceData);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2015
//---------------------------------------------------------------------------------------
bool DgnFontPersistence::Db::IsAnyFaceEmbedded(DgnFontCR font, DgnFonts::DbFaceDataDirect& fontData)
    {
    // As opposed to adding a method to IDgnFontData or doing dynamic casting, I'm choosing this runaround until proven inadequate.
    return (fontData.Exists(DgnFonts::DbFaceDataDirect::FaceKey(font.GetType(), font.GetName().c_str(), DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Regular))
        || fontData.Exists(DgnFonts::DbFaceDataDirect::FaceKey(font.GetType(), font.GetName().c_str(), DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Bold))
        || fontData.Exists(DgnFonts::DbFaceDataDirect::FaceKey(font.GetType(), font.GetName().c_str(), DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Italic))
        || fontData.Exists(DgnFonts::DbFaceDataDirect::FaceKey(font.GetType(), font.GetName().c_str(), DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_BoldItalic)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontPtr DgnFontPersistence::Missing::CreateMissingFont(DgnFontType type, Utf8CP name)
    {
    switch (type)
        {
        case DgnFontType::TrueType: return new DgnTrueTypeFont(name, nullptr);
        case DgnFontType::Rsc: return new DgnRscFont(name, nullptr);
        case DgnFontType::Shx: return new DgnShxFont(name, nullptr);
        }

    BeAssert(false);
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2015
//---------------------------------------------------------------------------------------
DgnFontId DgnImportContext::_RemapFont(DgnFontId srcId)
    {
    // Alreay remapped once? Use cached result.
    DgnFontId dstId = m_remap.Find(srcId);
    if (dstId.IsValid())
        return dstId;
    
    // N.B. This logic assumes that font type and name is unique enough for remapping purposes.
    // While it is technically possible for a font with the same type and name to have differing metadata or glyph data, this should be rare enough in practice that we don't want to code around that scenario.

    DgnFontCP srcFont = m_sourceDb.Fonts().FindFontById(srcId);
    if (nullptr == srcFont)
        {
        // Invalid source ID? Options might include:
        //      (a) simply retain the ID -- bad because who knows if this ID is or will become valid in the destination and thus result in different behavior
        //      (b) remap to 0 (the invalid ID) -- would effectively keep behavior, but also not great because we'd be propagating junk into the destination
        //      (c) remap to a last resort font -- change in display and behavior, but would ensure usable data is written to the destination
        //  I'll side with (c) right now... as a "least bad" option.
        BeDataAssert(false);
        srcFont = &T_HOST.GetFontAdmin().GetAnyLastResortFont();
        }

    // Does it already exist in the destination? Use the destination ID. Don't bother bringing over glyph data in this case; ensure the destination continues to act the same.
    DgnFontCP dstFont = m_destDb.Fonts().FindFontByTypeAndName(srcFont->GetType(), srcFont->GetName().c_str());
    if (nullptr != dstFont)
        {
        dstId = m_destDb.Fonts().FindId(*dstFont);
        BeDataAssert(dstId.IsValid()); // If FindFontByTypeAndName returned a font, it'd better have an ID. No good way to recover anyway.
        return m_remap.Add(srcId, dstId);
        }
    
    // Doesn't exist in the destination? Create a copy.
    dstId = m_destDb.Fonts().AcquireId(*srcFont);
    BeDataAssert(dstId.IsValid()); // No good way to recover.
    
    // If the source had embedded glyph data, copy it as well. Don't force embedding in case the app interjects and manages separately.
    if (DgnFontPersistence::Db::IsAnyFaceEmbedded(*srcFont, m_sourceDb.Fonts().DbFaceData()))
        DgnFontPersistence::Db::Embed(m_destDb.Fonts().DbFaceData(), *srcFont);
    
    return m_remap.Add(srcId, dstId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool mightHaveHole(GPArrayCP gpa)
    {
    size_t  loopCount = 0;

    for (int loopStart = 0, loopEnd = 0; loopStart < gpa->GetCount(); loopStart = loopEnd+1)
        {
        GraphicsPoint const* gPt = gpa->GetConstPtr(loopStart);

        if (gPt && (HMASK_RSC_HOLE == (gPt->mask & HMASK_RSC_HOLE))) // NOTE: Mask value weirdness, hole shares bits with line/poly...
            return true;

        loopEnd = jmdlGraphicsPointArray_findMajorBreakAfter(gpa, loopStart);
        loopCount++;
        }

    return loopCount > 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isPhysicallyClosed(ICurvePrimitiveCR primitive)
    {
    switch (primitive.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            return (primitive.GetLineStringCP ()->size() > 3 && primitive.GetLineStringCP ()->front().IsEqual(primitive.GetLineStringCP ()->back()));

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            return primitive.GetArcCP ()->IsFullEllipse();

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            return (primitive.GetProxyBsplineCurveCP ()->IsClosed());

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  DgnGlyph::GetCurveVector(bool& isFilled) const
    {
    CurveVectorPtr      curveVector = _GetCurveVector(isFilled);    // Give subclasses a chance to implement directly (instead of going through (stupid) GPA).

    if (curveVector.IsValid())
        return curveVector;

    GPArraySmartP  gpaText;

    if (SUCCESS != FillGpa(gpaText) || 0 == gpaText->GetCount())
        return curveVector;;
    
    isFilled = (0 != gpaText->GetArrayMask(HPOINT_ARRAYMASK_FILL));

    if (DoFixup::Always == _DoFixup() || (DoFixup::IfHoles == _DoFixup() && isFilled && mightHaveHole(gpaText)))
        {
        CurveVectorPtr  curves = gpaText->CreateCurveVector();

        if (!curves.IsValid())
            return curves;
            
        curves->ConsolidateAdjacentPrimitives();
        curves->FixupXYOuterInner();

        return curves;
        }

    // Create curve vector that is just a collection of curves and not an open/closed path or region...
    BentleyStatus            status = SUCCESS;
    bvector<CurveVectorPtr>  glyphCurves;

    for (int i=0, count = gpaText->GetCount(); i < count && SUCCESS == status; )
        {
        bool                isPoly = 0 != (gpaText->GetConstPtr(i)->mask & HMASK_RSC_POLY);
        ICurvePrimitivePtr  primitive;

        switch (gpaText->GetCurveType(i))
            {
            case GPCurveType::LineString:
                {
                bvector<DPoint3d> points;

                if (SUCCESS != (status = gpaText->GetLineString(&i, points)))
                    break;

                primitive = ICurvePrimitive::CreateLineString(points);
                break;
                }

            case GPCurveType::Ellipse:
                {
                DEllipse3d  ellipse;

                if (SUCCESS != (status = gpaText->GetEllipse(&i, &ellipse)))
                    break;

                primitive = ICurvePrimitive::CreateArc(ellipse);
                break;
                }

            case GPCurveType::Bezier:
            case GPCurveType::BSpline:
                {
                MSBsplineCurve  bcurve;

                if (SUCCESS != (status = gpaText->GetBCurve(&i, &bcurve)))
                    break;

                primitive = ICurvePrimitive::CreateBsplineCurve(bcurve);
                bcurve.ReleaseMem();
                break;
                }

            default:
                {
                i++;
                break;
                }
            }

        if (!primitive.IsValid())
            continue;

        CurveVectorPtr  singleCurve = CurveVector::Create((isPoly && isPhysicallyClosed(*primitive)) ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open);

        singleCurve->push_back(primitive);
        glyphCurves.push_back(singleCurve);
        }

    size_t  nGlyphCurves = glyphCurves.size();

    if (0 == nGlyphCurves)
        return CurveVectorPtr(); // Empty glyph?!?

    if (1 == nGlyphCurves)
        {
        return glyphCurves.front(); // NOTE: Glyph fill flag should be set correctly for this case...
        }
    else
        {
        size_t  nClosed = 0, nOpen = 0;
        CurveVectorPtr  curves;

        for (CurveVectorPtr singleCurve: glyphCurves)
            {
            if (singleCurve->IsClosedPath())
                nClosed++;
            else
                nOpen++;
            }

        // NOTE: Create union region if all closed, create none for all open or mix...
        curves = CurveVector::Create((nClosed == nGlyphCurves) ? CurveVector::BOUNDARY_TYPE_UnionRegion : CurveVector::BOUNDARY_TYPE_None);

        for (CurveVectorPtr singleCurve: glyphCurves)
            {
            if (nOpen == nGlyphCurves)
                curves->push_back(singleCurve->front()); // NOTE: Flatten hierarchy, better to not have child vectors for disjoint collection of sticks...
            else
                curves->push_back(ICurvePrimitive::CreateChildCurveVector_SwapFromSource(*singleCurve));
            }

        if (0 != nClosed && 0 != nOpen)
            isFilled = true; // NOTE: Poly in mixed glyph treated as filled but glyph fill flag isn't set...

        return curves;
        }
    }
