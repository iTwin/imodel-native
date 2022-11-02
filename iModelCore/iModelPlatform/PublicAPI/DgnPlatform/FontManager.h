/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "DgnPlatform.h"

#define FACE_NAME_Regular "regular"
#define FACE_NAME_Bold "bold"
#define FACE_NAME_Italic "italic"
#define FACE_NAME_BoldItalic "bolditalic"

DGNPLATFORM_REF_COUNTED_PTR(FontDbReader)

BEGIN_BENTLEY_DGN_NAMESPACE

enum class FontType {TrueType=1, Rsc=2, Shx=3,};
enum class FaceStyle {Regular, Bold, Italic, BoldItalic};

/**
 * Encoding of a font, for non-Unicode fonts. This includes the glyphIds for the special characters `Degree`, `PlusMinus`, and `Diameter`.
 * @note this is not applicable for TrueTypeFonts - they are always Unicode. All RscFonts and some old ShxFonts are non-Unicode.
 */
struct FontEncoding {
    LangCodePage m_codePage = LangCodePage::Unicode;
    uint32_t m_degreeCode = 176;
    uint32_t m_plusMinusCode = 177;
    uint32_t m_diameterCode = 8709;
    DGNPLATFORM_EXPORT void FromJSON(BeJsConst, FontType);
    DGNPLATFORM_EXPORT void ToJSON(BeJsValue) const;
};

/** A single face of a font stored in a row in the EmbeddedFont data */
struct FontFace {
    BE_JSON_NAME(codePage)
    BE_JSON_NAME(degree)
    BE_JSON_NAME(diameter)
    BE_JSON_NAME(encoding)
    BE_JSON_NAME(faceName)
    BE_JSON_NAME(familyName)
    BE_JSON_NAME(plusMinus)
    BE_JSON_NAME(subId)
    BE_JSON_NAME(type)
    BE_JSON_NAME(fallback)

    FontType m_type;
    Utf8String m_familyName; // the name used to load a DbFont
    FaceStyle m_faceStyle;
    uint32_t m_subId; // only non-zero when a single FontFile holds more than one face.
    FontEncoding m_encoding;
    bool m_isFallback = false;

    static FaceStyle StyleFromString(Utf8StringCR);
    static Utf8String StyleToString(FaceStyle);
    DGNPLATFORM_EXPORT FontFace(BeJsConst val);
    FontFace(FontType type, Utf8CP familyName, FaceStyle faceStyle = FaceStyle::Regular, FontEncoding const* encoding = nullptr, uint32_t subId = 0) :
        m_type(type), m_familyName(familyName), m_faceStyle(faceStyle), m_subId(subId) {
        if (encoding)
            m_encoding = *encoding;
    }
};

/**
 * A row in the be_prop table with the name "EmbeddedFaceData". Used to read a FontFile.
 * @note a single FaceReader may be used for multiple faces and even multiple fonts, since that's the way TrueType collection files work.
 * @note for RSC and SHX, fonts and faces are 1:1 (every font has a single face and a single FontFile.) For TrueType, a font may have
 * multiple faces, each stored in a separate FontFile. Or, a single FontFile may hold multiple faces, and even faces for more than one
 * font - that's why all this is so complicated.
*/
struct FontDbReader : RefCountedBase {
private:
    StreamBuffer m_data;
    BeSQLite::BlobIO m_blobIO;
    uint32_t m_blobPos = 0;

public:
    BeSQLite::DbR m_db;
    uint32_t m_rowId; // the rowId in the `be_Props` table, for blobIo
    uint32_t m_id; // the Id for the `dgn_Font` property in the `be_Props` table for property query
    uint32_t m_nBytes; // uncompressed number of bytes in the font
    bvector<FontFace> m_faces;
    bool m_compressed;
    bool m_loaded = false;

    FontDbReader(BeSQLite::DbR db, uint32_t rowId, uint32_t id, uint32_t nBytes, bool compressed) : m_db(db), m_rowId(rowId), m_id(id), m_nBytes(nBytes), m_compressed(compressed) {}
    ~FontDbReader() { Unload(); }
    uint32_t GetNumBytes() const { return m_nBytes; }
    void SetPos(uint32_t pos);
    bool Skip(uint32_t numBytes);
    uint32_t GetPos() const;
    bool ReadBytes(void* buf, uint32_t size);
    uint8_t* FillAndGetDataP();
    void Load();
    void Unload();
};

