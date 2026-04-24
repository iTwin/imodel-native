/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include "GeomStreamVTab.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

// Geometry blob header — matches the format written by GeometryStream::WriteGeometryStream
static constexpr uint32_t GEOM_BLOB_SIGNATURE = 0x0600;

//=======================================================================================
// GeomStreamModule
//=======================================================================================
GeomStreamModule::GeomStreamModule(ECDbR ecdb)
    : ECDbModule(ecdb, NAME, DECLARATION, ECSCHEMA_XML)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult GeomStreamModule::Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv)
    {
    out = new GeomStreamVirtualTable(*this);
    return BE_SQLITE_OK;
    }

//=======================================================================================
// GeomStreamVirtualTable — BestIndex
//=======================================================================================
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult GeomStreamModule::GeomStreamVirtualTable::BestIndex(IndexInfo& indexInfo)
    {
    for (int i = 0; i < indexInfo.GetConstraintCount(); ++i)
        {
        auto constraint = indexInfo.GetConstraint(i);
        if (constraint->GetColumn() == (int)GeomStreamCursor::Columns::GeomStreamBlob
            && constraint->GetOp() == IndexInfo::Operator::EQ
            && constraint->IsUsable())
            {
            auto usage = indexInfo.GetConstraintUsage(i);
            usage->SetArgvIndex(1);
            usage->SetOmit(true);
            indexInfo.SetEstimatedCost(100);
            indexInfo.SetEstimatedRows(10);
            indexInfo.SetIdxNum(1);
            return BE_SQLITE_OK;
            }
        }

    // No blob input — return prohibitive cost
    indexInfo.SetEstimatedCost(1e12);
    indexInfo.SetEstimatedRows(0);
    indexInfo.SetIdxNum(0);
    return BE_SQLITE_OK;
    }

