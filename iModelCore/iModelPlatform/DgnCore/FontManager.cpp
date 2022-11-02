/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define EMBEDDED_FACE_DATA_NAMESPACE "dgn_Font"
#define EMBEDDED_FACE_DATA_NAME "EmbeddedFaceData"

/** Load the data for a FontFileReader from its Db. If the data for the font is compressed,
 * this will decompress the data into memory. Otherwise it opens a BlobIO to read the data
 * @note It is safe to call this more than once and from multiple threads, it does nothing
 * after the fist time.
 */
void FontDbReader::Load() {
    FontManager::FlagHolder isLoaded(m_loaded);
    if (isLoaded)
        return;

    // for compressed blobs, we have to decompress the data into memory since we can't read it directly
    if (m_compressed) {
        m_data.Resize(m_nBytes);
        if (BE_SQLITE_ROW != m_db.QueryProperty(m_data.data(), m_nBytes, PropertySpec(EMBEDDED_FACE_DATA_NAME, EMBEDDED_FACE_DATA_NAMESPACE), m_id, 0))
            m_data.Clear();
    } else {
        DbResult stat = m_blobIO.Open(m_db, BEDB_TABLE_Property, "Data", m_rowId, false);
        UNUSED_VARIABLE(stat);
        BeAssert(stat == BE_SQLITE_OK);
        m_blobPos = 0;
    }
}

/** unload the data for this reader and/or close its blobIO */
void FontDbReader::Unload() {
    BeMutexHolder lock(FontManager::GetMutex());
    m_data.Clear();
    m_blobIO.Close();
    m_blobPos = 0;
    m_loaded = false;
}

/**
 * Read `size` bytes for this font, starting at the current position. If the data is "filled" (i.e. in memory)
 * it is copied from memory. Otherwise it is read from the blob in the FontDb.
 */
bool FontDbReader::ReadBytes(void* buf, uint32_t size) {
    if (m_data.size() > 0) // if there is data is in our StreamBuffer, use it
        return m_data.ReadBytes(buf, size);

    if (BE_SQLITE_OK != m_blobIO.Read(buf, size, m_blobPos)) {
        BeAssert(false); // what happened?
        return false;
    }

    m_blobPos += size;
    return true;
}

/**
 *
 */
void FontDbReader::SetPos(uint32_t pos) {
    if (m_data.size() > 0)
        m_data.SetPos(pos);
    else
        m_blobPos = pos;
}

bool FontDbReader::Skip(uint32_t numBytes) {
    if (m_data.size() > 0)
        return m_data.Skip(numBytes);

    if ((m_blobPos + numBytes) > m_nBytes)
        return false;

    m_blobPos += numBytes;
    return true;
}

uint32_t FontDbReader::GetPos() const {
    return m_data.size() > 0 ? m_data.GetPos() : m_blobPos;
}

/**
 * For RscFonts we need to read the entire file into a memory buffer.
 * This method makes sure that's been done (it will always be true if the font is compressed), and then
 * returns a pointer to the start of data.
*/
uint8_t* FontDbReader::FillAndGetDataP() {
    if (m_data.size() == 0) {
        m_data.Resize(m_nBytes);
        ReadBytes(m_data.GetDataP(), m_nBytes);
    }
    return m_data.GetDataP();
}

/** A WorkspaceDb holding fonts */
struct FontWorkspaceDb : RefCounted<Db> {
    FontDb m_fontDb;
    FontWorkspaceDb() : m_fontDb(*this, true) {}
};

/** Convert a JSON-stringified-FaceStyle to a FaceStyle */
FaceStyle FontFace::StyleFromString(Utf8StringCR faceName) {
    if (faceName.EqualsI(FACE_NAME_Bold))
        return FaceStyle::Bold;
    if (faceName.EqualsI(FACE_NAME_BoldItalic))
        return FaceStyle::BoldItalic;
    if (faceName.EqualsI(FACE_NAME_Italic))
        return FaceStyle::Italic;
    return FaceStyle::Regular;
}

/** JSON Stringify a FaceStyle */
Utf8String FontFace::StyleToString(FaceStyle style) {
    if (style == FaceStyle::Bold)
        return FACE_NAME_Bold;
    if (style == FaceStyle::BoldItalic)
        return FACE_NAME_BoldItalic;
    if (style == FaceStyle::Italic)
        return FACE_NAME_Italic;
    return FACE_NAME_Regular;
}

