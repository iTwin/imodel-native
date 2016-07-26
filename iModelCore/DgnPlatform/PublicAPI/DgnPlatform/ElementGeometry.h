/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ElementGeometry.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"
#include "SolidKernel.h"
#include "GeomPart.h"
#include "ViewContext.h"
#include "Annotations/TextAnnotation.h"

BEGIN_BENTLEY_DGN_NAMESPACE

typedef RefCountedPtr<GeometricPrimitive> GeometricPrimitivePtr;

//=======================================================================================
//! Class for multiple RefCounted geometry types: ICurvePrimitive, CurveVector, 
//! ISolidPrimitive, MSBsplineSurface, PolyfaceHeader, ISolidKernelEntity.
//! @ingroup GROUP_Geometry
//=======================================================================================
struct GeometricPrimitive : RefCountedBase
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

    GeometricPrimitive(ICurvePrimitivePtr const& source);
    GeometricPrimitive(CurveVectorPtr const& source);
    GeometricPrimitive(ISolidPrimitivePtr const& source);
    GeometricPrimitive(MSBsplineSurfacePtr const& source);
    GeometricPrimitive(PolyfaceHeaderPtr const& source);
    GeometricPrimitive(ISolidKernelEntityPtr const& source);
    GeometricPrimitive(TextStringPtr const& source);

public:
    DGNPLATFORM_EXPORT GeometryType GetGeometryType() const;

    //! Return true if the geometry is or would be represented by a solid body. Accepted geometry includes BRep solids, capped SolidPrimitves, and closed Polyfaces. 
    DGNPLATFORM_EXPORT bool IsSolid() const;

    //! Return true if the geometry is or would be represented by a sheet body. Accepted geometry includes BRep sheets, un-capped SolidPrimitives, region CurveVectors, Bspline Surfaces, and unclosed Polyfaces.
    DGNPLATFORM_EXPORT bool IsSheet() const;

    //! Return true if the geometry is or would be represented by a wire body. Accepted geometry includes BRep wires and CurveVectors.
    DGNPLATFORM_EXPORT bool IsWire() const;

    // Return the type of solid kernel entity that would be used to represent this geometry.
    DGNPLATFORM_EXPORT ISolidKernelEntity::KernelEntityType GetKernelEntityType() const;

    DGNPLATFORM_EXPORT ICurvePrimitivePtr GetAsICurvePrimitive() const;
    DGNPLATFORM_EXPORT CurveVectorPtr GetAsCurveVector() const;
    DGNPLATFORM_EXPORT ISolidPrimitivePtr GetAsISolidPrimitive() const;
    DGNPLATFORM_EXPORT MSBsplineSurfacePtr GetAsMSBsplineSurface() const;
    DGNPLATFORM_EXPORT PolyfaceHeaderPtr GetAsPolyfaceHeader() const;
    DGNPLATFORM_EXPORT ISolidKernelEntityPtr GetAsISolidKernelEntity() const;
    DGNPLATFORM_EXPORT TextStringPtr GetAsTextString() const;

    DGNPLATFORM_EXPORT void AddToGraphic(Render::GraphicBuilderR) const;
    DGNPLATFORM_EXPORT bool GetLocalCoordinateFrame(TransformR localToWorld) const;
    DGNPLATFORM_EXPORT bool GetLocalRange(DRange3dR localRange, TransformR localToWorld) const; // Expensive - copies geometry!
    DGNPLATFORM_EXPORT bool GetRange(DRange3dR range, TransformCP transform = nullptr) const;
    DGNPLATFORM_EXPORT bool TransformInPlace(TransformCR transform);

    DGNPLATFORM_EXPORT GeometricPrimitivePtr Clone() const; // Deep copy

    DGNPLATFORM_EXPORT static GeometricPrimitivePtr Create(ICurvePrimitiveCR source);
    DGNPLATFORM_EXPORT static GeometricPrimitivePtr Create(CurveVectorCR source);
    DGNPLATFORM_EXPORT static GeometricPrimitivePtr Create(ISolidPrimitiveCR source);
    DGNPLATFORM_EXPORT static GeometricPrimitivePtr Create(MSBsplineSurfaceCR source);
    DGNPLATFORM_EXPORT static GeometricPrimitivePtr Create(PolyfaceQueryCR source);
    DGNPLATFORM_EXPORT static GeometricPrimitivePtr Create(ISolidKernelEntityCR source);
    DGNPLATFORM_EXPORT static GeometricPrimitivePtr Create(TextStringCR source);

    DGNPLATFORM_EXPORT static GeometricPrimitivePtr Create(ICurvePrimitivePtr const& source);
    DGNPLATFORM_EXPORT static GeometricPrimitivePtr Create(CurveVectorPtr const& source);
    DGNPLATFORM_EXPORT static GeometricPrimitivePtr Create(ISolidPrimitivePtr const& source);
    DGNPLATFORM_EXPORT static GeometricPrimitivePtr Create(MSBsplineSurfacePtr const& source);
    DGNPLATFORM_EXPORT static GeometricPrimitivePtr Create(PolyfaceHeaderPtr const& source);
    DGNPLATFORM_EXPORT static GeometricPrimitivePtr Create(ISolidKernelEntityPtr const& source);
    DGNPLATFORM_EXPORT static GeometricPrimitivePtr Create(TextStringPtr const& source);

}; // GeometricPrimitive

