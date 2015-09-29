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
#include "GeomPart.h"
#include "ViewContext.h"
#include "Annotations/TextAnnotation.h"

BEGIN_BENTLEY_DGN_NAMESPACE

typedef RefCountedPtr<ElementGeometry> ElementGeometryPtr;

//=======================================================================================
//! Class for multiple RefCounted geometry types: ICurvePrimitive, CurveVector, 
//! ISolidPrimitive, MSBsplineSurface, PolyfaceHeader, ISolidKernelEntity.
//! @ingroup ElementGeometryGroup
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
        TextString          = 7,
        };

protected:
    GeometryType                m_type;
    RefCountedPtr<IRefCounted>  m_data;

    ElementGeometry(ICurvePrimitivePtr const& source);
    ElementGeometry(CurveVectorPtr const& source);
    ElementGeometry(ISolidPrimitivePtr const& source);
    ElementGeometry(MSBsplineSurfacePtr const& source);
    ElementGeometry(PolyfaceHeaderPtr const& source);
    ElementGeometry(ISolidKernelEntityPtr const& source);
    ElementGeometry(TextStringPtr const& source);

public:
    DGNPLATFORM_EXPORT GeometryType GetGeometryType() const;

    DGNPLATFORM_EXPORT ICurvePrimitivePtr GetAsICurvePrimitive() const;
    DGNPLATFORM_EXPORT CurveVectorPtr GetAsCurveVector() const;
    DGNPLATFORM_EXPORT ISolidPrimitivePtr GetAsISolidPrimitive() const;
    DGNPLATFORM_EXPORT MSBsplineSurfacePtr GetAsMSBsplineSurface() const;
    DGNPLATFORM_EXPORT PolyfaceHeaderPtr GetAsPolyfaceHeader() const;
    DGNPLATFORM_EXPORT ISolidKernelEntityPtr GetAsISolidKernelEntity() const;
    DGNPLATFORM_EXPORT TextStringPtr GetAsTextString() const;

    DGNPLATFORM_EXPORT void Draw(ViewContextR) const;
    DGNPLATFORM_EXPORT bool GetLocalCoordinateFrame(TransformR localToWorld) const;
    DGNPLATFORM_EXPORT bool GetLocalRange(DRange3dR localRange, TransformR localToWorld) const; // Expensive - copies geometry!
    DGNPLATFORM_EXPORT bool GetRange(DRange3dR range, TransformCP transform = nullptr) const;
    DGNPLATFORM_EXPORT bool TransformInPlace(TransformCR transform);

    DGNPLATFORM_EXPORT ElementGeometryPtr Clone() const; // Deep copy

    DGNPLATFORM_EXPORT static ElementGeometryPtr Create(ICurvePrimitiveCR source);
    DGNPLATFORM_EXPORT static ElementGeometryPtr Create(CurveVectorCR source);
    DGNPLATFORM_EXPORT static ElementGeometryPtr Create(ISolidPrimitiveCR source);
    DGNPLATFORM_EXPORT static ElementGeometryPtr Create(MSBsplineSurfaceCR source);
    DGNPLATFORM_EXPORT static ElementGeometryPtr Create(PolyfaceQueryCR source);
    DGNPLATFORM_EXPORT static ElementGeometryPtr Create(ISolidKernelEntityCR source);
    DGNPLATFORM_EXPORT static ElementGeometryPtr Create(TextStringCR source);

    DGNPLATFORM_EXPORT static ElementGeometryPtr Create(ICurvePrimitivePtr const& source);
    DGNPLATFORM_EXPORT static ElementGeometryPtr Create(CurveVectorPtr const& source);
    DGNPLATFORM_EXPORT static ElementGeometryPtr Create(ISolidPrimitivePtr const& source);
    DGNPLATFORM_EXPORT static ElementGeometryPtr Create(MSBsplineSurfacePtr const& source);
    DGNPLATFORM_EXPORT static ElementGeometryPtr Create(PolyfaceHeaderPtr const& source);
    DGNPLATFORM_EXPORT static ElementGeometryPtr Create(ISolidKernelEntityPtr const& source);
    DGNPLATFORM_EXPORT static ElementGeometryPtr Create(TextStringPtr const& source);
};

