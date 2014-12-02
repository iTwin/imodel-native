/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/XGraphics.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "DisplayHandler.h"
#include    "SolidKernel.h"
#include    <Bentley/bvector.h>
#include    <Bentley/RefCounted.h>
#include    <DgnPlatform/DgnCore/GPArray.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct XGraphicsContext;
struct XGraphicsRecorder;
struct XGraphicsSplitter;
struct XGraphicsSymbolStamp;

enum XGraphicsMinorId
{
    XGraphicsMinorId_Data                   = 0,
    XGraphicsMinorId_SymbolTransform        = 1,
    XGraphicsMinorId_Quick                  = 2,    // No longer generated or read
    XGraphicsMinorId_SymbolId               = 3,
};

enum XGraphicsOpCodes
{
    XGRAPHIC_OpCode_DrawLineString3d        = 1,
    XGRAPHIC_OpCode_DrawLineString2d        = 2,
    XGRAPHIC_OpCode_DrawPointString3d       = 3,
    XGRAPHIC_OpCode_DrawPointString2d       = 4,
    XGRAPHIC_OpCode_DrawArc3d               = 5,
    XGRAPHIC_OpCode_DrawArc2d               = 6,
    XGRAPHIC_OpCode_DrawEllipse3d           = 7,
    XGRAPHIC_OpCode_DrawEllipse2d           = 8,
    XGRAPHIC_OpCode_DrawShape3d             = 9,
    XGRAPHIC_OpCode_DrawShape2d             = 10,
    XGRAPHIC_OpCode_DrawCone                = 11,
    XGRAPHIC_OpCode_DrawTorus               = 12,
    XGRAPHIC_OpCode_PushTransClip           = 13,
    XGRAPHIC_OpCode_PopTransClip            = 14,
    XGRAPHIC_OpCode_PopAll                  = 15,
    XGRAPHIC_OpCode_BeginSweepProject       = 16,
    XGRAPHIC_OpCode_BeginSweepExtrude       = 17,
    XGRAPHIC_OpCode_BeginSweepRevolve       = 18,
    XGRAPHIC_OpCode_EndSweep                = 19,
    XGRAPHIC_OpCode_BeginComplexString      = 20,
    XGRAPHIC_OpCode_BeginComplexShape       = 21,
    XGRAPHIC_OpCode_EndComplex              = 22,
    XGRAPHIC_OpCode_AddDisconnect           = 23,
    XGRAPHIC_OpCode_DrawBody                = 24,
    XGRAPHIC_OpCode_AddIndexPolys           = 25,
    XGRAPHIC_OpCode_DrawBSplineCurve        = 26,
    XGRAPHIC_OpCode_DrawBSplineSurface      = 27,
    XGRAPHIC_OpCode_MatSymb                 = 28,
    XGRAPHIC_OpCode_AnnotationData          = 29,   // This is not an operation but identifies annotation data that pertains to the NEXT op.
    XGRAPHIC_OpCode_DrawBox                 = 30,
    XGRAPHIC_OpCode_DrawSphere              = 31,
    XGRAPHIC_OpCode_DrawTriStrip3d          = 32,
    XGRAPHIC_OpCode_DrawTriStrip2d          = 33,
    XGRAPHIC_OpCode_DrawSymbol              = 34,
    XGRAPHIC_OpCode_BeginMultiSymbologyBody = 35,
    XGRAPHIC_OpCode_EndMultiSymbologyBody   = 36,
    XGRAPHIC_OpCode_DrawText                = 37,
    XGRAPHIC_OpCode_OBSOLETEDrawAreaPattern = 38,   // We no longer write or use this op code in order to minimize damage from a Graphite merge problem.
    XGRAPHIC_OpCode_EnableZTesting          = 39,
    XGRAPHIC_OpCode_MatSymb2                = 40,
    XGRAPHIC_OpCode_IfConditionalDraw       = 41,
    XGRAPHIC_OpCode_ElseIfConditionalDraw   = 42,
    XGRAPHIC_OpCode_ElseConditionalDraw     = 43,
    XGRAPHIC_OpCode_EndConditionalDraw      = 44,
    XGRAPHIC_OpCode_DrawAligned             = 45,   // Unimplemented for now, needs to be merged from Topaz.
    XGRAPHIC_OpCode_NoOp                    = 46,
    XGRAPHIC_OpCode_SetLocatePriority       = 47,
    XGRAPHIC_OpCode_SetNonSnappable         = 48,
    XGRAPHIC_OpCode_DrawAreaPattern         = 49,
    MAX_XGraphicsOpCode
};

