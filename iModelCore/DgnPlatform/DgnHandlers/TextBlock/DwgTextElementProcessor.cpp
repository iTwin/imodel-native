/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/DwgTextElementProcessor.cpp $
|
|   $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnTrueTypeFont.h>

// Most of the code in this file has been adapted from Paul's original TextProcessor code.

USING_NAMESPACE_BENTLEY_DGNPLATFORM

static const WCharCP  DWG_START_OVERLINE                          =   L"\\O";
static const WCharCP  DWG_END_OVERLINE                            =   L"\\o";
static const WCharCP  DWG_START_UNDERLINE                         =   L"\\L";
static const WCharCP  DWG_END_UNDERLINE                           =   L"\\l";
static const WCharCP  DWG_COLOR                                   =   L"\\C";
static const WCharCP  DWG_TRUECOLOR                               =   L"\\c";
static const WCharCP  DWG_STACKEDFRACTIONALIGN                    =   L"\\A";
static const WCharCP  DWG_LINEALIGNMENT                           =   L"\\A";
#if defined (BENTLEY_WIN32)
static const WCharCP  DWG_ITALICANGLE                             =   L"\\Q";
#endif
static const WCharCP  DWG_INTERCHARACTERSPACING                   =   L"\\T";
#if defined (BENTLEY_WIN32)
static const WCharCP  DWG_TRUETYPEFONT                            =   L"\\f";
static const WCharCP  DWG_SHXFONT                                 =   L"\\F";
#endif
static const WCharCP  DWG_STACKEDFRACTION                         =   L"\\S";
// static const WCharCP  DWG_PARAGRAPH                               =   L"\\P";
static const WCharCP  DWG_SEMICOLON                               =   L";";
// static const WCharCP  DWG_STYLESECTION_CLOSE                      =   L"}";
// static const WCharCP  DWG_STYLESECTION_OPEN                       =   L"{";
static const WCharCP  DWG_SUPERSCRIPT_OPEN                        =   L"{\\H0.3x;\\A2;\\S";
static const WCharCP  DWG_SUPERSCRIPT_CLOSE                       =   L"^;}";
static const WCharCP  DWG_SUBSCRIPT_OPEN                          =   L"{\\H0.3x;\\A0;\\S^";
static const WCharCP  DWG_SUBSCRIPT_CLOSE                         =   L";}";
// static const WCharCP  DWG_HEIGHT                                  =   L"\\H%lfx;";
// static const WCharCP  DWG_WIDTH                                   =   L"\\W%lf;";

static const double     DWG_TRUETYPE_INTERCHARACTERSPACING_FACTOR   =   0.615;

// static const int        DWG_PERCENTCHARSLENGTH                      =   2;

static const double     TRUETYPE_UNDERLINEOFFSET                    =   0.296;
static const double     SHX_UNDERLINEOFFSET                         =   0.2;
static const double     TRUETYPE_OVERLINEOFFSET                     =   0.296;
static const double     SHX_OVERLINEOFFSET                          =   0.2;

static const double     DWG_CHARACTERSPACING_LOWER_LIMIT            =   0.75;
static const double     DWG_CHARACTERSPACING_HIGHER_LIMIT           =   4.00;
static const double     DWG_CHARACTERSPACING_DEFAULT                =   1.00;
static const double     DWG_CHARACTERSPACING_COMPARISON_TOLERANCE   =   1E-05;

static const double     DWG_WIDTHHEIGHTCHANGE_TOLERANCE             =   0.01;

static const double     DWG_VERTICAL_LINESPACING_FACTOR             =   1.0 / 1.2;

#define ISEQUAL(x,y)                                    (fabs (x-y) < mgds_fc_epsilon)

#define IS_DWG_DEGREE(charCode)                         (charCode == 'D' || charCode == 'd')
#define IS_DWG_DIAMETER(charCode)                       (charCode == 'C' || charCode == 'c')
#define IS_DWG_PLUSMINUS(charCode)                      (charCode == 'P' || charCode == 'p')
#define IS_DIGIT(charCode)                              (charCode >= '0' && charCode <= '9')
#define IS_DECIMALPOINT(charCode)                       (charCode == '.')
#define IS_PERCENT(charCode)                            (charCode == '%')
#define IS_DWG_SYMBOL(charCode)                         (IS_DWG_DEGREE(charCode) || IS_DWG_DIAMETER(charCode) || IS_DWG_PLUSMINUS(charCode))

#define IS_CHARCODE_SINGLEBYTE(x)                       (x < 0xFF)
#define IS_CHARCODE_ASCII(x)                            (x < 0x80)

#define HAS_HEIGHT_CHANGED(currentH, newH)              (fabs (fabs(newH) - fabs(currentH)) > DWG_WIDTHHEIGHTCHANGE_TOLERANCE * currentH)
#define HAS_WIDTH_CHANGED(currentW, newW)               (fabs (fabs(newW) - fabs(currentW)) > DWG_WIDTHHEIGHTCHANGE_TOLERANCE * currentW)
#define IS_HEIGHT_DIFFERENT_FROM_WIDTH(height, width)   (fabs (fabs(height) - fabs(width)) > fabs(height) * DWG_WIDTHHEIGHTCHANGE_TOLERANCE)

//=======================================================================================
// @bsiclass                                                    Venkat.Kalyan   06/07
//=======================================================================================
class DwgMTextImporter
{
private:
    TextBlockR                  m_textBlock;
    WString                     m_markup;
    WString::const_iterator     m_markupIter;
    std::stack<RunProperties>   m_propertiesStack;
    TextBlockNodeArray          m_unitArray;
    WString                     m_currentStream;
    IDwgContextCP               m_dwgContext;
    double                      m_scaleToDGN;
    double                      m_indentationScaleFactor;

public:
    
    DwgMTextImporter (TextBlockR textBlock, WCharCP markupString, IDwgContextCR dwgContext, double scaleToDGN) :
        m_textBlock                 (textBlock),
        m_markup                    (markupString),
        m_dwgContext                (&dwgContext),
        m_scaleToDGN                (scaleToDGN),
        m_indentationScaleFactor    (scaleToDGN)
        {
        m_propertiesStack.push (textBlock.GetRunPropertiesForAdd ());
        m_markupIter = m_markup.begin ();
        }

    void                ProcessMarkup                   ();
#ifdef BEIJING_DGNPLATFORM_WIP_Fields
    bool                ProcessField                    ();
#endif // BEIJING_DGNPLATFORM_WIP_Fields
    void                ProcessBackslash                ();
    void                DumpStream                      ();
    void                DumpUnitArray                   ();
    BentleyStatus       ExtractDoubleValueFromMarkup    (double& value);
    BentleyStatus       ExtractIntegerValueFromMarkup   (int& value);
    void                ProcessIndentation              ();
    void                ProcessStackedFraction          ();
    void                ProcessStyleChange              ();
    RunPropertiesR      GetProperties                   ()                                                          { return m_propertiesStack.top (); }

    IDwgContextCP       GetDwgContext                   () const                                                    { return m_dwgContext; }
    double              GetScaleToDGN                   () const                                                    { return m_scaleToDGN; }

    void                GetFontFromMTextFontString      (RunPropertiesR, WStringR mtextFontMarkup, bool file);
    bool                GetFontDataFromMarkup           (RunPropertiesR, WStringR fontName, int& pitch);
    DgnModelP        GetDgnModel                     () const                                                    { return &m_textBlock.GetDgnModelR (); }
    DgnProjectR GetDgnProject() const { return GetDgnModel()->GetDgnProject(); }
    
    bool                ProcessFontMarkup               (RunPropertiesR, bool file);
    void                SetProperties                   (RunPropertiesCR runProps)                                  { m_propertiesStack.pop (); m_propertiesStack.push (runProps); }

}; // DwgMTextImporter

//=======================================================================================
// @bsiclass                                                    Venkat.Kalyan   06/07
//=======================================================================================
#if defined (_WIN32) || !defined (ANDROID) // WIP_NONPORT - stringstream not supported on Android
struct DwgExporter
    {
    private:    DgnModelP    m_modelRef;
    protected:  wostringstream  m_markupStream;
    protected:  TextBlockCR     m_textBlock;
    protected:  bool            m_shouldExportEdfsAsUnderscores;
    
    protected: DwgExporter (TextBlockCR textBlock, DgnModelP modelRef, bool shouldExportEdfsAsUnderscores) :
        m_textBlock                     (textBlock),
        m_modelRef                      (modelRef),
        m_shouldExportEdfsAsUnderscores (shouldExportEdfsAsUnderscores)
        {
        }

    protected:  void            AddCharToString         (WChar);
    protected:  bool            AddCharStreamToString   (CharStreamCR charStream);
    public:     void            GetMarkup               (WString& markup)                    { markup.assign (m_markupStream.str ().c_str ()); }
    public:     DgnModelP    GetDgnModel             () const                                { return m_modelRef; }

    }; // DwgExporter

