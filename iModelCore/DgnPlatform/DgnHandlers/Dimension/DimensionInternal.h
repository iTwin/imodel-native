/*----------------------------------------------------------------------+                            
|
|   $Source: DgnHandlers/Dimension/DimensionInternal.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma once

#if defined (NEEDS_WORK_DGNITEM)
/* .h File Dependencies */
#include <Bentley/ScopedArray.h>
#include <DgnPlatform/DgnHandlers/MdlTextInternal.h>
#include <DgnPlatform/DgnHandlers/TextBlock/TextAPICommon.h>

#define CHECK_LOCATE               1
#define CHECK_SNAP                 2

#define ESC                       27
#define ABORT                      1
#define WONT_FIT                  -2

#define DIM_STROKED_NONE          0
#define DIM_STROKED_DIM           1
#define DIM_STROKED_PROXY         2
#define DIM_STROKED_DIMANDPROXY   3

/* Dimension command numbers  */
#define LABEL_LINE               213
#define SIZE_ARROW               214
#define SIZE_STROKE              215
#define LOCATE_SINGLE            216
#define LOCATE_STACKED           217
#define ORDINATE                 297
#define CUSTOM_LINEAR            364

#define ANGLE_SIZE               224
#define ANGLE_LOCATION           225
#define ANGLE_LINES              361
#define ARC_SIZE                 226
#define ARC_LOCATION             227

/* Dimension Command (1 - 24 index into dimension template) */
#define MAX_DIMSTR        MAX_DIMSTRING
extern const WChar DIMMLTEXT_ValuePlaceHolder[];

/* Command Names (cmdname.txt) used during dimension modification */
#define CN_MODIFY_ELEMENT  92
#define CN_INSERT_VERTEX  138

/* Dimension error messages */
#define ERRMSG_ASSOC         2097      /* Errors.r (base = 2000)            */

/* Used by adim_generateDimLine */
#define TRIM_NONE       (0)
#define TRIM_LEFT       (1<<0)
#define TRIM_RIGHT      (1<<1)
#define TRIM_BOTH       (TRIM_LEFT | TRIM_RIGHT)

#define AUXSYM_BUILTIN   1        /* Generated symbol                  */
#define AUXSYM_SYMBOL    2        /* Character from symbol font        */
#define AUXSYM_CELL      3        /* Shared cell symbol                */

#define CURRENT_DIM_CHECKSUM_TYPE   3

BEGIN_BENTLEY_DGN_NAMESPACE

typedef struct _elementProcess
    {
    int                             elmType;        /* Primitive type from locate (not passed)      */
    int                             segment;        /* Element segment            (not passed)      */
    int                             elmNumber;      /* Element number                               */
    int                             flags;          /* Private flags for use in compound strokers       */
    int                             lineNo;         /* Line number (used only in multi-lines)       */
    bool                            is3d;           // is original element being hatched a 3D element
    } ElementProcess;

/*-----------------------------------------------------------------------
Dimension symbols - two prefix, two suffix
-----------------------------------------------------------------------*/
struct DimStackInfo
    {
    int         segNo;            /* segment of stack height requested */
    double      height;           /* stacked (relative) height from 1st dimension line */
    };

struct DimAuxSymbol
    {
    double      width;            /* Symbol width                      */
    uint64_t cellId;           /* Id of shared cell definition      */

    Byte type;             /* Symbol type (AUXSYM_ ) 0 = none   */
    Byte index;            /* Index of built in symbol          */
    unsigned short symbol;           /* Symbol character                  */
    uint32_t    font;             /* Symbol font                       */
    };

struct DimStringData
    {
    double         charWidth;

    DPoint2d       textTile;
    DPoint2d       tolTile;

    RotMatrix      rMatrix;

    double         preSymMar;
    double         sufSymMar;
    double         symWidth;
    DimAuxSymbol   auxSym[4];

    DPoint2d       upperSize;
    DPoint2d       lowerSize;
    
    DimStrings     m_strings;

    DimStringData ()
        {
        charWidth = 0;
        DPoint2d zero = {0};
        upperSize = lowerSize = textTile = tolTile = zero;
        memset (&rMatrix, 0, sizeof(rMatrix));
        preSymMar = sufSymMar = symWidth = 0;
        memset (auxSym, 0, sizeof(auxSym));
        }

    WString*    GetPrimaryStrings()                 { return m_strings.GetString (DIMTEXTPART_Primary,   DIMTEXTSUBPART_Main); }
    WString*    GetSecondaryStrings ()              { return m_strings.GetString (DIMTEXTPART_Secondary, DIMTEXTSUBPART_Main); }
    };

struct DimOptionBlockHeader
    {
    Byte nWords;
    Byte type;
    };

/*---------------------------------------------------------------------------------------
    Dimension primitive element symbology
---------------------------------------------------------------------------------------*/
enum DimMaterialIndex
    {
    DIM_MATERIAL_DimLine       = 0,
    DIM_MATERIAL_Extension     = 1,
    DIM_MATERIAL_Terminator    = 2,
    DIM_MATERIAL_Text          = 3,
    DIM_MATERIAL_MLUserText    = 4,

    DIM_MATERIAL_MAX           = 5,
    };

#define ADIMSEGMENTTEXTBOXES_TYPED

/*---------------------------------------------------------------------------------------
    Structures used to carry over dimension override information.
---------------------------------------------------------------------------------------*/
typedef struct
    {
    DimOverrideHdr      hdr;
    uint16_t            pointNo;
    DimPointOverrides   point;
    } PointOverride;

typedef struct
    {
    DimOverrideHdr      hdr;
    uint16_t            segmentNo;
    DimSegmentOverrides segment;
    } SegmentOverride;

typedef struct
    {
    DimOverrideHdr          hdr;
    uint16_t                segmentNo;
    DimSegmentFlagOverrides sgmtflg;
    } SegmentFlagOverride;

typedef struct
    {
    DimOverrideHdr      hdr;
    uint16_t            unused;
    DimOverallOverrides overall;
    } OverallOverride;

typedef struct
    {
    DimOverrideHdr       hdr;
    uint16_t             unused;
    DimStyleExtensions   styleExt;
    } StyleExtension;

typedef struct
    {
    DimOverrideHdr      hdr;
    uint16_t            unused;
    DimOrdinateOverrides ordinate;
    } OrdinateOverride;

typedef struct
    {
    DimOverrideHdr      hdr;
    uint16_t            segmentNo;
    DimMLText           *pText;
    } DimMLTextOverride;

struct DimOverrides
    {
    int                 nStyleExt;
    int                 nOverall;
    int                 nSegments;
    int                 nSgmtFlgs;
    int                 nPoints;
    int                 nOrdinate;
    int                 nMLTexts;
    StyleExtension      *pStyleExt;
    OverallOverride     *pOverall;
    SegmentOverride     *pSegment;
    SegmentFlagOverride *pSgmtFlag;
    PointOverride       *pPoint;
    OrdinateOverride    *pOrdinate;
    DimMLTextOverride   *pMLText;
    };

