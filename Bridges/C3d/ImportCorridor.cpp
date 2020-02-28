/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "C3dInternal.h"

BEGIN_C3D_NAMESPACE

DWG_PROTOCOLEXT_DEFINE_MEMBERS(AeccCorridorExt)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccCorridorExt::_ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer)
    {
    BentleyStatus   status = BentleyStatus::BSIERROR;

    m_importer = dynamic_cast<C3dImporterP>(&importer);
    m_aeccCorridor = AECCDbCorridor::cast (context.GetEntityPtrR().get());
    if (nullptr == m_importer || nullptr == m_aeccCorridor || !context.GetModel().Is3d())
        return  status;

    m_name.Assign (reinterpret_cast<WCharCP>(m_aeccCorridor->GetName().c_str()));
    m_description.Assign (reinterpret_cast<WCharCP>(m_aeccCorridor->GetDescription().c_str()));
    m_toDgnContext = &context;
    m_importedElement = nullptr;
    m_parametersInstance = nullptr;
    m_baseAlignmentId.Invalidate ();

    this->FindCandidateAeccAlignment ();

    status = this->ImportCorridor ();
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    return  BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccCorridorExt::ImportCorridor ()
    {
    auto corridorClass =  m_importer->GetC3dECClass (ECCLASSNAME_AeccCorridor);
    if (corridorClass == nullptr)
        return  BentleyStatus::BSIERROR;

    m_c3dCorridorInstance = corridorClass->GetDefaultStandaloneEnabler()->CreateInstance ();
    if (!m_c3dCorridorInstance.IsValid())
        return  BentleyStatus::BSIERROR;

    auto& results = m_toDgnContext->GetElementResultsR ();
    auto& inputs = m_toDgnContext->GetElementInputsR ();

    // re-target the corridor element to an appropriate network model
    if (m_aeccAlignment->GetAlignmentType() == AlignmentType::Rail)
        m_networkModel = m_importer->GetRailNetworkModel ();
    else
        m_networkModel = m_importer->GetRoadNetworkModel ();
    if (!m_networkModel.IsValid())
        return  BentleyStatus::BSIERROR;

    DwgImporter::ElementImportInputs retargetedInputs(*m_networkModel);
    retargetedInputs.SetClassId (inputs.GetClassId());
    retargetedInputs.SetElementLabel (m_name.empty() ? inputs.GetElementLabel() : m_name);
    retargetedInputs.SetTransform (inputs.GetTransform());
    retargetedInputs.SetModelMapping (inputs.GetModelMapping());
    retargetedInputs.SetEntityId (inputs.GetEntityId());

    // need to add a refCounted within the scope of this method:
    retargetedInputs.m_entity = inputs.m_entity;

    // now ready to have base bridge create geometry - will callback to CreateElement and UpdateElement to handle the the Civil domain element
    auto status = m_importer->_ImportEntity (results, retargetedInputs);
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    // we are back with the corridor element created as imported elements in results - add AeccCorridor properties as multi-aspects
    m_importedElement = results.GetImportedElement ();
    if (m_importedElement != nullptr)
        {
        if (!m_description.empty())
            m_c3dCorridorInstance->SetValue (ECPROPNAME_Description, ECValue(m_description.c_str()));

        // Parameters
        this->ProcessBaselines ();

        // Features
        this->ProcessFeatureStyles ();

        // Codes
        this->ProcessCodes ();

        // set the ECInstance as a multi-aspect to the element
        ECValue corridorValue(VALUEKIND_Struct);
        corridorValue.SetStruct (m_c3dCorridorInstance.get());

        auto dbStatus = DgnElement::GenericMultiAspect::AddAspect (*m_importedElement, *m_c3dCorridorInstance.get());
        status = static_cast<BentleyStatus>(dbStatus);
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccCorridorExt::ProcessBaselines ()
    {
    uint32_t count = m_aeccCorridor->GetBaselineCount ();
    if (count < 1)
        return  BentleyStatus::BSISUCCESS;

    m_parametersInstance = m_importer->CreateC3dECInstance (ECCLASSNAME_CorridorParameters);
    if (!m_parametersInstance.IsValid())
        return  BentleyStatus::BSIERROR;

    auto status = m_c3dCorridorInstance->InsertArrayElements (ECPROPNAME_CorridorParameters, 0, count);
    if (status != ECObjectsStatus::Success)
        return  static_cast<BentleyStatus>(status);

    for (uint32_t i = 0; i < count; i++)
        {
        auto baseline = m_aeccCorridor->GetBaselineByIndex (i);
        if (!baseline.isNull())
            {
            if (this->ProcessBaseline(*baseline) == BentleyStatus::BSISUCCESS)
                {
                ECValue paramsValue(VALUEKIND_Struct);
                paramsValue.SetStruct (m_parametersInstance.get());

                status = m_c3dCorridorInstance->SetValue (ECPROPNAME_CorridorParameters, paramsValue, i);
                if (status != ECObjectsStatus::Success)
                    break;
                }
            }
        }

    return  BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccCorridorExt::ProcessBaseline (AECCCorridorBaseline const& baseline)
    {
    if (m_parametersInstance.IsNull())
        return  BentleyStatus::BSIERROR;

    // WIP - get baseline name and description from the Civil toolkit!!
    Utf8String  name;
    Utf8String  desc;

    AECCDbAlignmentPtr  aeccAlignment = baseline.GetAlignment().openObject (OdDb::OpenMode::kForRead);
    if (!aeccAlignment.isNull())
        {
        name.Assign (reinterpret_cast<WCharCP>(aeccAlignment->GetName().c_str()));
        if (name.empty())
            name.assign ("Baseline");
        m_parametersInstance->SetValue (ECPROPNAME_HorizontalAlignment, ECValue(name.c_str()));
        }

    AECCDbVAlignmentPtr aeccVAlignment = baseline.GetVAlignment().openObject (OdDb::OpenMode::kForRead);
    if (!aeccVAlignment.isNull())
        {
        name.Assign (reinterpret_cast<WCharCP>(aeccVAlignment->GetVAlignmentName().c_str()));
        if (name.empty())
            name.Assign (reinterpret_cast<WCharCP>(aeccVAlignment->GetName().c_str()));
        if (!name.empty())
            m_parametersInstance->SetValue (ECPROPNAME_VerticalAlignment, ECValue(name.c_str()));
        }

    // Regions
    if (this->ProcessRegions(baseline) != BentleyStatus::BSISUCCESS)
        return  BentleyStatus::BSIERROR;

#ifdef FEATURE_COLLECTIONS
    if (this->ProcessFeatureCollections(baseline) != BentleyStatus::BSISUCCESS)
        return  BentleyStatus::BSIERROR;
#endif
    
    return  BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccCorridorExt::ProcessRegions (AECCCorridorBaseline const& baseline)
    {
    // regions are nested in the parameters
    if (m_parametersInstance.IsNull())
        return  BentleyStatus::BSIERROR;

    uint32_t    count = baseline.GetRegionCount ();
    if (count < 1 || m_parametersInstance == nullptr)
        return  BentleyStatus::BSISUCCESS;

    auto status = m_parametersInstance->InsertArrayElements (ECPROPNAME_Regions, 0, count);
    if (status != ECObjectsStatus::Success)
        return  static_cast<BentleyStatus>(status);

    ECValue regionValue(VALUEKIND_Struct);
    for (uint32_t i = 0; i < count; i++)
        {
        auto region = baseline.GetRegionByIndex (i);
        if (!region.isNull())
            {
            auto regionInstance = m_importer->CreateC3dECInstance (ECCLASSNAME_CorridorRegion);
            if (!regionInstance.IsValid())
                return  BentleyStatus::BSIERROR;

            regionInstance->SetValue (ECPROPNAME_StartStation, ECValue(region->GetStartStation()));
            regionInstance->SetValue (ECPROPNAME_EndStation, ECValue(region->GetEndStation()));

            regionValue.SetStruct (regionInstance.get());
            status = m_parametersInstance->SetValue (ECPROPNAME_Regions, regionValue, i);

            if (status != ECObjectsStatus::Success)
                break;
            }
        }

    return  static_cast<BentleyStatus>(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccCorridorExt::ProcessFeatureStyles ()
    {
    uint32_t count = m_aeccCorridor->GetFeatureStyleCount ();
    if (count < 1)
        return  BentleyStatus::BSISUCCESS;

    auto status = m_c3dCorridorInstance->InsertArrayElements (ECPROPNAME_CorridorFeatures, 0, count);
    if (status != ECObjectsStatus::Success)
        return  static_cast<BentleyStatus>(status);
    
    for (uint32_t i = 0; i < count; i++)
        {
        auto featureStyle = m_aeccCorridor->GetFeatureStyleByIndex (i);
        if (!featureStyle.isNull())
            {
            auto featureInstance = m_importer->CreateC3dECInstance (ECCLASSNAME_CorridorFeature);
            if (!featureInstance.IsValid())
                return  BentleyStatus::BSIERROR;
            
            Utf8String  code(reinterpret_cast<WCharCP>(featureStyle->GetName().c_str()));
            if (!code.empty())
                featureInstance->SetValue (ECPROPNAME_Code, ECValue(code.c_str()));

            Utf8String  name, descr;
            AECCDbFeatureLineStylePtr   featurelineStyle = featureStyle->GetFeatureLineStyle().openObject (OdDb::OpenMode::kForRead);
            if (!featurelineStyle.isNull())
                {
                name.Assign (reinterpret_cast<WCharCP>(featurelineStyle->GetName().c_str()));
                if (!name.empty())
                    featureInstance->SetValue (ECPROPNAME_FeatureLineStyle, ECValue(name.c_str()));

                descr.Assign (reinterpret_cast<WCharCP>(featurelineStyle->GetDescription().c_str()));
                if (!descr.empty())
                    featureInstance->SetValue (ECPROPNAME_Description, ECValue(descr.c_str()));
                }
#ifdef DUMP_AECC_PROPERTIES
            LOG.debugv ("Feature[%d]=%s, %s, %s", i, code.c_str(), name.c_str(), descr.c_str());
#endif

            ECValue featureValue(VALUEKIND_Struct);
            featureValue.SetStruct (featureInstance.get());

            status = m_c3dCorridorInstance->SetValue (ECPROPNAME_CorridorFeatures, featureValue, i);
            if (status != ECObjectsStatus::Success)
                break;
            }
        }

    return  BentleyStatus::BSISUCCESS;
    }

#ifdef FEATURE_COLLECTIONS
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccCorridorExt::ProcessFeatureCollections (AECCCorridorBaseline const& baseline)
    {
    uint32_t    collectionCount = baseline.GetFeatureCollectionCount ();
    if (collectionCount < 1)
        return  BentleyStatus::BSISUCCESS;

    // count features for array size
    uint32_t propertyCount = 0;
    for (uint32_t i = 0; i < collectionCount; i++)
        {
        auto featureCollection = baseline.GetFeatureCollectionByIndex (i);
        if (!featureCollection.isNull())
            propertyCount += featureCollection->GetItemCount ();
        }

    // insert string property array - WIP change to struct array when Civil toolkit can extract other values
    auto status = m_importer->InsertArrayProperty (*m_importedElement, ECPROPNAME_CorridorFeatures, propertyCount);
    if (status != DgnDbStatus::Success)
        return  static_cast<BentleyStatus>(status);

    // set struct property one at a time
    propertyCount = 0;
    for (uint32_t i = 0; i < collectionCount; i++)
        {
        uint32_t itemCount = 0;
        auto featureCollection = baseline.GetFeatureCollectionByIndex (i);
        if (featureCollection.isNull() || (itemCount = featureCollection->GetItemCount()) < 1)
            continue;

        for (uint32_t j = 0; j < itemCount; j++)
            {
            auto featureItem = featureCollection->GetItemByIndex (j);
            if (!featureItem.isNull())
                {
                Utf8String  styleName (reinterpret_cast<WCharCP>(featureItem->GetFeatureStyleName().c_str()));
                if (!styleName.empty())
                    {
                    ECValue featureValue(VALUEKIND_Primitive);
                    featureValue.SetUtf8CP (styleName.c_str());

                    status = m_importedElement->SetPropertyValue (ECPROPNAME_CorridorFeatures, featureValue, PropertyArrayIndex(propertyCount));
                    if (status != DgnDbStatus::Success)
                        break;
                    }
                LOG.debugv ("GetFeatureStyleName[%d][%d]=%s", i, j, styleName.empty() ? "<empty>" : styleName.c_str());
                }
            propertyCount++;
            }
        }

    return  static_cast<BentleyStatus>(status);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccCorridorExt::ProcessCodes ()
    {
    if (m_aeccCorridor == nullptr)
        return  BentleyStatus::BSIERROR;

    AECCDbRoadwayStyleSetPtr    roadwayStyleSet = m_aeccCorridor->GetRoadwayStyleSet().openObject (OdDb::OpenMode::kForRead);
    if (roadwayStyleSet.isNull())
        return  BentleyStatus::BSIERROR;

    // Link codes
    uint32_t count = roadwayStyleSet->GetCustomLinkCount ();
    if (count > 1)
        {
        auto status = m_c3dCorridorInstance->InsertArrayElements (ECPROPNAME_LinkCodes, 0, count);
        if (status != ECObjectsStatus::Success)
            return  static_cast<BentleyStatus>(status);

        for (uint32_t i = 0; i < count; i++)
            {
            auto code = roadwayStyleSet->GetCustomLinkCode (i);
            AECCSubassemblyEntTraitsSubPtr linkCustom = roadwayStyleSet->GetLinkCustom (code);
            if (!linkCustom.isNull())
                {
                status = this->ProcessCode (code, *linkCustom, ECPROPNAME_LinkCodes, i);
                if (status != ECObjectsStatus::Success)
                    return  static_cast<BentleyStatus>(status);
                }
            }
        }
    
    // Point codes
    count = roadwayStyleSet->GetCustomPointCount ();
    if (count > 1)
        {
        auto status = m_c3dCorridorInstance->InsertArrayElements (ECPROPNAME_PointCodes, 0, count);
        if (status != ECObjectsStatus::Success)
            return  static_cast<BentleyStatus>(status);

        for (uint32_t i = 0; i < count; i++)
            {
            auto code = roadwayStyleSet->GetCustomPointCode (i);
            AECCSubassemblyEntTraitsSubPtr pointCustom = roadwayStyleSet->GetPointCustom (code);
            if (!pointCustom.isNull())
                {
                status = this->ProcessCode (code, *pointCustom, ECPROPNAME_PointCodes, i);
                if (status != ECObjectsStatus::Success)
                    return  static_cast<BentleyStatus>(status);
                }
            }
        }
    
    // Shape codes
    count = roadwayStyleSet->GetCustomShapeCount ();
    if (count > 1)
        {
        auto status = m_c3dCorridorInstance->InsertArrayElements (ECPROPNAME_ShapeCodes, 0, count);
        if (status != ECObjectsStatus::Success)
            return  static_cast<BentleyStatus>(status);

        for (uint32_t i = 0; i < count; i++)
            {
            auto code = roadwayStyleSet->GetCustomShapeCode (i);
            AECCSubassemblyEntTraitsSubPtr shapeCustom = roadwayStyleSet->GetShapeCustom (code);
            if (!shapeCustom.isNull())
                {
                status = this->ProcessCode (code, *shapeCustom, ECPROPNAME_ShapeCodes, i);
                if (status != ECObjectsStatus::Success)
                    return  static_cast<BentleyStatus>(status);
                }
            }
        }
    
    return  BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AeccCorridorExt::ProcessCode (OdString const& code, AECCSubassemblyEntTraits const& subassentTraits, Utf8StringCR propName, uint32_t index)
    {
    auto codeInstance = m_importer->CreateC3dECInstance (ECCLASSNAME_CorridorCode);
    if (!codeInstance.IsValid())
        return  ECObjectsStatus::Error;

    // Code
    Utf8String  codeString(reinterpret_cast<WCharCP>(code.c_str()));
    if (!codeString.empty())
        codeInstance->SetValue (ECPROPNAME_Code, ECValue(codeString.c_str()));

    // Description
    Utf8String  descr(reinterpret_cast<WCharCP>(subassentTraits.GetDescription().c_str()));
    if (!descr.empty())
        codeInstance->SetValue (ECPROPNAME_Description, ECValue(descr.c_str()));

    // Style
    Utf8String  styleName, flstyleName;
    AECCDbStylePtr  style = subassentTraits.GetStyle().openObject (OdDb::OpenMode::kForRead);
    if (!style.isNull())
        {
        styleName.Assign (reinterpret_cast<WCharCP>(style->GetName().c_str()));
        if (!styleName.empty())
            codeInstance->SetValue (ECPROPNAME_Style, ECValue(styleName.c_str()));
        }

    // Feature Line Style
    AECCDbFeatureLineStylePtr   featurelineStyle = subassentTraits.GetFeatureLineStyle().openObject (OdDb::OpenMode::kForRead);
    if (!featurelineStyle.isNull())
        {
        flstyleName.Assign (reinterpret_cast<WCharCP>(featurelineStyle->GetName().c_str()));
        if (!flstyleName.empty())
            codeInstance->SetValue (ECPROPNAME_FeatureLineStyle, ECValue(flstyleName.c_str()));
        }

#ifdef DUMP_AECC_PROPERTIES
    LOG.debugv ("%s: %s, %s, %s, %s", propName.c_str(), codeString.c_str(), descr.c_str(), styleName.c_str(), flstyleName.c_str());
#endif

    ECValue codeValue(VALUEKIND_Struct);
    codeValue.SetStruct (codeInstance.get());

    return  m_c3dCorridorInstance->SetValue(propName.c_str(), codeValue, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/20
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr   AeccCorridorExt::CreateElement (DgnElement::CreateParams& params, DwgImporter::ElementImportInputsR inputs, size_t elementIndex)
    {
    /*-----------------------------------------------------------------------------------
    This method is called back from ElementFactory for both the header and child elements 
    as a result of above call to m_importer->_ImportEntity.  It only creates and returns 
    a Civil corridor element.  Leave the insertion or update to the base bridge, which will
    also insert or update ExternalSourceAspect.  For dependents and relationships that
    require the corridor to be inserted, next method UpdateElement will be called upon
    detection results have been processed.
    -----------------------------------------------------------------------------------*/
    if (m_aeccCorridor == nullptr || m_importer == nullptr || m_toDgnContext == nullptr || !m_networkModel.IsValid())
        {
        BeAssert (false && "An uninitialized AeccCorridorExt is not expected at this step!");
        return  nullptr;
        }

    if (elementIndex > 0 || m_aeccAlignment.isNull())
        {
        // we are either creating a child element, or the corridor has no alignment - fallback to the base bridge to create a default SpatialObject
        if (!m_name.empty())
            inputs.SetElementLabel (m_name);
        return  nullptr;
        }
    
    // we are creating a header element - try to create a Civil corridor in a re-targeted model
    auto& db = m_importer->GetDgnDb ();

    // AeccCorridors are post-imported, so when we reached here, all alignments should have been imported in the C3D model (not the re-targeted corridor model)
    DwgDbHandle entityHandle = m_aeccAlignment->objectId().getHandle ();
    auto aspect = m_importer->GetSourceAspects().FindObjectAspect (entityHandle, m_toDgnContext->GetElementInputsR().GetTargetModelR());
    if (!aspect.IsValid())
        return  nullptr;

    // extract the Civil alignment from the imported C3D element
    auto horizontalId = C3dHelper::GetCivilReferenceElementId (aspect, &m_baseAlignmentId);
    auto baseAlignment = RoadRailAlignment::Alignment::Get (db, m_baseAlignmentId);
    if (!baseAlignment.IsValid())
        return  nullptr;

    double totalLength = baseAlignment->GetLength ();

    // create either a Rail or a Road network
    RoadRailPhysical::TransportationNetworkCPtr network;
    if (m_aeccAlignment->GetAlignmentType() == AlignmentType::Rail)
        network = RailPhysical::RailNetwork::Get (db, m_networkModel->GetModeledElementId());
    else
        network = RoadPhysical::RoadNetwork::Get (db, m_networkModel->GetModeledElementId());
    if (!network.IsValid() || network->get() == nullptr)
        return  nullptr;

    RoadRailPhysical::Corridor::CreateFromToParams fromTo(*baseAlignment, LinearReferencing::DistanceExpression(0.0), LinearReferencing::DistanceExpression(totalLength));
    m_corridorElement = RoadRailPhysical::Corridor::Create (*network, fromTo);

    if (m_networkModel->IsPrivate())
        m_networkModel->SetIsPrivate (false);

    return m_corridorElement.IsValid() ? m_corridorElement->getP() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/20
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AeccCorridorExt::UpdateElement (DwgImporter::ElementImportResultsR results)
    {
    /*-----------------------------------------------------------------------------------
    This method is called after the change detection results have been successfully processed 
    by the base bridge, such that the corridor is now a database resident.
    -----------------------------------------------------------------------------------*/
    if (m_aeccCorridor == nullptr || m_importer == nullptr || m_toDgnContext == nullptr || !m_baseAlignmentId.IsValid() || !m_corridorElement.IsValid() || m_aeccAlignment.isNull())
        {
        BeAssert (false && "An uninitialized AeccCorridorExt is not expected at this step!");
        return  BentleyStatus::BSIERROR;
        }

    auto& db = m_importer->GetDgnDb ();
    if (!m_importer->IsUpdating())
        {
        // insert relationship
        auto status = m_corridorElement->InsertLinearElementRelationship ();
        if (status == DgnDbStatus::Success)
            m_corridorElement->Update (&status);
        if (status != DgnDbStatus::Success)
            return  static_cast<BentleyStatus>(status);

        // create a new model
        auto corridorModel = PhysicalModel::Create (*m_corridorElement->get());
        if (corridorModel.IsValid())
            {
            // copy unit formats
            auto rootModel = m_importer->GetRootModel().GetModel ();
            if (rootModel != nullptr)
                {
                auto c3dModel = rootModel->ToGeometricModelP ();
                if (c3dModel != nullptr)
                    corridorModel->GetFormatterR() = c3dModel->GetFormatter ();
                }
            corridorModel->Insert ();
            }
        }

    auto baseAlignment = RoadRailAlignment::Alignment::GetForEdit (db, m_baseAlignmentId);
    if (!baseAlignment.IsValid())
        return  BentleyStatus::BSIERROR;

    // set the corridor as an ILinearElementSource to the alignment
    baseAlignment->SetSource (m_corridorElement.get());
    baseAlignment->Update ();

    // get or set transforportation system
    auto existingId = RoadRailPhysical::TransportationSystem::QueryId (*m_corridorElement, TRANSPORTATIONSYSTEM_NAME);
    RoadRailPhysical::TransportationSystemCPtr  transportation;
    if (existingId.IsValid())
        transportation = RoadRailPhysical::TransportationSystem::Get (db, existingId);
    else
        transportation = RoadRailPhysical::TransportationSystem::Insert (*m_corridorElement, TRANSPORTATIONSYSTEM_NAME);
    if (!transportation.IsValid() || transportation->get() == nullptr)
        return  BentleyStatus::BSIERROR;

    if (m_importer->GetC3dOptions().IsAlignedModelPrivate())
        {
        transportation->GetTransportationSystemModel()->SetIsPrivate (true);
        transportation->GetTransportationSystemModel()->Update();
        }

    // get or set a corridor portion
    RoadRailPhysical::CorridorPortionElementCPtr corridorPortion;
    auto existingIds = transportation->QueryCorridorPortionIds ();
    if (!existingIds.empty())
        corridorPortion = RoadRailPhysical::CorridorPortionElement::Get (db, *existingIds.begin());
    if (!corridorPortion.IsValid() || corridorPortion->get() == nullptr)
        {
        if (m_aeccAlignment->GetAlignmentType() == AlignmentType::Rail)
            {
            auto rail = RailPhysical::Railway::Create (*transportation, baseAlignment.get());
            if (rail.IsValid())
                {
                rail->SetMainAlignment (baseAlignment.get());
                corridorPortion = rail->Insert ();
                }
            }
        else
            {
            // WIP - how to tell this really is a road or not?
            auto road = RoadPhysical::Roadway::Create (*transportation, baseAlignment.get());
            if (road.IsValid())
                {
                road->SetMainAlignment (baseAlignment.get());
                corridorPortion = road->Insert ();
                }
            }
        }

    return  BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/20
+---------------+---------------+---------------+---------------+---------------+------*/
bool AeccCorridorExt::FindCandidateAeccAlignment ()
    {
    if (m_aeccCorridor == nullptr)
        return  false;

    // get the first center or rail alignment from the corridor; otherwise the last baseline is returned:
    uint32_t count = m_aeccCorridor->GetBaselineCount ();
    for (uint32_t i = 0; i < count; count++)
        {
        auto baseline = m_aeccCorridor->GetBaselineByIndex (i);
        if (!baseline.isNull())
            {
            m_aeccAlignment = baseline->GetAlignment().openObject (OdDb::OpenMode::kForRead);
            if (!m_aeccAlignment.isNull())
                {
                auto type = m_aeccAlignment->GetAlignmentType ();
                if (type == AlignmentType::Centerline || type == AlignmentType::Rail)
                    return  true;
                }
            }
        }
    return  !m_aeccAlignment.isNull();
    }


END_C3D_NAMESPACE
