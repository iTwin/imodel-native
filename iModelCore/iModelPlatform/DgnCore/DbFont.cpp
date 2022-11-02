/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/** set a FontReader for a face in this DbFont */
void DbFont::SetReader(FaceStyle style, FontFaceR face, FontDbReaderR reader) {
    switch (style) {
    case FaceStyle::Bold:
        m_bold.Init(face, reader);
        break;
    case FaceStyle::BoldItalic:
        m_boldItalic.Init(face, reader);
        break;
    case FaceStyle::Italic:
        m_italic.Init(face, reader);
        break;
    default:
        m_regular.Init(face, reader);
        break;
    }
}

/** Find the ReaderForFace for a font by FaceStyle. If no reader exists for the requested face, attempt to return another (fallback) face. */
ReaderForFaceR DbFont::FindReader(FaceStyle style) {
    BeAssert(m_regular.m_reader || m_bold.m_reader || m_boldItalic.m_reader || m_italic.m_reader);
    if (style == FaceStyle::Bold)
        return m_bold.m_reader ? m_bold : FindReader(FaceStyle::Regular);
    if (style == FaceStyle::Italic)
        return m_italic.m_reader ? m_italic : FindReader(FaceStyle::Regular);
    if (style == FaceStyle::BoldItalic)
        return m_boldItalic.m_reader ? m_boldItalic : m_bold.m_reader ? m_bold : FindReader(FaceStyle::Italic);

    return m_regular.m_reader ? m_regular : m_bold.m_reader ? m_bold : m_italic.m_reader ? m_italic : m_boldItalic;
}

/** Get and Load the ReaderForFace for a font by FaceStyle */
ReaderForFaceR DbFont::GetReader(FaceStyle style) {
    ReaderForFaceR reader = FindReader(style);
    reader.m_reader->Load();
    return reader;
}

/**
 * Determine whether this is a fallback font.
 */
 bool DbFont::IsFallback() {
     return FindReader(FaceStyle::Regular).m_face->m_isFallback;
}

/**
 * Insert a new font type/name into the font table for this iModel and return its FontId.
 * @note on success, the Id->font mapping table for this iModel is cleared and will be reloaded the next time it is needed.
 * This does *not* cause any fonts to be unloaded - that only happens when the FontDb they're loaded from is closed.
 * @note this method should only be called with the schema lock held, typically by an administrator setting up an iModel.
 */
FontId DgnDbFonts::InsertFont(FontType fontType, Utf8CP name) {
    FontId id;
    DbResult rc = m_fontDb.m_db.GetServerIssuedId(id, DGN_TABLE_Font, "Id");
    if (BE_SQLITE_OK != rc) {
        BeAssert(false);
        return id;
    }

    Statement insert;
    insert.Prepare(m_fontDb.m_db, "INSERT INTO " DGN_TABLE_Font " (Id,Type,Name,Metadata) VALUES (?,?,?,?)");
    insert.BindId(1, id);
    insert.BindInt(2, (int)fontType);
    insert.BindText(3, name, Statement::MakeCopy::Yes);

    rc = insert.Step();
    if (BE_SQLITE_DONE != rc) {
        BeAssert(false);
        return FontId();
    }

    Invalidate(); // clear id->font table so new font will be found the next time someone uses this id - does NOT unload fonts
    return id;
}

/**
 * Resolve a font type/name for a DgnDb to a DbFont object.
 * Prefer workspace fonts over embedded fonts, because they:
 *  - are more shareable. That is, they may be loaded from previous iModels in this session
 *  - are not compressed so they can be read directly from the FontDb rather than decompressed into (sometimes very large) contiguous memory blocks
 *  - can be updated externally
 *  - should be eliminated and this will make them less necessary
 */
DbFontR DgnDbFonts::ResolveFont(FontType fontType, Utf8CP name, Utf8StringCR metaJson) const {
    DbFontP font = FontManager::GetWorkspaceFont(fontType, name);
    if (nullptr != font)
        return *font;

    font = m_fontDb.FindFont(fontType, name);
    if (nullptr == font)
        return FontManager::GetFallbackFont(fontType);

    // we've found and are going to use a font embedded in the iModel. Due to a flaw in the design of embedding, the "metadata" about the font
    // was stored in the font table, not on the font itself. If it's present, move it over to the font.
    if (font->GetType() != FontType::TrueType && !metaJson.empty()) {
        BeJsDocument meta(metaJson);
        auto encoding = &font->GetReader().m_face->m_encoding;
        if (meta.isNumericMember(FontFace::json_codePage()))
            encoding->m_codePage = (LangCodePage)meta[FontFace::json_codePage()].asInt();
        if (meta.isNumericMember(FontFace::json_degree()))
            encoding->m_degreeCode = meta[FontFace::json_degree()].asUInt();
        if (meta.isNumericMember(FontFace::json_diameter()))
            encoding->m_diameterCode = meta[FontFace::json_diameter()].asUInt();
        if (meta.isNumericMember(FontFace::json_plusMinus()))
            encoding->m_plusMinusCode = meta[FontFace::json_plusMinus()].asUInt();
    }
    return *font;
}

/** Load the FontId -> DbFont mapping table for this iModel.
 * @note it is safe to call this multiple times and from multiple threads, only the first call does anything.
 */