enum    DimProxyOverride
    {
    DIMPROXY_OVERRIDE_None          = 0,
    DIMPROXY_OVERRIDE_NoProxy       = 1,
    DIMPROXY_OVERRIDE_NotOnlyProxy  = 2,
    };
struct AdimProcess
    {
private:
    ElementHandleCR     m_dimensionElement;
    void                InitRotMatrix ();
public:
    ViewContextP        context;
    ElementProcess      ep;
    
    DPoint3dP           points;
    RotMatrix           rMatrix;
    DimArcInfo          dimArc;

    struct
        {
        unsigned short allDontFit:1;       // text+margins+terminators do not fit (manual offset is not considered fit)
        unsigned short textNotFit:1;       // text alone does not fit
        unsigned short fitTermsInside:1;   // keep terminators inside even if allDontFit=true
        unsigned short pushTextOutside:1;  // push text outside if allDontFit=true
        unsigned short firstSeg:1;
        unsigned short lastSeg:1;
        unsigned short textBlockPopulated:1;
        unsigned short proxyStroked:1;
        unsigned short embed:1;
        unsigned short fitOption:3;
        unsigned short tightFitTextAbove:1;
        unsigned short ignoreMinLeader:1;  // ignore minimum leader dim->geom.margin and activates auto min leader
        unsigned short      unused:2;
        }           flags;

    DimStringData   strDat;
    double          stackHeight;
    uint32_t        partName;
    RotMatrix       vuMatrix;
    DimStackInfo    stack;
    DimOverrides    *pOverrides;
    double          *pdDimLengths;
    BitMaskP         pLeaderedMask;
    TextBlockPtr    m_textBlock;
    DimProxyOverride    proxyOverride;

    AdimRotatedTextBox      textBox[2];
    AdimSegmentTextBoxes   *pTextBoxes;

    LegacyTextStyle       *pTextStyle;
    DimDerivedData  *pDerivedData;
    double          dProjectedTextWidth;

    DimensionElm const* GetDimElementCP() const {return (DimensionElm const*) m_dimensionElement.GetGraphicsCP();}
    DgnModelP       GetDgnModelP() const {BeAssert(NULL != m_dimensionElement.GetDgnModelP()); return m_dimensionElement.GetDgnModelP();}
    ElementHandleCR    GetElemHandleCR() const {return m_dimensionElement;}
    
    AdimProcess (ElementHandleCR element, ViewContextP context);
    ~ AdimProcess();
    void Init ();
    } ;

struct DimFormattedText
{
private:
    typedef bvector<Byte> ByteVector;

    uint16_t    m_component;

    uint32_t    m_nodeNumber;         // stored only if text node
    DPoint2d    m_origin;
    DPoint2d    m_scale;              // text block width and height 'range'
    DPoint2d    m_size;               // text character width and height
    double      m_rotation;           // text rotation in radians

    ByteVector      m_variStringBuffer;
    TextParamWide   m_textParams;

public:

    enum ComponentID
        {
        COMPONENTID_PreValue       = 0,     // Text before dimension value
        COMPONENTID_Prefix         = 1,     // Text immediately before value
        COMPONENTID_Value          = 2,     // Dimension value text & prefix
        COMPONENTID_Suffix         = 3,     // Text immediately after value
        COMPONENTID_PostValue      = 4,     // Text after dimension value
        //COMPONENTID_Note           = 5,   // Text below dimension line
        COMPONENTID_NodeProperties = 7,     // Text node properties
        };

    static const ComponentID s_firstComponentID = COMPONENTID_PreValue;
    static const ComponentID s_lastComponentID  = COMPONENTID_NodeProperties;


DimFormattedText ();

// Getters
uint16_t        GetComponentID () const    { return m_component; }
WString         GetString (DgnDbR file) const;
TextParamWide   GetTextParamWide () const;
DPoint2d        GetOrigin () const         { return m_origin; }
DPoint2d        GetScale () const          { return m_scale; }
double          GetWidth () const          { return m_size.x; }
double          GetHeight () const         { return m_size.y; }
double          GetRotation () const       { return m_rotation; }
uint32_t        GetNodeNumber () const     { return m_nodeNumber; }
bool            IsNodeComponent () const   { return COMPONENTID_NodeProperties == GetComponentID(); }
int             GetCrCount () const        { return m_textParams.exFlags.crCount; }
uint16_t        GetJustification () const  { return (uint16_t)m_textParams.just; }
DPoint2d        GetTileSize () const       { return m_size; }

// Setters
void            SetComponentID (uint16_t val)             { m_component  = val; }
void            SetString (WCharCP val, DgnDbR file);
void            SetTextParamWide (TextParamWideCR val);
void            SetOrigin (DPoint2dCR val)              { m_origin     = val; }
void            SetScale (DPoint2dR val)                { m_scale      = val; }
void            SetWidth (double val)                   { m_size.x     = val; }
void            SetHeight (double val)                  { m_size.y     = val; }
void            SetRotation (double val)                { m_rotation   = val; }
void            SetNodeNumber (uint32_t val)              { m_nodeNumber = val; }

// Used for serialization / deserialization
int             GetLinkageMaxNumBytes() const;
TextFormattingLinkage        GetTextParamAsLinkageData () const;
void            SetTextParamFromLinkageData (TextFormattingLinkageCR textLink);
const char*     GetVariString () const;
int             GetVariStringNumBytes () const  { return (int)m_variStringBuffer.size(); }
void            SetVariString (const char* pText, int nBytesIn);

}; // DimFormattedText

/*----------------------------------------------------------------------+
|                                                                       |
|   Dimension primitive part names macros                               |
|                                                                       |
+----------------------------------------------------------------------*/
#define ADIM_SETNAME(dimVar,elmType,subType)    (dimVar = (dimVar&0x0000ff00) | (elmType?(elmType<<4):(dimVar&0x000000f0)) | subType)

#define ADIM_SETSEG(dimVar,segNo)               (dimVar = (segNo << 8) | (dimVar&0x000000ff))

struct AdimStringUtil
{
//static size_t  MSWCharFromVariChar (WCharP outUnicodeString, UInt32 outNumChars, char const * inVariCharString, DgnFontCR effectiveFont);
//static size_t  MSWCharFromVariChar (WCharP outUnicodeString, UInt32 outNumChars, char const * inVariCharString, UInt16 fontNumber, UInt16 shxBigFontNumber, DgnDbR dgnFile);
static WString WStringFromVariChar (char const * inVariCharString, DgnFontCR effectiveFont);
static WString WStringFromVariChar (char const * inVariCharString, uint16_t fontNumber, uint16_t shxBigFontNumber, DgnDbR dgnFile);

static void VariCharFromMSWChar (char* outVariCharString, size_t outNumBytes, WCharCP inUnicodeString, DgnFontCR effectiveFont);
static void VariCharFromMSWChar (char* outVariCharString, size_t outNumBytes, WCharCP inUnicodeString, uint16_t fontNumber, uint16_t shxBigFontNumber, DgnDbR dgnFile);
};

