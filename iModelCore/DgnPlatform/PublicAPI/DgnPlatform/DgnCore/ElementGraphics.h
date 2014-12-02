/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ElementGraphics.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnPlatform.h>
#include "DgnFontManager.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ElementGraphics
{
enum class OpCode : UInt32
    {
    Invalid         = 0,
    Header          = 1, //!< Required to be first opcode.
    PushTransform   = 2,
    PopTransform    = 3,
    IGeometry       = 4,
    };

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct Header
    {
    UInt32          m_version:8;
    UInt32          m_reserved:24;
    UInt32          m_flags;

    Header (UInt8 version = 1, UInt32 flags = 0) : m_version (version), m_reserved (0), m_flags (flags) {}

    }; // Header

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct Operation
    {
    OpCode          m_opCode;
    UInt32          m_dataSize;
    UInt8 const*    m_data;

    Operation () : m_opCode (OpCode::Invalid), m_dataSize (0), m_data (NULL) {}
    Operation (OpCode opCode, UInt32 dataSize = 0, UInt8 const* data = NULL) : m_opCode (opCode), m_dataSize (dataSize), m_data (data) {}

    }; // Operation

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct Writer
    {
    bvector<UInt8>  m_buffer;

    Writer () {Header hdr; Append (Operation (OpCode::Header, (UInt32) sizeof (hdr), (const UInt8 *) &hdr));}

    DGNPLATFORM_EXPORT void Append (Operation const& xgOp);

    void PushTransform (TransformCR transform) {Append (Operation (OpCode::PushTransform, (UInt32) sizeof (transform), (const UInt8 *) &transform));}
    void PopTransform () {Append (Operation (OpCode::PopTransform));}

    DGNPLATFORM_EXPORT void Append (IGeometryCR geom);

    }; // Writer;
    
//=======================================================================================
//! @bsiclass
//=======================================================================================
struct Reader
    {
    static Header const* GetHeader (Operation const& xgOp) {return (OpCode::Header == xgOp.m_opCode ? (Header const*) xgOp.m_data : NULL);}
    static TransformCP GetTransform (Operation const& xgOp) {return (OpCode::PushTransform == xgOp.m_opCode ? (TransformCP) xgOp.m_data : NULL);}

    DGNPLATFORM_EXPORT static IGeometryPtr Get (Operation const& xgOp);

    }; // Reader

typedef UInt8 const* UInt8CP;

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct Iterator : std::iterator<std::forward_iterator_tag, UInt8CP const>
    {
    private:

    Operation       m_xgOp;
    UInt8CP         m_data;
    size_t          m_dataOffset;
    size_t          m_totalDataSize;

    DGNPLATFORM_EXPORT void ToNext ();

    Iterator (UInt8 const* data, size_t totalDataSize) {m_dataOffset = 0; m_data = data; m_totalDataSize = totalDataSize; ToNext ();}
    Iterator () {m_dataOffset = 0; m_data = NULL; m_totalDataSize = 0;}

    friend struct ElementGraphics;

    public:

    Iterator&        operator++() {ToNext (); return *this;}
    bool             operator==(Iterator const& rhs) const {return m_dataOffset == rhs.m_dataOffset;}
    bool             operator!=(Iterator const& rhs) const {return !(*this == rhs);}
    Operation const& operator*() const {return m_xgOp;}

    }; // Iterator

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct Collection
    {
    UInt8 const* m_data;
    size_t       m_dataSize;

    public:

    typedef Iterator const_iterator;
    typedef const_iterator iterator; //!< only const iteration is possible
    
    Collection (UInt8 const* data, size_t dataSize) : m_data (data), m_dataSize (dataSize) {}

    const_iterator begin () const {return const_iterator (m_data, m_dataSize);}
    const_iterator end   () const {return const_iterator ();}

    DGNPLATFORM_EXPORT void Draw (ViewContextR) const;

    }; // Collection

}; // ElementGraphics

//__PUBLISH_SECTION_START__

/// @addtogroup GeometryCollectors
/// @beginGroup

