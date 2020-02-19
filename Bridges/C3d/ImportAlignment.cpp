/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "C3dInternal.h"

BEGIN_C3D_NAMESPACE

DWG_PROTOCOLEXT_DEFINE_MEMBERS(AeccAlignmentExt)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccAlignmentExt::_ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer)
    {
    BentleyStatus   status = BentleyStatus::BSIERROR;

    m_importer = dynamic_cast<C3dImporterP>(&importer);
    m_aeccAlignment = AECCDbAlignment::cast (context.GetEntityPtrR().get());
    if (nullptr == m_importer || nullptr == m_aeccAlignment || !context.GetModel().Is3d())
        return  status;

    m_name.Assign (reinterpret_cast<WCharCP>(m_aeccAlignment->GetName().c_str()));
    m_description.Assign (reinterpret_cast<WCharCP>(m_aeccAlignment->GetDescription().c_str()));
    m_toDgnContext = &context;
    m_baseAlignment = nullptr;
    m_alignmentModel = nullptr;
    m_verticalAlignmentModelId.Invalidate ();

    status = this->ImportAlignment ();
    if (status == BentleyStatus::BSISUCCESS)
        status = this->ImportVAlignments ();

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccAlignmentExt::ImportAlignment ()
    {
    // C3D alignment
    auto status = this->CreateOrUpdateAeccAlignment ();
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    // Civil domain horizontal alignment
    m_alignmentModel = m_importer->GetAlignmentModel ();
    if (m_alignmentModel.IsValid())
        status = this->CreateOrUpdateHorizontalAlignment ();

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AeccAlignmentExt::CreateOrUpdateAeccAlignment ()
    {
    auto ecInstance = m_importer->CreateC3dECInstance (ECCLASSNAME_AeccAlignment);
    if (!ecInstance.IsValid())
        return  BentleyStatus::BSIERROR;

    auto& results = m_toDgnContext->GetElementResultsR ();
    auto& inputs = m_toDgnContext->GetElementInputsR ();

    inputs.SetClassId (ecInstance->GetClass().GetId());
    if (!m_name.empty())
        inputs.SetElementLabel (m_name);

    // unset rendering options - they should have no impact on generating geometry for an alignment but ODA may gives no geometry!
    auto&   geometryOpts = m_importer->GetCurrentGeometryOptions ();
    auto currViewDir = geometryOpts.GetViewDirection ();
    auto currRegenType = geometryOpts.GetRegenType ();

    geometryOpts.SetViewDirection (DVec3d::From(0,0,1));
    geometryOpts.SetRegenType (DwgGiRegenType::StandardDisplay);

    auto status = m_importer->_ImportEntity (results, inputs);

    // restore current geometry settings
    geometryOpts.SetViewDirection (currViewDir);
    geometryOpts.SetRegenType (currRegenType);
    
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    auto element = results.GetImportedElement ();
    if (element != nullptr)
        {
        if (!m_description.empty())
            element->SetPropertyValue (ECPROPNAME_Description, m_description.c_str());

        element->SetPropertyValue (ECPROPNAME_ReferenceStation, m_aeccAlignment->GetReferencePointStation());
        element->SetPropertyValue (ECPROPNAME_StartStation, m_aeccAlignment->GetStartStation());
        element->SetPropertyValue (ECPROPNAME_EndStation, m_aeccAlignment->GetEndStation());

        OdGePoint2d point;
        if (m_aeccAlignment->GetReferencePoint(point))
            element->SetPropertyValue (ECPROPNAME_ReferencePoint, DPoint2d::From(point.x, point.y));

        this->SetDesignSpeedProperties (*element);
        this->SetVAlignmentProperties (*element);
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AeccAlignmentExt::SetDesignSpeedProperties (DgnElementR element)
    {
    auto count = m_aeccAlignment->GetDesignSpeedCount ();
    if (count < 1)
        return  BentleyStatus::BSISUCCESS;

    auto ecInstance = m_importer->CreateC3dECInstance (ECCLASSNAME_DesignSpeed);
    if (!ecInstance.IsValid())
        return  BentleyStatus::BSIERROR;

    auto status = m_importer->InsertArrayProperty (element, ECPROPNAME_DesignSpeeds, count);
    if (status != DgnDbStatus::Success)
        return  static_cast<BentleyStatus>(status);

    for (int32_t i = 0; i < static_cast<int32_t>(count); i++)
        {
        auto designSpeed = m_aeccAlignment->GetDesignSpeedsByIndex (i);
        if (!designSpeed.isNull())
            {
            Utf8String  comment(reinterpret_cast<WCharCP>(designSpeed->GetComment().c_str()));
            if (!comment.empty())
                ecInstance->SetValue (ECPROPNAME_Comment, ECValue(comment.c_str()));

            ecInstance->SetValue (ECPROPNAME_Station, ECValue(designSpeed->GetStation()));
            ecInstance->SetValue (ECPROPNAME_DesignSpeed, ECValue(designSpeed->GetValue()));

            ECValue ecValue(VALUEKIND_Struct);
            ecValue.SetStruct (ecInstance.get());

            status = element.SetPropertyValue(ECPROPNAME_DesignSpeeds, ecValue, PropertyArrayIndex(i));
            if (status != DgnDbStatus::Success)
                break;
            }
        }

    return  static_cast<BentleyStatus>(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AeccAlignmentExt::SetVAlignmentProperties (DgnElementR element)
    {
    uint32_t    count = m_aeccAlignment->GetVAlignmentCount ();
    if (count < 1)
        return  BentleyStatus::BSISUCCESS;

    auto ecInstance = m_importer->CreateC3dECInstance (ECCLASSNAME_VAlignment);
    if (!ecInstance.IsValid())
        return  BentleyStatus::BSIERROR;

    auto status = m_importer->InsertArrayProperty (element, ECPROPNAME_VAlignments, count);
    if (status != DgnDbStatus::Success)
        return  static_cast<BentleyStatus>(status);

    for (uint32_t i = 0; i < count; i++)
        {
        auto objectId = m_aeccAlignment->GetVAlignmentByIndex (i);
        AECCDbVAlignmentPtr aeccVAlignment = objectId.openObject (OdDb::OpenMode::kForRead);
        if (!aeccVAlignment.isNull())
            {
            Utf8String  descr(reinterpret_cast<WCharCP>(aeccVAlignment->GetDescription().c_str()));
            if (!descr.empty())
                ecInstance->SetValue (ECPROPNAME_Description, ECValue(descr.c_str()));

            ecInstance->SetValue (ECPROPNAME_StartStation, ECValue(aeccVAlignment->GetStartStation()));
            ecInstance->SetValue (ECPROPNAME_EndStation, ECValue(aeccVAlignment->GetEndStation()));
            ecInstance->SetValue (ECPROPNAME_StartOffset, ECValue(aeccVAlignment->GetStartOffset()));
            ecInstance->SetValue (ECPROPNAME_EndOffset, ECValue(aeccVAlignment->GetEndOffset()));
            ecInstance->SetValue (ECPROPNAME_SampleOffset, ECValue(aeccVAlignment->GetSampleOffset()));
            ecInstance->SetValue (ECPROPNAME_MinElevation, ECValue(aeccVAlignment->GetElevationMin()));
            ecInstance->SetValue (ECPROPNAME_MaxElevation, ECValue(aeccVAlignment->GetElevationMax()));
            ecInstance->SetValue (ECPROPNAME_Length, ECValue(aeccVAlignment->GetLength()));

            ECValue ecValue(VALUEKIND_Struct);
            ecValue.SetStruct (ecInstance.get());

            status = element.SetPropertyValue(ECPROPNAME_VAlignments, ecValue, PropertyArrayIndex(i));
            if (status != DgnDbStatus::Success)
                break;
            }
        }

    return  BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccAlignmentExt::ImportVAlignments ()
    {
    /*-----------------------------------------------------------------------------------
    This method takes over the default implementation of DwgImporter to import C3D VAlignment 
    entities attached to this alignment, i.e. open the object and detect changes.  Then
    act based on the detection results: ignore, insert or update the target element.
    Upon successfully imported a VAlignment, the imported element ID is cached for a later
    step where Civil vertical alignments are created.

    C3D VAlignments are filtered out in _FilterEntity to let DwgImporter skip them.
    -----------------------------------------------------------------------------------*/
    uint32_t    count = m_aeccAlignment->GetVAlignmentCount ();
    if (count < 1)
        return  BentleyStatus::BSISUCCESS;

    auto ecInstance = m_importer->CreateC3dECInstance (ECCLASSNAME_AeccVAlignment);
    if (!ecInstance.IsValid())
        return  BentleyStatus::BSIERROR;

    // when a change is detected, a Civil vertical alignment may also be created/updated - validate vertical alignment model
    if (m_baseAlignment.IsValid())
        {
        m_verticalAlignmentModelId = m_baseAlignment->QueryVerticalAlignmentSubModelId ();
        if (!m_verticalAlignmentModelId.IsValid())
            {
            auto verticalModel = RoadRailAlignment::VerticalAlignmentModel::Create (VerticalAlignmentModel::CreateParams(m_importer->GetDgnDb(), m_baseAlignment->GetElementId()));
            if (verticalModel->Insert() != DgnDbStatus::Success)
                return BentleyStatus::BSIERROR;
            m_verticalAlignmentModelId = verticalModel->GetModelId ();
            }
        }

    DwgImporter::ElementImportInputs    vaInputs(m_toDgnContext->GetModel());
    vaInputs.SetClassId (ecInstance->GetClass().GetId());
    vaInputs.SetTransform (m_toDgnContext->GetTransform());
    vaInputs.SetModelMapping (m_toDgnContext->GetElementInputsR().GetModelMapping());

    for (uint32_t i = 0; i < count; i++)
        {
        auto objectId = m_aeccAlignment->GetVAlignmentByIndex (i);
        if (vaInputs.GetEntityPtrR().OpenObject(objectId, DwgDbOpenMode::ForRead) != DwgDbStatus::Success)
            continue;

        DwgDbEntityP entity = vaInputs.GetEntityP ();
        if (entity == nullptr || !entity->isKindOf(AECCDbVAlignment::desc()))
            continue;

        vaInputs.SetParentEntity (entity);
        vaInputs.SetEntityId (objectId);
        this->DetectAndImportAeccVAlignment (vaInputs);
        }

    return  BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccAlignmentExt::DetectAndImportAeccVAlignment (DwgImporter::ElementImportInputs& inputs)
    {
    DwgDbObjectP    object = DwgDbObject::Cast (inputs.GetEntityP());
    if (nullptr == object)
        return  BentleyStatus::BSIERROR;

    AECCDbVAlignment* aeccVAlignment = AECCDbVAlignment::cast (object);
    if (aeccVAlignment == nullptr)
        return  BentleyStatus::BSIERROR;

    Utf8String  vaName(reinterpret_cast<WCharCP>(aeccVAlignment->GetVAlignmentName().c_str()));
    if (!vaName.empty())
        inputs.SetElementLabel (vaName);

    // detect changes in the AeccVAlignment
    IDwgChangeDetector::DetectionResults    detectionResults;
    bool changed = m_importer->GetChangeDetector()._IsElementChanged (detectionResults, *m_importer, *object, inputs.GetModelMapping());

    // prepare element results for the AeccVAlignment
    DwgImporter::ElementImportResults   elementResults;
    DwgSourceAspects::ObjectAspect::SourceData source(inputs.GetEntityId().GetHandle(), inputs.GetTargetModel().GetModelId(), detectionResults.GetCurrentProvenance(), Utf8String());
    elementResults.SetObjectSourceData (source);
    elementResults.SetExistingElement (detectionResults.GetObjectAspect());

    BentleyStatus   status = BentleyStatus::SUCCESS;
    if (changed)
        {
        status = m_importer->_ImportEntity (elementResults, inputs);
        if (status != BentleyStatus::BSISUCCESS)
            return  status;

        auto importedElement = elementResults.GetImportedElement ();
        if (importedElement != nullptr)
            {
            importedElement->SetPropertyValue ("VAlignment.StartStation", ECValue(aeccVAlignment->GetStartStation()));
            importedElement->SetPropertyValue ("VAlignment.EndStation", ECValue(aeccVAlignment->GetEndStation()));
            importedElement->SetPropertyValue ("VAlignment.StartOffset", ECValue(aeccVAlignment->GetStartOffset()));
            importedElement->SetPropertyValue ("VAlignment.EndOffset", ECValue(aeccVAlignment->GetEndOffset()));
            importedElement->SetPropertyValue ("VAlignment.SampleOffset", ECValue(aeccVAlignment->GetSampleOffset()));
            importedElement->SetPropertyValue ("VAlignment.MinElevation", ECValue(aeccVAlignment->GetElevationMin()));
            importedElement->SetPropertyValue ("VAlignment.MaxElevation", ECValue(aeccVAlignment->GetElevationMax()));
            importedElement->SetPropertyValue ("VAlignment.Length", ECValue(aeccVAlignment->GetLength()));
            }

        // create or update Civil domain vertical alignment from C3D valignment, update SourceData in elementResults as necessary
        status = this->CreateOrUpdateVerticalAlignment (elementResults, aeccVAlignment);
        }

    // process the imported AeccVAlignment element based on the results from the change detection
    status = m_importer->_ProcessDetectionResults(detectionResults, elementResults, inputs);

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccAlignmentExt::CreateOrUpdateHorizontalAlignment ()
    {
    if (m_aeccAlignment == nullptr)
        return  BentleyStatus::BSIERROR;

    auto alignType = m_aeccAlignment->GetAlignmentType ();
    if (alignType != AlignmentType::Centerline && alignType != AlignmentType::Rail)
        return  BentleyStatus::BSIERROR;

    CurveVectorPtr  curves;
    BentleyStatus   status = this->CreateCurveVectorFromImportedElements (curves);
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    // from now on, m_name will be used in Civil domain codes - make sure it's not emptry
    if (m_name.empty())
        m_name.Assign (reinterpret_cast<WCharCP>(m_aeccAlignment->isA()->name().c_str()));

    if (m_importer->GetOptions().IsUpdating())
        status = this->UpdateHorizontalAlignment (*curves.get());
    else
        status = this->CreateHorizontalAlignment (*curves.get());

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AeccAlignmentExt::CreateCurveVectorFromImportedElements (CurveVectorPtr& curves)
    {
    auto& results = m_toDgnContext->GetElementResultsR ();
    auto importedElement = results.GetImportedElement ();
    if (importedElement == nullptr)
        return  BentleyStatus::BSIERROR;

    auto geomSource = importedElement->ToGeometrySource ();
    auto toDgn = m_toDgnContext->GetTransform ();

    DwgDbPolylinePtr polyline(m_aeccAlignment->GetPolyline());
    if (!polyline.IsNull())
        curves = DwgHelper::CreateCurveVectorFrom (*polyline, CurveVector::BoundaryType::BOUNDARY_TYPE_Open, &toDgn);
    if (!curves.IsValid())
        {
        // try extracting geometry from the imported element
        curves = CurveVector::Create (CurveVector::BoundaryType::BOUNDARY_TYPE_Open);
        if (!curves.IsValid() || C3dHelper::GetLinearCurves(*curves, geomSource) != BentleyStatus::BSISUCCESS)
            return  BentleyStatus::BSIERROR;
        }

    if (polyline.IsNull() && curves.IsValid())
        {
        // the alignment geometry is not built with a single valid polyline - collect multi-segments from children into a CurveVector
        for (auto& child : results.m_childElements)
            {
            importedElement = child.GetImportedElement ();
            if (importedElement != nullptr && (geomSource = importedElement->ToGeometrySource()) != nullptr)
                {
                CurveVectorPtr  segment = CurveVector::Create (CurveVector::BoundaryType::BOUNDARY_TYPE_Open);
                if (segment.IsValid() && C3dHelper::GetLinearCurves(*segment, geomSource) == BentleyStatus::BSISUCCESS)
                    {
                    curves->AddPrimitives (*segment);
                    }
                else
                    {
                    Utf8PrintfString msg("missing a horizontal segment from AeccAlignment ID=%ls", m_aeccAlignment->objectId().getHandle().ascii().c_str());
                    m_importer->ReportIssue (DwgImporter::IssueSeverity::Info, IssueCategory::MissingData(), Issue::Message(), msg.c_str());
                    }
                }
            if (curves->size() > 1)
                curves = curves->AssembleChains ();
            }
        }

    return  (curves.IsValid() && !curves->empty()) ? BentleyStatus::BSISUCCESS : BentleyStatus::BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccAlignmentExt::CreateHorizontalAlignment (CurveVectorCR curves)
    {
    m_baseAlignment = RoadRailAlignment::Alignment::Create (*m_alignmentModel);
    if (!m_baseAlignment.IsValid() || !m_baseAlignment->Insert().IsValid())
        return  BentleyStatus::BSIERROR;

    Utf8PrintfString  codeValue("%s_%ls", m_name.c_str(), m_aeccAlignment->objectId().getHandle().ascii().c_str());
    DgnCode code = RoadRailAlignmentDomain::CreateCode(*m_alignmentModel, codeValue);
    size_t  index = 0;
    while (m_alignmentModel->GetDgnDb().Elements().QueryElementIdByCode(code).IsValid())
        {
        codeValue.Sprintf ("%s_%d", codeValue.c_str(), index++);
        code = RoadRailAlignmentDomain::CreateCode (*m_alignmentModel, codeValue);
        }

    m_baseAlignment->SetCode (code);
    m_baseAlignment->SetStartStation (m_aeccAlignment->GetStartStation());
    m_baseAlignment->Insert ();

    auto horizontalAlign = HorizontalAlignment::Create (*m_baseAlignment, curves);
    horizontalAlign->SetCode (code);
    horizontalAlign->SetUserLabel (m_name.c_str());

    // create ElementGeometry from previously converted Curvevector
    auto  status = horizontalAlign->GenerateElementGeom ();
    if (status != DgnDbStatus::Success)
        return  static_cast<BentleyStatus>(status);

    // insert the new element to db, and add the element id as a json property in the provenance
    auto inserted = horizontalAlign->Insert ();
    if (inserted.IsValid())
        {
        auto& results = m_toDgnContext->GetElementResultsR ();
        C3dHelper::AddCivilReferenceElementId (results, inserted->GetElementId(), m_baseAlignment->GetElementId());
        }

    return inserted.IsValid() ? BentleyStatus::BSISUCCESS : BentleyStatus::BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccAlignmentExt::UpdateHorizontalAlignment (CurveVectorCR curves)
    {
    // retrieve the Civil reference elementId for the imported element.  If this is the first segment, also retrieve the base Civil alignmentId.
    DgnElementId    baseAlignmentId;
    auto& results = m_toDgnContext->GetElementResultsR ();
    auto existingId = C3dHelper::GetCivilReferenceElementId (results, &baseAlignmentId);
    if (!existingId.IsValid() || !baseAlignmentId.IsValid())
        return  this->CreateHorizontalAlignment(curves);

    HorizontalAlignmentPtr horizontalAlign = HorizontalAlignment::GetForEdit (m_importer->GetDgnDb(), existingId);
    if (!horizontalAlign.IsValid())
        return  BentleyStatus::BSIERROR;

    if (!m_baseAlignment.IsValid())
        {
        m_baseAlignment = RoadRailAlignment::Alignment::GetForEdit (m_importer->GetDgnDb(), baseAlignmentId);
        if (!m_baseAlignment.IsValid())
            return  BentleyStatus::BSIERROR;

        m_baseAlignment->SetStartStation (m_aeccAlignment->GetStartStation());
        m_baseAlignment->Update ();
        }

    horizontalAlign->SetGeometry (curves);

    BentleyStatus   status = static_cast<BentleyStatus> (horizontalAlign->GenerateElementGeom());
    if (status == BentleyStatus::BSISUCCESS)
        {
        DgnDbStatus dbStatus = DgnDbStatus::Success;

        horizontalAlign->Update (&dbStatus);

        status = static_cast<BentleyStatus>(dbStatus);
        }
    
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccAlignmentExt::CreateOrUpdateVerticalAlignment (DwgImporter::ElementImportResultsR results, AECCDbVAlignment* aeccVAlignment)
    {
    // Civil VerticalAlignment seems to require geoemtry whereas C3D's does not?
    if (!m_baseAlignment.IsValid() || !m_verticalAlignmentModelId.IsValid() || aeccVAlignment == nullptr)
        return  BentleyStatus::BSISUCCESS;

    auto importedElement = results.GetImportedElement ();
    if (importedElement == nullptr)
        return  BentleyStatus::BSISUCCESS;

    CurveVector curves(CurveVector::BoundaryType::BOUNDARY_TYPE_None);
    if (C3dHelper::GetLinearCurves(curves, importedElement->ToGeometrySource()) != BentleyStatus::BSISUCCESS)
        return  BentleyStatus::BSISUCCESS;

    if (m_importer->GetOptions().IsUpdating())
        return this->UpdateVerticalAlignment (curves, results);
    else
        return this->CreateVerticalAlignment (curves, results);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccAlignmentExt::CreateVerticalAlignment (CurveVectorCR curves, DwgImporter::ElementImportResultsR results)
    {
    auto verticalAlignment = VerticalAlignment::Create (*m_baseAlignment, curves);
    if (!verticalAlignment.IsValid())
        return  BentleyStatus::BSIERROR;

    auto status = verticalAlignment->GenerateElementGeom ();
    if (status == DgnDbStatus::Success)
        {
        // insert the new element to db
        auto inserted = verticalAlignment->Insert (&status);
        // add element id as a json property in the provenance
        if (inserted.IsValid())
            C3dHelper::AddCivilReferenceElementId (results, inserted->GetElementId());
        }

    return static_cast<BentleyStatus>(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccAlignmentExt::UpdateVerticalAlignment (CurveVectorCR curves, DwgImporter::ElementImportResultsR results)
    {
    auto existingId = C3dHelper::GetCivilReferenceElementId (results);
    if (!existingId.IsValid())
        return  this->CreateVerticalAlignment(curves, results);

    VerticalAlignmentPtr verticalAlignment = VerticalAlignment::GetForEdit (m_importer->GetDgnDb(), existingId);
    if (!verticalAlignment.IsValid())
        return  BentleyStatus::BSIERROR;

    verticalAlignment->SetGeometry (curves);

    DgnDbStatus status = DgnDbStatus::Success;
    verticalAlignment->Update (&status);

    return  static_cast<BentleyStatus>(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/20
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccAlignmentExt::UpdateElementRepresentedBy (DgnDbR db, DgnElementId civilAlignmentId, DgnElementId aeccAlignmentId)
    {
    if (!civilAlignmentId.IsValid() || !aeccAlignmentId.IsValid())
        return  BentleyStatus::BSIERROR;

    auto civilAlignmentElement = RoadRailAlignment::Alignment::Get (db, civilAlignmentId);
    auto aeccAlignmentElement = db.Elements().Get<GeometricElement3d> (aeccAlignmentId);
    if (!civilAlignmentElement.IsValid() || !aeccAlignmentElement.IsValid())
        return  BentleyStatus::BSIERROR;

    auto geomSource = aeccAlignmentElement->ToGeometrySource ();
    if (geomSource == nullptr)
        return  BentleyStatus::BSIERROR;
    
    if (!civilAlignmentElement->QueryIsRepresentedBy(*geomSource))
        RoadRailAlignment::Alignment::AddRepresentedBy (*civilAlignmentElement, *geomSource);

    return  BentleyStatus::BSISUCCESS;
    }

END_C3D_NAMESPACE