enum XGraphicsCreateOptions
{
    XGRAPHIC_CreateOptions_None                     = 0,
    XGRAPHIC_CreateOptions_PreserveECXAttributes    = 0x0001 << 0,
    XGRAPHIC_CreateOptions_ViewIndependentOnly      = 0x0001 << 1,
    XGRAPHIC_CreateOptions_NoCustomLineStyles       = 0x0001 << 2,
    XGRAPHIC_CreateOptions_NoDisplayFilters         = 0x0001 << 3,
    XGRAPHIC_CreateOptions_StrokeAreaPatterns       = 0x0001 << 4,
    XGRAPHIC_CreateOptions_ForceSymbologyInclusion  = 0x0001 << 5,
    XGRAPHIC_CreateOptions_RemoveSymbols            = 0x0001 << 6,
};

enum XGraphicsOptimizeOptions
{
    XGRAPHIC_OptimizeOptions_None                    = 0,
    XGRAPHIC_OptimizeOptions_MeshFromShapes          = 0x0001 << 0,
    XGRAPHIC_OptimizeOptions_MeshFromPlanarBRep      = 0x0001 << 1,
    XGRAPHIC_OptimizeOptions_MeshFromPlanarSweep     = 0x0001 << 2,
    XGRAPHIC_OptimizeOptions_CreateSymbols           = 0x0001 << 3,
    XGRAPHIC_OptimizeOptions_ConesFromProjections    = 0x0001 << 4,
    XGRAPHIC_OptimizeOptions_MeshFromRevolvedSweeps  = 0x0001 << 5,             // Not recommended.  - Large memory increase for little performance gain.
    XGRAPHIC_OptimizeOptions_MeshParasolid           = 0x0001 << 6,
    XGRAPHIC_OptimizeOptions_MeshFromBCurveSweeps    = 0x0001 << 7,
    XGRAPHIC_OptimizeOptions_MeshFromBilinearBSurf   = 0x0001 << 8,
    XGRAPHIC_OptimizeOptions_Quick                   = 0x0001 << 9,
    XGRAPHIC_OptimizeOptions_RemoveSymbols           = 0x0001 << 11,
    XGRAPHIC_OptimizeOptions_Default                 = (XGRAPHIC_OptimizeOptions_MeshParasolid | 
                                                        XGRAPHIC_OptimizeOptions_MeshFromBilinearBSurf |
                                                        XGRAPHIC_OptimizeOptions_CreateSymbols |
                                                        XGRAPHIC_OptimizeOptions_ConesFromProjections),
};

enum XGraphicsSymbolVersion
{
    XGraphicsSymbolVersion_1 = 1,
};

//=======================================================================================
//! @bsiclass                                                     RayBentley      12/06
//=======================================================================================
struct XGraphicsHeader
{
    UInt16   m_version:8;
    UInt16   m_useCache:1;
    UInt16   m_cannotUseDirectlyForStencil:1; // Can be used if re-stroked w/DRAW_OPTION_ClipStencil...
    UInt16   m_validForStencil:1;             // Contains only Open/non-surface geometry...
    UInt16   m_isRenderable:1;
    UInt16   m_filtersPresent:1;
    UInt16   m_uncachablePresent:1;
    UInt16   m_brepsPresent:1; // NOTE: Valid when m_version > 1.
    UInt16   m_unused:1;

    XGraphicsHeader ()
        {
        m_version = 2; // Version bumped to 2 for Vancouver.
        m_useCache = false;
        m_cannotUseDirectlyForStencil = false;
        m_validForStencil = false;
        m_filtersPresent = false;
        m_uncachablePresent = false;
        m_brepsPresent = false;
        m_unused = 0;
        m_isRenderable = false;
        }
};

//=======================================================================================
//! @bsiclass                                                     RayBentley      06/2009
//=======================================================================================
struct XGraphicsData : bvector<UInt8>
{
    void Append (void const* data, size_t dataSize);
};

typedef XGraphicsData*          XGraphicsDataP;
typedef XGraphicsData&          XGraphicsDataR;
typedef XGraphicsData const&    XGraphicsDataCR;
typedef bvector <BeRepositoryBasedId>                   T_XGraphicsSymbolIds;   //  allowing this to be either an ElementId or a stamp ID
typedef T_XGraphicsSymbolIds&                           T_XGraphicsSymbolIdsR;
typedef T_XGraphicsSymbolIds const&                     T_XGraphicsSymbolIdsCR;

//=======================================================================================
//! @bsiclass                                                     Sam.Wilson      05/2008
//=======================================================================================
struct XGraphicsAnnotationData
{
    enum Signature
        {
        SIGNATURE_EdgeId = 'EdId'    // used by XGraphicsOutput when recording EdgeIds
        };

    Signature       m_sig;
    XGraphicsData   m_data;
};

