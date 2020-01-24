/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "C3dInternal.h"

BEGIN_C3D_NAMESPACE

DWG_PROTOCOLEXT_DEFINE_MEMBERS(AeccStructureExt)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccStructureExt::_ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer)
    {
    m_importer = dynamic_cast<C3dImporterP>(&importer);
    m_aeccStructure = AECCDbStructure::cast (context.GetEntityPtrR().get());
    if (nullptr == m_importer || nullptr == m_aeccStructure || !context.GetModel().Is3d())
        return  BentleyStatus::BSIERROR;

    m_toDgnContext = &context;

    return this->ImportStructure ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccStructureExt::ImportStructure ()
    {
    auto status = this->CreateOrUpdateAeccStructure ();
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    // WIP - Civil domain structure support
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccStructureExt::CreateOrUpdateAeccStructure ()
    {
    auto ecInstance = m_importer->CreateC3dECInstance (ECCLASSNAME_AeccStructure);
    if (!ecInstance.IsValid())
        return  BentleyStatus::BSIERROR;

    auto& results = m_toDgnContext->GetElementResultsR ();
    auto& inputs = m_toDgnContext->GetElementInputsR ();

    inputs.SetClassId (ecInstance->GetClass().GetId());

    Utf8String  name(reinterpret_cast<WCharCP>(m_aeccStructure->GetPartName().c_str()));
    if (!name.empty())
        inputs.SetElementLabel (name);

    auto status = m_importer->_ImportEntity (results, inputs);
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    auto importedElement = results.GetImportedElement ();
    if (importedElement == nullptr)
        return  BentleyStatus::BSIERROR;

    Utf8String  strValue(reinterpret_cast<WCharCP>(m_aeccStructure->GetDescription().c_str()));
    if (!strValue.empty())
        ecInstance->SetValue (ECPROPNAME_Description, ECValue(strValue.c_str()));

    AECCDbStructureStylePtr  style = m_aeccStructure->GetPartStyle().openObject ();
    if (!style.isNull())
        {
        strValue.Assign (reinterpret_cast<WCharCP>(style->GetName().c_str()));
        if (!strValue.empty())
            ecInstance->SetValue (ECPROPNAME_Style, ECValue(strValue.c_str()));
        }

    AECCDbAlignmentPtr  alignment = m_aeccStructure->GetReferenceAlignment().openObject ();
    if (!alignment.isNull())
        {
        strValue.Assign (reinterpret_cast<WCharCP>(alignment->GetName().c_str()));
        if (!strValue.empty())
            ecInstance->SetValue (ECPROPNAME_ReferenceAlignment, ECValue(strValue.c_str()));
        }

    AECCDbSurfacePtr  surface = m_aeccStructure->GetReferenceSurface().openObject ();
    if (!surface.isNull())
        {
        strValue.Assign (reinterpret_cast<WCharCP>(surface->GetName().c_str()));
        if (!strValue.empty())
            ecInstance->SetValue (ECPROPNAME_ReferenceSurface, ECValue(strValue.c_str()));
        }

    int nPipes = m_aeccStructure->GetConnectorCount ();
    auto controlSumpBy = m_aeccStructure->GetControlSumpMode ();
    auto location = m_aeccStructure->GetLocation ();

    ecInstance->SetValue (ECPROPNAME_ConnectedPipes, ECValue(nPipes));
    ecInstance->SetValue (ECPROPNAME_InsertionElevation, ECValue(location.z));
    ecInstance->SetValue (ECPROPNAME_SurfaceAdjustment, ECValue(m_aeccStructure->GetSurfaceAdjustment()));
    ecInstance->SetValue (ECPROPNAME_AutoSurfaceAdjustment, ECValue(m_aeccStructure->IsAutomaticSurfaceAdjustment()));
    ecInstance->SetValue (ECPROPNAME_SumpDepth, ECValue(m_aeccStructure->GetSumpDepth()));
    ecInstance->SetValue (ECPROPNAME_ControlSumpBy, ECValue(static_cast<int>(controlSumpBy)));

    this->SetPartDefinition (*ecInstance);
    this->SetPartRecord (*ecInstance);

    auto dbStatus = importedElement->SetPropertyValues (*ecInstance);

    return  static_cast<BentleyStatus>(dbStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/20
+---------------+---------------+---------------+---------------+---------------+------*/
void    AeccStructureExt::SetPartDefinition (IECInstanceR ecInstance)
    {
    AECCDbNetworkPartDefPtr partDef = m_aeccStructure->GetPartDefinition().openObject ();
    if (partDef.isNull())
        return;

    // walk through the variant fields and parse the data we are interested
    size_t  count = 0;
    for (uint32_t i = 0; i < partDef->GetDataFieldCount(); i++)
        {
        AECCNetworkPartDataFieldSubPtr dataField = partDef->GetDataFieldByIndex(i);
        if (!dataField.isNull())
            {
            Utf8String  fieldKey (reinterpret_cast<WCharCP>(dataField->GetContext().c_str()));
            AECCVariant fieldValue = dataField->GetValue ();
            if (fieldKey.CompareToI("StructInnerDiameter") == 0)
                {
                if (fieldValue.type() == AECCVariant::eDouble)
                    {
                    ecInstance.SetValue (ECPROPNAME_InnerStructDiameter, ECValue(fieldValue.getDouble()));
                    count++;
                    }
                }
            else if (fieldKey.CompareToI("StructFrameDiameter") == 0)
                {
                if (fieldValue.type() == AECCVariant::eDouble)
                    {
                    ecInstance.SetValue (ECPROPNAME_FrameDiameter, ECValue(fieldValue.getDouble()));
                    count++;
                    }
                }
            else if (fieldKey.CompareToI("StructFrameHeight") == 0)
                {
                if (fieldValue.type() == AECCVariant::eDouble)
                    {
                    ecInstance.SetValue (ECPROPNAME_FrameHeight, ECValue(fieldValue.getDouble()));
                    count++;
                    }
                }
            else if (fieldKey.CompareToI("WallThickness") == 0)
                {
                if (fieldValue.type() == AECCVariant::eDouble)
                    {
                    ecInstance.SetValue (ECPROPNAME_WallThickness, ECValue(fieldValue.getDouble()));
                    count++;
                    }
                }
            else if (fieldKey.CompareToI("FloorThickness") == 0)
                {
                if (fieldValue.type() == AECCVariant::eDouble)
                    {
                    ecInstance.SetValue (ECPROPNAME_FloorThickness, ECValue(fieldValue.getDouble()));
                    count++;
                    }
                }
            else if (fieldKey.CompareToI("StructRimToSumpHeight") == 0)
                {
                if (fieldValue.type() == AECCVariant::eDouble)
                    {
                    ecInstance.SetValue (ECPROPNAME_RimToSumpHeight, ECValue(fieldValue.getDouble()));
                    count++;
                    }
                }
            else if (fieldKey.CompareToI("StructBoundingShape") == 0)
                {
                if (fieldValue.type() == AECCVariant::eString)
                    {
                    Utf8String strValue(reinterpret_cast<WCharCP>(fieldValue.getString().c_str()));
                    strValue = strValue.substr (::strlen("BoundingShape_"));
                    ecInstance.SetValue (ECPROPNAME_StructureShape, ECValue(strValue.c_str()));
                    count++;
                    }
                }
            else if (fieldKey.CompareToI("StructBarrelPipeClearance") == 0)
                {
                if (fieldValue.type() == AECCVariant::eDouble)
                    {
                    ecInstance.SetValue (ECPROPNAME_BarrelPipeClearance, ECValue(fieldValue.getDouble()));
                    count++;
                    }
                }
            else if (fieldKey.CompareToI("StructConeHeight") == 0)
                {
                if (fieldValue.type() == AECCVariant::eDouble)
                    {
                    ecInstance.SetValue (ECPROPNAME_ConeHeight, ECValue(fieldValue.getDouble()));
                    count++;
                    }
                }
            else if (fieldKey.CompareToI("StructSlabThickness") == 0)
                {
                if (fieldValue.type() == AECCVariant::eDouble)
                    {
                    ecInstance.SetValue (ECPROPNAME_SlabThickness, ECValue(fieldValue.getDouble()));
                    count++;
                    }
                }
            else if (fieldKey.CompareToI("StructDiameter") == 0)
                {
                if (fieldValue.type() == AECCVariant::eDouble)
                    {
                    ecInstance.SetValue (ECPROPNAME_StructureDiameter, ECValue(fieldValue.getDouble()));
                    count++;
                    }
                }
            else if (fieldKey.CompareToI("StructHeight") == 0)
                {
                if (fieldValue.type() == AECCVariant::eDouble)
                    {
                    ecInstance.SetValue (ECPROPNAME_StructureHeight, ECValue(fieldValue.getDouble()));
                    count++;
                    }
                }
            else if (fieldKey.CompareToI("StructureShape") == 0)
                {
                if (this->SetStringPartProperty(ECPROPNAME_StructureShape, fieldValue, ecInstance) == ECObjectsStatus::Success)
                    count++;
                }
            else if (fieldKey.CompareToI("StructVertPipeClearance") == 0)
                {
                if (fieldValue.type() == AECCVariant::eDouble)
                    {
                    ecInstance.SetValue (ECPROPNAME_VerticalPipeClearance, ECValue(fieldValue.getDouble()));
                    count++;
                    }
                }
            else if (fieldKey.CompareToI("Catalog_PartType") == 0)
                {
                if (fieldValue.type() == AECCVariant::eString)
                    {
                    Utf8String strValue(reinterpret_cast<WCharCP>(fieldValue.getString().c_str()));
                    if (strValue.CompareToI("Struct_Junction") == 0)
                        strValue.assign ("Junction structure");
                    ecInstance.SetValue (ECPROPNAME_PartType, ECValue(strValue.c_str()));
                    count++;
                    }
                }
            else if (fieldKey.CompareToI("Catalog_SubType") == 0)
                {
                if (this->SetStringPartProperty(ECPROPNAME_PartSubType, fieldValue, ecInstance) == ECObjectsStatus::Success)
                    count++;
                }
            else if (fieldKey.CompareToI("Catalog_PartDesc") == 0)
                {
                if (this->SetStringPartProperty(ECPROPNAME_PartDescription, fieldValue, ecInstance) == ECObjectsStatus::Success)
                    count++;
                }
            else if (fieldKey.CompareToI("Catalog_PartSizeName") == 0)
                {
                if (this->SetStringPartProperty(ECPROPNAME_PartSizeName, fieldValue, ecInstance) == ECObjectsStatus::Success)
                    count++;
                }
            else if (fieldKey.CompareToI("Catalog_PartID") == 0)
                {
                if (this->SetStringPartProperty(ECPROPNAME_PartID, fieldValue, ecInstance) == ECObjectsStatus::Success)
                    count++;
                }
            }
        // only need to retrieve what we need, but not sure there could exist dup entries
        if (count >= 18)
            break;
        }

    if (count < 18)
        {
        Utf8PrintfString msg("Expected 18 fields in part definition, but only seen %d, structure ID=%ls", count, m_aeccStructure->objectId().getHandle().ascii().c_str());
        m_importer->ReportIssue(DwgImporter::IssueSeverity::Info, IssueCategory::MissingData(), Issue::Message(), msg.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/20
+---------------+---------------+---------------+---------------+---------------+------*/
void    AeccStructureExt::SetPartRecord (IECInstanceR ecInstance)
    {
    AECCNetworkPartRecordSubPtr partRecord = m_aeccStructure->GetPartRecord ();
    if (partRecord.isNull())
        return;

    // walk through the variant fields and parse the data we are interested
    size_t  count = 0;
    for (uint32_t i = 0; i < partRecord->GetDataFieldCount(); i++)
        {
        AECCNetworkPartDataFieldSubPtr dataField = partRecord->GetDataFieldByIndex(i);
        if (!dataField.isNull())
            {
            Utf8String  fieldKey (reinterpret_cast<WCharCP>(dataField->GetContext().c_str()));
            AECCVariant fieldValue = dataField->GetValue ();
            if (fieldKey.CompareToI("Material_Type") == 0)
                {
                if (this->SetStringPartProperty(ECPROPNAME_Material, fieldValue, ecInstance) == ECObjectsStatus::Success)
                    count++;
                }
            else if (fieldKey.CompareToI("StructFrame") == 0)
                {
                if (this->SetStringPartProperty(ECPROPNAME_Frame, fieldValue, ecInstance) == ECObjectsStatus::Success)
                    count++;
                }
            else if (fieldKey.CompareToI("StructGrate") == 0)
                {
                if (this->SetStringPartProperty(ECPROPNAME_Grate, fieldValue, ecInstance) == ECObjectsStatus::Success)
                    count++;
                }
            else if (fieldKey.CompareToI("StructCover") == 0)
                {
                if (this->SetStringPartProperty(ECPROPNAME_Cover, fieldValue, ecInstance) == ECObjectsStatus::Success)
                    count++;
                }
            }
        if (count >= 4)
            break;
        }

    if (count < 4)
        {
        Utf8PrintfString msg("Expected 4 in part record, but only seen %d, structure ID=%ls", count, m_aeccStructure->objectId().getHandle().ascii().c_str());
        m_importer->ReportIssue (DwgImporter::IssueSeverity::Info, IssueCategory::MissingData(), Issue::Message(), msg.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/20
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus  AeccStructureExt::SetStringPartProperty (Utf8CP propName, AECCVariant const& var, IECInstanceR ecInstance)
    {
    ECObjectsStatus status = ECObjectsStatus::ParseError;
    if (var.type() == AECCVariant::eString && propName != nullptr)
        {
        Utf8String strValue(reinterpret_cast<WCharCP>(var.getString().c_str()));
        if (!strValue.empty())
            status = ecInstance.SetValue (propName, ECValue(strValue.c_str()));
        }
    return  status;
    }

END_C3D_NAMESPACE