/*=================================================================================**//**
* Callback methods for collecting graphical output of an element's Draw method.
* @see ElementGraphicsOutput
* @bsiclass                                                     Brien.Bastings  06/2009
+===============+===============+===============+===============+===============+======*/
struct  IElementGraphicsProcessor
{
//! Controls whether pattern graphics will be output.
//! @return true to output pattern and hatch graphics, false to skip patterns.
virtual bool        _ExpandPatterns () const {return false;}

//! Controls whether linestyle graphics will be output.
//! @param[in] lsStyle Current linestyle.
//! @return true to output graphics for linestyle symbols and strokes, false to output the un-styled curve geometry.
virtual bool        _ExpandLineStyles (ILineStyleCP lsStyle) const {return false;}

//! Controls whether to output clipped graphics if any clipping is pushed by display.
//! @return true to output clipped graphics.
virtual bool        _WantClipping () const {return true;}

//! Process surfaces and solids not handled directly or are clipped through _ProcessFaceta.
//! @param[in] isPolyface facets are from a call to DrawPolyface, ex. mesh element.
//! @return true to output facets for surface and solid geometry. If returning false,
//! edge and face isoline curves will be output through _ProcessCurveVector.
//! @remarks When both _ProcessAsFacets and _ProcessAsBody return true, _ProcessAsBody has precedence.
//! @note When returning true you also need to implement _ProcessFacets.
virtual bool        _ProcessAsFacets (bool isPolyface) const {return isPolyface;}

//! Process surfaces and solids not handled directly or are clipped through _ProcessBody.
//! @param[in] isCurved Graphics output would contain non-linear edge and/or non-planar face geometry.
//! @return true to output solid kernel entities for surface and solid geometry. If returning false,
//! facets will be output if _ProcessAsFacets returns true, otherwise edge and face isoline curves will
//! be output through _ProcessCurveVector.
//! @remarks Requires host implementation of SolidsKernelAdmin methods that take or return a ISolidKernelEntity.
//! @note When returning true you also need to implement _ProcessBody.
//! @see DgnPlatformLib::Host::SolidsKernelAdmin
virtual bool        _ProcessAsBody (bool isCurved) const {return true;}

//! Supply the current context that is processing the geometry.
//! @param[in] context The current view context.
virtual void        _AnnounceContext (ViewContextR context) {}

//! Supply the current transform that subsequent geometry is displayed through.
//! @param[in] trans The transform to apply to subsequent process calls.
virtual void        _AnnounceTransform (TransformCP trans) {}

//! Supply the current symbology that subsequent geometry is displayed with.
//! @param[in] matSymb The symbology to apply to subsequent process calls.
virtual void        _AnnounceElemMatSymb (ElemMatSymbCR matSymb) {}

//! Supply the current symbology used to generate the ElemMatSym values.
//! @param[in] displayParams The symbology to apply to subsequent process calls.
virtual void        _AnnounceElemDisplayParams (ElemDisplayParamsCR displayParams) {}

//! Collect output as text.
//! @param[in] text The text data.
//! @return SUCCESS if handled, ERROR to output glyph graphics through _ProcessCurveVector.
virtual BentleyStatus   _ProcessTextString (TextStringCR text) {return SUCCESS;} // Don't export glyph geometry...

//! Collect output as a single curve component.
//! @param[in] curve The curve data.
//! @param[in] isClosed The data is from a closed path or region instead of a physically closed path.
//! @param[in] isFilled A closed path or region should have opaque fill.
//! @remarks All curve geometry can be handled through _ProcessCurveVector.
//! @see _ProcessCurveVector.
virtual BentleyStatus   _ProcessCurvePrimitive (ICurvePrimitiveCR curve, bool isClosed, bool isFilled) {return ERROR;}

//! Collect output as a CurveVector.
//! @param[in] curves The curve data.
//! @param[in] isFilled A closed path or region should have opaque fill.
//! @return SUCCESS if handled, ERROR to output individual curves through _ProcessCurvePrimitive.
virtual BentleyStatus   _ProcessCurveVector (CurveVectorCR curves, bool isFilled) {return ERROR;}

//! Collect output as a solid primitive.
//! @param[in] primitive The solid primitive data.
//! @return SUCCESS if handled, return ERROR to output according to _ProcessBody, _ProcessFacets, and _ProcessCurveVector rules.
virtual BentleyStatus   _ProcessSolidPrimitive (ISolidPrimitiveCR primitive) {return ERROR;}

//! Collect output as a bspline surface.
//! @param[in] surface The bspline surface data.
//! @return SUCCESS if handled, return ERROR to output according to _ProcessBody, _ProcessFacets, and _ProcessCurveVector rules.
virtual BentleyStatus   _ProcessSurface (MSBsplineSurfaceCR surface) {return ERROR;}

//! Collect output for surfaces and solids using a solid kernel entity.
//! @param[in] entity The solid kernel entity.
//! @param[in] attachments Optional face color/material attachment information.
//! @note Requires host implementation of SolidsKernelAdmin methods that take or return a ISolidKernelEntity.
//! @remarks Only called if _ProcessAsBody returns true.
//! @return SUCCESS if handled.
virtual BentleyStatus   _ProcessBody (ISolidKernelEntityCR entity, IFaceMaterialAttachmentsCP attachments) {return ERROR;}

//! Collect output for surfaces and solids as facets.
//! @param[in] meshData The indexed polyface data.
//! @param[in] isFilled The wireframe display of the mesh has opaque fill.
//! @remarks Only called if _ProcessAsFacets returns true.
//! @return SUCCESS if handled.
virtual BentleyStatus   _ProcessFacets (PolyfaceQueryCR meshData, bool isFilled) {return ERROR;}

//! Allow processor to override the default facet options.
//! @return A pointer to facet option structure to use or NULL to use default options.
virtual IFacetOptionsP  _GetFacetOptionsP () {return NULL;}

//! Allow processor to override the default draw purpose.
virtual DrawPurpose     _GetDrawPurpose () {return DrawPurpose::CaptureGeometry;}

//! Allow processor to output graphics to it's process methods.
//! @param[in] context The current view context.
//! @remarks The implementor is responsible for setting up the ViewContext. Might want to attach/detach a viewport, etc.
virtual void _OutputGraphics (ViewContextR context) {}

}; // IElementGraphicsProcessor