//=======================================================================================
//! Records and plays back graphics.
//! Also see XGraphicsRecorder.
//! @bsiclass                                                     RayBentley      12/06
//=======================================================================================
struct XGraphicsContainer
{
private:
    enum class ConsumeParsedResult
        {
        MovedToElement      = 0,
        UsedElement         = 1,
        UsedCurrent         = 2,
        AppendedToContainer = 3
        };

    XGraphicsHeader         m_header;
    XGraphicsData           m_buffer;
    T_XGraphicsSymbolIds    m_symbolIds;
    bool                    m_unsupportedPrimitivePresent;
    ElementRefP             m_elementRef;
    DgnStampId              m_stampId;

    friend struct XGraphicsRecorder;
    friend struct XGraphicsSplitter;
    struct IProcessOperations
        {
        virtual void    _ProcessOperation (UInt16 opcode, byte* opData, UInt32 opDataSize) = 0;
        };

    DGNPLATFORM_EXPORT void ProcessOperations (IProcessOperations& processor);
    ConsumeParsedResult ConsumeParsedDisplayParams(XGraphicsData& output, EditElementHandleR eeh, bvector <byte>& previousMatSymb, bool allowExtractDisplayParamsToHeader, bool useHeaderAllowed, UInt16 mask, ElemDisplayParamsR params, Int64 materialId);

public:

    enum DrawOptions
        {
        DRAW_OPTION_None                          = 0,
        DRAW_OPTION_ClipStencil                   = (1<<0),
        DRAW_OPTION_IgnoreUseCache                = (1<<2), //! Purpose is to force immediate mode draw of a container that doesn't include BReps (was DRAW_OPTION_UseLineStyle)...
        DRAW_OPTION_Quick                         = (1<<4),
        DRAW_OPTION_CutGraphicsOverstroke         = (1<<5),
        DRAW_OPTION_Default                       = DRAW_OPTION_None,
        };

    DGNPLATFORM_EXPORT XGraphicsContainer () { m_unsupportedPrimitivePresent = false; m_elementRef = NULL; }
    DGNPLATFORM_EXPORT XGraphicsContainer (void const* data, size_t dataSize);

    DGNPLATFORM_EXPORT void ExtractSymbolIdsFromElement (ElementHandleCR);

    byte*       GetData () { return &m_buffer.front(); }
    XGraphicsHeader const&GetHeader () { return m_header; }
    byte const* GetGraphicsData () const { return &m_buffer[0] + sizeof(XGraphicsHeader); }
    byte const* GetDataEnd () const { return &m_buffer[0] + GetDataSize(); }
    size_t      GetDataSize () const { return m_buffer.size(); }
    void        SetElement (ElementRefP elementRef) { m_elementRef = elementRef; }
    void        SetStampId (DgnStampId stampId) { m_stampId = stampId; }
    ElementRefP GetElementRef () const { return m_elementRef; }
    BeRepositoryBasedId GetParentId () const { if (m_stampId.IsValid()) return m_stampId; return NULL == m_elementRef ? ElementId() : m_elementRef->GetElementId(); }
    size_t      GetSymbolCount () const { return m_symbolIds.size(); }
    void        AddSymbol (BeRepositoryBasedId persistentId) { m_symbolIds.push_back (persistentId); }
    BeRepositoryBasedId   GetSymbolId (size_t index) { return index < m_symbolIds.size() ? m_symbolIds[index] : ElementId(); }
    StatusInt   ValidateBuffer ();
    void        Write (void const *data, size_t dataSize);
    bool        UseCache () const { return m_header.m_useCache; }
    void        SetUseCache (bool useCache) { m_header.m_useCache = useCache; }
    bool        UseForStencil () const { return !m_header.m_cannotUseDirectlyForStencil; }
    void        SetUseForStencil (bool useForStencil) { m_header.m_cannotUseDirectlyForStencil = !useForStencil; }
    bool        IsValidForStencil () const { return m_header.m_validForStencil; }
    void        SetValidForStencil (bool validAsStencil){ m_header.m_validForStencil = validAsStencil; }
    bool        IsRenderable () const { return m_header.m_isRenderable; }
    void        SetIsRenderable (bool isRenderable) { m_header.m_isRenderable = isRenderable; }
    void        SetFilterPresent () { m_header.m_filtersPresent = true; }
    void        SetUncachablePresent () { m_header.m_uncachablePresent = true; }
    bool        GetBRepsPresent () { return m_header.m_brepsPresent; }
    void        SetBRepsPresent (bool hasBreps = true) { m_header.m_brepsPresent = hasBreps; }
    void        SetUnsupportedPrimitivePresent () { m_unsupportedPrimitivePresent = true; }
    bool        GetUnsupportedPrimitivePresent () { return m_unsupportedPrimitivePresent; }
    void        ReplaceHeader (XGraphicsHeader const& header) { m_header = header; }
    bool        IsEmpty () { return m_buffer.size () <= sizeof (XGraphicsHeader); }
    void        Resize (size_t size) { m_buffer.resize (size); }

