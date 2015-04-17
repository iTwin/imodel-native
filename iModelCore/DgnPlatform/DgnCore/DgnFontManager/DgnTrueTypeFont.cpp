/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnFontManager/DgnTrueTypeFont.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)

    #define UNICODE
    #include <Windows.h>
    #include <objbase.h>
    #include <mlang.h>
    #include <Shlobj.h>
    #include <usp10.h>

#else

    // Avoid modifying third-party library code.
    #if defined (__clang__) && defined (__apple_build_version__) && (__apple_build_version__ >= 5030038)
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wdeprecated-register"
    #endif

    #include <freetype2/ft2build.h>
    #include FT_FREETYPE_H
    #include FT_TRUETYPE_TABLES_H
    #include FT_OUTLINE_H
    typedef char* LPSTR;
    typedef unsigned short WORD;

    #if defined (__clang__) && defined (__apple_build_version__) && (__apple_build_version__ >= 5030038)
        #pragma clang diagnostic pop
    #endif
#endif

#include <DgnPlatformInternal.h>
#include <DgnPlatformInternal/DgnCore/PlatformTextServices.h>

USING_NAMESPACE_BENTLEY_SQLITE

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

#if defined (BENTLEY_WIN32)
static const int FONT_LOGICAL_UNITS = 2048;
#endif

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct Win32TTThreadGlobals : public DgnHost::IHostObject
    {
    private:    mutable HDC                 m_dc;
    public:     T_WStringVector             m_localFontFilePaths;
    public:     DgnTrueTypeFontCP           m_selectedFont;
    public:     bool                        m_isSelectedFontBold;
    public:     bool                        m_isSelectedFontItalic;
    private:    mutable bool                m_triedToLoadFontLinking;
    private:    mutable IMLangFontLink2*    m_fontLinking;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: HDC GetDC () const
        {
        if (NULL == m_dc)
            m_dc = ::CreateDCW (L"DISPLAY", NULL, NULL, NULL);

        return m_dc;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                               Grigas.Petraitis    03/2015
    //---------------------------------------------------------------------------------------
    protected: virtual void _OnHostTermination(bool isProgramExit)
        {
        if (nullptr != m_dc)
            {
            ::DeleteDC(m_dc);
            m_dc = nullptr;
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: IMLangFontLink2* GetFontLinking () const
        {
        if (!m_triedToLoadFontLinking)
            {
            m_triedToLoadFontLinking = true;

            IMultiLanguage* mLang;
            if (FAILED (::CoCreateInstance (CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, __uuidof (IUnknown), (void**)&mLang)))
                return NULL;

            IMLangFontLink2* fontLinking;
            if (FAILED (mLang->QueryInterface (__uuidof (IMLangFontLink2), (void**)&fontLinking)))
                return NULL;
            
            m_fontLinking = fontLinking;
            }

        return m_fontLinking;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: Win32TTThreadGlobals () :
        m_dc                        (NULL),
        m_selectedFont              (NULL),
        m_isSelectedFontBold        (false),
        m_isSelectedFontItalic      (false),
        m_triedToLoadFontLinking    (false),
        m_fontLinking               (NULL)
        {
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: static Win32TTThreadGlobals& Instance ()
        {
        static DgnHost::Key s_ttThreadGlobals;
    
        Win32TTThreadGlobals* globals = reinterpret_cast<Win32TTThreadGlobals*>(T_HOST.GetHostObject (s_ttThreadGlobals));
        if (NULL == globals)
            {
            globals = new Win32TTThreadGlobals ();
            T_HOST.SetHostObject (s_ttThreadGlobals, globals);
            }

        DWORD w = 0;
        w;
    
        return *globals;
        }

    }; // Win32TTThreadGlobals

#else

// WinGDI.h
typedef struct _FIXED
    {
    unsigned short  fract;
    short           value;
    
    } FIXED;

#define TT_POLYGON_TYPE     24

#define TT_PRIM_LINE        1
#define TT_PRIM_QSPLINE     2
#define TT_PRIM_CSPLINE     3

typedef struct tagPOINTFX
    {
    FIXED   x;
    FIXED   y;
    
    } POINTFX, *LPPOINTFX;

typedef struct tagTTPOLYCURVE
    {
    unsigned short  wType;
    unsigned short  cpfx;
    POINTFX         apfx[1];
    
    } TTPOLYCURVE, *LPTTPOLYCURVE;

typedef struct tagTTPOLYGONHEADER
    {
    unsigned long   cb;
    unsigned long   dwType;
    POINTFX         pfxStart;
    
    } TTPOLYGONHEADER, *LPTTPOLYGONHEADER;

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct FreeTypeThreadGlobals : public NonCopyableClass
    {
    private:    mutable bool        m_triedToLoadFTLibrary;
    private:    mutable FT_Library  m_ftLibrary;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: FT_Library GetFTLibrary () const
        {
        if (!m_triedToLoadFTLibrary)
            {
            m_triedToLoadFTLibrary = true;

            FT_Error ftStatus = FT_Init_FreeType (&m_ftLibrary);
            if ((NULL == m_ftLibrary) || (FT_Err_Ok != ftStatus))
                { BeAssert (false && L"Could not initialize the freetype2 library."); }
            }

        return m_ftLibrary;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: FreeTypeThreadGlobals () :
        m_ftLibrary (NULL),
        m_triedToLoadFTLibrary(false)
        {
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: static FreeTypeThreadGlobals& Instance ()
        {
        static DgnHost::Key s_ttThreadGlobals;
    
        FreeTypeThreadGlobals* globals = reinterpret_cast<FreeTypeThreadGlobals*>(T_HOST.GetHostVariable (s_ttThreadGlobals));
        if (NULL == globals)
            {
            globals = new FreeTypeThreadGlobals ();
            T_HOST.SetHostVariable (s_ttThreadGlobals, globals);
            }
    
        return *globals;
        }

    }; // FreeTypeThreadGlobals

#endif

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct FontMetrics
    {
    public: double  m_scaleFactor;
    public: double  m_descenderRatio;

    }; // FontMetrics

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct GlyphMetrics
    {
    public: short   m_cellBoxWidth;
    public: long    m_blackBoxLeft;
    public: long    m_blackBoxTop;
    public: uint32_t m_blackBoxWidth;
    public: uint32_t m_blackBoxHeight;

    }; // GlyphMetrics

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct ITTPImpl
    {
    public: virtual ~ITTPImpl () { }
    
    public: virtual BentleyStatus _Activate (DgnTrueTypeFontCR, bool isBold, bool isItalic) = 0;
    public: virtual bool _CanEmbed (DgnTrueTypeFontCR, bool isBold, bool isItalic) = 0;
    public: virtual BentleyStatus _ComputeAdvanceWidths(DgnTrueTypeFontCR, WCharCP, size_t totalNumChars, T_DoubleVectorR outAdvanceWidths, bvector<DPoint2d>& outGlyphOffsets, bvector<bool>& glyphIndexMask, DgnGlyphLayoutResult::T_GlyphCodesR outGlyphCodes, DgnGlyphLayoutContextCR, bool shouldDisableGlyphShaping, bool shouldAllowMissingGlyphs) = 0;
    public: virtual bool _ContainsCharacter (WChar, DgnTrueTypeFontCR, bool isBold, bool isItalic) = 0;
    public: virtual BentleyStatus _GetFontMetrics (FontMetrics&, DgnTrueTypeFontCR, bool isBold, bool isItalic) = 0;
    public: virtual BentleyStatus _GetGlyphData (bvector<Byte>&, FontChar glyphID, bool isGlyphIndex, DgnTrueTypeFontCR, bool isBold, bool isItalic) = 0;
    public: virtual BentleyStatus _GetGlyphMetrics (GlyphMetrics&, FontChar glyphID, bool isGlyphIndex, DgnTrueTypeFontCR, bool isBold, bool isItalic) = 0;
    public: virtual BentleyStatus _LayoutGlyphs (DgnTrueTypeFontCR, DgnGlyphLayoutContextCR, WCharCP processedChars, size_t processedCharsCount, DgnGlyphLayoutResultR) = 0;
    public: virtual BentleyStatus _RegisterFileFont (WCharCP filePath) = 0;
    public: virtual BentleyStatus _RegisterMemoryFont (Byte* data, size_t dataSize, bool& callerMustRetain) = 0;
    
    }; // ITTPImpl

END_BENTLEY_DGNPLATFORM_NAMESPACE

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static DgnFontConfigurationData getEffectiveFontConfig (DgnFontKey const& fontKey)
    {
    DgnFontConfigurationData const* customConfig = DgnFontManager::FindCustomFontConfiguration (fontKey);
    if (NULL != customConfig)
        return *customConfig;
    
    DgnFontConfigurationData defaultConfig;
    
    LangCodePage acp;
    BeStringUtilities::GetCurrentCodePage (acp);
                
    defaultConfig.m_codePage                = LangCodePage::Unicode;
    defaultConfig.m_degreeCharCode          = SPECIALCHAR_UnicodeDegree;
    defaultConfig.m_diameterCharCode        = SPECIALCHAR_UnicodeDiameter;
    defaultConfig.m_plusMinusCharCode       = SPECIALCHAR_UnicodePlusMinus;
    defaultConfig.m_shouldCreateShxUnifont  = true;

    return defaultConfig;
    }

// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct Win32TTPImpl : public ITTPImpl
    {
    private: HANDLE m_handle;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: Win32TTPImpl () :
        m_handle (NULL)
        {
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual ~Win32TTPImpl ()
        {
        if (NULL == m_handle)
            return;
        
        if (0 == ::RemoveFontMemResourceEx (m_handle))
            { BeAssert (false); }
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    private: static BentleyStatus ActivateInternal (DgnTrueTypeFontCR font, bool isBold, bool isItalic)
        {
        Win32TTThreadGlobals& ttGlobals = Win32TTThreadGlobals::Instance ();
        if ((&font == ttGlobals.m_selectedFont) && (isBold == ttGlobals.m_isSelectedFontBold) && (isItalic == ttGlobals.m_isSelectedFontItalic))
            return SUCCESS;

        HFONT newFont = ::CreateFontW (FONT_LOGICAL_UNITS, 0, 0, 0, isBold ? FW_BOLD : FW_LIGHT, isItalic, false, false, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, WString (font.GetName ().c_str (), BentleyCharEncoding::Utf8).c_str ());
        if (NULL == newFont)
            return ERROR;

        ttGlobals.m_selectedFont            = &font;
        ttGlobals.m_isSelectedFontBold      = isBold;
        ttGlobals.m_isSelectedFontItalic    = isItalic;
    
        HFONT previousFont = (HFONT)::SelectObject (ttGlobals.GetDC (), newFont);
        if (NULL != previousFont)
            ::DeleteObject (previousFont);

        return SUCCESS;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _Activate (DgnTrueTypeFontCR font, bool isBold, bool isItalic) override
        {
        return Win32TTPImpl::ActivateInternal(font, isBold, isItalic);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual bool _CanEmbed (DgnTrueTypeFontCR font, bool isBold, bool isItalic) override
        {
        OUTLINETEXTMETRIC otm;
        memset (&otm, 0, sizeof (otm));

        if (SUCCESS != Win32TTPImpl::ActivateInternal (font, isBold, isItalic))
            return false;

        if (!::GetOutlineTextMetrics (Win32TTThreadGlobals::Instance ().GetDC (), sizeof (otm), &otm))
            return false;
        
        return (0 == (0x1 & otm.otmfsType));
        }
    
    //---------------------------------------------------------------------------------------
    // Official Msdn docs: http://msdn.microsoft.com/en-us/library/dd374091(VS.85).aspx
    // Additional documentation and examples from a Google Chrome developer (Brett Wilson): http://maxradi.us/documents/uniscribe/.
    // @bsimethod                                                   Jeff.Marker     05/2010
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _ComputeAdvanceWidths
    (
    DgnTrueTypeFontCR                   font,
    WCharCP                             chars,
    size_t                              totalNumChars,
    T_DoubleVectorR                     outAdvanceWidths,
    bvector<DPoint2d>&                  outGlyphOffsets,
    bvector<bool>&                      glyphIndexMask,
    DgnGlyphLayoutResult::T_GlyphCodesR outGlyphCodes, 
    DgnGlyphLayoutContextCR             context,
    bool                                shouldDisableGlyphShaping,
    bool                                shouldAllowMissingGlyphs
    ) override
        {
        POSTCONDITION (SUCCESS == Win32TTPImpl::ActivateInternal (font, context.IsBold (), context.ShouldUseItalicTypeface ()), ERROR);
    
        UniscribeServices::T_ScriptItemVector   scriptItems     (UniscribeServices::GetRecommendedScripItemVectorSize ());
        UniscribeServices::T_OpenTypeTagVector  scriptTags      (UniscribeServices::GetRecommendedScripItemVectorSize());
        size_t                                  numScriptItems  = 0;
        WString                                 hintedString    (chars, totalNumChars);

        // Similar to other things in UniscribeServices::ItemizeString, having this drives me nuts because isn't this Uniscribe's job?
        // According to TFS#42271, things like dates and scales should have each section flow from right-to-left. For example, the logical (date) string "10/22/2013" should display as "2013/22/10"; similarly, the scale string "1/5000" should display as "5000/1". Uniscribe does not consider '/' as a strong right-to-left character, and displays the strings as logically entered. If you set scriptState.uBidiLevel=1, '-' is treated as such, but not '/'.
        // I found no series of flags to get the desired behavior, but according to Unicode Annex #9 (Unicode Bidirectional Algorithm, http://www.unicode.org/reports/tr9/), there are many control characters that can be used to suggest character ordering. Implicit Directional Marks are described as follows: These characters are very light-weight formatting. They act exactly like right-to-left or left-to-right characters, except that they do not display or have any other semantic effect. Their use is more convenient than using explicit embeddings or overrides because their scope is much more local.
        // 0x200f is the "RIGHT-TO-LEFT MARK Right-to-left zero-width non-Arabic character". By injecting this in front of slashes, we force the slash to be processed as an RTL character, giving us the visual display we want.
        // I have some resveration about doing this for every slash (vs. a heuristic or rule set), but testing with Arabic analysts has proven acceptable so far.
        if (DgnFontManager::IsUsingAnRtlLocale())
            {
            WChar findSlash[]     = { L'/', 0 };
            WChar replaceSlash[]  = { 0x200f, L'/', 0 };
            hintedString.ReplaceAll(findSlash, replaceSlash);
            }
    
        if (SUCCESS != UniscribeServices::ItemizeString (chars, totalNumChars, scriptItems, scriptTags, numScriptItems))
            return ERROR;
    
        //...............................................................................................................................................
        // Determine the visual ordering of items so that we display the entire run correctly.
        Win32TTThreadGlobals& ttGlobals = Win32TTThreadGlobals::Instance ();

        ScopedArray<BYTE> bidiLevels (numScriptItems);
        for (size_t iScriptItem = 0; iScriptItem < numScriptItems; ++iScriptItem)
            bidiLevels.GetData ()[iScriptItem] = scriptItems[iScriptItem].a.s.uBidiLevel;
    
        ScopedArray<int> visualToLogicalMapping (numScriptItems);
        ScopedArray<int> logicalToVisualMapping (numScriptItems);

        POSTCONDITION (0 == ::ScriptLayout ((int)numScriptItems, bidiLevels.GetData (), visualToLogicalMapping.GetData (), logicalToVisualMapping.GetData ()), ERROR);
    
        bvector<T_DoubleVector> leadingCaretOffsetsPerVisual;
        bvector<T_DoubleVector> trailingCaretOffsetsPerVisual;
        double                  logicalCaretOffsetOffset        = 0.0;

        leadingCaretOffsetsPerVisual.reserve (numScriptItems);
        trailingCaretOffsetsPerVisual.reserve (numScriptItems);

        void* activeUniscribeScriptCache = NULL;
        
        if (ttGlobals.m_isSelectedFontBold && ttGlobals.m_isSelectedFontItalic)
            activeUniscribeScriptCache = font.m_uniscribeScriptCaches[DgnTrueTypeFont::SCRIPT_CACHE_INDEX_BoldItalic];
        else if (ttGlobals.m_isSelectedFontBold)
            activeUniscribeScriptCache = font.m_uniscribeScriptCaches[DgnTrueTypeFont::SCRIPT_CACHE_INDEX_Bold];
        else if (ttGlobals.m_isSelectedFontItalic)
            activeUniscribeScriptCache = font.m_uniscribeScriptCaches[DgnTrueTypeFont::SCRIPT_CACHE_INDEX_Italic];
        else
            activeUniscribeScriptCache = font.m_uniscribeScriptCaches[DgnTrueTypeFont::SCRIPT_CACHE_INDEX_Normal];

        for (size_t iVisualItem = 0; iVisualItem < numScriptItems; ++iVisualItem)
            {
            //...............................................................................................................................................
            // For-each item, first determine its glyph indices (and other Uniscribe state data).

            int     iScriptItem                 = visualToLogicalMapping.GetData ()[iVisualItem];
            int     numGlyphCodesInScriptItem   = 0;
            HRESULT hr;
        
            // ::ScriptItemize will always put a trailing fake SCRIPT_ITEM into the array, pointing one past the last real item, allowing you to always look one past numScriptItems to compute number of characters.
            size_t numCharsInScriptItem = (scriptItems[iScriptItem + 1].iCharPos - scriptItems[iScriptItem].iCharPos);

            // One per-logical character, so buffer size is known/fixed at this point.
            ScopedArray<WORD>               scriptItemLogicalClusters   (numCharsInScriptItem);
            ScopedArray<SCRIPT_CHARPROP>    scriptItemCharProps         (numCharsInScriptItem);

            // Docs recommend an initial size of (1.5 * number of logical characters), although more may be required.
            bvector<WORD>               scriptItemGlyphCodes    ((size_t)(numCharsInScriptItem * 1.5));
            bvector<SCRIPT_GLYPHPROP>   scriptItemGlyphProps    ((size_t)(numCharsInScriptItem * 1.5));
        
            // Shaping can prevent legacy glyph/font substitution (see TFS#1133). For example, even though SimSun does not contain Korean glyphs, if you disable shaping, they will appear by magic (but if you enable shaping, you get the correct answer that they aren't in the font).
            if (shouldDisableGlyphShaping)
                scriptItems[iScriptItem].a.fNoGlyphIndex = true;

            while (S_OK != (hr = ::ScriptShapeOpenType(ttGlobals.GetDC(),
                                                        &activeUniscribeScriptCache,
                                                        &scriptItems[iScriptItem].a,
                                                        scriptTags[iScriptItem],
                                                        SCRIPT_TAG_UNKNOWN,
                                                        NULL,
                                                        NULL,
                                                        0,
                                                        chars + scriptItems[iScriptItem].iCharPos,
                                                        (int)numCharsInScriptItem,
                                                        (int)scriptItemGlyphCodes.size (),
                                                        scriptItemLogicalClusters.GetData (),
                                                        scriptItemCharProps.GetData (),
                                                        &scriptItemGlyphCodes[0],
                                                        &scriptItemGlyphProps[0],
                                                        &numGlyphCodesInScriptItem)))
                {
                if (USP_E_SCRIPT_NOT_IN_FONT == hr)
                    {
                    // This assert is commented out, partially because it gets hit often enough, and partially because it gets called within WPF render code (e.g. symbol selector), which will crash if you try to open a dialog (e.g. the assert dialog).
                    // If the user enables font linking, font substitution will occur at the front of _LayoutGlyphs, which will split the string up into single-font chunks (where said font should be able to support each chunk) before getting into this area of code.
                    // However, Uniscribe vs. Font Linking are different technologies, and there may be some gaps between the coverage.
                    // BeDataAssert (false);
                    scriptItems[iScriptItem].a.eScript = SCRIPT_UNDEFINED;
                    continue;
                    }
            
                POSTCONDITION(E_OUTOFMEMORY == hr, ERROR);

                scriptItemGlyphCodes.resize(2 * scriptItemGlyphCodes.size());
                scriptItemGlyphProps.resize(scriptItemGlyphCodes.size());
                }

            if (!shouldAllowMissingGlyphs)
                {
                SCRIPT_FONTPROPERTIES fontProps;
                memset(&fontProps, 0, sizeof(fontProps));
                fontProps.cBytes = sizeof(fontProps);

                POSTCONDITION (S_OK == (hr = ::ScriptGetFontProperties(ttGlobals.GetDC(), &activeUniscribeScriptCache, &fontProps)), ERROR);

                for (size_t iGlyphCode = 0; iGlyphCode < (size_t)numGlyphCodesInScriptItem; ++iGlyphCode)
                    {
                    if (fontProps.wgDefault == scriptItemGlyphCodes[iGlyphCode])
                        return ERROR;
                    }
                }
        
            outGlyphCodes.insert (outGlyphCodes.end (), scriptItemGlyphCodes.begin (), scriptItemGlyphCodes.begin () + (size_t)numGlyphCodesInScriptItem);
            glyphIndexMask.insert (glyphIndexMask.end (), (size_t)numGlyphCodesInScriptItem, !scriptItems[iScriptItem].a.fNoGlyphIndex);
        
            //...............................................................................................................................................
            // Then, for-each item, determine each glyph's advance width (ScriptPlace).

            // One per-glyph, so buffer size is known/fixed at this point.
            ScopedArray<int>        advanceWidths   ((size_t)numGlyphCodesInScriptItem);
            ScopedArray<GOFFSET>    gOffsets        ((size_t)numGlyphCodesInScriptItem);
            ABC                     runAbcWidth;    memset (&runAbcWidth, 0, sizeof (runAbcWidth));
            
            POSTCONDITION (0 == ::ScriptPlaceOpenType(ttGlobals.GetDC(),
                                                        &activeUniscribeScriptCache,
                                                        &scriptItems[iScriptItem].a,
                                                        scriptTags[iScriptItem],
                                                        SCRIPT_TAG_UNKNOWN,
                                                        NULL,
                                                        NULL,
                                                        0,
                                                        chars + scriptItems[iScriptItem].iCharPos,
                                                        scriptItemLogicalClusters.GetData (),
                                                        scriptItemCharProps.GetData (),
                                                        (int)numCharsInScriptItem,
                                                        &scriptItemGlyphCodes[0],
                                                        &scriptItemGlyphProps[0],
                                                        numGlyphCodesInScriptItem,
                                                        advanceWidths.GetData (),
                                                        gOffsets.GetData (),
                                                        &runAbcWidth),
                                                    ERROR);

            outAdvanceWidths.reserve (outAdvanceWidths.size () + (size_t)numGlyphCodesInScriptItem);
            
            for (size_t iAdvanceWidth = 0; iAdvanceWidth < (size_t)numGlyphCodesInScriptItem; ++iAdvanceWidth)
                {
                outAdvanceWidths.push_back (advanceWidths.GetData ()[iAdvanceWidth] * font.GetScaleFactor ());
            
                DPoint2d glyphOffset = { (gOffsets.GetData ()[iAdvanceWidth].du * font.GetScaleFactor ()), (gOffsets.GetData ()[iAdvanceWidth].dv * font.GetScaleFactor ()) };
                outGlyphOffsets.push_back (glyphOffset);
                }
        
            //...............................................................................................................................................
            // Then, for-each item, determine each logical character's caret offset.
        
            leadingCaretOffsetsPerVisual.push_back (T_DoubleVector ());
            T_DoubleVectorR leadingCaretOffsets = leadingCaretOffsetsPerVisual.back ();

            trailingCaretOffsetsPerVisual.push_back (T_DoubleVector ());
            T_DoubleVectorR trailingCaretOffsets = trailingCaretOffsetsPerVisual.back ();
        
            int offset = 0;

            // Compute leading offsets first as the primary (why not?),
            //  then make the assumption (to reduce computations) that {trailingCaretOffsets[0..n]} = {leadingCaretOffsets[1..n], computed last offset}.

            double runningCharacterSpacingOffset = 0.0;

            // There is no OpenType version of ScriptCPtoX; need to extract the SCRIPT_VISATTR out of SCRIPT_GLYPHPROP into a contiguous array for it.
            ScopedArray<SCRIPT_VISATTR> scriptItemVisualAttributes(scriptItemGlyphProps.size());
            for (size_t iGlyphProp = 0; iGlyphProp < scriptItemGlyphProps.size(); ++iGlyphProp)
                scriptItemVisualAttributes.GetData()[iGlyphProp] = scriptItemGlyphProps[iGlyphProp].sva;

            // When glyph shaping is disabled, giving Arabic text to ScriptCPtoX can cause an access violation (see TFS#78630).
            // While I also considered detecting Arabic text in the string and forcing shaping on, I figured this is a user option, so don't introduce the overhead for something they can change.
            // In theory, GetTextExtentExPointW should be faster than ScriptCPtoX, which is also in the spirit of the configuration option, arguing for this approach.
            bvector<int> legacyGlyphOffsets;
            if (shouldDisableGlyphShaping)
                {
                legacyGlyphOffsets.resize(numCharsInScriptItem + 1);
                SIZE totalSize;

                // Note that legacyGlyphOffsets correlates to leading offset; assume the first is always 0, and let GetTextExtentExPointW fill in the rest.
                legacyGlyphOffsets[0] = 0;
                POSTCONDITION(0 != ::GetTextExtentExPointW(ttGlobals.GetDC(), chars + scriptItems[iScriptItem].iCharPos, (int)numCharsInScriptItem, INT_MAX, NULL, &legacyGlyphOffsets[0], &totalSize), ERROR);
                }

            for (size_t iLogicalChar = 0; iLogicalChar <= numCharsInScriptItem; ++iLogicalChar)
                {
                bool            isTrailingPosition  = (iLogicalChar == numCharsInScriptItem);
                T_DoubleVectorR offsetsVector       = (isTrailingPosition ? trailingCaretOffsets : leadingCaretOffsets);
            
                if (isTrailingPosition)
                    trailingCaretOffsets.insert (trailingCaretOffsets.end (), leadingCaretOffsets.begin () + 1, leadingCaretOffsets.end ());
            
                if (shouldDisableGlyphShaping)
                    {
                    // Note that we put the total size of the string on the end of the vector, allowing this index.
                    offset = legacyGlyphOffsets[iLogicalChar];
                    }
                else
                    {
                    POSTCONDITION(0 == ::ScriptCPtoX((isTrailingPosition ? (int)(iLogicalChar - 1) : (int)iLogicalChar),
                                                        isTrailingPosition,
                                                        (int)numCharsInScriptItem,
                                                        numGlyphCodesInScriptItem,
                                                        scriptItemLogicalClusters.GetData(),
                                                        scriptItemVisualAttributes.GetData(),
                                                        advanceWidths.GetData(),
                                                        &scriptItems[iScriptItem].a,
                                                        &offset),
                                                    ERROR);
                    }
                
                offsetsVector.push_back(logicalCaretOffsetOffset + (offset * font.GetScaleFactor()) + runningCharacterSpacingOffset);
                }
        
            // ScriptCPtoX only works within script items; track an offset since we need to promote values to a CharStream (which can contain multiple script items).
            //  Note that it's important to sum runAbcWidth as an int, because the computed value relies on arithmatic overflow
            //  (e.g. for an RLM character, the B and C widths are non-zero, but their sum is 0, which is the desired result).
            logicalCaretOffsetOffset += (((int)(runAbcWidth.abcA + runAbcWidth.abcB + runAbcWidth.abcC) * font.GetScaleFactor ()) + runningCharacterSpacingOffset);
            }
    
        return SUCCESS;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual bool _ContainsCharacter (WChar uniChar, DgnTrueTypeFontCR font, bool isBold, bool isItalic) override
        {
        if (SUCCESS != Win32TTPImpl::ActivateInternal (font, isBold, isItalic))
            return false;

        WChar   testChar[2] = { uniChar, 0 };
        WORD    glyph[2];
        
        if (GDI_ERROR == ::GetGlyphIndicesW (Win32TTThreadGlobals::Instance ().GetDC (), testChar, 1, glyph, GGI_MARK_NONEXISTING_GLYPHS))
            return false;

        return (0xffff != glyph[0]);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     07/2012
    //---------------------------------------------------------------------------------------
    public: static DgnFontPtr CreateLastResortFont ()
        {
        // Assume windows will always have Arial... don't try to get fancy by loading custom fonts.
        static CharCP LAST_RESORT_FONT_NAME = "Arial";

        DgnTrueTypeFontP font = new DgnTrueTypeFont (LAST_RESORT_FONT_NAME, getEffectiveFontConfig (DgnFontKey (DgnFontType::TrueType, Utf8String (LAST_RESORT_FONT_NAME))));
        font->m_isLastResort = true;

        return font;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     07/2012
    //---------------------------------------------------------------------------------------
    public: static void EnumerateSystemFonts (T_FontCatalogMap& systemFonts)
        {
        WString                 ttFontSearchPaths   = T_HOST.GetFontAdmin ()._GetTrueTypeFontPaths ();
        T_WStringVector         localFontFilePaths;
        Win32TTThreadGlobals&   globals             = Win32TTThreadGlobals::Instance ();
    
        if (!ttFontSearchPaths.empty ())
            Win32TTPImpl::LoadLocalFonts (ttFontSearchPaths, globals.m_localFontFilePaths);
    
        LOGFONT lf;
        memset (&lf, 0, sizeof (lf));
        lf.lfCharSet = DEFAULT_CHARSET;

        ::EnumFontFamiliesEx (globals.GetDC (), &lf, Win32TTPImpl::EnumerateSystemFontsCallback, reinterpret_cast<LPARAM>(&systemFonts), 0);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     07/2012
    //---------------------------------------------------------------------------------------
    private: static int CALLBACK EnumerateSystemFontsCallback (LOGFONTW CONST* lpelfe, TEXTMETRICW CONST*, DWORD, LPARAM lParam)
        {
        T_FontCatalogMap&   callbackData    = *reinterpret_cast<T_FontCatalogMap*>(lParam);
        WString             faceNameW       = lpelfe->lfFaceName;
    
        // If the font name is ?????? or @????? then it is not useable
        size_t iChar = 0;
        for (; 0 != faceNameW[iChar]; ++iChar)
            {
            if ((L'?' != faceNameW[iChar]) && (L'@' != faceNameW[iChar]))
                break;
            }

        if (0 == faceNameW[iChar])
            return 1;

        Utf8String  faceName    (faceNameW.c_str ());
        DgnFontKey  fontKey     (DgnFontType::TrueType, faceName);
        callbackData[fontKey] = new DgnTrueTypeFont (faceName.c_str (), getEffectiveFontConfig (fontKey));

        return 1;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _GetFontMetrics (FontMetrics& metrics, DgnTrueTypeFontCR font, bool isBold, bool isItalic) override
        {
        OUTLINETEXTMETRIC otm;
        memset (&otm, 0, sizeof (otm));

        if (SUCCESS != Win32TTPImpl::ActivateInternal (font, isBold, isItalic))
            return ERROR;

        if (!::GetOutlineTextMetrics (Win32TTThreadGlobals::Instance ().GetDC (), sizeof (otm), &otm))
            return ERROR;

        // For DWG compatibility, if possible, use the height of the letter 'A' as the ascent (vs. what the font declares).
        GlyphMetrics aGlyphMetrics;
        if (SUCCESS == _GetGlyphMetrics (aGlyphMetrics, 'A', false, font, false, false))
            otm.otmAscent = aGlyphMetrics.m_blackBoxTop;
    
        metrics.m_scaleFactor       = ((otm.otmTextMetrics.tmHeight / (double)otm.otmAscent) / FONT_LOGICAL_UNITS);
        metrics.m_descenderRatio    = fabs (otm.otmDescent / (double)otm.otmAscent);

        return SUCCESS;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _GetGlyphData (bvector<Byte>& buffer, FontChar glyphID, bool isGlyphIndex, DgnTrueTypeFontCR font, bool isBold, bool isItalic) override
        {
        buffer.clear ();
        
        if (SUCCESS != Win32TTPImpl::ActivateInternal (font, isBold, isItalic))
            return ERROR;
        
        GLYPHMETRICS    metrics;
        MAT2            matrix;     Win32TTPImpl::InitIdentityMAT2 (matrix);
        UINT            format      = (isGlyphIndex ? (GGO_NATIVE | GGO_GLYPH_INDEX) : GGO_NATIVE);
        DWORD           status      = ::GetGlyphOutlineW (Win32TTThreadGlobals::Instance ().GetDC (), glyphID, format, &metrics, 0, NULL, &matrix);
    
        if (GDI_ERROR == status)
            return ERROR;
    
        size_t bufferSize = (size_t)status;
        buffer.resize (bufferSize);

        status = ::GetGlyphOutlineW (Win32TTThreadGlobals::Instance ().GetDC (), glyphID, format, &metrics, (DWORD)bufferSize, &buffer[0], &matrix);
        if (GDI_ERROR == status)
            return ERROR;
        
        return SUCCESS;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _GetGlyphMetrics (GlyphMetrics& metrics, FontChar glyphID, bool isGlyphIndex, DgnTrueTypeFontCR font, bool isBold, bool isItalic) override
        {
        memset (&metrics, 0, sizeof (metrics));
        
        if (SUCCESS != Win32TTPImpl::ActivateInternal (font, isBold, isItalic))
            return ERROR;
        
        GLYPHMETRICS    win32GlyphMetrics;  memset (&win32GlyphMetrics, 0, sizeof (win32GlyphMetrics));
        MAT2            matrix;             Win32TTPImpl::InitIdentityMAT2 (matrix);
        UINT            format              = (isGlyphIndex ? (GGO_METRICS | GGO_GLYPH_INDEX) : GGO_METRICS);
        
        if (GDI_ERROR == ::GetGlyphOutlineW (Win32TTThreadGlobals::Instance ().GetDC (), glyphID, format, &win32GlyphMetrics, 0, NULL, &matrix))
            return ERROR;
        
        metrics.m_cellBoxWidth      = win32GlyphMetrics.gmCellIncX;
        metrics.m_blackBoxLeft      = win32GlyphMetrics.gmptGlyphOrigin.x;
        metrics.m_blackBoxTop       = win32GlyphMetrics.gmptGlyphOrigin.y;
        metrics.m_blackBoxWidth     = win32GlyphMetrics.gmBlackBoxX;
        metrics.m_blackBoxHeight    = win32GlyphMetrics.gmBlackBoxY;

        return SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: static BentleyStatus GetPitchAndCharSet (Byte& pitch, Byte& charSet, DgnTrueTypeFontCR font, bool isBold, bool isItalic)
        {
        pitch   = 0;
        charSet = 0;
        
        if (SUCCESS != Win32TTPImpl::ActivateInternal (font, isBold, isItalic))
            return ERROR;
        
        TEXTMETRIC metrics;
        if (0 == ::GetTextMetricsW (Win32TTThreadGlobals::Instance ().GetDC (), &metrics))
            return ERROR;

        pitch   = metrics.tmPitchAndFamily;
        charSet = metrics.tmCharSet;
        
        return SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    private: static void InitIdentityMAT2 (MAT2& matrix)
        {
        static const FIXED  zero    = { 0, 0 };
        static const FIXED  one     = { 0, 1 };
        
        matrix.eM11 = one;
        matrix.eM12 = zero;
        matrix.eM21 = zero;
        matrix.eM22 = one;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     01/2011
    //---------------------------------------------------------------------------------------
    private: static bool IsScriptItemComplex (SCRIPT_ITEM const& scriptItem)
        {
        // This data should truly be static per-process (really per-operating system version).
        static SCRIPT_PROPERTIES const**   s_scriptProperties       = NULL;
        static int                         s_numScriptProperties    = -1;
 
        if (-1 == s_numScriptProperties)
            {
            HRESULT status = ::ScriptGetProperties (&s_scriptProperties, &s_numScriptProperties);
            if (0 != status)
                {
                BeAssert (false && L"Could not get script properties?");
                s_numScriptProperties = 0;
                }
            }
    
        // Should never get here, but since scripts that actually require shaping are the minority, going with this fallback.
        if (scriptItem.a.eScript >= s_numScriptProperties)
            {
            BeAssert (false && L"SCRIPT_ITEM is using a script identifier that ScriptGetProperties doesn't know about?");
            return false;
            }

        return TO_BOOL (s_scriptProperties[scriptItem.a.eScript]->fComplex);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _LayoutGlyphs (DgnTrueTypeFontCR font, DgnGlyphLayoutContextCR layoutContext, WCharCP processedChars, size_t processedCharsCount, DgnGlyphLayoutResultR layoutResult) override
        {
        // The actual glyph layout code is now in DgnTrueTypeFont::LayoutGlyphsInternal. If text requires automagic font substitution due to missing characters, it must be done at a higher level before calls to computeAdvanceWidths. In the absence of font linking, this largely just passes through to LayoutGlyphsInternal, potentially displaying empty glyphs.
        // Font linking incurs memory/resource overhead; only attempt it if the user requests it (common for CJK, but not so much otherwise).

        if (!DgnFontManager::IsFontLinkingEnabled () || (NULL == Win32TTThreadGlobals::Instance ().GetFontLinking ()))
            return font.LayoutGlyphsInternal (layoutContext, layoutResult, processedChars, processedCharsCount);
    
        // Unlike V8i, font linking must now be done at this higher level, before Uniscribe processing occurs.
        //  If we wait until we call ::ScriptShape, there is no good way to interrupt the logic at that point to split up the run and pick multiple fonts.
        //  Much of this is borrowed from http://blogs.msdn.com/b/oldnewthing/archive/2004/07/16/185261.aspx
        //  See also:
        //      http://blogs.msdn.com/b/michkap/archive/2005/03/20/399322.aspx
        //      http://blogs.msdn.com/b/michkap/archive/2005/05/16/417711.aspx
        //      http://blogs.msdn.com/b/michkap/archive/2005/06/18/430507.aspx
        //      http://blogs.msdn.com/b/michkap/archive/2005/10/01/476022.aspx
        //      http://blogs.msdn.com/b/michkap/archive/2006/01/22/515864.aspx
        //  
        //  The overall workflow is to use GetStrCodePages to break the string up into runs of characters that share at least one code page, MapFont if necessary, and call LayoutGlyphsInternal for each run.

        Win32TTThreadGlobals&   ttGlobals       = Win32TTThreadGlobals::Instance ();
        IMLangFontLink2*        mlangFontLink   = ttGlobals.GetFontLinking ();
        HDC                     displayDC       = ttGlobals.GetDC ();
        DgnTrueTypeFontCP       originalFont    = ttGlobals.m_selectedFont;
        HFONT                   originalHFont   = (HFONT)::GetCurrentObject (displayDC, OBJ_FONT);
        DWORD                   fontCodePages   = 0;
        
        if (FAILED (mlangFontLink->GetFontCodePages (displayDC, originalHFont, &fontCodePages)))
            {
            // Unexpected, but can leave fontCodePages=0 to always force linking below.
            BeAssert (false);
            }

        size_t numCharsLeft = processedCharsCount;

        while (numCharsLeft > 0)
            {
            DWORD   charGroupCodePages;
            long    charGroupCount;
                
            if (FAILED (mlangFontLink->GetStrCodePages (processedChars, (long)numCharsLeft, fontCodePages, &charGroupCodePages, &charGroupCount)))
                {
                BeAssert (false);
                // We couldn't classify the string to determine which font to use... do as much as possible with the original font, even if blank glyphs end up in the output.
                font.LayoutGlyphsInternal (layoutContext, layoutResult, processedChars, numCharsLeft);
                return ERROR;
                }
         
            // The result of GetStrCodePages is every possible code page that can represent the run of characters; thus, linking is only required if a font supports none (e.g. there is no overlap at all).

            bool   isLinkingRequired   = !(charGroupCodePages & fontCodePages);
            HFONT  linkedFont          = 0;

            // Only incur the resource overhead if linking is actually required.
            if (isLinkingRequired)
                {
                if (FAILED (mlangFontLink->MapFont (displayDC, charGroupCodePages, NULL, &linkedFont)))
                    {
                    BeAssert (false);
                    // Do as much as possible with the original font, even if blank glyphs end up in the output.
                    font.LayoutGlyphsInternal (layoutContext, layoutResult, processedChars, numCharsLeft);
                    return ERROR;
                    }
                
                LOGFONTW linkedFontInfo;
                ::GetObjectW (linkedFont, sizeof (linkedFontInfo), &linkedFontInfo);
            
                // It is important to DgnTrueTypeFontManager::SelectFont (vs. a simple ::SelecteObject) because there are still globals in use within DgnTrueTypeFontManager (e.g. Uniscribe script caches, which can technically be shared at a global level).
                DgnTrueTypeFontCP dgnLinkedFont = static_cast<DgnTrueTypeFontCP>(DgnFontManager::FindFont (Utf8String (linkedFontInfo.lfFaceName).c_str (), DgnFontType::TrueType, NULL));
                if ((NULL == dgnLinkedFont) || (SUCCESS != Win32TTPImpl::ActivateInternal (*dgnLinkedFont, ttGlobals.m_isSelectedFontBold, ttGlobals.m_isSelectedFontItalic)))
                    {
                    BeAssert (false);
                    // Do as much as possible with the original font, even if blank glyphs end up in the output.
                    font.LayoutGlyphsInternal (layoutContext, layoutResult, processedChars, numCharsLeft);
                    return ERROR;
                    }
                }
        
            bool didLayoutSucceed = (SUCCESS == font.LayoutGlyphsInternal (layoutContext, layoutResult, processedChars, charGroupCount));
        
            if (isLinkingRequired)
                {
                if (SUCCESS != Win32TTPImpl::ActivateInternal (*originalFont, ttGlobals.m_isSelectedFontBold, ttGlobals.m_isSelectedFontItalic))
                    BeAssert (false);
                
                if (FAILED (mlangFontLink->ReleaseFont (linkedFont)))
                    BeAssert (false);
                }
        
            if (!didLayoutSucceed)
                {
                BeAssert (false);
                return ERROR;
                }

            processedChars  += charGroupCount;
            numCharsLeft    -= charGroupCount;
            }
    
        return SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     07/2012
    //---------------------------------------------------------------------------------------
    public: static void LoadLocalFonts (WString searchPaths, T_WStringVector& localFontFilePaths)
        {
        for (size_t pathSubStrStart = 0, pathSubStrEnd = 0; WString::npos != pathSubStrEnd; pathSubStrStart = pathSubStrEnd)
            {
            pathSubStrStart = searchPaths.find_first_not_of (PATH_SEPARATOR_CHAR, pathSubStrStart);
            pathSubStrEnd   = searchPaths.find (PATH_SEPARATOR_CHAR, pathSubStrStart);

            if (pathSubStrStart == pathSubStrEnd)
                break;

            // BeFileName::IsDirectory does not detect a directory if the name ends in a trailing slash.
            WString currPathW = searchPaths.substr (pathSubStrStart, (pathSubStrEnd - pathSubStrStart));
            if (WCSDIR_SEPARATOR_CHAR == currPathW[currPathW.size () - 1])
                currPathW = currPathW.substr (0, currPathW.size () - 1);

            BeFileName  currPath        (currPathW.c_str ());
            WString     fileName;
            WString     fileExtension;
        
            currPath.ParseName (NULL, NULL, &fileName, &fileExtension);
        
            if (BeFileName::IsDirectory (currPath.GetName ()))
                currPath.BuildName (NULL, currPath.GetName (), L"*", L"ttf");

            BeFileListIterator  iterator (currPath.GetName (), false);
            BeFileName          currFile;
        
            while (SUCCESS == iterator.GetNextFileName (currFile))
                {
                if (BeFileName::IsDirectory (currFile.GetName ()))
                    continue;
            
                if (0 == ::AddFontResourceExW (currFile.GetName (), FR_PRIVATE, 0))
                    { BeDataAssert (false); continue; }
            
                localFontFilePaths.push_back (WString (currFile.GetName ()));
                }
            }
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _RegisterFileFont (WCharCP filePath) override
        {
        BeAssert (false && L"Unimplemented on non-Unix.");
        return ERROR;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _RegisterMemoryFont (Byte* data, size_t dataSize, bool& callerMustRetain) override
        {
        // We don't use this, but the API requires a non-NULL value.
        DWORD unused = 0;
        
        m_handle = ::AddFontMemResourceEx (data, (DWORD)dataSize, 0, &unused);
        
        callerMustRetain = false;

        return ((NULL != m_handle) ? SUCCESS : ERROR);
        }

    }; // Win32TTPImpl

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

// WIP_NONPORT - TrueType
#if !defined (BENTLEY_WIN32)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct FreeTypeTTPImpl : public ITTPImpl
    {
    //=======================================================================================
    // Adapted for DGNPlatform from Mark Schlosser's prototype.
    // @bsiclass                                                    Jeff.Marker     06/2012
    //=======================================================================================
    private: struct FTOutlineParseState
        {
        Byte*               buffer;
        size_t              bufferSize;
        intptr_t            bufferOffset;
        TTPOLYGONHEADER*    currentContour;
        POINTFX             lastContourPoint;

        }; // FTOutlineParseState

    private: FT_Face m_ftFace;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: FreeTypeTTPImpl () :
        m_ftFace (NULL)
        {
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual ~FreeTypeTTPImpl ()
        {
        if (NULL == m_ftFace)
            return;
        
        if (FT_Err_Ok != FT_Done_Face (m_ftFace))
            BeAssert (false);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _Activate (DgnTrueTypeFontCR font, bool isBold, bool isItalic) override
        {
        // FreeType doesn't have a global font object, so no need to "activate".
        return SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual bool _CanEmbed (DgnTrueTypeFontCR font, bool isBold, bool isItalic) override
        {
        if (NULL == m_ftFace)
            return false;
    
        TT_OS2 const* os2Table = static_cast<TT_OS2 const*>(FT_Get_Sfnt_Table (m_ftFace, ft_sfnt_os2));
        if (NULL == os2Table)
            return false;
            
        return (0 == (0x1 & os2Table->fsType));
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _ComputeAdvanceWidths
    (
    DgnTrueTypeFontCR                   font,
    WCharCP                             chars,
    size_t                              totalNumChars,
    T_DoubleVectorR                     outAdvanceWidths,
    bvector<DPoint2d>&                  outGlyphOffsets,
    bvector<bool>&                      glyphIndexMask,
    DgnGlyphLayoutResult::T_GlyphCodesR outGlyphCodes, 
    DgnGlyphLayoutContextCR             context,
    bool                                shouldDisableGlyphShaping,
    bool                                shouldAllowMissingGlyphs
    ) override
        {
        if (NULL == m_ftFace)
            return ERROR;
        
        double totalLength = 0.0;

        // Currently only supports unidirectional, left-to-right text.
        for (size_t iChar = 0; iChar < totalNumChars; ++iChar)
            {
            FT_UInt glyphIndex = FT_Get_Char_Index (m_ftFace, chars[iChar]);
            if (0 == glyphIndex)
                { BeAssert (false); continue; }

            if (FT_Err_Ok != FT_Load_Glyph (m_ftFace, glyphIndex, FT_LOAD_DEFAULT))
                { BeAssert (false); continue; }
    
            FT_Vector   kerning;
            FT_UInt     nextGlyphIndex  = (((totalNumChars - 1) == iChar) ? 0 : FT_Get_Char_Index (m_ftFace, chars[iChar + 1]));

            if ((0 == nextGlyphIndex) || (FT_Err_Ok != FT_Get_Kerning (m_ftFace, glyphIndex, nextGlyphIndex, FT_KERNING_DEFAULT, &kerning)))
                memset (&kerning, 0, sizeof (kerning));
        
            outAdvanceWidths.push_back (((m_ftFace->glyph->advance.x >> 6) + (kerning.x >> 6)) * font.GetScaleFactor ());
            outGlyphOffsets.push_back (DPoint2d::From (0.0, 0.0));
            glyphIndexMask.push_back (false);
            outGlyphCodes.push_back (chars[iChar]);

            totalLength += outAdvanceWidths.back ();
            }
    
        return SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // Adapted for DGNPlatform from Mark Schlosser's prototype.
    // @bsimethod                                                   Jeff.Marker     06/2012
    //---------------------------------------------------------------------------------------
    private: static size_t ComputeFTOutlineWin32Size (FT_Outline& outline)
        {
        FT_Outline_Funcs outlineFuncs;
        memset (&outlineFuncs, 0, sizeof (outlineFuncs));
    
        outlineFuncs.conic_to   = (FT_Outline_ConicToFunc)&FreeTypeTTPImpl::FTOutlineConicTo;
        outlineFuncs.line_to    = (FT_Outline_LineToFunc)&FreeTypeTTPImpl::FTOutlineLineTo;
        outlineFuncs.cubic_to   = (FT_Outline_CubicToFunc)&FreeTypeTTPImpl::FTOutlineCubicTo;
        outlineFuncs.move_to    = (FT_Outline_MoveToFunc)&FreeTypeTTPImpl::FTOutlineMoveTo;
        outlineFuncs.shift      = 0;
        outlineFuncs.delta      = (FT_Pos)0;

        FTOutlineParseState parseState;
        memset (&parseState, 0, sizeof (parseState));
    
        if (FT_Err_Ok != FT_Outline_Decompose (&outline, &outlineFuncs, &parseState))
            return 0;

        return parseState.bufferSize;
        }
        
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    private: void ConfigureFontSize ()
        {
        // This craziness is legacy and DWG...
        double effectiveAscender = m_ftFace->ascender;
    
        if (FT_Err_Ok == FT_Load_Char (m_ftFace, L'A', FT_LOAD_NO_SCALE))
            effectiveAscender = m_ftFace->glyph->metrics.horiBearingY;

        // This is the real key to getting the glyphs to display at the right scale.
        // Unlike Win32, we utilize units_per_EM instead of requesting 2048 and having it pre-scaled.
        if (FT_Err_Ok != FT_Set_Pixel_Sizes (m_ftFace, 0, (FT_UInt)((double)m_ftFace->units_per_EM * ((double)m_ftFace->units_per_EM / effectiveAscender))))
            { BeAssert (false); }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual bool _ContainsCharacter (WChar uniChar, DgnTrueTypeFontCR font, bool isBold, bool isItalic) override
        {
        if (NULL == m_ftFace)
            return false;
        
        return (0 != FT_Get_Char_Index (m_ftFace, font.RemapUnicodeCharToFontChar (uniChar)));
        }

    //---------------------------------------------------------------------------------------
    // Adapted for DGNPlatform from Mark Schlosser's prototype.
    // @bsimethod                                                   Jeff.Marker     06/2012
    //---------------------------------------------------------------------------------------
    private: static BentleyStatus ConvertFTOutlineToWin32 (FT_Outline& outline, size_t bufferSize, Byte* buffer)
        {
        FT_Outline_Funcs outlineFuncs;
        memset (&outlineFuncs, 0, sizeof (outlineFuncs));

        outlineFuncs.conic_to   = (FT_Outline_ConicToFunc)&FreeTypeTTPImpl::FTOutlineConicTo;
        outlineFuncs.line_to    = (FT_Outline_LineToFunc)&FreeTypeTTPImpl::FTOutlineLineTo;
        outlineFuncs.cubic_to   = (FT_Outline_CubicToFunc)&FreeTypeTTPImpl::FTOutlineCubicTo;
        outlineFuncs.move_to    = (FT_Outline_MoveToFunc)&FreeTypeTTPImpl::FTOutlineMoveTo;
        outlineFuncs.shift      = 0;
        outlineFuncs.delta      = (FT_Pos)0;

        FTOutlineParseState parseState;
        memset (&parseState, 0, sizeof (parseState));
    
        parseState.buffer       = buffer;
        parseState.bufferSize   = bufferSize;

        if (FT_Err_Ok != FT_Outline_Decompose (&outline, &outlineFuncs, &parseState))
            return ERROR;

        return SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // Adapted for DGNPlatform from Mark Schlosser's prototype.
    // @bsimethod                                                   Jeff.Marker     06/2012
    //---------------------------------------------------------------------------------------
    private: static FIXED ConvertFTPosToFIXED (FT_Pos ftPos)
        {
        return FreeTypeTTPImpl::ToFixed (ftPos >> 6);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: static DgnFontPtr CreateLastResortFont ()
        {
        BeFileName lastResortFilePath = T_HOST.GetFontAdmin ()._GetLastResortTrueTypeFontFilePath ();
        if (lastResortFilePath.IsEmpty () || !BeFileName::DoesPathExist (lastResortFilePath.GetName ()))
            { BeAssert (false); return NULL; }
        
        static const CharCP LAST_RESORT_FONT_NAME = "LastResortTrueTypeFont";
        
        DgnTrueTypeFontP font = new DgnTrueTypeFont (LAST_RESORT_FONT_NAME, lastResortFilePath.GetName (), getEffectiveFontConfig (DgnFontKey (DgnFontType::TrueType, Utf8String (LAST_RESORT_FONT_NAME))));
        font->m_isLastResort = true;

        return font;
        }

    //---------------------------------------------------------------------------------------
    // Adapted for DGNPlatform from Mark Schlosser's prototype.
    // @bsimethod                                                   Jeff.Marker     06/2012
    //---------------------------------------------------------------------------------------
    private: static int FTOutlineConicTo (FT_Vector const* ftVecControl, FT_Vector const* ftVecTo, void* args)
        {
        FTOutlineParseState* parseState = static_cast<FTOutlineParseState*>(args);
        if (NULL == parseState->buffer)
            {
            parseState->bufferSize  += (2 * sizeof (WORD));
            parseState->bufferSize  += (3 * sizeof (POINTFX));
        
            return 0;
            }

        TTPOLYCURVE* currCurve  = reinterpret_cast<TTPOLYCURVE*>(parseState->buffer + parseState->bufferOffset);
        currCurve->wType        = TT_PRIM_QSPLINE;
        currCurve->cpfx         = 3;
    
        parseState->bufferOffset += (sizeof (currCurve->wType) + sizeof (currCurve->cpfx));
    
        POINTFX* currPoint  = reinterpret_cast<POINTFX*>(parseState->buffer + parseState->bufferOffset);
        currPoint->x        = parseState->lastContourPoint.x;
        currPoint->y        = parseState->lastContourPoint.y;
    
        ++currPoint;
        currPoint->x        = FreeTypeTTPImpl::ConvertFTPosToFIXED (ftVecControl->x);
        currPoint->y        = FreeTypeTTPImpl::ConvertFTPosToFIXED (ftVecControl->y);
    
        ++currPoint;
        currPoint->x        = FreeTypeTTPImpl::ConvertFTPosToFIXED (ftVecTo->x);
        currPoint->y        = FreeTypeTTPImpl::ConvertFTPosToFIXED (ftVecTo->y);
    
        parseState->bufferOffset        += (currCurve->cpfx * sizeof (POINTFX));
        parseState->currentContour->cb  += ((2 * sizeof (WORD)) + (3 * sizeof (POINTFX)));
        parseState->lastContourPoint.x  = currPoint->x;
        parseState->lastContourPoint.y  = currPoint->y;
    
        return 0;
        }

    //---------------------------------------------------------------------------------------
    // Adapted for DGNPlatform from Mark Schlosser's prototype.
    // @bsimethod                                                   Jeff.Marker     06/2012
    //---------------------------------------------------------------------------------------
    private: static int FTOutlineCubicTo (FT_Vector const* ftVecControl1, FT_Vector const* ftVecControl2, FT_Vector const* ftVecTo, void* args)
        {
        FTOutlineParseState* parseState = static_cast<FTOutlineParseState*>(args);
        if (NULL == parseState->buffer)
            {
            parseState->bufferSize  += (2 * sizeof (WORD));
            parseState->bufferSize  += (4 * sizeof (POINTFX));

            return 0;
            }

        TTPOLYCURVE* currCurve  = reinterpret_cast<TTPOLYCURVE*>(parseState->buffer + parseState->bufferOffset);
        currCurve->wType        = TT_PRIM_CSPLINE;
        currCurve->cpfx         = 4;
    
        parseState->bufferOffset += (sizeof (currCurve->wType) + sizeof (currCurve->cpfx));
    
        POINTFX* currPoint  = reinterpret_cast<POINTFX*>(parseState->buffer + parseState->bufferOffset);
        currPoint->x        = parseState->lastContourPoint.x;
        currPoint->y        = parseState->lastContourPoint.y;
    
        ++currPoint;
        currPoint->x        = FreeTypeTTPImpl::ConvertFTPosToFIXED (ftVecControl1->x);
        currPoint->y        = FreeTypeTTPImpl::ConvertFTPosToFIXED (ftVecControl1->y);
    
        ++currPoint;
        currPoint->x        = FreeTypeTTPImpl::ConvertFTPosToFIXED (ftVecControl2->x);
        currPoint->y        = FreeTypeTTPImpl::ConvertFTPosToFIXED (ftVecControl2->y);
    
        ++currPoint;
        currPoint->x        = FreeTypeTTPImpl::ConvertFTPosToFIXED (ftVecTo->x);
        currPoint->y        = FreeTypeTTPImpl::ConvertFTPosToFIXED (ftVecTo->y);
    
        parseState->bufferOffset        += currCurve->cpfx * sizeof (POINTFX);
        parseState->currentContour->cb  += ((2 * sizeof (WORD)) + (4 * sizeof (POINTFX)));
        parseState->lastContourPoint.x  = currPoint->x;
        parseState->lastContourPoint.y  = currPoint->y;
    
        return 0;
        }

    //---------------------------------------------------------------------------------------
    // Adapted for DGNPlatform from Mark Schlosser's prototype.
    // @bsimethod                                                   Jeff.Marker     06/2012
    //---------------------------------------------------------------------------------------
    private: static int FTOutlineLineTo (FT_Vector const* ftVecTo, void* args)
        {
        FTOutlineParseState* parseState = static_cast<FTOutlineParseState*>(args);
        if (NULL == parseState->buffer)
            {
            parseState->bufferSize  += (2 * sizeof (WORD));
            parseState->bufferSize  += (2 * sizeof (POINTFX));
        
            return 0;
            }

        TTPOLYCURVE* currCurve  = reinterpret_cast<TTPOLYCURVE*>(parseState->buffer + parseState->bufferOffset);
        currCurve->wType        = TT_PRIM_LINE;
        currCurve->cpfx         = 2;
    
        parseState->bufferOffset += (sizeof (currCurve->wType) + sizeof (currCurve->cpfx));
    
        POINTFX* currPoint  = reinterpret_cast<POINTFX*>(parseState->buffer + parseState->bufferOffset);
        currPoint->x        = parseState->lastContourPoint.x;
        currPoint->y        = parseState->lastContourPoint.y;
    
        ++currPoint;
        currPoint->x        = FreeTypeTTPImpl::ConvertFTPosToFIXED (ftVecTo->x);
        currPoint->y        = FreeTypeTTPImpl::ConvertFTPosToFIXED (ftVecTo->y);
    
        parseState->bufferOffset        += currCurve->cpfx * sizeof (POINTFX);
        parseState->currentContour->cb  += ((2 * sizeof (WORD)) + (2 * sizeof (POINTFX)));
        parseState->lastContourPoint.x  = currPoint->x;
        parseState->lastContourPoint.y  = currPoint->y;
    
        return 0;
        }

    //---------------------------------------------------------------------------------------
    // Adapted for DGNPlatform from Mark Schlosser's prototype.
    // @bsimethod                                                   Jeff.Marker     06/2012
    //---------------------------------------------------------------------------------------
    private: static int FTOutlineMoveTo (FT_Vector const* ftVecTo, void* args)
        {
        FTOutlineParseState* parseState = static_cast<FTOutlineParseState*>(args);
        if (NULL == parseState->buffer)
            {
            parseState->bufferSize += sizeof (TTPOLYGONHEADER);

            return 0;
            }

        TTPOLYGONHEADER* currCountour   = reinterpret_cast<TTPOLYGONHEADER*>(parseState->buffer + parseState->bufferOffset);
        currCountour->cb                = sizeof (TTPOLYGONHEADER);
        currCountour->dwType            = TT_POLYGON_TYPE;
        currCountour->pfxStart.x        = FreeTypeTTPImpl::ConvertFTPosToFIXED (ftVecTo->x);
        currCountour->pfxStart.y        = FreeTypeTTPImpl::ConvertFTPosToFIXED (ftVecTo->y);
    
        parseState->bufferOffset        += sizeof (TTPOLYGONHEADER);
        parseState->currentContour      = currCountour;
        parseState->lastContourPoint.x  = currCountour->pfxStart.x;
        parseState->lastContourPoint.y  = currCountour->pfxStart.y;
    
        return 0;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _GetFontMetrics (FontMetrics& metrics, DgnTrueTypeFontCR, bool isBold, bool isItalic) override
        {
        if (NULL == m_ftFace)
            return ERROR;
        
        metrics.m_scaleFactor       = (1.0 / m_ftFace->units_per_EM);
        metrics.m_descenderRatio    = fabs (m_ftFace->descender / (double)m_ftFace->ascender);

        return SUCCESS;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _GetGlyphData (bvector<Byte>& data, FontChar glyphID, bool isGlyphIndex, DgnTrueTypeFontCR, bool isBold, bool isItalic) override
        {
        if (NULL == m_ftFace)
            return ERROR;
        
        FT_UInt glyphIndex = (isGlyphIndex ? glyphID : FT_Get_Char_Index (m_ftFace, glyphID));
        if (0 == glyphIndex)
            { BeAssert (false); return ERROR; }

        if (FT_Err_Ok != FT_Load_Glyph (m_ftFace, glyphIndex, FT_LOAD_DEFAULT))
            { BeAssert (false); return ERROR; }
    
        size_t dataSize = FreeTypeTTPImpl::ComputeFTOutlineWin32Size (m_ftFace->glyph->outline);
        if (0 == dataSize)
            return SUCCESS;
        
        data.resize (dataSize);
        FreeTypeTTPImpl::ConvertFTOutlineToWin32 (m_ftFace->glyph->outline, dataSize, &data[0]);

        return SUCCESS;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _GetGlyphMetrics (GlyphMetrics& metrics, FontChar glyphID, bool isGlyphIndex, DgnTrueTypeFontCR, bool isBold, bool isItalic) override
        {
        if (NULL == m_ftFace)
            return ERROR;
        
        FT_UInt glyphIndex = (isGlyphIndex ? glyphID : FT_Get_Char_Index (m_ftFace, glyphID));
        if (0 == glyphIndex)
            { BeAssert (false); return ERROR; }

        if (FT_Err_Ok != FT_Load_Glyph (m_ftFace, glyphIndex, FT_LOAD_DEFAULT))
            { BeAssert (false); return ERROR; }
    
        metrics.m_cellBoxWidth      = (short) (m_ftFace->glyph->metrics.horiAdvance >> 6);
        metrics.m_blackBoxLeft      = (m_ftFace->glyph->metrics.horiBearingX >> 6);
        metrics.m_blackBoxTop       = (m_ftFace->glyph->metrics.horiBearingY >> 6);
        metrics.m_blackBoxWidth     = (m_ftFace->glyph->metrics.width >> 6);
        metrics.m_blackBoxHeight    = (m_ftFace->glyph->metrics.height >> 6);
        
        return SUCCESS;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _LayoutGlyphs (DgnTrueTypeFontCR font, DgnGlyphLayoutContextCR layoutContext, WCharCP processedChars, size_t processedCharsCount, DgnGlyphLayoutResultR layoutResult) override
        {
        if (NULL == m_ftFace)
            return ERROR;
        
        return font.LayoutGlyphsInternal (layoutContext, layoutResult, processedChars, processedCharsCount);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _RegisterFileFont (WCharCP filePath) override
        {
        Utf8String filePathUtf8 (filePath);

        if ((FT_Err_Ok != FT_New_Face (FreeTypeThreadGlobals::Instance ().GetFTLibrary (), filePathUtf8.c_str (), 0, &m_ftFace)) || (NULL == m_ftFace))
            {
            BeAssert (false);
            return ERROR;
            }

        ConfigureFontSize ();

        return SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _RegisterMemoryFont (Byte* data, size_t dataSize, bool& callerMustRetain) override
        {
        if ((FT_Err_Ok != FT_New_Memory_Face (FreeTypeThreadGlobals::Instance ().GetFTLibrary (), data, (FT_Long)dataSize, 0, &m_ftFace)) || (NULL == m_ftFace))
            {
            BeAssert (false);
            callerMustRetain = false;
            return ERROR;
            }

        callerMustRetain = true;

        ConfigureFontSize ();

        return SUCCESS;
        }
        
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    private: static FIXED ToFixed (double d)
        {
        long l = (long)(d * 65536L);
        return *(FIXED*)&l;
        }

    }; // FreeTypeTTPImpl

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct ITrueTypeDataAccessor
    {
    virtual ~ITrueTypeDataAccessor () {}
    virtual bool _CanEmbed (bool isBold, bool isItalic) = 0;
    virtual bool _ContainsCharacter (WChar, bool isBold, bool isItalic) = 0;
    virtual BentleyStatus _Embed (DgnDbR, uint32_t fontNumber) = 0;
    virtual BentleyStatus _GetFontMetrics (FontMetrics&, bool isBold, bool isItalic) = 0;
    virtual BentleyStatus _GetGlyphData (bvector<Byte>&, FontChar glyphID, bool isGlyphIndex, bool isBold, bool isItalic) = 0;
    virtual BentleyStatus _GetGlyphMetrics (GlyphMetrics&, FontChar glyphID, bool isGlyphIndex, bool isBold, bool isItalic) = 0;
    }; 

END_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
PropertySpec DgnTrueTypeFont::CreateEmbeddedFontPropertySpec ()
    {
    return PropertySpec ("ttdata", PROPERTY_APPNAME_DgnFont);
    }

// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)

// *** WIP_ForeignFormat - We should move this code into foreignformat as part of the implementation of an IDgnFontFinder. See DgnFontFinder for an example.

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct Win32SystemTTDataAcessor : public ITrueTypeDataAccessor
    {
    DEFINE_T_SUPER (ITrueTypeDataAccessor)

    private:    ITTPImpl*           m_ttPImpl;
    private:    DgnTrueTypeFontCR   m_font;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: Win32SystemTTDataAcessor (DgnTrueTypeFontCR font, ITTPImpl& ttPImpl) :
        m_ttPImpl   (&ttPImpl),
        m_font      (font)
        {
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual bool _CanEmbed (bool isBold, bool isItalic) override
        {
        return m_ttPImpl->_CanEmbed (m_font, isBold, isItalic);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual bool _ContainsCharacter (WChar uniChar, bool isBold, bool isItalic) override
        {
        return m_ttPImpl->_ContainsCharacter (uniChar, m_font, isBold, isItalic);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _Embed (DgnDbR project, uint32_t fontNumber) override
        {
        if (SUCCESS != m_ttPImpl->_Activate (m_font, false, false))
            { BeAssert(false); return ERROR; }

        Win32TTThreadGlobals& ttGlobals = Win32TTThreadGlobals::Instance ();
        
        // There is no reliable way to go from HFONT to file, but we can use GetFontData to read the raw bytes from the font.
        // The dwTable parameter to GetFontData allows as follows: If this parameter is zero, the information is retrieved starting at the beginning of the file for TrueType font files or from the beginning of the data for the currently selected font for TrueType Collection files. To retrieve the data from the beginning of the file for TrueType Collection files specify 'ttcf' (0x66637474).
        // Since TTC files are a superset of TTF, check if the font is a collection first with the special table, then fall back for normal fonts.
        DWORD dwTable = 0x66637474; // Big endian DWORD of 'ttcf'.
        DWORD fontDataSize = ::GetFontData(ttGlobals.GetDC(), dwTable, 0, NULL, 0);
        if (GDI_ERROR == fontDataSize)
            {
            dwTable = 0;
            fontDataSize = ::GetFontData(ttGlobals.GetDC(), dwTable, 0, NULL, 0);
            }

        if ((GDI_ERROR == fontDataSize) || (0 == fontDataSize))
            { BeAssert(false); return ERROR; }
        
        ScopedArray<Byte> fontData ((size_t)fontDataSize);
        DWORD bytesRetrieved = ::GetFontData(ttGlobals.GetDC(), dwTable, 0, fontData.GetData(), fontDataSize);
        if ((GDI_ERROR == bytesRetrieved) || (0 == bytesRetrieved))
            { BeAssert(false); return ERROR; }
        
        BeSQLite::DbResult res = project.SaveProperty(DgnTrueTypeFont::CreateEmbeddedFontPropertySpec(), fontData.GetData(), (uint32_t)fontDataSize, fontNumber);
    
        return ((BE_SQLITE_OK == res) ? SUCCESS : ERROR);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _GetFontMetrics (FontMetrics& metrics, bool isBold, bool isItalic) override
        {
        return m_ttPImpl->_GetFontMetrics (metrics, m_font, isBold, isItalic);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _GetGlyphData (bvector<Byte>& buffer, FontChar glyphID, bool isGlyphIndex, bool isBold, bool isItalic) override
        {
        return m_ttPImpl->_GetGlyphData (buffer, glyphID, isGlyphIndex, m_font, isBold, isItalic);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _GetGlyphMetrics (GlyphMetrics& metrics, FontChar glyphID, bool isGlyphIndex, bool isBold, bool isItalic) override
        {
        return m_ttPImpl->_GetGlyphMetrics (metrics, glyphID, isGlyphIndex, m_font, isBold, isItalic);
        }

    }; // Win32SystemTTDataAcessor

#endif

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

// WIP_NONPORT - TrueType
#if !defined (BENTLEY_WIN32)

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct FileTrueTypeDataAccessor : public ITrueTypeDataAccessor
    {
    private:    ITTPImpl*           m_ttPImpl;
    private:    DgnTrueTypeFontCR   m_font;
    private:    WString             m_filePath;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: FileTrueTypeDataAccessor (WCharCP filePath, DgnTrueTypeFontCR font, ITTPImpl& ttPImpl) :
        m_ttPImpl   (&ttPImpl),
        m_font      (font),
        m_filePath  (filePath)
        {
        if (SUCCESS != m_ttPImpl->_RegisterFileFont (filePath))
            { BeAssert (false); }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual bool _CanEmbed (bool isBold, bool isItalic) override
        {
        return m_ttPImpl->_CanEmbed (m_font, isBold, isItalic);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual bool _ContainsCharacter (WChar uniChar, bool isBold, bool isItalic) override
        {
        return m_ttPImpl->_ContainsCharacter (uniChar, m_font, isBold, isItalic);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _Embed (DgnDbR project, uint32_t fontNumber) override
        {
        if (m_filePath.empty ())
            { BeAssert (false); return ERROR; }
    
        BeFile fontFile;
        if (BeFileStatus::Success != fontFile.Open (m_filePath.c_str (), BeFileAccess::Read))
            { BeAssert (false); return ERROR; }
    
        uint64_t fileSize = 0;
        if ((BeFileStatus::Success != fontFile.GetSize (fileSize)) || (0 == fileSize))
            { BeAssert (false); return ERROR; }
        
        ScopedArray<Byte> fileData ((size_t)fileSize);
        if (BeFileStatus::Success != fontFile.Read (fileData.GetData (), NULL, (uint32_t)fileSize))
            { BeAssert (false); return ERROR; }

        BeSQLite::DbResult res = project.SaveProperty (DgnTrueTypeFont::CreateEmbeddedFontPropertySpec(), fileData.GetData (), (uint32_t)fileSize, fontNumber);
    
        return ((BE_SQLITE_OK == res) ? SUCCESS : ERROR);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _GetFontMetrics (FontMetrics& metrics, bool isBold, bool isItalic) override
        {
        return m_ttPImpl->_GetFontMetrics (metrics, m_font, isBold, isItalic);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _GetGlyphData (bvector<Byte>& buffer, FontChar glyphID, bool isGlyphIndex, bool isBold, bool isItalic) override
        {
        return m_ttPImpl->_GetGlyphData (buffer, glyphID, isGlyphIndex, m_font, isBold, isItalic);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _GetGlyphMetrics (GlyphMetrics& metrics, FontChar glyphID, bool isGlyphIndex, bool isBold, bool isItalic) override
        {
        return m_ttPImpl->_GetGlyphMetrics (metrics, glyphID, isGlyphIndex, m_font, isBold, isItalic);
        }

    }; // FileTrueTypeDataAccessor

#endif

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct DbTrueTypeDataAccessor : public ITrueTypeDataAccessor
    {
    private:    ITTPImpl*           m_ttPImpl;
    private:    uint32_t            m_fontNumber;
    private:    DgnDbCR        m_dgndb;
    private:    DgnTrueTypeFontCR   m_font;
    private:    bvector<Byte>       m_ttData;
    private:    bool                m_wasRegistered;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: DbTrueTypeDataAccessor (uint32_t fontNumber, DgnDbCR project, DgnTrueTypeFontCR font, ITTPImpl& ttPImpl) :
        m_ttPImpl       (&ttPImpl),
        m_fontNumber    (fontNumber),
        m_dgndb       (project),
        m_font          (font),
        m_wasRegistered (false)
        {
        // Do a basic sanity check to ensure the font is there.
        PropertySpec ttDataPropSpec = DgnTrueTypeFont::CreateEmbeddedFontPropertySpec();
        if (!m_dgndb.HasProperty(ttDataPropSpec, m_fontNumber))
            { BeAssert (false); return; }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual bool _CanEmbed (bool isBold, bool isItalic) override
        {
        EnsureRegistered();
        return m_ttPImpl->_CanEmbed (m_font, isBold, isItalic);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual bool _ContainsCharacter (WChar uniChar, bool isBold, bool isItalic) override
        {
        EnsureRegistered();
        return m_ttPImpl->_ContainsCharacter (uniChar, m_font, isBold, isItalic);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _Embed (DgnDbR project, uint32_t fontNumber) override
        {
        PropertySpec    ttDataPropSpec = DgnTrueTypeFont::CreateEmbeddedFontPropertySpec();
        uint32_t        ttDataPropSize;
        if (BE_SQLITE_ROW != m_dgndb.QueryPropertySize (ttDataPropSize, ttDataPropSpec, m_fontNumber))
            { BeAssert (false); return ERROR; }
    
        ScopedArray<Byte> ttDataPropValue (ttDataPropSize);
        if (BE_SQLITE_ROW != m_dgndb.QueryProperty (ttDataPropValue.GetData (), ttDataPropSize, ttDataPropSpec, m_fontNumber))
            { BeAssert (false); return ERROR; }
        
        if (BE_SQLITE_OK != project.SaveProperty (ttDataPropSpec, ttDataPropValue.GetData (), ttDataPropSize, fontNumber))
            { BeAssert (false); return ERROR; }
        
        return SUCCESS;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     01/2013
    //---------------------------------------------------------------------------------------
    private: void EnsureRegistered()
        {
        if (m_wasRegistered)
            return;
        
        m_wasRegistered = true;
        uint32_t ttDataPropSize;
        
            {
            // Font data is loaded on-demand, which could be during an update when drawing an element that uses a font for the first time.
            HighPriorityOperationBlock highPriorityOperationBlock;

            PropertySpec ttDataPropSpec = DgnTrueTypeFont::CreateEmbeddedFontPropertySpec();
        
            if (BE_SQLITE_ROW != m_dgndb.QueryPropertySize (ttDataPropSize, ttDataPropSpec, m_fontNumber))
                { BeAssert (false); return; }

            m_ttData.resize (ttDataPropSize);
        
            if (BE_SQLITE_ROW != m_dgndb.QueryProperty (&m_ttData[0], ttDataPropSize, ttDataPropSpec, m_fontNumber))
                { BeAssert (false); return; }
            }
        
        bool mustRetainData;
        if (SUCCESS != m_ttPImpl->_RegisterMemoryFont (&m_ttData[0], ttDataPropSize, mustRetainData))
            BeAssert (false);

        if (!mustRetainData)
            m_ttData.clear ();
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _GetFontMetrics (FontMetrics& metrics, bool isBold, bool isItalic) override
        {
        EnsureRegistered();
        return m_ttPImpl->_GetFontMetrics (metrics, m_font, isBold, isItalic);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _GetGlyphData (bvector<Byte>& buffer, FontChar glyphID, bool isGlyphIndex, bool isBold, bool isItalic) override
        {
        EnsureRegistered();
        return m_ttPImpl->_GetGlyphData (buffer, glyphID, isGlyphIndex, m_font, isBold, isItalic);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2012
    //---------------------------------------------------------------------------------------
    public: virtual BentleyStatus _GetGlyphMetrics (GlyphMetrics& metrics, FontChar glyphID, bool isGlyphIndex, bool isBold, bool isItalic) override
        {
        EnsureRegistered();
        return m_ttPImpl->_GetGlyphMetrics (metrics, glyphID, isGlyphIndex, m_font, isBold, isItalic);
        }

    }; // DbTrueTypeDataAccessor

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2012
//=======================================================================================
struct MissingTrueTypeDataAccessor : public ITrueTypeDataAccessor
    {
    virtual bool _CanEmbed (bool isBold, bool isItalic) override { return false; }
    virtual bool _ContainsCharacter (WChar, bool isBold, bool isItalic) override { return false; }
    virtual BentleyStatus _Embed (DgnDbR, uint32_t fontNumber) override { return ERROR; }
    virtual BentleyStatus _GetFontMetrics (FontMetrics&, bool isBold, bool isItalic) override { return ERROR; }
    virtual BentleyStatus _GetGlyphData (bvector<Byte>&, FontChar glyphID, bool isGlyphIndex, bool isBold, bool isItalic) override { return ERROR; }
    virtual BentleyStatus _GetGlyphMetrics (GlyphMetrics&, FontChar glyphID, bool isGlyphIndex, bool isBold, bool isItalic) override { return ERROR; }
    
    }; // MissingTrueTypeDataAccessor

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static void toDPoint4d (DPoint4d* pt, POINTFX* pPoint, double scaleFactor)
    {
    pt->x   = ((double)(pPoint->x.value + (double)pPoint->x.fract / 65536.0) * scaleFactor);
    pt->y   = ((double)(pPoint->y.value + (double)pPoint->y.fract / 65536.0) * scaleFactor);
    pt->z   = 0.0;
    pt->w   = 1.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static void convertNativeGlyphToGraphicsPoints (GraphicsPointArrayR gpa, LPTTPOLYGONHEADER lpHeader, size_t size, double scaleFactor)
    {
    LPTTPOLYGONHEADER   lpStart = lpHeader;
    LPTTPOLYCURVE       lpCurve;
    int                 i = 0;

    while ((uintptr_t)lpHeader < (uintptr_t)(((LPSTR)lpStart) + size))
        {
        if (TT_POLYGON_TYPE == lpHeader->dwType)
            {
            DPoint4d ptHead;
            DPoint4d ptLast;

            // Get to first curve.
            lpCurve = (LPTTPOLYCURVE)(lpHeader + 1);

            while ((uintptr_t)lpCurve < (uintptr_t)(((LPSTR)lpHeader) + lpHeader->cb))
                {
                // Format assumption:
                //  The bytes immediately preceding a POLYCURVE structure contain a valid POINTFX.
                //  If this is first curve, this points to the pfxStart of the POLYGONHEADER. Otherwise, this points to the last point of the previous POLYCURVE.
                //  In either case, this is representative of the previous curve's last point.
            
                if (TT_PRIM_LINE == lpCurve->wType)
                    {
                    DPoint4d pt;
                    toDPoint4d  (&pt, (LPPOINTFX)((LPSTR)lpCurve - sizeof(POINTFX)), scaleFactor);
                
                    gpa.Add (GraphicsPoint (pt));

                    for (i = 0; i < lpCurve->cpfx; ++i)
                        {
                        toDPoint4d (&pt, &lpCurve->apfx[i], scaleFactor);
                        gpa.Add (GraphicsPoint (pt));
                        }
                    }
                else if (lpCurve->wType == TT_PRIM_QSPLINE)
                    {
                    DPoint4d    pts[100];
                    DPoint4d    ptsp[50];
                    DPoint4d*   pt          = pts;
                    DPoint4d*   pt1         = ptsp;
                    DPoint4d*   pMem        = NULL;
                    DPoint4d*   pMem1       = NULL;
                    DPoint4d*   pStart      = pts;
                    POINTFX*    pFixed;
                    int         iSegments   = 1;
                    int         numPoles    = (lpCurve->cpfx + 1);
                    int         order       = 3;

                    if ((lpCurve->cpfx + 1) > 100)
                        {
                        pMem = (DPoint4d*)malloc ((lpCurve->cpfx + 1) * sizeof (DPoint4d));
                        if (NULL == pMem)
                            return;
                    
                        pStart  = pMem;
                        pt      = pMem;
                        }

                    if ((numPoles > 3) && (numPoles + (numPoles - order) * (order - 1)) > 50)
                        {
                        pMem1 = (DPoint4d*)malloc ((numPoles + (numPoles - order) * (order - 1)) * sizeof (DPoint4d));
                        if (NULL == pMem1)
                            return;
                    
                        pt1 = pMem1;
                        }

                    toDPoint4d (pt, (LPPOINTFX)((LPSTR)lpCurve - sizeof(POINTFX)), scaleFactor);
                    ++pt;

                    for (i = 0, pFixed = lpCurve->apfx; i < lpCurve->cpfx; ++i, ++pt, ++pFixed)
                        toDPoint4d (pt, pFixed, scaleFactor);

                    if (numPoles > 3)
                        {
                        iSegments   = bsiBezierDPoint4d_convertOpenUniformBsplineToBeziers (pt1, pStart, numPoles, order, false);
                        pStart      = pt1;
                        }

                    for (int i = 0; i < iSegments; ++i)
                        gpa.AddBezier ((pStart + (i * order)), order);

                    if(pMem)
                        free(pMem);
                
                    if(pMem1)
                        free(pMem1);
                    }

                    // Move on to next curve.
                    lpCurve = (LPTTPOLYCURVE)&(lpCurve->apfx[i]);
                }

                // Add points to close curve. Depending on the specific font and glyph being used, these may not always be needed, but it never hurts. Our last point could have been a curve so we need to add that point again.
                toDPoint4d (&ptLast, (LPPOINTFX)((LPSTR)lpCurve - sizeof(POINTFX)), scaleFactor);
                gpa.Add (GraphicsPoint (ptLast));

                toDPoint4d (&ptHead, &lpHeader->pfxStart, scaleFactor);
                gpa.Add (GraphicsPoint (ptHead));

                // Move on to next polygon.
                lpHeader = (LPTTPOLYGONHEADER)(((LPSTR)lpHeader) + lpHeader->cb);
            }
            else
            {
                break;
            }
    
        gpa.MarkMajorBreak ();
        }
    
    // Add user break so we can transform individual glyphs.
    gpa.MarkMajorBreak ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontType DgnTrueTypeGlyph::_GetType  () const    { return DgnFontType::TrueType; }
bool        DgnTrueTypeGlyph::_IsBlank  () const    { return m_isBlank; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnTrueTypeGlyph::DgnTrueTypeGlyph (FontChar charCode, bool isGlyphIndex, ITrueTypeDataAccessor& data, double fontScaleFactor, bool isBold, bool isItalic) :
    T_Super (charCode),
    m_data              (&data),
    m_fontScaleFactor   (fontScaleFactor),
    m_isGlyphIndex      (isGlyphIndex),
    m_isBold            (isBold),
    m_isItalic          (isItalic)
    {
    GlyphMetrics metrics;
    if (SUCCESS != m_data->_GetGlyphMetrics (metrics, m_charCode, m_isGlyphIndex, m_isBold, m_isItalic))
        {
        BeAssert (false);
        
        m_blackBoxStart.Zero ();
        m_blackBoxEnd.Zero ();
        m_cellBoxStart.Zero ();
        m_cellBoxEnd.Zero ();
        m_isBlank = true;
        
        return;
        }

    m_blackBoxStart.x   = (m_fontScaleFactor * metrics.m_blackBoxLeft);
    m_blackBoxEnd.y     = (m_fontScaleFactor * metrics.m_blackBoxTop);
    m_blackBoxEnd.x     = (m_blackBoxStart.x + (m_fontScaleFactor * metrics.m_blackBoxWidth));
    m_blackBoxStart.y   = (m_blackBoxEnd.y - (m_fontScaleFactor * metrics.m_blackBoxHeight));
    m_cellBoxStart.x    = 0.0;
    m_cellBoxStart.y    = 0.0;
    m_cellBoxEnd.x      = (m_fontScaleFactor * metrics.m_cellBoxWidth);
    m_cellBoxEnd.y      = 0.0;
    m_isBlank           = (metrics.m_blackBoxWidth <= 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeGlyph::_FillGpa (GPArrayR gpa) const
    {
    bvector<Byte> glyphData = GetTrueTypeData ();
    if (glyphData.empty ())
        return ERROR;
    
    convertNativeGlyphToGraphicsPoints (gpa, reinterpret_cast<TTPOLYGONHEADER*>(&glyphData[0]), glyphData.size (), m_fontScaleFactor);
    gpa.SetArrayMask (HPOINT_ARRAYMASK_FILL);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
bvector<Byte> DgnTrueTypeGlyph::GetTrueTypeData () const
    {
    bvector<Byte> buffer;
    if (SUCCESS != m_data->_GetGlyphData (buffer, m_charCode, m_isGlyphIndex, m_isBold, m_isItalic))
        BeAssert (false);

    return buffer;
    }

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
LangCodePage    DgnTrueTypeFont::_GetCodePage               () const    { return LangCodePage::Unicode; }
FontChar        DgnTrueTypeFont::_GetDegreeCharCode         () const    { return (FontChar)SPECIALCHAR_UnicodeDegree; }
double          DgnTrueTypeFont::_GetDescenderRatio         () const    { if (m_isMissing) { return 0.0; } EnsureFontIsLoaded (); return m_descenderRatio; }
FontChar        DgnTrueTypeFont::_GetDiameterCharCode       () const    { return (FontChar)SPECIALCHAR_UnicodeDiameter; }
FontChar        DgnTrueTypeFont::_GetPlusMinusCharCode      () const    { return (FontChar)SPECIALCHAR_UnicodePlusMinus; }
double          DgnTrueTypeFont::GetScaleFactor             () const    { if (m_isMissing) { return 0.0; } EnsureFontIsLoaded (); return m_scaleFactor; }
DgnFontType     DgnTrueTypeFont::_GetType                   () const    { return DgnFontType::TrueType; }
DgnFontVariant  DgnTrueTypeFont::_GetVariant                () const    { return DGNFONTVARIANT_TrueType; }
bool            DgnTrueTypeFont::_CanDrawWithLineWeight  () const    { return false; }

// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)

// Data accessors need the DgnTrueTypeFont object for later reference, and will not attempt to access it in the constructor.
#pragma warning (push)
#pragma warning (disable:4355) 

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnTrueTypeFont::DgnTrueTypeFont (Utf8CP name, DgnFontConfigurationData const& config) :
    T_Super (name, config),
    m_ttPImpl           (new Win32TTPImpl ()),
    m_data              (new Win32SystemTTDataAcessor (*this, *m_ttPImpl)),
    m_wasLoaded         (false),
    m_scaleFactor       (0.0),
    m_descenderRatio    (0.0)
    {
    memset (m_uniscribeScriptCaches, 0, sizeof (m_uniscribeScriptCaches));
    }

#else

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnTrueTypeFont::DgnTrueTypeFont (Utf8CP name, WCharCP filePath, DgnFontConfigurationData const& config) :
    T_Super (name, config),
    m_ttPImpl           (new FreeTypeTTPImpl ()),
    m_wasLoaded         (false),
    m_scaleFactor       (0.0),
    m_descenderRatio    (0.0)
    {
    if (NULL == filePath)
        m_data = new MissingTrueTypeDataAccessor ();
    else
        m_data = new FileTrueTypeDataAccessor (filePath, *this, *m_ttPImpl);
    }

#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnTrueTypeFont::DgnTrueTypeFont (Utf8CP name, DgnFontConfigurationData const& config, uint32_t fontNumber, DgnDbCR project) :
    T_Super (name, config),
// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)
    m_ttPImpl           (new Win32TTPImpl ()),
#else
    m_ttPImpl           (new FreeTypeTTPImpl ()),
#endif
    m_data              (new DbTrueTypeDataAccessor (fontNumber, project, *this, *m_ttPImpl)),
    m_wasLoaded         (false),
    m_scaleFactor       (0.0),
    m_descenderRatio    (0.0)
    {
// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)
    memset (m_uniscribeScriptCaches, 0, sizeof (m_uniscribeScriptCaches));
#endif
    }

// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)

#pragma warning (pop)

#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnTrueTypeFont::~DgnTrueTypeFont ()
    {
    delete m_data;
    delete m_ttPImpl;

// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)

    for (size_t iScriptCache = 0; iScriptCache < _countof (m_uniscribeScriptCaches); ++iScriptCache)
        ::ScriptFreeCache (&m_uniscribeScriptCaches[iScriptCache]);

#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
void DgnTrueTypeFont::AcquireSystemFonts (T_FontCatalogMap& systemFonts)
    {
// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)
    Win32TTPImpl::EnumerateSystemFonts (systemFonts);
#endif
    }

//---------------------------------------------------------------------------------------
// Official Msdn docs: http://msdn.microsoft.com/en-us/library/dd374091(VS.85).aspx
// Additional documentation and examples from a Google Chrome developer (Brett Wilson): http://maxradi.us/documents/uniscribe/.
// @bsimethod                                                   Jeff.Marker     05/2010
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeFont::ComputeAdvanceWidths
(
WCharCP                             chars,
size_t                              totalNumChars,
T_DoubleVectorR                     outAdvanceWidths,
bvector<DPoint2d>&                  outGlyphOffsets,
bvector<bool>&                      glyphIndexMask,
DgnGlyphLayoutResult::T_GlyphCodesR outGlyphCodes, 
DgnGlyphLayoutContextCR             context,
bool                                shouldDisableGlyphShaping,
bool                                shouldAllowMissingGlyphs
) const
    {
    return m_ttPImpl->_ComputeAdvanceWidths(*this, chars, totalNumChars, outAdvanceWidths, outGlyphOffsets, glyphIndexMask, outGlyphCodes, context, shouldDisableGlyphShaping, shouldAllowMissingGlyphs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
bool DgnTrueTypeFont::_ContainsCharacter (WChar uniChar) const
    {
    // Because one DgnTrueTypeFont object represents 4 faces, assume the font metrics are uniform across all faces, and use no-bold/no-italic here.
    return m_data->_ContainsCharacter (uniChar, false, false);
    }

// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontPtr DgnTrueTypeFont::CreateDecoratorLogicalFont ()
    {
    // Windows logical fonts are supposed to remap under various locales to accomodate the active locale's character set.
    // The logical font names don't show up when enumerating (so can't use normal find functions), and are Windows-specific.
    // Due to face name remapping, I'm also not sure how to detect if the requested name actually worked.
    
    static CharCP LOGICAL_NAME = "MS Shell Dlg 2";
    
    return new DgnTrueTypeFont (LOGICAL_NAME, getEffectiveFontConfig (DgnFontKey (DgnFontType::TrueType, Utf8String (LOGICAL_NAME))));
    }

#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontPtr DgnTrueTypeFont::CreateFromEmbeddedFont (Utf8CP name, uint32_t fontNumber, DgnDbCR project)
    {
    return new DgnTrueTypeFont (name, getEffectiveFontConfig (DgnFontKey (DgnFontType::TrueType, Utf8String (name))), fontNumber, project);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontPtr DgnTrueTypeFont::CreateLastResortFont ()
    {
// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)
    return Win32TTPImpl::CreateLastResortFont ();
#else
    return FreeTypeTTPImpl::CreateLastResortFont ();
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontPtr DgnTrueTypeFont::CreateMissingFont (Utf8CP name)
    {
// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)
    DgnTrueTypeFontP font = new DgnTrueTypeFont (name, getEffectiveFontConfig (DgnFontKey (DgnFontType::TrueType, Utf8String (name))));
#else
    DgnTrueTypeFontP font = new DgnTrueTypeFont (name, NULL, getEffectiveFontConfig (DgnFontKey (DgnFontType::TrueType, Utf8String (name))));
#endif

    font->m_isMissing = true;
    
    return font;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontPtr DgnTrueTypeFont::_Embed (DgnDbR project) const
    {
    // Can't embed missing fonts; don't bother embedding last resort fonts.
    if (IsLastResort() || IsMissing())
        return NULL;
    
    // Because one DgnTrueTypeFont object represents 4 faces, assume embedability is uniform across all faces, and use no-bold/no-italic here.
    if (!m_data->_CanEmbed (false, false))
        return NULL;
    
    T_FontCatalogMap::const_iterator foundFont = project.Fonts().EmbeddedFonts().find(DgnFontKey (DgnFontType::TrueType, m_name));
    if (project.Fonts().EmbeddedFonts ().end () != foundFont)
        return foundFont->second;
    
    uint32_t fontNumber;
    if (SUCCESS != project.Fonts().AcquireFontNumber (fontNumber, *this))
        { BeAssert (false); return NULL; }
    
    if (SUCCESS != m_data->_Embed (project, fontNumber))
        return NULL;

    return DgnTrueTypeFont::CreateFromEmbeddedFont (m_name.c_str (), fontNumber, project);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void DgnTrueTypeFont::EnsureFontIsLoaded () const
    {
    if (m_wasLoaded)
        return;
    
    m_wasLoaded = true;
    
    if (m_isMissing)
        { BeAssert (false && L"Don't attempt to load a missing font!"); return; }

    // Because one DgnTrueTypeFont object represents 4 faces, assume the font metrics are uniform across all faces, and use no-bold/no-italic here.

    FontMetrics fontMetrics;
    if (SUCCESS != m_data->_GetFontMetrics (fontMetrics, false, false))
        {
        BeAssert (false);
        m_isMissing = true;
        return;
        }
    
    m_scaleFactor       = fontMetrics.m_scaleFactor;
    m_descenderRatio    = fontMetrics.m_descenderRatio;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
WChar DgnTrueTypeFont::FixDiameterCharCode (WChar uniChar) const
    {
    if ((SPECIALCHAR_UnicodeDiameter != uniChar) && (SPECIALCHAR_UnicodeRealDiameter != uniChar))
        return uniChar;

    if (_ContainsCharacter(uniChar))
        return uniChar;

    if (_ContainsCharacter(SPECIALCHAR_UnicodeDiameter))
        return (WChar)SPECIALCHAR_UnicodeDiameter;

    if (_ContainsCharacter(SPECIALCHAR_UnicodeRealDiameter))
        return (WChar)SPECIALCHAR_UnicodeRealDiameter;

    if (_ContainsCharacter(SPECIALCHAR_UnicodeCapitalOWithStroke))
        return (WChar)SPECIALCHAR_UnicodeCapitalOWithStroke;

    if (_ContainsCharacter(SPECIALCHAR_UnicodeSmallOWithStroke))
        return (WChar)SPECIALCHAR_UnicodeSmallOWithStroke;

    return uniChar;
    }

// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)

static const int MAX_VALUE_NAME = 1024;

//---------------------------------------------------------------------------------------
// @bsimethod                                   PaulChater                      01/99
//---------------------------------------------------------------------------------------
static bool queryRegistryKey (CharP pFaceName, CharCP pFileName, HANDLE hKey, bool (*pFunc)(char*, CharCP, char*, CharP))
    {
    CHAR        achKey[MAX_PATH];
    CHAR        achClass[MAX_PATH] = "";    // buffer for class name
    DWORD       cchClassName = MAX_PATH;    // length of class string
    DWORD       cSubKeys;                   // number of subkeys
    DWORD       cbMaxSubKey;                // longest subkey size
    DWORD       cchMaxClass;                // longest class string

    DWORD       cValues;                    // number of values for key
    DWORD       cchMaxValue;                // longest value name
    DWORD       cbMaxValueData;             // longest value data
    DWORD       cbSecurityDescriptor;       // size of security descriptor
    FILETIME   ftLastWriteTime;            // last write time

    DWORD i, j;
    DWORD retCode, retValue;

    CHAR  achValue[MAX_VALUE_NAME];

    DWORD cchValue = MAX_VALUE_NAME;

    // Get the class name and the value count.

    ::RegQueryInfoKeyA((HKEY) hKey,    // key handle
        achClass,                   // buffer for class name
        &cchClassName,              // length of class string
        NULL,                       // reserved
        &cSubKeys,                  // number of subkeys
        &cbMaxSubKey,               // longest subkey size

        &cchMaxClass,               // longest class string
        &cValues,                   // number of values for this key
        &cchMaxValue,               // longest value name
        &cbMaxValueData,            // longest value data
        &cbSecurityDescriptor,      // security descriptor
        &ftLastWriteTime);          // last write time

    // Loop until RegEnumKey fails. Then get the name of each child key and type it into the box.
    // Enumerate the child keys.

    for (i = 0, retCode = ERROR_SUCCESS; retCode == ERROR_SUCCESS; i++)
        {
        retCode = ::RegEnumKeyA ((HKEY)hKey, i, achKey, MAX_PATH);

        if (retCode == (DWORD)ERROR_SUCCESS)
            return false;
        }

    // Enumerate the key values.

    if (cValues)
        {
        for (j = 0, retValue = ERROR_SUCCESS; j < cValues; j++)
            {
            char    data[MAX_VALUE_NAME];
            DWORD   dataLen                 = sizeof(data);
            char    *faceName               = achValue;

            *data = 0;
            cchValue = MAX_VALUE_NAME;
            achValue[0] = '\0';
            retValue = ::RegEnumValueA ((HKEY)hKey, j, achValue, &cchValue, NULL, NULL, (LPBYTE)data, &dataLen);

            if (!lstrlenA (achValue))
                continue;

            if (pFunc (data, pFileName, faceName, pFaceName))
                return true;
            }
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Paul.Chater                   03/99
//---------------------------------------------------------------------------------------
static bool getFaceName (CharP data, CharCP pFileName, CharP faceName, CharP pFaceName)
    {
    if (0 == BeStringUtilities::Stricmp (data, pFileName))
        {
        char const * const delim = strstr (faceName, " (TrueType)");
        if (delim)
            {
            int const len = (int)(delim - faceName);
            strncpy ((char*) pFaceName, faceName, len);
            pFaceName[len] = 0;
            return true;
            }
        // On Win9X, some TTF fonts do not have (TrueType) in the face name, so we will look instead of .TTF as the font file extension.
        else
            {
            char fileExt[_MAX_EXT];

            memset (fileExt, 0, sizeof(fileExt));
            _splitpath (pFileName, NULL, NULL, NULL, fileExt);

            if (!BeStringUtilities::Stricmp (fileExt, ".TTF"))
                {
                strcpy (pFaceName, faceName);
                return true;
                }

            // Looking ahead, are there other cases where TTF fonts do not fit any of the above two cases: "(TrueType)" faceName suffix or .TTF font filename?
            BeAssert (delim);
            }
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/06
//---------------------------------------------------------------------------------------
static bool util_getFontFaceNameFromFileName (WStringR fontFaceName, WCharCP fontFileName)
    {
    HKEY        hkFonts;
    char        faceName[MAX_PATH];
    AString     fileName;
    
    BeStringUtilities::WCharToCurrentLocaleChar (fileName, fontFileName);
    
    if (!::RegOpenKeyW (HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fonts", &hkFonts))
        {
        if (queryRegistryKey (faceName, fileName.c_str (), hkFonts, getFaceName))
            {
            ::RegCloseKey (hkFonts);

            fontFaceName.erase ();
            fontFaceName.AppendA (faceName);

            return true;
            }

        ::RegCloseKey (hkFonts);
        }

    if (!::RegOpenKeyW (HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", &hkFonts))
        {
        if (queryRegistryKey (faceName, fileName.c_str (), hkFonts, getFaceName))
            {
            ::RegCloseKey (hkFonts);

            fontFaceName.erase ();
            fontFaceName.AppendA (faceName);

            return true;
            }

        ::RegCloseKey (hkFonts);
        }
    
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeFont::GetFontFaceNameFromFileName (WStringR faceName, WCharCP fileName)
    {
    return (util_getFontFaceNameFromFileName (faceName, fileName) ? SUCCESS : ERROR);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeFont::GetPitchAndCharSet (Byte& pitch, Byte& charSet) const
    {
    // Because one DgnTrueTypeFont object represents 4 faces, assume the font metrics are uniform across all faces, and use no-bold/no-italic here.
    return Win32TTPImpl::GetPitchAndCharSet (pitch, charSet, *this, false, false);
    }

#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
bool DgnTrueTypeFont::_IsGlyphBlank (FontChar charCode) const
    {
    // Because one DgnTrueTypeFont object represents 4 faces, assume the font metrics are uniform across all faces, and use no-bold/no-italic here.
    DgnTrueTypeGlyph glyph (charCode, false, *m_data, m_scaleFactor, false, false);
    
    return glyph.IsBlank ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeFont::_LayoutGlyphs (DgnGlyphLayoutContextCR layoutContext, DgnGlyphLayoutResultR layoutResult) const
    {
    if (m_isMissing)
        return ERROR;    
    
    // Expand any escape sequences to get the display string.
    DgnGlyphLayoutContext::T_FontCharsCR    fontChars           = layoutContext.GetFontChars ();
    ScopedArray<WChar>                      processedCharsBuff  (fontChars.size ());
    WCharP                                  processedChars      = processedCharsBuff.GetData ();
    size_t                                  processedCharsCount = 0;
    
    EnsureFontIsLoaded ();

    for (CharIter iter (*this, &fontChars.front (), fontChars.size (), layoutContext.ShouldIgnorePercentEscapes ()); iter.IsValid (); iter.ToNext (), ++processedCharsCount)
        processedChars[processedCharsCount] = FixDiameterCharCode (iter.CurrCharCode ());
    
    if (0 == processedCharsCount)
        return SUCCESS;

    return m_ttPImpl->_LayoutGlyphs (*this, layoutContext, processedChars, processedCharsCount, layoutResult);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2009
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeFont::LayoutGlyphsInternal (DgnGlyphLayoutContextCR layoutContext, DgnGlyphLayoutResultR layoutResult, WCharCP processedChars, size_t numChars) const
    {
    // If you make any changes to this method, also consider examining DgnRscFont::_LayoutGlpyhs and DgnShxFont::_LayoutGlyphs.
    //  This method differs from the V8i variants in that it is designed to compute only the low-level information needed,
    //  and to serve both TextString and TextBlock through a single code path. This does mean that some extraneous information
    //  is potentially computed, but should be cheap compared to the overall layout operation.

    // Local accessors for what we need to generate.
    DRange2dR outRange = layoutResult.GetRangeR();
    DRange2dR outJustificationRange = layoutResult.GetJustificationRangeR();
    DgnGlyphLayoutResult::T_GlyphsR outGlyphs = layoutResult.GetGlyphsR();
    DgnGlyphLayoutResult::T_GlyphCodesR outGlyphCodes = layoutResult.GetGlyphCodesR();
    DgnGlyphLayoutResult::T_GlyphOriginsR outGlyphOrigins = layoutResult.GetGlyphOriginsR();

    DPoint3d penPosition = DPoint3d::FromZero();
    DPoint2d drawSize = layoutContext.GetSize();

    // Compute the advance widths.
    size_t glyphOffset = outGlyphCodes.size();
    T_DoubleVector widths;
    bvector<DPoint2d> glyphOffsets;
    bvector<bool> glyphIndexMask;

    if (SUCCESS != ComputeAdvanceWidths(processedChars, numChars, widths, glyphOffsets, glyphIndexMask, outGlyphCodes, layoutContext, DgnFontManager::IsGlyphShapingDisabled(), true))
        return ERROR;

    // We need the glyphs for two loops below, so cache them up first.
    outGlyphs.reserve(outGlyphCodes.size());
    for (size_t iGlyph = glyphOffset; iGlyph < outGlyphCodes.size(); ++iGlyph)
        outGlyphs.push_back(FindGlyphCP(outGlyphCodes[iGlyph], glyphIndexMask[iGlyph - glyphOffset], layoutContext.IsBold(), layoutContext.ShouldUseItalicTypeface()));

    // Right-justified text needs to ignore trailing blanks.
    size_t numNonBlankGlyphs = (outGlyphCodes.size() - glyphOffset);
    for (; numNonBlankGlyphs > 0; --numNonBlankGlyphs)
        {
        DgnGlyphCR glyph = *outGlyphs[numNonBlankGlyphs - 1];
        if (!glyph.IsBlank())
            break;
        }
    
    outGlyphOrigins.reserve(outGlyphCodes.size());

    // Compute origins, ranges, etc...
    for (size_t i = glyphOffset; i < outGlyphCodes.size(); ++i)
        {
        outGlyphOrigins.push_back(penPosition);

        outGlyphOrigins[i].x += (glyphOffsets[i].x * drawSize.x);
        outGlyphOrigins[i].y += (glyphOffsets[i].y * drawSize.y);

        DgnGlyphCR glyph = *outGlyphs[i - glyphOffset];

        DRange2d glyphRange;
        glyphRange.low.x = outGlyphOrigins[i].x;
        glyphRange.low.y = outGlyphOrigins[i].y;
        glyphRange.high.x = glyphRange.low.x + drawSize.x * glyph.GetCellBoxWidth();
        glyphRange.high.y = glyphRange.low.y + drawSize.y;

        // It is important to use the array overload of DRange2d::Extend; the ranges can be inverted for backwards/upside-down text, and the DRange2d overload enforces low/high semantics.
        outRange.Extend(&glyphRange.low, 2);
        
        if ((i - glyphOffset) < numNonBlankGlyphs)
            outJustificationRange.Extend(&glyphRange.low, 2);

        penPosition.x += (widths[i - glyphOffset] * drawSize.x);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
void DgnTrueTypeFont::OnHostTermination ()
    {
// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)

    T_WStringVector& localFontFilePaths = Win32TTThreadGlobals::Instance ().m_localFontFilePaths;
    for (T_WStringVector::const_iterator pathIter = localFontFilePaths.begin (); localFontFilePaths.end () != pathIter; ++pathIter)
        ::RemoveFontResourceExW (pathIter->c_str (), FR_PRIVATE, 0);
    
    localFontFilePaths.clear ();

#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
DgnTrueTypeGlyphCP DgnTrueTypeFont::FindGlyphCP(FontChar code, bool isGlyphIndex, bool isBold, bool isItalic) const
    {
    T_GlyphMap* glyphMap = (isGlyphIndex ? &m_glyphIndexGlyphMap : &m_glyphCodeGlyphMap);
    auto foundGlyph = glyphMap->find(code);
    if (glyphMap->end() != foundGlyph)
        return foundGlyph->second;
    
    auto glyph = new DgnTrueTypeGlyph(code, isGlyphIndex, *m_data, m_scaleFactor, isBold, isItalic);
    glyphMap->Insert(code, glyph);
    
    return glyph;
    }
