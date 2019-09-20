/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
ElementFactory::ElementFactory (DwgImporter::ElementImportResults& results, DwgImporter::ElementImportInputs& inputs, DwgImporter::ElementCreateParams& params, DwgImporter& importer)
    : m_results(results), m_inputs(inputs), m_createParams(params), m_importer(importer), 
      m_elementParams(inputs.GetTargetModelR().GetDgnDb(), inputs.GetTargetModelR().GetModelId(), inputs.GetClassId())
    {
    // update DgnCode & caller will set user label as needed
    m_elementCode = m_createParams.GetElementCode ();
    m_elementParams.SetCode (m_elementCode);
    m_elementLabel = m_importer._GetElementLabel (m_inputs);
    m_elementParams.SetUserLabel (m_elementLabel.c_str());

    m_partModel = m_importer.GetOrCreateJobDefinitionModel ();
    if (!m_partModel.IsValid())
        {
        // should not occur but back it up anyway!
        m_importer.ReportError (IssueCategory::Unknown(), Issue::MissingJobDefinitionModel(), "GeometryParts");
        m_partModel = &m_importer.GetDgnDb().GetDictionaryModel ();
        }
    m_geometryMap = nullptr;

    m_modelTransform = m_inputs.GetTransform ();
    m_baseTransform = Transform::FromIdentity ();
    m_invBaseTransform = Transform::FromIdentity ();
    m_hasBaseTransform = false;
    m_basePartScale = 0.0;
    m_is3d = m_inputs.GetTargetModelR().Is3d ();
    m_canCreateSharedParts = m_importer.GetOptions().IsBlockAsSharedParts ();
    m_sourceBlockId.SetNull ();
    m_sourceLayerId.SetNull ();

    // find the element handler
    m_elementHandler = dgn_ElementHandler::Element::FindHandler (m_inputs.GetTargetModelR().GetDgnDb(), m_inputs.GetClassId());
    BeAssert(nullptr != m_elementHandler && "Null element handler!");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    ElementFactory::SetDefaultCreation ()
    {
    // if user does not want shared parts, honor the request:
    if (!m_canCreateSharedParts)
        return;

    DwgDbBlockReferenceCP   insert = DwgDbBlockReference::Cast(m_inputs.GetEntityP());
    if (nullptr != insert)
        {
        // prefer share parts for a named block
        m_canCreateSharedParts = true;

        auto blockTrans = Transform::FromIdentity ();
        insert->GetBlockTransform (blockTrans);

        m_sourceBlockId = insert->GetBlockTableRecordId ();
        m_sourceLayerId = insert->GetLayerId ();

        // don't need to create shared parts for an anoymouse block
        DwgDbBlockTableRecordPtr    block (m_sourceBlockId, DwgDbOpenMode::ForRead);
        if (block.OpenStatus() == DwgDbStatus::Success && block->IsAnonymous())
            m_canCreateSharedParts = false;

        // this call may also reset m_canCreateSharedParts=false
        this->SetBaseTransform (blockTrans);
        }
    else
        {
        // individual elements for all other entities
        m_canCreateSharedParts = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    ElementFactory::SetBaseTransform (TransformCR blockTrans)
    {
    /*-----------------------------------------------------------------------------------
    DgnDb geometry requires normalized and uniformly scaled transform.  To create shared
    parts we normalize base transform and save a uniform scale if it has one. We will call 
    ApplyUniformPartScale to apply this scale on a part transform at a later step creating
    elements for the parts.

    For a base transform from which we cannot create parts, we will create individual 
    elements instead of shared parts.  We will directly transform individual geometry 
    in-place as needed via TransformGeometry.

    A local part scale, m_basePartScale, is saved here only for the purpose of caching share
    parts in block-parts map for this block.  The cached parts are only used during the same 
    import session. If this block is instantiated multiple times at the same scale in the model,
    the cached parts can be directly re-used, bypassing the whole step of CreateSharedParts().
    In essense, m_basePartScale serves for the sake of performance.  it does not participate 
    in the calculation of the share parts.

    The input transform is the block transform before model transform. Model transform is
    applied to the translation.
    -----------------------------------------------------------------------------------*/
    auto blockToModel = Transform::FromProduct (m_modelTransform, blockTrans);

    if (!DwgHelper::GetTransformForSharedParts(&m_baseTransform, &m_basePartScale, blockToModel))
        m_canCreateSharedParts = false;

    if (this->Validate2dTransform(m_baseTransform) && m_canCreateSharedParts && m_basePartScale != 0.0)
        DwgHelper::NegateScaleForSharedParts (m_basePartScale, blockTrans);

    m_invBaseTransform.InverseOf (m_baseTransform);

    if (!m_canCreateSharedParts)
        m_basePartScale = 0.0;

    m_hasBaseTransform = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    ElementFactory::TransformGeometry (GeometricPrimitiveR geometry, TransformR geomTrans, double* partScale) const
    {
    /*-----------------------------------------------------------------------------------
    Factor out "valid" matrix parts of the input DWG geometry transform, build a return 
    transform from a pure rotaton and a translation, and then apply scales & skews to the
    input geometry in-place.

    When the geometry is used to build a share part, i.e. partScale != nullptr, a part scale
    is calculated based on the input geometry transform.  The caller will save the scale
    into the part entry, which will be inverse applied to the geometry element at next step.

    When the geometry is used to create an individual geometry, i.e. nullptr == partScale, 
    there is no part scale applied.
    -----------------------------------------------------------------------------------*/
    auto inplaceTrans = Transform::FromProduct (m_modelTransform, geomTrans);
    auto matrix = inplaceTrans.Matrix ();
    auto translation = inplaceTrans.Translation ();

    if (nullptr != partScale)
        *partScale = 0.0;

    RotMatrix   rotation, skew;
    if (matrix.RotateAndSkewFactors(rotation, skew, 0, 1))
        {
        // apply skew transform in place
        if (skew.IsIdentity())
            {
            // no scales
            inplaceTrans.InitIdentity ();
            }
        else
            {
            if (nullptr != partScale)
                {
                // scales exist and we are creating a shared part - only if it is uniformaly scaled, we will apply the scale on the part:
                RotMatrix   rigid;
                double      scale = 1.0;
                if (skew.IsRigidSignedScale(rigid, scale))
                    *partScale = scale;
                }
            inplaceTrans.InitFrom (skew);
            }

        // return a transform of pure ration + translation for GeometryBuilder
        geomTrans.InitFrom (rotation, translation);
        }
    else
        {
        // should not occur!
        geomTrans.InitIdentity ();
        }

    // remove/invert the part scale, if building parts
    double scale = nullptr == partScale ? 0.0 : *partScale;
    this->ApplyPartScale (inplaceTrans, scale, true);

    if (!inplaceTrans.IsIdentity())
        geometry.TransformInPlace (inplaceTrans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    ElementFactory::ApplyPartScale (TransformR transform, double scale, bool invert) const
    {
    // valid only for building parts - input scale should have been set to 0 for individual elements.
    scale = fabs (scale);
    if (scale > 1.0e-10 && fabs(scale - 1.0) > 0.001)
        {
        if (invert)
            scale = 1.0 / scale;
        transform.ScaleMatrixColumns (scale, scale, scale);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ElementFactory::Validate2dTransform (TransformR transform) const
    {
    DPoint3d    placementPoint;
    transform.GetTranslation (placementPoint);

    RotMatrix   matrix;
    transform.GetMatrix (matrix);

    YawPitchRollAngles  angles;
    YawPitchRollAngles::TryFromRotMatrix (angles, matrix);

    bool changed = false;

    // validate Placement2d
    if (!m_is3d)
        {
        // Element2d only uses Yaw:
        if (0.0 != angles.GetPitch().Degrees() || 0.0 != angles.GetRoll().Degrees())
            {
            angles = YawPitchRollAngles (AngleInDegrees::FromDegrees(0.0), angles.GetPitch(), angles.GetRoll());
            transform = Transform::FromProduct (transform, angles.ToTransform(DPoint3d::FromZero()));
            // reset near zero rotation components which will cause GeometryBuilder to fail
            for (int i = 0; i < 3; i++)
                {
                for (int j = 0; j < 3; j++)
                    {
                    if (fabs(transform.form3d[i][j]) < 1.0e-4)
                        transform.form3d[i][j] = 0.0;
                    }
                }
            changed = true;
            }
        // and has no z-translation:
        placementPoint.z = 0.0;
        transform.SetTranslation (placementPoint);
        }
    return  changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String  ElementFactory::BuildPartCodeValue (DwgImporter::GeometryEntry const& geomEntry)
    {
    // set a persistent code value, "blockName[fileId:blockId]-partIndex"
    auto name = geomEntry.GetBlockName().c_str ();
    auto blockid = geomEntry.GetBlockId().ToUInt64 ();
    auto fileid = geomEntry.GetDwgFileId().GetValue ();
    auto partNo = geomEntry.GetPartIndex ();

    // check part scale per block
    double checkPartScale = m_basePartScale;
    if (m_sourceBlockId.ToUInt64() != blockid)
        {
        checkPartScale = geomEntry.GetTransform().Determinant ();
        if (m_basePartScale < 0.0)
            checkPartScale = -checkPartScale;
        }

    // a different code value for a mirrored block
    if (checkPartScale < 0.0)
        return Utf8PrintfString ("%s[%d:%llx:m]-%lld", name, fileid, blockid, partNo);
    else
        return Utf8PrintfString ("%s[%d:%llx]-%lld", name, fileid, blockid, partNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ElementFactory::NeedsSeparateElement (DgnCategoryId id) const
    {
    if (!m_geometryBuilder.IsValid())
        return  false;
    // different category    
    if (m_geometryBuilder->GetGeometryParams().GetCategoryId() != id)
        return  true;
    // too big of an element
    return  m_geometryBuilder->GetCurrentSize() > 20000000;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeometryPartId ElementFactory::CreateGeometryPart (DRange3dR range, double& partScale, TransformR geomToLocal, Utf8StringCR partTag, DwgImporter::GeometryEntry const& geomEntry)
    {
    DgnGeometryPartId   partId;
    partId.Invalidate ();

    auto& db = m_importer.GetDgnDb ();
    auto partBuilder = GeometryBuilder::CreateGeometryPart (db, m_is3d);
    if (!partBuilder.IsValid())
        return  partId;

    auto geomPart = DgnGeometryPart::Create (*m_partModel);
    if (!geomPart.IsValid())
        return  partId;

    // show part name as block name
    geomPart->SetUserLabel (geomEntry.GetBlockName().c_str());

    // build a valid part transform, and transform geometry in-place as necessary
    auto geometry = geomEntry.GetGeometry ();
    this->TransformGeometry (geometry, geomToLocal, &partScale);

    partBuilder->Append (geometry);

    // insert the new part to db
    if (partBuilder->Finish(*geomPart) != BSISUCCESS || !db.Elements().Insert<DgnGeometryPart>(*geomPart).IsValid())
        return  partId;

    partId = geomPart->GetId ();
    range = geomPart->GetBoundingBox ();

    // insert the new part into the syncInfo
    DwgSourceAspects::GeomPartAspect aspect = DwgSourceAspects::GeomPartAspect::Create(m_partModel->GetModeledElementId(), partTag, db);
    if (aspect.IsValid())
        {
        aspect.AddAspect(*geomPart);
        geomPart->Update();
        }
    else
        {
        m_importer.ReportError (IssueCategory::Sync(), Issue::Error(), "failed adding geometry part in the SyncInfo");
        }

    return  partId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementFactory::GetGeometryPart (DRange3dR range, double& partScale, TransformR geomToLocal, DgnGeometryPartId partId, DwgImporter::GeometryEntry const& geomEntry)
    {
    auto& db = m_importer.GetDgnDb ();
    if (DgnGeometryPart::QueryGeometryPartRange(range, db, partId) != BSISUCCESS)
        {
        BeAssert (false && "cannot query existing part range!!");
        return  BSIERROR;
        }

    // build a valid part transform from DWG transform
    geomToLocal.InitProduct (m_modelTransform, geomToLocal);
    DwgHelper::GetTransformForSharedParts (&geomToLocal, &partScale, geomToLocal);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementFactory::GetOrCreateGeometryPart (DwgImporter::SharedPartEntry& part, DwgImporter::GeometryEntry const& geomEntry)
    {
    // this method creates a new shared part geometry
    auto& db = m_importer.GetDgnDb ();
    auto partTag = this->BuildPartCodeValue (geomEntry);
    auto geomToLocal = geomEntry.GetTransform ();
    double partScale = 0.0;
    DRange3d range;

    auto partId = DwgSourceAspects::GeomPartAspect::FindGeometryPart(m_partModel->GetModeledElementId(), partTag, db);
    if (!partId.IsValid())
        {
        // create a new geometry part:
        partId = this->CreateGeometryPart (range, partScale, geomToLocal, partTag, geomEntry);
        if (!partId.IsValid())
            return  BSIERROR;
        }
    else
        {
        // use existing geometry part
        if (this->GetGeometryPart(range, partScale, geomToLocal, partId, geomEntry) != BSISUCCESS)
            return  BSIERROR;
        }

    part.SetTransform (Transform::FromProduct(m_invBaseTransform, geomToLocal));
    part.SetGeometryParams (geomEntry.GetGeometryParams());
    part.SetPartScale (partScale);
    part.SetPartId (partId);
    part.SetPartRange (range);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementFactory::CreateSharedParts ()
    {
    BentleyStatus status = BSIERROR;
    if (nullptr == m_geometryMap || !m_sourceBlockId.IsValid())
        return  status;
    auto sourceInsert = DwgDbBlockReference::Cast(m_inputs.GetEntityP());
    if (nullptr == sourceInsert)
        return  status;

    DwgImporter::T_SharedPartList parts;

    // create shared parts from geometry collection
    for (auto blockEntry : *m_geometryMap)
        {
        // build parts for all geometries, including nested blocks
        for (auto geomEntry : blockEntry.second)
            {
            // create a new geometry builder:
            DwgImporter::SharedPartEntry  part;
            status = this->GetOrCreateGeometryPart (part, geomEntry);
            if (status == BSISUCCESS)
                parts.push_back (part);

            m_importer.Progress ();
            }
        }

    // cache the parts created for this block
    auto& blockPartsMap = m_importer.GetBlockPartsR ();
    DwgImporter::SharedPartKey  key(*sourceInsert, m_basePartScale);
    blockPartsMap.insert (DwgImporter::T_BlockPartsEntry(key, parts));

    // create part elements from the part cache:
    status = this->CreatePartElements (parts);
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementFactory::CreatePartElements (DwgImporter::T_SharedPartList const& parts)
    {
    // create shared part elements
    BentleyStatus   status = BSIERROR;
    if (parts.empty())
        return  status;

    auto& targetModel = m_inputs.GetTargetModelR ();

    size_t  failed = 0;
    m_geometryBuilder = nullptr;

    // create elements from shared parts
    for (auto part : parts)
        {
        auto partCategoryId = part.GetGeometryParams().GetCategoryId ();

        // if the part is on a different category, create a new element and reset geometry builder for a new run
        if (this->NeedsSeparateElement(partCategoryId))
            status = this->CreateElement ();

        // initialize geometry builder for a new run, either for the 1st element or for a separated child element
        if (!m_geometryBuilder.IsValid())
            m_geometryBuilder = GeometryBuilder::Create (targetModel, partCategoryId, m_baseTransform);
        
        if (!m_geometryBuilder.IsValid())
            {
            BeAssert (false && "GeometryBuilder::Create has failed!");
            continue;
            }

        // apply base part scale
        auto geomToLocal = part.GetTransform ();
        this->ApplyPartScale (geomToLocal, part.GetPartScale(), false);

        m_geometryBuilder->Append (part.GetGeometryParams());
        m_geometryBuilder->Append (part.GetPartId(), geomToLocal, part.GetPartRange());

        m_importer.Progress ();
        }

    if (!m_geometryBuilder.IsValid())
        {
        BeAssert (false && "GeometryBuilder::Create has failed!");
        return  BSIERROR;
        }

    status = this->CreateElement ();
    if (status != BSISUCCESS)
        failed++;

    if (failed > 0)
        {
        Utf8PrintfString msg("%lld out of %lld shared part geometry element(s) is/are not created for entity[%s, handle=%llx]!", failed, parts.size(), m_elementCode.GetValueUtf8CP(), m_inputs.GetEntityId().ToUInt64());
        m_importer.ReportError (IssueCategory::VisualFidelity(), Issue::Error(), msg.c_str());
        if (BSISUCCESS == status)
            status = BSIERROR;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementFactory::CreateIndividualElements ()
    {
    BentleyStatus status = BSIERROR;
    if (nullptr == m_geometryMap)
        return  status;

    // create new elements from the geometry collection
    auto headerCategoryId = m_createParams.GetCategoryId ();
    auto& targetModel = m_inputs.GetTargetModelR ();
    auto firstLocalToGeom = Transform::FromIdentity ();
    auto firstGeomToLocal = Transform::FromIdentity ();
    auto firstGeomToWorld = Transform::FromIdentity ();
    auto firstWorldToGeom = Transform::FromIdentity ();

    size_t  failed = 0, total = 0;
    for (auto blockEntry : *m_geometryMap)
        {
        for (auto geomEntry : blockEntry.second)
            {
            auto currentGeomToWorld = geomEntry.GetTransform ();
            auto categoryId = geomEntry.GetGeometryParams().GetCategoryId ();
            auto geometry = geomEntry.GetGeometryP ();
            if (nullptr == geometry)
                {
                BeAssert (false && "Invalid geometry!");
                continue;
                }

            // transform DWG geometry
            this->TransformGeometry (*geometry, currentGeomToWorld);

            // if the geometry is on a different category, create a separate element and reset geometry builder for a new run
            if (this->NeedsSeparateElement(categoryId))
                status = this->CreateElement ();

            if (!m_geometryBuilder.IsValid())
                {
                Transform   localToWorld;
                if (m_hasBaseTransform)
                    {
                    // build geometries relative to the base transform
                    localToWorld = m_baseTransform;

                    firstGeomToLocal.InitProduct (m_invBaseTransform, currentGeomToWorld);
                    firstLocalToGeom = firstGeomToLocal.ValidatedInverse ();
                    }
                else
                    {
                    // build geometries relative to the first geometry
                    if (geometry->GetLocalCoordinateFrame(firstLocalToGeom))
                        this->Validate2dTransform (firstLocalToGeom);
                    else
                        firstLocalToGeom.InitIdentity ();
                    firstGeomToLocal = firstLocalToGeom.ValidatedInverse ();

                    localToWorld = Transform::FromProduct (currentGeomToWorld, firstLocalToGeom);
                    }

                m_geometryBuilder = GeometryBuilder::Create (targetModel, categoryId, localToWorld);
                if (!m_geometryBuilder.IsValid())
                    continue;
                
                // save off of the first geometry transform for rest of the geometries in the collection
                firstGeomToWorld = currentGeomToWorld;
                firstWorldToGeom = currentGeomToWorld.ValidatedInverse ();
                }

            if (m_geometryBuilder.IsValid())
                {
                Transform currentLocalToGeom;
                if (firstGeomToWorld.IsEqual(currentGeomToWorld))
                    {
                    // the first geometry or one has the same tranform
                    currentLocalToGeom = firstLocalToGeom;
                    }
                else
                    {
                    // transform geometry relative to first local
                    auto toFirstGeom = Transform::FromProduct (firstWorldToGeom, currentGeomToWorld);
                    auto toFirstLocal = Transform::FromProduct (firstGeomToLocal, toFirstGeom);
                    currentLocalToGeom = toFirstLocal.ValidatedInverse ();
                    }

                if (!currentLocalToGeom.IsIdentity())
                    {
                    // transform geometry relative to the first geometry
                    auto geomToLocal = currentLocalToGeom.ValidatedInverse ();
                    geometry->TransformInPlace (geomToLocal);

                    // transform pattern but not linestyles
                    auto& display = geomEntry.GetGeometryParamsR ();
                    display.ApplyTransform (geomToLocal, 0x01);
                    }

                m_geometryBuilder->Append (geomEntry.GetGeometryParams());
                m_geometryBuilder->Append (*geometry);

                m_importer.Progress ();
                }

            total++;
            }
        }

    if (m_geometryBuilder.IsValid())
        {
        status = this->CreateElement ();
        if (status != BSISUCCESS)
            failed++;
        }

    if (failed > 0)
        {
        Utf8PrintfString msg("%lld out of %lld individual geometry element(s) is/are not created for entity[%s, handle=%llx]!", failed, total, m_elementCode.GetValueUtf8CP(), m_inputs.GetEntityId().ToUInt64());
        m_importer.ReportError (IssueCategory::VisualFidelity(), Issue::Error(), msg.c_str());
        if (BSISUCCESS == status)
            status = BSIERROR;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementFactory::CreateEmptyElement ()
    {
    m_results.m_importedElement = m_elementHandler->Create (m_elementParams);

    auto geomSource = m_results.m_importedElement->ToGeometrySourceP ();
    if (nullptr == geomSource)
        {
        BeAssert (false && L"Null geometry source!");
        return  BSIERROR;
        }
    geomSource->SetCategoryId (m_createParams.GetCategoryId());
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementFactory::CreateElement ()
    {
    // create elements from current GeometryBuilder containing up to date geometry collection, then flush the builder.
    if (!m_geometryBuilder.IsValid() || nullptr == m_elementHandler)
        {
        BeAssert (false && L"Invalid GeometricBuilder and/or ElementHandler!");
        return  BSIERROR;
        }

#ifdef NEED_UNIQUE_CODE_PER_ELEMENT
    if (m_results.m_importedElement.IsValid())
        {
        // a parent element has been created - set element params for children
        Utf8PrintfString    codeValue("%s-%d", m_elementCode.GetValue(), m_results.m_childElements.size() + 1);
        DgnCode             childCode = m_importer.CreateCode (codeValue);
        m_elementParams.SetCode (childCode);
        }
#endif  // NEED_UNIQUE_CODE_PER_ELEMENT

    // create a new element from current geometry builder:
    DgnElementPtr   element = m_elementHandler->Create (m_elementParams);
    if (!element.IsValid())
        {
        BeAssert (false && L"Null element!");
        return  BSIERROR;
        }

    auto geomSource = element->ToGeometrySourceP ();
    if (nullptr == geomSource)
        {
        BeAssert (false && L"Null geometry source!");
        return  BSIERROR;
        }

    // category must be preset through GeometryBuilder inialization or else Finish will fail
    auto categoryId = m_geometryBuilder->GetGeometryParams().GetCategoryId ();

    // set category and save geometry source to the new element
    if (DgnDbStatus::Success != geomSource->SetCategoryId(categoryId) || BSISUCCESS != m_geometryBuilder->Finish(*geomSource))
        return  BSIERROR;

    if (!m_results.m_importedElement.IsValid())
        {
        // the first geometry builder is the parent element:
        m_results.m_importedElement = element;
        }
    else
        {
        // rest of the geomtry collection are children:
        DwgImporter::ElementImportResults    childResults(element.get());
        m_results.m_childElements.push_back (childResults);
        }

    // reset the geometry builder for next new element
    m_geometryBuilder = nullptr;

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementFactory::CreateElements (DwgImporter::T_BlockGeometryMap const* geometryMap)
    {
    if (nullptr == geometryMap)
        return  BSIERROR;

    m_geometryMap = geometryMap;
    m_geometryBuilder = nullptr;

    // if no geometry collected, return an empty element as a host element for ElementAspects etc
    size_t  num = m_geometryMap->size ();
    if (num == 0)
        return  this->CreateEmptyElement ();

    // check the inputs and set the factory to create either shared parts or individual elements
    this->SetDefaultCreation ();

    // begin creating elements from the input geometry collection
    if (m_canCreateSharedParts)
        return this->CreateSharedParts ();
    else
        return this->CreateIndividualElements ();
    }