    T_XGraphicsSymbolIds& GetSymbols () { return m_symbolIds; }
    T_XGraphicsSymbolIds const& GetSymbols () const { return m_symbolIds; }
    DGNPLATFORM_EXPORT static bool IsXGraphicsSymbol (ElementRefP);

    DGNPLATFORM_EXPORT bool        ContainsGraphics ();

    // Record (also see XGraphicsRecorder)
    DGNPLATFORM_EXPORT StatusInt CreateFromElement (ElementHandleCR elemHandle, UInt32 optimizeOptions = XGRAPHIC_OptimizeOptions_Default, UInt32 createOptions = XGRAPHIC_CreateOptions_None, struct XGraphicsSymbolCache* symbolCache = NULL);
    DGNPLATFORM_EXPORT BentleyStatus CreateFromElementAgenda (ElementAgendaCR agenda);
    DGNPLATFORM_EXPORT void BeginDraw ();
    DGNPLATFORM_EXPORT StatusInt EndDraw ();
    DGNPLATFORM_EXPORT StatusInt AddCurveVector (CurveVectorCR curveVector, bool filled);
    DGNPLATFORM_EXPORT void DeleteViewDraw (IViewDrawP viewDraw);
    DGNPLATFORM_EXPORT IViewDrawP ConnectToViewDraw (ViewContextR viewContext, UInt32 createOptions = XGRAPHIC_CreateOptions_None, UInt32 optimizeOptions = XGRAPHIC_OptimizeOptions_None);
    DGNPLATFORM_EXPORT StatusInt CreateFromStroker (IStrokeForCache& stroker, ElementHandleCR element, double pixelSize, UInt32 optimizeOptions = XGRAPHIC_OptimizeOptions_None);
    DGNPLATFORM_EXPORT QvElem* CreateQvElem (ElementHandleCR eh, ViewContextR viewContext, QvCache* qvCache, double pixelSize, DrawOptions drawOptions);

    // Save
    DGNPLATFORM_EXPORT StatusInt AddToElement (EditElementHandleR elemHandle, XGraphicsMinorId  minorId = XGraphicsMinorId_Data);
    DGNPLATFORM_EXPORT StatusInt ReplaceOnElement (EditElementHandleR element, XGraphicsMinorId minorId = XGraphicsMinorId_Data);
    DGNPLATFORM_EXPORT StatusInt OnWriteToElement (ElementHandleCR eh);
    DGNPLATFORM_EXPORT StatusInt AddInstance (XGraphicsContainerR definition, EditElementHandleR eh, TransformCR transform, Int32 symbolId, XGraphicsMinorId  minorId = XGraphicsMinorId_Data);
    static DGNPLATFORM_EXPORT StatusInt RemoveFromElement (EditElementHandleR elemHandle, XGraphicsMinorId  minorId = XGraphicsMinorId_Data);
    DGNPLATFORM_EXPORT StatusInt ExtractFromElement (ElementHandleCR elemHandle, XGraphicsMinorId  minorId = XGraphicsMinorId_Data);
    DGNPLATFORM_EXPORT StatusInt AddSymbolIdsToElement (EditElementHandleR eh);

    // Playback
    static DGNPLATFORM_EXPORT StatusInt Draw (ViewContextR context, ElementHandleCR eh, DrawOptions options = DRAW_OPTION_Default, XGraphicsMinorId  minorId = XGraphicsMinorId_Data);
    static DGNPLATFORM_EXPORT StatusInt Draw (ViewContextR context, CachedDrawHandleCR dh, ElementHandle::XAttributeIter*, T_XGraphicsSymbolIds& symbolIds, DrawOptions options = DRAW_OPTION_Default);
    static DGNPLATFORM_EXPORT StatusInt DrawSymbol (ViewContextR context, ElementRefP symbolElemRef, XGraphicsOperationContextR opContext, TransformCR transform);
    static DGNPLATFORM_EXPORT StatusInt DrawSymbolFromStamp (ViewContextR context, XGraphicsSymbolStamp& symbolStamp, XGraphicsOperationContextR opContext, TransformCR transform);
    static StatusInt DrawXGraphicsFromMemory (ViewContextR context, byte const* data, size_t sizeOfData, DgnModelP dgnCache, DrawOptions options = DRAW_OPTION_Default);
    static DGNPLATFORM_EXPORT bool IsRenderable (ElementHandleCR eh);
    DGNPLATFORM_EXPORT StatusInt Draw (ViewContextR context, DrawOptions options = DRAW_OPTION_Default);
    DGNPLATFORM_EXPORT StatusInt ExtractPrimitives (bvector<XGraphicsContainer>& containerVector) const;