//=======================================================================================
//! @private
//=======================================================================================
struct ElementGeomIO
{
    enum class OpCode : uint32_t
    {
        Invalid             = 0,
        Header              = 1,    //!< Required to be first opcode
        BeginSubCategory    = 2,    //!< Mark start of geometry having same sub-category and transform
        GeomPartInstance    = 3,    //!< Draw referenced geometry part
        BasicSymbology      = 4,    //!< Set symbology for subsequent geometry that doesn't follow subCategory appearance
        PointPrimitive      = 5,    //!< Simple lines, line strings, shapes, point strings, etc.
        PointPrimitive2d    = 6,    //!< Simple 2d lines, line strings, shapes, point strings, etc.
        ArcPrimitive        = 7,    //!< Single arc/ellipse
        CurveVector         = 8,    //!< CurveVector
        Polyface            = 9,    //!< PolyfaceQueryCarrier
        CurvePrimitive      = 10,   //!< Single CurvePrimitive
        SolidPrimitive      = 11,   //!< SolidPrimitive
        BsplineSurface      = 12,   //!< BSpline surface
        ParasolidBRep       = 13,   //!< Parasolid body
        BRepPolyface        = 14,   //!< PolyfaceQueryCarrier from Parasolid BRep
        BRepPolyfaceExact   = 15,   //!< PolyfaceQueryCarrier from Parasolid BRep with only straight edges and planar faces
        BRepEdges           = 16,   //!< CurveVector from Parasolid body edges (Not created/necessary for BRepPolyfaceExact)
        BRepFaceIso         = 17,   //!< CurveVector from Parasolid body face iso lines (Not created/necessary for BRepPolyfaceExact)
        LineStyle           = 18,   //!< Raster/vector line styles
        AreaFill            = 19,   //!< Opaque and gradient fills
        Pattern             = 20,   //!< Hatch, cross-hatch, and area pattern
        Material            = 21,   //!< Render material
        TextString          = 22,   //!< TextString (single-line/single-format run of characters)
        LineStyleModifiers  = 23,   //!< Specifies line style overrides to populate a LineStyleParams structure
    };

    //=======================================================================================
    //! Internal Header op code
    //! @private
    //=======================================================================================
    struct Header
    {
        uint32_t        m_version:8;
        uint32_t        m_reserved:24;
        uint32_t        m_flags;
        Header(uint8_t version = 1, uint32_t flags = 0) : m_version(version), m_reserved(0), m_flags(flags) {}
    };

    //=======================================================================================
    //! Internal op code
    //! @private
    //=======================================================================================
    struct Operation
    {
        OpCode          m_opCode;
        uint32_t        m_dataSize;
        uint8_t const*  m_data;

        Operation() : m_opCode(OpCode::Invalid), m_dataSize(0), m_data(nullptr) {}
        Operation(OpCode opCode, uint32_t dataSize = 0, uint8_t const* data = nullptr) : m_opCode(opCode), m_dataSize(dataSize), m_data(data) {}

        bool IsGeometryOp() const;
    };

    //=======================================================================================
    //! Internal op code writer
    //! @private
    //=======================================================================================
    struct Writer
    {
        DgnDbR m_db;
        bvector<uint8_t> m_buffer;

        Writer(DgnDbR db) : m_db(db) {AppendHeader();}
        void AppendHeader() {Header hdr; Append(Operation(OpCode::Header, (uint32_t) sizeof (hdr), (const uint8_t *) &hdr));}
        void Reset() {m_buffer.clear(); AppendHeader();};
        bool AppendSimplified(ICurvePrimitiveCR, bool isClosed, bool is3d);
        bool AppendSimplified(CurveVectorCR, bool is3d);
        bool AppendSimplified(ElementGeometryCR, bool is3d);
        void Append(Operation const& egOp);
        void Append(DPoint2dCP, size_t nPts, int8_t boundary);
        void Append(DPoint3dCP, size_t nPts, int8_t boundary);
        void Append(DEllipse3dCR, int8_t boundary);
        void Append(ICurvePrimitiveCR);
        void Append(CurveVectorCR);
        void Append(PolyfaceQueryCR);
        void Append(ISolidPrimitiveCR);
        void Append(MSBsplineSurfaceCR);
        void Append(ISolidKernelEntityCR, bool saveBRepOnly = false); // Adds multiple op-codes for when PSolid is un-available unless saveBRepOnly is true...
        void Append(ElementGeometryCR);
        void Append(DgnSubCategoryId, TransformCR geomToElem);
        void Append(DgnGeomPartId);
        void Append(Render::ElemDisplayParamsCR); // Adds multiple op-codes...
        void Append(TextStringCR);
    };