void DgnDbFonts::Load() const {
    FontManager::FlagHolder isLoaded(m_isFontMapLoaded);
    if (isLoaded)
        return;

    Statement stmt;
    stmt.Prepare(m_fontDb.m_db, "SELECT Id,Type,Name,Metadata FROM " DGN_TABLE_Font);
    while (BE_SQLITE_ROW == stmt.Step()) {
        FontId id(stmt.GetValueUInt64(0));
        FontType fontType = FontType::TrueType;
        auto type = stmt.GetValueInt(1);
        if (type == (int)FontType::Rsc)
            fontType = FontType::Rsc;
        else if (type == (int)FontType::Shx)
            fontType = FontType::Shx;
        Utf8String name = stmt.GetValueText(2);
        if (!name.empty())
            m_fontMap.Insert(id, FontInfo(fontType, name.c_str(), &ResolveFont(fontType, name.c_str(), Utf8String(stmt.GetValueText(3)))));
    }
}

/**
 * Get the "info" about a font, by Id
 */
DgnDbFonts::FontInfo const* DgnDbFonts::GetFontInfo(FontId id) const {
    Load();
    auto iter = m_fontMap.find(id);
    return m_fontMap.end() != iter ? &iter->second : nullptr;
}

/** Search the font table for a FontId for a DbFont. If font is not in the font table, return an invalid Id. */
FontId DgnDbFonts::FindId(FontType type, Utf8CP name) const {
    Load();
    for (auto const& entry : m_fontMap) {
        auto& info = entry.second;
        if ((info.m_type == type) && info.m_fontName.EqualsI(name))
            return entry.first;
    }
    return FontId();
}

/** Search the font table for a FontId for a font. If font is not in the font table, add it to the table and return its new Id.
*/
FontId DgnDbFonts::GetId(FontType type, Utf8CP name) {
    BeMutexHolder lock(FontManager::GetMutex()); // We need the mutex in case we or another thread is accessing or mutating the map
    FontId existingId = FindId(type, name);
    return existingId.IsValid() ?  existingId : InsertFont(type, name);
}

/**
 *
 */
uint32_t const* DbFont::Utf8ToUcs4(bvector<Byte>& ucs4CharsBuffer, size_t& numUcs4Chars, Utf8StringCR str) {
    if (SUCCESS != BeStringUtilities::TranscodeStringDirect(ucs4CharsBuffer, "UCS-4", (Byte const*)str.c_str(), sizeof(Utf8Char) * (str.size() + 1), "UTF-8")) {
        ucs4CharsBuffer.clear();
        numUcs4Chars = 0;
        return nullptr;
    }

    // ICU for UCS-4 injects a BOM on the front which we don't want.
    BeAssert(0xfeff == *(uint32_t const*)&ucs4CharsBuffer[0]);
    uint32_t const* ucs4Chars = (uint32_t const*)&ucs4CharsBuffer[sizeof(uint32_t)];
    size_t maxNumUcs4Chars = ((ucs4CharsBuffer.size() - 1) / 4);
    numUcs4Chars = 0;
    while ((0 != ucs4Chars[numUcs4Chars]) && (numUcs4Chars < maxNumUcs4Chars))
        ++numUcs4Chars;

    return ucs4Chars;
}

/**
 *
 */
void DbFont::ScaleAndOffsetGlyphRange(DRange2dR range, DPoint2dCR scale, DPoint2d offset) {
    range.low.x = (range.low.x * scale.x) + offset.x;
    range.low.y = (range.low.y * scale.y) + offset.y;
    range.high.x = (range.high.x * scale.x) + offset.x;
    range.high.y = (range.high.y * scale.y) + offset.y;
}

/**
 *
 */
void GlyphLayoutResult::ZeroNullRanges() {
    if (m_range.IsNull())
        m_range.InitFrom(0.0, 0.0);

    if (m_exactRange.IsNull())
        m_exactRange.InitFrom(0.0, 0.0);

    if (m_justificationRange.IsNull())
        m_justificationRange.InitFrom(0.0, 0.0);
}

/** remap a fontId from source to dest. If source font is embedded, it will be embedded in dest */
FontId DgnImportContext::_RemapFont(FontId srcId) {
    FontId dstId = m_remap.Find(srcId); // Already remapped once? Use cached result.
    if (dstId.IsValid())
        return dstId;

    auto& srcFonts = m_sourceDb.Fonts();
    auto& destFonts = m_destDb.Fonts();
    auto srcFontInfo = srcFonts.GetFontInfo(srcId);
    if (nullptr == srcFontInfo)
        return FontId(); // srcId is not valid

    auto srcType = srcFontInfo->m_type;
    auto srcName = srcFontInfo->m_fontName.c_str();

    // if the source font is embedded, and is not embedded in destination iModel, embed it now.
    auto srcFont = srcFonts.m_fontDb.FindFont(srcType, srcName);
    if (srcFont && (nullptr == destFonts.m_fontDb.FindFont(srcType, srcName))) {
        // TODO: this isn't really right since it only copies over the regular face...
        auto reader = srcFont->GetReader().m_reader;
        ByteStream fontData;
        fontData.resize(reader->GetNumBytes());
        reader->ReadBytes(fontData.GetDataP(), reader->GetNumBytes());
        destFonts.m_fontDb.EmbedFont(reader->m_faces, fontData, reader->m_compressed);
    }

    dstId = destFonts.GetId(srcType, srcName);
    return m_remap.Add(srcId, dstId);
}