    // Maintain
    DGNPLATFORM_EXPORT StatusInt OnTransform (TransformInfoCR transform);
    DGNPLATFORM_EXPORT StatusInt CompressTransforms ();
    DGNPLATFORM_EXPORT StatusInt Optimize (UInt32 options);
    static DGNPLATFORM_EXPORT BentleyStatus DropSymbolsToElement (EditElementHandleR eeh);

    // Query
    DGNPLATFORM_EXPORT bool IsEmpty () const;
    bool IsEqual (XGraphicsContainer const& rhs) const {return m_buffer == rhs.m_buffer;} 
    DGNPLATFORM_EXPORT bool IsEqual (XGraphicsContainer const& rhs, double distanceTolerance) const;
    DGNPLATFORM_EXPORT StatusInt GetBasisTransform (TransformR transform);
    DGNPLATFORM_EXPORT bool IsQuick () const;
    DGNPLATFORM_EXPORT void Dump (DgnModelP cache) const;
    DGNPLATFORM_EXPORT CurveVectorPtr   GetCurveVector () const;
    static DGNPLATFORM_EXPORT bool IsPresent (ElementHandleCR element, XGraphicsMinorId minorId = XGraphicsMinorId_Data);
    BentleyStatus CalculateRange (DRange3dR range, DgnModelCR dgnModel, TransformCP transform);

    // Modify
    DGNPLATFORM_EXPORT StatusInt CreateMeshFromShapes ();
    DGNPLATFORM_EXPORT StatusInt CreateMeshFromParasolid ();
    StatusInt DropSymbolInstance (EditElementHandleR eh, XGraphicsSymbolR symbol);
    DGNPLATFORM_EXPORT BentleyStatus FlattenTransforms ();
    //! Flatten mat symb operations that have no effect. If a mat symb is the first operation, remove and
    //! push its symbology to eeh.
    //! @param[in/out] eeh The element associated with this container. May have its header symbology modified.
    //! @return SUCCESS if any modifications were made, ERROR otherwise
    DGNPLATFORM_EXPORT BentleyStatus FlattenSymbology (EditElementHandleR eeh);
    //! Fix branch offsets that may be invalid due to FlattenTransforms or FlattenSymbology
    //! removing operations that occur within a condition.
    //! @param[in/out] eeh The element associated with this container.
    //! @return SUCCESS if any modifications were made, ERROR otherwise
    DGNPLATFORM_EXPORT BentleyStatus RecomputeBranches (EditElementHandleR eeh);
    DGNPLATFORM_EXPORT BentleyStatus SplitByRange (std::list<XGraphicsContainer>& splitGraphics, DgnModelR model);
    DGNPLATFORM_EXPORT BentleyStatus SplitSurfacesByPoleCount (std::list<XGraphicsContainer>& splitSurfaces, UInt32 maxPoleCount);
    DGNPLATFORM_EXPORT BentleyStatus SplitMeshesByFaceCount (std::list<XGraphicsContainer>& splitMeshes, UInt32 maxFace);

    // Properties
    DGNPLATFORM_EXPORT void ProcessProperties (PropertyContextR context);
//    DGNPLATFORM_EXPORT bool DeepCopyRoots (ElementCopyContextR copyContext); removed in graphite
//    DGNPLATFORM_EXPORT void DoClone (ElementCopyContextR copyContext); // Calls DeepCopyRoots and ProcessProperties with CloneRemapper...  removed in graphite

    static DGNPLATFORM_EXPORT void QueryProperties (ElementHandleCR eh, PropertyContextR context);
    static DGNPLATFORM_EXPORT void EditProperties (EditElementHandleR eeh, PropertyContextR context);
//    static DGNPLATFORM_EXPORT void PreprocessCopy (EditElementHandleR eeh, ElementCopyContextR copyContext); // ExtractFromElement/DeepCopyRoots/AddToElement... removed in graphite
    static DGNPLATFORM_EXPORT void ConvertTo3d (EditElementHandleR eeh, double elevation); // ExtractFromElement/DoConvert3d/AddToElement...
    static DGNPLATFORM_EXPORT void ConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir); // ExtractFromElement/DoConvert2d/AddToElement...

    // Proxy Support
    DGNPLATFORM_EXPORT StatusInt ExtractProxyCutEdge (XGraphicsContainerR edgeGraphics, ProxyEdgeIdDataCR edgeId) const;
    DGNPLATFORM_EXPORT StatusInt ExtractProxyGPArray (XGraphicsContainerR edgeGraphics, ProxyHLEdgeSegmentIdCR edgeId, GPArrayParamCP) const;
    DGNPLATFORM_EXPORT StatusInt DrawProxy (ViewContextR context, ElementHandleCR eh, UInt32 qvIndex, DrawOptions options = DRAW_OPTION_Default) const;
    DGNPLATFORM_EXPORT StatusInt AddProxyCurve (struct GPArray const& gpa, struct GPArrayInterval const* interval, ProxyHLEdgeSegmentIdCR segmentId, CompoundDrawStateP compoundDrawState);

    // Unification
    DGNPLATFORM_EXPORT static BentleyStatus Unify (XGraphicsContainerP& unifiedXGraphics, ElementHandleCR eh, bvector<ISolidKernelEntityPtr>& otherBodies);

}; // XGraphicsContainer

