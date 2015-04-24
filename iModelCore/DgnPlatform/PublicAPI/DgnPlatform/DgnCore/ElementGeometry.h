/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ElementGeometry.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "../DgnPlatform.h"
#include "SolidKernel.h"
#include "ViewContext.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef RefCountedPtr<ElementGeometry> ElementGeometryPtr;

//=======================================================================================
//! Class for multiple RefCounted geometry types: ICurvePrimitive, CurveVector, 
//! ISolidPrimitive, MSBsplineSurface, PolyfaceHeader, ISolidKernelEntity.
//! @bsiclass 
//=======================================================================================
struct ElementGeometry : RefCountedBase
{
public:

enum class GeometryType
    {
    CurvePrimitive      = 1,
    CurveVector         = 2,
    SolidPrimitive      = 3,
    BsplineSurface      = 4,
    Polyface            = 5,
    SolidKernelEntity   = 6,
    };

//__PUBLISH_SECTION_END__
protected:

GeometryType                m_type;
RefCountedPtr<IRefCounted>  m_data;

ElementGeometry (ICurvePrimitivePtr const& source);
ElementGeometry (CurveVectorPtr const& source);
ElementGeometry (ISolidPrimitivePtr const& source);
ElementGeometry (MSBsplineSurfacePtr const& source);
ElementGeometry (PolyfaceHeaderPtr const& source);
ElementGeometry (ISolidKernelEntityPtr const& source);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

DGNPLATFORM_EXPORT GeometryType GetGeometryType () const;

DGNPLATFORM_EXPORT ICurvePrimitivePtr GetAsICurvePrimitive () const;
DGNPLATFORM_EXPORT CurveVectorPtr GetAsCurveVector () const;
DGNPLATFORM_EXPORT ISolidPrimitivePtr GetAsISolidPrimitive () const;
DGNPLATFORM_EXPORT MSBsplineSurfacePtr GetAsMSBsplineSurface () const;
DGNPLATFORM_EXPORT PolyfaceHeaderPtr GetAsPolyfaceHeader () const;
DGNPLATFORM_EXPORT ISolidKernelEntityPtr GetAsISolidKernelEntity () const;

DGNPLATFORM_EXPORT bool GetLocalCoordinateFrame (TransformR localToWorld) const;
DGNPLATFORM_EXPORT bool GetLocalRange (DRange3dR localRange, TransformR localToWorld) const; // Expensive - copies geometry!
DGNPLATFORM_EXPORT bool GetRange (DRange3dR range, TransformCP transform = nullptr) const;
DGNPLATFORM_EXPORT bool TransformInPlace (TransformCR transform);

DGNPLATFORM_EXPORT ElementGeometryPtr Clone () const; // Deep copy

DGNPLATFORM_EXPORT static ElementGeometryPtr Create (ICurvePrimitiveCR source);
DGNPLATFORM_EXPORT static ElementGeometryPtr Create (CurveVectorCR source);
DGNPLATFORM_EXPORT static ElementGeometryPtr Create (ISolidPrimitiveCR source);
DGNPLATFORM_EXPORT static ElementGeometryPtr Create (MSBsplineSurfaceCR source);
DGNPLATFORM_EXPORT static ElementGeometryPtr Create (PolyfaceQueryCR source);
DGNPLATFORM_EXPORT static ElementGeometryPtr Create (ISolidKernelEntityCR source);

DGNPLATFORM_EXPORT static ElementGeometryPtr Create (ICurvePrimitivePtr const& source);
DGNPLATFORM_EXPORT static ElementGeometryPtr Create (CurveVectorPtr const& source);
DGNPLATFORM_EXPORT static ElementGeometryPtr Create (ISolidPrimitivePtr const& source);
DGNPLATFORM_EXPORT static ElementGeometryPtr Create (MSBsplineSurfacePtr const& source);
DGNPLATFORM_EXPORT static ElementGeometryPtr Create (PolyfaceHeaderPtr const& source);
DGNPLATFORM_EXPORT static ElementGeometryPtr Create (ISolidKernelEntityPtr const& source);

}; // ElementGeometry