/** Add the encoding values from this FontEncoding to a JSON object. */
void FontEncoding::ToJSON(BeJsValue val) const {
    val[FontFace::json_codePage()] = (int) m_codePage;
    val[FontFace::json_degree()] = m_degreeCode;
    val[FontFace::json_diameter()] = m_diameterCode;
    val[FontFace::json_plusMinus()] = m_plusMinusCode;
}

/** Initialize this FontEncoding from JSON */
void FontEncoding::FromJSON(BeJsConst encoding, FontType type) {
    m_codePage = (LangCodePage)encoding[FontFace::json_codePage()].asInt((int32_t)(type == FontType::Rsc ? LangCodePage::Unknown : LangCodePage::Unicode));
    m_degreeCode = encoding[FontFace::json_degree()].asUInt(type == FontType::Rsc ? 94 : 176);
    m_plusMinusCode = encoding[FontFace::json_plusMinus()].asUInt(type == FontType::Rsc ? 200 : 177);
    m_diameterCode = encoding[FontFace::json_diameter()].asUInt(type == FontType::Rsc ? 216 : 8709);
}

/** Construct a FontFace from JSON values */
FontFace::FontFace(BeJsConst val) {
    m_type = (FontType)val[json_type()].asUInt((uint32_t)FontType::TrueType);
    m_subId = val[json_subId()].asUInt(0);
    m_familyName = val[json_familyName()].asString();
    m_faceStyle = StyleFromString(val[json_faceName()].asString());
    m_encoding.FromJSON(val[json_encoding()], m_type); //  must be after m_type is set
    m_isFallback = val[json_fallback()].asBool(false);
}

/** Embed the data for a set of font faces into this FontDb */
BentleyStatus FontDb::EmbedFont(bvector<FontFace> const& faces, ByteStreamCR data, bool compress) {
    // make sure we don't already have any faces in this fontFile
    for (auto face : faces) {
        auto font = FindFont(face.m_type, face.m_familyName.c_str());
        if (font && font->HasFace(face.m_faceStyle))
            return ERROR; // already present
    }
    Statement idQuery;
    idQuery.Prepare(m_db, "SELECT MAX(Id) FROM " BEDB_TABLE_Property " WHERE Namespace='" EMBEDDED_FACE_DATA_NAMESPACE "' AND Name='" EMBEDDED_FACE_DATA_NAME "'");
    if (BE_SQLITE_ROW != idQuery.Step()) {
        BeAssert(false);
        return ERROR;
    }

    uint32_t id = (idQuery.GetValueInt(0) + 1);
    PropertySpec spec(EMBEDDED_FACE_DATA_NAME, EMBEDDED_FACE_DATA_NAMESPACE, PropertySpec::Mode::Normal, compress ? PropertySpec::Compress::Yes : PropertySpec::Compress::No);
    if (BE_SQLITE_OK != m_db.SaveProperty(spec, data.GetData(), data.GetSize(), id, 0))
        return ERROR;

    Statement stmt;
    stmt.Prepare(m_db, "SELECT rowId FROM " BEDB_TABLE_Property " WHERE Namespace='" EMBEDDED_FACE_DATA_NAMESPACE "' AND Name='" EMBEDDED_FACE_DATA_NAME "' AND Id=?");
    stmt.BindInt64(1, id);
    if (BE_SQLITE_ROW != stmt.Step()) {
        BeAssert(false);
        return ERROR;
    }
    uint32_t rowId = stmt.GetValueInt(0);

    BeJsDocument facesJson;
    for (auto const& face : faces) {
        auto entry = facesJson.appendObject();
        entry[FontFace::json_subId()] = face.m_subId;
        entry[FontFace::json_type()] = (int)face.m_type;
        entry[FontFace::json_familyName()] = face.m_familyName;
        entry[FontFace::json_faceName()] = FontFace::StyleToString(face.m_faceStyle);

        if (face.m_encoding.m_codePage != LangCodePage::Unicode)
            face.m_encoding.ToJSON(entry[FontFace::json_encoding()]);
    }

    Statement update;
    update.Prepare(m_db, "UPDATE " BEDB_TABLE_Property " SET StrData=? WHERE rowId=?");
    update.BindText(1, facesJson.Stringify(), Statement::MakeCopy::Yes);
    update.BindInt64(2, rowId);
    if (BE_SQLITE_DONE != update.Step()) {
        BeAssert(false);
        return ERROR;
    }

    auto reader = new FontDbReader(m_db, rowId, id, data.GetSize(), compress);
    for (auto& face : faces)
        reader->m_faces.push_back(face);

    AddDbReader(reader);
    return SUCCESS;
}

