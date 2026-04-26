/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDbVirtualTab.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/DgnDb.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Virtual table module that decomposes a GeometryStream blob into one row per entry.
//! Registered from iModelPlatform. Usage:
//!   SELECT gs.* FROM BisCore.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs
// @bsiclass
//=======================================================================================
struct GeomStreamModule : BeSQLite::EC::ECDbModule
    {
    constexpr static Utf8CP NAME = "imodel_geom_stream";

    //=================================================================================
    struct GeomStreamVirtualTable : ECDbVirtualTable
        {
        //=============================================================================
        struct GeomStreamCursor : ECDbCursor
            {
            enum class Columns : int
                {
                GeomStreamBlob = 0,     // hidden input
                EntryIndex,
                OpCode,
                EntryType,
                IsGeometry,
                SubCategoryId,
                Color,
                Weight,
                LineStyle,
                Transparency,
                GeomClass,
                DisplayPriority,
                MaterialId,
                GeometryPartId,
                PartOriginX,
                PartOriginY,
                PartOriginZ,
                PartYaw,
                PartPitch,
                PartRoll,
                PartScale,
                RangeLowX,
                RangeLowY,
                RangeLowZ,
                RangeHighX,
                RangeHighY,
                RangeHighZ,
                HeaderFlags,
                TextContent,
                GeometryBlob,
                COUNT
                };

            private:
                // Decompressed geometry stream bytes
                bvector<uint8_t> m_decompressed;

                // Current entry state
                uint8_t const*  m_data;
                size_t          m_dataSize;
                size_t          m_dataOffset;
                int             m_entryIndex;
                bool            m_eof;

                // Current opcode
                GeometryStreamIO::Operation m_currentOp;

                // Accumulated symbology
                Render::GeometryParams m_geomParams;
                bool m_symbologyValid;

                // SubGraphicRange from last SubGraphicRange opcode
                DRange3d m_subGraphicRange;
                bool     m_hasSubGraphicRange;

                // GeometryPart instance data
                DgnGeometryPartId m_geomPartId;
                DPoint3d    m_partOrigin;
                double      m_partYaw;
                double      m_partPitch;
                double      m_partRoll;
                double      m_partScale;
                bool        m_hasPartData;

                // Header flags
                uint32_t m_headerFlags;
                bool     m_isHeaderEntry;

                // TextString content
                Utf8String m_textContent;
                bool       m_hasTextContent;

                // GeometryBlob: [4-byte opcode][flatbuffer payload]
                bvector<uint8_t> m_geometryBlob;

                DgnDbP GetDgnDb();
                void Reset();
                DbResult ReadNextEntry();
                void ProcessCurrentOp();
                static Utf8CP OpCodeToString(GeometryStreamIO::OpCode);
                static Utf8CP OpCodeToEntryType(GeometryStreamIO::OpCode);

            public:
                GeomStreamCursor(GeomStreamVirtualTable& vt)
                    : ECDbCursor(vt), m_data(nullptr), m_dataSize(0), m_dataOffset(0),
                      m_entryIndex(-1), m_eof(true), m_symbologyValid(false),
                      m_hasSubGraphicRange(false), m_hasPartData(false), m_headerFlags(0),
                      m_isHeaderEntry(false), m_hasTextContent(false),
                      m_partYaw(0), m_partPitch(0), m_partRoll(0), m_partScale(1.0)
                    {
                    m_subGraphicRange.Init();
                    m_partOrigin = DPoint3d::FromZero();
                    }

                bool Eof() final { return m_eof; }
                DbResult Next() final;
                DbResult GetColumn(int i, Context& ctx) final;
                DbResult GetRowId(int64_t& rowId) final;
                DbResult Filter(int idxNum, const char* idxStr, int argc, DbValue* argv) final;
            };

        public:
            GeomStreamVirtualTable(GeomStreamModule& module) : ECDbVirtualTable(module) {}
            DbResult Open(DbCursor*& cur) override { cur = new GeomStreamCursor(*this); return BE_SQLITE_OK; }
            DbResult BestIndex(IndexInfo& indexInfo) final;
        };

    public:
        GeomStreamModule(BeSQLite::EC::ECDbR ecdb);
        DbResult Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv) final;

        //! Process-wide maximum uncompressed GeometryStream size (bytes) the vtab will decompose.
        //! Blobs exceeding this are silently skipped (zero rows). Default: 50 MB. Minimum enforced: 4 KB.
        static size_t GetMaxGeomStreamVTabBytes() { return s_maxGeomStreamVTabBytes; }
        static void SetMaxGeomStreamVTabBytes(size_t bytes) { s_maxGeomStreamVTabBytes = std::max(bytes, static_cast<size_t>(4096)); }

    private:
        static size_t s_maxGeomStreamVTabBytes;

        static constexpr Utf8CP ECSCHEMA_XML =
            R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema
                    schemaName="IModelVLib"
                    alias="IModelVLib"
                    version="1.0.0"
                    xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbVirtual" version="01.00.00" alias="ecdbvir" />
                <ECCustomAttributes>
                    <VirtualSchema xmlns="ECDbVirtual.01.00.00"/>
                </ECCustomAttributes>
                <ECEntityClass typeName="imodel_geom_stream" modifier="Abstract">
                    <ECCustomAttributes>
                        <VirtualType xmlns="ECDbVirtual.01.00.00"/>
                    </ECCustomAttributes>
                    <ECProperty propertyName="EntryIndex"      typeName="int" />
                    <ECProperty propertyName="OpCode"           typeName="string" />
                    <ECProperty propertyName="EntryType"        typeName="string" />
                    <ECProperty propertyName="IsGeometry"       typeName="int" />
                    <ECProperty propertyName="SubCategoryId"    typeName="long" />
                    <ECProperty propertyName="Color"            typeName="int" />
                    <ECProperty propertyName="Weight"           typeName="int" />
                    <ECProperty propertyName="LineStyle"        typeName="long" />
                    <ECProperty propertyName="Transparency"     typeName="double" />
                    <ECProperty propertyName="GeomClass"        typeName="int" />
                    <ECProperty propertyName="DisplayPriority"  typeName="int" />
                    <ECProperty propertyName="MaterialId"       typeName="long" />
                    <ECProperty propertyName="GeometryPartId"   typeName="long" />
                    <ECProperty propertyName="PartOriginX"      typeName="double" />
                    <ECProperty propertyName="PartOriginY"      typeName="double" />
                    <ECProperty propertyName="PartOriginZ"      typeName="double" />
                    <ECProperty propertyName="PartYaw"          typeName="double" />
                    <ECProperty propertyName="PartPitch"        typeName="double" />
                    <ECProperty propertyName="PartRoll"         typeName="double" />
                    <ECProperty propertyName="PartScale"        typeName="double" />
                    <ECProperty propertyName="RangeLowX"        typeName="double" />
                    <ECProperty propertyName="RangeLowY"        typeName="double" />
                    <ECProperty propertyName="RangeLowZ"        typeName="double" />
                    <ECProperty propertyName="RangeHighX"       typeName="double" />
                    <ECProperty propertyName="RangeHighY"       typeName="double" />
                    <ECProperty propertyName="RangeHighZ"       typeName="double" />
                    <ECProperty propertyName="HeaderFlags"      typeName="int" />
                    <ECProperty propertyName="TextContent"      typeName="string" />
                    <ECProperty propertyName="GeometryBlob"     typeName="binary" />
                </ECEntityClass>
            </ECSchema>)xml";

        static constexpr Utf8CP DECLARATION =
            "CREATE TABLE x("
            "geom_stream BLOB HIDDEN,"
            "EntryIndex, OpCode, EntryType, IsGeometry,"
            "SubCategoryId, Color, Weight, LineStyle, Transparency, GeomClass, DisplayPriority, MaterialId,"
            "GeometryPartId, PartOriginX, PartOriginY, PartOriginZ, PartYaw, PartPitch, PartRoll, PartScale,"
            "RangeLowX, RangeLowY, RangeLowZ, RangeHighX, RangeHighY, RangeHighZ,"
            "HeaderFlags, TextContent,"
            "GeometryBlob"
            ")";
    };

END_BENTLEY_DGN_NAMESPACE