//=======================================================================================
// @bsiclass                                                    Venkat.Kalyan   06/07
//=======================================================================================
struct DwgDTextExporter : public DwgExporter
    {
    public: DwgDTextExporter (TextBlockCR textBlock, DgnModelP modelRef, bool shouldExportEdfsAsUnderscores) :
        DwgExporter (textBlock, modelRef, shouldExportEdfsAsUnderscores)
        {
        }

    public: void Export ();

    }; // DwgDTextExporter

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void DwgExporter::AddCharToString (WChar charCode)
    {
    m_markupStream << charCode;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
bool DwgExporter::AddCharStreamToString (CharStreamCR charStream)
    {
    bool    allSpaces   = charStream.ContainsOnlyWhitespace ();
    WString charArray   = charStream.GetString ();

    if (m_shouldExportEdfsAsUnderscores)
        {
        EdfCharStreamCP edf = dynamic_cast<EdfCharStreamCP>(&charStream);
        if (NULL != edf)
            {
            for (size_t iChar = 0; iChar < edf->GetString ().length (); ++iChar)
                {
                if (iswspace (charArray[iChar]))
                    charArray[iChar] = L'_';
                }
            
            allSpaces = false;
            }
        }
    
    for (size_t i = 0; i < charArray.size (); ++i)
        AddCharToString (charArray[i]);

    return allSpaces;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void DwgDTextExporter::Export ()
    {
    bool            allSpaces   = true;
    bool            overline    = false;
    bool            underline   = false;
    
    RunRange runRange (m_textBlock);
    FOR_EACH (RunCR run, runRange)
        {
        CharStreamCP charStream = dynamic_cast<CharStreamCP>(&run);
        BeAssert (NULL != charStream);

        RunPropertiesCR runProperties = run.GetProperties ();
        if (runProperties.IsUnderlined () != underline)
            m_markupStream << ((underline = runProperties.IsUnderlined ()) ? L"%%u" : L"%%U");

        if (runProperties.IsOverlined () != overline)
            m_markupStream << ((overline = runProperties.IsOverlined ()) ? L"%%o" : L"%%O");

        allSpaces = allSpaces && AddCharStreamToString (*charStream);
        }

    if (underline)
        m_markupStream << L"%%U";

    if (overline)
        m_markupStream << L"%%O";

    if (allSpaces)
        m_markupStream.flush ();
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void TextBlock::ToDText (WStringR markup, DgnModelP modelRef, bool shouldExportEdfsAsUnderscores) const
    {
#if defined (_WIN32) || !defined (ANDROID) // WIP_NONPORT - stringstream not supported on Android
    DwgDTextExporter dwgDTextExporter (*this, modelRef, shouldExportEdfsAsUnderscores);
    dwgDTextExporter.Export ();
    dwgDTextExporter.GetMarkup (markup);
#endif
    }

//=======================================================================================
// @bsiclass                                                    Venkat.Kalyan   06/07
//=======================================================================================
#if defined (_WIN32) || !defined (ANDROID) // WIP_NONPORT - stringstream not supported on Android
struct DwgMTextExporter : public DwgExporter
    {
    private:    ParagraphCP     m_paragraph;
    private:    RunProperties   m_properties;
#ifdef BEIJING_DGNPLATFORM_WIP_Fields
    private:    bool            m_expandFields;
#endif // BEIJING_DGNPLATFORM_WIP_Fields
    private:    Caret           m_caret;
    private:    IDwgContextCP   m_dwgContext;
    private:    double          m_scaleToDGN;

    public: DwgMTextExporter (TextBlockCR textBlock, DgnModelP modelRef, IDwgContextCP dwgContext, double scaleToDGN/*, bool expandFields*/, bool shouldExportEdfsAsUnderscores) :
        DwgExporter (textBlock, modelRef, shouldExportEdfsAsUnderscores),
#ifdef BEIJING_DGNPLATFORM_WIP_Fields
        m_expandFields  (expandFields),
#endif // BEIJING_DGNPLATFORM_WIP_Fields
        m_caret         (textBlock.End ()),
        m_dwgContext    (dwgContext),
        m_scaleToDGN    (scaleToDGN),
        m_properties    (*modelRef)
        {
        }

    public: void            GetReferenceStyle               ();
    public: void            Export                          ();
    public: void            AppendIndentationMarkup         (IndentationDataCP previousIndentation, IndentationDataCR newIndentation);
    public: void            ExportParagraph                 (CaretR caret, bool& firstChar);
    public: void            ExportSectionClosing            ();
    public: void            ExportTab                       ();
    public: void            ExportLineBreak                 ();
    public: void            ExportParagraphBreak            ();
    public: void            ExportXMLPrefix                 (WCharCP tag);
    public: void            ExportXMLSuffix                 (WCharCP tag);
    public: bool            ExportStyleChangeMarkup         (RunPropertiesCR newRunProperties, bool firstChar);
    public: bool            ExportAlignmentMarkup           (RunPropertiesCR runProperties);
    public: bool            ExportSubscriptMarkup           (RunPropertiesCR runProperties);
    public: bool            ExportHeightWidthChangedMarkup  (RunPropertiesCR runProperties, bool firstUnit);
    public: bool            ExportColorMarkup               (RunPropertiesCR runProperties);
    public: bool            ExportFontMarkup                (RunPropertiesCR runProperties);
    public: void            ExportStackedFraction           (FractionCP fraction, bool firstChar);
    public: bool            ExportUnderlineMarkup           (RunPropertiesCR runProperties);
    public: bool            ExportOverlineMarkup            (RunPropertiesCR runProperties);
    public: bool            ExportCharacterSpacingMarkup    (RunPropertiesCR runProperties);
    public: bool            ExportSuperscriptMarkup         (RunPropertiesCR runProperties);
    public: void            ExportCharStream                (CharStreamCP charStream, bool firstChar, bool inFraction);
    public: void            ExportSectionStart              ();
    public: void            ExportSectionEnd                ();
    public: void            ExportRun                       (RunCP run, bool firstChar, bool inFraction);
#ifdef BEIJING_DGNPLATFORM_WIP_Fields
    public: void            ExportField                     (RunCP run, bool firstChar, bool inFraction);
    public: FieldCP         GetField                        (RunCP run) const;
#endif // BEIJING_DGNPLATFORM_WIP_Fields
    public: IDwgContextCP   GetDwgContext                   () const    { return m_dwgContext; }
    public: double          GetScaleToDGN                   () const    { return m_scaleToDGN; }

    }; // DwgMTextExporter

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
bool DwgMTextExporter::ExportFontMarkup (RunPropertiesCR runProperties)
    {
// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)

    DgnFontCR   font            = runProperties.GetFont ();
    bool        isRscOrShxFont  = (DgnFontType::Rsc == font.GetType () || DgnFontType::Shx == font.GetType ());
    WString     fontName        (font.GetName ().c_str (), BentleyCharEncoding::Utf8);

    if (isRscOrShxFont)
        {
        m_markupStream << DWG_SHXFONT << fontName.c_str () << DWG_SEMICOLON;

        if (runProperties.IsItalic () != m_properties.IsItalic () || (runProperties.IsItalic () && m_properties.GetCustomSlantAngle () != runProperties.GetCustomSlantAngle ()))
            m_markupStream << DWG_ITALICANGLE << runProperties.GetCustomSlantAngle () * msGeomConst_degreesPerRadian << DWG_SEMICOLON;

        return true;
        }

    byte charSet;
    byte pitch;
    
    static_cast<DgnTrueTypeFontCP>(font.ResolveToRenderFont())->GetPitchAndCharSet (pitch, charSet);

    m_markupStream << DWG_TRUETYPEFONT << fontName << (runProperties.IsBold () ? L"|b1" : L"|b0");
    m_markupStream << (runProperties.IsItalic () ? L"|i1" : L"|i0");
    m_markupStream << L"|c" << charSet << charSet;
    m_markupStream << L"|p" << pitch << DWG_SEMICOLON;
    
    return true;

#else
    
    BeAssert (false && L"Not supported on non-Win32.");
    return false;

#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
bool DwgMTextExporter::ExportUnderlineMarkup (RunPropertiesCR runProperties)
    {
    m_markupStream << (runProperties.IsUnderlined () ? DWG_START_UNDERLINE : DWG_END_UNDERLINE);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
bool DwgMTextExporter::ExportOverlineMarkup (RunPropertiesCR runProperties)
    {
    m_markupStream << (runProperties.IsOverlined () ? DWG_START_OVERLINE : DWG_END_OVERLINE);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
bool DwgMTextExporter::ExportColorMarkup (RunPropertiesCR runProperties)
    {
    IntColorDef colorDef;
    bool        isTrueColor;
    if (SUCCESS == GetDgnModel()->GetDgnProject().Colors().Extract (&colorDef, NULL, &isTrueColor, NULL, NULL, runProperties.GetColor()) && isTrueColor)
        {
        UInt32  value       = colorDef.m_int;
        byte    smallColor  = (byte)runProperties.GetColor ();
        
        m_markupStream << DWG_COLOR << GetDwgContext ()->DgnColorToDwgColor (smallColor) << DWG_SEMICOLON << DWG_TRUECOLOR << value << DWG_SEMICOLON;
        }
    else
        {
        // Do not down cast dgnColorIndex which may be an extended color, like the remapped DWG color 255.
        m_markupStream << DWG_COLOR << GetDwgContext ()->DgnColorToDwgColor (runProperties.GetColor()) << DWG_SEMICOLON;
        }
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
bool DwgMTextExporter::ExportCharacterSpacingMarkup (RunPropertiesCR runProperties)
    {
    double  value   = DWG_CHARACTERSPACING_DEFAULT;
    DgnFontCR  font    = runProperties.GetFont ();
    
    if (CharacterSpacingType::Factor == runProperties.GetCharacterSpacingType ())
        {
        value = runProperties.GetCharacterSpacingValue ();
        }
    else
        {
        // The spacing calculation formulas are different for TT and SHX fonts in ACAD.
        //  SHX fonts use a type of fixed width spacing while TT fonts use a different
        //  kind of incremental spacing. Accordingly, we have three cases:
        //  
        //      a.  TT font and non-fixed width spacing
        //      b.  SHX font and fixed width spacing
        //      c.  All other scenarios.
        
        if (DgnFontType::TrueType == font.GetType () && (CharacterSpacingType::FixedWidth != runProperties.GetCharacterSpacingType ()))
            {
            // Use empirically determined formula.
            value = runProperties.GetCharacterSpacingValue () / (runProperties.GetFontSize ().y * DWG_TRUETYPE_INTERCHARACTERSPACING_FACTOR) + 1.0;
            }
        else if (CharacterSpacingType::FixedWidth == runProperties.GetCharacterSpacingType ())
            {
            // In the case of fixed width spacing for SHX/RSC fonts, simply take the ratio of the character spacing value to
            //  the height to get the value ACAD intercharspacing factor.
            value = runProperties.GetCharacterSpacingValue () / runProperties.GetFontSize ().y;
            }
        else if (fabs(runProperties.GetCharacterSpacingValue ()) > DWG_CHARACTERSPACING_COMPARISON_TOLERANCE)
            {
            // We should preprocess textblock to change the character spacing; we cannot handle it here.
            BeAssert (false);
            }
        }

    // Apply autocad limits to inter char spacing.
    if (value > DWG_CHARACTERSPACING_HIGHER_LIMIT)
        value = DWG_CHARACTERSPACING_HIGHER_LIMIT;
    else if (value < DWG_CHARACTERSPACING_LOWER_LIMIT)
        value = DWG_CHARACTERSPACING_LOWER_LIMIT;

    WChar buffer[128];
    BeStringUtilities::Snwprintf (buffer, _countof(buffer), L"%ls%.3lf%ls", DWG_INTERCHARACTERSPACING, value, DWG_SEMICOLON);
    
    m_markupStream << buffer;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
bool DwgMTextExporter::ExportHeightWidthChangedMarkup (RunPropertiesCR runProperties, bool firstUnit)
    {
    double  oldWidthFactor  = (m_properties.GetFontSize ().x / m_properties.GetFontSize ().y);
    double  newWidthFactor  = (runProperties.GetFontSize ().x / runProperties.GetFontSize ().y);
    
    m_markupStream << L"\\H" << runProperties.GetFontSize ().y / m_properties.GetFontSize ().y << L"x;";
    
    if ((firstUnit && !ISEQUAL (newWidthFactor, 1.0)) || (!ISEQUAL (oldWidthFactor, newWidthFactor)))
        m_markupStream << L"\\W" << newWidthFactor << L";";
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
bool DwgMTextExporter::ExportSuperscriptMarkup (RunPropertiesCR runProperties)
    {
    m_markupStream << (runProperties.IsSuperScript () ? DWG_SUPERSCRIPT_OPEN : DWG_SUPERSCRIPT_CLOSE);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
bool DwgMTextExporter::ExportSubscriptMarkup (RunPropertiesCR runProperties)
    {
    m_markupStream << (runProperties.IsSubScript () ? DWG_SUBSCRIPT_OPEN : DWG_SUBSCRIPT_CLOSE);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/05
//---------------------------------------------------------------------------------------
bool DwgMTextExporter::ExportAlignmentMarkup (RunPropertiesCR runProperties)
    {
    m_markupStream << DWG_LINEALIGNMENT << runProperties.GetLineAlignment () << DWG_SEMICOLON;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
bool DwgMTextExporter::ExportStyleChangeMarkup (RunPropertiesCR newRunProperties, bool firstChar)
    {
    bool styleChanged = false;

    if (m_properties.IsSuperScript () != newRunProperties.IsSuperScript () && !newRunProperties.IsSuperScript ())
        styleChanged = ExportSuperscriptMarkup (newRunProperties);

    if (m_properties.IsSubScript () != newRunProperties.IsSubScript () && !newRunProperties.IsSubScript ())
        styleChanged = ExportSubscriptMarkup (newRunProperties);

    if ((&m_properties.GetFont () != &newRunProperties.GetFont ())
        || (m_properties.IsItalic () != newRunProperties.IsItalic ())
        || (m_properties.IsBold () != newRunProperties.IsBold ()))
        {
        styleChanged = ExportFontMarkup (newRunProperties);
        }

    if ((HAS_HEIGHT_CHANGED (newRunProperties.GetFontSize ().y, m_properties.GetFontSize ().y)
        || HAS_WIDTH_CHANGED (newRunProperties.GetFontSize ().x, m_properties.GetFontSize ().x))
        || (firstChar && IS_HEIGHT_DIFFERENT_FROM_WIDTH (newRunProperties.GetFontSize ().x, newRunProperties.GetFontSize ().y)))
        {
        styleChanged = ExportHeightWidthChangedMarkup (newRunProperties, firstChar);
        }

    if (m_properties.IsUnderlined () != newRunProperties.IsUnderlined ())
        styleChanged = ExportUnderlineMarkup (newRunProperties);

    if (m_properties.IsOverlined () != newRunProperties.IsOverlined ())
        styleChanged = ExportOverlineMarkup (newRunProperties);

    if (m_properties.GetColor () != newRunProperties.GetColor ())
        styleChanged = ExportColorMarkup (newRunProperties);

    if (m_properties.GetCharacterSpacingValue () != newRunProperties.GetCharacterSpacingValue ())
        styleChanged = ExportCharacterSpacingMarkup (newRunProperties);

    if (m_properties.GetLineAlignment () != newRunProperties.GetLineAlignment ())
        styleChanged = ExportAlignmentMarkup (newRunProperties);

    if (m_properties.IsSuperScript () != newRunProperties.IsSuperScript () && newRunProperties.IsSuperScript ())
        styleChanged = ExportSuperscriptMarkup (newRunProperties);

    if (m_properties.IsSubScript () != newRunProperties.IsSubScript () && newRunProperties.IsSubScript ())
        styleChanged = ExportSubscriptMarkup (newRunProperties);

    if (styleChanged)
        m_properties = newRunProperties;

    return styleChanged;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void DwgMTextExporter::ExportCharStream (CharStreamCP charStream, bool firstChar, bool inFraction)
    {
    if (!inFraction)
        ExportStyleChangeMarkup (charStream->GetProperties (), firstChar);

    WString charArray = charStream->GetString ();

    if (m_shouldExportEdfsAsUnderscores)
        {
        EdfCharStreamCP edf = dynamic_cast<EdfCharStreamCP>(charStream);
        if (NULL != edf)
            {
            for (size_t iChar = 0; iChar < edf->GetString ().length (); ++iChar)
                {
                if (iswspace (charArray[iChar]))
                    charArray[iChar] = L'_';
                }
            }
        }
    
    for (size_t i = 0; i < charArray.size (); ++i)
        {
        WChar charCode = charArray[i];
        
        switch (charCode)
            {
            case L'/':
            case L'#':
            case L'^':
                if (inFraction)
                    m_markupStream << L"\\";
                
                m_markupStream << (WChar) charCode;
                break;

            case 0x5c:
                m_markupStream << L"\\\\";
                break;

            case L'{':
            case L'}':
                m_markupStream << L"\\";
                m_markupStream << (WChar) charCode;
                break;

            case L' ':
                m_markupStream << (WChar) charCode;
                break;

            case 0x00A0: // non-breaking space
                m_markupStream << (WChar) 0x00A0;
                break;

            case L'%':
                if (i + 2 < charArray.size() && charArray[i+1] == '%' && !IS_DWG_SYMBOL(charArray[i+2]))
                    charCode = charArray[i+=2];
                
                // fall through to add a % or whatever follows by %% as a regular character...

            default:
                AddCharToString (charCode);
                break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/05
//---------------------------------------------------------------------------------------
void DwgMTextExporter::ExportSectionStart ()
    {
    m_markupStream << L"{";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/05
//---------------------------------------------------------------------------------------
void DwgMTextExporter::ExportSectionEnd ()
    {
    m_markupStream << L"}";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void DwgMTextExporter::ExportStackedFraction (FractionCP fraction, bool firstChar)
    {
    RunProperties runProperties = m_properties;

    ExportSectionStart ();

    RunProperties   fractionProperties  = fraction->GetProperties ();
    DPoint2d        scaledFontSize      = fractionProperties.GetFontSize ();
    
    scaledFontSize.Scale (scaledFontSize, fraction->GetTextScale ().y);
    
    fractionProperties.SetFontSize (scaledFontSize);
    
    // handle fraction divider vs. underlines:
    if (fraction->IsUnderlined())
        {
        // fix underline flag in fraction properties to make sure we get an underline:
        if (!m_properties.IsUnderlined () || !fractionProperties.IsUnderlined())
            fractionProperties.SetIsUnderlined (true);
        }
    else
        {
        // do not set underline for entire fraction. A divider will be added as needed:
        fractionProperties.SetIsUnderlined (false);
        }

    ExportStyleChangeMarkup (fractionProperties, firstChar);

    if ((int)fraction->GetAlignment () != (int)fraction->GetProperties ().GetLineAlignment ())
        {
        BeAssert (false);
        m_markupStream << DWG_STACKEDFRACTIONALIGN << static_cast<UInt32>(fraction->GetAlignment ()) << DWG_SEMICOLON;
        }

    m_markupStream << DWG_STACKEDFRACTION;
    ExportRun (fraction->GetNumerator (), firstChar, true);
    
    m_markupStream << fraction->GetSeparatorChar ();
    ExportRun (fraction->GetDenominator (), firstChar, true);
    
    m_markupStream << DWG_SEMICOLON;
    
    ExportSectionEnd ();
    
    m_properties = runProperties;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void DwgMTextExporter::ExportParagraphBreak ()
    {
    m_markupStream << L"\\P";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     03/05
//---------------------------------------------------------------------------------------
void DwgMTextExporter::ExportXMLPrefix (WCharCP tag)
    {
    m_markupStream << L"<" << tag << L">";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     03/05
//---------------------------------------------------------------------------------------
void DwgMTextExporter::ExportXMLSuffix (WCharCP tag)
    {
    m_markupStream << L"</" << tag << L">";
    }

#ifdef BEIJING_DGNPLATFORM_WIP_Fields
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     03/05
//---------------------------------------------------------------------------------------
void DwgMTextExporter::ExportField (RunCP run, bool firstChar, bool inFraction)
    {
    FieldCP     field               = GetField (run);
    WCharCP   fieldTag            = L"Field";
    WCharCP   specificationTag    = L"Specification";
    WCharCP   resultTag           = L"Result";

    m_expandFields = false;

    Bentley::WString dwgField;
    field->GetDWGSpecification (dwgField, GetDgnModel ());
    
    if (0 != dwgField.size ())
        {
        ExportXMLPrefix (fieldTag);
        ExportXMLPrefix (specificationTag);

        m_markupStream << dwgField.c_str ();

        ExportXMLSuffix (specificationTag);
        ExportXMLPrefix (resultTag);

        Caret   caret   (m_caret);
        RunCP   run     = caret.GetCurrentRunCP ();
        
        do
            {
            ExportRun (run, firstChar, inFraction);
            
            if (SUCCESS != caret.MoveToNextRun ())
                break;
            
            run = caret.GetCurrentRunCP ();
            if (m_paragraph != caret.GetCurrentParagraphCP () || !run->IsPartOfSpan (field))
                break;
            
            m_caret = caret;
            } while (true);

        ExportXMLSuffix (resultTag);
        ExportXMLSuffix (fieldTag);
        
        m_expandFields = true;
        
        return;
        }

    // We won't make a field out of this - just export the chars (with markup).
    while (NULL != (run = m_caret.GetCurrentRunCP ()) && run->IsPartOfSpan (field))
        {
        ExportRun (run, firstChar, inFraction);
        
        if (SUCCESS != m_caret.MoveToNextRun () || m_paragraph != m_caret.GetCurrentParagraphCP ())
            break;
        }

    // Our parent loop will advance m_caret; we must ensure we leave it in a condition
    //  that is safe to advance to the next run that's supposed to be exported.
    m_caret.MoveToPreviousRun ();

    m_expandFields = true;
    }
#endif // BEIJING_DGNPLATFORM_WIP_Fields

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
void DwgMTextExporter::ExportTab ()
    {
    m_markupStream << L"\t";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
void DwgMTextExporter::ExportLineBreak ()
    {
    m_markupStream << L"\n";
    }

#ifdef BEIJING_DGNPLATFORM_WIP_Fields
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
FieldCP DwgMTextExporter::GetField (RunCP run) const
    {
    return (FieldCP)run->FindSpanIf (TextBlockUtilities::IsField);
    }
#endif // BEIJING_DGNPLATFORM_WIP_Fields

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void DwgMTextExporter::ExportRun (RunCP run, bool firstChar, bool inFraction)
    {
    if (NULL == run)
        return;

    if (NULL != dynamic_cast <FractionCP> (run))
        ExportStackedFraction (static_cast <FractionCP> (run), firstChar);
#ifdef BEIJING_DGNPLATFORM_WIP_Fields
    else if (m_expandFields && NULL != GetField (run))
        ExportField (run, firstChar, inFraction);
#endif // BEIJING_DGNPLATFORM_WIP_Fields
    else if (NULL != dynamic_cast <CharStreamCP> (run))
        ExportCharStream (static_cast <CharStreamCP> (run), firstChar, inFraction);
    else if (NULL != dynamic_cast <ParagraphBreakCP> (run))
        ExportParagraphBreak ();
    else if (NULL != dynamic_cast <TabCP> (run))
        ExportTab ();
    else if (NULL != dynamic_cast <LineBreakCP> (run))
        ExportLineBreak ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
void DwgMTextExporter::ExportSectionClosing ()
    {
    // TEST CASES: dwgexportsupersubscript.dgn
    if (m_properties.IsSuperScript ())
        {
        m_markupStream << DWG_SUPERSCRIPT_CLOSE;
        m_properties.SetIsSuperScript (false);
        }

    if (m_properties.IsSubScript ())
        {
        m_markupStream << DWG_SUBSCRIPT_CLOSE;
        m_properties.SetIsSubScript (false);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
void DwgMTextExporter::ExportParagraph (CaretR caret, bool& firstChar)
    {
    ParagraphCP paragraph = caret.GetCurrentParagraphCP ();
    if (!paragraph)
        { BeAssert (false); return; }
    
    ParagraphIterator   paragraphIter       = caret.CreateParagraphBegin ();
    LineCP              lastProcessedLine   = caret.GetCurrentLineCP ();
    
    for (RunIterator runIter = paragraphIter.CreateRunBegin (); paragraphIter.CreateRunEnd () != runIter; ++runIter)
        {
        ExportRun (&*runIter, firstChar, false);
        
        firstChar = false;
        
        LineCP currentLine = m_caret.GetCurrentLineCP ();
        
        if (currentLine != lastProcessedLine)
            {
            // If the line break is due to max units per line setting, insert a line feed to intentionally break the line. This is to match displays. Whatever we do this is going to be a loser. Also, right now I am ignoring the case where there is a line break due to exceedng the max number of units but word wrap length > 0. That is harder to determine - NEEDS_WORK
            if ((0.0 == m_textBlock.GetProperties ().GetDocumentWidth ())
                && !currentLine->EndsInParagraphBreak ()
                && !currentLine->EndsInLineBreak ()
                && (currentLine != m_textBlock.End ().GetCurrentLineCP ())
                && !currentLine->IsBlankLine ())
                {
                m_markupStream << L"\n";
                }
            
            lastProcessedLine = currentLine;
            }
        }
    
    if (SUCCESS != caret.MoveToNextParagraph ())
        caret = m_textBlock.End ();
    
    // MAY_NEED_ENHANCEMENT: At this point, we need to close down current styles such as subscript, superscript and fractions. We may be in the middle or a markup or not but AutoCAD will not accept a \P within a flowing markup such as \A.  So, append any closing stuff and reset the styling information.  More stuff may be needed here but for now I am adding stacked fractions, subscripts and superscripts.
    this->ExportSectionClosing ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void DwgMTextExporter::GetReferenceStyle ()
    {
    m_properties = *m_textBlock.GetFirstRunProperties ();

    // The MicroStation text style structure contains almost all the parameters that
    //  a text element has.  However, all these cannot go into AutoCAD's text style.
    //  There are four places where we can put text attributes on an AutoCAD text
    //  entity.  The text style, the entity attributes, the text attributes and
    //  finally as markup.  The list of attributes and the places they end up in are
    //  as follows:
    //  ACAD Text Style:
    //      1. Font Name - initial font
    //      2. Height - initial height
    //      3. Upside down - cannot be changed within the text element
    //      4. Backwards - cannot be changed within element
    //      5. Vertical - cannot be changed within element
    //      6. width factor - initial width factor
    //      7. oblique angle - initial oblique angle
    //  ACAD Entity:
    //      1. Color - initial color
    //      2. Layer - not changeable
    //      3. Linetype - not changeable
    //      4. Linetype Scale - not changeable
    //      5. Lineweight - not changeable
    //      6. Thickness - not changeable
    //  Text Entity:
    //      1. Contents (markup string)
    //      2. Style - cannot be changed
    //      3. Justification - cannot be changed
    //      4. Direction (vertical/horizontal)- cannot be changed
    //      5. Width - total width (read only property) (fit width)
    //      6. Height - Initial Height
    //      7. Rotation - cannot be changed
    //      8. Background Mask (cannot be changed)
    //      9. Line Spacing Factor (cannot be changed)
    //      10. Line spacing distance/offset (cannot be changed)
    //      11. Line spacing style
    //      12. Location
    //  Markup:
    //      1. Overline/underline
    //      2. S - stacked fraction (super/sub scripts)
    //      3. T - character spacing
    //      4. A - fraction alignment
    //      5. Style parameters such as bold and italics
    
    if (m_properties.HasTextStyle())
        {
        DgnTextStylePtr resolvedTextStyle = m_properties.GetTextStyleInFile();
        if (resolvedTextStyle.IsValid ())
            m_properties.ApplyTextStyle(*resolvedTextStyle, false);
        }
    
    m_properties.SetIsUnderlined(false);
    m_properties.SetIsOverlined(false);
    m_properties.SetCharacterSpacingValue(0.0);
    m_properties.SetIsBold(false);
    m_properties.SetLineAlignment (LINE_ALIGNMENT_Bottom);

    // NEEDS_WORK: There is a corresponding code in OdtMText. This code has to correspond to that. In general, I would like to set the node
    //  number height. But that causes problems. Revisit later.

    DgnLineSpacingType lineSpacingType = m_textBlock.GetParagraph (0)->GetLineSpacingType ();
    if  (lineSpacingType != DgnLineSpacingType::Automatic
        && lineSpacingType != DgnLineSpacingType::Exact
        && 0.0 != m_textBlock.GetNodeOrFirstRunHeight ())
        {
        DPoint2d fontSize = m_properties.GetFontSize ();
        fontSize.x = fontSize.y = m_textBlock.GetNodeOrFirstRunHeight ();
        m_properties.SetFontSize (fontSize);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
void DwgMTextExporter::AppendIndentationMarkup (IndentationDataCP previousIndentation, IndentationDataCR newIndentation)
    {
    if ((NULL == previousIndentation && newIndentation.IsDefault ()))
        return;

    if (NULL != previousIndentation && previousIndentation->Equals (newIndentation))
        return;

    m_markupStream << L"\\p";

    double  oldRelativeIndentation  = ((NULL == previousIndentation) ? 0.0 : previousIndentation->GetFirstLineIndent () - previousIndentation->GetHangingIndent ());
    double  newRelativeIndentation  = (newIndentation.GetFirstLineIndent () - newIndentation.GetHangingIndent ());
    
    if (oldRelativeIndentation != newRelativeIndentation)
        m_markupStream << L"i" << newRelativeIndentation / GetScaleToDGN () << L",";

    if ((NULL == previousIndentation && 0.0 != newIndentation.GetHangingIndent ())
        || (NULL != previousIndentation && previousIndentation->GetHangingIndent () != newIndentation.GetHangingIndent ()))
        {
        m_markupStream << L"l" << newIndentation.GetHangingIndent () / GetScaleToDGN () << L",";
        }

    // NEEDS_WORK: Compare two tab sequences and output tab data only if needed.
    T_DoubleVectorCR    tabStops    = newIndentation.GetTabStops ();
    size_t              nTabStops   = tabStops.size ();
    
    if (0 == nTabStops)
        {
        m_markupStream << L"tz;";
        }
    else
        {
        m_markupStream << L"t";

        for (size_t i = 0; i < nTabStops; i++)
            {
            if (i > 0)
                m_markupStream << L",";

            m_markupStream << ((double)(tabStops[i] / GetScaleToDGN ()));
            }

        m_markupStream << L";";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void DwgMTextExporter::Export ()
    {
    GetReferenceStyle ();

    bool                firstChar           = true;
    IndentationDataCP   previousIndentation = NULL;

    m_caret = m_caret.GetTextBlock ().Begin ();
    
    while (NULL != (m_paragraph = m_caret.GetCurrentParagraphCP ()))
        {
        AppendIndentationMarkup (previousIndentation, m_paragraph->GetProperties().GetIndentation ());

        ExportParagraph (m_caret, firstChar);
        previousIndentation = &m_paragraph->GetProperties().GetIndentation ();

        if (m_caret.IsAtEnd ())
            break;
        }
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void TextBlock::ToMText
(
WStringR     markup,
DgnModelP    modelRef,
IDwgContextCP   dwgContext,
double          scaleToDGN,
bool            expandFields,
bool            shouldConvertEmptyEdfsToSpaces
) const
    {
#if defined (_WIN32) || !defined (ANDROID) // WIP_NONPORT - stringstream not supported on Android
    BeAssert (GetType () == TEXTBLOCK_TYPE_DwgMText);
    if (GetType () != TEXTBLOCK_TYPE_DwgMText)
        return;

    if (NULL == GetFirstRunProperties ())
        return;

    DwgMTextExporter dwgMTextExporter (*this, modelRef, dwgContext, scaleToDGN/*, expandFields*/, shouldConvertEmptyEdfsToSpaces);
    dwgMTextExporter.Export ();
    dwgMTextExporter.GetMarkup (markup);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
static bool isDTextStyleMarkup (WString::const_iterator iter, WString::const_iterator end)
    {
    if ('%' != *iter)
        return false;

    if ((end - iter) < 3)
        return false;

    wchar_t ch = *(iter+2);
    
    return ('u' == ch || 'U' == ch || 'o' == ch || 'O' == ch);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/08
//---------------------------------------------------------------------------------------
static bool isPercentageCharacter (WString::const_iterator iter, WString::const_iterator end)
    {
    return (((end - iter) >= 3) && ('%' == *iter) && ('%' == *(iter+1)) && ('%' == *(iter+2)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
static void updateStyle (RunPropertiesR currentRunProperties, wchar_t val)
    {
    DPoint2dCR textScale = currentRunProperties.GetFontSize ();
    
    switch (val)
        {
        case 'u':
        case 'U':
            {
            currentRunProperties.SetIsUnderlined (!currentRunProperties.IsUnderlined ());
            currentRunProperties.SetUnderlineOffset ((DgnFontType::TrueType == currentRunProperties.GetFont ().GetType ()) ?
                                textScale.y * TRUETYPE_UNDERLINEOFFSET : textScale.y * SHX_UNDERLINEOFFSET);
            }
            break;

        case 'o':
        case 'O':
            {
            currentRunProperties.SetIsOverlined (!currentRunProperties.IsOverlined ());
            currentRunProperties.SetOverlineOffset ((DgnFontType::TrueType == currentRunProperties.GetFont ().GetType ()) ?
                                textScale.y * TRUETYPE_OVERLINEOFFSET : textScale.y * SHX_OVERLINEOFFSET);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
static void importString (TextBlockR textBlock, WStringCR string, bool isShapeCode)
    {
    if (0 == string.size ())
        return;

    CharStreamP charStream = new CharStream (string.c_str (), textBlock.GetRunPropertiesForAdd (), textBlock.ComputeRunLayoutFlags ());
    
    TextBlockNodeArray unitArray;
    unitArray.push_back (charStream);
    
    textBlock.AppendNodes (unitArray);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void TextBlock::FromDText (WStringCR string, bool isShapeCode)
    {
    WString currentStream;

    for (WString::const_iterator iter = string.begin (); iter != string.end (); ++iter)
        {
        if (isDTextStyleMarkup (iter, string.end ()))
            {
            importString (*this, currentStream, isShapeCode);
            iter += 2;
            updateStyle (m_runPropertiesForAdd, *iter);
            currentStream.assign (L"");
            
            continue;
            }
        else if (isPercentageCharacter (iter, string.end ()))
            {
            // push %%%
            currentStream.push_back (*(iter++));
            currentStream.push_back (*(iter++));
            }

        currentStream.push_back (*iter);
        }

    importString (*this, currentStream, isShapeCode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
BentleyStatus DwgMTextImporter::ExtractIntegerValueFromMarkup (int& value)
    {
    WString  localStr    = m_markup.substr (m_markupIter - m_markup.begin (), m_markup.end () - m_markupIter);
    WCharP    endStr;
    
    value = BeStringUtilities::Wcstol (localStr.c_str (), &endStr, 10);

    BentleyStatus status = ((localStr.c_str () == endStr) ? ERROR : SUCCESS);
    
    m_markupIter += endStr - localStr.c_str ();
    
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
BentleyStatus DwgMTextImporter::ExtractDoubleValueFromMarkup (double& value)
    {
    WString  localStr    = m_markup.substr (m_markupIter - m_markup.begin (), m_markup.end () - m_markupIter);
    WCharP    endStr;
    
    value = wcstod (localStr.c_str (), &endStr);

    BentleyStatus status = localStr.c_str () == endStr ? ERROR : SUCCESS;
    
    m_markupIter += endStr - localStr.c_str ();
    
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
bool DwgMTextImporter::GetFontDataFromMarkup (RunPropertiesR runProperties, WString& fontName, int& pitch)
    {
    // NEEDS_WORK: Need to have a separate flag for slant and italics if I can make this work; I have to disable this as of now.
    // pTextParams->flags.slant = false;

    WString::size_type   offset  = (m_markupIter - m_markup.begin () + 2);
    WString::size_type   index   = m_markup.find_first_of (L"|;", offset);
    
    if (-1 == index)
        return false;

    fontName        = m_markup.substr (offset, (index - offset));
    m_markupIter    += (index - offset + 2);

    runProperties.SetIsBold (false);
    
    while ((m_markupIter != m_markup.end ()) && (';' != *m_markupIter))
        {
        switch (*m_markupIter++)
            {
            case 'b':
                {
                runProperties.SetIsBold ('1' == *m_markupIter++);
                break;
                }

            case 'i':
                {
                runProperties.SetIsItalic ('1' == *m_markupIter++);
                break;
                }
            
            case 'c':
                {
                BeDataAssert (false && L"Not currently supporting arbitrary codesets.");
                
                // Not used, but still have to eat it from the markup...
                int notUsed;
                ExtractIntegerValueFromMarkup (notUsed);

                break;
                }
            
            case 'p':
                {
                ExtractIntegerValueFromMarkup (pitch);
                break;
                }
            }
        }

    if (m_markupIter != m_markup.end ())
        ++m_markupIter;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/05
//---------------------------------------------------------------------------------------
static void getTruetypeFontNameFromMTextFontName (WStringR faceName, WCharCP fontName, bool file)
    {
// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)

    if (file)
        {
        if (SUCCESS == DgnTrueTypeFont::GetFontFaceNameFromFileName (faceName, fontName))
            return;

        faceName = BeFileName::GetFileNameWithoutExtension (fontName);
        return;
        }

    faceName = fontName;

#else

    BeAssert (false && L"Not implemented on non-Win32.");
    faceName = fontName;

#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void DwgMTextImporter::GetFontFromMTextFontString (RunPropertiesR runProperties, WStringR mtextFontMarkup, bool file)
    {
    if (mtextFontMarkup.size () == 0)
        return;

    // parse possible small & big fonts in markup string:
    WString::size_type   comma       = mtextFontMarkup.find_first_of (L",");
    WString              fontName    = mtextFontMarkup.substr (0, comma);
    WString              shxBigFontName;
    
    if (-1 != comma && ++comma < mtextFontMarkup.length())
        shxBigFontName = mtextFontMarkup.substr (comma);

    WString             fileName, fileExtension;
    WString             bigFileName;
    BeFileName          fontNameBuffer;
    DgnFontType         fontType;
    
    if (file)
        {
        BeFileName::ParseName (NULL, NULL, &fileName, &fileExtension, mtextFontMarkup.c_str ());

        if (fileExtension.EqualsI(L"ttf") || fileExtension.EqualsI(L"ttc"))
            {
            fontType = DgnFontType::TrueType;
            fontNameBuffer.BuildName (NULL, NULL, fileName.c_str(), fileExtension.c_str());

            WString faceName;
            
// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)

            if (SUCCESS == DgnTrueTypeFont::GetFontFaceNameFromFileName (faceName, fontNameBuffer))
                fileName = faceName;

#else

            BeAssert (false && L"Unimplemented on non-Win32.");

#endif
            }
        else if (fileExtension.EqualsI(L"shx"))
            {
            fontType = DgnFontType::Shx;
            }
        else
            {
            fontType = DgnFontType::Shx;
            fontNameBuffer.AppendExtension (L".shx");
            }

        if (!shxBigFontName.empty())
            BeFileName::ParseName (NULL, NULL, &bigFileName, NULL, shxBigFontName.c_str());
        }
    else
        {
        fontType = DgnFontType::TrueType;
        fontNameBuffer.SetName (mtextFontMarkup);
        
        getTruetypeFontNameFromMTextFontName (fileName, fontNameBuffer, file);
        }

    DgnFontCP font = DgnFontManager::FindFont (Utf8String (fileName).c_str (), ((DgnFontType::Shx == fontType) ? DgnFontType::Shx : DgnFontType::TrueType), &this->GetDgnModel ()->GetDgnProject ());
    
#if defined (_WIN32)
    if (NULL == font)
        font = &DgnFontManager::GetDefaultShxFont ();
#elif defined (__unix__)
        // SHX font not supported on Linux
#endif

    runProperties.SetFont (*font);
    
    if (fontType != DgnFontType::Shx)
        runProperties.SetShxBigFont (NULL);

    // find and set big SHX font:
    if (0 != bigFileName[0])
        {
        font = DgnFontManager::FindFont (Utf8String (bigFileName).c_str (), DgnFontType::Shx, &this->GetDgnModel ()->GetDgnProject ());
        if (NULL == font)
            font = DgnFontManager::GetDefaultShxBigFont ();

        if (NULL != font)
            runProperties.SetShxBigFont (font);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
bool DwgMTextImporter::ProcessFontMarkup (RunPropertiesR runProperties, bool file)
    {
    int         pitch;
    WString  mtextFontName;
    
    if (!GetFontDataFromMarkup (runProperties, mtextFontName, pitch))
        return false;

    GetFontFromMTextFontString (runProperties, mtextFontName, file);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void DwgMTextImporter::ProcessStyleChange ()
    {
    RunProperties   runProperties   = m_propertiesStack.top ();
    bool            styleChanged    = false;

    switch (*(m_markupIter + 1))
        {
        case 'L':
            runProperties.SetIsUnderlined (true);
            runProperties.SetUnderlineOffset ((DgnFontType::TrueType == runProperties.GetFont ().GetType ()) ?
                runProperties.GetFontSize ().y * TRUETYPE_UNDERLINEOFFSET :
                runProperties.GetFontSize ().y * SHX_UNDERLINEOFFSET);
            
            styleChanged = true;
            m_markupIter += 2;
            break;

        case 'l':
            runProperties.SetIsUnderlined (false);
            runProperties.SetUnderlineOffset (0.0);
            styleChanged = true;
            m_markupIter += 2;
            break;

        case 'O':
            runProperties.SetIsOverlined (true);
            runProperties.SetOverlineOffset ((DgnFontType::TrueType == runProperties.GetFont ().GetType ()) ?
                runProperties.GetFontSize ().y * TRUETYPE_OVERLINEOFFSET :
                runProperties.GetFontSize ().y * SHX_OVERLINEOFFSET);
            
            styleChanged = true;
            m_markupIter += 2;
            break;

        case 'o':
            runProperties.SetIsOverlined (false);
            runProperties.SetOverlineOffset (0.0);
            styleChanged = true;
            m_markupIter += 2;
            break;

        case 'A':
        case 'a':
            {
            int value;
            m_markupIter += 2;
            
            if (SUCCESS == ExtractIntegerValueFromMarkup (value))
                {
                runProperties.SetLineAlignment ((LineAlignment) value);
                styleChanged = true;
                }

            if (';' == *m_markupIter)
                m_markupIter++;
            
            break;
            }

        case 'C':
            {
            int value;
            m_markupIter += 2;
            
            if (SUCCESS == ExtractIntegerValueFromMarkup (value))
                {
                runProperties.SetColor (GetDwgContext ()->DwgColorToDgnColor (value));
                styleChanged = true;
                }

            if (';' == *m_markupIter)
                m_markupIter++;
            
            break;
            }

        case 'H':
            {
            double value;
            m_markupIter += 2;
            
            if (SUCCESS == ExtractDoubleValueFromMarkup (value))
                {
                if (0.0 == runProperties.GetFontSize ().x)
                    {
                    DPoint2d fontSize = runProperties.GetFontSize ();
                    fontSize.x = 1.0;
                    runProperties.SetFontSize (fontSize);
                    }

                if (0.0 == runProperties.GetFontSize ().y)
                    {
                    DPoint2d fontSize = runProperties.GetFontSize ();
                    fontSize.y = 1.0;
                    runProperties.SetFontSize (fontSize);
                    }

                if (0.0 == value)
                    value = 1.0;

                if (*m_markupIter == 'x' || *m_markupIter == 'X')
                    {
                    DPoint2d fontSize = runProperties.GetFontSize ();
                    fontSize.scale (&fontSize, value);
                    runProperties.SetFontSize (fontSize);
                    m_markupIter++;
                    }
                else
                    {
                    DPoint2d    fontSize            = runProperties.GetFontSize ();
                    double      currentWidthFactor  = fontSize.x / fontSize.y;
                    
                    fontSize.y  = value * GetScaleToDGN ();
                    fontSize.x  = value * GetScaleToDGN () * currentWidthFactor;
                    
                    runProperties.SetFontSize (fontSize);
                    }

                styleChanged = true;
                }
            
            if (';' == *m_markupIter)
                m_markupIter++;
            
            break;
            }

        case 'W':
            {
            double value;
            m_markupIter += 2;
            
            if (SUCCESS == ExtractDoubleValueFromMarkup (value))
                {
                if (0.0 == value)
                    value = 1.0;

                if ('x' == *m_markupIter)
                    {
                    DPoint2d fontSize = runProperties.GetFontSize ();
                    fontSize.x *= value;
                    runProperties.SetFontSize (fontSize);
                    
                    m_markupIter++;
                    }
                else
                    {
                    DPoint2d fontSize = runProperties.GetFontSize ();
                    fontSize.x = runProperties.GetFontSize ().y * value;
                    runProperties.SetFontSize (fontSize);
                    }

                // Looks like this says that the width factor cannot be greater than 10.0. Not sure why.
                //  if (val <= (pTextSize->size.height * 10.0))
                //      pTextSize->size.width = val;
                
                styleChanged = true;
                }

            if (m_markupIter != m_markup.end () && ';' == *m_markupIter)
                m_markupIter++;

            break;
            }

        case 'Q':
        case 'q':
            {
            double value;
            m_markupIter += 2;
            
            if (SUCCESS == ExtractDoubleValueFromMarkup (value) && *m_markupIter == ';')
                {
                runProperties.SetCustomSlantAngle (value * msGeomConst_radiansPerDegree);
                runProperties.SetIsItalic (true);
                styleChanged = true;
                }
            
            while (m_markupIter != m_markup.end () && *m_markupIter++ != ';');
            
            break;
            }

        case 'T':
            {
            double value;
            m_markupIter += 2;
            if (SUCCESS == ExtractDoubleValueFromMarkup (value))
                {
                // If value is 1.0 or greater than 0.75, do not use acadintercharspacing.
                if (0.75 > value || value > 4.0)
                    {
                    runProperties.SetCharacterSpacingType (CharacterSpacingType::Absolute);
                    runProperties.SetCharacterSpacingValue (0.0);
                    }
                else
                    {
                    runProperties.SetCharacterSpacingValue (value);
                    runProperties.SetCharacterSpacingType (CharacterSpacingType::Factor);
                    }
                
                styleChanged = true;
                }

            if ('x' == *m_markupIter)
                m_markupIter++;
            
            if (';' == *m_markupIter)
                m_markupIter++;
            
            break;
            }

        // Test Case: filenametest.dwg
        case 'F':
            {
            if (!ProcessFontMarkup (runProperties, true))
                {
                m_currentStream.push_back (*m_markupIter++);
                styleChanged = false;
                }
            else
                {
                styleChanged = true;
                }
            
            break;
            }
        
        case 'f':
            {
            if (!ProcessFontMarkup (runProperties, false))
                {
                m_currentStream.push_back (*m_markupIter++);
                styleChanged = false;
                }
            else
                {
                styleChanged = true;
                }
            
            break;
            }
        
        case 'p':
            // I dont see why this case is present. There is no markup designated with a p.
            BeAssert (false);
            break;
        
        case 'c': // This is some markup - not sure what this means. Dont know how to check.
            {
            int value;
            m_markupIter += 2;
            
            if (SUCCESS == ExtractIntegerValueFromMarkup (value))
                {
                IntColorDef colorDef;
                colorDef.m_int = (int)value;

#ifdef DGNV10FORMAT_CHANGES_WIP
                UInt32 color = GetDgnModel ()->GetDgnProject ()->GetColorMapP ()->CreateElementColor (colorDef, NULL, NULL, *GetDgnModel ()->GetDgnProject ());
                runProperties.SetColor (color);
#endif
                styleChanged = true;
                }

            if (';' == *m_markupIter)
                m_markupIter++;
            
            break;
            }
        
        default:
            m_currentStream.push_back (*m_markupIter);
            m_markupIter++;
            break;
        }

    if (styleChanged)
        {
        DumpStream ();
        SetProperties (runProperties);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2011
//---------------------------------------------------------------------------------------
static void stripBackslashes (WStringR str)
    {
    size_t index = 0;
    
    while (WString::npos != (index = str.find_first_of (L'\\', index)))
        {
        str.erase (index, 1);
        
        if (L'\\' == str[index])
            ++index;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void DwgMTextImporter::ProcessStackedFraction ()
    {
    m_markupIter += 2;

    WString::size_type      index               = m_markup.find_first_of (';', m_markupIter - m_markup.begin ());
    WString::const_iterator endIter             = (index == WString::npos) ? m_markup.end () : m_markup.begin () + index;
    WString                 fractionString      = m_markup.substr (m_markupIter - m_markup.begin (), endIter - m_markupIter);
    size_t                  fractionStringLen   = fractionString.size ();
    
    index = 0;
    
    // Allow non-stacked fractions within stacked fraction when the separator is escaped with a backslash.
    do
        {
        index = fractionString.find_first_of (L"#/^", ((0 == index) ? 0 : (index + 1)));
        }
        while ((WString::npos != index) && (index > 0) && (L'\\' == fractionString[index - 1]));
    
    WString numerator       = fractionString.substr (0, index);
    WString separator;
    WString denominator;
    
    if (index != WString::npos)
        {
        separator   = fractionString.substr (index, 1);
        denominator = fractionString.substr (index + 1, fractionString.size ());
        }
    
    // Pursuent to above, take the escaped character after a backslash as literal (e.g. remove all remaining backslahses).
    stripBackslashes (numerator);
    stripBackslashes (denominator);
    
    FractionP   fraction    = NULL;
    DPoint2d    textScale;  textScale.setComponents (1.0, 1.0);
    
    switch (separator[0])
        {
        case '#':
            fraction = new DiagonalBarFraction (numerator.c_str (), denominator.c_str (), m_propertiesStack.top (), m_textBlock.ComputeRunLayoutFlags (), (StackedFractionAlignment) m_propertiesStack.top ().GetLineAlignment (), &textScale);
            break;

        case '^':
            fraction = new NoBarFraction (numerator.c_str (), denominator.c_str (), m_propertiesStack.top (), m_textBlock.ComputeRunLayoutFlags (), (StackedFractionAlignment) m_propertiesStack.top ().GetLineAlignment (), &textScale);
            break;

        case '/':
        default:
            fraction = new HorizontalBarFraction (numerator.c_str (), denominator.c_str (), m_propertiesStack.top (), m_textBlock.ComputeRunLayoutFlags (), (StackedFractionAlignment) m_propertiesStack.top ().GetLineAlignment (), &textScale);
            break;
        }

    DumpStream ();

    // TR#96281 - If you do not hit any separator character, then we want to add the text as normal text.
    m_unitArray.push_back (fraction);
    m_markupIter += fractionStringLen;
    
    if (m_markupIter != m_markup.end ())
        ++m_markupIter;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void DwgMTextImporter::ProcessIndentation ()
    {
    ParagraphProperties updatedSeedParaProps    (m_textBlock.GetParagraphPropertiesForAdd ());
    IndentationData     updatedIndentationData  (updatedSeedParaProps.GetIndentation ());

    m_markupIter += 2;
    
    while ((m_markup.end () != m_markupIter) && (L';' != *m_markupIter))
        {
        switch (*m_markupIter)
            {
            case L'x':
                {
                ++m_markupIter;
                m_indentationScaleFactor = (m_textBlock.GetNodeOrFirstRunHeight () / m_textBlock.GetProperties ().GetAnnotationScale ());
                
                break;
                }

            case L'l':
                {
                ++m_markupIter;
                
                double          paragraphIndent         = 0.0;
                double          relativeFirstLineIndent = (updatedIndentationData.GetFirstLineIndent () - updatedIndentationData.GetHangingIndent ());
                
                this->ExtractDoubleValueFromMarkup (paragraphIndent);
                paragraphIndent *= m_indentationScaleFactor;
                
                updatedIndentationData.SetHangingIndent (paragraphIndent);
                updatedIndentationData.SetFirstLineIndent (paragraphIndent + relativeFirstLineIndent);
                
                break;
                }

            case L'i':
                {
                ++m_markupIter;
                
                double relativeFirstLineIndent = 0;
                
                this->ExtractDoubleValueFromMarkup (relativeFirstLineIndent);
                relativeFirstLineIndent *= m_indentationScaleFactor;
                
                updatedIndentationData.SetFirstLineIndent (updatedIndentationData.GetHangingIndent () + relativeFirstLineIndent);
                
                break;
                }

            case L't':
                {
                if (L'z' == *(m_markupIter + 1))
                    {
                    m_markupIter += 2;
                    
                    updatedIndentationData.SetTabStops (0, NULL);
                    
                    break;
                    }

                T_DoubleVector tabStops;
                
                do
                    {
                    ++m_markupIter;
                    
                    double nextTabValue = 0.0;
                    
                    this->ExtractDoubleValueFromMarkup (nextTabValue);
                    nextTabValue *= m_indentationScaleFactor;
                    tabStops.push_back (nextTabValue);
                    
                    } while ((m_markup.end () != m_markupIter) && (L';' != *m_markupIter));

                updatedIndentationData.SetTabStops (tabStops);
                
                break;
                }

            case L'q':
                {
                ++m_markupIter;
                
                HorizontalJustification horizontal;
                VerticalJustification   vertical;
                m_textBlock.GetHorizontalVerticalJustifications (&horizontal, &vertical, updatedSeedParaProps.GetJustification ());

                HorizontalJustification currHor = horizontal;

                switch (*m_markupIter)
                    {
                    case L'l':  horizontal = HORIZONTAL_JUSTIFICATION_Left;     break;
                    case L'c':  horizontal = HORIZONTAL_JUSTIFICATION_Center;   break;
                    case L'r':  horizontal = HORIZONTAL_JUSTIFICATION_Right;    break;
                    case L'j':  /* justified to be supported */                 break;
                    case L'd':  /* distributed to be supported */               break;
                    }

                ++m_markupIter;
                
                if (currHor != horizontal)
                    {
                    updatedSeedParaProps.SetJustification (TextBlock::GetJustificationFromAlignments (horizontal, vertical));
                    
                    // Dgn text does not support per-paragraph justification overrides. We are currently choosing to fake it (for a single override) by modifying our user origin. While we cannot achieve full-fidelity round-tripping, this gets us visual fidelity and reasonable behavior. This is not done in ConvertToDgnContext::ProcessMText after discussion with Don to attempt to isolate q* markup processing here.
                    double wordWrapDistance = m_textBlock.GetProperties ().GetDocumentWidth ();
                    
                    if ((wordWrapDistance > 0.0) && m_textBlock.IsEmpty ())
                        {
                        double originXShift = 0.0;
                
                        switch (currHor)
                            {
                            case HORIZONTAL_JUSTIFICATION_Left:
                                {
                                switch (horizontal)
                                    {
                                    case HORIZONTAL_JUSTIFICATION_Center:   originXShift = (wordWrapDistance / 2.0);    break;
                                    case HORIZONTAL_JUSTIFICATION_Right:    originXShift = wordWrapDistance;            break;
                                    }
                                
                                break;
                                }
                            
                            case HORIZONTAL_JUSTIFICATION_Center:
                                {
                                switch (horizontal)
                                    {
                                    case HORIZONTAL_JUSTIFICATION_Left:     originXShift = -(wordWrapDistance / 2.0);   break;
                                    case HORIZONTAL_JUSTIFICATION_Right:    originXShift = (wordWrapDistance / 2.0);    break;
                                    }
                                
                                break;
                                }
                            
                            case HORIZONTAL_JUSTIFICATION_Right:
                                {
                                switch (horizontal)
                                    {
                                    case HORIZONTAL_JUSTIFICATION_Left:     originXShift = -wordWrapDistance;           break;
                                    case HORIZONTAL_JUSTIFICATION_Center:   originXShift = -(wordWrapDistance / 2.0);   break;
                                    }
                                
                                break;
                                }
                            }
                        
                        DPoint3d originShift;
                        originShift.init (originXShift, 0.0, 0.0);
                        
                        m_textBlock.GetOrientation ().multiply (&originShift);
                        
                        DPoint3d userOrigin = m_textBlock.GetUserOrigin ();
                        userOrigin.add (&originShift);
                        m_textBlock.SetUserOrigin (userOrigin);
                        }
                    }
                
                break;
                }

            default:
                {
                ++m_markupIter;
                break;
                }
            }
        }

    if (m_markup.end () != m_markupIter)
        ++m_markupIter;

    // AutoCAD ignores leading indents when line break length is 0.0. If we encounter this scenario, strip the indentation information so our normal layout logic suffices. While I'm not a huge fan of stripping data, we feel that this is more-or-less a bug in AutoCAD, and don't want to disrupt our layout logic to work around it. This data does not have a visible impact in AutoCAD, hence we feel it is okay to strip it.
    if (m_textBlock.GetProperties ().GetDocumentWidth () > 0.0)
        updatedSeedParaProps.SetIndentation (updatedIndentationData);
    
    m_textBlock.SetParagraphPropertiesForAdd (updatedSeedParaProps);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void DwgMTextImporter::ProcessBackslash ()
    {
    if ((m_markupIter + 1) == m_markup.end ())
        {
        m_currentStream.push_back (*m_markupIter++);
        return;
        }

    switch (*(m_markupIter + 1))
        {
        case 'P':
            {
            DumpStream ();
            m_unitArray.push_back (new ParagraphBreak (m_propertiesStack.top (), m_textBlock.ComputeRunLayoutFlags ()));
            
            DumpUnitArray ();
            m_markupIter += 2;
            
            break;
            }

        case '{':
        case '}':
        case '\\':
        case '#':
        case '^':
        case '/':
            {
            m_currentStream.push_back (*(m_markupIter + 1));
            m_markupIter += 2;
            }
            break;

        case '~':
            m_currentStream.push_back (0x00A0);
            m_markupIter += 2;
            break;

        case 'S':
        case 's':
            ProcessStackedFraction ();
            break;

        case 'p':
            ProcessIndentation ();
            break;

        case 'U':
            {
            if ((m_markupIter + 6) >= m_markup.end () || '+' != *(m_markupIter + 2))
                {
                m_currentStream.push_back (*m_markupIter++);
                return;
                }

            WString    characterCodeString ((m_markupIter + 3), (m_markupIter + 7));
            int             characterCode;

            BE_STRING_UTILITIES_SWSCANF (characterCodeString.c_str (), L"%x", &characterCode);

            m_currentStream.push_back ((WChar)characterCode);

            m_markupIter += 7;

            break;
            }

        default:
            ProcessStyleChange ();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      03/05
//---------------------------------------------------------------------------------------
StatusInt   parseXMLTag (WCharP string, WCharCP beginInput, WCharCP endInput, WCharCP tag)
    {
    WChar prefix[1024];
    WChar suffix[1024];
    
    BeStringUtilities::Snwprintf (prefix, _countof(prefix), L"<%ls>", tag);
    BeStringUtilities::Snwprintf (suffix, _countof(suffix), L"</%ls", tag);

    WCharCP  pStart;
    WCharCP  pEnd;
    
    if (NULL != (pStart = ::wcsstr (beginInput, prefix)) && pStart < endInput && NULL != (pEnd = ::wcsstr (beginInput, suffix)) && pEnd < endInput)
        {
        pStart += wcslen (prefix);

        ptrdiff_t length = pEnd - pStart;
        
        wcsncpy (string, pStart, length);
        string[length] = '\0';
        
        return SUCCESS;
        }

    return ERROR;
    }

#ifdef BEIJING_DGNPLATFORM_WIP_Fields
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     02/04
//---------------------------------------------------------------------------------------
bool DwgMTextImporter::ProcessField ()
    {
    WString  localString     = m_markup.substr (m_markupIter - m_markup.begin (), m_markup.end () - m_markupIter);
    WCharCP   pString         = localString.c_str ();
    WCharCP   pSuffix;
    WCharCP   prefix          = L"%<Field>";
    WCharCP   suffix          = L"</Field>%";

    if (0 == wcsncmp (pString, prefix, wcslen (prefix)) && NULL != (pSuffix = ::wcsstr (pString, suffix)))
        {
        WCharCP   pField      = (pString + wcslen (prefix));
        WCharCP   pEndField   = (pSuffix + wcslen (suffix));

        m_markupIter += pEndField - pString;

        WChar expression[MAX_FieldSize];
        WChar result[MAX_FieldResultSize];
        
        if (SUCCESS == parseXMLTag (expression, pField, pSuffix, L"Specification") && SUCCESS == parseXMLTag (result, pField, pSuffix, L"Result"))
            {
            DumpStream ();

            CharStreamP charStream = new CharStream (result, m_propertiesStack.top (), m_textBlock.GetProperties ().ComputeRunLayoutFlags ());
            m_unitArray.push_back (charStream);
            charStream->AttachSpan (new Field (expression, DgnFileFormatType::DWG));
            }

        return true;
        }
    
    return false;
    }
#endif // BEIJING_DGNPLATFORM_WIP_Fields

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void DwgMTextImporter::DumpStream ()
    {
    if (0 == m_currentStream.size ())
        return;

    m_unitArray.push_back (new CharStream (m_currentStream.c_str (), m_propertiesStack.top (), m_textBlock.ComputeRunLayoutFlags ()));
    m_currentStream.assign (L"");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void DwgMTextImporter::DumpUnitArray ()
    {
    m_textBlock.AppendNodes (m_unitArray);
    m_unitArray.clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void DwgMTextImporter::ProcessMarkup ()
    {
    while (m_markupIter != m_markup.end ())
        {
        switch (*m_markupIter)
            {
            case L'\\':
                ProcessBackslash ();
                break;

            case '{':
                {
                m_propertiesStack.push (m_propertiesStack.top ());
                m_markupIter++;
                
                break;
                }

            case '}':
                {
                BeAssert (m_propertiesStack.size () != 0);
                
                DumpStream ();
                
                if (m_propertiesStack.size () > 1) // never leave an empty stack
                    m_propertiesStack.pop ();
                
                m_markupIter++;
                
                break;
                }

            case '\t':
                {
                DumpStream ();

                // AutoCAD ignores leading indents when line break length is 0.0. If we encounter this scenario,
                //  strip the indentation information so our normal layout logic suffices. While I'm not a huge
                //  fan of stripping data, we feel that this is more-or-less a bug in AutoCAD, and don't want to
                //  disrupt our layout logic to work around it. This data does not have a visible impact in AutoCAD,
                //  hence we feel it is okay to strip it.
                if (m_textBlock.GetProperties ().GetDocumentWidth () > 0.0
                    || (0 != m_unitArray.size () && NULL == dynamic_cast<ParagraphBreakCP>(m_unitArray.back ()) && NULL == dynamic_cast<LineBreakCP>(m_unitArray.back ())))
                    {
                    m_unitArray.push_back (new Tab (m_propertiesStack.top (), m_textBlock.ComputeRunLayoutFlags ()));
                    }

                m_markupIter++;
                
                break;
                }

            case L'\n':
                {
                DumpStream ();
                m_unitArray.push_back (new LineBreak (m_propertiesStack.top (), m_textBlock.ComputeRunLayoutFlags ()));
                m_markupIter++;
                
                break;
                }

            case '%':
                {
                // See if it is a field marker. If it is process it as a field and break.
#ifdef BEIJING_DGNPLATFORM_WIP_Fields
                if (((m_markupIter+1) != m_markup.end ()) && '<' == *(m_markupIter+1) && ProcessField ())
                    break;
#endif // BEIJING_DGNPLATFORM_WIP_Fields

                // mtext does not support %% markup pairs except for diameter, degree, & plus/minus:
                if (m_markupIter+1 != m_markup.end() && m_markupIter+2 != m_markup.end() && '%' == *(m_markupIter+1))
                    {
                    if (IS_DWG_SYMBOL(*(m_markupIter+2)))
                        {
                        m_currentStream.push_back (*m_markupIter++);
                        m_currentStream.push_back (*m_markupIter++);
                        }
                    else
                        {
                        m_currentStream.push_back ('%');
                        m_currentStream.push_back ('%');
                        m_currentStream.push_back (*m_markupIter++);
                        m_currentStream.push_back ('%');
                        m_currentStream.push_back ('%');
                        }
                    }

                // Fall through to default:
                }

            default:
                m_currentStream.push_back (*m_markupIter++);
                break;
            }
        }

    DumpStream ();
    
    m_textBlock.AppendNodes (m_unitArray);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void TextBlock::FromMText (WCharCP markupString, IDwgContextCR dwgContext, double scaleToDGN)
    {
    if (WString::IsNullOrEmpty (markupString))
        { BeAssert (false); return; }
    
    DwgMTextImporter dwgImporter (*this, markupString, dwgContext, scaleToDGN);
    dwgImporter.ProcessMarkup ();
    }

//=======================================================================================
// @bsiclass                                                    Venkat.Kalyan   06/07
//=======================================================================================
class DgnDwgMTextConverter
{
private:
    
    TextBlockR  m_textBlock;

private:
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   01/06
    //---------------------------------------------------------------------------------------
    double ComputeLaidOutTextBlockLength ()
        {
        double laidOutTextBlockLength = 0.0;
        
        LineRange lineRange (m_textBlock);
        FOR_EACH (LineCR line, lineRange)
            laidOutTextBlockLength += (m_textBlock.GetProperties ().IsVertical () ? line.GetNominalHeight () : line.GetNominalWidth ());

        return laidOutTextBlockLength;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   01/06
    //---------------------------------------------------------------------------------------
    void ChangeInterCharSpacing (double newInterCharSpacingValue)
        {
        m_textBlock.SetCharacterSpacing (newInterCharSpacingValue, CharacterSpacingType::Factor, m_textBlock.Begin (), m_textBlock.End ());
        m_textBlock.Reprocess ();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   01/06
    //---------------------------------------------------------------------------------------
    bool IsInterCharSpacingConversionNeeded ()
        {
        // If any of the basic units have an inter char spacing and if it is not acad inter char spacing, then we need a conversion.
        RunRange runRange (m_textBlock);
        FOR_EACH (RunCR run, runRange)
            {
            if (0.0 != run.GetProperties ().GetCharacterSpacingValue ()
                && !(run.GetProperties ().GetCharacterSpacingType () == CharacterSpacingType::Factor)
                && (fabs (run.GetProperties ().GetCharacterSpacingValue ()) > DWG_CHARACTERSPACING_COMPARISON_TOLERANCE))
                {
                return true;
                }
            }

        return false;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   PaulChater      02/03
    //---------------------------------------------------------------------------------------
    void CorrectTextForMarginJustification ()
        {
        int     nParagraphs             = m_textBlock.GetParagraphCount ();
        DVec3d  offset;
        bool    changeNeeded            = false;
        bool    isZeroMaxCharsInLine    = (0 == m_textBlock.GetProperties ().GetMaxCharactersPerLine ());

        offset.zero ();

        for (int i = 0; i < nParagraphs; i++)
            {
            ParagraphP                  paragraph       = m_textBlock.GetParagraph (i);
            TextElementJustification    justification   = paragraph->GetJustification ();
            double                      width           = m_textBlock.GetPaperWidth ();
            HorizontalJustification     horJust;
            VerticalJustification       verJust;

            m_textBlock.GetHorizontalVerticalJustifications (&horJust, &verJust, justification);

            switch (horJust)
                {
                case HORIZONTAL_JUSTIFICATION_LeftMargin:
                    {
                    if (isZeroMaxCharsInLine)
                        {
                        horJust = HORIZONTAL_JUSTIFICATION_Left;
                        }
                    else
                        {
                        horJust     = HORIZONTAL_JUSTIFICATION_Right;
                        offset.x    = width;
                        }
                
                    changeNeeded = true;
                
                    break;
                    }

                case HORIZONTAL_JUSTIFICATION_RightMargin:
                    {
                    if (isZeroMaxCharsInLine)
                        {
                        horJust = HORIZONTAL_JUSTIFICATION_Left;
                        }
                    else
                        {
                        horJust     = HORIZONTAL_JUSTIFICATION_Left;
                        offset.x    = -width;
                        }
                
                    changeNeeded = true;
                
                    break;
                    }
                }

            if (changeNeeded)
                paragraph->SetJustification (TextBlock::GetJustificationFromAlignments (horJust, verJust));
            }

        if (changeNeeded && !isZeroMaxCharsInLine)
            {
            RotMatrix rMatrix = m_textBlock.GetOrientation ();
            rMatrix.multiply (&offset);

            DPoint3d userOrigin = m_textBlock.GetUserOrigin ();
            userOrigin.add (&offset);

            m_textBlock.SetUserOrigin (userOrigin);
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   10/06
    //---------------------------------------------------------------------------------------
    void ConvertLineSpacingToExactlyLineSpacing ()
        {
        DgnLineSpacingType lineSpacingType = m_textBlock.Begin ().GetCurrentParagraphCP ()->GetProperties ().GetLineSpacingType ();
        if (lineSpacingType == DgnLineSpacingType::ExactFromLineTop || lineSpacingType == DgnLineSpacingType::AtLeast)
            return;

        // AutoCAD has no equivalent to exact line spacing, so the best outcome is to ensure we stay
        //  within our exact bounding box when converting. The primary difficulty comes when you have
        //  a text block with non-uniform text heights. AutoCAD measures baseline-to-baseline, and since
        //  this value is constant, cannot deal with variable text heights. The formula is as such:
        //
        //  TextBlockExactHeight = FirstLineAscension + DWGLineSpacing * (NumberOfLines - 1) + LastLineDescension
        //  (or)
        //  DWGLineSpacing = (TextBlockExactHeight - FirstLineAscension - LastLineDescension) / (NumberOfLines - 1)
        //
        //  For a single line, we will assume DWGLineSpacing = DGNLineSpacing + FirstLineAscension.
        //
        //  For vertical text, we believe that the line spacing value is the distance from the center of one line to
        //  the center of the next, divided by 1.2. It can be witnessed in AutoCAD that text converted from horizontal
        //  to vertical will have its line spacing adjusted by 20% (x1.2). This can also be measured in AutoCAD by
        //  setting a line spacing distance value, and measuring the actual width from line center to line center.

        size_t numLines = m_textBlock.GetLineCount (m_textBlock.Begin (), m_textBlock.End ());

        // For layout purposes, ignore a potential single trailing blank line.
        if (m_textBlock.GetLine (numLines - 1)->IsEmpty ())
            numLines--;

        double  dgnTextBlockHeight  = m_textBlock.GetExactHeight ();
        double  dwgLineSpacing;

        if (1 == numLines)
            {
            if (!m_textBlock.GetProperties ().IsVertical ())
                dwgLineSpacing = m_textBlock.Begin ().GetCurrentParagraphCP ()->GetProperties ().GetLineSpacingValue () + m_textBlock.GetLine (0)->GetNominalHeight ();
            else
                dwgLineSpacing = (m_textBlock.Begin ().GetCurrentParagraphCP ()->GetProperties ().GetLineSpacingValue () + m_textBlock.GetLine (0)->GetNominalWidth ()) * DWG_VERTICAL_LINESPACING_FACTOR;
            }
        else
            {
            if (!m_textBlock.GetProperties ().IsVertical ())
                {
                LineP   firstLine               = m_textBlock.GetLine (0);
                size_t  firstLineRunCount       = firstLine->GetRunCount ();
                double  firstLineMaxAscension   = 0.0;

                for (size_t i = 0; i < firstLineRunCount; ++i)
                    {
                    DRange3d runRange = firstLine->GetRun (i)->GetExactRange ();

                    if (runRange.high.y > firstLineMaxAscension)
                        firstLineMaxAscension = runRange.high.y;
                    }

                LineP   lastLine                = m_textBlock.GetLine (numLines - 1);
                size_t  lastLineRunCount        = lastLine->GetRunCount ();
                double  lastLineMaxDescension   = 0.0;

                for (size_t i = 0; i < lastLineRunCount; ++i)
                    {
                    DRange3d runRange = lastLine->GetRun (i)->GetExactRange ();

                    if (runRange.low.y < lastLineMaxDescension)
                        lastLineMaxDescension = runRange.low.y;
                    }

                dwgLineSpacing = (dgnTextBlockHeight - firstLineMaxAscension + lastLineMaxDescension /*negative*/) / ((double)numLines - 1.0);
                }
            else
                {
                LineP   firstLine           = m_textBlock.GetLine (0);
                size_t  firstLineRunCount   = firstLine->GetRunCount ();
                double  firstLineMaxWidth   = 0.0;

                for (size_t i = 0; i < firstLineRunCount; ++i)
                    {
                    double width = firstLine->GetRun (i)->GetProperties ().GetFontSize ().x;

                    if (width > firstLineMaxWidth)
                        firstLineMaxWidth = width;
                    }

                LineP   lastLine            = m_textBlock.GetLine (numLines - 1);
                size_t  lastLineRunCount    = lastLine->GetRunCount ();
                double  lastLineMaxWidth    = 0.0;

                for (size_t i = 0; i < lastLineRunCount; ++i)
                    {
                    double width = lastLine->GetRun (i)->GetProperties ().GetFontSize ().x;

                    if (width > lastLineMaxWidth)
                        lastLineMaxWidth = width;
                    }

                dwgLineSpacing = ((m_textBlock.GetNominalWidth () - (firstLineMaxWidth / 2.0) - (lastLineMaxWidth / 2.0)) / ((double)numLines - 1.0)) * DWG_VERTICAL_LINESPACING_FACTOR;
                }
            }

        dwgLineSpacing /= m_textBlock.GetProperties ().GetAnnotationScale ();

        ParagraphRange paragraphRange (m_textBlock);
        m_textBlock.SetLineSpacingType (DgnLineSpacingType::ExactFromLineTop, paragraphRange);
        m_textBlock.SetLineSpacingValue (dwgLineSpacing, paragraphRange);
        m_textBlock.Reprocess ();

        double  dgnFirstLineMaxHeight   = m_textBlock.GetParagraph (0)->GetLine (0)->GetMaxUnitHeight ();
        double  dgnFirstUnitHeight      = m_textBlock.GetNodeOrFirstRunHeight ();

        RotMatrix rMatrix = m_textBlock.GetOrientation ();

        DVec3d firstRunOffset;
        firstRunOffset.zero ();
        if (NULL != m_textBlock.GetParagraph (0) && NULL != m_textBlock.GetParagraph (0)->GetLine (0) && NULL != m_textBlock.GetParagraph (0)->GetLine (0)->GetRun (0))
            firstRunOffset = m_textBlock.GetParagraph (0)->GetLine (0)->GetRun (0)->GetLineOffset ();

        DVec3d yVec;
        rMatrix.getColumn (&yVec, 1);

        VerticalJustification vJust = VERTICAL_JUSTIFICATION_Top;
        if (NULL != m_textBlock.GetParagraph (0))
            m_textBlock.GetHorizontalVerticalJustifications (NULL, &vJust, m_textBlock.GetParagraph (0)->GetJustification ());

        switch (vJust)
            {
            case (VERTICAL_JUSTIFICATION_Top):
            case (VERTICAL_JUSTIFICATION_Cap):
                {
                yVec.scale (-(dgnFirstLineMaxHeight - dgnFirstUnitHeight - firstRunOffset.y));
                break;
                }

            case (VERTICAL_JUSTIFICATION_Middle):
                {
                yVec.scale (-(dgnFirstLineMaxHeight - dgnFirstUnitHeight - firstRunOffset.y) / 2.0);
                break;
                }

            case (VERTICAL_JUSTIFICATION_Baseline):
                {
                yVec.scale (dgnFirstLineMaxHeight - dgnFirstUnitHeight - firstRunOffset.y);
                break;
                }

            case (VERTICAL_JUSTIFICATION_Descender):
                {
                yVec.scale ((dgnFirstLineMaxHeight - dgnFirstUnitHeight - firstRunOffset.y) + m_textBlock.ComputeHorizontalDescenderJustificationOffset ());
                break;
                }
            }

        DPoint3d origin = m_textBlock.GetTextOrigin ();
        origin.sumOf (&origin, &yVec);

        m_textBlock.SetTextOrigin (origin);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   05/06
    //---------------------------------------------------------------------------------------
    void AdjustOriginForJustification ()
        {
        if (!m_textBlock.IsDGNType ())
            return;

        LineCP firstLine = m_textBlock.GetLine (0);
        if (NULL == firstLine)
            return;

        TextElementJustification    justification           = m_textBlock.GetParagraph (m_textBlock.GetParagraphCount () - 1)->GetJustification ();
        VerticalJustification       verticalJustification;
        double                      nodeNumberHeight        = m_textBlock.GetNodeOrFirstRunHeight ();
        DVec3d                      offset;                 offset.zero ();

        m_textBlock.GetHorizontalVerticalJustifications (NULL, &verticalJustification, justification);

        // How AutoCAD places user origin in relation to the baseline:
        //  Top:        User origin is at a height of NodeNumberHeight above the baseline of the first line.
        //  Middle:     User origin is at a height that is half way up from the last line's baseline, and the node number height above the first line.
        //  Bottom:     User origin is at a height of 1/3 * NodeNumberHeight below the baseline of the last line.

        switch (verticalJustification)
            {
            case (VERTICAL_JUSTIFICATION_Top):          offset.y    = nodeNumberHeight - firstLine->GetMaxDistanceAboveBaseline ();                         break;
            case (VERTICAL_JUSTIFICATION_Middle):       offset.y    = ((1.0 / 3.0) * nodeNumberHeight) - (firstLine->GetMaxDistanceAboveBaseline () / 2.0); break;
            case (VERTICAL_JUSTIFICATION_Baseline):     offset.y    = -(1.0 / 3.0) * nodeNumberHeight;                                                      break;
            case (VERTICAL_JUSTIFICATION_Descender):    /* No offset in this case; no direct mapping exists for our descender justification. */             break;
        
            default:
                // I don't expect to get here, as no other justifications are valid for DWG export.
                BeAssert (false);
            }

        double maxLineOffset = 0.0;
        for (UInt32 runIndex = 0; runIndex < firstLine->GetRunCount (); runIndex++)
            {
            RunCR   currRun             = *firstLine->GetRun (runIndex);
            double  currRunLineOffset   = currRun.GetLineOffset ().y;

            if (currRunLineOffset > maxLineOffset)
                maxLineOffset = currRunLineOffset;
            }

        offset.y += maxLineOffset;
    
        m_textBlock.GetOrientation ().Multiply (offset);

        DPoint3d origin = m_textBlock.GetUserOrigin ();
        origin.Add (offset);

        m_textBlock.SetUserOrigin (origin);
        }

public:

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   06/07
    //---------------------------------------------------------------------------------------
    DgnDwgMTextConverter (TextBlockR textBlock) :
        m_textBlock (textBlock)
        {
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   06/07
    //---------------------------------------------------------------------------------------
    void Convert ()
        {
        // The overall goal is to always keep user origin (e.g. snap / placement point), except in some subjective cases we deem result in shifts that are too great.

        this->AdjustOriginForJustification ();

        DPoint3d dgnUserOrigin = m_textBlock.GetUserOrigin ();

        // In its initial form, I am providing a way for the inter char spacing to be converted to the correct form.
        // ACAD inter char spacing equation: TLW (Total Laid Out Width) = A + (X - 1.0) * W
        // With the condition that when X = 0.0, A = TLW.
        // So, set ACAD inter char spacing = 0.0, and calculate A.
        // Then set X = 2.0 and calculate W as TLW - A.
        // In the above equation, we know A, we know W. So, for the actual TLW, calculate X as (TLW - A) / W + 1.0
        if (this->IsInterCharSpacingConversionNeeded ())
            {
            double actualLaidOutTextBlockLength = ComputeLaidOutTextBlockLength ();
            
            this->ChangeInterCharSpacing (1.0);
            double A = ComputeLaidOutTextBlockLength ();
            
            this->ChangeInterCharSpacing (2.0);
            double W = ComputeLaidOutTextBlockLength () - A;

            // Calculate a new inter char spacing based on the above values.
            double newInterCharSpacing = 1.0;
            
            if (fabs (W) > DWG_CHARACTERSPACING_COMPARISON_TOLERANCE)
                newInterCharSpacing = 1.0 + (actualLaidOutTextBlockLength - A) / W;

            this->ChangeInterCharSpacing (newInterCharSpacing);
            }

        m_textBlock.SetType (TEXTBLOCK_TYPE_DwgMText);
        
        this->ConvertLineSpacingToExactlyLineSpacing ();

        m_textBlock.SetUserOrigin (dgnUserOrigin);

        this->CorrectTextForMarginJustification ();
        }

}; // DgnDwgMTextConverter

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/08
//---------------------------------------------------------------------------------------
void TextBlock::ConvertToMTextCompatibleTextBlock ()
    {
    if (IsEmpty ())
        return;

    DgnDwgMTextConverter dgnDwgMTextConverter (*this);
    dgnDwgMTextConverter.Convert ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
bool TextBlock::IsDTextCompatible () const
    {
    if (TEXTBLOCK_TYPE_DwgMText == GetType ())
        return false;

    Caret caret = this->Begin ();

    if (NULL == caret.GetCurrentRunCP ())
        return false;

    if (m_properties.GetDocumentWidth () > 0.0)
        return false;

    if (GetParagraphCount () > 1)
        return false;

    ParagraphP paragraph = GetParagraph (0);

    if (paragraph->GetLineCount () > 1 || 0.0 != paragraph->GetLineSpacing ())
        return false;

    if (NULL == GetFirstRunProperties ())
        return false;

    if (IsAlongElement ())
        return false;

    RunPropertiesCP firstRunProperties = &caret.GetCurrentRunCP ()->GetProperties ();

    if (firstRunProperties->GetCharacterSpacingValue () > 0.0)
        return false;

    while (SUCCESS == caret.MoveToNextRun ())
        {
        RunPropertiesCP runProperties = &caret.GetCurrentRunCP ()->GetProperties ();
        
        if (!runProperties->GetFontSize ().isEqual (&firstRunProperties->GetFontSize ()))
            return false;

        if (!runProperties->Equals (*firstRunProperties))
            {
            RunProperties currRunProperties = *runProperties;

            // Exclude properties that are supported by DTEXT:
            currRunProperties.SetIsOverlined (firstRunProperties->IsOverlined ());
            currRunProperties.SetIsUnderlined (firstRunProperties->IsUnderlined ());

            if (!currRunProperties.Equals (*firstRunProperties))
                return false;
            }

        if (runProperties->m_overrides.m_font || runProperties->m_overrides.m_shxBigFont || runProperties->m_overrides.m_isBold)
            return false;
        }

    caret = this->Begin ();
    RunCP run = caret.GetCurrentRunCP ();
    if ((NULL != dynamic_cast <ParagraphBreakCP> (run) || NULL != dynamic_cast <LineBreakCP> (run) || NULL != dynamic_cast <TabCP> (run)))
        return false;

    return (SUCCESS != caret.MoveToNextRun () || NULL == caret.GetCurrentRunCP ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
bool TextBlock::IsMTextCompatible (DgnModelP modelRef) const
    {
    if (IsAlongElement ())
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
bool TextBlock::IsValidUserOriginForDText () const
    {
    ParagraphP  paragraph   = GetParagraph (0);
    DRange3d    range       = paragraph->ComputeTransformedNominalRange ();
    DVec3d      offset      = GetJustificationOffset ();

    RotMatrix textRMatrix = GetOrientation ();
    textRMatrix.multiply (&offset, &offset);

    DPoint3d textOrigin = GetTextOrigin ();
    textOrigin.sumOf (&textOrigin, &offset, 1.0);

    DPoint3d textUserOrigin = GetUserOrigin ();
    if (textOrigin.distance (&textUserOrigin) > range.low.y * 0.1) // 10% tolerance
        return  false;
    
    return  true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void TextBlock::ComputeUserOriginForDText (DPoint3dR userOrigin) const
    {
    DVec3d      offset      = GetJustificationOffset ();

    RotMatrix textRMatrix = GetOrientation ();
    textRMatrix.multiply (&offset, &offset);

    DPoint3d textOrigin = GetTextOrigin ();
    userOrigin.sumOf (&textOrigin, &offset, 1.0);
    }