/**
 * Embed either an .shx file or a TrueType file, by file name, into this FontDb
 */
BentleyStatus FontDb::EmbedFontFile(Utf8CP fileName, bool compress) {
    BeFileName fileW(fileName, true);
    if (fileW.empty())
        return ERROR;

    WString extension = fileW.GetExtension();
    WString fileBase = fileW.GetFileNameWithoutExtension();
    extension.ToLower();

    if (extension.Equals(L"shx")) {
        BeFile shxFile;
        if (BeFileStatus::Success != shxFile.Open(fileW, BeFileAccess::Read))
            return ERROR;

        ByteStream data;
        shxFile.ReadEntireFile(data);
        shxFile.Close();

        auto shxType = ShxFont::ValidateHeader((CharCP)data.GetData());
        if (shxType == ShxFont::ShxType::Invalid)
            return ERROR;

        bvector<FontFace> faces;
        faces.emplace_back(FontFace(FontType::Shx, Utf8String(fileBase).c_str(), FaceStyle::Regular));
        return EmbedFont(faces, data, compress);
    }

    if (extension.Equals(L"ttf") || extension.Equals(L"ttc") || extension.Equals(L"otf") || extension.Equals(L"otc")) {
        TrueTypeFile ttFile(fileName, compress);
        return ttFile.Embed(*this) ? SUCCESS : ERROR;
    }

    return ERROR;
}

/** Empty the map of fonts for FontType. Deletes all fonts owned by it.
 *  @note this can't be in header file due to circular dependencies
 */
void FontDb::FontsByName::Empty() {
    for (auto& font : m_map)
        delete font.second;
}

/**
 * Add a FontDbReader to this FontDb.
 * @note This method will create a DbFont object of the appropiate type for all familyNames in the FontDbReader, as necessary.
 * If a font object already exists, its reader will be set for the faces found in the FontDbReader.
 */
void FontDb::AddDbReader(FontDbReaderPtr reader) const {
    if (reader->m_faces.size() <= 0)
        return;

    m_fileReaders.push_back(reader);
    FontType fontType = reader->m_faces[0].m_type; // this is stored on every face, but they all have to be the same.
    auto& fonts = GetMap(fontType);
    for (auto& face : reader->m_faces) {
        auto familyName = face.m_familyName.c_str();
        auto font = fonts.Find(familyName);
        if (font == nullptr) {
            switch (fontType) {
            case FontType::Rsc:
                font = new RscFont(familyName, *this);
                break;
            case FontType::Shx:
                font = new ShxFont(familyName, *this);
                break;
            default:
                font = new TrueTypeFont(familyName, *this);
                break;
            }
            fonts.Insert(familyName, font);
        }
        font->SetReader(face.m_faceStyle, face, *reader);
    }
}

/**
 * Load all of the fonts stored in this FontDb.
 * Creates DbFont objects for all familyNames found, and saves them on the FontsByName maps (one for each font type.)
 */
void FontDb::Load() const {
    FontManager::FlagHolder isLoaded(m_loaded);
    if (isLoaded)
        return;

    Statement stmt;
    DbResult rc = stmt.Prepare(m_db, "SELECT Id,StrData,RawSize,Data,rowId FROM " BEDB_TABLE_Property " WHERE Namespace='" EMBEDDED_FACE_DATA_NAMESPACE "' AND Name='" EMBEDDED_FACE_DATA_NAME "'");
    if (rc != BE_SQLITE_OK) {
        BeAssert(false);
        return;
    }

    // walk through all EmbeddedFaceData rows, adding FontDbReaders for each
    while (BE_SQLITE_ROW == stmt.Step()) {
        uint32_t id = stmt.GetValueInt(0);
        Utf8String props(stmt.GetValueText(1));
        uint32_t rawSize = stmt.GetValueInt(2);
        uint32_t blobSize = stmt.GetColumnBytes(3);
        uint32_t rowId = stmt.GetValueInt(4);
        bool compressed = 0 != rawSize;

        FontDbReaderPtr reader = new FontDbReader(m_db, rowId, id, compressed ? rawSize : blobSize, compressed);
        BeJsDocument rows(props);
        rows.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst row) {
            FontFace face(row);
            if (!face.m_familyName.empty())
                reader->m_faces.emplace_back(face);
            return true; // keep going
        });
        AddDbReader(reader);
    }
}