//__PUBLISH_SECTION_END__
//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ElementGeomIO
{
enum class OpCode : uint32_t
    {
    Invalid             = 0,
    Header              = 1,    //!< Required to be first opcode
    BeginSubCategory    = 2,    //!< Mark start of geometry having same sub-category and transform
    GeomPartInstance    = 4,    //!< Draw referenced geometry part
    BasicSymbology      = 5,    //!< Set symbology for subsequent geometry that doesn't follow subCategory appearance
    PointPrimitive      = 6,    //!< Simple lines, line strings, shapes, point strings, etc.
    PointPrimitive2d    = 7,    //!< Simple 2d lines, line strings, shapes, point strings, etc.
    ArcPrimitive        = 8,    //!< Single arc/ellipse
    CurveVector         = 9,    //!< CurveVector
    CurveVectorFilled   = 10,   //!< Region CurveVector with fill
    Polyface            = 11,   //!< PolyfaceQueryCarrier
    PolyfaceFilled      = 12,   //!< PolyfaceQueryCarrier with fill
    CurvePrimitive      = 13,   //!< Single CurvePrimitive
    SolidPrimitive      = 14,   //!< SolidPrimitive
    BsplineSurface      = 15,   //!< BSpline surface
    ParasolidBRep       = 16,   //!< Parasolid body
    BRepPolyface        = 17,   //!< PolyfaceQueryCarrier from Parasolid BRep
    BRepPolyfaceExact   = 18,   //!< PolyfaceQueryCarrier from Parasolid BRep with only straight edges and planar faces
    BRepEdges           = 19,   //!< CurveVector from Parasolid body edges (Not created/necessary for BRepPolyfaceExact)
    BRepFaceIso         = 20,   //!< CurveVector from Parasolid body face iso lines (Not created/necessary for BRepPolyfaceExact)
    LineStyle           = 21,   //!< Raster/vector line styles
    AreaFill            = 22,   //!< Opaque and gradient fills
    Pattern             = 23,   //!< Hatch, cross-hatch, and area pattern
    Material            = 24,   //!< Render material
    };

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct Header
    {
    uint32_t        m_version:8;
    uint32_t        m_reserved:24;
    uint32_t        m_flags;

    Header (uint8_t version = 1, uint32_t flags = 0) : m_version (version), m_reserved (0), m_flags (flags) {}

    }; // Header

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct Operation
    {
    OpCode          m_opCode;
    uint32_t        m_dataSize;
    uint8_t const*  m_data;

    Operation () : m_opCode (OpCode::Invalid), m_dataSize (0), m_data (nullptr) {}
    Operation (OpCode opCode, uint32_t dataSize = 0, uint8_t const* data = nullptr) : m_opCode (opCode), m_dataSize (dataSize), m_data (data) {}

    }; // Operation

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct Writer
    {
    bvector<uint8_t>  m_buffer;

    Writer () {AppendHeader ();}

    void AppendHeader () {Header hdr; Append (Operation (OpCode::Header, (uint32_t) sizeof (hdr), (const uint8_t *) &hdr));}
    void Reset () {m_buffer.clear (); AppendHeader ();};

    DGNPLATFORM_EXPORT void Append (Operation const& egOp);    
    DGNPLATFORM_EXPORT void Append (DPoint3dCP, size_t nPts, int8_t boundary);
    DGNPLATFORM_EXPORT void Append (DEllipse3dCR, int8_t boundary);
    DGNPLATFORM_EXPORT void Append (ICurvePrimitiveCR, bool isClosed = false, bool isFilled = false);
    DGNPLATFORM_EXPORT void Append (CurveVectorCR, bool isFilled = false);
    DGNPLATFORM_EXPORT void Append (PolyfaceQueryCR, bool isFilled = false);
    DGNPLATFORM_EXPORT void Append (ISolidPrimitiveCR);
    DGNPLATFORM_EXPORT void Append (MSBsplineSurfaceCR);
    DGNPLATFORM_EXPORT void Append (ISolidKernelEntityCR, IFaceMaterialAttachmentsCP attachments = nullptr, bool saveBRepOnly = false); // Adds multiple op-codes for when PSolid is un-available unless saveBRepOnly is true...
    DGNPLATFORM_EXPORT void Append (ElementGeometryCR);
    DGNPLATFORM_EXPORT void Append (DgnSubCategoryId, TransformCR geomToWorld);
    DGNPLATFORM_EXPORT void Append (DgnGeomPartId);
    DGNPLATFORM_EXPORT void Append (ElemDisplayParamsCR); // Adds multiple op-codes...

    }; // Writer;
    
//=======================================================================================
//! @bsiclass
//=======================================================================================
struct Reader
    {
    static Header const* GetHeader (Operation const& egOp) {return (OpCode::Header == egOp.m_opCode ? (Header const*) egOp.m_data : nullptr);}

    DGNPLATFORM_EXPORT static bool Get (Operation const&, DPoint3dCP&, int& nPts, int8_t& boundary);
    DGNPLATFORM_EXPORT static bool Get (Operation const&, DEllipse3dR, int8_t& boundary);
    DGNPLATFORM_EXPORT static bool Get (Operation const&, PolyfaceQueryCarrier&, bool& isFilled);
    DGNPLATFORM_EXPORT static bool Get (Operation const&, ICurvePrimitivePtr&);
    DGNPLATFORM_EXPORT static bool Get (Operation const&, CurveVectorPtr&, bool& isFilled);
    DGNPLATFORM_EXPORT static bool Get (Operation const&, ISolidPrimitivePtr&);
    DGNPLATFORM_EXPORT static bool Get (Operation const&, MSBsplineSurfacePtr&);
    DGNPLATFORM_EXPORT static bool Get (Operation const&, ISolidKernelEntityPtr&);
    DGNPLATFORM_EXPORT static bool Get (Operation const&, ElementGeometryPtr&);
    DGNPLATFORM_EXPORT static bool Get (Operation const&, DgnSubCategoryId&, TransformR);
    DGNPLATFORM_EXPORT static bool Get (Operation const&, DgnGeomPartId&);
    DGNPLATFORM_EXPORT static bool Get (Operation const&, ElemDisplayParamsR); // Updated by multiple op-codes, true if changed

    }; // Reader

typedef uint8_t const* UInt8CP;

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct Iterator : std::iterator<std::forward_iterator_tag, UInt8CP const>
    {
    private:

    Operation       m_egOp;
    UInt8CP         m_data;
    size_t          m_dataOffset;
    size_t          m_totalDataSize;

    DGNPLATFORM_EXPORT void ToNext ();

    Iterator (uint8_t const* data, size_t totalDataSize) {m_dataOffset = 0; m_data = data; m_totalDataSize = totalDataSize; ToNext ();}
    Iterator () {m_dataOffset = 0; m_data = NULL; m_totalDataSize = 0;}

    friend struct ElementGeomIO;

    public:

    Iterator&        operator++() {ToNext (); return *this;}
    bool             operator==(Iterator const& rhs) const {return m_dataOffset == rhs.m_dataOffset;}
    bool             operator!=(Iterator const& rhs) const {return !(*this == rhs);}
    Operation const& operator*() const {return m_egOp;}

    }; // Iterator

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct Collection
    {
    uint8_t const*  m_data;
    size_t          m_dataSize;

    public:

    typedef Iterator const_iterator;
    typedef const_iterator iterator; //!< only const iteration is possible
    
    Collection (uint8_t const* data, size_t dataSize) : m_data (data), m_dataSize (dataSize) {}

    const_iterator begin () const {return const_iterator (m_data, m_dataSize);}
    const_iterator end   () const {return const_iterator ();}

    DGNPLATFORM_EXPORT void Draw (ViewContextR, DgnCategoryId, bool isQVis, bool isQVWireframe, bool isPick, bool useBRep) const; // NEEDSWORK: Simplify...
    DGNPLATFORM_EXPORT BentleyStatus ToGeometry (bvector<ElementGeometryPtr>&) const;

    }; // Collection

}; // ElementGeomIO