END_BENTLEY_DGN_NAMESPACE

DGNPLATFORM_TYPEDEFS(AdimProcess)

BEGIN_BENTLEY_NAMESPACE

void    shiftElementData (Dgn::DimensionElm *dim, char *pStart, int nMove);

int adim_extractTextCluster
(
WString*                        strings,                /* <= Dimension strings         */
Dgn::DimStringConfig*   stringConfig,           /* <= Text configuration        */
ElementHandleCR                 dimElement,             /* => Dimension element buffer   */
int                             pointNo,                /* => Point number of text       */
DgnFontCR                       effectiveFont           /* => Font used for encoding varichar */
);

int     adim_extensionsGetNoteTerminator (Dgn::DimStyleExtensions const& extensions);
void    adim_extensionsSetNoteTerminator (Dgn::DimStyleExtensions& extensions, int value);

int     adim_generateSingleDimension (AdimProcessP , bool, DPoint3dP , DVec3dP );
int     adim_generateTextSymbols (AdimProcessP , double, DPoint2dP, DPoint3dP , DVec3dP );

void     adim_rotateText
(
DVec3dP pDirection,    /* <=> Dimension text direction        */
DPoint3dP pOrigin,       /* <=> Dimension text origin           */
DPoint2dP        pBoxSize,      /* => text frame box size              */
AdimProcessP pAdimProcess   /* => Function used to process elements*/
);

/*---------------------------------------------------------------------------------**//**
* @return       elemproc status SUCCESS/ERROR or -1 not processed -> not enhanced text
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
int      adim_generateTextUsingDescr
(
AdimProcessP ep,
DPoint3dP origin,
DVec3dP direction
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/02
+---------------+---------------+---------------+---------------+---------------+------*/
 StatusInt       adim_loadTextBlockFromMarkedUpString
(
TextBlockR      textBlock,
WCharCP       pString,
Dgn::TextParamWide*  pTextParamWide,
DPoint2dCP      pTileSize,
AdimProcessP     pAdimProcess
);

void    adim_uorToDimString (WString&, Dgn::DimUnitBlock*, WCharCP, WCharCP, double, bool, uint16_t, uint16_t, int, double *, bool, AdimProcessP, unsigned short primaryAccuracy);
void    adim_getTileSize (DPoint2dP, DPoint2dP, ElementHandleCR dimElement);

void adim_getLengthStrings
(
AdimProcessP pAdimProcess,   /* <=> */
double          dimLength,       /*  => Length of dimension line        */
unsigned short  primaryAccuracy
);

int     adim_convertUnits
(
double          *pDistanceUorOut,
double          uorsIn,
Dgn::DimUnitBlock    *pDimUnitsIn,
DgnModelP    pDgnModel
);

void adim_getEffectiveSymbology
(
Dgn::Symbology              *pSymbologyOut,
ElementHandleCR            dimElement,
Dgn::DimMaterialIndex        materialIndex,
uint32_t                partName,
Dgn::DimOverrides           *pOverrides
);

void    adim_harvestTextBoxForDerivedData
(
AdimProcessP ap
);

bool     adim_checkNoLineFlag
(
AdimProcessCP ap,
int                 termIndex
);

/*---------------------------------------------------------------------------+
|                                                                            |
|   adimterm.c - function prototypes                                  |
|                                                                            |
+---------------------------------------------------------------------------*/
int  adim_generateLineTerminator (AdimProcessP , DPoint3dP , DPoint3dP , int, bool);
int  adim_generateTerminator (AdimProcessP , DPoint3dP , DVec3dP , int);

/*---------------------------------------------------------------------------+
|                                                                            |
|   adimutil.c - function prototypes                                  |
|                                                                            |
+---------------------------------------------------------------------------*/
int     adim_generateLine (AdimProcessP , DPoint3dP , DPoint3dP , Dgn::DimMaterialIndex);
int     adim_generateLineString (AdimProcessP , DPoint3dP , int, int, Dgn::DimMaterialIndex);
int     adim_generateText (AdimProcessP , WCharCP, DPoint3dP , DVec3dP , Dgn::TextElementJustification, DPoint2dP);
int     adim_generateSymbol (AdimProcessP , int, uint16_t, DPoint3dP , RotMatrixP , DPoint2dP, Dgn::TextElementJustification, Dgn::DimMaterialIndex);
int     adim_generateArc (AdimProcessCP , DPoint3dCP, double, double, RotMatrixCP, double, double, Dgn::DimMaterialIndex material = Dgn::DIM_MATERIAL_DimLine);
int     adim_generateArcByPoints (AdimProcessP , DPoint3dP , Dgn::DimMaterialIndex material);
int     adim_generateCircle (AdimProcessP , DPoint3dP , double, RotMatrixP , bool, Dgn::DimMaterialIndex);
int     adim_generateCell (AdimProcessP , uint64_t , DPoint3dP , RotMatrixP , double, double, Dgn::DimMaterialIndex);
int     adim_generateCellScale (AdimProcessP , uint64_t, DPoint3dP , RotMatrixP , double, Dgn::DimMaterialIndex);
int     adim_findSharedCellDef (double*, double*, DgnElementPtr*, uint64_t, DgnModelP);

void    adim_updateTextBox
(
AdimProcessP     ep,            // <=>
DPoint3dCP      org,
DVec3dCP        baseDir,
RotMatrixCP     rMatrix,
DPoint2dCP      adSize
);

/*---------------------------------------------------------------------------------**//**
* Check whether dimension with leader aka ball-n-chain is in use
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool     adim_checkForLeader
(
uint16_t                *chainType,       /* <=  return the ball and chain type */
AdimProcessCP  const ep,               /*  => working process data           */
double const            offsetY,          /*  => text offset data               */
DPoint2dCP              textSize          /*  => text size data                 */
);