/** A pointer to a FontFace and its FontFileReader */
struct ReaderForFace {
    FontFaceP m_face = nullptr;
    FontDbReaderP m_reader = nullptr;
    void Init(FontFaceR face, FontDbReaderR reader) {
        m_face = &face;
        m_reader = &reader;
    }
};

/** Points to a SQLiteDb holding fonts. May be either a WorkspaceDb or a DgnDb */
struct FontDb {
    /** Holds a map of font familyName->DbFontP, case insensitive */
    struct FontsByName {
    private:
        bmap<Utf8String, DbFontP> m_map;
    public:
        DbFontP Find(Utf8CP name) const {
            auto font = m_map.find(Utf8String::FromLower(name));
            return font != m_map.end() ? font->second : nullptr;
        };
        DGNPLATFORM_EXPORT void Empty();
        void Insert(Utf8CP name, DbFontP font) {
            m_map.Insert(Utf8String::FromLower(name), font);
        }
        DbFontP GetFirst() {
            return m_map.size() > 0 ? m_map.begin()->second : nullptr;
        }
        ~FontsByName() { Empty(); }
    };

private:
    void AddDbReader(FontDbReaderPtr) const;
    mutable bvector<FontDbReaderPtr> m_fileReaders;
    mutable bool m_loaded = false;
    mutable FontsByName m_trueType;
    mutable FontsByName m_rsc;
    mutable FontsByName m_shx;

public:
    BeSQLite::DbR m_db;
    mutable bool m_isWorkspace;
    FontDb(BeSQLite::DbR db, bool isWorkspace) : m_db(db), m_isWorkspace(isWorkspace) {}
    void Load() const;
    FontsByName& GetMap(FontType fontType) const {
        switch (fontType) {
        case FontType::Rsc:
            return m_rsc;
        case FontType::Shx:
            return m_shx;
        }
        return m_trueType;
    }
    DGNPLATFORM_EXPORT BentleyStatus EmbedFont(bvector<FontFace> const& faces, ByteStreamCR data, bool compress);
    DGNPLATFORM_EXPORT BentleyStatus EmbedFontFile(Utf8CP fileName, bool compress);
    DGNPLATFORM_EXPORT DbFontP FindFont(FontType fontType, Utf8CP fontName) const { Load(); return GetMap(fontType).Find(fontName); }
};

/**  Manages access to font WorkspaceDbs */
struct FontManager {
    DGNPLATFORM_EXPORT static BeMutex& GetMutex();

    /**
     * Simplifies common pattern in racy font-related code wherein an object holds a boolean flag indicating
     * whether or not a member is initialized; if it is not initialized, it does thread-unsafe work and sets the flag.
     * In particular, addresses bugs wherein flag is set *before* initialization completes.
     */
    struct FlagHolder {
    private:
        mutable BeMutexHolder m_lock;
        bool& m_flag;
        bool m_initialValue;

    public:
        FlagHolder(FlagHolder const&) = delete;
        FlagHolder& operator=(FlagHolder const&) = delete;

        explicit FlagHolder(bool& flag) : m_lock(FontManager::GetMutex(), BeMutexHolder::Lock::No), m_flag(flag), m_initialValue(flag) {
            if (!m_initialValue) { // Avoid acquiring mutex in common already-initialized case...
                m_lock.lock(); // Double-checked locking...
                m_initialValue = m_flag;
                if (m_initialValue)
                    m_lock.unlock();
            }
        }

        ~FlagHolder() {
            if (!m_initialValue) {
                // Set the flag after all work completed
                BeAssert(m_lock.owns_lock());
                BeAssert(!m_flag);
                m_flag = true;
            }
        }

        operator bool() const {
            BeAssert(m_lock.owns_lock() || m_initialValue);
            return m_initialValue;
        }
    };
    static void Initialize();
    static void Shutdown();
    static DbFontP GetWorkspaceFont(FontType fontType, Utf8CP fontName);
    DGNPLATFORM_EXPORT static bool AddWorkspaceDb(Utf8CP filename, BeSQLite::CloudContainerP);
    DGNPLATFORM_EXPORT static DbFontR FindFont(FontType fontType, Utf8CP fontName);
    DGNPLATFORM_EXPORT static DbFontR GetFallbackFont(FontType fontType = FontType::TrueType);
    DGNPLATFORM_EXPORT static FaceStyle FaceStyleFromBoldItalic(bool isBold, bool isItalic);
    DGNPLATFORM_EXPORT static void FaceStyleToBoldItalic(bool& isBold, bool& isItalic, FaceStyle);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