//__PUBLISH_SECTION_START__
typedef RefCountedPtr<ElementGeometryBuilder> ElementGeometryBuilderPtr;

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ElementGeometryBuilder : RefCountedBase
{
//__PUBLISH_SECTION_END__
protected:

bool                    m_appearanceChanged;
bool                    m_haveLocalGeom;
bool                    m_havePlacement;
Placement3d             m_placement3d;
Placement2d             m_placement2d;
DgnModelR               m_model;
DgnSubCategoryId        m_prevSubCategory;
Transform               m_prevGeomToWorld;
ElemDisplayParams       m_elParams;
ElementGeomIO::Writer   m_writer;

ElementGeometryBuilder (DgnModelR model, DgnCategoryId categoryId, Placement3dCR placement);
ElementGeometryBuilder (DgnModelR model, DgnCategoryId categoryId, Placement2dCR placement);
ElementGeometryBuilder (DgnModelR model, DgnCategoryId categoryId);

bool ConvertToLocal (ElementGeometryR);
bool AppendWorld (ElementGeometryR, bool deferWrite = false);
bool AppendLocal (ElementGeometryCR, TransformCP geomToElement, bool deferWrite = false);
void OnNewGeom (DRange3dCR localRange, TransformCP geomToElement);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

DGNPLATFORM_EXPORT BentleyStatus SetGeomStreamAndPlacement (GeometricElementR);

DGNPLATFORM_EXPORT bool Append (DgnSubCategoryId);
DGNPLATFORM_EXPORT bool Append (ElemDisplayParamsCR);
DGNPLATFORM_EXPORT bool Append (DgnGeomPartId, TransformCP geomToElement = nullptr);
DGNPLATFORM_EXPORT bool Append (ElementGeometryCR, TransformCP geomToElement = nullptr);
DGNPLATFORM_EXPORT bool Append (ICurvePrimitiveCR, TransformCP geomToElement = nullptr);
DGNPLATFORM_EXPORT bool Append (CurveVectorCR, TransformCP geomToElement = nullptr);
DGNPLATFORM_EXPORT bool Append (ISolidPrimitiveCR, TransformCP geomToElement = nullptr);
DGNPLATFORM_EXPORT bool Append (MSBsplineSurfaceCR, TransformCP geomToElement = nullptr);
DGNPLATFORM_EXPORT bool Append (PolyfaceQueryCR, TransformCP geomToElement = nullptr);
DGNPLATFORM_EXPORT bool Append (ISolidKernelEntityCR, TransformCP geomToElement = nullptr, IFaceMaterialAttachmentsCP attachments = nullptr);

DGNPLATFORM_EXPORT static ElementGeometryBuilderPtr Create (DgnModelR model, DgnCategoryId categoryId, DPoint3dCR origin, YawPitchRollAngles angles = YawPitchRollAngles());
DGNPLATFORM_EXPORT static ElementGeometryBuilderPtr Create (DgnModelR model, DgnCategoryId categoryId, DPoint2dCR origin, double angle = 0.0);
DGNPLATFORM_EXPORT static ElementGeometryBuilderPtr CreateWorld (DgnModelR model, DgnCategoryId categoryId);

DGNPLATFORM_EXPORT static ElementGeometryBuilderPtr Create (DgnElement3dCR, DPoint3dCR origin, YawPitchRollAngles angles = YawPitchRollAngles());
DGNPLATFORM_EXPORT static ElementGeometryBuilderPtr Create (DgnElement2dCR, DPoint2dCR origin, double angle = 0.0);
DGNPLATFORM_EXPORT static ElementGeometryBuilderPtr CreateWorld (GeometricElementCR);

}; // ElementGeometryBuilder

END_BENTLEY_DGNPLATFORM_NAMESPACE