    //=======================================================================================
    //! Internal op code reader
    //! @private
    //=======================================================================================
    struct Reader
    {
        DgnDbR m_db;

        Reader(DgnDbR db) : m_db(db) {}

        static Header const* GetHeader(Operation const& egOp) {return (OpCode::Header == egOp.m_opCode ? (Header const*)egOp.m_data : nullptr);}

        bool Get(Operation const&, DPoint2dCP&, int& nPts, int8_t& boundary) const;
        bool Get(Operation const&, DPoint3dCP&, int& nPts, int8_t& boundary) const;
        bool Get(Operation const&, DEllipse3dR, int8_t& boundary) const;
        bool Get(Operation const&, PolyfaceQueryCarrier&) const;
        bool Get(Operation const&, ICurvePrimitivePtr&) const;
        bool Get(Operation const&, CurveVectorPtr&) const;
        bool Get(Operation const&, ISolidPrimitivePtr&) const;
        bool Get(Operation const&, MSBsplineSurfacePtr&) const;
        bool Get(Operation const&, ISolidKernelEntityPtr&) const;
        bool Get(Operation const&, ElementGeometryPtr&) const;
        bool Get(Operation const&, DgnSubCategoryId&, TransformR) const;
        bool Get(Operation const&, DgnGeomPartId&) const;
        bool Get(Operation const&, Render::ElemDisplayParamsR) const; // Updated by multiple op-codes, true if changed
        bool Get(Operation const&, TextStringR) const;
    }; // Reader

    //=======================================================================================
    //! Iternal op code iterator
    //! @private
    //=======================================================================================
    struct Iterator : std::iterator<std::forward_iterator_tag, uint8_t const*>
    {
    private:
        uint8_t const*  m_data;
        size_t          m_dataOffset;
        size_t          m_totalDataSize;
        Operation       m_egOp;

        void ToNext();
        Iterator(uint8_t const* data, size_t totalDataSize) : m_data(data), m_totalDataSize(totalDataSize), m_dataOffset(0) {ToNext();}
        Iterator() : m_data(nullptr), m_totalDataSize(0), m_dataOffset(0) {}

        friend struct ElementGeomIO;

    public:
        Iterator&        operator++() {ToNext(); return *this;}
        bool             operator==(Iterator const& rhs) const {return m_dataOffset == rhs.m_dataOffset;}
        bool             operator!=(Iterator const& rhs) const {return !(*this == rhs);}
        Operation const& operator*() const {return m_egOp;}

    }; // Iterator

    //=======================================================================================
    //! Internal op code helper
    //! @private
    //=======================================================================================
    struct Collection
    {
    private:
        uint8_t const*  m_data;
        size_t          m_dataSize;

    public:
        typedef Iterator const_iterator;
        typedef const_iterator iterator; //!< only const iteration is possible

        Collection(uint8_t const* data, size_t dataSize) : m_data(data), m_dataSize(dataSize) {}

        const_iterator begin() const {return const_iterator(m_data, m_dataSize);}
        const_iterator end() const {return const_iterator();}
        void GetGeomPartIds(IdSet<DgnGeomPartId>&, DgnDbR) const;
        void Draw(ViewContextR, DgnCategoryId, ViewFlags) const;
    };

    //! Remap embedded IDs. The dest and source GeomStreams can be the same.
    //! @param dest     The output GeomStream
    //! @param source   The input GeomStream
    //! @param remapper  The ID remapper
    static DgnDbStatus Import(GeomStreamR dest, GeomStreamCR source, DgnImportContext& remapper);


}; // ElementGeomIO

//=======================================================================================
//! ElementGeometryCollection provides iterator for a Geometric Element's GeomStream.
//! @ingroup ElementGeometryGroup
//=======================================================================================
struct ElementGeometryCollection
{
enum class BRepOutput
    {
    None    = 0,       //!< Output nothing.
    BRep    = 1,       //!< Output ISolidKernelEntity when solids kernel is available and Polyface(s) when it is not. (Default)
    Mesh    = 1 << 1,  //!< Output Polyface(s) even if solids kernel is available.
    Edges   = 1 << 2,  //!< Output CurveVector for edges.
    FaceIso = 1 << 3,  //!< Output CurveVector for face hatch lines.
    };

//=======================================================================================
//! Iterator
//! @ingroup ElementGeometryGroup
//=======================================================================================
struct Iterator : std::iterator<std::forward_iterator_tag, uint8_t const*>
    {
    private:

