/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnCategory::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    DgnDbStatus status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        m_rank = static_cast<Rank>(stmt.GetValueInt(params.GetSelectIndex(prop_Rank())));
        m_descr = stmt.GetValueText(params.GetSelectIndex(prop_Description()));
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCategory::_ToJson(BeJsValue val, BeJsConst opts) const
    {
    T_Super::_ToJson(val, opts);
    val[json_rank()] = (int) m_rank;
    val[json_description()] = m_descr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCategory::_FromJson(BeJsConst props)
    {
    T_Super::_FromJson(props);

    if (props.hasMember(json_description())) // support partial update, only update m_descr if member present
        {
        auto description = props[json_description()];
        if (description.isString())
            m_descr = description.asString();
        else
            m_descr.clear(); // allow undefined to clear an existing value
        }

    if (props.isMember(json_rank()))
        m_rank = static_cast<Rank>(props[json_rank()].asInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCategory::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);
    stmt.BindText(stmt.GetParameterIndex(prop_Description()), m_descr.c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt(stmt.GetParameterIndex(prop_Rank()), static_cast<int32_t>(m_rank));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCategory::_CopyFrom(DgnElementCR el, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(el, opts);
    auto other = dynamic_cast<DgnCategoryCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr != other)
        {
        m_rank = other->m_rank;
        m_descr = other->m_descr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnCategory::QueryCategoryId(DgnDbR db, DgnCodeCR code)
    {
    return DgnCategoryId(db.Elements().QueryElementIdByCode(code).GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnCategory::_OnDelete() const
    {
    // can only be deleted through a purge operation
    return GetDgnDb().IsPurgeOperationActive() ? T_Super::_OnDelete() : DgnDbStatus::DeletionProhibited;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCategory::_OnInserted(DgnElementP copiedFrom) const
    {
    T_Super::_OnInserted(copiedFrom);

    // Create the default sub-category.
    DgnSubCategoryId defaultSubCatId = GetDefaultSubCategoryId();
    UNUSED_VARIABLE(defaultSubCatId);
    BeAssert(defaultSubCatId.IsValid());
    BeAssert(!GetDgnDb().Elements().GetElement(defaultSubCatId).IsValid());

    DgnSubCategory defaultSubCat(DgnSubCategory::CreateParams(GetDgnDb(), GetCategoryId(), GetCategoryName(), DgnSubCategory::Appearance()));
    // can't access protected member m_flags without copiedFrom being the same type as the `this` type of this method,
    // so we need to cast. Since we only need a base member, a non dynamic_cast is possible but avoiding it for now
    // this branch will therefore be ignored if the copiedFrom element's class were to not match
    DgnCategoryP copiedFromCategory = dynamic_cast<DgnCategoryP>(copiedFrom);
    if (copiedFromCategory != nullptr && copiedFromCategory->m_flags.m_preassignedId) {
        // as of now, we avoid needlessly generating a guid here because the only use case for this path is the transformer which will immediately
        // replace the guid. If we find further use cases, we can extend parameters to pass down the federation guid to _OnInserted
        defaultSubCat.CopyIdentityFrom(defaultSubCatId, BeGuid());
    }
    DgnSubCategoryCPtr persistentSubCat = defaultSubCat.Insert();
    BeAssert(persistentSubCat.IsValid());
    BeAssert(persistentSubCat->GetSubCategoryId() == defaultSubCatId);
    BeAssert(persistentSubCat->GetCategoryId() == GetCategoryId());

    UNUSED_VARIABLE(persistentSubCat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCategory::SetDefaultAppearance(DgnSubCategory::Appearance const& app) const
    {
    DgnSubCategoryCPtr subCat = DgnSubCategory::Get(GetDgnDb(), GetDefaultSubCategoryId());
    BeAssert(subCat.IsValid());
    if (!subCat.IsValid())
        {
        BeAssert(false);
        return;
        }
    auto updatedSubCat = subCat->MakeCopy<DgnSubCategory>();
    updatedSubCat->GetAppearanceR() = app;
    updatedSubCat->Update();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DrawingCategoryCPtr DrawingCategory::Insert(DgnSubCategory::Appearance const& appearance, DgnDbStatus* status)
    {
    DrawingCategoryCPtr category = GetDgnDb().Elements().Insert<DrawingCategory>(*this, status);
    if (!category.IsValid())
        return nullptr;

    SetDefaultAppearance(appearance);
    return category;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialCategoryCPtr SpatialCategory::Insert(DgnSubCategory::Appearance const& appearance, DgnDbStatus* status)
    {
    SpatialCategoryCPtr category = GetDgnDb().Elements().Insert<SpatialCategory>(*this, status);
    if (!category.IsValid())
        return nullptr;

    SetDefaultAppearance(appearance);
    return category;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId DgnCategory::GetDefaultSubCategoryId(DgnCategoryId catId)
    {
    // default sub-category id is always catId + 1
    return catId.IsValid() ? DgnSubCategoryId(catId.GetValueUnchecked() + 1) : DgnSubCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ElementIterator DrawingCategory::MakeIterator(DgnDbR db, Utf8CP whereClause, Utf8CP orderByClause)
    {
    return db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_DrawingCategory), whereClause, orderByClause);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ElementIterator SpatialCategory::MakeIterator(DgnDbR db, Utf8CP whereClause, Utf8CP orderByClause)
    {
    return db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialCategory), whereClause, orderByClause);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DgnCategory::_GenerateDefaultCode() const
    {
    BeAssert(false && "Creator of a category must set its code");
    return DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnSubCategory::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        m_data.m_descr = stmt.GetValueText(params.GetSelectIndex(prop_Description()));
        BeJsDocument json(stmt.GetValueText(params.GetSelectIndex(prop_Properties())));
        m_data.m_appearance.FromJson(json);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::_ToJson(BeJsValue val, BeJsConst opts) const
    {
    T_Super::_ToJson(val, opts);

    m_data.m_appearance.ToJson(val[json_appearance()]);
    val[json_description()] = m_data.m_descr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::_FromJson(BeJsConst props)
    {
    T_Super::_FromJson(props);

    if (props.hasMember(json_description())) // support partial update, only update description if member present
        {
        auto description = props[json_description()];
        if (description.isString())
            m_data.m_descr = description.asString();
        else
            m_data.m_descr.clear(); // allow undefined to clear an existing value
        }

    if (props.isMember(json_appearance()))
        m_data.m_appearance.FromJson(props[json_appearance()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    // default sub-categories don't have a description
    if (!IsDefaultSubCategory())
        stmt.BindText(stmt.GetParameterIndex(prop_Description()), m_data.m_descr.c_str(), IECSqlBinder::MakeCopy::No);

    BeJsDocument val;
    m_data.m_appearance.ToJson(val);
    stmt.BindText(stmt.GetParameterIndex(prop_Properties()), val.Stringify().c_str(), IECSqlBinder::MakeCopy::Yes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::_CopyFrom(DgnElementCR el, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(el, opts);
    auto other = dynamic_cast<DgnSubCategoryCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr != other)
        m_data = other->m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategory::CreateParams::CreateParams(DgnDbR db, DgnCategoryId categoryId, Utf8StringCR name, Appearance const& app, Utf8StringCR descr)
    : T_Super(db, DgnModel::DictionaryId(), QueryDgnClassId(db), CreateCode(db, categoryId, name), nullptr, categoryId), m_data(app, descr)
    {
    DgnCategoryCPtr category = DgnCategory::Get(db, categoryId);
    SetModelId(category.IsValid() ? category->GetModelId() : DgnModelId()); // A SubCategory must be in the same DefinitionModel as its parent Category
    m_parentRelClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_CategoryOwnsSubCategories);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DgnSubCategory::CreateCode(DgnDbR db, DgnCategoryId categoryId, Utf8StringCR subCategoryName)
    {
    DgnCategoryCPtr category = DgnCategory::Get(db, categoryId);
    return category.IsValid() ? CreateCode(*category, subCategoryName) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DgnSubCategory::CreateCode(DgnCategoryCR category, Utf8StringCR subCategoryName)
    {
    return CodeSpec::CreateCode(BIS_CODESPEC_SubCategory, category, subCategoryName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId DgnSubCategory::QuerySubCategoryId(DgnDbR db, DgnCodeCR code)
    {
    return DgnSubCategoryId(db.Elements().QueryElementIdByCode(code).GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnSubCategory::_SetParentId(DgnElementId, DgnClassId)
    {
    // a sub-category cannot be re-parented. Its parent must be a category. The parent category is set up when we create + insert the subcategory.
    return DgnDbStatus::InvalidParent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnSubCategory::QueryCategoryId(DgnDbR db, DgnSubCategoryId subCatId)
    {
    if (!subCatId.IsValid())
        return DgnCategoryId();

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT Parent.Id FROM " BIS_SCHEMA(BIS_CLASS_SubCategory) " WHERE ECInstanceId=? LIMIT 1");
    if (stmt.IsValid())
        {
        stmt->BindId(1, subCatId);
        if (BE_SQLITE_ROW == stmt->Step())
            return stmt->GetValueId<DgnCategoryId>(0);
        }

    BeAssert(false && "Subcategory has no parent category");
    return DgnCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ElementIterator DgnSubCategory::MakeIterator(DgnDbR db, DgnCategoryId categoryId, Utf8CP whereClause, Utf8CP orderByClause)
    {
    BeAssert(categoryId.IsValid());

    Utf8String combinedWhere;
    if (whereClause)
        {
        combinedWhere.append(whereClause);
        combinedWhere.append(" AND Parent.Id=?");
        }
    else
        {
        combinedWhere.append("WHERE Parent.Id=?");
        }

    ElementIterator iterator = db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_SubCategory), combinedWhere.c_str(), orderByClause);
    iterator.GetStatement()->BindId(1, categoryId);
    return iterator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnSubCategory::QueryCount(DgnDbR db, DgnCategoryId catId)
    {
    size_t count = 0;
    Utf8String ecsql("SELECT count(*) FROM " BIS_SCHEMA(BIS_CLASS_SubCategory));
    if (catId.IsValid())
        ecsql.append(" WHERE Parent.Id=?");

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(ecsql.c_str());
    if (stmt.IsValid())
        {
        if (catId.IsValid())
            stmt->BindId(1, catId);

        if (BE_SQLITE_ROW == stmt->Step())
            count = stmt->GetValueInt(0);
        }

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnSubCategory::_OnInsert()
    {
    // A sub-category requires a parent category.
    if (!DgnCategory::IsValidName(GetSubCategoryName()))
        return DgnDbStatus::InvalidName;

    DgnCategoryId catId(GetParentId().GetValueUnchecked());
    DgnCategoryCPtr cat = catId.IsValid() ? DgnCategory::Get(GetDgnDb(), catId) : nullptr;
    return cat.IsValid() ? T_Super::_OnInsert() : DgnDbStatus::InvalidParent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnSubCategory::_OnDelete() const
    {
    // can only be deleted through a purge operation
    return GetDgnDb().IsPurgeOperationActive() ? T_Super::_OnDelete() : DgnDbStatus::DeletionProhibited;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnSubCategory::IsDefaultSubCategory() const
    {
    return DgnCategory::GetDefaultSubCategoryId(GetCategoryId()) == GetSubCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DgnSubCategory::_GenerateDefaultCode() const
    {
    DgnCategoryCPtr cat = IsDefaultSubCategory() ? DgnCategory::Get(GetDgnDb(), GetCategoryId()) : nullptr;
    if (cat.IsValid())
        return CreateCode(*cat, cat->GetCategoryName());

    BeAssert(false && "The creator of a sub-category must set its code");
    return DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::Appearance::FromJson(BeJsConst val)
    {
    Init();

    if (val.isNull())
        return;

    m_invisible = val.getMemberBoolean(json_invisible(), false);
    m_dontPlot = val.getMemberBoolean(json_dontPlot(), false);
    m_dontSnap = val.getMemberBoolean(json_dontSnap(), false);
    m_dontLocate = val.getMemberBoolean(json_dontLocate(), false);
    m_color = ColorDef(val[json_color()].asUInt());
    m_weight = val[json_weight()].asUInt();
    if (val.isMember(json_style()))
        m_style  = DgnStyleId(val[json_style()].asUInt64());
    else
        m_style.Invalidate();
    m_displayPriority = val[json_priority()].asInt();
    m_transparency = val[json_transp()].asDouble();

    if (val.isMember(json_material()))
        m_material = RenderMaterialId(val[json_material()].asUInt64());
    else
        m_material.Invalidate();

    if (m_useFillColor = val.isMember(json_fill()))
        m_fillColor = ColorDef(val[json_fill()].asUInt());

    if (m_useFillTransparency = val.isMember(json_transpFill()))
        m_fillTransparency = val[json_transpFill()].asDouble();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::Appearance::ToJson(BeJsValue val) const
    {
    val.SetNull();
    if (m_invisible)            val[json_invisible()] = true;
    if (m_dontPlot)             val[json_dontPlot()] = true;
    if (m_dontSnap)             val[json_dontSnap()] = true;
    if (m_dontLocate)           val[json_dontLocate()] = true;
    if (ColorDef::Black() != m_color)  val[json_color()]  = m_color.GetValue();
    if (0 != m_weight)          val[json_weight()] = m_weight;
    if (m_style.IsValid())      val[json_style()]  = m_style;
    if (0 != m_displayPriority) val[json_priority()] = m_displayPriority;
    if (m_material.IsValid())   val[json_material()] = m_material;
    if (0.0 != m_transparency)  val[json_transp()] = m_transparency;

    if (m_useFillColor)
        val[json_fill()] = m_fillColor.GetValue();

    if (m_useFillTransparency)
        val[json_transpFill()] = m_fillTransparency;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnSubCategory::Appearance::operator==(Appearance const& other) const
    {
    return m_invisible==other.m_invisible &&
           m_color==other.m_color &&
           m_weight==other.m_weight &&
           m_style==other.m_style &&
           m_displayPriority==other.m_displayPriority &&
           m_material==other.m_material &&
           m_transparency==other.m_transparency &&
           m_useFillColor==other.m_useFillColor && (!m_useFillColor || m_fillColor==other.m_fillColor) &&
           m_useFillTransparency==other.m_useFillTransparency && (!m_useFillTransparency || m_fillTransparency==other.m_fillTransparency);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::Appearance::RelocateToDestinationDb(DgnImportContext& context)
    {
    if (!context.IsBetweenDbs())
        return;

    if (m_style.IsValid())
        m_style = context.RemapLineStyleId(m_style);

    if (m_material.IsValid())
        m_material = context.RemapRenderMaterialId(m_material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::Override::ToJson(BeJsValue outValue) const
    {
    if (m_flags.m_invisible)    outValue[Appearance::json_invisible()] = m_value.IsInvisible();
    if (m_flags.m_color)        outValue[Appearance::json_color()] = m_value.GetColor().GetValue();
    if (m_flags.m_weight)       outValue[Appearance::json_weight()] = m_value.GetWeight();
    if (m_flags.m_style)        outValue[Appearance::json_style()] = m_value.GetStyle();
    if (m_flags.m_material)     outValue[Appearance::json_material()] = m_value.GetRenderMaterial();
    if (m_flags.m_priority)     outValue[Appearance::json_priority()] = m_value.GetDisplayPriority();
    if (m_flags.m_transparency) outValue[Appearance::json_transp()] = m_value.GetTransparency();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::Override::FromJson(BeJsConst val)
    {
    Init();

    if (val.isMember(Appearance::json_invisible()))    SetInvisible(val[Appearance::json_invisible()].asBool());
    if (val.isMember(Appearance::json_color()))        SetColor(ColorDef(val[Appearance::json_color()].asUInt()));
    if (val.isMember(Appearance::json_weight()))       SetWeight(val[Appearance::json_weight()].asUInt());
    if (val.isMember(Appearance::json_style()))        SetStyle(DgnStyleId(val[Appearance::json_style()].asUInt64()));
    if (val.isMember(Appearance::json_material()))     SetRenderMaterial(RenderMaterialId(val[Appearance::json_material()].asUInt64()));
    if (val.isMember(Appearance::json_priority()))     SetDisplayPriority(val[Appearance::json_priority()].asInt());
    if (val.isMember(Appearance::json_transp())) SetTransparency(val[Appearance::json_transp()].asDouble());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::Override::ApplyTo(Appearance& appear) const
    {
    if (m_flags.m_invisible)    appear.SetInvisible(m_value.IsInvisible());
    if (m_flags.m_color)        appear.SetColor(m_value.GetColor());
    if (m_flags.m_weight)       appear.SetWeight(m_value.GetWeight());
    if (m_flags.m_style)        appear.SetStyle(m_value.GetStyle());
    if (m_flags.m_material)     appear.SetRenderMaterial(m_value.GetRenderMaterial());
    if (m_flags.m_priority)     appear.SetDisplayPriority(m_value.GetDisplayPriority());
    if (m_flags.m_transparency) appear.SetTransparency(m_value.GetTransparency());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCategory::_OnImported(DgnElementCR original, DgnImportContext& importer) const
    {
    DgnCategoryId srcCatId = DgnCategoryId(original.GetElementId().GetValue());

    // Copy over the default appearance and remap its internal IDs
    DgnSubCategoryCPtr srcSubCat = DgnSubCategory::Get(importer.GetSourceDb(), DgnCategory::GetDefaultSubCategoryId(srcCatId));
    DgnSubCategory::Appearance appearance = srcSubCat->GetAppearance();
    appearance.RelocateToDestinationDb(importer);
    SetDefaultAppearance(appearance);
    importer.AddSubCategory(srcSubCat->GetSubCategoryId(), GetDefaultSubCategoryId(GetCategoryId()));

    if (importer.IsBetweenDbs())
        {
        BeAssert(nullptr != dynamic_cast<DgnCategoryCP>(&original));

        // When we import a Category, we currently import all of its SubCategories too.
        // If we decide to change this policy and wait until the caller asks for a SubCategory,
        // we must change GeometryStreamIO::Import to call RemapSubCategory, rather than FindSubCategory.
        for (ElementIteratorEntryCR srcSubCategory : DgnSubCategory::MakeIterator(importer.GetSourceDb(), DgnCategoryId(original.GetElementId().GetValue())))
            importer.RemapSubCategory(GetCategoryId(), srcSubCategory.GetId<DgnSubCategoryId>());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCategory::_RemapIds(DgnImportContext& importer)
    {
    // Note - Wait for _OnImported to remap the default SubCategory -- This DgnCategory doesn't have an ElementId yet.
    T_Super::_RemapIds(importer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::_RemapIds(DgnImportContext& importer)
    {
    m_data.m_appearance.RelocateToDestinationDb(importer);
    T_Super::_RemapIds(importer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnCategory::ImportCategory(DgnCategoryId srcCatId, DgnImportContext& importer)
    {
    //  See if we already have a category with the same name in the destination Db.
    //  If so, we'll map the source category to it.
    DgnCategoryCPtr srcCat = DgnCategory::Get(importer.GetSourceDb(), srcCatId);
    BeAssert(srcCat.IsValid());
    if (!srcCat.IsValid())
        {
        BeAssert(false && "invalid source category ID");
        return DgnCategoryId();
        }

    DgnCategoryId dstCatId = QueryCategoryId(importer.GetDestinationDb(), srcCat->GetCode());
    if (dstCatId.IsValid())
        {
        importer.AddCategory(srcCatId, dstCatId);

        for (ElementIteratorEntryCR srcSubCategory : DgnSubCategory::MakeIterator(importer.GetSourceDb(), srcCatId)) // Make sure the subcats are remapped!
            importer.RemapSubCategory(dstCatId, srcSubCategory.GetId<DgnSubCategoryId>());

        return dstCatId;
        }

    //  No such Category in the destination. Ask the source Category to import itself.
    auto importedElem = srcCat->Import(nullptr, importer.GetDestinationDb().GetDictionaryModel(), importer);
    return importedElem.IsValid()? DgnCategoryId(importedElem->GetElementId().GetValue()): DgnCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId DgnSubCategory::ImportSubCategory(DgnSubCategoryId srcSubCatId, DgnCategoryId dstCatId, DgnImportContext& importer)
    {
    //  See if we already have a SubCategory in the destination Db.
    //  If so, map the source SubCategory to it.
    DgnSubCategoryCPtr srcSubCat = DgnSubCategory::Get(importer.GetSourceDb(), srcSubCatId);
    BeAssert(srcSubCat.IsValid());
    if (srcSubCat.IsNull())
        {
        BeAssert(false && "Invalid SubCategory ID");
        return DgnSubCategoryId();
        }

    DgnCode dstSubCatCode = DgnSubCategory::CreateCode(importer.GetDestinationDb(), dstCatId, srcSubCat->GetSubCategoryName());
    DgnSubCategoryId dstSubCatId = QuerySubCategoryId(importer.GetDestinationDb(), dstSubCatCode);
    if (dstSubCatId.IsValid())
        {
        //  *** TBD: Check if the Appearances match. If not, rename and remap
        importer.AddSubCategory(srcSubCatId, dstSubCatId);
        return dstSubCatId;
        }

    //  No such SubCategory in the destination. Ask the source SubCategory to import itself.
    BeAssert(!srcSubCat->IsDefaultSubCategory() && "DgnCategory::_OnImported should have remapped the default SubCategory");

    auto importedElem = srcSubCat->Import(nullptr, importer.GetDestinationDb().GetDictionaryModel(), importer);
    return importedElem.IsValid()? DgnSubCategoryId(importedElem->GetElementId().GetValue()): DgnSubCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnImportContext::_RemapCategory(DgnCategoryId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnCategoryId dest = m_remap.Find(source);
    if (dest.IsValid())
        return dest;

    return DgnCategory::ImportCategory(source, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId DgnImportContext::_RemapSubCategory(DgnCategoryId destCategoryId, DgnSubCategoryId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnSubCategoryId dest = m_remap.Find(source);
    if (dest.IsValid())
        return dest;

    return DgnSubCategory::ImportSubCategory(source, destCategoryId, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnImportContext::_RemapGeometryStreamIds(GeometryStreamR geom)
    {
    return GeometryStreamIO::Import(geom, geom, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnCategory::IsValidName(Utf8StringCR name)
    {
    return DgnDbTable::IsValidName(name, GetIllegalCharacters());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnCategory::_OnInsert()
    {
    return IsValidName(GetCategoryName()) ? T_Super::_OnInsert() : DgnDbStatus::InvalidName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnCategory::_OnUpdate(DgnElementCR el)
    {
    return IsValidName(GetCategoryName()) ? T_Super::_OnUpdate(el) : DgnDbStatus::InvalidName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnSubCategory::_OnUpdate(DgnElementCR el)
    {
    return DgnCategory::IsValidName(GetSubCategoryName()) ? T_Super::_OnUpdate(el) : DgnDbStatus::InvalidName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
DgnSubCategory::CreateParams DgnSubCategory::CreateParamsFromECInstance(DgnDbR db, ECN::IECInstanceCR properties, DgnDbStatus* inStat)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);
    DgnCategoryId categoryId;
        {
        ECValue v;
        if (ECObjectsStatus::Success != properties.GetValue(v, "Parent") || v.IsNull())
            {
            stat = DgnDbStatus::InvalidParent;
            return DgnSubCategory::CreateParams(db, DgnCategoryId(), "", Appearance());
            }
        categoryId = v.GetNavigationInfo().GetId<DgnCategoryId>();
        if (!categoryId.IsValid())
            {
            stat = DgnDbStatus::InvalidParent;
            return DgnSubCategory::CreateParams(db, DgnCategoryId(), "", Appearance());
            }
        }

    ECN::ECValue codeValue;
    if (ECN::ECObjectsStatus::Success != properties.GetValue(codeValue, "CodeValue"))
        {
        stat = DgnDbStatus::InvalidName;
        return DgnSubCategory::CreateParams(db, DgnCategoryId(), "", Appearance());
        }

    ECN::ECValue props;
    if (ECN::ECObjectsStatus::Success != properties.GetValue(props, "Properties"))
        {
        stat = DgnDbStatus::BadArg;
        return DgnSubCategory::CreateParams(db, DgnCategoryId(), "", Appearance());
        }

    ECN::ECValue descr;
    properties.GetValue(props, "Description");

    BeJsDocument val(props.GetUtf8CP());
    return DgnSubCategory::CreateParams(db, categoryId, codeValue.GetUtf8CP(), Appearance(val), !descr.IsNull() ? descr.GetUtf8CP() : "");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementPtr dgn_ElementHandler::SubCategory::_CreateNewElement(DgnDbR db, ECN::IECInstanceCR properties, bool ignoreErrors, DgnDbStatus* inStat)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);
    auto params = DgnSubCategory::CreateParamsFromECInstance(db, properties, inStat);
    if (!params.IsValid())
        return nullptr;

    auto ele = new DgnSubCategory(params);

    bset<Utf8String> ignoreProps;
    ignoreProps.insert("Parent");
    DgnElement::SetPropertyFilter filter(DgnElement::SetPropertyFilter::Ignore::WriteOnlyNullBootstrapping, ignoreErrors, ignoreProps);

    stat = ele->_SetPropertyValues(properties, filter);
    return (DgnDbStatus::Success == stat) ? ele : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void dgn_ElementHandler::Category::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    params.RegisterPropertyAccessors(layout, DgnCategory::prop_Description(),
        [] (ECValueR value, DgnElementCR elIn)
            {
            DgnCategory const& el = (DgnCategory const&) elIn;
            value.SetUtf8CP(el.GetDescription());
            return DgnDbStatus::Success;
            },
        [] (DgnElementR elIn, ECValueCR value)
            {
            if (!value.IsString())
                return DgnDbStatus::BadArg;
            DgnCategory& el = (DgnCategory&) elIn;
            el.SetDescription(value.GetUtf8CP());
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, DgnCategory::prop_Rank(),
        [] (ECValueR value, DgnElementCR elIn)
            {
            DgnCategory const& el = (DgnCategory const&) elIn;
            value.SetInteger(static_cast<int32_t>(el.GetRank()));
            return DgnDbStatus::Success;
            },
        [] (DgnElementR elIn, ECValueCR value)
            {
            if (!value.IsInteger())
                return DgnDbStatus::BadArg;
            DgnCategory& el = (DgnCategory&) elIn;
            el.SetRank(static_cast<DgnCategory::Rank>(value.GetInteger()));
            return DgnDbStatus::Success;
            });
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//--------------+---------------+---------------+---------------+---------------+-------
void dgn_ElementHandler::SubCategory::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    params.RegisterPropertyAccessors(layout, DgnSubCategory::prop_Description(),
        [] (ECValueR value, DgnElementCR elIn)
            {
            auto const& el = (DgnSubCategory const&) elIn;
            value.SetUtf8CP(el.GetDescription());
            return DgnDbStatus::Success;
            },
        [] (DgnElementR elIn, ECValueCR value)
            {
            if (!value.IsString())
                return DgnDbStatus::BadArg;
            auto& el = (DgnSubCategory&) elIn;
            if (el.IsDefaultSubCategory())
                return DgnDbStatus::ReadOnly; // default sub-categories don't have a description
            el.SetDescription(value.GetUtf8CP());
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, DgnSubCategory::prop_Properties(),
        [] (ECValueR value, DgnElementCR elIn)
            {
            auto const& el = (DgnSubCategory const&) elIn;
            BeJsDocument val;
            el.m_data.m_appearance.ToJson(val);
            value.SetUtf8CP(val.Stringify().c_str());
            return DgnDbStatus::Success;
            },
        [] (DgnElementR elIn, ECValueCR value)
            {
            if (!value.IsUtf8())
                return DgnDbStatus::BadArg;
            auto& el = (DgnSubCategory&) elIn;
            el.m_data.m_appearance.FromJson(BeJsDocument(value.GetUtf8CP()));
            return DgnDbStatus::Success;
            });
    }