//=======================================================================================
//! @private
//! @note If adding a new "geometry" OpCode, update Operation::IsGeometryOp!
//=======================================================================================
struct GeometryStreamIO
{
    enum class OpCode : uint32_t
    {
        Invalid                 = 0,
        Header                  = 1,    //!< Required to be first opcode
        SubGraphicRange         = 2,    //!< Local range of next geometric primitive
        GeometryPartInstance    = 3,    //!< Draw referenced geometry part
        BasicSymbology          = 4,    //!< Set symbology for subsequent geometry that doesn't follow subCategory appearance
        PointPrimitive          = 5,    //!< Simple lines, line strings, shapes, point strings, etc.
        PointPrimitive2d        = 6,    //!< Simple 2d lines, line strings, shapes, point strings, etc.
        ArcPrimitive            = 7,    //!< Single arc/ellipse
        CurveVector             = 8,    //!< CurveVector
        Polyface                = 9,    //!< PolyfaceQueryCarrier
        CurvePrimitive          = 10,   //!< Single CurvePrimitive
        SolidPrimitive          = 11,   //!< SolidPrimitive
        BsplineSurface          = 12,   //!< BSpline surface
        AreaFill                = 19,   //!< Opaque and gradient fills
        Pattern                 = 20,   //!< Hatch, cross-hatch, and area pattern
        Material                = 21,   //!< Render material
        TextString              = 22,   //!< TextString (single-line/single-format run of characters)
        LineStyleModifiers      = 23,   //!< Specifies line style overrides to populate a LineStyleParams structure
#if defined (BENTLEYCONFIG_OPENCASCADE)
        OpenCascadeBRep         = 24,   //!< Open Cascade TopoDS_Shape
#endif
    };