    uint8_t const*      m_data;
    size_t              m_dataOffset;
    size_t              m_totalDataSize;
    ViewContextP        m_context;
    bool                m_geomToElemPushed;
    ElementGeometryPtr  m_elementGeometry;
    DgnGeomPartPtr      m_partGeometry;
    uint8_t const*      m_saveData;
    size_t              m_saveDataOffset;
    size_t              m_saveTotalDataSize;
    BRepOutput          m_bRepOutput;

    DGNPLATFORM_EXPORT void ToNext ();

    Iterator (uint8_t const* data, size_t totalDataSize, ViewContextP context, BRepOutput bRep) : m_data(data), m_totalDataSize(totalDataSize), m_dataOffset(0), m_context(context), m_geomToElemPushed(false), m_bRepOutput(bRep) {ToNext();}
    Iterator () : m_data(nullptr), m_totalDataSize(0), m_dataOffset(0), m_context(nullptr), m_bRepOutput(BRepOutput::BRep) {}

    friend struct ElementGeometryCollection;

    public:

    Iterator&           operator++() {ToNext (); return *this;}
    bool                operator==(Iterator const& rhs) const {return m_dataOffset == rhs.m_dataOffset;}
    bool                operator!=(Iterator const& rhs) const {return !(*this == rhs);}
    ElementGeometryPtr  operator*() const {return m_elementGeometry;}

    }; // Iterator

private:

uint8_t const*      m_data;
size_t              m_dataSize;
ViewContextP        m_context;
Transform           m_elemToWorld;
Transform           m_geomToElem;
Transform           m_geomToWorld;
BRepOutput          m_bRepOutput;

public:

typedef Iterator const_iterator;
typedef const_iterator iterator; //!< only const iteration is possible
    
const_iterator begin () const {return const_iterator (m_data, m_dataSize, m_context, m_bRepOutput);}
const_iterator end   () const {return const_iterator ();}

void SetBRepOutput(BRepOutput bRep) {m_bRepOutput = bRep;}

DGNPLATFORM_EXPORT Render::ElemDisplayParamsCR GetElemDisplayParams();
DGNPLATFORM_EXPORT GeomStreamEntryId GetGeomStreamEntryId(); //!< Returns primitive id for current geometry
DGNPLATFORM_EXPORT TransformCR GetElementToWorld();
DGNPLATFORM_EXPORT TransformCR GetGeometryToWorld();
DGNPLATFORM_EXPORT TransformCR GetGeometryToElement();

DGNPLATFORM_EXPORT ElementGeometryCollection (DgnDbR dgnDb, GeomStreamCR geom);
DGNPLATFORM_EXPORT ElementGeometryCollection (GeometricElementCR element);
DGNPLATFORM_EXPORT ~ElementGeometryCollection ();

}; // ElementGeometryCollection

typedef RefCountedPtr<ElementGeometryBuilder> ElementGeometryBuilderPtr;

ENUM_IS_FLAGS(ElementGeometryCollection::BRepOutput)