DEFINE_REF_COUNTED_PTR(FontWorkspaceDb);
static std::list<FontWorkspaceDbPtr> s_workspaceDbs;

/** Get a font from any workspaceDb, by name */
DbFontP FontManager::GetWorkspaceFont(FontType fontType, Utf8CP fontName) {
    for (auto const& wsDb : s_workspaceDbs) {
        auto font = wsDb->m_fontDb.FindFont(fontType, fontName);
        if (font)
            return font;
    }
    return nullptr;
}

/** Find a font by type and name in any WorkspaceDb.
 * If font cannot be found in any workspace, a fallback font is returned.
 * @note you can tell whether the requested font was found by comparing the name of the returned font.
 */
DbFontR FontManager::FindFont(FontType fontType, Utf8CP fontName) {
    auto font = GetWorkspaceFont(fontType, fontName);
    return font ? *font : GetFallbackFont(fontType);
}

/** Get the last resort font for a type */
DbFontR FontManager::GetFallbackFont(FontType fontType) {
    DbFontP font = nullptr;
    switch (fontType) {
    case FontType::Rsc:
        font = GetWorkspaceFont(fontType, "FallbackRsc");
        break;
    case FontType::Shx:
        font = GetWorkspaceFont(fontType, "FallbackShx");
        break;
    case FontType::TrueType:
        font = GetWorkspaceFont(fontType, "FallbackTrueType");
        break;
    }
    if (nullptr != font)
        return *font;

    for (auto const& wsDb : s_workspaceDbs) {
        auto firstFont = wsDb->m_fontDb.GetMap(fontType).GetFirst();
        if (firstFont)
            return *firstFont;
    }
    if (fontType != FontType::TrueType)
        return GetFallbackFont(FontType::TrueType);

    BeAssert(false); // no fonts
    return *font;
}

/** Add a WorkspaceDb to the list of font workspaces. */
bool FontManager::AddWorkspaceDb(Utf8CP inName, CloudContainerP container) {
    auto openParams = Db::OpenParams(Db::OpenMode::Readonly);
    openParams.SetImmutable(); // no locks will be held on workspace files. Otherwise iOS complains when we are suspended
    Utf8String dbName = openParams.SetFromContainer(inName, container);

    for (auto wsFile : s_workspaceDbs) {
        if (dbName.EqualsI(wsFile->GetDbFileName()))
            return false; // already open
    }

    FontWorkspaceDbPtr fontWs = new FontWorkspaceDb();
    if (BE_SQLITE_OK != fontWs->OpenBeSQLiteDb(dbName.c_str(), openParams))
        return false;
    s_workspaceDbs.emplace_back(fontWs);
    return true;
}

/** Get the Mutex for all font related activities */
BeMutex& FontManager::GetMutex() {
    static BeMutex s_fontsMutex;
    return s_fontsMutex;
}

/** Initialize font manager. Adds the default "FallbackFonts" WorkspaceDb. */
void FontManager::Initialize() {
    BeFileName path = PlatformLib::GetHost().GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    path.AppendToPath(L"FallbackFonts.itwin-workspace");
    AddWorkspaceDb(path.GetNameUtf8().c_str(), nullptr);
}

/** Shut down font manager. Releases all workspaceDbs. */
void FontManager::Shutdown() {
    s_workspaceDbs.clear();
}

/** convert "bold" and "italic" boolean flags to a FaceStyle */
FaceStyle FontManager::FaceStyleFromBoldItalic(bool isBold, bool isItalic) {
    if (isBold && isItalic)
        return FaceStyle::BoldItalic;
    if (isBold)
        return FaceStyle::Bold;
    return isItalic ?  FaceStyle::Italic : FaceStyle::Regular;
}

/** convert a FaceStyle to  "bold" and "italic" boolean flags */
void FontManager::FaceStyleToBoldItalic(bool& isBold, bool& isItalic, FaceStyle fontStyle) {
    isBold = FaceStyle::Bold == fontStyle || FaceStyle::BoldItalic == fontStyle;
    isItalic = FaceStyle::Italic == fontStyle || FaceStyle::BoldItalic == fontStyle;
}