/*---------------------------------------------------------------------------------**//**
* Generate leader portion of the dimension "note" aka ball-and-chain.
* @param        pChainedText    OUT     chained text location
* @param        ep              IN OUT  adim process information
* @param        pOriginPnt      IN      text location
* @param        pStartPnt       IN      leader start point on dimension line
* @param        direction       IN      direction of dimension text
* @param        textSize        IN      text size information
* @bsimethod                                                    petri.niiranen  04/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    adim_generateBallAndChain
(
DPoint3dP                pChainedText,
AdimProcessP             ep,
DPoint3dCP  const   pOriginPnt,
DPoint3dCP  const   pStartPnt,
DVec3dCP  const   direction,
DPoint2dCP              textSize,
double const            offsetY
);

void    adim_getRMatrixFromDir
(
RotMatrixP pOutRMatrix,
DVec3dCP pDirection,
RotMatrixCP pDimRMatrix,
RotMatrixCP pViewRMatrix
);

void    adim_getYVec
(
DVec3dP          pYDir,
DVec3dCP    pXDir,
AdimProcessP pAdimProcess
);

void     adim_offsetText
(
DPoint3dP                output_origin,    /* <= point to be offset             */
DPoint3dCP  const   input_origin,     /* => point to be offset             */
DVec3dCP  const     direction,        /* => line perpindicular to offset  */
double                  distance,         /* => distance to offset             */
AdimProcessP             pAdimProcess
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  02/02
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t adim_getDimTextJustification
(
AdimProcessCP  const pAdimProcess
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    sunand.sandurkar 08/02
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::TextElementJustification adim_getDimTextJustificationHorizontal
(
AdimProcessCP  const pAdimProcess
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  02/02
+---------------+---------------+---------------+---------------+---------------+------*/
double   adim_computeLeftEdgeOffset
(
AdimProcessCP  const pAdimProcess,
double      const        dOffsetIn,
double      const        dWidth
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
double   adim_getIntersectLength
(
DPoint3dCP  const pCheck,
DPoint2dCP            pTextSize,
double   const        dMargin
);

/*---------------------------------------------------------------------------+
|                                                                            |
|   adim.c - function prototypes                                      |
|                                                                            |
+---------------------------------------------------------------------------*/

int adim_modifyDimension
(
DPoint3dP currpoint
);

void adim_dropDimension
(
Dgn::DimensionElm *dim,
uint32_t     filePos,
int          freeze
);


/*---------------------------------------------------------------------------+
|                                                                            |
|   adim1.c - function prototypes                                     |
|                                                                            |
+---------------------------------------------------------------------------*/
int  adim_strokeSizeDimension (Dgn::DimensionElm const*, AdimProcessP , bool);
int  adim_strokeOrdinateDimension (Dgn::DimensionElm const*, AdimProcessP , bool);


int     adim_generateDimension (double*, AdimProcessP , DPoint3dP , DPoint3dP ,
            DPoint3dP , double, Dgn::DimText const*, int, int);

int     adim_generateLinearDimension (AdimProcessP , DPoint3dP , DVec3dP , const double offsetY);

/*---------------------------------------------------------------------------------**//**
* Generate dimension text and symbols.
*
* @param        ep          IN OUT  adim process information
* @param        startpt     IN      dimension line mid point or NULL if not with leader
* @param        origin      IN      dimension text origin point
* @param        direction   IN      text running direction
* @return       element process status
* @bsimethod                                                    petri.niiranen  04/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt adim_generateLinearDimensionWithOffset
(
AdimProcessP             ep,
DPoint3dCP  const   startpt,
DPoint3dCP  const   origin,
DVec3dCP  const   direction,
const double          offsetY
);

/*---------------------------------------------------------------------------+
|                                                                            |
|   l_adtext.c - function prototypes                                  |
|                                                                            |
+---------------------------------------------------------------------------*/
void    adim_getStringSize
(
DPoint2dP       stringSize,
WCharCP       string,
DPoint2dCP      tileSize,
int             fontNo,
AdimProcessP     pAdimProcess
);

int     adim_insertLengthString
(
WString&        dimStg,     /* <=> string to be used in dimension      */
WCharCP       lenStg      /*  => length string for dimension string  */
);

/*----------------------------------------------------------------------+
|                                                                       |
| Sharelib dimension functions                                          |
|                                                                       |
+----------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* Check input string for possible fraction. This check searches for digit-'/'-digit
* pattern.
* @param        pwStringIn      => string to check for fraction
* @param        ppwSlashPos     <= slash position or NULL
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     adim_hasFraction
(
WCharP        pwStringIn,
WCharP*       ppwSlashPos
);

/*---------------------------------------------------------------------------------**//**
* Parse fraction components
*
* @param        pwIntegerOut     <= prevalue and integer text
* @param        pwNumeratorOut   <= numerator text
* @param        pwDenominatorOut <= denominator text
* @param        pwReminingOut    <= text following denominator
* @param        pwStringIn       => string to parse
* @return       StatusInt, SUCCESS if fraction can be parsed
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        adim_parseFraction
(
WCharP      pwIntegerOut,
WCharP      pwNumeratorOut,
WCharP      pwDenominatorOut,
WCharP      pwReminingOut,
WCharCP     pwStringIn
);

void     adim_calcTextOffset
(
DPoint2d&       offset,
int             textJust,
bool            pushRight,
double          lineLength,     /*  => Length of dimension line        */
double          stringLength,   /*  => Length of dimension string      */
double          charWidth,      /*  => Char width used in dim text     */
double          leader,         /*  => Leader length                   */
int             termMode,       /*  => Terminator mode                 */
DPoint2dCP      pTextSize,   /*  => total text size                 */
DVec3dCP pTextDir,    /*  => Text running direction          */
DVec3dCP pDimlineDir, /*  => Dim line running direction      */
AdimProcessP pAdimProcess  /* <=> Function used to process elements */
);

void    adim_unpackFromFourDoubles
(
RotMatrixP rMatrix,
const double    *pQuat,
bool            is3D
);

void     adim_packToFourDoubles
(
double             *pQuat,
RotMatrixCP rmP,
bool                is3D
);

int      adim_setStringLinkage
(
ElementGraphicsP pElementIn,
const WChar  *pStringIn,
int             categoryIn,
int             linkageKeyIn,
int             maxStringSizeIn
);

int      adim_setStringLinkageUsingDescr
(
DgnElementDescrH ppDescrIn,
WChar         *pStringIn,
int             categoryIn,
int             linkageKeyIn,
int             maxStringSizeIn
);

int      adim_getStringLinkage
(
WChar         *pStringOut,
ElementGraphicsCP  pElementIn,
int             categoryIn,
int             linkageKeyIn,
int             maxStringSizeIn
);

int      adim_deleteStringLinkage
(
ElementGraphicsP pElementIn,    /* <=> */
int         categoryIn,     /* => */
int         linkageKeyIn    /* => */
);

void adim_setHitDetail
(
AdimProcessCP ep             /* => Function used to process element    */
);

int      mdlDim_setCellName
(
ElementGraphicsP pElementIn,        /* <=> */
WChar     *pCellNameIn,       /*  => */
int         cellFlag            /* DIMCELL_xxx */
);

int      mdlDim_setCellNameUsingDescr
(
DgnElementDescrH ppDescrIn,
WChar         *pCellNameIn,       /*  => */
int             cellFlag                /* DIMCELL_xxx */
);

int      mdlDim_setCellName
(
ElementGraphicsP pElementIn,        /* <=> */
WChar     *pCellNameIn,       /*  => */
int         cellFlag            /* DIMCELL_xxx */
);

int      mdlDim_getCellName
(
WChar     *pCellNameOut,  /* <= */
ElementGraphicsP pElementIn,    /* <=> */
int         cellFlag        /* => DIMCELL_xxx */
);

int      mdlDim_removeCellName
(
ElementGraphicsP pElementIn,
int         cellFlag
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public WString* adim_getSimpleStringPtrByType
(
Dgn::DimStrings  *pwDimStrings,
int             iPartType,
int             iSubType
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
double  adim_getTrimDistance
(
AdimProcessP ap,
int             termIndex,
Dgn::DimTermBlock const *pTermBlock,
bool            sameSide
);

StatusInt    adim_generateDimLine
(
AdimProcessP ap,
DPoint3dP pLeftPt,
DPoint3dP pRightPt,
Dgn::DimMaterialIndex    material,
int                 leftTermIn,
int                 rightTermIn,
int                 trimCode,
bool                lineInside,
bool                termsInside,
bool                bLeftToRight
);

StatusInt    adim_generateExtensionLine
(
AdimProcessP ap,            /* => Adim Process                        */
DPoint3dP start,         /* => Start point of line                 */
DPoint3dP end,           /* => End point of line                   */
bool                useExtSymb     /* => Use extension symbology             */
);

StatusInt    adim_generateBSpline
(
DPoint3dP termDir,               /* <=  */
AdimProcessP                 ep,                     /*  => */
int                         nPoints,                /*  => */
DPoint3dP dPoints,               /*  => */
DPoint3dCP  const startTangent,           /*  => can be (0,0,0) */
DPoint3dCP  const endTangent              /*  => can be (0,0,0) */
);


void     adim_changeTextHeapEncoding
(
EditElementHandleR dimElement,
uint32_t        newFont,
uint32_t        newShxBigFont,
uint32_t        oldFont,
uint32_t        oldShxBigFont
);

/*------------------------------------------------------------------------------
* WARNING: It is typically not safe to call this function directly. Use
*          mdlDim_setTextStyle2.
------------------------------------------------------------------------------*/
StatusInt     adim_setTextStyle
(
EditElementHandleR  dimElm,             /* => */
const Dgn::LegacyTextStyle  *pTextStyle,        /* => */
bool                sizeChangeAllowed   /* => */
);

/*---------------------------------------------------------------------------------------
    Dimension override collection functions
---------------------------------------------------------------------------------------*/
StatusInt    mdlDim_overridesGet
(
Dgn::DimOverrides    **ppOverridesOut,
ElementHandleCR    elementIn
);

void     mdlDim_overridesPointInserted
(
Dgn::DimOverrides    *pOverridesIn,
int             pointNo
);

StatusInt    mdlDim_overridesSet
(
EditElementHandleR elementIn,
Dgn::DimOverrides    *pOverridesIn
);

void     mdlDim_overridesFreeAll
(
Dgn::DimOverrides    **ppOverridesIn
);

/*---------------------------------------------------------------------------------------
    Dimension override collection parameter functions
---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* Get extension property
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        defaultValueIn  =>
* @return       true if extension exists
* @bsimethod                                                    JoshSchifter    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     mdlDim_extensionsGetPrimaryTolAccuracy
(
uint16_t        *pProperty,
Dgn::DimOverrides    *pOverridesIn,
uint16_t        defaultValueIn
);

bool     mdlDim_extensionsGetSecondaryTolAccuracy
(
uint16_t        *pProperty,
Dgn::DimOverrides    *pOverridesIn,
uint16_t        defaultValueIn
);

void     mdlDim_extensionsGetOrdinateUseDatumValueFlag
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

void     mdlDim_extensionsGetOrdinateReverseDecrementFlag
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

void     mdlDim_extensionsGetOrdinateFreeLocationFlag
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

StatusInt    mdlDim_setOrdinateFreeLocationFlag
(
EditElementHandleR     dimElement,
bool                *pFlagValue
);

void     mdlDim_extensionsGetLabelLineSuppressAngleFlag
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

void     mdlDim_extensionsGetLabelLineSuppressLengthFlag
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

void     mdlDim_extensionsGetLabelLineInvertLabelsFlag
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

void     mdlDim_extensionsGetLabelLineAdjacentLabelsFlag
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

void     mdlDim_extensionsGetMultiJustVerticalFlag
(
int             *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             defaultValueIn
);

void     mdlDim_extensionsGetNoReduceFractionFlag
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

void     mdlDim_extensionsGetNoReduceAltFractionFlag
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

void     mdlDim_extensionsGetNoReduceTolFractionFlag
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

void     mdlDim_extensionsGetNoReduceSecFractionFlag
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

void     mdlDim_extensionsGetNoReduceAltSecFractionFlag
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

void     mdlDim_extensionsGetNoReduceTolSecFractionFlag
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

void     mdlDim_extensionsGetNoteLeaderType
(
int             *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

void     mdlDim_extensionsGetNoteTerminator
(
int             *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             defaultValueIn
);

void     mdlDim_extensionsGetNoteTerminatorType
(
int             *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             defaultValueIn
);

void     mdlDim_extensionsGetNoteTextRotation
(
int             *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             defaultValueIn
);

void     mdlDim_extensionsGetNoteHorAttachment
(
int             *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             defaultValueIn
);

void     mdlDim_extensionsGetNoteVerLeftAttachment
(
int             *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             defaultValueIn
);

void     mdlDim_extensionsGetNoteVerRightAttachment
(
int             *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             defaultValueIn
);

void     mdlDim_extensionsGetNotUseModelAnnotationScaleFlag
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

bool     mdlDim_extensionsGetAnnotationScale
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
double          defaultValueIn
);

bool     mdlDim_extensionsGetRoundOff
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
double          defaultValueIn
);

bool     mdlDim_extensionsGetSecondaryRoundOff
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
double          defaultValueIn
);

bool     mdlDim_extensionsGetOrdinateDatumValue
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
double          defaultValueIn
);

bool     mdlDim_extensionsGetNoteElbowLength
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
double          defaultValueIn
);

bool     mdlDim_extensionsGetStackedFractionScale
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
double          defaultValueIn
);

bool     mdlDim_extensionsGetInlineTextLift
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
double          defaultValueIn
);

bool     mdlDim_extensionsGetNoteLeftMargin
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
double          defaultValueIn
);

bool     mdlDim_extensionsGetNoteLowerMargin
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
double          defaultValueIn
);

bool     mdlDim_extensionsGetNoteTermChar
(
uint16_t        *pProperty,
Dgn::DimOverrides    *pOverridesIn,
uint16_t        defaultValueIn
);

bool     mdlDim_extensionsGetNoteTermFont
(
uint32_t        *pProperty,
Dgn::DimOverrides    *pOverridesIn,
uint32_t        defaultValueIn
);

bool     mdlDim_extensionsGetBncElbowLength
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
double          defaultValueIn
);

bool     mdlDim_isMinLeaderIgnored
(
Dgn::DimOverrides    *pOverridesIn,
bool            defaultValueIn
);

bool     mdlDim_extensionsGetFitOption
(
uint16_t        *pProperty,
Dgn::DimOverrides    *pOverridesIn,
uint16_t        defaultValueIn
);

bool     mdlDim_extensionsGetSuppressUnfitTerminatorsFlag
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool             defaultValueIn
);

bool     mdlDim_extensionsGetPushTextRight
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool             defaultValueIn
);

bool     mdlDim_extensionsGetTightFitTextAbove
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool             defaultValueIn
);

bool     mdlDim_extensionsGetAutoBallNChain
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool             defaultValueIn
);

bool     mdlDim_extensionsGetFitInclinedTextBox
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool             defaultValueIn
);

bool     mdlDim_extensionsGetExtendDimLineUnderText
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
bool             defaultValueIn
);

DGNPLATFORM_EXPORT bool     adim_updateExtensionsFromDirectionFormat
(
Dgn::DimStyleExtensions&    elmExtensions,
DirectionFormatterCR                dirFormat
);

bool     mdlDim_extensionsGetDirectionMode
(
Dgn::DirectionMode  *pProperty,
Dgn::DimOverrides            *pOverridesIn
);

bool     mdlDim_extensionsGetDirectionBaseDir
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn
);

bool     mdlDim_extensionsGetDirectionClockwise
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn
);

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    JoshSchifter    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     mdlDim_overridesGetOverallRefScale
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
double          defaultValueIn
);

bool     mdlDim_overridesGetOverallAngleQuadrant
(
uint16_t        *pProperty,
Dgn::DimOverrides    *pOverridesIn,
uint16_t        defaultValueIn
);

bool     mdlDim_overridesGetOverallSlantAngle
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
double          defaultValueIn
);

/*---------------------------------------------------------------------------------**//**
* Get witnessline property.
*
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        pointNo         =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     mdlDim_overridesGetPointWitnessExtend
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             pointNo,
double          defaultValueIn
);

bool     mdlDim_overridesGetPointWitnessOffset
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             pointNo,
double          defaultValueIn
);

bool     mdlDim_overridesGetPointWitnessColor
(
uint32_t        *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             pointNo,
uint32_t        defaultValueIn
);

bool     mdlDim_overridesGetPointWitnessWeight
(
uint32_t        *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             pointNo,
uint32_t        defaultValueIn
);

bool     mdlDim_overridesGetPointWitnessStyle
(
long            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             pointNo,
long            defaultValueIn
);

/*---------------------------------------------------------------------------------**//**
* Get segment property.
*
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        segmentNo       =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     mdlDim_overridesGetSegmentTextRotation
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             segmentNo,
double          defaultValueIn
);

bool     mdlDim_overridesGetSegmentTextJustification
(
uint16_t        *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             segmentNo,
uint16_t        defaultValueIn
);

/*---------------------------------------------------------------------------------**//**
* Get segment property.
*
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        segmentNo       =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    SunandSandurkar 12/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool     mdlDim_overridesGetSegmentCurveStartTangent
(
DPoint3dP pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             segmentNo,
DPoint3dP defaultValueIn
);

bool     mdlDim_overridesGetSegmentCurveEndTangent
(
DPoint3dP pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             segmentNo,
DPoint3dP defaultValueIn
);

/*---------------------------------------------------------------------------------**//**
* Get segment flag property.
*
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        segmentNo       =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     mdlDim_overridesGetSegmentFlagUnderlineText
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             segmentNo,
bool            defaultValueIn
);

bool     mdlDim_overridesGetSegmentFlagSuppressLeftDimLine
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             segmentNo,
bool            defaultValueIn
);

bool     mdlDim_overridesGetSegmentFlagSuppressRightDimLine
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             segmentNo,
bool            defaultValueIn
);

bool     mdlDim_overridesGetSegmentFlagPrimaryIsReference
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             segmentNo,
bool            defaultValueIn
);

bool     mdlDim_overridesGetSegmentFlagSecondaryIsReference
(
bool            *pProperty,
Dgn::DimOverrides    *pOverridesIn,
int             segmentNo,
int             defaultValueIn
);

/*---------------------------------------------------------------------------------**//**
* Get ordinate property
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     mdlDim_overridesGetOrdinateStartValueX
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
double          defaultValueIn
);

bool     mdlDim_overridesGetOrdinateStartValueY
(
double          *pProperty,
Dgn::DimOverrides    *pOverridesIn,
double          defaultValueIn
);

/*---------------------------------------------------------------------------------**//**
* Get text dimMLText blob for given segment.
* @param    ppText        <= found text
* @param    pOverridesIn  =>
* @param    segmentNo     =>
* @return   SUCCESS, if dimMLText exists; otherwise ERROR.
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        mdlDim_overridesGetDimMLText
(
Dgn::DimMLText       **ppText,
Dgn::DimOverrides    *pOverridesIn,
int             segmentNo
);

/*---------------------------------------------------------------------------------**//**
* Clear segment flag property.
*
* @param        pOverridesIn    <=>
* @param        segmentNo       =>
* @param        propertyField   =>
* @return   SUCCESS, if override exist and got cleared.
* @bsimethod                                                    petri.niiranen  02/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     mdlDim_overridesClearSegmentPropertyBit
(
Dgn::DimOverrides*   pOverridesIn,
const int       segmentNo,
const uint32_t  propertyField
);

/*---------------------------------------------------------------------------------**//**
* Get the text location flag.
* @param    pProperty       <=  text location property
* @param    pOverridesIn     => overrides struct pointer
* @param    pElm             => dim element
* @return       true if override exists
* @bsimethod                                                    SunandSandurkar 12/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool                 mdlDim_overridesGetTextLocation
(
Dgn::DimStyleProp_Text_Location  *pValueOut,
Dgn::DimOverrides                *pOverridesIn,
bool                        embedOveride
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
enum AdimTextSizeOption
{
ADIM_TEXTSIZE_Exact     = 0,
ADIM_TEXTSIZE_Nominal   = 1,
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        dimTextBlock_populateWithValues
(
AdimProcessP            pAdimProcess,
Dgn::DimMLText* pText,
WString*                strings
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       dimTextBlock_getTextSize
(
AdimProcessP        pAdimProcess,
DPoint2dP           pSizeOut,
AdimTextSizeOption  sizeOption
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementDescrPtr dimTextBlock_getTextDescr
(
AdimProcessP pAdimProcess,
Dgn::DimMLText       *pText,
DPoint3dP pTxtOrigin,
DVec3dP pDirection
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     dimTextBlock_getRequiresTextBlock
(
AdimProcessP pAdimProcess,
Dgn::DimMLText       **ppTextOut
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
void     adim_getTextSize
(
DPoint2dP                   pTextSize,
AdimProcessP                pAdimProcess,
Dgn::DimStringData* pDstr,
AdimTextSizeOption          sizeOption
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool            dimTextBlock_stringContainsMarkup
(
WCharCP       string
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        dimTextBlock_appendMarkedUpString
(
TextBlockR      textBlock,
WCharCP       pwString,
Dgn::TextParamWide*  pTextParamWide,
DPoint2dCP      pTextSize,
AdimProcessP     pAdimProcess
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
void     dimTextBlock_setPopulated
(
AdimProcessP pAdimProcess,
bool            bState
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     dimTextBlock_isPopulated
(
AdimProcessP pAdimProcess
);

StatusInt   adimUtil_scaleDimValue
(
EditElementHandleR dimElement,
double          scale,
bool            modifyAllowed
);


void* mdlDim_getEditOptionBlock (EditElementHandleR dimElement, int reqType, uint64_t*);
void const* mdlDim_getEditOptionBlockFromElement (ElementGraphicsCR , ElementHandleCR dimElement, int reqType, uint64_t*);

void     adim_setDimTextWitnessLineFromTemplate
(
ElementHandleCR        dimElement,
Dgn::DimText *           dimText,
int                 iPoint,
bool                noWitness   /* => true: no witness, false: use template */
);

StatusInt        adim_allocateAndExtractTextStyleForStroke
(
LegacyTextStyleP          *ppTextStyle,
ElementHandleCR      dimElement
);

double  adim_getScaleForCellDynamics
(
DgnModelP        srcDgnModel,
DgnModelP        dstDgnModel
);

Dgn::StackedFractionType adim_textFracTypeFromDim (int dimFracType);

int adim_dimFracTypeFromText (Dgn::StackedFractionType textFracType);


void            adim_getMLNoteVerLeftAttachment
(
const Dgn::DimStyleExtensions            *pExtensions,
uint16_t                            *pValue
);

void            adim_setMLNoteVerLeftAttachment
(
Dgn::DimStyleExtensions                  *pExtensions,
uint16_t                            eValue
);

void            adim_getMLNoteVerRightAttachment
(
const Dgn::DimStyleExtensions            *pExtensions,
uint16_t                            *pValue
);

void            adim_setMLNoteVerRightAttachment
(
Dgn::DimStyleExtensions                  *pExtensions,
uint16_t                            eValue
);

void     adim_setCurrentSegmentTextIsOutside
(
AdimProcessP pAdimProcess,
int             dimJust
);

bool      adim_areTerminatorsBetweenExtensionLines
(
AdimProcessP pAdimProcess,
double              effectiveWidth,
double              fitMargin,
double              dimLineLength,
double              insideMinLeader,
DPoint2dCR          offset,
int                 textJust
);

bool         adim_needExtraTextMargin
(
AdimProcessCP pAdimProcess
);

bool         adim_isDimlineThruEitherTerm
(
AdimProcessCP pAdimProcess
);

void         adim_getEffectiveTerminators
(
int                 *pLeftTerm,
int                 *pRightTerm,
AdimProcessCP pAdimProcess
);

void         adim_getEffectiveMinLeaders
(
double              *pInside,
double              *pOutside,
AdimProcessCP pAdimProcess
);

StatusInt                       mdlDim_getBallNChainMode
(
Dgn::DimStyleProp_BallAndChain_Mode  *pBncMode,
ElementHandleCR
);

StatusInt                mdlDim_getFitOption
(
Dgn::DimStyleProp_FitOptions         *pFitOption,
ElementHandleCR                    dimElement
);

StatusInt                mdlDim_setFitOption
(
EditElementHandleR                 dimElement,
Dgn::DimStyleProp_FitOptions         fitOption
);

bool                     mdlDim_isFrozenInSharedCell
(
ElementGraphicsCP pDimElm
);

int mdlDim_scale2
(
EditElementHandleR dimElement,           /* <=> dimension element to scale */
double          scale,          /*  => scale factor */
bool         modifyAllowed,  /*  => allowed to change element size */
bool         setShields,     /*  => protect scaled sizes from dimstyle */
bool         updateRange     /*  => update element range */
);

double   adim_projectTextSize
(
DPoint2dCP          pTextSize,
DPoint3dCP pTextDir,
double              textMargin
);

bool    adim_useWitnessLineOffset (AdimProcessCP  ep);

void    adim_getFloatingAngularFormat
(
Dgn::AngleFormatVals*   pFormat,       /* <=> */
int*    pAccuracy      /* <=> */
);

StatusInt adim_createUnitBlock
(
Dgn::DimUnitBlock*       pUnitBlock,       /* <= */
bool                        isPrimary,        /* => */
double const*               pUorPerStorage,   /* => */
UnitDefinitionCP    masterUnit,       /* => */
UnitDefinitionCP    subUnit           /* => */
);

StatusInt adim_extractUnitBlock
(
double*             pUorPerStorage,   /* <= */
UnitDefinitionP     masterUnit,       /* <= */
UnitDefinitionP     subUnit,          /* <= */
Dgn::DimUnitBlock*       pUnitBlock        /* => */
);

/*------------------------------------------------------------------------*//**
The mdlDim_getStyleExtension function is used to retrieve style extensions from
the specified dimension element.
* @Param        pExtensionsOut OUT the style extensions retrieved from the dimension
element.
* @Param        pElementIn IN is the dimension element to retrieve the extensions from.
* @Return       SUCCESS if the extensions information is retrieved successfully.
* @ALinkJoin    usmthmdlDim_deleteStyleExtensionC usmthmdlDim_setStyleExtensionC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/

int     adim_strokeDimension (AdimProcessR ep);

END_BENTLEY_NAMESPACE

DGNPLATFORM_TYPEDEFS (IDimElementHelper)

BEGIN_BENTLEY_DGN_NAMESPACE

enum DimensionCategory
    {
    Invalid     = 0,
    Linear      = 1,
    Angular     = 2,
    Radial      = 3,
    Note        = 4,
    LabelLine   = 5
    };

struct DimensionHelperFactory;
typedef RefCountedPtr<IDimElementHelper>    IDimElementHelperPtr;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct          IDimElementHelper : public RefCountedBase
    {
    friend struct DimensionHelperFactory;

    protected:
        ElementHandleCR        m_dimension;
        DimensionHandler &  m_hdlr;
        IDimElementHelper (ElementHandleCR dimension, DimensionHandler& hdlr)
            :m_dimension (dimension), m_hdlr(hdlr)
            {}
        
        void    ResolveTerminators (EditElementHandleR, int iSegment) const;
        bool    CanReturnWithoutStroke () const;
    public:
        virtual BentleyStatus   DropToSegment (ElementAgendaR droppedDimension) const {return ERROR;}
        virtual int             GetNumberofSegments () const = 0;
        virtual StatusInt       StrokeDimension (AdimProcess&) const = 0;
        virtual StatusInt       ReEvaluateElement (EditElementHandleR dimElement) const {return SUCCESS;}
        virtual bool            IsVertexDeletable (HitDetailCP hitPath) const = 0;
        virtual BentleyStatus   DeleteVertex (EditElementHandleR dimElement, int pointNo);
        virtual BentleyStatus   InsertVertex (EditElementHandleR dimElement, DPoint3dCR point, HitDetailCR hitPath, DimensionStyleCR dimStyle) = 0;
        virtual bool            HasText () const {return true;}
    };

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct          AngularDimensionHelper: public IDimElementHelper
    {
    DEFINE_T_SUPER(IDimElementHelper)
    friend struct DimensionHelperFactory;

    AngularDimensionHelper (ElementHandleCR dimension, DimensionHandler& hdlr):IDimElementHelper (dimension, hdlr) {}
    BentleyStatus   DropToSegment (ElementAgendaR droppedDimension) const override;
    int             GetNumberofSegments () const override {return m_hdlr.GetNumPoints (m_dimension) - 2;}
    StatusInt       StrokeDimension (AdimProcess&) const override;
    bool            IsVertexDeletable (HitDetailCP hitPath) const override {return m_hdlr.GetNumPoints (m_dimension) > 3;}
    BentleyStatus   DeleteVertex (EditElementHandleR dimElement, int pointNo) override;
    BentleyStatus   InsertVertex (EditElementHandleR dimElement, DPoint3dCR point, HitDetailCR hitPath, DimensionStyleCR dimStyle) override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct          LinearDimensionHelper: public IDimElementHelper
    {
    DEFINE_T_SUPER(IDimElementHelper)
    friend struct DimensionHelperFactory;
    private:
    void    RecalcTextOffsetForLinearSingleDims(EditElementHandleR segmentDim, ElementHandleCR origDim,
                    bvector<double> const& xTextOffsets, uint32_t iSegment) const;
    double  GetExtensionHeightDifference (int segNo) const;
    
    protected:
    LinearDimensionHelper (ElementHandleCR dimension, DimensionHandler& hdlr):IDimElementHelper (dimension, hdlr) {}
    BentleyStatus   DropToSegment (ElementAgendaR droppedDimension) const override;
    int             GetNumberofSegments () const override{return m_hdlr.GetNumPoints (m_dimension) -1;}
    StatusInt       StrokeDimension (AdimProcess&) const override;
    bool            IsVertexDeletable (HitDetailCP hitPath) const override {return m_hdlr.GetNumPoints (m_dimension) > 2;}
    BentleyStatus   DeleteVertex (EditElementHandleR dimElement, int pointNo) override;
    BentleyStatus   InsertVertex (EditElementHandleR dimElement, DPoint3dCR point, HitDetailCR hitPath, DimensionStyleCR dimStyle) override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct          OrdinateDimensionHelper : public IDimElementHelper
    {
    DEFINE_T_SUPER(IDimElementHelper)
    friend struct DimensionHelperFactory;
    private:
    bool            IsWitnesslineSuppressed (int pointNo) const;

    protected:
    OrdinateDimensionHelper (ElementHandleCR dimension, DimensionHandler& hdlr):IDimElementHelper (dimension, hdlr) {}
    
    BentleyStatus   DropToSegment (ElementAgendaR droppedDimension) const override;
    int             GetNumberofSegments () const override {return m_hdlr.GetNumPoints (m_dimension);}
    StatusInt       StrokeDimension (AdimProcess&) const override;
    StatusInt       ReEvaluateElement (EditElementHandleR dimElement) const override;
    bool            IsVertexDeletable (HitDetailCP hitPath) const override {return m_hdlr.GetNumPoints (m_dimension) > 2;}
    BentleyStatus   InsertVertex (EditElementHandleR dimElement, DPoint3dCR point, HitDetailCR hitPath, DimensionStyleCR dimStyle) override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct          RadialDimensionHelper : public IDimElementHelper
    {
    DEFINE_T_SUPER(IDimElementHelper)
    friend struct DimensionHelperFactory;
    protected:
    RadialDimensionHelper (ElementHandleCR dimension, DimensionHandler& hdlr):IDimElementHelper (dimension, hdlr) {}
    int             GetNumberofSegments () const override {return 1;}
    StatusInt       StrokeDimension (AdimProcess&) const override;
    StatusInt       ReEvaluateElement (EditElementHandleR dimElement) const override;
    bool            IsVertexDeletable (HitDetailCP hitPath) const override;
    BentleyStatus   InsertVertex (EditElementHandleR dimElement, DPoint3dCR point, HitDetailCR hitPath, DimensionStyleCR dimStyle) override;
    bool            HasText () const override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct          LabelLineHelper: public IDimElementHelper
    {
    DEFINE_T_SUPER(IDimElementHelper)
    friend struct DimensionHelperFactory;
    protected:
    LabelLineHelper (ElementHandleCR dimension, DimensionHandler& hdlr):IDimElementHelper (dimension, hdlr) {}
    int             GetNumberofSegments () const override {return 1;}
    StatusInt       StrokeDimension (AdimProcess&) const override;
    StatusInt       ReEvaluateElement (EditElementHandleR dimElement) const override;
    bool            IsVertexDeletable (HitDetailCP hitPath) const override {return false;}
    BentleyStatus   InsertVertex (EditElementHandleR dimElement, DPoint3dCR point, HitDetailCR hitPath, DimensionStyleCR dimStyle) override{return ERROR;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct          NoteDimensionHelper: public IDimElementHelper
    {
    DEFINE_T_SUPER(IDimElementHelper)
    friend struct DimensionHelperFactory;
    protected:
    NoteDimensionHelper (ElementHandleCR dimension, DimensionHandler& hdlr):IDimElementHelper (dimension, hdlr) {}
    int             GetNumberofSegments () const override {return 1;}
    StatusInt       StrokeDimension (AdimProcess&) const override;
    bool            IsVertexDeletable (HitDetailCP hitPath) const override {return m_hdlr.GetNumPoints (m_dimension) > 2;}
    BentleyStatus   DeleteVertex (EditElementHandleR dimElement, int pointNo) override;
    BentleyStatus   InsertVertex (EditElementHandleR dimElement, DPoint3dCR point, HitDetailCR hitPath, DimensionStyleCR dimStyle) override;
    bool            HasText () const override;
    };
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct          DimensionHelperFactory
    {
    static IDimElementHelperPtr CreateHelper (ElementHandleCR element);
    };

StatusInt        mdlDimText_traverseFormatters
(
DimMLTextP                       pText,
PFDimTextFmtTraverseFunction    pUserTraverseFunction,
void                            *pUserData,
DgnModelP                    modelRef
);

END_BENTLEY_DGN_NAMESPACE





Public void     updateTextBoxFromTextBlock
(
AdimProcessP    ep,            // <=>
TextBlockCR     textBlock
);
#endif