//=======================================================================================
//! ElementGeometryBuilder provides methods for setting up an element's GeomStream and Placement(2d/3d).
//! An element's geometry should always be stored relative to its placement. A placement defines the
//! element to world transform.
//!
//! Example: To create a 10m line defined by its start point and located at 5,5,0 you could call Create and specify a 
//!          placement origin of 5,5,0 and append a line curve primitive with start point 0,0,0 and endpoint 10,0,0.
//!          This result could also be achieve more simply by using CreateWorld and supplying a line from 5,5,0 to 15,5,0.
//!
//!          A reason to use Create instead of CreateWorld is to specify a placement that is more meaningful to 
//!          the application. The application might want to set things up so that the element origin was the line’s 
//!          midpoint instead of the start point. In this case the line curve primitive would be created with start 
//!          point -5,0,0 and end point 5,0,0. To create a line parallel to the y axis instead of x, specify a yaw angle of 90, etc.
//!
//! An element can be "moved" simply by updating it's placement. It may be convenient for the the application to initially specify
//! an identity placement, append the geometry, and then just update the placement to reflect the element’s world coordinates and
//! orientation. It’s not expected for the placement to remain identity unless the element is really at the origin.
//!
//! For repeated geometry that can be shared in a single GeomStream or by multiple GeomStreams, a DgnGeomPart should be
//! created. When appending a DgnGeomPartId you specify the part geometry to element transform in order to position the 
//! part's geometry relative to the other geometry/parts display by the element.
//! @ingroup ElementGeometryGroup
//=======================================================================================
struct ElementGeometryBuilder : RefCountedBase
{
private:

bool                    m_appearanceChanged;
bool                    m_haveLocalGeom;
bool                    m_havePlacement;
bool                    m_isPartCreate;
bool                    m_is3d;
Placement3d             m_placement3d;
Placement2d             m_placement2d;
DgnDbR                  m_dgnDb;
DgnSubCategoryId        m_prevSubCategory;
Transform               m_prevGeomToElem;
Render::ElemDisplayParams       m_elParams;
ElementGeomIO::Writer   m_writer;

ElementGeometryBuilder (DgnDbR dgnDb, DgnCategoryId categoryId, Placement3dCR placement);
ElementGeometryBuilder (DgnDbR dgnDb, DgnCategoryId categoryId, Placement2dCR placement);
ElementGeometryBuilder (DgnDbR dgnDb, DgnCategoryId categoryId, bool is3d);

bool ConvertToLocal (ElementGeometryR);
bool AppendWorld (ElementGeometryR);
bool AppendLocal (ElementGeometryCR);
void OnNewGeom (DRange3dCR localRange, TransformCP geomToElement);

public:

DGNPLATFORM_EXPORT BentleyStatus GetGeomStream (GeomStreamR);
DGNPLATFORM_EXPORT BentleyStatus GetPlacement (Placement3dR);

DGNPLATFORM_EXPORT BentleyStatus SetGeomStream (DgnGeomPartR);
DGNPLATFORM_EXPORT BentleyStatus SetGeomStreamAndPlacement (GeometricElementR);

DGNPLATFORM_EXPORT bool Append (DgnSubCategoryId);
DGNPLATFORM_EXPORT bool Append (Render::ElemDisplayParamsCR);
DGNPLATFORM_EXPORT bool Append (DgnGeomPartId, TransformCR geomToElement); //! Placement must already be specified, not valid for CreateWorld.
DGNPLATFORM_EXPORT bool Append (ElementGeometryCR);
DGNPLATFORM_EXPORT bool Append (ICurvePrimitiveCR);
DGNPLATFORM_EXPORT bool Append (CurveVectorCR);
DGNPLATFORM_EXPORT bool Append (ISolidPrimitiveCR); //! 3d only
DGNPLATFORM_EXPORT bool Append (MSBsplineSurfaceCR); //! 3d only
DGNPLATFORM_EXPORT bool Append (PolyfaceQueryCR); //! 3d only
DGNPLATFORM_EXPORT bool Append (ISolidKernelEntityCR); //! 3d only
DGNPLATFORM_EXPORT bool Append (TextStringCR);
DGNPLATFORM_EXPORT bool Append (TextAnnotationCR);

DGNPLATFORM_EXPORT static ElementGeometryBuilderPtr CreateGeomPart (DgnDbR db, bool is3d);

DGNPLATFORM_EXPORT static ElementGeometryBuilderPtr Create (DgnModelR model, DgnCategoryId categoryId, DPoint3dCR origin, YawPitchRollAngles const& angles = YawPitchRollAngles());
DGNPLATFORM_EXPORT static ElementGeometryBuilderPtr Create (DgnModelR model, DgnCategoryId categoryId, DPoint2dCR origin, AngleInDegrees const& angle = AngleInDegrees());
DGNPLATFORM_EXPORT static ElementGeometryBuilderPtr CreateWorld (DgnModelR model, DgnCategoryId categoryId);

DGNPLATFORM_EXPORT static ElementGeometryBuilderPtr Create (DgnElement3dCR, DPoint3dCR origin, YawPitchRollAngles const& angles = YawPitchRollAngles());
DGNPLATFORM_EXPORT static ElementGeometryBuilderPtr Create (DgnElement2dCR, DPoint2dCR origin, AngleInDegrees const& angle = AngleInDegrees());
DGNPLATFORM_EXPORT static ElementGeometryBuilderPtr Create (GeometricElementCR); //! Create builder from element using current placement.
DGNPLATFORM_EXPORT static ElementGeometryBuilderPtr CreateWorld (GeometricElementCR);

}; // ElementGeometryBuilder

END_BENTLEY_DGN_NAMESPACE
