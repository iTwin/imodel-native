/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/lstyle.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include "../DgnPlatform.r.h"

#include    <map>
#include    <Bentley/bvector.h>
#include    "LineStyle.h"

#error none of this should apply.

typedef struct dwgLineStyleInfo* DwgLineStyleInfoP;     // this is outside the linestyle namespace.

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

enum LineStyleTreeInfoFields
    {
    LS_TREE_INFOFIELD_StyleType = 0,
    LS_TREE_INFOFIELD_PropType  = 1,
    LS_TREE_INFOFIELD_PropId    = 2,
    LS_TREE_INFOFIELD_NumFields     // Must be last; number of fields to put on cell.
    };

enum LineStyleDwgExportReason
    {
    LS_DWG_Ok                   = 0,
    LS_DWG_NoComponents         = 1,    // Compound with no components
    LS_DWG_ShxOnlyCompatSet     = 2,    // CAPABILITY_ALLOW_NON_SHX_STYLES is set to false; really limits stuff
    LS_DWG_CompoundHasOffsets   = 3,    // Compound with offsets
    LS_DWG_TooManySegments      = 4,    // Too many segments
    LS_DWG_SymbolAtStartOrEnd   = 5,    // Symbol at start, end, or segment of line rather than part of a dash
    LS_DWG_PartialWidthDash     = 6,    // A dash with left width or right width
    LS_DWG_OpenDashCap          = 7,    // Open dash caps
    LS_DWG_WidthsChange         = 8,    // Widths vary between strokes - one fat, one thin
    LS_DWG_WidthTapers          = 9,    // A dash has taper
    LS_DWG_IterationLimit       = 10,   // Style has limit to number of iterations
    };

typedef struct LsElmComponent*              LsElmComponentP;
typedef struct LsElmComponent const*        LsElmComponentCP;

typedef struct DecomposedDwgLine*           DecomposedDwgLineP;

//=======================================================================================
// @bsiclass                                                      Chuck.Kirschman   06/07
//=======================================================================================
struct LsElmComponent
    {
public:
    virtual ~LsElmComponent () { }
    virtual BentleyStatus FromResource (void* rsc, DgnElementId id, bool createSubComponents) = 0;
    virtual BentleyStatus ToResource (void** rsc) const = 0;
    virtual BentleyStatus FromElement (DgnElementCP elm, bool createSubComponents) = 0;
    virtual BentleyStatus ToElement (DgnElementP* elm) const = 0;
    virtual LsElementType GetType () const = 0;
    virtual DgnElementId GetId () const = 0;
    virtual WCharCP GetName () const = 0;
    virtual bool IsModified () const = 0;

    virtual bool ExportableToDwg (bool checkSegmentCount, LineStyleDwgExportReason& reason) const = 0;
    virtual BentleyStatus _DecomposeForDwg (DecomposedDwgLineP pDDLine) const = 0;
    virtual double GetLength () const = 0;
    virtual bool IsContinuous () const {return false;}
    virtual bool HasComponentsFilled () const = 0; // This data structure can hold chains or individual components.
    };

