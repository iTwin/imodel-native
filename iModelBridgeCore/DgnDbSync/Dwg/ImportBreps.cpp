/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/ImportBreps.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

DWG_PROTOCALEXT_DEFINE_MEMBERS(DwgBrepExt)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgBrepExt::_ConvertToBim (ProtocalExtensionContext& context, DwgImporter& importer)
    {
    // a sanity check:
    m_entity = context.GetEntityPtrR().get ();
    if (nullptr == m_entity || !context.GetModel().Is3d() || !UtilsLib::IsAsmEntity(m_entity))
        return  BSIERROR;

    m_toBimContext = &context;
    m_importer = &importer;

    BentleyStatus   status = BSIERROR;
    ElementInputsR  inputs = context.GetElementInputsR ();
    ElementResultsR results = context.GetElementResultsR ();

    // convert ASM Brep as Parasolid Brep only as an option:
    if (importer.GetOptions().IsAsmAsParasolid())
        {
        DwgImporter::ElementCreateParams  params(inputs.GetTargetModelR());

        status = importer._GetElementCreateParams (params, inputs.GetTransform(), *m_entity);
        if (BSISUCCESS != status)
            return  status;

#if defined (BENTLEYCONFIG_PARASOLID)
        PSolidKernelManager::StartSession ();

        PK_BODY_create_topology_2_r_t   brep;
        GeometricPrimitivePtr   geometry = this->CreateGeometry (brep);
        if (geometry.IsValid())
            status = this->CreateElement (*geometry.get(), params);

        // we are done with creating element - always free the Parasolid body:
        this->FreeBrep (brep);
        PSolidKernelManager::StopSession ();
#endif
        }

    // if Brep conversion fails, always falls back to drawing the entity:
    if (status != BSISUCCESS)
        status = importer._ImportEntity (results, inputs);
    
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/18
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr DwgBrepExt::_ConvertToGeometry (DwgDbEntityCP entity, DwgImporter& importer)
    {
    // a sanity check:
    if (nullptr == entity || !UtilsLib::IsAsmEntity(entity))
        return  nullptr;

    m_toBimContext = nullptr;
    m_importer = &importer;
    m_entity = entity;

#if defined (BENTLEYCONFIG_PARASOLID)
    PK_BODY_create_topology_2_r_t brep;
    GeometricPrimitivePtr   geometry = this->CreateGeometry (brep);

    // return the geometry to the caller and let the caller free the Parasolid body:
    if (!geometry.IsValid())
        this->FreeBrep (brep);

    return  geometry;
#endif

    return  nullptr;
    }

#if defined (BENTLEYCONFIG_PARASOLID)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/18
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr DwgBrepExt::CreateGeometry (PK_BODY_create_topology_2_r_t& brep)
    {
    brep.body = PK_ENTITY_null;

    GeometricPrimitivePtr   geometry;

    // convert ASM Brep as Parasolid Brep only as an option:
    if (m_importer->GetOptions().IsAsmAsParasolid())
        {
        if (!PSolidKernelManager::IsSessionStarted())
            {
            BeAssert (false && "Parasolid session has not started!!");
            return  geometry;
            }

        Transform   transform = Transform::FromIdentity ();

        auto status = UtilsLib::ConvertAsmToParasolid (brep, transform, m_entity);
        if (status == BSISUCCESS)
            {
            if (!transform.IsIdentity())
                transform = transform.ValidatedInverse ();

            // if the geometry is to be directly added into the target model, apply model transform:
            if (nullptr != m_toBimContext)
                transform.InitProduct (m_toBimContext->GetTransform(), transform);

            auto psEntity = PSolidUtil::CreateNewEntity (brep.body, transform, false);
            if (psEntity.IsValid())
                geometry = GeometricPrimitive::Create (psEntity);
            }
        }

    return  geometry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgBrepExt::FreeBrep (PK_BODY_create_topology_2_r_t& brep) const
    {
    if (brep.body != PK_ENTITY_null)
        {
        PK_ENTITY_t deletes[] = {brep.body};

        auto pkError = ::PK_ENTITY_delete (1, deletes);
        if (PK_ERROR_no_errors != pkError)
            {
            Utf8PrintfString msg("Failled deleting a Parasolid body. [Error=%d, ID=%llx]!\n", pkError, m_entity->GetObjectId().ToUInt64());
            m_importer->ReportIssue (DwgImporter::IssueSeverity::Info, IssueCategory::UnexpectedData(), Issue::Error(), msg.c_str());
            }
        brep.body = PK_ENTITY_null;
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/18
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef    DwgBrepExt::GetEffectiveColor () const
    {
    /*-----------------------------------------------------------------------------------
    This is not a true resolver for entity color, which can float to a different block 
    reference or a different viewport.  This method only resolves modelspace entities
    that are not in a block.  Should we support Breps in blocks in the future we have
    to figure out how to handle floating colors.
    -----------------------------------------------------------------------------------*/
    auto colorDef = ColorDef::White ();
    auto entityColor = m_entity->GetEntityColor ();

    switch (entityColor.GetColorMethod())
        {
        case DwgCmEntityColor::Method::ByLayer:
            {
            DwgDbLayerTableRecordPtr layer(m_entity->GetLayerId(), DwgDbOpenMode::ForRead);
            if (layer.OpenStatus() == DwgDbStatus::Success)
                {
                // at least for now, ignore color overridden by viewport:
                auto layerColor = layer->GetColor ();
                if (layerColor.IsByACI())
                    colorDef = DwgHelper::GetColorDefFromACI (layerColor.GetIndex());
                }
            break;
            }
        case DwgCmEntityColor::Method::ByBlock:
            // in modelspace, layerByBlock floats to color White.
            break;
        case DwgCmEntityColor::Method::ByACI:
            colorDef = DwgHelper::GetColorDefFromACI (entityColor.GetIndex());
            break;
        }

    return colorDef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgBrepExt::CreateElement (GeometricPrimitiveR geometry, DwgImporter::ElementCreateParams& params)
    {
    // add elements to model only for modelspace entities:
    if (m_toBimContext == nullptr)
        return  BSIERROR;

    auto builder = GeometryBuilder::Create (params.GetModelR(), params.GetCategoryId(), DPoint3d::FromZero());
    if (!builder.IsValid())
        return  BSIERROR;

    Render::GeometryParams  display;
    display.SetCategoryId (params.GetCategoryId());
    display.SetSubCategoryId (params.GetSubCategoryId());
    display.SetGeometryClass (Render::DgnGeometryClass::Primary);
    display.SetLineColor (this->GetEffectiveColor());

    builder->Append (display);
    builder->Append (geometry);

    ElementFactory  factory(m_toBimContext->GetElementResultsR(), m_toBimContext->GetElementInputsR(), params, *m_importer);
    factory.SetGeometryBuilder (builder.get());

    return factory.CreateElement ();
    }