    //=======================================================================================
    //! Internal Header op code
    //! @private
    //=======================================================================================
    struct Header
    {
        uint32_t    m_version:8;
        uint32_t    m_reserved:24;
        uint32_t    m_flags;

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
        void AppendHeader(uint32_t flags = 0) {Header hdr(1, flags); Append(Operation(OpCode::Header, (uint32_t) sizeof (hdr), (const uint8_t *) &hdr));}
        void Reset(uint32_t flags = 0) {m_buffer.clear(); AppendHeader(flags);};
        bool AppendSimplified(ICurvePrimitiveCR, bool isClosed, bool is3d);
        bool AppendSimplified(CurveVectorCR, bool is3d);
        bool AppendSimplified(GeometricPrimitiveCR, bool is3d);
        void Append(Operation const& egOp);
        void Append(DPoint2dCP, size_t nPts, int8_t boundary);
        void Append(DPoint3dCP, size_t nPts, int8_t boundary);
        void Append(DEllipse3dCR, int8_t boundary);
        void Append(ICurvePrimitiveCR);
        void Append(CurveVectorCR, OpCode opCode = OpCode::CurveVector);
        void Append(PolyfaceQueryCR, OpCode opCode = OpCode::Polyface);
        void Append(ISolidPrimitiveCR);
        void Append(MSBsplineSurfaceCR);
        void Append(ISolidKernelEntityCR);
        void Append(GeometricPrimitiveCR);
        void Append(DgnGeometryPartId, TransformCP geomToElem);
        void Append(Render::GeometryParamsCR, bool ignoreSubCategory); // Adds multiple op-codes...
        void Append(TextStringCR);
        void Append(DRange3dCR);
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
        bool Get(Operation const&, GeometricPrimitivePtr&) const;
        bool Get(Operation const&, DgnGeometryPartId&, TransformR) const;
        bool Get(Operation const&, Render::GeometryParamsR) const; // Updated by multiple op-codes, true if changed
        bool Get(Operation const&, TextStringR) const;
        bool Get(Operation const&, DRange3dR) const;
    };

    //=======================================================================================
    //! Iternal op code iterator
    //! @private
    //=======================================================================================
    struct Iterator : std::iterator<std::forward_iterator_tag, uint8_t const*>
    {
    private:
        uint8_t const*  m_data;
        size_t          m_dataSize;
        size_t          m_dataOffset;
        Operation       m_egOp;

        void ToNext();
        Iterator(uint8_t const* data, size_t dataSize) : m_data(data), m_dataSize(dataSize), m_dataOffset(0) {ToNext();}
        Iterator() : m_data(nullptr), m_dataSize(0), m_dataOffset(0) {}

        friend struct GeometryStreamIO;

    public:
        Iterator&        operator++() {ToNext(); return *this;}
        bool             operator==(Iterator const& rhs) const {return m_dataOffset == rhs.m_dataOffset;}
        bool             operator!=(Iterator const& rhs) const {return !(*this == rhs);}
        Operation const& operator*() const {return m_egOp;}
    };

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
        void GetGeometryPartIds(BeSQLite::IdSet<DgnGeometryPartId>&, DgnDbR) const;
        void Draw(Render::GraphicBuilderR, ViewContextR, Render::GeometryParamsR, bool activateParams=true, DgnElementCP=nullptr) const;
    };

    //=======================================================================================
    //! Internal debug helper
    //! @private
    //=======================================================================================
    struct IDebugOutput
    {
        virtual bool _WantVerbose() const {return false;}
        virtual bool _WantPartGeometry() const {return true;}
        virtual bool _WantGeomEntryIds() const {return false;}
        virtual void _DoOutputLine(Utf8CP msg) = 0;
    };

    //! Output contents of GeometryStream for debugging purposes.
    DGNPLATFORM_EXPORT static void Debug(IDebugOutput&, GeometryStreamCR, DgnDbR, bool isPart=false);

    //! Remap embedded IDs. The dest and source GeometryStreams can be the same.
    //! @param dest     The output GeometryStream
    //! @param source   The input GeometryStream
    //! @param remapper  The ID remapper
    static DgnDbStatus Import(GeometryStreamR dest, GeometryStreamCR source, DgnImportContext& remapper);

}; // GeometryStreamIO

//=======================================================================================
//! GeometryCollection provides iterator for a Geometric Element's GeometryStream.
//! @ingroup GROUP_Geometry
//=======================================================================================
struct GeometryCollection
{
    //=======================================================================================
    //! Iterator
    //! @ingroup GROUP_Geometry
    //=======================================================================================
    struct Iterator : std::iterator<std::forward_iterator_tag, uint8_t const*>
    {
    private:

        enum class EntryType
        {
            Unknown             = 0,  //!< Invalid
            GeometryPart        = 1,  //!< DgnGeometryPart reference
            CurvePrimitive      = 2,  //!< A single open curve
            CurveVector         = 3,  //!< Open paths, planar regions, and unstructured curve collections
            SolidPrimitive      = 4,  //!< ISolidPrimitive
            BsplineSurface      = 5,  //!< MSBSplineSurface
            Polyface            = 6,  //!< Polyface
            SolidKernelEntity   = 7,  //!< SolidKernelEntity
            TextString          = 8,  //!< TextString
        };

        struct CurrentState
        {
            DgnDbR                  m_dgnDb;
            Render::GeometryParams  m_geomParams;
            Transform               m_sourceToWorld;
            Transform               m_geomToSource;
            Transform               m_geomToWorld;
            GeometricPrimitivePtr   m_geometry;
            GeometryStreamEntryId   m_geomStreamEntryId;

            CurrentState(DgnDbR dgnDb) : m_dgnDb(dgnDb)
            {
                m_sourceToWorld = Transform::FromIdentity();
                m_geomToSource  = Transform::FromIdentity();
                m_geomToWorld   = Transform::FromIdentity();
            }
        };

        uint8_t const*              m_data;
        size_t                      m_dataSize;
        size_t                      m_dataOffset;
        GeometryStreamIO::Operation m_egOp;
        mutable CurrentState*       m_state;

        DGNPLATFORM_EXPORT void ToNext();

        Iterator(uint8_t const* data, size_t dataSize, CurrentState& state) : m_data(data), m_dataSize(dataSize), m_dataOffset(0), m_state(&state) {ToNext();}
        Iterator() : m_data(nullptr), m_dataSize(0), m_dataOffset(0), m_state(nullptr) {}

        friend struct GeometryCollection;

    public:

        Iterator& operator++() {ToNext (); return *this;}
        bool operator==(Iterator const& rhs) const {return m_dataOffset == rhs.m_dataOffset;}
        bool operator!=(Iterator const& rhs) const {return !(*this == rhs);}
        Iterator const& operator*() const {return *this;}

        Render::GeometryParamsCR GetGeometryParams() const {return m_state->m_geomParams;}
        DgnGeometryPartId GetGeometryPartId() const {return m_state->m_geomStreamEntryId.GetGeometryPartId();} //!< Returns invalid id if not a GeometryPart... 
        GeometryStreamEntryId GetGeometryStreamEntryId() const {return m_state->m_geomStreamEntryId;} //!< Returns primitive id for current geometry...
        
        DGNPLATFORM_EXPORT EntryType GetEntryType() const;  //!< check geometry type to avoid creating GeometricPrimitivePtr for un-desired types.
        DGNPLATFORM_EXPORT bool IsCurve() const;            //!< open and unstructured curves check that avoids creating GeometricPrimitivePtr when possible.
        DGNPLATFORM_EXPORT bool IsSurface() const;          //!< closed curve, planar region, surface, and open mesh check that avoids creating GeometricPrimitivePtr when possible.
        DGNPLATFORM_EXPORT bool IsSolid() const;            //!< solid and volumetric mesh check that avoids creating GeometricPrimitivePtr when possible.

        DgnGeometryPartPtr GetGeometryPartPtr() const {return m_state->m_dgnDb.Elements().GetForEdit<DgnGeometryPart>(m_state->m_geomStreamEntryId.GetGeometryPartId());}
        DgnGeometryPartCPtr GetGeometryPartCPtr() const {return m_state->m_dgnDb.Elements().Get<DgnGeometryPart>(m_state->m_geomStreamEntryId.GetGeometryPartId());}
        DGNPLATFORM_EXPORT GeometricPrimitivePtr GetGeometryPtr() const;