//=======================================================================================
//! Provides a ViewContext that can be used to record graphics.
//! Generates an XGraphicsContainer, which can be used to save and playback graphics.
//! @bsiclass                                                     Sam.Wilson      05/2008
//=======================================================================================
struct          XGraphicsRecorder
{
private:
    XGraphicsContainer  m_container;
    XGraphicsContext*   m_context;

    XGraphicsRecorder (XGraphicsRecorder const&) {BeAssert(false);}
    XGraphicsRecorder& operator=(XGraphicsRecorder const&) {BeAssert(false);return *this;}

public:
    DGNPLATFORM_EXPORT XGraphicsRecorder (DgnModelP root);
    DGNPLATFORM_EXPORT ~XGraphicsRecorder ();

    DGNPLATFORM_EXPORT ViewContextP GetContext () const;
    XGraphicsContainer& GetContainer () {return m_container;}

    DGNPLATFORM_EXPORT void EnableInitialWeightMatSym ();
    DGNPLATFORM_EXPORT void EnableInitialColorMatSym ();
    DGNPLATFORM_EXPORT void EnableInitialLineCodeMatSym ();
    DGNPLATFORM_EXPORT void WriteAnnotationData (XGraphicsAnnotationData const&);
                   void SetUseCache (bool useCache) { m_container.SetUseCache (useCache); }
    DGNPLATFORM_EXPORT void SetElemDisplayParams (ElemDisplayParamsCP elemDisplayParams);
    DGNPLATFORM_EXPORT void SetFillOutlineThreshold (UInt32 threshold);
    DGNPLATFORM_EXPORT void Reset ();
};

//=======================================================================================
//! @bsiclass                                                     RayBentley      12/06
//=======================================================================================
struct XGraphicsOperationContext
{
    XGraphicsHeader*                    m_header;
    ptrdiff_t                           m_brepOffset;
    int                                 m_baseElemId;
    XGraphicsContainer::DrawOptions     m_drawOptions;
    DgnProjectR                         m_dgnProject;
    T_XGraphicsSymbolIds&               m_symbolIds;
    ElementRefP                         m_sourceElement;    // May be NULL!
    bool                                m_sizeDependentGeometryExists;

    CurveVectorPtr                      m_curve;
    bvector<CurveVectorPtr>             m_loops;
    bool                                m_filled;
    bool                                m_is3d;
    double                              m_zDepth;
    CurvePrimitiveIdPtr                 m_curvePrimitiveId;

    SolidPrimitiveType                  m_solidPrimitiveType;
    DgnRuledSweepDetail*                m_projection;
    DgnExtrusionDetail*                 m_extrusion;
    DgnRotationalSweepDetail*           m_revolution;

    bool                                m_inMultiSymbBody;
    ISolidKernelEntityPtr               m_brepEntity;
    IFaceMaterialAttachmentsPtr         m_faceAttachments;
    ElementHandleCP                     m_element;

    bool AllowDrawStyled (ViewContextR context, bool isRenderable);
    bool AllowDrawFilled (ViewContextR context, bool isFilled);
    bool AllowDrawWireframe (ViewContextR);
    void DrawStyledCurveVector (ViewContextR context, CurveVectorCR curves, bool isFilled, bool is3d, double zDepth);

    void BeginComplex (bool isClosed, bool filled);
    bool IsComplexComponent ();
    void AddComplexComponent (ICurvePrimitivePtr&, bool is3d, double zDepth);
    void AddDisconnect ();
    void EndComplex (ViewContextR);

    void BeginSweepProject (bool capped);
    void BeginSweepExtrude (bool capped, DVec3dCR extrusionVector);
    void BeginSweepRevolve (bool capped, DRay3dCR axisOfRotation, double sweepAngle);
    void EndSweep (ViewContextR);

    double        ComputeSolidPixelSize (DRange3dCR bodyRange, ViewContextR context, TransformCR transform);
    bool          WantMultiSymbologyBody ();
    bool          IsMultiSymbologyBodyValid ();
    void          BeginMultiSymbologyBody ();
    void          EndMultiSymbologyBody (ViewContextR);
    void          DrawBodyAndCacheEdges (ViewContextR, ISolidKernelEntityCR, IFaceMaterialAttachmentsCP attachments = NULL);