//=======================================================================================
// @bsiclass                                                      Chuck.Kirschman   06/07
//=======================================================================================
struct LsElmCompoundComponent : public LsElmComponent
    {
public:
    struct LsComponentInfo
    {
        LsElmComponentP     m_subComponent;
        double              m_offset;

        uint32_t            m_type;  // Used if no subcomponent; good for going to-from resources.
        DgnElementId        m_id;

        LsComponentInfo ()
            {
            m_subComponent = NULL;
            m_offset = 0.0;
            m_type = 0;
            }

        void                Destroy                         ();
        uint32_t            GetType                         () const {return m_subComponent ? (uint32_t)m_subComponent->GetType() : m_type;}
        LsElementType       GetLsType                       () const;
        uint32_t            GetRscType                      () const;
        DgnElementId        GetId                           () const {return m_subComponent ? m_subComponent->GetId() : m_id;}
        double              GetOffset                       () const {return m_offset;}
        void                SetOffset                       (double offset) {m_offset = offset;}
        LsElmComponentP     GetLsComponent                 () const {return m_subComponent;}
        BentleyStatus       LoadSubComponentFromResource    (RscFileHandle rfHandle, bool createSubComponents);
        BentleyStatus       LoadSubComponentFromElement     (DgnModelP modelRef, bool createSubComponents);
     };

private:
    bool                m_isModified;
    WChar               m_descr[LS_MAX_DESCR];
    WChar               m_styleName[LS_MAX_NAME]; // Stored on element, for round trip.  Kind of dumb though; remove in future file format.
    DgnElementId        m_id;
    bvector<LsComponentInfo> m_components;

    size_t ResourceSize () const;
#if defined (BEIJING_DGNPLATFORM_WIP_DWG)
#endif
    virtual BentleyStatus   _DecomposeForDwg             (DecomposedDwgLineP  pDDLine) const override {return ERROR;}
    StatusInt               ExtractCompoundComponent    (DwgLineStyleInfoP styleInfo, int32_t* pNSegments, WCharP pStrings, int32_t* pUnitMode,
                                            bool useUnits, WCharCP suggestedFontPathName, DgnModelP modelRef, LsDefinitionP nameRec) const;

public:
    virtual BentleyStatus FromResource (void* rsc, DgnElementId id, bool createSubComponents) override;
    virtual BentleyStatus ToResource (void** rsc) const override;
    virtual BentleyStatus FromElement (DgnElementCP elm, bool createSubComponents) override;
    virtual BentleyStatus ToElement (DgnElementP* elm) const override;
    virtual LsElementType GetType () const override {return LsElementType::Compound;}
    virtual DgnElementId GetId () const override {return m_id;}
    virtual WCharCP GetName () const override {return m_descr;}
    virtual bool IsModified () const override {return m_isModified;}
    virtual bool ExportableToDwg (bool checkSegmentCount, LineStyleDwgExportReason& reason) const override;
    virtual double GetLength () const override;
    virtual bool HasComponentsFilled () const override;

    LsElmCompoundComponent ();
    ~LsElmCompoundComponent ();
    uint32_t GetNumComponents () const {return static_cast<uint32_t>(m_components.size());}
    LsElmComponentP GetComponent (uint32_t iComp) const {return m_components[iComp].GetLsComponent();}
    LsComponentInfo const & GetComponentInfo (uint32_t iComp) const {return m_components[iComp];}
    uint32_t GetComponentType (uint32_t iComp) const {return m_components[iComp].GetType();}
    DgnElementId GetComponentId (uint32_t iComp) const {return m_components[iComp].GetId();}
    double GetComponentOffset (uint32_t iComp) const {return m_components[iComp].GetOffset();}
    };

//=======================================================================================
// @bsiclass                                                      Chuck.Kirschman   06/07
//=======================================================================================
struct LsLineCodeComponent : public LsElmComponent
{
    enum LsCapMode
    {
        LsCapClosed     = 0,
        LsCapOpen       = 1,
        LsCapExtended   = 2,
        LsCapHexagon    = 3,
        LsCapOctagon    = 4,
        LsCapDecagon    = 5,
        LsCapArc        = 30
    };

    enum LsPhaseMode
    {
        LsPhaseMode_None    = 0,
        LsPhaseMode_Fixed   = 1,
        LsPhaseMode_Fraction= 2,
        LsPhaseMode_Center  = 3,
    };

    struct  LsStrokeData
    {
        double      m_length;
        double      m_startWidth;           // Start width.  Full width if taper bit not set below.
        double      m_endWidth;             // End width

        bool        m_isDash;               // Dash or Gap
        bool        m_isRigid;              // Apply "Ray" algorithm at corner - continue past corner rather than bending
        bool        m_canScale;             // Stroke can be scaled (variable length)
        bool        m_invertStart;          // Invert first stroke
        bool        m_invertEnd;            // Invert last stroke

