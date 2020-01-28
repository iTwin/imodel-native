/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "C3dInternal.h"

BEGIN_C3D_NAMESPACE

DWG_PROTOCOLEXT_DEFINE_MEMBERS(AeccPipeExt)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccPipeExt::_ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer)
    {
    m_importer = dynamic_cast<C3dImporterP>(&importer);
    m_aeccPipe = AECCDbPipe::cast (context.GetEntityPtrR().get());
    if (nullptr == m_importer || nullptr == m_aeccPipe || !context.GetModel().Is3d())
        return  BentleyStatus::BSIERROR;

    m_toDgnContext = &context;

    return this->ImportPipe ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccPipeExt::ImportPipe ()
    {
    auto status = this->CreateOrUpdateAeccPipe ();
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    // WIP - Civil domain pipe support
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccPipeExt::CreateOrUpdateAeccPipe ()
    {
    auto ecInstance = m_importer->CreateC3dECInstance (ECCLASSNAME_AeccPipe);
    if (!ecInstance.IsValid())
        return  BentleyStatus::BSIERROR;

    auto& results = m_toDgnContext->GetElementResultsR ();
    auto& inputs = m_toDgnContext->GetElementInputsR ();

    inputs.SetClassId (ecInstance->GetClass().GetId());

    Utf8String  name(reinterpret_cast<WCharCP>(m_aeccPipe->GetPartName().c_str()));
    if (!name.empty())
        inputs.SetElementLabel (name);

    auto status = m_importer->_ImportEntity (results, inputs);
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    auto importedElement = results.GetImportedElement ();
    if (importedElement == nullptr)
        return  BentleyStatus::BSIERROR;

    Utf8String  strValue(reinterpret_cast<WCharCP>(m_aeccPipe->GetDescription().c_str()));
    if (!strValue.empty())
        ecInstance->SetValue (ECPROPNAME_Description, ECValue(strValue.c_str()));

    AECCDbPipeStylePtr  style = m_aeccPipe->GetPartStyle().openObject ();
    if (!style.isNull())
        {
        strValue.Assign (reinterpret_cast<WCharCP>(style->GetName().c_str()));
        if (!strValue.empty())
            ecInstance->SetValue (ECPROPNAME_Style, ECValue(strValue.c_str()));
        }

    switch (m_aeccPipe->GetPipeFlowDirectionMethod())
        {
        case AECCDbPipe::FlowDirectionMethod::eBidirectional: strValue.assign("Bi-directional"); break;
        case AECCDbPipe::FlowDirectionMethod::eStartToEnd: strValue.assign("Start to End"); break;
        case AECCDbPipe::FlowDirectionMethod::eEndToStart: strValue.assign("End to Start"); break;
        case AECCDbPipe::FlowDirectionMethod::eBySlope: strValue.assign("By Slope");  break;
        }
    if (!strValue.empty())
        ecInstance->SetValue (ECPROPNAME_FlowDirectionMethod, ECValue(strValue.c_str()));

    AECCDbAlignmentPtr  alignment = m_aeccPipe->GetReferenceAlignment().openObject ();
    if (!alignment.isNull())
        {
        strValue.Assign (reinterpret_cast<WCharCP>(alignment->GetName().c_str()));
        if (!strValue.empty())
            ecInstance->SetValue (ECPROPNAME_ReferenceAlignment, ECValue(strValue.c_str()));
        }

    AECCDbSurfacePtr  surface = m_aeccPipe->GetReferenceSurface().openObject ();
    if (!surface.isNull())
        {
        strValue.Assign (reinterpret_cast<WCharCP>(surface->GetName().c_str()));
        if (!strValue.empty())
            ecInstance->SetValue (ECPROPNAME_ReferenceSurface, ECValue(strValue.c_str()));
        }

    AECCDbStructurePtr  structure = m_aeccPipe->GetStartStructure().openObject ();
    if (!structure.isNull())
        {
        strValue.Assign (reinterpret_cast<WCharCP>(structure->GetPartName().c_str()));
        if (!strValue.empty())
            ecInstance->SetValue (ECPROPNAME_StartStructure, ECValue(strValue.c_str()));
        }

    structure = m_aeccPipe->GetEndStructure().openObject ();
    if (!structure.isNull())
        {
        strValue.Assign (reinterpret_cast<WCharCP>(structure->GetPartName().c_str()));
        if (!strValue.empty())
            ecInstance->SetValue (ECPROPNAME_EndStructure, ECValue(strValue.c_str()));
        }

    ecInstance->SetValue (ECPROPNAME_Slope, ECValue(m_aeccPipe->GetSlope()));
    ecInstance->SetValue (ECPROPNAME_Length2dCenterToCenter, ECValue(m_aeccPipe->GetLength2DCenterToCenter()));
    ecInstance->SetValue (ECPROPNAME_Length3dCenterToCenter, ECValue(m_aeccPipe->GetLength3DCenterToCenter()));
    ecInstance->SetValue (ECPROPNAME_HydraGradeLineUp, ECValue(m_aeccPipe->GetHydraulicGradeLineUp()));
    ecInstance->SetValue (ECPROPNAME_HydraGradeLineDown, ECValue(m_aeccPipe->GetHydraulicGradeLineDown()));
    ecInstance->SetValue (ECPROPNAME_EnergyGradeLineUp, ECValue(m_aeccPipe->GetEnergyGradeLineUp()));
    ecInstance->SetValue (ECPROPNAME_EnergyGradeLineDown, ECValue(m_aeccPipe->GetEnergyGradeLineDown()));
    ecInstance->SetValue (ECPROPNAME_FlowRate, ECValue(m_aeccPipe->GetFlowRate()));
    ecInstance->SetValue (ECPROPNAME_JunctionLoss, ECValue(m_aeccPipe->GetJunctionLoss()));

    // wall thickness in meters
    double  thicknessInMeters = m_aeccPipe->GetWallThickness ();
    ecInstance->SetValue (ECPROPNAME_WallThickness, ECValue(thicknessInMeters));

    this->SetPartData (*ecInstance);

    auto dbStatus = importedElement->SetPropertyValues (*ecInstance);

    return  static_cast<BentleyStatus>(dbStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/20
+---------------+---------------+---------------+---------------+---------------+------*/
void    AeccPipeExt::SetPartData (IECInstanceR ecInstance)
    {
    AECCDbNetworkPartDefPtr partDef = m_aeccPipe->GetPartDefinition().openObject ();
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
            if (fieldKey.CompareToI("InnerStructDiameter") == 0)
                {
                if (fieldValue.type() == AECCVariant::eDouble)
                    {
                    // WIP - determine what units to show
                    double  diameterInMM = ::DrawingUnitsTo(AECDefs::Units::euMillimeters, fieldValue.getDouble(), m_aeccPipe->database());
                    ecInstance.SetValue (ECPROPNAME_InnerStructDiameter, ECValue(diameterInMM));
                    count++;
                    }
                }
            else if (fieldKey.CompareToI("Catalog_PartType") == 0)
                {
                if (this->SetStringPartProperty(ECPROPNAME_PartType, fieldValue, ecInstance) == ECObjectsStatus::Success)
                    count++;
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
            else if (fieldKey.CompareToI("Catalog_PartName") == 0)
                {
                // AeccCircularConnectPipe_Metric
                }
            else if (fieldKey.CompareToI("Material_Type") == 0)
                {
                if (this->SetStringPartProperty(ECPROPNAME_Material, fieldValue, ecInstance) == ECObjectsStatus::Success)
                    count++;
                }
            else if (fieldKey.CompareToI("SweptShape") == 0)
                {
                if (fieldValue.type() == AECCVariant::eString)
                    {
                    Utf8String strValue(reinterpret_cast<WCharCP>(fieldValue.getString().c_str()));
                    strValue = strValue.substr (::strlen("SweptShape_"));
                    if (!strValue.empty())
                        {
                        ecInstance.SetValue (ECPROPNAME_CrossSectionalShape, ECValue(strValue.c_str()));
                        count++;
                        }
                    }
                }
            else if (fieldKey.CompareToI("FlowAnalysis_Manning") == 0)
                {
                // optional field - do not count
                if (fieldValue.type() == AECCVariant::eDouble)
                    ecInstance.SetValue (ECPROPNAME_ManningCoefficient, ECValue(fieldValue.getDouble()));
                }
            }
        // only need to retrieve what we need
        if (count >= 8)
            break;
        }

    if (count < 8)
        {
        Utf8PrintfString msg("Expected 8 fields in the part definition, but only seen %d, pipe ID=%ls", count, m_aeccPipe->objectId().getHandle().ascii().c_str());
        m_importer->ReportIssue (DwgImporter::IssueSeverity::Info, IssueCategory::MissingData(), Issue::Message(), msg.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/20
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus  AeccPipeExt::SetStringPartProperty (Utf8CP propName, AECCVariant const& var, IECInstanceR ecInstance)
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