    bool IsCurvePrimitiveRequired ();
    bool          UseUnCachedDraw (ViewContextR context, bool isFilled);
    void          DrawOrAddComplexComponent (ICurvePrimitivePtr&, ViewContextR, bool isClosed, bool isFilled, bool is3d, double zDepth);
    void          DrawOrAddLineString (DPoint3dCP points, size_t nPoints, ViewContextR context, bool isClosed, bool isFilled, bool is3d, double zDepth);
    BeRepositoryBasedId GetSymbolId (size_t index) { return index < m_symbolIds.size() ? m_symbolIds[index] : ElementId(); }

    DGNPLATFORM_EXPORT XGraphicsOperationContext (XGraphicsHeader* hdr, DgnProjectR project, T_XGraphicsSymbolIds& symbolIds, ElementHandleCP element, XGraphicsContainer::DrawOptions drawOptions = XGraphicsContainer::DRAW_OPTION_Default);
    ~XGraphicsOperationContext();
};


//=======================================================================================
//! @bsiclass                                                     RayBentley      12/06
//=======================================================================================
struct XGraphicsOptimizeContext
{
    typedef bmap<size_t, size_t> T_InputDestToOutputBranch;

    UInt32                      m_options;
    size_t                      m_inputLocation;
    size_t                      m_outputLocation;
    T_InputDestToOutputBranch   m_branches;     // Input destination to output location.

    XGraphicsOptimizeContext (UInt32 options) : m_inputLocation(0), m_outputLocation(0), m_options (options) { }
    void    PushBranch (UInt32 branchDelta)  { m_branches[m_inputLocation + branchDelta] = m_outputLocation; }
    void    RemapBranches (XGraphicsData& optimizedData);
};

typedef XGraphicsOptimizeContext& XGraphicsOptimizeContextR;

//=======================================================================================
//! @bsiclass                                                     RayBentley      04/09
//=======================================================================================
struct XGraphicsOperator
{
    virtual ~XGraphicsOperator() {}
    virtual StatusInt _DoOperation (struct XGraphicsOperation& operation, byte* pData, UInt32 size, byte* pEnd, XGraphicsOpCodes opCode) = 0;
    virtual void _Increment (struct XGraphicsOperation& operation, byte*& pData, UInt32 dataSize);
};

//=======================================================================================
//! @bsiclass                                                     RayBentley      04/09
//=======================================================================================
struct XGraphicsConstOperator
{
    virtual ~XGraphicsConstOperator() {}
    virtual StatusInt _DoOperation (struct XGraphicsOperation& operation, byte const* pData, UInt32 size, byte const* pEnd, XGraphicsOpCodes opCode) = 0;
    virtual void _Increment (struct XGraphicsOperation& operation, byte const*& pData, UInt32 dataSize);
};

typedef XGraphicsOperator&      XGraphicsOperatorR;
typedef XGraphicsConstOperator& XGraphicsConstOperatorR;

//=======================================================================================
//! @bsiclass                                                     RayBentley      12/06
//=======================================================================================
struct XGraphicsOperation
{
    virtual void      _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) = 0;
    virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) = 0;
    virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) = 0;
    virtual StatusInt _TestTransform (TransformCR transform, byte* pData, UInt32 dataSize) {return SUCCESS;}
    virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) {return ERROR;}
    virtual bool      _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) {return 0 == memcmp (data, rhsData, dataSize);}
    virtual void      _ProcessProperties (byte* pData, UInt32 dataSize, PropertyContextR context) {}
//    virtual bool      _DeepCopyRoots (byte* pData, UInt32 dataSize, ElementCopyContextR context) {return false;} removed in graphite
    virtual StatusInt _Optimize (XGraphicsContainerR optimizedData, byte*& pData, UInt32 opSize, byte* pEnd, XGraphicsOptimizeContextR context) {return ERROR;}
    virtual StatusInt _AppendToGPA (byte const* data, UInt32 dataSize, GPArrayR gpa) { return ERROR; }
    virtual bool      _DrawBranch (byte*& pData, UInt32 dataSize, ViewContextR context, XGraphicsOperationContextR opContext);
    virtual bool      _IsUncachable (ViewContextR viewContext, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) { return false; }
    virtual bool      _IsCachable (ViewContextR viewContext, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) { return !_IsUncachable (viewContext, pData, dataSize, opContext); }
    virtual bool      _IsStateChange () { return false; }
    virtual StatusInt _OnWriteToElement (byte* pData, UInt32 dataSize, ElementHandleCR eh) { return SUCCESS; }
    virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& opSize, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext);
};

typedef struct XGraphicsOperation* XGraphicsOperationP;
typedef struct XGraphicsOperation& XGraphicsOperationR;