        TransformCR GetSourceToWorld() const {return m_state->m_sourceToWorld;}
        TransformCR GetGeometryToSource() const {return m_state->m_geomToSource;}
        TransformCR GetGeometryToWorld() const {return m_state->m_geomToWorld = Transform::FromProduct(m_state->m_sourceToWorld, m_state->m_geomToSource);};
    };

private:

    uint8_t const*                  m_data;
    size_t                          m_dataSize;
    mutable Iterator::CurrentState  m_state;

public:

    typedef Iterator const_iterator;
    typedef const_iterator iterator; //!< only const iteration is possible
    
    const_iterator begin() const {return const_iterator(m_data, m_dataSize, m_state);}
    const_iterator end() const {return const_iterator();}

    //! Iterate a GeometryStream for a DgnGeometryPart using the current GeometryParams and geometry to world transform
    //! for of part instance as found when iterating a GeometrySource with a part reference.
    DGNPLATFORM_EXPORT void SetNestedIteratorContext(Iterator const& iter);
    
    //! Iterate a GeometryStream.
    //! @note It is up to the caller to keep the GeometryStream in memory by holding onto a DgnGeometryPartPtr, etc. until done iterating.
    DGNPLATFORM_EXPORT GeometryCollection (GeometryStreamCR geom, DgnDbR dgnDb);

    //! Iterate a GeometrySource.
    //! @note It is up to the caller to keep the GeometrySource in memory by holding onto a DgnElementPtr, etc. until done iterating.
    DGNPLATFORM_EXPORT GeometryCollection (GeometrySourceCR source);

}; // GeometryCollection

typedef RefCountedPtr<GeometryBuilder> GeometryBuilderPtr;

//=======================================================================================
//! GeometryBuilder provides methods for setting up an element's GeometryStream and Placement(2d/3d).
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
//! For repeated geometry that can be shared in a single GeometryStream or by multiple GeometryStreams, a DgnGeometryPart should be
//! created. When appending a DgnGeometryPartId you specify the part geometry to element transform in order to position the 
//! part's geometry relative to the other geometry/parts display by the element.
//! @ingroup GROUP_Geometry
//=======================================================================================
struct GeometryBuilder : RefCountedBase
{
private:

    bool                     m_appearanceChanged;
    bool                     m_haveLocalGeom;
    bool                     m_havePlacement;
    bool                     m_isPartCreate;
    bool                     m_is3d;
    bool                     m_appendAsSubGraphics;
    Placement3d              m_placement3d;
    Placement2d              m_placement2d;
    DgnDbR                   m_dgnDb;
    Render::GeometryParams   m_elParams;
    GeometryStreamIO::Writer m_writer;

    GeometryBuilder (DgnDbR dgnDb, DgnCategoryId categoryId, Placement3dCR placement);
    GeometryBuilder (DgnDbR dgnDb, DgnCategoryId categoryId, Placement2dCR placement);
    GeometryBuilder (DgnDbR dgnDb, DgnCategoryId categoryId, bool is3d);

    bool ConvertToLocal (GeometricPrimitiveR);
    bool AppendWorld (GeometricPrimitiveR);
    bool AppendLocal (GeometricPrimitiveCR);
    void OnNewGeom (DRange3dCR localRange, bool isSubGraphic);

public:

    DgnDbR GetDgnDb() const {return m_dgnDb;} //!< @private
    bool Is3d() const {return m_is3d;} //!< @private
    Placement2dCR GetPlacement2d() const {return m_placement2d;} //!< @private
    Placement3dCR GetPlacement3d() const {return m_placement3d;} //!< @private
    Render::GeometryParamsCR GetGeometryParams() const {return m_elParams;} //!< @private
    DGNPLATFORM_EXPORT BentleyStatus GetGeometryStream (GeometryStreamR); //!< @private
    DGNPLATFORM_EXPORT GeometryStreamEntryId GetGeometryStreamEntryId() const; //! Return the primitive id of the geometry last added to the builder.
    DGNPLATFORM_EXPORT bool MatchesGeometryPart (DgnGeometryPartId, DgnDbR db, bool ignoreSymbology = false, bool ignoreInitialSymbology = true); //!< @private assume change already checked...