/*=================================================================================**//**
* Provides an implementation of a ViewContext and IViewDraw suitable for collecting 
* a "picture" of an element's graphics. The element handler's Draw method is called
* and the output is sent to the supplied IElementGraphicsProcessor.
* @bsiclass                                                     Brien.Bastings  06/2009
+===============+===============+===============+===============+===============+======*/
struct          ElementGraphicsOutput
{
//! Visit the supplied element and send it's Draw output to the supplied processor.
//! @param[in] processor The object to send the Draw output to.
//! @param[in] eh The element to output the graphics of.
//! @bsimethod
DGNPLATFORM_EXPORT static void Process (IElementGraphicsProcessorR processor, ElementHandleCR eh);

//! Call _OutputGraphics on the supplied processor and send whatever it draws to it's process methods.
//! @param[in] processor The object to send the Draw output to.
//! @param[in] project The DgnProject to set for the ViewContext.
//! @bsimethod
DGNPLATFORM_EXPORT static void Process (IElementGraphicsProcessorR processor, DgnProjectR project);

//__PUBLISH_SECTION_END__
DGNPLATFORM_EXPORT static void Process (IElementGraphicsProcessorR processor, IDisplaySymbolR dispSymbol, DgnProjectR project);
DGNPLATFORM_EXPORT static void Process (IElementGraphicsProcessorR processor); //! !!!FOR INTERNAL USE ONLY!!! Must call context.SetDgnProject in _OutputGraphics...
//__PUBLISH_SECTION_START__

#if !defined (DOCUMENTATION_GENERATOR)
//! Output debug information for extended element graphics to console.
DGNPLATFORM_EXPORT static void DebugXGraphics (ElementHandleCR eh);
#endif

}; // ElementGraphicsOutput

/** @endGroup */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