        bool        m_applyWidthLeft;       // Apply width to left of stroke
        bool        m_applyWidthRight;      // Apply width to right of stroke
//      bool        m_taperWidth;           // Taper width along stroke - redundant with taperEndWidth?
//      bool        m_taperEndWidth;        // Seems to always be set in the editor if taperWidth is set.  Used in lsCache.

        LsCapMode   m_capMode;

public:
        double    GetLength () const {return m_length;}
        double    GetStartWidth () const {return m_startWidth;}
        double    GetEndWidth () const {return m_endWidth;}
        LsCapMode GetCapMode () const {return m_capMode;}
        int32_t   GetResourceWidthMode () const;
        bool      IsDash () const {return m_isDash;}
        bool      IsRigid () const {return m_isRigid;}
        bool      CanScale () const {return m_canScale;}
        bool      InvertStart () const {return m_invertStart;}
        bool      InvertEnd () const {return m_invertEnd;}
        bool      IsConstantWidth () const {return m_startWidth == m_endWidth;}
        bool      IsHalfWidth() const { return m_applyWidthLeft != m_applyWidthRight; } // Not no width or full width; only half width
        bool      _HasUniformFullWidth() const { return !IsDash() || (!IsHalfWidth() && IsConstantWidth()); }
        bool      HasWidth () const {return IsDash() && (m_applyWidthLeft || m_applyWidthRight);}
    };

protected:
    bool                m_isModified;
    WChar             m_descr[LS_MAX_DESCR];
    WChar             m_styleName[LS_MAX_NAME];   // Stored on element, for round trip.  Kind of dumb though; remove in future file format.
    DgnElementId        m_id;
    double              m_phase;
    double              m_orgAngle;
    double              m_endAngle;

    bool                m_useIterationLimit;        // Uses a fixed number of iterations (see m_maxIterate).
    bool                m_singleSegment;            // Restart the line code at a corner.

    enum LsPhaseMode   m_phaseMode;

    uint32_t            m_maxIterate;
    bvector<LsStrokeData>   m_strokes;

    size_t ResourceSize () const;
#if defined (BEIJING_DGNPLATFORM_WIP_DWG)
#endif
    virtual BentleyStatus   _DecomposeForDwg (DecomposedDwgLineP  pDDLine) const override {return ERROR;}

public:
    virtual BentleyStatus FromResource (void* rsc, DgnElementId id, bool createSubComponents) override;
    virtual BentleyStatus ToResource (void** rsc) const override;
    virtual BentleyStatus FromElement (DgnElementCP elm, bool createSubComponents) override;
    virtual BentleyStatus ToElement (DgnElementP* elm) const override;
    virtual LsElementType GetType () const override {return LsElementType::LineCode;}
    virtual WCharCP     GetName () const override {return m_descr;}
    virtual bool          IsModified () const override {return m_isModified;}
    virtual bool          ExportableToDwg (bool checkSegmentCount, LineStyleDwgExportReason& reason) const override;
    virtual double        GetLength () const override;
    virtual bool          HasComponentsFilled () const override {return true;}
    virtual bool          HasIterationLimit () const {return m_useIterationLimit;}
    virtual uint32_t      GetNumIterations () const {return m_maxIterate;}
    virtual bool          IsSingleSegmentMode () const {return m_singleSegment;}
    LsPhaseMode           GetPhaseMode () const {return m_phaseMode;}
    double                GetPhaseValue () const {return m_phase;}
    virtual bool          _HasUniformFullWidth (double *pWidth) const;
    virtual bool          HasWidth () const;
    virtual DgnElementId  GetId () const override {return m_id;}
    virtual size_t        GetNumStrokes () const {return m_strokes.size();}
    virtual LsStrokeData const * GetStroke (size_t iStroke) const {return &(m_strokes[iStroke]);}

};