//=======================================================================================
// GeomStreamCursor — helpers
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbP GeomStreamModule::GeomStreamVirtualTable::GeomStreamCursor::GetDgnDb()
    {
    auto& ecdb = static_cast<GeomStreamModule&>(GetTable().GetModule()).GetECDb();
    return dynamic_cast<DgnDbP>(&ecdb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeomStreamModule::GeomStreamVirtualTable::GeomStreamCursor::Reset()
    {
    m_decompressed.clear();
    m_data = nullptr;
    m_dataSize = 0;
    m_dataOffset = 0;
    m_entryIndex = -1;
    m_eof = true;
    m_currentOp = GeometryStreamIO::Operation();
    m_geomParams = Render::GeometryParams();
    m_symbologyValid = false;
    m_subGraphicRange.Init();
    m_hasSubGraphicRange = false;
    m_geomPartId = DgnGeometryPartId();
    m_partOrigin = DPoint3d::FromZero();
    m_partYaw = 0;
    m_partPitch = 0;
    m_partRoll = 0;
    m_partScale = 1.0;
    m_hasPartData = false;
    m_headerFlags = 0;
    m_isHeaderEntry = false;
    m_textContent.clear();
    m_hasTextContent = false;
    }

/*---------------------------------------------------------------------------------**//**
* OpCode enum → string name
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP GeomStreamModule::GeomStreamVirtualTable::GeomStreamCursor::OpCodeToString(GeometryStreamIO::OpCode opCode)
    {
    switch (opCode)
        {
        case GeometryStreamIO::OpCode::Header:               return "Header";
        case GeometryStreamIO::OpCode::SubGraphicRange:      return "SubGraphicRange";
        case GeometryStreamIO::OpCode::GeometryPartInstance: return "GeometryPartInstance";
        case GeometryStreamIO::OpCode::BasicSymbology:       return "BasicSymbology";
        case GeometryStreamIO::OpCode::PointPrimitive:       return "PointPrimitive";
        case GeometryStreamIO::OpCode::PointPrimitive2d:     return "PointPrimitive2d";
        case GeometryStreamIO::OpCode::ArcPrimitive:         return "ArcPrimitive";
        case GeometryStreamIO::OpCode::CurveVector:          return "CurveVector";
        case GeometryStreamIO::OpCode::Polyface:             return "Polyface";
        case GeometryStreamIO::OpCode::CurvePrimitive:       return "CurvePrimitive";
        case GeometryStreamIO::OpCode::SolidPrimitive:       return "SolidPrimitive";
        case GeometryStreamIO::OpCode::BsplineSurface:       return "BsplineSurface";
        case GeometryStreamIO::OpCode::AreaFill:             return "AreaFill";
        case GeometryStreamIO::OpCode::Pattern:              return "Pattern";
        case GeometryStreamIO::OpCode::Material:             return "Material";
        case GeometryStreamIO::OpCode::TextString:           return "TextString";
        case GeometryStreamIO::OpCode::LineStyleModifiers:   return "LineStyleModifiers";
        case GeometryStreamIO::OpCode::ParasolidBRep:        return "ParasolidBRep";
        case GeometryStreamIO::OpCode::Image:                return "Image";
        default:                                             return "Unknown";
        }
    }

/*---------------------------------------------------------------------------------**//**
* OpCode → user-facing entry type name
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP GeomStreamModule::GeomStreamVirtualTable::GeomStreamCursor::OpCodeToEntryType(GeometryStreamIO::OpCode opCode)
    {
    switch (opCode)
        {
        case GeometryStreamIO::OpCode::Header:               return "Header";
        case GeometryStreamIO::OpCode::SubGraphicRange:      return "SubGraphicRange";
        case GeometryStreamIO::OpCode::GeometryPartInstance: return "GeometryPart";
        case GeometryStreamIO::OpCode::BasicSymbology:       return "BasicSymbology";
        case GeometryStreamIO::OpCode::PointPrimitive:       return "PointPrimitive";
        case GeometryStreamIO::OpCode::PointPrimitive2d:     return "PointPrimitive";
        case GeometryStreamIO::OpCode::ArcPrimitive:         return "ArcPrimitive";
        case GeometryStreamIO::OpCode::CurveVector:          return "CurveVector";
        case GeometryStreamIO::OpCode::Polyface:             return "Polyface";
        case GeometryStreamIO::OpCode::CurvePrimitive:       return "CurvePrimitive";
        case GeometryStreamIO::OpCode::SolidPrimitive:       return "SolidPrimitive";
        case GeometryStreamIO::OpCode::BsplineSurface:       return "BsplineSurface";
        case GeometryStreamIO::OpCode::AreaFill:             return "AreaFill";
        case GeometryStreamIO::OpCode::Pattern:              return "Pattern";
        case GeometryStreamIO::OpCode::Material:             return "Material";
        case GeometryStreamIO::OpCode::TextString:           return "TextString";
        case GeometryStreamIO::OpCode::LineStyleModifiers:   return "LineStyleModifiers";
        case GeometryStreamIO::OpCode::ParasolidBRep:        return "BRepEntity";
        case GeometryStreamIO::OpCode::Image:                return "Image";
        default:                                             return "Unknown";
        }
    }

/*---------------------------------------------------------------------------------**//**
* Process the current opcode — accumulate symbology, extract part/range/text data.
* Called after each successful ReadNextEntry(). Extracts per-entry metadata from the
* opcode payload using GeometryStreamIO::Reader when a DgnDb is available.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeomStreamModule::GeomStreamVirtualTable::GeomStreamCursor::ProcessCurrentOp()
    {
    // Reset per-entry state
    m_hasPartData = false;
    m_isHeaderEntry = false;
    m_hasTextContent = false;
    m_hasSubGraphicRange = false;

    DgnDbP db = GetDgnDb();

    switch (m_currentOp.m_opCode)
        {
        case GeometryStreamIO::OpCode::Header:
            {
            auto header = GeometryStreamIO::Reader::GetHeader(m_currentOp);
            if (nullptr != header)
                m_headerFlags = (uint32_t)header->m_flags;
            m_isHeaderEntry = true;
            break;
            }

        case GeometryStreamIO::OpCode::BasicSymbology:
        case GeometryStreamIO::OpCode::AreaFill:
        case GeometryStreamIO::OpCode::Pattern:
        case GeometryStreamIO::OpCode::Material:
        case GeometryStreamIO::OpCode::LineStyleModifiers:
            {
            if (nullptr != db)
                {
                GeometryStreamIO::Reader reader(*db);
                reader.Get(m_currentOp, m_geomParams);
                m_symbologyValid = true;
                }
            break;
            }

        case GeometryStreamIO::OpCode::SubGraphicRange:
            {
            if (nullptr != db)
                {
                GeometryStreamIO::Reader reader(*db);
                m_hasSubGraphicRange = reader.Get(m_currentOp, m_subGraphicRange);
                }
            break;
            }

        case GeometryStreamIO::OpCode::GeometryPartInstance:
            {
            if (nullptr != db)
                {
                GeometryStreamIO::Reader reader(*db);
                Transform geomToElem;
                if (reader.Get(m_currentOp, m_geomPartId, geomToElem))
                    {
                    m_hasPartData = true;
                    // Extract origin from transform
                    geomToElem.GetTranslation(m_partOrigin);
                    // Extract YPR from transform matrix
                    RotMatrix matrix;
                    geomToElem.GetMatrix(matrix);
                    YawPitchRollAngles angles;
                    if (YawPitchRollAngles::TryFromRotMatrix(angles, matrix))
                        {
                        m_partYaw = angles.GetYaw().Degrees();
                        m_partPitch = angles.GetPitch().Degrees();
                        m_partRoll = angles.GetRoll().Degrees();
                        }
                    else
                        {
                        m_partYaw = m_partPitch = m_partRoll = 0.0;
                        }
                    // Extract scale from matrix columns
                    DVec3d col0, col1, col2;
                    matrix.GetColumns(col0, col1, col2);
                    m_partScale = col0.Magnitude();
                    }
                }
            break;
            }

        case GeometryStreamIO::OpCode::TextString:
            {
            if (nullptr != db)
                {
                TextStringPtr text = TextString::Create(*db);
                GeometryStreamIO::Reader reader(*db);
                if (reader.Get(m_currentOp, *text))
                    {
                    m_textContent = text->GetText();
                    m_hasTextContent = true;
                    }
                }
            break;
            }

        default:
            break;
        }
    }

//=======================================================================================
// GeomStreamCursor — ReadNextEntry (with bounds checking)
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* Read the next opcode from the decompressed stream. Returns BE_SQLITE_OK on success
* (sets m_eof=true when stream is exhausted). Performs strict bounds checking to
* prevent buffer overreads on corrupt/truncated data.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult GeomStreamModule::GeomStreamVirtualTable::GeomStreamCursor::ReadNextEntry()
    {
    // Check if we've consumed all data
    if (m_dataOffset >= m_dataSize)
        {
        m_eof = true;
        return BE_SQLITE_OK;
        }

    // Need at least 8 bytes for opcode header (4-byte opcode + 4-byte size)
    if (m_dataOffset + 8 > m_dataSize)
        {
        m_eof = true;
        return BE_SQLITE_OK;
        }

    uint8_t const* ptr = m_data + m_dataOffset;
    uint32_t opCode  = *reinterpret_cast<uint32_t const*>(ptr);
    uint32_t dataSize = *reinterpret_cast<uint32_t const*>(ptr + sizeof(uint32_t));

    // Validate payload fits within remaining buffer
    size_t totalOpSize = sizeof(uint32_t) + sizeof(uint32_t) + dataSize;
    if (m_dataOffset + totalOpSize > m_dataSize)
        {
        m_eof = true;
        return BE_SQLITE_OK;
        }

    uint8_t const* data = (0 != dataSize) ? (ptr + sizeof(uint32_t) + sizeof(uint32_t)) : nullptr;
    m_currentOp = GeometryStreamIO::Operation(static_cast<GeometryStreamIO::OpCode>(opCode), dataSize, data);
    m_dataOffset += totalOpSize;
    m_entryIndex++;

    ProcessCurrentOp();
    m_eof = false;
    return BE_SQLITE_OK;
    }

//=======================================================================================
// GeomStreamCursor — Filter
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult GeomStreamModule::GeomStreamVirtualTable::GeomStreamCursor::Filter(int idxNum, const char* idxStr, int argc, DbValue* argv)
    {
    Reset();
    try
        {
        if (argc < 1 || argv[0].IsNull())
            {
            m_eof = true;
            return BE_SQLITE_OK;
            }

        int blobSize = argv[0].GetValueBytes();
        void const* blob = argv[0].GetValueBlob();
        if (blobSize <= 0 || nullptr == blob)
            {
            m_eof = true;
            return BE_SQLITE_OK;
            }

        // Decompress the blob: GeomBlobHeader (8 bytes) + Snappy-compressed opcode data
        SnappyFromMemory& snappy = SnappyFromMemory::GetForThread();
        snappy.Init(const_cast<void*>(blob), static_cast<uint32_t>(blobSize));

        // Read the GeomBlobHeader (signature + uncompressed size)
        struct { uint32_t m_signature; uint32_t m_size; } header;
        uint32_t actuallyRead;
        if (ZIP_SUCCESS != snappy._Read(reinterpret_cast<Byte*>(&header), sizeof(header), actuallyRead)
            || actuallyRead != sizeof(header))
            {
            m_eof = true;
            return BE_SQLITE_OK;
            }

        if (GEOM_BLOB_SIGNATURE != header.m_signature || 0 == header.m_size)
            {
            m_eof = true;
            return BE_SQLITE_OK;
            }

        // Enforce the per-DgnDb size limit before decompressing
        DgnDbP dgndb = GetDgnDb();
        if (nullptr != dgndb && header.m_size > dgndb->GetMaxGeomStreamVTabBytes())
            {
            m_eof = true;
            return BE_SQLITE_TOOBIG;
            }

        // Decompress the opcode data
        m_decompressed.resize(header.m_size);
        if (ZIP_SUCCESS != snappy._Read(m_decompressed.data(), header.m_size, actuallyRead)
            || actuallyRead != header.m_size)
            {
            m_decompressed.clear();
            m_eof = true;
            return BE_SQLITE_OK;
            }

        m_data = m_decompressed.data();
        m_dataSize = header.m_size;
        m_dataOffset = 0;
        m_entryIndex = -1;

        // Read the first entry
        return ReadNextEntry();
        }
    catch (...)
        {
        Reset();
        m_eof = true;
        return BE_SQLITE_OK;
        }
    }

//=======================================================================================
// GeomStreamCursor — Next
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult GeomStreamModule::GeomStreamVirtualTable::GeomStreamCursor::Next()
    {
    try
        {
        return ReadNextEntry();
        }
    catch (...)
        {
        m_eof = true;
        return BE_SQLITE_OK;
        }
    }

//=======================================================================================
// GeomStreamCursor — GetRowId
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult GeomStreamModule::GeomStreamVirtualTable::GeomStreamCursor::GetRowId(int64_t& rowId)
    {
    rowId = m_entryIndex;
    return BE_SQLITE_OK;
    }

//=======================================================================================
// GeomStreamCursor — GetColumn
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult GeomStreamModule::GeomStreamVirtualTable::GeomStreamCursor::GetColumn(int i, Context& ctx)
    {
    try
        {
        switch (static_cast<Columns>(i))
            {
            case Columns::GeomStreamBlob:
                ctx.SetResultNull();
                break;

            case Columns::EntryIndex:
                ctx.SetResultInt(m_entryIndex);
                break;

            case Columns::OpCode:
                {
                Utf8CP name = OpCodeToString(m_currentOp.m_opCode);
                ctx.SetResultText(name, (int)strlen(name), Context::CopyData::No);
                break;
                }

            case Columns::EntryType:
                {
                Utf8CP name = OpCodeToEntryType(m_currentOp.m_opCode);
                ctx.SetResultText(name, (int)strlen(name), Context::CopyData::No);
                break;
                }

            case Columns::IsGeometry:
                ctx.SetResultInt(m_currentOp.IsGeometryOp() ? 1 : 0);
                break;

            // Symbology columns — return accumulated state
            case Columns::SubCategoryId:
                if (m_symbologyValid)
                    ctx.SetResultInt64(m_geomParams.GetSubCategoryId().GetValueUnchecked());
                else
                    ctx.SetResultNull();
                break;

            case Columns::Color:
                if (m_symbologyValid && !m_geomParams.IsLineColorFromSubCategoryAppearance())
                    ctx.SetResultInt(static_cast<int>(m_geomParams.GetLineColor().GetValue()));
                else
                    ctx.SetResultNull();
                break;

            case Columns::Weight:
                if (m_symbologyValid && !m_geomParams.IsWeightFromSubCategoryAppearance())
                    ctx.SetResultInt(static_cast<int>(m_geomParams.GetWeight()));
                else
                    ctx.SetResultNull();
                break;

            case Columns::LineStyle:
                if (m_symbologyValid && !m_geomParams.IsLineStyleFromSubCategoryAppearance() && m_geomParams.GetLineStyle() != nullptr)
                    ctx.SetResultInt64(m_geomParams.GetLineStyle()->GetStyleId().GetValueUnchecked());
                else
                    ctx.SetResultNull();
                break;

            case Columns::Transparency:
                if (m_symbologyValid)
                    ctx.SetResultDouble(m_geomParams.GetTransparency());
                else
                    ctx.SetResultNull();
                break;

            case Columns::GeomClass:
                if (m_symbologyValid)
                    ctx.SetResultInt(static_cast<int>(m_geomParams.GetGeometryClass()));
                else
                    ctx.SetResultNull();
                break;

            case Columns::DisplayPriority:
                if (m_symbologyValid)
                    ctx.SetResultInt(m_geomParams.GetDisplayPriority());
                else
                    ctx.SetResultNull();
                break;

            case Columns::MaterialId:
                if (m_symbologyValid && !m_geomParams.IsMaterialFromSubCategoryAppearance())
                    {
                    auto matId = m_geomParams.GetMaterialId();
                    if (matId.IsValid())
                        ctx.SetResultInt64(matId.GetValueUnchecked());
                    else
                        ctx.SetResultNull();
                    }
                else
                    ctx.SetResultNull();
                break;

            // GeometryPart columns
            case Columns::GeometryPartId:
                if (m_hasPartData)
                    ctx.SetResultInt64(m_geomPartId.GetValueUnchecked());
                else
                    ctx.SetResultNull();
                break;

            case Columns::PartOriginX:
                if (m_hasPartData) ctx.SetResultDouble(m_partOrigin.x);
                else ctx.SetResultNull();
                break;

            case Columns::PartOriginY:
                if (m_hasPartData) ctx.SetResultDouble(m_partOrigin.y);
                else ctx.SetResultNull();
                break;

            case Columns::PartOriginZ:
                if (m_hasPartData) ctx.SetResultDouble(m_partOrigin.z);
                else ctx.SetResultNull();
                break;

            case Columns::PartYaw:
                if (m_hasPartData) ctx.SetResultDouble(m_partYaw);
                else ctx.SetResultNull();
                break;

            case Columns::PartPitch:
                if (m_hasPartData) ctx.SetResultDouble(m_partPitch);
                else ctx.SetResultNull();
                break;

            case Columns::PartRoll:
                if (m_hasPartData) ctx.SetResultDouble(m_partRoll);
                else ctx.SetResultNull();
                break;

            case Columns::PartScale:
                if (m_hasPartData) ctx.SetResultDouble(m_partScale);
                else ctx.SetResultNull();
                break;

            // SubGraphicRange columns
            case Columns::RangeLowX:
                if (m_hasSubGraphicRange) ctx.SetResultDouble(m_subGraphicRange.low.x);
                else ctx.SetResultNull();
                break;

            case Columns::RangeLowY:
                if (m_hasSubGraphicRange) ctx.SetResultDouble(m_subGraphicRange.low.y);
                else ctx.SetResultNull();
                break;

            case Columns::RangeLowZ:
                if (m_hasSubGraphicRange) ctx.SetResultDouble(m_subGraphicRange.low.z);
                else ctx.SetResultNull();
                break;

            case Columns::RangeHighX:
                if (m_hasSubGraphicRange) ctx.SetResultDouble(m_subGraphicRange.high.x);
                else ctx.SetResultNull();
                break;

            case Columns::RangeHighY:
                if (m_hasSubGraphicRange) ctx.SetResultDouble(m_subGraphicRange.high.y);
                else ctx.SetResultNull();
                break;

            case Columns::RangeHighZ:
                if (m_hasSubGraphicRange) ctx.SetResultDouble(m_subGraphicRange.high.z);
                else ctx.SetResultNull();
                break;

            // Header flags
            case Columns::HeaderFlags:
                if (m_isHeaderEntry) ctx.SetResultInt(static_cast<int>(m_headerFlags));
                else ctx.SetResultNull();
                break;

            // TextString content
            case Columns::TextContent:
                if (m_hasTextContent)
                    ctx.SetResultText(m_textContent.c_str(), (int)m_textContent.size(), Context::CopyData::Yes);
                else
                    ctx.SetResultNull();
                break;

            // Raw geometry payload — return data for any entry that has a non-null payload
            case Columns::GeometryBlob:
                if (nullptr != m_currentOp.m_data && m_currentOp.m_dataSize > 0)
                    ctx.SetResultBlob(m_currentOp.m_data, m_currentOp.m_dataSize, Context::CopyData::No);
                else
                    ctx.SetResultNull();
                break;

            default:
                ctx.SetResultNull();
                break;
            }
        }
    catch (...)
        {
        ctx.SetResultNull();
        }

    return BE_SQLITE_OK;
    }
