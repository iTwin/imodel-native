/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"

using namespace IModelJsNative;
using OpCode = GeometryStreamIO::OpCode;

BEGIN_UNNAMED_NAMESPACE


bmap<DgnGeometryPartId, DgnElementId> findSingleUsePartRefs(DgnDbR db)
    {
    // NOTE: Currently don't care about 2d elements.
    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(
        "SELECT ECInstanceId FROM bis.GeometricElement3d "
        "WHERE GeometryStream IS NOT NULL");

    bset<DgnGeometryPartId> multiRefParts;
    bmap<DgnGeometryPartId, DgnElementId> singleRefParts;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto elemId = stmt->GetValueId<DgnElementId>(0);
        auto elem = db.Elements().Get<GeometricElement3d>(elemId);
        if (elem.IsNull())
            continue;

        GeometryCollection geom(*elem);
        for (auto const& iter : geom)
            {
            auto partId = iter.GetGeometryPartId();
            if (!partId.IsValid() || multiRefParts.end() != multiRefParts.find(partId))
                continue;

            auto inserted = singleRefParts.Insert(partId, elemId);
            if (inserted.second)
                continue; // first reference to this part.

            // Second reference to this part.
            singleRefParts.erase(inserted.first);
            multiRefParts.insert(partId);
            }
        }

    return singleRefParts;
    }

bool inlineGeometryPart(GeometryBuilderR builder, DgnGeometryPartCR part, TransformCR transform, GeometryParamsCR elParams)
    {
    GeometryParams params = elParams;
    auto const& input = part.GetGeometryStream();
    if (!input.HasGeometry())
        return false;

    GeometryStreamIO::Reader reader(part.GetDgnDb());
    GeometryStreamIO::Collection collection(input.GetData(), input.GetSize());

    bool paramsNeedTransform = false;
    for (auto const& egOp : collection)
        {
        switch (egOp.m_opCode)
            {
            case OpCode::SubGraphicRange:
            case OpCode::GeometryPartInstance:
                BeAssert(false); // and fall through...
            case OpCode::Header:
                break;
            case OpCode::LineStyleModifiers:
            case OpCode::Pattern:
                paramsNeedTransform = true;
                // and fall through...
            case OpCode::BasicSymbology:
            case OpCode::AreaFill:
            case OpCode::Material:
                if (!reader.Get(egOp, params))
                    return false;

                // We will append the accumulated params before next geometric primitive.
                break;
            default:
                {
                GeometricPrimitivePtr primitive;
                if (!reader.Get(egOp, primitive) || !primitive->TransformInPlace(transform))
                    return false;

                if (paramsNeedTransform)
                    {
                    params.ApplyTransform(transform);
                    paramsNeedTransform = false;
                    }

                builder.Append(params);
                builder.Append(*primitive);
                break;
                }
            }
        }

    return true;
    }

bool inlineGeometryPartReference(DgnGeometryPartCR part, GeometricElement3dCR elem)
    {
    auto const& input = elem.ToGeometrySource()->GetGeometryStream();
    BeAssert(input.HasGeometry());

    // The part may contain symbology changes. Keep track of the element's symbology so we can restore it after embedding the part.
    Render::GeometryParams params;
    params.SetCategoryId(elem.GetCategoryId());

    auto builder = GeometryBuilder::Create(elem, GeometryStream());
    if (builder.IsNull())
        return false;

    bool inlined = false;
    GeometryStreamIO::Reader reader(part.GetDgnDb());
    GeometryStreamIO::Collection collection(input.GetData(), input.GetSize());
    for (auto const& egOp : collection)
        {
        switch (egOp.m_opCode)
            {
            case OpCode::Header:
                {
                auto pHeader = GeometryStreamIO::Reader::GetHeader(egOp);
                if (nullptr == pHeader || !builder->SetHeaderFlags(pHeader->m_flags))
                    return false;

                break;
                }
            case OpCode::BasicSymbology:
            case OpCode::LineStyleModifiers:
            case OpCode::AreaFill:
            case OpCode::Pattern:
            case OpCode::Material:
                if (!reader.Get(egOp, params))
                    return false;

                // We will append the accumulated params before the next part reference or geometric primitive.
                break;
            case OpCode::GeometryPartInstance:
                {
                builder->Append(params);

                DgnGeometryPartId partId;
                Transform transform;
                if (reader.Get(egOp, partId, transform) && partId == part.GetId())
                    {
                    builder->SetAppendAsSubGraphics();
                    BeAssert(!inlined);
                    inlined = inlineGeometryPart(*builder, part, transform, params);
                    if (!inlined)
                        return false;

                    builder->Append(params); // Reset symbology to what it was prior to part.
                    }
                else
                    {
                    // Not the part we're looking for.
                    builder->Append(partId, transform);
                    }

                break;
                }
            case OpCode::SubGraphicRange:
                builder->SetAppendAsSubGraphics();
                break;
            default:
                {
                GeometricPrimitivePtr primitive;
                if (!reader.Get(egOp, primitive))
                    return false;

                builder->Append(params);
                builder->Append(*primitive);
                break;
                }
            }
        }

    if (!inlined)
        return false;

    auto updatedElem = elem.CopyForEdit();
    return SUCCESS == builder->Finish(*updatedElem->ToGeometrySourceP()) && DgnDbStatus::Success == updatedElem->Update();
    }

END_UNNAMED_NAMESPACE

InlineGeometryPartsResult JsInterop::InlineGeometryParts(DgnDbR db)
    {
    auto partRefs = findSingleUsePartRefs(db);
    InlineGeometryPartsResult result { static_cast<uint32_t>(partRefs.size()) };

    DgnDb::PurgeOperation purgeOp(db);
    for (auto const& pair : partRefs)
        {
        auto part = db.Elements().Get<DgnGeometryPart>(pair.first);
        if (part.IsNull())
            continue;

        auto elem = db.Elements().Get<GeometricElement3d>(pair.second);
        if (elem.IsNull())
            continue;

        if (!inlineGeometryPartReference(*part, *elem))
            continue;

        ++result.m_numRefsInlined;
        if (DgnDbStatus::Success == part->Delete())
            ++result.m_numPartsDeleted;
        }

    return result;
    }