//=======================================================================================
// @bsiclass                                                      Chuck.Kirschman   06/07
//=======================================================================================
struct LsElmInternalComponent : public LsLineCodeComponent
{
private:
    uint32_t    m_hardwareLineCode;
    bool        m_isDefaultInternal;

public:
    virtual BentleyStatus FromResource (void* rsc, DgnElementId id, bool createSubComponents) override;
    virtual BentleyStatus ToResource (void** rsc) const override;
    virtual BentleyStatus FromElement (DgnElementCP elm, bool createSubComponents) override;
    virtual BentleyStatus ToElement (DgnElementP* elm) const override;
    virtual LsElementType GetType () const override {return LsElementType::Internal;}
    virtual WCharCP     GetName () const override {return m_descr;}
    virtual bool          IsModified () const override {return m_isModified;}
    virtual bool          ExportableToDwg (bool checkSegmentCount, LineStyleDwgExportReason& reason) const override;
    virtual double        GetLength () const override;
    virtual bool          HasComponentsFilled () const override {return true;}

    bool                  HasIterationLimit () const {return false;}
    uint32_t              GetNumIterations () const {return 0;}
    bool                  IsSingleSegmentMode () const {return false;}
    virtual bool          IsContinuous () const override {return 0==m_hardwareLineCode ? true : false;}
    virtual bool          _HasUniformFullWidth (double *pWidth) const override {return m_isDefaultInternal;}
    virtual bool           HasWidth () const override {return false;} // Only has width if from element

    bool IsHardwareStyle () const {return (0 != m_hardwareLineCode ? true : false);}
    uint32_t GetHardwareStyle () const {return m_hardwareLineCode;}
};

enum SymbolLocation
{
    SymbolStrokeOrigin  = 1,
    SymbolStrokeEnd     = 2,
    SymbolStrokeCenter  = 3,
    SymbolLineOrigin    = 4,
    SymbolLineEnd       = 5,
    SymbolLineVertex    = 6
};

//=======================================================================================
// @bsiclass                                                      Chuck.Kirschman   06/07
//=======================================================================================
struct LsLinePointComponent : public LsElmComponent
{
    struct LsPointSymInfo
    {
        DgnElementId               m_symbolID;                 // Used if there is no point symbol.
        struct LsPointSymbolComponent*      m_pointSymbol;

        unsigned short          m_strokeNo;
        enum SymbolLocation     m_location;

        bool                    m_adjustedRotation;      // Adjust rotation left->right.
        bool                    m_absoluteRotation;      // Angles not relative to line.
        bool                    m_noScale;               // No scale on variable strokes.
        bool                    m_noClip;                // No clip on partial strokes.
        bool                    m_noPartialStrokes;      // No partial strokes.
        bool                    m_projectPartialOrigin;  // Project partial origin.
        bool                    m_useSymbolColor;        // Use color from symbol.
        bool                    m_useSymbolWeight;       // Use weight from symbol.

        double                  m_xOffset;
        double                  m_yOffset;
        double                  m_angle;

        struct SymbolRef*       m_symbolRef;            // For now, need this for DWG decomp.  Have to keep it around and free it with the object.

        void    SetModifiers    (unsigned short modifiers);
        unsigned short GetModifiers    () const;

        struct LsPointSymbolComponent* GetSymbol () const {return m_pointSymbol;}
        struct SymbolRef* GetSymbolRef () const;
        LsPointSymInfo ();
        ~LsPointSymInfo ();
        BentleyStatus LoadSymbFromResource (RscFileHandle rfHandle, uint32_t type, bool createSubComponents);

        int32_t GetStrokeNumber () const {return m_strokeNo;}
        struct LsPointSymbolComponent* GetSymbolComponent () const {return GetSymbol();}

        SymbolLocation GetStrokeLocation () const {return m_location;}

        bool IsAdjustedRotation () const {return m_adjustedRotation;}
        bool IsAbsoluteRotation () const {return m_absoluteRotation;}
        bool IsScaled () const {return m_noScale ? false : true;}
        bool IsClipped () const {return m_noClip ? false : true;}
        bool AllowPartialStrokes () const {return m_noPartialStrokes ? false : true;}
        bool ProjectPartialOrigin () const {return m_projectPartialOrigin;}
        bool ColorFromSymbol () const {return m_useSymbolColor;}
        bool WeightFromSymbol () const {return m_useSymbolWeight;}

