/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "C3dImporter.h"
#include "C3dHelper.h"

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

    auto& results = m_toDgnContext->GetElementResultsR ();
    auto& inputs = m_toDgnContext->GetElementInputsR ();

    inputs.SetClassId (corridorClass->GetId());
    if (!m_name.empty())
        inputs.SetElementLabel (m_name);

    auto status = m_importer->_ImportEntity (results, inputs);
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    m_importedElement = results.GetImportedElement ();
    if (m_importedElement != nullptr)
        {
        if (!m_description.empty())
            m_importedElement->SetPropertyValue (ECPROPNAME_Description, m_description.c_str());

        // Parameters
        this->ProcessBaselines ();

        // Features
        this->ProcessFeatureStyles ();

        // Codes
        this->ProcessCodes ();
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

    auto status = m_importer->InsertArrayProperty (*m_importedElement, ECPROPNAME_CorridorParameters, count);
    if (status != DgnDbStatus::Success)
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

                status = m_importedElement->SetPropertyValue (ECPROPNAME_CorridorParameters, paramsValue, PropertyArrayIndex(i));
                if (status != DgnDbStatus::Success)
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

    auto status = m_importer->InsertArrayProperty (*m_importedElement, ECPROPNAME_CorridorFeatures, count);
    if (status != DgnDbStatus::Success)
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

            status = m_importedElement->SetPropertyValue (ECPROPNAME_CorridorFeatures, featureValue, PropertyArrayIndex(i));
            if (status != DgnDbStatus::Success)
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
        auto status = m_importer->InsertArrayProperty (*m_importedElement, ECPROPNAME_LinkCodes, count);
        if (status != DgnDbStatus::Success)
            return  static_cast<BentleyStatus>(status);

        for (uint32_t i = 0; i < count; i++)
            {
            auto code = roadwayStyleSet->GetCustomLinkCode (i);
            AECCSubassemblyEntTraitsSubPtr linkCustom = roadwayStyleSet->GetLinkCustom (code);
            if (!linkCustom.isNull())
                {
                status = this->ProcessCode (code, *linkCustom, ECPROPNAME_LinkCodes, i);
                if (status != DgnDbStatus::Success)
                    return  static_cast<BentleyStatus>(status);
                }
            }
        }
    
    // Point codes
    count = roadwayStyleSet->GetCustomPointCount ();
    if (count > 1)
        {
        auto status = m_importer->InsertArrayProperty (*m_importedElement, ECPROPNAME_PointCodes, count);
        if (status != DgnDbStatus::Success)
            return  static_cast<BentleyStatus>(status);

        for (uint32_t i = 0; i < count; i++)
            {
            auto code = roadwayStyleSet->GetCustomPointCode (i);
            AECCSubassemblyEntTraitsSubPtr pointCustom = roadwayStyleSet->GetPointCustom (code);
            if (!pointCustom.isNull())
                {
                status = this->ProcessCode (code, *pointCustom, ECPROPNAME_PointCodes, i);
                if (status != DgnDbStatus::Success)
                    return  static_cast<BentleyStatus>(status);
                }
            }
        }
    
    // Shape codes
    count = roadwayStyleSet->GetCustomShapeCount ();
    if (count > 1)
        {
        auto status = m_importer->InsertArrayProperty (*m_importedElement, ECPROPNAME_ShapeCodes, count);
        if (status != DgnDbStatus::Success)
            return  static_cast<BentleyStatus>(status);

        for (uint32_t i = 0; i < count; i++)
            {
            auto code = roadwayStyleSet->GetCustomShapeCode (i);
            AECCSubassemblyEntTraitsSubPtr shapeCustom = roadwayStyleSet->GetShapeCustom (code);
            if (!shapeCustom.isNull())
                {
                status = this->ProcessCode (code, *shapeCustom, ECPROPNAME_ShapeCodes, i);
                if (status != DgnDbStatus::Success)
                    return  static_cast<BentleyStatus>(status);
                }
            }
        }
    
    return  BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AeccCorridorExt::ProcessCode (OdString const& code, AECCSubassemblyEntTraits const& subassentTraits, Utf8StringCR propName, uint32_t index)
    {
    auto codeInstance = m_importer->CreateC3dECInstance (ECCLASSNAME_CorridorCode);
    if (!codeInstance.IsValid())
        return  DgnDbStatus::WrongClass;

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

    return  m_importedElement->SetPropertyValue(propName.c_str(), codeValue, PropertyArrayIndex(index));
    }


END_C3D_NAMESPACE