    DGNPLATFORM_EXPORT BentleyStatus SetGeometryStream (DgnGeometryPartR);
    DGNPLATFORM_EXPORT BentleyStatus SetGeometryStreamAndPlacement (GeometrySourceR);
    void SetAppendAsSubGraphics() {m_appendAsSubGraphics = !m_isPartCreate;} //! Subsequent Append calls will produce sub-graphics with local ranges for picking. Not valid when creating a GeometryPart.

    DGNPLATFORM_EXPORT bool Append (DgnSubCategoryId);
    DGNPLATFORM_EXPORT bool Append (Render::GeometryParamsCR);
    DGNPLATFORM_EXPORT bool Append (DgnGeometryPartId, TransformCR geomToElement); //! Relative placement, not valid for CreateWorld.
    DGNPLATFORM_EXPORT bool Append (DgnGeometryPartId, DPoint3dCR origin, YawPitchRollAngles const& angles = YawPitchRollAngles()); //! Relative placement, not valid for CreateWorld.
    DGNPLATFORM_EXPORT bool Append (DgnGeometryPartId, DPoint2dCR origin, AngleInDegrees const& angle = AngleInDegrees()); //! Relative placement, not valid for CreateWorld.
    DGNPLATFORM_EXPORT bool Append (GeometricPrimitiveCR);
    DGNPLATFORM_EXPORT bool Append (ICurvePrimitiveCR);
    DGNPLATFORM_EXPORT bool Append (CurveVectorCR);
    DGNPLATFORM_EXPORT bool Append (ISolidPrimitiveCR); //! 3d only
    DGNPLATFORM_EXPORT bool Append (MSBsplineSurfaceCR); //! 3d only
    DGNPLATFORM_EXPORT bool Append (PolyfaceQueryCR); //! 3d only
    DGNPLATFORM_EXPORT bool Append (ISolidKernelEntityCR); //! 3d only
    DGNPLATFORM_EXPORT bool Append (IGeometryCR); //! 3d only
    DGNPLATFORM_EXPORT bool Append (TextStringCR);
    DGNPLATFORM_EXPORT bool Append (TextAnnotationCR);

    DGNPLATFORM_EXPORT static GeometryBuilderPtr CreateGeometryPart (DgnDbR db, bool is3d);
    DGNPLATFORM_EXPORT static GeometryBuilderPtr CreateGeometryPart (GeometryStreamCR, DgnDbR db, bool ignoreSymbology = false, Render::GeometryParamsP params = nullptr); //!< @private

    DGNPLATFORM_EXPORT static GeometryBuilderPtr Create (DgnModelR model, DgnCategoryId categoryId, DPoint3dCR origin, YawPitchRollAngles const& angles = YawPitchRollAngles());
    DGNPLATFORM_EXPORT static GeometryBuilderPtr Create (DgnModelR model, DgnCategoryId categoryId, DPoint2dCR origin, AngleInDegrees const& angle = AngleInDegrees());
    DGNPLATFORM_EXPORT static GeometryBuilderPtr CreateWorld (DgnModelR model, DgnCategoryId categoryId);

    DGNPLATFORM_EXPORT static GeometryBuilderPtr Create (GeometrySource3dCR, DPoint3dCR origin, YawPitchRollAngles const& angles = YawPitchRollAngles());
    DGNPLATFORM_EXPORT static GeometryBuilderPtr Create (GeometrySource2dCR, DPoint2dCR origin, AngleInDegrees const& angle = AngleInDegrees());
    DGNPLATFORM_EXPORT static GeometryBuilderPtr Create (GeometrySourceCR); //! Create builder from GeometrySource using current placement.
    DGNPLATFORM_EXPORT static GeometryBuilderPtr CreateWorld (GeometrySourceCR);

}; // GeometryBuilder

END_BENTLEY_DGN_NAMESPACE