//=======================================================================================
//! @bsiclass                                                     RayBentley      12/06
//=======================================================================================
struct XGraphicsOperations
{
private:
    static XGraphicsOperationP s_xGraphicOps[MAX_XGraphicsOpCode];

public:
    XGraphicsOperations ();
    DGNPLATFORM_EXPORT static StatusInt GetOperation (UInt16& opCode, UInt32& opSize, byte*& pData, byte const* pEnd);
    DGNPLATFORM_EXPORT static StatusInt Traverse (byte* pData, byte* pEnd, XGraphicsOperatorR xGraphicsOperator);
    DGNPLATFORM_EXPORT static StatusInt Traverse (XGraphicsDataR data, XGraphicsOperatorR xGraphicsOperator);
    DGNPLATFORM_EXPORT static StatusInt Traverse (byte const* pData, byte const* pEnd, XGraphicsConstOperatorR xGraphicsOperator);
    DGNPLATFORM_EXPORT static StatusInt Traverse (XGraphicsDataCR data, XGraphicsConstOperatorR xGraphicsOperator);
    DGNPLATFORM_EXPORT static bool IsEqual (UInt16 opCode, byte const* thisData, byte const* otherData, UInt32 size, double distanceTolerance);
    DGNPLATFORM_EXPORT static StatusInt FindOperation (byte** ppOpStart, UInt32* pOpSize, byte** ppData, byte const* pEnd, XGraphicsOpCodes opToFind);
    DGNPLATFORM_EXPORT static StatusInt FindComplex (byte** ppStart, byte** ppEnd, byte** ppData, byte const* pEnd, XGraphicsOpCodes opCode);
    DGNPLATFORM_EXPORT static StatusInt FindBoundary (byte** ppBoundary, byte** ppEnd,  byte* pData, byte* pEnd);
    DGNPLATFORM_EXPORT static StatusInt FindBoundaries (byte** ppBoundary0, byte** ppEnd0, byte** ppBoundary1, byte** ppEnd1, byte* pData, byte* pEnd);
    DGNPLATFORM_EXPORT static StatusInt Optimize (XGraphicsContainerR optimizedData, UInt16 opCode, byte*& pData, UInt32 opSize, byte* pEnd, XGraphicsOptimizeContextR context);
    DGNPLATFORM_EXPORT static BentleyStatus CalculateRange (UInt16 opCode, byte*& pData, UInt32& opSize, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext);
    DGNPLATFORM_EXPORT static StatusInt OnTransform (UInt16 opCode, TransformInfoCR transform, XGraphicsDataR data);
    DGNPLATFORM_EXPORT static StatusInt TestTransform (TransformCR transform, byte* pData, byte* pEnd);
    DGNPLATFORM_EXPORT static StatusInt GetBasisTransform (TransformR transform, byte* pData, byte* pEnd);
    DGNPLATFORM_EXPORT static StatusInt AppendToGPA (UInt16 opCode, byte const* data, UInt32 size, GPArrayR gpa);
    DGNPLATFORM_EXPORT static void ProcessProperties (UInt16 opCode, byte* data, UInt32 size, PropertyContextR context);
//    DGNPLATFORM_EXPORT static bool      DeepCopyRoots (UInt16 opcode, byte* data, UInt32 size, ElementCopyContextR context); removed in graphite
    DGNPLATFORM_EXPORT static StatusInt OnWriteToElement (UInt16 opcode, byte* data, UInt32 size, ElementHandleCR eh);
    static StatusInt Draw (UInt16 opCode, ViewContextP context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext);
    static void StaticInitialize ();
};

//=======================================================================================
//! Support i-model publishing code that's currently not suitable for DgnPlatform
//! @bsiclass
//=======================================================================================
struct XGraphicsPublish
{
    DGNPLATFORM_EXPORT static ViewContextP CreateXGraphicsContext (XGraphicsContainer& container, UInt32 createOptions, UInt32 optimizeOptions, DgnModelR model);
    DGNPLATFORM_EXPORT static void DeleteXGraphicsContext (ViewContextP context);
    DGNPLATFORM_EXPORT static void BeginSymbol (ViewContextP context);
    DGNPLATFORM_EXPORT static void EndSymbol (ViewContextP context, DgnModelP modelRef);
};

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      04/2012
+===============+===============+===============+===============+===============+======*/
struct XGraphicsDebug
{         
    static DGNPLATFORM_EXPORT bool     DoForceQuick();
    static DGNPLATFORM_EXPORT void     SetForceQuick (bool force);

    static DGNPLATFORM_EXPORT bool     DoForceStatistics();
    static DGNPLATFORM_EXPORT void     SetForceStatistics (bool force);

};  // XGraphicsDebug

END_BENTLEY_DGNPLATFORM_NAMESPACE