        double GetXOffset () const {return m_xOffset;}
        double GetYOffset () const {return m_yOffset;}
        double GetRotation () const {return m_angle;}

        void SetXOffset (double value) {m_xOffset = value;}
        void SetYOffset (double value) {m_yOffset = value;}
        void SetRotation (double value) {m_angle = value;}
    };

private:
    bool                m_isModified;
    WChar             m_descr[LS_MAX_DESCR];
    WChar             m_styleName[LS_MAX_NAME];   // Stored on element, for round trip.  Kind of dumb though; remove in future file format.
    DgnElementId           m_id;
    LsElementType          m_lineCodeType;
    DgnElementId           m_lineCodeID;               // Used if there is no m_lineCodeComponent
    LsLineCodeComponent*        m_lineCodeComponent;
    bvector<LsPointSymInfo*> m_symbols;

    size_t ResourceSize () const;
    virtual BentleyStatus _DecomposeForDwg (DecomposedDwgLineP pDDLine) const override {return (SUCCESS);}

public:
    virtual BentleyStatus FromResource (void* rsc, DgnElementId id, bool createSubComponents) override;
    virtual BentleyStatus ToResource (void** rsc) const override;
    virtual BentleyStatus FromElement (DgnElementCP elm, bool createSubComponents) override;
    virtual BentleyStatus ToElement (DgnElementP* elm) const override;
    virtual LsElementType GetType () const override {return LsElementType::LinePoint;}
    virtual WCharCP GetName () const override {return m_descr;}
    virtual bool IsModified () const override {return m_isModified;}
    virtual bool ExportableToDwg (bool checkSegmentCount, LineStyleDwgExportReason& reason) const override;
    virtual double GetLength () const override {return NULL == GetLineCodeComponent() ? 0.0 : GetLineCodeComponent()->GetLength();}
    virtual bool HasComponentsFilled () const override {return NULL != m_lineCodeComponent;}

    LsLinePointComponent ();
    ~LsLinePointComponent ();

    BentleyStatus DecomposeForDwgPoint (DecomposedDwgLineP pDDLine) const;

    virtual DgnElementId GetId () const override {return m_id;}
    DgnElementId GetLineCodeId () const {return NULL == m_lineCodeComponent ? m_lineCodeID : m_lineCodeComponent->GetId();}
    LsLineCodeComponent* GetLineCodeComponent () const {return m_lineCodeComponent;}
    BentleyStatus LoadLineCodeFromResource (RscFileHandle rfHandle, bool createSubComponents);

    size_t GetNumSymbols () const {return m_symbols.size();}
    LsPointSymInfo* GetSymbolInfo (uint32_t iSymbol) const {return m_symbols[iSymbol];}
    void SetLineCode (const LsLineCodeComponent& lcComp) {m_lineCodeID = lcComp.GetId();}
 };

//=======================================================================================
// @bsiclass                                                      Chuck.Kirschman   06/07
//=======================================================================================
struct LsPointSymbolComponent : public LsElmComponent
{
private:
    struct SymbolRange
        {
        DPoint3d    low;
        DPoint3d    high;
        };

    bool                m_isModified;
    WChar             m_descr[LS_MAX_DESCR];
    WChar             m_styleName[LS_MAX_NAME];   // Stored on element, for round trip.  Kind of dumb though; remove in future file format.
    DgnElementId        m_id;
    DRange3d            m_range;

    double              m_muDef;                    // Master units definition (in PU).
    bool                m_noScale;                  // Acad compat - don't scale symbol at all.
    DgnElementPtr     m_elementChain;

