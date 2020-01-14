/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "C3dInternal.h"

BEGIN_C3D_NAMESPACE

DWG_PROTOCOLEXT_DEFINE_MEMBERS(AeccFeatureLineExt)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccFeatureLineExt::_ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer)
    {
    m_importer = dynamic_cast<C3dImporterP>(&importer);
    m_aeccFeatureLine = AECCDbFeatureLine::cast (context.GetEntityPtrR().get());
    if (nullptr == m_importer || nullptr == m_aeccFeatureLine || !context.GetModel().Is3d())
        return  BentleyStatus::BSIERROR;

    m_toDgnContext = &context;

    return this->ImportFeatureLine ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AeccFeatureLineExt::ImportFeatureLine ()
    {
    auto featureLineClass =  m_importer->GetC3dECClass (ECCLASSNAME_AeccFeatureLine);
    if (featureLineClass == nullptr)
        return  BentleyStatus::BSIERROR;

    auto& results = m_toDgnContext->GetElementResultsR ();
    auto& inputs = m_toDgnContext->GetElementInputsR ();

    inputs.SetClassId (featureLineClass->GetId());

    auto status = m_importer->_ImportEntity (results, inputs);
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    m_importedElement = results.GetImportedElement ();
    if (m_importedElement == nullptr)
        return  BentleyStatus::BSIERROR;

    try
        {
        AECCDbFeatureLineStylePtr   style = m_aeccFeatureLine->GetFeatureLineStyle().openObject ();
        if (!style.isNull())
            {
            Utf8String  styleName(reinterpret_cast<WCharCP>(style->GetName().c_str()));
            if (!styleName.empty())
                m_importedElement->SetPropertyValue (ECPROPNAME_Style, styleName.c_str());
            }
        }
    catch (OdError error)
        {
        LOG.errorv ("Exception thrown opening FeatureLineStyle [%s]", error.description().c_str());
        }

    m_importedElement->SetPropertyValue (ECPROPNAME_NumberOfPoints, static_cast<int32_t>(m_aeccFeatureLine->GetEdgeCount()));
    m_importedElement->SetPropertyValue (ECPROPNAME_MinElevation, m_aeccFeatureLine->GetMinimumElevation());
    m_importedElement->SetPropertyValue (ECPROPNAME_MaxElevation, m_aeccFeatureLine->GetMaximumElevation());

    OdDbCurvePtr    odCurve;
    bool is3d = m_aeccFeatureLine->GetPolyline (odCurve);
    if (!odCurve.isNull())
        {
        double  end = 0.0, length = 0.0;
        if (odCurve->getEndParam(end) == OdResult::eOk && odCurve->getDistAtParam(end, length) == OdResult::eOk)
            m_importedElement->SetPropertyValue (ECPROPNAME_Length3d, length);
        }

    FacetModeler::Contour2D contour;
    std::vector<double> elevations;
    if (m_aeccFeatureLine->GetContour2D(contour, elevations))
        {
        double  length = contour.length ();
        if (length > 0.0)
            m_importedElement->SetPropertyValue (ECPROPNAME_Length2d, length);
        }

    return  status;
    }

END_C3D_NAMESPACE
