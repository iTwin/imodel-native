/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "C3dImporter.h"
#include "C3dHelper.h"

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
    m_importedVAlignmentMap.clear ();

    // C3D elements
    status = this->CreateOrUpdateAeccAlignment ();
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    this->CreateOrUpdateAeccVAlignments ();

    m_alignmentModel = m_importer->GetAlignmentModel ();
    if (!m_alignmentModel.IsValid())
        return  status;

    // Civil domain elements
    status = this->CreateOrUpdateHorizontalAlignment ();
    if (status == BentleyStatus::BSISUCCESS)
        this->CreateOrUpdateVerticalAlignments ();

    return  BentleyStatus::BSISUCCESS;
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

    auto status = m_importer->_ImportEntity (results, inputs);
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    auto element = results.GetImportedElement ();
    if (element != nullptr)
        {
        if (!m_name.empty())
            element->SetUserLabel (m_name.c_str());
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
BentleyStatus AeccAlignmentExt::CreateOrUpdateAeccVAlignments ()
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

    IDwgChangeDetector::DetectionResults    detectionResults;
    DwgImporter::ElementImportResults   elementResults;
    BentleyStatus   status = BentleyStatus::SUCCESS;

    if (m_importer->GetChangeDetector()._IsElementChanged(detectionResults, *m_importer, *object, inputs.GetModelMapping()))
        {
        elementResults.SetExistingElement (detectionResults.GetObjectAspect());

        auto status = m_importer->_ImportEntity (elementResults, inputs);
        if (status != BentleyStatus::BSISUCCESS)
            return  status;

        auto importedElement = elementResults.GetImportedElement ();
        if (importedElement != nullptr)
            {
            if (!m_name.empty())
                importedElement->SetUserLabel (m_name.c_str());

            importedElement->SetPropertyValue ("VAlignment.StartStation", ECValue(aeccVAlignment->GetStartStation()));
            importedElement->SetPropertyValue ("VAlignment.EndStation", ECValue(aeccVAlignment->GetEndStation()));
            importedElement->SetPropertyValue ("VAlignment.StartOffset", ECValue(aeccVAlignment->GetStartOffset()));
            importedElement->SetPropertyValue ("VAlignment.EndOffset", ECValue(aeccVAlignment->GetEndOffset()));
            importedElement->SetPropertyValue ("VAlignment.SampleOffset", ECValue(aeccVAlignment->GetSampleOffset()));
            importedElement->SetPropertyValue ("VAlignment.MinElevation", ECValue(aeccVAlignment->GetElevationMin()));
            importedElement->SetPropertyValue ("VAlignment.MaxElevation", ECValue(aeccVAlignment->GetElevationMax()));
            importedElement->SetPropertyValue ("VAlignment.Length", ECValue(aeccVAlignment->GetLength()));
            }
        }

    status = m_importer->ProcessDetectionResults(detectionResults, elementResults, inputs);
    if (status == BentleyStatus::BSISUCCESS)
        {
        bpair<DwgDbObjectId,DgnElementId> entry(object->GetObjectId(), elementResults.GetImportedElementId());
        m_importedVAlignmentMap.insert (entry);
        }

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

    auto importedElement = m_toDgnContext->GetElementResultsR().GetImportedElement ();
    if (importedElement == nullptr)
        return  BentleyStatus::BSIERROR;

    auto toDgn = m_toDgnContext->GetTransform ();
    CurveVectorPtr  curves;
    DwgDbPolylinePtr polyline(m_aeccAlignment->GetPolyline());
    if (!polyline.IsNull())
        curves = DwgHelper::CreateCurveVectorFrom (*polyline, CurveVector::BoundaryType::BOUNDARY_TYPE_Open, &toDgn);
    if (!curves.IsValid())
        {
        // try extracting geometry from the imported element
        curves = CurveVector::Create (CurveVector::BoundaryType::BOUNDARY_TYPE_None);
        if (!curves.IsValid() || C3dHelper::GetLinearCurves(*curves, importedElement->ToGeometrySource()) != BentleyStatus::BSISUCCESS)
            return  BentleyStatus::BSIERROR;
        }

    auto alignment = Alignment::Create (*m_alignmentModel);
    if (!alignment.IsValid() || !alignment->Insert().IsValid())
        return  BentleyStatus::BSIERROR;

    m_baseAlignmentId = alignment->GetElementId ();
    
    alignment->SetCode (RoadRailAlignmentDomain::CreateCode(*m_alignmentModel, m_name));
    alignment->SetStartStation (m_aeccAlignment->GetStartStation());

    auto horizontalAlign = HorizontalAlignment::Create (*alignment, *curves);
    horizontalAlign->SetCode (RoadRailAlignmentDomain::CreateCode(*horizontalAlign->GetModel(), m_name));
    horizontalAlign->SetUserLabel (m_name.c_str());

    // copy GeometrySource from the imported element
    auto status = C3dHelper::CopyGeometrySource (horizontalAlign->getP(), importedElement->ToGeometrySource());
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    auto inserted = horizontalAlign->Insert ();

    return inserted.IsValid() ? BentleyStatus::BSISUCCESS : BentleyStatus::BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccAlignmentExt::CreateOrUpdateVerticalAlignments ()
    {
    // Civil VerticalAlignment seems to require geoemtry whereas C3D's does not?
    if (!m_baseAlignmentId.IsValid() || m_importedVAlignmentMap.empty())
        return  BentleyStatus::BSISUCCESS;

    auto& db = m_importer->GetDgnDb ();
    auto baseAlignment = RoadRailAlignment::Alignment::Get (db, m_baseAlignmentId);
    if (!baseAlignment.IsValid())
        return  BentleyStatus::BSIERROR;
        
    auto verticalModelId = baseAlignment->QueryVerticalAlignmentSubModelId ();
    if (!verticalModelId.IsValid())
        {
        auto verticalModel = RoadRailAlignment::VerticalAlignmentModel::Create (VerticalAlignmentModel::CreateParams(db, m_baseAlignmentId));
        if (verticalModel->Insert() != DgnDbStatus::Success)
            return BentleyStatus::BSIERROR;
        verticalModelId = verticalModel->GetModelId ();
        }

    BentleyStatus   status = BentleyStatus::BSISUCCESS;
    uint32_t    count = m_aeccAlignment->GetVAlignmentCount ();

    for (uint32_t i = 0; i < count; i++)
        {
        AECCDbVAlignmentPtr aeccVAlignment = m_aeccAlignment->GetVAlignmentByIndex(i).openObject (OdDb::OpenMode::kForRead);
        if (aeccVAlignment.isNull())
            continue;

        auto found = m_importedVAlignmentMap.find (aeccVAlignment->objectId());
        if (found == m_importedVAlignmentMap.end())
            continue;

        auto importedElement = m_importer->GetDgnDb().Elements().Get<GeometricElement> (found->second);
        if (!importedElement.IsValid())
            continue;

        CurveVector curves(CurveVector::BoundaryType::BOUNDARY_TYPE_None);
        if (C3dHelper::GetLinearCurves(curves, importedElement->ToGeometrySource()) != BentleyStatus::BSISUCCESS)
            continue;

        auto verticalAlignment = RoadRailAlignment::VerticalAlignment::Create (*baseAlignment, curves);
        if (verticalAlignment.IsValid() && verticalAlignment->GenerateElementGeom() == DgnDbStatus::Success)
            {
            DgnDbStatus dbStatus = DgnDbStatus::Success;

            auto inserted = verticalAlignment->Insert (&dbStatus);
            if (dbStatus != DgnDbStatus::Success)
                status = static_cast<BentleyStatus>(dbStatus);
            }
        }
    return  BentleyStatus::BSISUCCESS;
    }

END_C3D_NAMESPACE