    size_t ResourceSize () const;
    BentleyStatus ToResourceNoSymbols (void** outRsc) const;
    BentleyStatus FromResourceNoSymbols (void* rsc, DgnElementId id);
    virtual BentleyStatus _DecomposeForDwg (DecomposedDwgLineP pDDLine) const override {return (SUCCESS);}

public:
    LsPointSymbolComponent ()
        {
        memset (m_descr, 0, sizeof(m_descr));
        memset (m_styleName, 0, sizeof(m_styleName));
        m_elementChain = NULL;
        }

    ~LsPointSymbolComponent ()
        {
        m_elementChain = nullptr;
        }

    virtual BentleyStatus FromResource (void* rsc, DgnElementId id, bool createSubComponents) override;
    virtual BentleyStatus ToResource (void** rsc) const override;
            BentleyStatus ToResource (void** outRsc, bool asV7Element, bool forResourceFile) const;
    virtual BentleyStatus FromElement (DgnElementCP elm, bool createSubComponents) override;
    virtual BentleyStatus ToElement (DgnElementP* elm) const override;
    virtual LsElementType GetType () const override {return LsElementType::PointSymbol;}
    virtual DgnElementId  GetId () const override {return m_id;}
    virtual WCharCP       GetName () const override {return m_descr;}
    virtual bool          IsModified () const override {return m_isModified;}
    virtual bool          ExportableToDwg (bool checkSegmentCount, LineStyleDwgExportReason& reason) const override {return true;}
    virtual double        GetLength () const override {return 0.0;}
    virtual bool          HasComponentsFilled () const override {return true;}

    DgnElementCP GetElements () const {return m_elementChain.get();}

    DRange3d GetRange () const {return m_range;}
    double GetUnitScale () const {return m_muDef;}
    bool SuppressScale () const {return m_noScale;}
#if defined (NEEDS_WORK_DGNITEM)
    bool Is3d () const {return GetElements()->Element().Is3d();}
#endif
};

typedef struct ILinestyleInfoHandle*        ILinestyleInfoHandleP;
typedef struct ILinestyleInfoHandle const * ILinestyleInfoHandleCP;



//=======================================================================================
// @bsiclass                                                      Chuck.Kirschman 05/07
//=======================================================================================
struct ILinestyleInfoHandle
    {
public:
    virtual ~ILinestyleInfoHandle() { }
    virtual WCharCP GetName () const = 0;
    virtual LsElementType GetType () const = 0;
    virtual struct ILinestyleComponentHandle const * GetComponentHandle () const { return NULL; }
    virtual struct ILinestyleNameHandle const * GetNameHandle () const { return NULL; }
    virtual DgnElementId GetIdKey () const = 0;
    };


//=======================================================================================
// @bsiclass                                                      Chuck.Kirschman 05/07
//=======================================================================================
struct ILinestyleComponentHandle  : public ILinestyleInfoHandle
{
public:
    virtual ~ILinestyleComponentHandle () { }

    virtual WCharCP GetName () const = 0;
    virtual LsElementType GetType () const override = 0;
    virtual DgnElementId GetIdKey () const override = 0;
    virtual LsElmComponentCP GetStyleDef (DgnModelP modelRef, bool loadIfNotLoaded) const = 0;
    virtual StatusInt SetStyleDef (LsElmComponentCP component) = 0;
    virtual BentleyStatus Save (DgnModelP modelRef) const = 0;

    virtual struct ILinestyleComponentHandle const* GetComponentHandle () const override { return this; }
};

//=======================================================================================
// @bsiclass                                                      Chuck.Kirschman 05/07
//=======================================================================================
struct ILinestyleNameHandle : public ILinestyleInfoHandle
{
public:
    virtual ~ILinestyleNameHandle () { }
    virtual WCharCP GetName () const = 0;
    virtual int32_t GetId () const = 0;
    virtual LsElementType GetType () const override = 0;
    virtual DgnElementId GetIdKey () const override = 0;

    virtual struct ILinestyleNameHandle const * GetNameHandle () const override { return this; }
};

typedef std::map<DgnElementId, ILinestyleInfoHandle*>   StyleInfoHandleMap;

END_BENTLEY_DGNPLATFORM_NAMESPACE
