/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    "C3dImporter.h"
#include    "C3dHelper.h"

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

    status = this->CreateAeccAlignment ();
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    m_alignmentModel = m_importer->GetAlignmentModel ();
    if (!m_alignmentModel.IsValid())
        return  status;

    status = this->CreateAlignment ();

    return  BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AeccAlignmentExt::CreateAeccAlignment ()
    {
    auto ecClass = this->GetECClass (ECCLASSNAME_AeccAligment);
    if (ecClass == nullptr)
        return  BentleyStatus::BSIERROR;

    auto ecInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance ();
    if (!ecInstance.IsValid())
        return  BentleyStatus::BSIERROR;

    auto& results = m_toDgnContext->GetElementResultsR ();
    auto& inputs = m_toDgnContext->GetElementInputsR ();
    inputs.SetClassId (ecClass->GetId());

    auto status = m_importer->_ImportEntity (results, inputs);
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    auto element = results.GetImportedElement ();
    if (element != nullptr)
        {
        if (!m_name.empty())
            element->SetUserLabel (m_name.c_str());

        element->SetPropertyValue ("Description", m_description.c_str());
        element->SetPropertyValue ("ReferenceStation", m_aeccAlignment->GetReferencePointStation());
        element->SetPropertyValue ("StartStation", m_aeccAlignment->GetStartStation());
        element->SetPropertyValue ("EndStation", m_aeccAlignment->GetEndStation());

        OdGePoint2d point;
        if (m_aeccAlignment->GetReferencePoint(point))
            element->SetPropertyValue ("ReferencePoint", DPoint2d::From(point.x, point.y));

        this->SetDesignSpeeds (*element);
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AeccAlignmentExt::SetDesignSpeeds (DgnElementR element)
    {
    auto ecClass = this->GetECClass ("DesignSpeed");
    if (ecClass == nullptr)
        return  BentleyStatus::BSIERROR;

    auto ecEnabler = ecClass->GetDefaultStandaloneEnabler ();
    if (!ecEnabler.IsValid())
        return  BentleyStatus::BSIERROR;

    auto count = m_aeccAlignment->GetDesignSpeedCount ();
    if (count < 1)
        return  BentleyStatus::BSISUCCESS;

    uint32_t    propIndex = 0;
    auto status = element.GetPropertyIndex (propIndex, "DesignSpeeds");
    if (status != DgnDbStatus::Success)
        return  static_cast<BentleyStatus>(status);

    element.InsertPropertyArrayItems (propIndex, 0, count);

    ECValue ecValue;
    status = element.GetPropertyValue (ecValue, "DesignSpeeds");
    if (!ecValue.IsArray() || ecValue.GetArrayInfo().GetCount() != count)
        BeAssert (false && "StructArray not correctly set");

    for (int32_t i = 0; i < static_cast<int32_t>(count); i++)
        {
        auto designSpeed = m_aeccAlignment->GetDesignSpeedsByIndex (i);
        if (!designSpeed.isNull())
            {
            Utf8String  comment(reinterpret_cast<WCharCP>(designSpeed->GetComment().c_str()));

            auto ecInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance ();
            if (ecInstance.IsValid())
                {
                ecInstance->SetValue ("StartStation", ECValue(designSpeed->GetStation()));
                ecInstance->SetValue ("DesignSpeed", ECValue(designSpeed->GetValue()));
                ecInstance->SetValue ("Comment", ECValue(comment.c_str()));

                ecValue.SetStruct (ecInstance.get());
                status = element.SetPropertyValue("DesignSpeeds", ecValue, PropertyArrayIndex(i));
                }
            }
        }

    return  static_cast<BentleyStatus>(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP   AeccAlignmentExt::GetECClass (Utf8StringCR className) const
    {
    ECClassCP   ecClass = nullptr;
    auto c3dSchema = m_importer->GetC3dSchema ();
    if (c3dSchema != nullptr)
        ecClass = c3dSchema->GetClassCP (DwgHelper::ValidateECNameFrom(className).c_str());
    return  ecClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccAlignmentExt::CreateAlignment ()
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

END_C3D_NAMESPACE
