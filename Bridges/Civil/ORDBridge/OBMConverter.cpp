/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/OBMConverter.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ORDBridgeInternal.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ORDBRIDGE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Halim.Salameh                     09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr OBMConverter::CreateCuttingShapeByPointAndNormal(DPoint3dCR pt, DVec3dCR normalDir)
    {
    DVec3d v0, v01, v02, v1, v2, v3, v4;
    v0 = DVec3d::From(pt);
    DVec3d xAxisDir = DVec3d::UnitZ();
    const double fixedDistance = 100.0;
    v01.SumOf(v0, xAxisDir, fixedDistance);
    v02.SumOf(v0, xAxisDir, -fixedDistance);
    v1.SumOf(v01, normalDir, fixedDistance);
    v2.SumOf(v01, normalDir, -fixedDistance);
    v3.SumOf(v02, normalDir, -fixedDistance);
    v4.SumOf(v02, normalDir, fixedDistance);
    DPoint3d pts[5];
    pts[0].Init(v1);
    pts[1].Init(v2);
    pts[2].Init(v3);
    pts[3].Init(v4);
    pts[4].Init(v1);
    CurveVectorPtr result;
    result = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    result->Add(ICurvePrimitive::CreateLineString(pts, _countof(pts)));

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Halim.Salameh                      09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntityPtr OBMConverter::CreateCuttingToolAlongPath(ICurvePrimitivePtr curvePrimitivePtr, const double& distanceFromStart)
    {
    CurveVectorPtr inputCurve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    inputCurve->Add(curvePrimitivePtr);

    CurveVectorWithDistanceIndexPtr cvwdi = CurveVectorWithDistanceIndex::Create();
    cvwdi->SetPath(inputCurve);

    PathLocationDetail pld;
    CurveVectorPtr result;
    if (cvwdi->SearchByDistanceFromPathStart(distanceFromStart, pld))
        {
        DVec3d tanDir;
        DPoint3d pt = pld.PointAndUnitTangent(tanDir);
        DVec3d normalDir = DVec3d::FromCCWPerpendicularXY(tanDir);
        result = CreateCuttingShapeByPointAndNormal(pt, normalDir);
        IBRepEntityPtr tmpObj;
        BRepUtil::Create::BodyFromCurveVector(tmpObj, *result);
        return tmpObj;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Halim.Salameh                      09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr OBMConverter::GetLongestEdge(IBRepEntityCR brepEntity)
    {
    bvector<BentleyM0200::Dgn::ISubEntityPtr> edges;
    BRepUtil::GetBodyEdges(&edges, brepEntity);

    double maxLength = 0;
    BentleyM0200::Dgn::ISubEntityPtr longestEdgePtr = nullptr;
    for (auto& edgePtr : edges)
        {
        double length;
        edgePtr->GetGeometry()->GetAsICurvePrimitive()->Length(length);

        if (length > maxLength)
            {
            maxLength = length;
            longestEdgePtr = edgePtr;
            }
        }

    auto longestEdgeGeometryCPtr = longestEdgePtr->GetGeometry();
    return longestEdgeGeometryCPtr->GetAsICurvePrimitive();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Halim.Salameh                      09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::SplitBarrierIntoSmallerParts(DgnDbSync::DgnV8::ElementConversionResults& elRes)
    {
    auto& geomStreamCR = elRes.m_element->ToGeometrySource3d()->GetGeometryStream();
    GeometryCollection collection(geomStreamCR, elRes.m_element->GetDgnDb());
    auto builderPtr = GeometryBuilder::Create(*elRes.m_element->ToGeometrySource());

    for (auto& iter : collection)
        {
        builderPtr->Append(iter.GetGeometryParams());

        if (iter.IsSolid())
            {
            builderPtr->SetAppendAsSubGraphics();

            auto geomPtr = iter.GetGeometryPtr();
            auto iBRepPtr = geomPtr->GetAsIBRepEntity();

            GeometricPrimitivePtr geom = iter.GetGeometryPtr();
            auto brepEntity = geom->GetAsIBRepEntity();

            if (!brepEntity.IsValid())
                continue;

            const auto longestEdgeCurvePrimitivePtr = GetLongestEdge(*brepEntity);
            double maxLength;
            longestEdgeCurvePrimitivePtr->Length(maxLength);
            const double cStep = 5.0; //meters

            if (maxLength < cStep)
                {
                //if the barrier was smaller than the step value, we don't need to cut anything. We just add the original barrier as is.
                builderPtr->Append(*brepEntity);
                continue;
                }

            const size_t partCount = static_cast<size_t>(maxLength / cStep) + 1;
            double distanceAlong = cStep;

            IBRepEntityPtr remainderPtr = brepEntity;

            for (size_t iPart = 0; iPart < partCount; iPart++, distanceAlong += cStep)
                {
                IBRepEntityPtr cuttingToolPtr = CreateCuttingToolAlongPath(longestEdgeCurvePrimitivePtr, distanceAlong);

                IBRepEntityPtr remainderClonePtr = remainderPtr->Clone();

                BRepUtil::Modify::BooleanCut(remainderClonePtr, *cuttingToolPtr, BRepUtil::Modify::CutDirectionMode::Backward,
                    BRepUtil::Modify::CutDepthMode::All, 0.0, false);

                builderPtr->Append(*remainderClonePtr);

                BRepUtil::Modify::BooleanCut(remainderPtr, *cuttingToolPtr, BRepUtil::Modify::CutDirectionMode::Forward,
                    BRepUtil::Modify::CutDepthMode::All, 0.0, false);
                }


            }
        else if (iter.GetGeometryPartId().IsValid())
            builderPtr->Append(iter.GetGeometryPartId(), iter.GetGeometryToSource());
        else
            builderPtr->Append(*iter.GetGeometryPtr());
        }

    builderPtr->Finish(*elRes.m_element->ToGeometrySourceP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
OBMConverter::OBMConverter(ORDConverter& converter) : m_converter(converter)
    {
    InitializeObmElementMappings();
    ObmApplicationSchemaCreator::InitializePhrases();
    
    bool shouldUpdateOBMApplicationSchema = CheckShouldUpdateOBMSchema();
    if (shouldUpdateOBMApplicationSchema)
    {
        BeFileName obmSchemaFileName = m_converter.GetParams().GetAssetsDir();
        obmSchemaFileName.AppendToPath(OBM_SCHEMA_LOCATION);
        obmSchemaFileName.AppendToPath(OBM_SCHEMA_FILE);
        m_obmSchemaCreator = ObmApplicationSchemaCreator::ObmApplicationSchemaCreator(obmSchemaFileName);
        m_obmSchemaCreatorP = &m_obmSchemaCreator;
    }
    else
        m_obmSchemaCreatorP = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbSync::DgnV8::XDomain::Result OBMConverter::_PreConvertElement(DgnV8EhCR v8el, ORDConverter& converter, DgnDbSync::DgnV8::ResolvedModelMapping const& v8mm)
    {
    if (m_elementsSeen.end() != m_elementsSeen.find(v8el.GetElementRef()))
        return DgnDbSync::DgnV8::XDomain::Result::Proceed;

	m_elementsSeen.insert(v8el.GetElementRef());
	Bentley::ObmNET::BridgePtr bridgePtr = Bentley::ObmNET::GeometryModel::SDK::Bridge::CreateFromElementHandle(v8el);
    Bentley::ObmNET::BMSolidPtr bmSolidPtr = Bentley::ObmNET::GeometryModel::SDK::BMSolid::CreateFromElementHandle(v8el);
	if (bmSolidPtr.IsValid())
	{
		m_bmSolidRefSet.insert(v8el.GetElementRef());
        return DgnDbSync::DgnV8::XDomain::Result::Proceed;
	}
    return DgnDbSync::DgnV8::XDomain::Result::SkipElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::_DetermineElementParams(DgnClassId& classId, DgnCode& code, DgnCategoryId& categoryId, DgnV8EhCR v8el, ORDConverter&, ECObjectsV8::IECInstance const* primaryV8Instance, DgnDbSync::DgnV8::ResolvedModelMapping const& v8mm)
    {
    //first attempt at detecting OBM civil data
    DgnV8Api::FindInstancesScopePtr scope = DgnV8Api::FindInstancesScope::CreateScope(v8el, DgnV8Api::FindInstancesScopeOption());
        ECQueryPtr selectAllV8ECQuery = DgnV8Api::ECQuery::CreateQuery(DgnV8Api::ECQUERY_PROCESS_SearchAllExtrinsic);
        for (DgnV8Api::DgnECInstance* v8Instance : DgnV8Api::DgnECManager::GetManager().FindInstances(*scope, *selectAllV8ECQuery))
            {
            BentleyM0200::Dgn::DgnDbSync::DgnV8::ECClassName v8Class(v8Instance->GetClass());
            Utf8String v8SchemaName(v8Class.GetSchemaName());
            Utf8String v8ClassName(v8Class.GetClassName());
            if (v8ClassName.Contains("Column"))
                {
                //Dgn::DgnCategoryId category = Structural::StructuralPhysicalCategory::QueryStructuralPhysicalCategoryId()
                }
            }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::_ProcessResults(DgnDbSync::DgnV8::ElementConversionResults& elRes, DgnV8EhCR v8el, DgnDbSync::DgnV8::ResolvedModelMapping const& v8mm, ORDConverter& converter)
    {

    if (m_bmSolidRefSet.end() == m_bmSolidRefSet.find(v8el.GetElementRef()))
        {
        m_converter.InsertIntoV8ToBimElmMap(v8el.GetElementRef(), elRes.m_element.get());
        }

    /* Disabling the logic associated with splitting parasolids from OBM barriers into smaller chunks
    
    // Temporary hack while the OBM-iModelBridge is developed...
    // Splitting up parasolids from OBM barriers into smaller chunks
    // in order to avoid inacurracies when equivalent meshes are used
    // in platforms not currently supporting parasolids (e.g. iOS).
    DgnDbSync::DgnV8::V8ElementECContent ecContent;
    converter.GetECContentOfElement(ecContent, v8el, v8mm, false);

    bool isBarrier = false;
    if (ecContent.m_primaryV8Instance != nullptr)
        {
        auto fullName = ecContent.m_primaryV8Instance->GetClass().GetFullName();
        isBarrier = 0 == BentleyApi::BeStringUtilities::Wcsicmp(fullName, L"OpenBridgeModeler:Barrier");
        }

    if (isBarrier)
        {
        SplitBarrierIntoSmallerParts(elRes);
        }

        */
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::_OnConversionComplete(ORDConverter& conv)
    {
    CreateBimBridges();
    if(m_obmSchemaCreatorP != nullptr)
        m_obmSchemaCreatorP->ApplySchemaFormatting();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus OBMConverter::_MakeSchemaChanges()
    {
    auto schemasDir = m_converter.GetParams().GetAssetsDir();
    schemasDir.AppendToPath(L"ECSchemas");

    auto schemaContextPtr = ECN::ECSchemaReadContext::CreateContext(true, true);
    auto dgnSchemasDir = schemasDir;
    dgnSchemasDir.AppendToPath(L"Dgn");

    auto ecdbSchemasDir = schemasDir;
    ecdbSchemasDir.AppendToPath(L"ECDb");

    auto stdSchemasDir = schemasDir;
    stdSchemasDir.AppendToPath(L"Standard");

    auto domainSchemasDir = schemasDir;
    domainSchemasDir.AppendToPath(L"Domain");

    schemaContextPtr->AddSchemaPath(ecdbSchemasDir.c_str());
    schemaContextPtr->AddSchemaPath(stdSchemasDir.c_str());
    schemaContextPtr->AddSchemaPath(dgnSchemasDir.c_str());
    schemaContextPtr->AddSchemaPath(domainSchemasDir.c_str());

    ECN::ECSchemaPtr obmSchemaPtr;
    auto obmSchemaFileName = m_converter.GetParams().GetAssetsDir();
    obmSchemaFileName.AppendToPath(OBM_SCHEMA_LOCATION);
    obmSchemaFileName.AppendToPath(OBM_SCHEMA_FILE);

    if (ECN::SchemaReadStatus::Success != ECN::ECSchema::ReadFromXmlFile(obmSchemaPtr, obmSchemaFileName.c_str(), *schemaContextPtr))
        return ERROR;

    auto lockSchemasResponse = m_converter.GetDgnDb().BriefcaseManager().LockSchemas();
    if (Dgn::RepositoryStatus::Success != lockSchemasResponse.Result())
        return BentleyStatus::ERROR;

    bvector<ECN::ECSchemaCP> schemas;
    schemas.push_back(obmSchemaPtr.get());
    if (Dgn::SchemaStatus::Success != m_converter.GetDgnDb().ImportSchemas(schemas))
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

bool OBMConverter::CheckShouldUpdateOBMSchema()
    {
    //set the variable ShouldUpdateOBMSchema to 1 in your batch file if you want to attempt to update the OBM Application schema
    auto shouldUpdateSchema = WCharCP(::_wgetenv(L"ShouldUpdateOBMSchema"));
    if (WString::IsNullOrEmpty(shouldUpdateSchema))
        return false;
    else
        {
        if (WString(shouldUpdateSchema).Equals(WString(L"1")))
            return true;
        }
    return false;
    }

void OBMConverter::InitializeObmElementMappings()
    {
    m_BeamSegmentMap.Insert(Bentley::ObmNET::GeometryModel::SDK::BeamSegmentType::BeamSegmentType_Undefined, StructuralElementType::StructuralElementType_Beam);
    m_BeamSegmentMap.Insert(Bentley::ObmNET::GeometryModel::SDK::BeamSegmentType::BeamSegmentType_UserBeamComponent, StructuralElementType::StructuralElementType_Beam);
    m_BeamSegmentMap.Insert(Bentley::ObmNET::GeometryModel::SDK::BeamSegmentType::BeamSegmentType_ConBeamComponent, StructuralElementType::StructuralElementType_Beam);
    m_BeamSegmentMap.Insert(Bentley::ObmNET::GeometryModel::SDK::BeamSegmentType::BeamSegmentType_SPCBeamComponent, StructuralElementType::StructuralElementType_Beam);

    m_PierCapMap.Insert(Bentley::ObmNET::GeometryModel::SDK::PierCapType::PierCapType_Tapered, StructuralElementType::StructuralElementType_Beam);
    m_PierCapMap.Insert(Bentley::ObmNET::GeometryModel::SDK::PierCapType::PierCapType_RectangleCap, StructuralElementType::StructuralElementType_Beam);
    m_PierCapMap.Insert(Bentley::ObmNET::GeometryModel::SDK::PierCapType::PierCapType_InvertedT, StructuralElementType::StructuralElementType_Beam);
    m_PierCapMap.Insert(Bentley::ObmNET::GeometryModel::SDK::PierCapType::PierCapType_PileCap, StructuralElementType::StructuralElementType_PileCap);
    m_PierCapMap.Insert(Bentley::ObmNET::GeometryModel::SDK::PierCapType::PierCapType_StemWall, StructuralElementType::StructuralElementType_Wall);
    m_PierCapMap.Insert(Bentley::ObmNET::GeometryModel::SDK::PierCapType::PierCapType_VariableCap, StructuralElementType::StructuralElementType_Beam);

    m_PierFootingMap.Insert(Bentley::ObmNET::GeometryModel::SDK::PierFootingType::PierFootingType_RectangleIsolated, StructuralElementType::StructuralElementType_SpreadFooting);
    m_PierFootingMap.Insert(Bentley::ObmNET::GeometryModel::SDK::PierFootingType::PierFootingType_RectangleCombined, StructuralElementType::StructuralElementType_StripFooting);

    m_CrossFrameMap.Insert(Bentley::ObmNET::GeometryModel::SDK::CrossFrameComponentType::CrossFrameComponentType_TopStrutDnStn, StructuralElementType::StructuralElementType_Brace);
    m_CrossFrameMap.Insert(Bentley::ObmNET::GeometryModel::SDK::CrossFrameComponentType::CrossFrameComponentType_TopStrutUpStn, StructuralElementType::StructuralElementType_Brace);
    m_CrossFrameMap.Insert(Bentley::ObmNET::GeometryModel::SDK::CrossFrameComponentType::CrossFrameComponentType_BtmStrutDnStn, StructuralElementType::StructuralElementType_Brace);
    m_CrossFrameMap.Insert(Bentley::ObmNET::GeometryModel::SDK::CrossFrameComponentType::CrossFrameComponentType_TopStrutUpStn, StructuralElementType::StructuralElementType_Brace);
    m_CrossFrameMap.Insert(Bentley::ObmNET::GeometryModel::SDK::CrossFrameComponentType::CrossFrameComponentType_LeftDiagStrutDnStn, StructuralElementType::StructuralElementType_Brace);
    m_CrossFrameMap.Insert(Bentley::ObmNET::GeometryModel::SDK::CrossFrameComponentType::CrossFrameComponentType_LeftDiagStrutUpStn, StructuralElementType::StructuralElementType_Brace);
    m_CrossFrameMap.Insert(Bentley::ObmNET::GeometryModel::SDK::CrossFrameComponentType::CrossFrameComponentType_RightDiagStrutDnStn, StructuralElementType::StructuralElementType_Brace);
    m_CrossFrameMap.Insert(Bentley::ObmNET::GeometryModel::SDK::CrossFrameComponentType::CrossFrameComponentType_LeftDiagStrutUpStn, StructuralElementType::StructuralElementType_Brace);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::CreateBimBridges()
    {
    bmap<Bentley::ElementRefP, Dgn::DgnElementId> cifAlignmentToBimIDMap = m_converter.GetCifAlignmentToBimIDMap();
    Bentley::ObmNET::SdkConnectionPtr sdkConnectionPtr = Bentley::ObmNET::GeometryModel::SDK::SdkConnection::Create(*m_converter.GetRootModelRefP());
    Bentley::ObmNET::BridgeModelPtr bridgeModelPtr = sdkConnectionPtr->GetBridgeModel();
    m_consensusConnectionPtr = sdkConnectionPtr->GetConnection();
    if (bridgeModelPtr.IsValid())
        {
        auto bridgesPtr = bridgeModelPtr->GetBridges();
        if (bridgesPtr.IsValid())
            {
            while (bridgesPtr->MoveNext())
                {
                auto currentBridgePtr = bridgesPtr->GetCurrent();
                if (currentBridgePtr.IsValid())
                    CreateBimBridge(*currentBridgePtr, cifAlignmentToBimIDMap);
                }
            }
        }
    sdkConnectionPtr->Close();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::CreateBimBridge(Bentley::ObmNET::BridgeR bridgeR, bmap<Bentley::ElementRefP, Dgn::DgnElementId> cifAlignmentToBimID)
    {
    Bentley::ObmNET::AlignmentsPtr bridgeAlignmentsPtr = bridgeR.GetAlignments();
    if (bridgeAlignmentsPtr != nullptr)
        {
        Bentley::Cif::AlignmentPtr bridgeAlignmentPtr = bridgeAlignmentsPtr->GetBridgeAlignment();
        if (bridgeAlignmentPtr != nullptr)
            {
            auto cifAlignmentRef = bridgeAlignmentPtr->GetElementHandle()->GetElementRef();
            auto cifIter = cifAlignmentToBimID.find(cifAlignmentRef);
            if (cifIter != cifAlignmentToBimID.end())
                {
                Dgn::DgnElementId bimAlignmentId = cifIter->second;
                RoadRailAlignment::AlignmentCPtr bimAlignmentCPtr = RoadRailAlignment::Alignment::Get(m_converter.GetDgnDb(), bimAlignmentId);
                if (bimAlignmentCPtr != nullptr)
                    {
                    double start, end;
                    GetBimBridgeFromToParams(bridgeR, start, end);
                    ILinearlyLocatedSingleFromTo::CreateFromToParams createFromToParams(*bimAlignmentCPtr, start, end);
                    auto bridgeName = bridgeR.GetName();
                    Utf8String bridgeNameUtf8(bridgeName.c_str());
                    auto physicalModelCP = m_converter.GetRoadNetwork()->GetModel()->ToPhysicalModel();
                    DgnCode bridgeCode = BridgeStructuralPhysical::Bridge::CreateCode(*physicalModelCP, bridgeNameUtf8);
                    BridgeStructuralPhysical::BridgePtr bimBridgePtr = BridgeStructuralPhysical::Bridge::Create(*physicalModelCP, bridgeCode, *bimAlignmentCPtr, createFromToParams);
                    if (bimBridgePtr != nullptr)
                        {
                        if (bimBridgePtr->Insert().IsValid())
                            {
                            CreateBimBridgeComponents(bridgeR, *bimBridgePtr, *bimAlignmentCPtr);
                            }
                        }
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::CreateBimBridgeComponents(Bentley::ObmNET::BridgeR obmBridgeR, BridgeStructuralPhysical::BridgeR bimBridgeR, RoadRailAlignment::AlignmentCR bimAlignmentCR)
    {
    if (DgnDbStatus::Success == BridgeStructuralPhysical::BridgeStructuralPhysicalDomain::SetUpModelHierarchy(bimBridgeR))
        {
        Dgn::PhysicalModelCP structuralSystemModelCP = bimBridgeR.QueryStructuralSystemModel();
        Dgn::PhysicalModelCP multidisciplinaryModelCP = bimBridgeR.QueryMultidisciplinaryModel();
        Dgn::DgnCategoryId bridgeCategoryId = BridgeStructuralPhysical::BridgeCategory::Get(m_converter.GetDgnDb());
        if (structuralSystemModelCP != nullptr && multidisciplinaryModelCP != nullptr)
            {
            auto defaultViewId = m_converter.GetDefaultViewId();
            if (defaultViewId.IsValid())
                {
                auto viewDefCPtr = structuralSystemModelCP->GetDgnDb().Elements().Get<SpatialViewDefinition>(defaultViewId);
                auto modelSelectorPtr = structuralSystemModelCP->GetDgnDb().Elements().GetForEdit<ModelSelector>(viewDefCPtr->GetModelSelectorId());
                auto categorySelectorPtr = structuralSystemModelCP->GetDgnDb().Elements().GetForEdit<CategorySelector>(viewDefCPtr->GetCategorySelectorId());
                
                modelSelectorPtr->AddModel(structuralSystemModelCP->GetModelId());
                modelSelectorPtr->Update();
                }
            auto bridgeUnits = obmBridgeR.GetBridgeUnits();
            //Iterate through all of the bridge units associated with the current bridge
            while (bridgeUnits->MoveNext())
                {
                ObmNET::BridgeUnitPtr currentBridgeUnitPtr = bridgeUnits->GetCurrent();
                CreateBimBridgeSubstructures(*currentBridgeUnitPtr, bimBridgeR, bimAlignmentCR, structuralSystemModelCP);
                CreateBimBridgeSuperstructures(*currentBridgeUnitPtr, bimBridgeR, bimAlignmentCR, structuralSystemModelCP);
                UpdateCategorySelector(structuralSystemModelCP);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::CreateBimBridgeSubstructures(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::BridgeR bimBridgeR, RoadRailAlignment::AlignmentCR bimAlignmentCR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    ProcessObmSupportLines(bridgeUnitR, bimBridgeR, bimAlignmentCR, structuralSystemModelCP);
    ProcessObmAbutmentWingWalls(bridgeUnitR, bimBridgeR, bimAlignmentCR, structuralSystemModelCP);
    ProcessObmBearingGroups(bridgeUnitR, bimBridgeR, bimAlignmentCR, structuralSystemModelCP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ProcessObmSupportLines(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::BridgeR bimBridgeR, RoadRailAlignment::AlignmentCR bimAlignmentCR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto supportLines = bridgeUnitR.GetSupportLines();
    Dgn::DgnCategoryId bridgeCategoryId = BridgeStructuralPhysical::BridgeCategory::Get(m_converter.GetDgnDb());

    //Iterate through all of the support lines associated with the current bridge unit
    while (supportLines->MoveNext())
        {
        auto currentSupportLinePtr = supportLines->GetCurrent();
        Dgn::GeometricElement3d::CreateParams createParams(m_converter.GetDgnDb(), structuralSystemModelCP->GetModelId(), BridgeStructuralPhysical::GenericSubstructureElement::QueryClassId(m_converter.GetDgnDb()), bridgeCategoryId);
        ILinearlyLocatedSingleAt::CreateAtParams createAtParams(bimAlignmentCR, currentSupportLinePtr->GetDistAlong());
        //TODO: Get correct units of skew angle
        auto bimSubstructureElementPtr = BridgeStructuralPhysical::GenericSubstructureElement::Create(createParams, createAtParams, currentSupportLinePtr->GetSkewAngle());
        if (!bimSubstructureElementPtr.IsValid())
            return;
        if (!bimSubstructureElementPtr->Insert().IsValid())
            return;

        m_SupportLineRefToConvertedSubstructureMap.Insert(currentSupportLinePtr->GetElementHandle()->GetElementRef(), bimSubstructureElementPtr);

        auto piers = currentSupportLinePtr->GetPiers();

        //Iterate through all of the piers associated with the current support line
        while (piers->MoveNext())
            {
            auto currentPierPtr = piers->GetCurrent();
            ProcessObmPierComponents(*currentPierPtr, *bimSubstructureElementPtr, structuralSystemModelCP);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ProcessObmPierComponents(Bentley::ObmNET::PierR pierR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    if (m_obmSchemaCreatorP != nullptr)
        m_obmSchemaCreatorP->UpdateOBMApplicationSchema(pierR.GetECInstance());
    ConvertProperties(pierR.GetECInstance(), *bimSubstructureR.getP());
    bimSubstructureR.Update();

    auto pierComponentCollectionPtr = pierR.GetPierComponentCollection();
    if (pierComponentCollectionPtr.IsValid())
        {
        auto pierComponentCollectionSubcomponentsPtr = pierComponentCollectionPtr->GetSubComponents();
        while (pierComponentCollectionSubcomponentsPtr->MoveNext())
            {
            auto currentPierSubcomponentPtr = pierComponentCollectionSubcomponentsPtr->GetCurrent();
            ConvertObmPierCaps(*currentPierSubcomponentPtr, bimSubstructureR, structuralSystemModelCP);
            ConvertObmPierCheekWalls(*currentPierSubcomponentPtr, bimSubstructureR, structuralSystemModelCP);
            ConvertObmPierColumns(*currentPierSubcomponentPtr, bimSubstructureR, structuralSystemModelCP);
            ConvertObmPierCrashWalls(*currentPierSubcomponentPtr, bimSubstructureR, structuralSystemModelCP);
            ConvertObmPierFootings(*currentPierSubcomponentPtr, bimSubstructureR, structuralSystemModelCP);
            ConvertObmPierPiles(*currentPierSubcomponentPtr, bimSubstructureR, structuralSystemModelCP);
            ConvertObmPierRockSockets(*currentPierSubcomponentPtr, bimSubstructureR, structuralSystemModelCP);
            ConvertObmPierStruts(*currentPierSubcomponentPtr, bimSubstructureR, structuralSystemModelCP);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ProcessObmBearingGroups(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::BridgeR bimBridgeR, RoadRailAlignment::AlignmentCR bimAlignmentCR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto bearingGroupsPtr = bridgeUnitR.GetBearings();
    if (bearingGroupsPtr.IsValid())
        {
        while (bearingGroupsPtr->MoveNext())
            {
            auto currentBearingGroupPtr = bearingGroupsPtr->GetCurrent();
            auto supportLinePtr = currentBearingGroupPtr->GetSupportLine();
            BridgeStructuralPhysical::GenericSubstructureElementPtr bimSubstructurePtr = nullptr;
            auto iterator = m_SupportLineRefToConvertedSubstructureMap.find(supportLinePtr->GetElementHandle()->GetElementRef());
            if (iterator != m_SupportLineRefToConvertedSubstructureMap.end())
                bimSubstructurePtr = iterator->second;

            auto bearingComponentCollectionPtr = currentBearingGroupPtr->GetSolidCollection();
            if(bearingComponentCollectionPtr.IsValid())
                ProcessObmBearingSubcomponents(*bearingComponentCollectionPtr, bimSubstructurePtr, structuralSystemModelCP);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ProcessObmBearingSubcomponents(Bentley::ObmNET::BearingComponentCollectionR bearingComponentsR, BridgeStructuralPhysical::GenericSubstructureElementPtr bimSubstructurePtr, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto bearingSubcomponentsPtr = bearingComponentsR.GetSubComponents();
    if (bearingSubcomponentsPtr.IsValid())
        {
        while (bearingSubcomponentsPtr->MoveNext())
            {
            auto currentBearingSubcomponentPtr = bearingSubcomponentsPtr->GetCurrent();
            ConvertObmBearings(*currentBearingSubcomponentPtr, bimSubstructurePtr, structuralSystemModelCP);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ProcessObmCrossFrameGroups(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto crossFrameGroupsPtr = bridgeUnitR.GetCrossFrameGroups();
    if (crossFrameGroupsPtr.IsValid())
        {
        while (crossFrameGroupsPtr->MoveNext())
            {
            auto currentCrossFrameGroupPtr = crossFrameGroupsPtr->GetCurrent();
            ProcessObmCrossFrames(*currentCrossFrameGroupPtr, bimSuperstructureR, structuralSystemModelCP);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ProcessObmCrossFrames(Bentley::ObmNET::CrossFrameGroupR crossFrameGroupR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto crossFramesPtr = crossFrameGroupR.GetCrossFrames();
    if (crossFramesPtr.IsValid())
        {
        while (crossFramesPtr->MoveNext())
            {
            auto currentCrossFramePtr = crossFramesPtr->GetCurrent();
            ConvertObmCrossFrameComponents(*currentCrossFramePtr, bimSuperstructureR, structuralSystemModelCP);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ProcessObmSegmentalBalances(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto segmentalBalancesPtr = bridgeUnitR.GetSegmentalBalances();
    if (segmentalBalancesPtr.IsValid())
        {
        while (segmentalBalancesPtr->MoveNext())
            {
            auto currentSegmentalBalancePtr = segmentalBalancesPtr->GetCurrent();
            ProcessObmSegmentCollectionSubcomponents(currentSegmentalBalancePtr, nullptr, bimSuperstructureR, structuralSystemModelCP);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ProcessObmSegmentalSpans(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto segmentalSpansPtr = bridgeUnitR.GetSegmentalSpans();
    if (segmentalSpansPtr.IsValid())
        {
        while (segmentalSpansPtr->MoveNext())
            {
            auto currentSegmentalSpanPtr = segmentalSpansPtr->GetCurrent();
            ProcessObmSegmentCollectionSubcomponents(nullptr, currentSegmentalSpanPtr, bimSuperstructureR, structuralSystemModelCP);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ProcessObmSegmentCollectionSubcomponents(Bentley::ObmNET::SegmentalBalancePtr segmentalBalancePtr, Bentley::ObmNET::SegmentalSpanPtr segmentalSpanPtr, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    Bentley::RefCountedPtr<Bentley::ObmNET::GeometryModel::SDK::SegmentCollection> segmentCollectionPtr;
    if (segmentalBalancePtr.IsValid())
        segmentCollectionPtr = segmentalBalancePtr->GetSegmentCollection();
    else
        segmentCollectionPtr = segmentalSpanPtr->GetSegmentCollection();

    if (segmentCollectionPtr.IsValid())
        {
        auto segmentCollectionSubcomponentsPtr = segmentCollectionPtr->GetSubComponents();
        if (segmentCollectionSubcomponentsPtr.IsValid())
            {
            while (segmentCollectionSubcomponentsPtr->MoveNext())
                {
                auto currentSegmentCollectionSubcomponentPtr = segmentCollectionSubcomponentsPtr->GetCurrent();
                ConvertObmSegments(*currentSegmentCollectionSubcomponentPtr, bimSuperstructureR, structuralSystemModelCP);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmCrossFrameComponents(Bentley::ObmNET::CrossFrameR crossFrameR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto crossFrameCollectionPtr = crossFrameR.GetCrossFrameCollection();
    if (crossFrameCollectionPtr.IsValid())
        {
        auto crossFrameComponentsPtr = crossFrameCollectionPtr->GetCrossFrameComponents();
        if (crossFrameComponentsPtr.IsValid())
            {
            while (crossFrameComponentsPtr->MoveNext())
                {
                auto currentCrossFrameComponentPtr = crossFrameComponentsPtr->GetCurrent();
                auto crossFrameComponentType = currentCrossFrameComponentPtr->GetComponentType();
                auto iterator = m_CrossFrameMap.find(crossFrameComponentType);
                if (iterator != m_CrossFrameMap.end())
                    {
                    auto structuralElementType = iterator->second;
                    DetermineElementMappingAndCreate(structuralElementType, *currentCrossFrameComponentPtr, *bimSuperstructureR.getP(), structuralSystemModelCP);
                    }
                else
                    DetermineElementMappingAndCreate(StructuralElementType_Default, *currentCrossFrameComponentPtr, *bimSuperstructureR.getP(), structuralSystemModelCP);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmSegments(Bentley::ObmNET::SegmentCollectionSubR segmentCollectionSubR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto segmentsPtr = segmentCollectionSubR.GetSegments();
    if (segmentsPtr.IsValid())
        {
        while (segmentsPtr->MoveNext())
            {
            auto currentSegmentPtr = segmentsPtr->GetCurrent();
            CreateBimStructuralElement<Structural::Beam>(*currentSegmentPtr, structuralSystemModelCP, bimSuperstructureR.getP());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ProcessObmAbutmentWingWalls(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::BridgeR bimBridgeR, RoadRailAlignment::AlignmentCR bimAlignmentCR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto abutmentWingWallsPtr = bridgeUnitR.GetAbutmentWingWalls();
    Dgn::DgnCategoryId bridgeCategoryId = BridgeStructuralPhysical::BridgeCategory::Get(m_converter.GetDgnDb());
    while (abutmentWingWallsPtr->MoveNext())
        {
        auto currentAbutmentWingWallPtr = abutmentWingWallsPtr->GetCurrent();
        ProcessObmAbutmentWingWallComponents(*currentAbutmentWingWallPtr, bimBridgeR, bimAlignmentCR, structuralSystemModelCP);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ProcessObmAbutmentWingWallComponents(Bentley::ObmNET::AbutmentWingWallR abutmentWingWallR, BridgeStructuralPhysical::BridgeR bimBridgeR, RoadRailAlignment::AlignmentCR bimAlignmentCR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto parentPierPtr = abutmentWingWallR.GetPier();
    auto supportLinePtr = parentPierPtr->GetSupportLine();
    BridgeStructuralPhysical::GenericSubstructureElementPtr bimSubstructurePtr = nullptr;
    auto iterator = m_SupportLineRefToConvertedSubstructureMap.find(supportLinePtr->GetElementHandle()->GetElementRef());
    if (iterator != m_SupportLineRefToConvertedSubstructureMap.end())
        bimSubstructurePtr = iterator->second;
    
    if (bimSubstructurePtr != nullptr)
        {
        if (m_obmSchemaCreatorP != nullptr)
            m_obmSchemaCreatorP->UpdateOBMApplicationSchema(abutmentWingWallR.GetECInstance());
        ConvertProperties(abutmentWingWallR.GetECInstance(), *bimSubstructurePtr->getP());
        bimSubstructurePtr->Update();
        }

    auto abutmentWingWallCmpCollectionPtr = abutmentWingWallR.GetCmpCollection();
    if (abutmentWingWallCmpCollectionPtr.IsValid())
        {
        auto abutmentWingWallSubComponentCollectionPtr = abutmentWingWallCmpCollectionPtr->GetSubComponents();
        if (abutmentWingWallSubComponentCollectionPtr.IsValid())
            {
            while (abutmentWingWallSubComponentCollectionPtr->MoveNext())
                {
                auto currentAbutmentWingWallSubComponentPtr = abutmentWingWallSubComponentCollectionPtr->GetCurrent();
                ConvertObmWingWalls(*currentAbutmentWingWallSubComponentPtr, bimSubstructurePtr, structuralSystemModelCP);
                ConvertObmWingWallFootings(*currentAbutmentWingWallSubComponentPtr, bimSubstructurePtr, structuralSystemModelCP);
                ConvertObmWingWallPiles(*currentAbutmentWingWallSubComponentPtr, bimSubstructurePtr, structuralSystemModelCP);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmBearings(Bentley::ObmNET::BearingComponentCollectionR bearingComponentCollectionR, BridgeStructuralPhysical::GenericSubstructureElementPtr bimSubstructurePtr, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto bearingsPtr = bearingComponentCollectionR.GetBearings();
    if (bearingsPtr.IsValid())
        {
        while (bearingsPtr->MoveNext())
            {
            auto currentBearingPtr = bearingsPtr->GetCurrent();
            CreateBimStructuralElement<Structural::StructuralMember>(*currentBearingPtr, structuralSystemModelCP, bimSubstructurePtr->getP());
            ConvertObmBeamSeats(*currentBearingPtr, bimSubstructurePtr, structuralSystemModelCP);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmBeamSeats(Bentley::ObmNET::BearingR bearingR, BridgeStructuralPhysical::GenericSubstructureElementPtr bimSubstructurePtr, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto beamSeatsPtr = bearingR.GetBeamSeats();
    if (beamSeatsPtr.IsValid())
        {
        while (beamSeatsPtr->MoveNext())
            {
            auto currentBeamSeatPtr = beamSeatsPtr->GetCurrent();
            CreateBimStructuralElement<Structural::StructuralMember>(*currentBeamSeatPtr, structuralSystemModelCP, bimSubstructurePtr->getP());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmWingWalls(Bentley::ObmNET::AbutmentWingWallCmpCollectionSubR abutmentWingWallSubcomponentCollectionR, BridgeStructuralPhysical::GenericSubstructureElementPtr bimSubstructurePtr, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto wingWallsPtr = abutmentWingWallSubcomponentCollectionR.GetWingWalls();
    if (wingWallsPtr.IsValid())
        {
        while (wingWallsPtr->MoveNext())
            {
            auto currentWingWallPtr = wingWallsPtr->GetCurrent();
            Bentley::WString properties = currentWingWallPtr->GetSerializedProperties();
            CreateBimStructuralElement<Structural::Wall>(*currentWingWallPtr, structuralSystemModelCP, bimSubstructurePtr->getP(), &properties);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmWingWallFootings(Bentley::ObmNET::AbutmentWingWallCmpCollectionSubR abutmentWingWallSubcomponentCollectionR, BridgeStructuralPhysical::GenericSubstructureElementPtr bimSubstructurePtr, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto wingWallFootingsPtr = abutmentWingWallSubcomponentCollectionR.GetWingWallFootings();
    if (wingWallFootingsPtr.IsValid())
        {
        while (wingWallFootingsPtr->MoveNext())
            {
            auto currentWingWallFootingPtr = wingWallFootingsPtr->GetCurrent();
            Bentley::WString properties = currentWingWallFootingPtr->GetSerializedProperties();
            CreateBimStructuralElement<Structural::StripFooting>(*currentWingWallFootingPtr, structuralSystemModelCP, bimSubstructurePtr->getP(), &properties);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmWingWallPiles(Bentley::ObmNET::AbutmentWingWallCmpCollectionSubR abutmentWingWallSubcomponentCollectionR, BridgeStructuralPhysical::GenericSubstructureElementPtr bimSubstructurePtr, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto wingWallPilesPtr = abutmentWingWallSubcomponentCollectionR.GetWingWallPiles();
    if (wingWallPilesPtr.IsValid())
        {
        while (wingWallPilesPtr->MoveNext())
            {
            auto currentWingWallPilePtr = wingWallPilesPtr->GetCurrent();
            Bentley::WString properties = currentWingWallPilePtr->GetSerializedProperties();
            CreateBimStructuralElement<Structural::Pile>(*currentWingWallPilePtr, structuralSystemModelCP, bimSubstructurePtr->getP(), &properties);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmPierColumns(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto pierColumnsPtr = pierSubcomponentR.GetPierColumns();
    if (pierColumnsPtr.IsValid())
        {
        while (pierColumnsPtr->MoveNext())
            {
            auto currentPierColumnPtr = pierColumnsPtr->GetCurrent();
            Bentley::WString properties = currentPierColumnPtr->GetSerializedProperties();
            CreateBimStructuralElement<Structural::Column>(*currentPierColumnPtr, structuralSystemModelCP, bimSubstructureR.getP(), &properties);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmPierCheekWalls(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto pierCheekWallsPtr = pierSubcomponentR.GetPierCheekWalls();
    if (pierCheekWallsPtr.IsValid())
        {
        while (pierCheekWallsPtr->MoveNext())
            {
            auto currentPierCheekWallPtr = pierCheekWallsPtr->GetCurrent();
            auto properties = currentPierCheekWallPtr->GetSerializedProperties();
            CreateBimStructuralElement<Structural::Wall>(*currentPierCheekWallPtr, structuralSystemModelCP, bimSubstructureR.getP(), &properties);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmPierCrashWalls(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto pierCrashWallsPtr = pierSubcomponentR.GetPierCrashWalls();
    if (pierCrashWallsPtr.IsValid())
        {
        while (pierCrashWallsPtr->MoveNext())
            {
            auto currentPierCrashWallPtr = pierCrashWallsPtr->GetCurrent();
            auto properties = currentPierCrashWallPtr->GetSerializedProperties();
            CreateBimStructuralElement<Structural::Beam>(*currentPierCrashWallPtr, structuralSystemModelCP, bimSubstructureR.getP(), &properties);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmPierCaps(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto pierCapsPtr = pierSubcomponentR.GetPierCaps();
    if (pierCapsPtr.IsValid())
        {
        while (pierCapsPtr->MoveNext())
            {
            auto currentPierCapPtr = pierCapsPtr->GetCurrent();
            auto properties = currentPierCapPtr->GetSerializedProperties();
            auto pierCapType = currentPierCapPtr->GetType();
            auto iterator = m_PierCapMap.find(pierCapType);
            if (iterator != m_PierCapMap.end())
                {
                auto structuralElementType = iterator->second;
                DetermineElementMappingAndCreate(structuralElementType, *currentPierCapPtr, *bimSubstructureR.getP(), structuralSystemModelCP, &properties);
                }
            else
                DetermineElementMappingAndCreate(StructuralElementType_Default, *currentPierCapPtr, *bimSubstructureR.getP(), structuralSystemModelCP, &properties);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmPierFootings(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto pierFootingsPtr = pierSubcomponentR.GetPierFootings();
    if (pierFootingsPtr.IsValid())
        {
        while (pierFootingsPtr->MoveNext())
            {
            auto currentPierFootingPtr = pierFootingsPtr->GetCurrent();
            auto properties = currentPierFootingPtr->GetSerializedProperties();
            auto pierFootingType = currentPierFootingPtr->GetType();
            auto iterator = m_PierFootingMap.find(pierFootingType);
            if (iterator != m_PierFootingMap.end())
                {
                auto structuralElementType = iterator->second;
                DetermineElementMappingAndCreate(structuralElementType, *currentPierFootingPtr, *bimSubstructureR.getP(), structuralSystemModelCP, &properties);
                }
            else
                DetermineElementMappingAndCreate(StructuralElementType_Default, *currentPierFootingPtr, *bimSubstructureR.getP(), structuralSystemModelCP, &properties);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmPierPiles(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto pierPilesPtr = pierSubcomponentR.GetPierPiles();
    if (pierPilesPtr.IsValid())
        {
        while (pierPilesPtr->MoveNext())
            {
            auto currentPierPilePtr = pierPilesPtr->GetCurrent();
            auto properties = currentPierPilePtr->GetSerializedProperties();
            CreateBimStructuralElement<Structural::Pile>(*currentPierPilePtr, structuralSystemModelCP, bimSubstructureR.getP());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmPierRockSockets(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto pierRockSocketsPtr = pierSubcomponentR.GetPierRockSockets();
    if (pierRockSocketsPtr.IsValid())
        {
        while (pierRockSocketsPtr->MoveNext())
            {
            auto currentPierRockSocketPtr = pierRockSocketsPtr->GetCurrent();
            auto properties = currentPierRockSocketPtr->GetSerializedProperties();
            CreateBimStructuralElement<Structural::StructuralMember>(*currentPierRockSocketPtr, structuralSystemModelCP, bimSubstructureR.getP());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmPierStruts(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto pierStrutsPtr = pierSubcomponentR.GetPierRockSockets();
    if (pierStrutsPtr.IsValid())
        {
        while (pierStrutsPtr->MoveNext())
            {
            auto currentPierStrutPtr = pierStrutsPtr->GetCurrent();
            auto properties = currentPierStrutPtr->GetSerializedProperties();
            CreateBimStructuralElement<Structural::Beam>(*currentPierStrutPtr, structuralSystemModelCP, bimSubstructureR.getP());
            }
        }
    }
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::CreateBimBridgeSuperstructures(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::BridgeR bimBridgeR, RoadRailAlignment::AlignmentCR bimAlignmentCR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    Dgn::DgnCategoryId bridgeCategoryId = BridgeStructuralPhysical::BridgeCategory::Get(m_converter.GetDgnDb());
    Dgn::GeometricElement3d::CreateParams createParams(m_converter.GetDgnDb(), structuralSystemModelCP->GetModelId(), BridgeStructuralPhysical::GenericSuperstructureElement::QueryClassId(m_converter.GetDgnDb()), bridgeCategoryId);
    //TODO: Get the appropriate distance expression for the superstructure element
    auto bimSuperstructureElementPtr = BridgeStructuralPhysical::GenericSuperstructureElement::Create(createParams, bimAlignmentCR, 0.0);
    if (bimSuperstructureElementPtr.IsValid())
        if (bimSuperstructureElementPtr->Insert().IsValid())
            {
            ProcessObmBeamGroups(bridgeUnitR, *bimSuperstructureElementPtr, structuralSystemModelCP);
            ProcessObmCrossFrameGroups(bridgeUnitR, *bimSuperstructureElementPtr, structuralSystemModelCP);
            ProcessObmSegmentalBalances(bridgeUnitR, *bimSuperstructureElementPtr, structuralSystemModelCP);
            ProcessObmSegmentalSpans(bridgeUnitR, *bimSuperstructureElementPtr, structuralSystemModelCP);
            ConvertObmDecks(bridgeUnitR, *bimSuperstructureElementPtr, structuralSystemModelCP);
            ConvertObmBarriers(bridgeUnitR, *bimSuperstructureElementPtr, structuralSystemModelCP);
            ConvertObmTendons(bridgeUnitR, *bimSuperstructureElementPtr, structuralSystemModelCP);
            }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::CreateBimElementGeometry(Bentley::ObmNET::BMSolidR obmElementR, BentleyM0200::Dgn::PhysicalElementR bimElementR, Dgn::DgnElementId& graphicsElementId, BentleyM0200::Utf8String& relClassName)
    {
    DgnFileP file = obmElementR.GetDgnModelP()->GetDgnFileP();
    WString  relName("RelBMSolidSolid");
    Bentley::ECN::ECRelationshipClassCP relClass = FindRelationshipClass(file, OBMNET_SCHEMANAME, relName.c_str());
    DgnPlatform::DgnECRelationshipIterable iterableECRelationship = FindRelationships(*obmElementR.GetDgnElementECInstanceP(), relClass, Bentley::ECN::STRENGTHDIRECTION_Forward);
    auto iterator = iterableECRelationship.begin();
    if (iterator == iterableECRelationship.end())
        {
        BeAssert(false);
        return;
        }
    auto relationshipInstance = *iterator;
    auto targetInstanceCP = relationshipInstance->GetTarget()->AsDgnECInstanceCP();
    auto targetElementRefP = targetInstanceCP->GetAsElementInstance()->GetElementRef();
    graphicsElementId = m_converter.GetBimElementFor(targetElementRefP);
    if (!graphicsElementId.IsValid())
        {
        BeAssert(false);
        return;
        }
    auto graphicsBimElmCP = bimElementR.GetDgnDb().Elements().GetElement(graphicsElementId)->ToGeometrySource3d();
    auto bimElementCategoryId = bimElementR.GetCategoryId();
    Dgn::DgnCategoryId categoryId = graphicsBimElmCP->GetCategoryId();
    bimElementR.SetCategoryId(categoryId);
    auto& bimGeomStreamCR = graphicsBimElmCP->GetGeometryStream();
    auto geomBuilderPtr = Dgn::GeometryBuilder::Create(*graphicsBimElmCP, bimGeomStreamCR);
    if (geomBuilderPtr->Finish(bimElementR) == BentleyStatus::ERROR)
        {
        BeAssert(false);
        return;
        }
    //TODO: Assign proper DgnCategroyId to the converted element
    //bimElementR.SetCategoryId(bimElementCategoryId);
    m_dgnCategoriesSeen.insert(categoryId);
    if (graphicsBimElmCP->Is2d())
        relClassName = BIS_REL_DrawingGraphicRepresentsElement;
    else
        relClassName = "GraphicalElement3dRepresentsElement";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ProcessObmBeamGroups(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto beamGroupsPtr = bridgeUnitR.GetBeamGroups();
    if (beamGroupsPtr.IsValid())
        {
        while (beamGroupsPtr->MoveNext())
            {
            auto currentBeamGroupPtr = beamGroupsPtr->GetCurrent();
            if (currentBeamGroupPtr.IsValid())
                {
                ProcessObmBeamSegmentCollectionSub(*currentBeamGroupPtr, bimSuperstructureR, structuralSystemModelCP);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmDecks(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto decksPtr = bridgeUnitR.GetDecks();
    if (decksPtr.IsValid())
        {
        while (decksPtr->MoveNext())
            {
            auto currentDeckPtr = decksPtr->GetCurrent();
            if (currentDeckPtr.IsValid())
                {
                auto bmSolidPtr = currentDeckPtr->GetBMSolid();
                if (bmSolidPtr.IsValid())
                    {
                    Structural::SlabPtr slabPtr = CreateBimStructuralElement2<Structural::Slab, Structural::SlabPtr>(*bmSolidPtr, structuralSystemModelCP, bimSuperstructureR.getP());
                    if (m_obmSchemaCreatorP != nullptr)
                        m_obmSchemaCreatorP->UpdateOBMApplicationSchema(currentDeckPtr->GetECInstance());
                    ConvertProperties(currentDeckPtr->GetECInstance(), *slabPtr);
                    slabPtr->Update();
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmBarriers(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto barriersPtr = bridgeUnitR.GetBarriers();
    if (barriersPtr.IsValid())
        {
        while (barriersPtr->MoveNext())
            {
            auto currentBarrierPtr = barriersPtr->GetCurrent();
            if (currentBarrierPtr.IsValid())
                {
                auto bmSolidPtr = currentBarrierPtr->GetBMSolid();
                if (bmSolidPtr.IsValid())
                    {
                    Dgn::DgnCategoryId bridgeCategoryId = BridgeStructuralPhysical::BridgeCategory::Get(m_converter.GetDgnDb());
                    Dgn::GenericPhysicalObjectPtr physicalObjectPtr = CreateBimStructuralElement1<Dgn::GenericPhysicalObject, Dgn::GenericPhysicalObjectPtr>(*bmSolidPtr, bimSuperstructureR.getP(), bridgeCategoryId, structuralSystemModelCP);
                    if (m_obmSchemaCreatorP != nullptr)
                        m_obmSchemaCreatorP->UpdateOBMApplicationSchema(currentBarrierPtr->GetECInstance());
                    ConvertProperties(currentBarrierPtr->GetECInstance(), *physicalObjectPtr);
                    physicalObjectPtr->Update();
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmTendons(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto tendonsPtr = bridgeUnitR.GetTendons();
    if (tendonsPtr.IsValid())
        {
        while (tendonsPtr->MoveNext())
            {
            auto currentTendonPtr = tendonsPtr->GetCurrent();
            auto bmSolidPtr = currentTendonPtr->GetBMSolid();
            CreateBimStructuralElement<Structural::StructuralMember>(*bmSolidPtr, structuralSystemModelCP, bimSuperstructureR.getP());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ProcessObmBeamSegmentCollectionSub(Bentley::ObmNET::BeamGroupR beamGroupR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto beamSegmentCollectionPtr = beamGroupR.GetBeamSegmentCollection();
    if (beamSegmentCollectionPtr.IsValid())
        {
        auto beamSegmentCollectionSubPtr = beamSegmentCollectionPtr->GetSubComponents();
        if (beamSegmentCollectionSubPtr.IsValid())
            {
            while (beamSegmentCollectionSubPtr->MoveNext())
                {
                auto beamSegmentSubComponentPtr = beamSegmentCollectionSubPtr->GetCurrent();
                if (beamSegmentSubComponentPtr.IsValid())
                    {
                    ConvertObmBeamSegments(*beamSegmentSubComponentPtr, bimSuperstructureR, structuralSystemModelCP);
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertObmBeamSegments(Bentley::ObmNET::BeamSegmentCollectionSubR beamSegmentSubComponentR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    //auto structuralSystemModelP = const_cast<Dgn::PhysicalModelP>(structuralSystemModelCP);
    auto beamSegmentsPtr = beamSegmentSubComponentR.GetBeamSegments();
    if (beamSegmentsPtr.IsValid())
        {
        while (beamSegmentsPtr->MoveNext())
            {
            auto currentBeamSegmentPtr = beamSegmentsPtr->GetCurrent();
            if (currentBeamSegmentPtr.IsValid())
                {
                auto beamSegmentType = currentBeamSegmentPtr->GetType();
                auto iterator = m_BeamSegmentMap.find(beamSegmentType);
                if (iterator != m_BeamSegmentMap.end())
                    {
                    auto structuralElementType = iterator->second;
                    DetermineElementMappingAndCreate(structuralElementType, *currentBeamSegmentPtr, *bimSuperstructureR.getP(), structuralSystemModelCP);
                    }
                else
                    {
                    DetermineElementMappingAndCreate(StructuralElementType_Default, *currentBeamSegmentPtr, *bimSuperstructureR.getP(), structuralSystemModelCP);
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::DetermineElementMappingAndCreate(StructuralElementType structuralElementType, Bentley::ObmNET::BMSolidR obmSolidR, Dgn::PhysicalElementR parentStructureR, Dgn::PhysicalModelCP structuralSystemModelCP, Bentley::WStringP serializedPropertiesP)
    {
    switch (structuralElementType)
        {
        case StructuralElementType_Beam:
            CreateBimStructuralElement<Structural::Beam>(obmSolidR, structuralSystemModelCP, &parentStructureR, serializedPropertiesP);
            break;
        case StructuralElementType_Brace:
            CreateBimStructuralElement<Structural::Brace>(obmSolidR, structuralSystemModelCP, &parentStructureR, serializedPropertiesP);
            break;
        case StructuralElementType_Column:
            CreateBimStructuralElement<Structural::Column>(obmSolidR, structuralSystemModelCP, &parentStructureR, serializedPropertiesP);
            break;
        case StructuralElementType_FoundationMember:
            CreateBimStructuralElement<Structural::FoundationMember>(obmSolidR, structuralSystemModelCP, &parentStructureR, serializedPropertiesP);
            break;
        case StructuralElementType_Pile:
            CreateBimStructuralElement<Structural::Pile>(obmSolidR, structuralSystemModelCP, &parentStructureR, serializedPropertiesP);
            break;
        case StructuralElementType_PileCap:
            CreateBimStructuralElement<Structural::PileCap>(obmSolidR, structuralSystemModelCP, &parentStructureR, serializedPropertiesP);
            break;
        case StructuralElementType_Slab:
            CreateBimStructuralElement<Structural::Slab>(obmSolidR, structuralSystemModelCP, &parentStructureR, serializedPropertiesP);
            break;
        case StructuralElementType_SpreadFooting:
            CreateBimStructuralElement<Structural::SpreadFooting>(obmSolidR, structuralSystemModelCP, &parentStructureR, serializedPropertiesP);
            break;
        case StructuralElementType_StripFooting:
            CreateBimStructuralElement<Structural::StripFooting>(obmSolidR, structuralSystemModelCP, &parentStructureR, serializedPropertiesP);
            break;
        case StructuralElementType_StructuralMember:
            CreateBimStructuralElement<Structural::StructuralMember>(obmSolidR, structuralSystemModelCP, &parentStructureR, serializedPropertiesP);
            break;
        case StructuralElementType_Wall:
            CreateBimStructuralElement<Structural::Wall>(obmSolidR, structuralSystemModelCP, &parentStructureR, serializedPropertiesP);
            break;
        case StructuralElementType_Default:
            CreateBimStructuralElement<Structural::StructuralMember>(obmSolidR, structuralSystemModelCP, &parentStructureR, serializedPropertiesP);
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::UpdateCategorySelector(Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto defaultViewId = m_converter.GetDefaultViewId();
    if (defaultViewId.IsValid())
        {
        auto viewDefCPtr = structuralSystemModelCP->GetDgnDb().Elements().Get<SpatialViewDefinition>(defaultViewId);
        auto categorySelectorPtr = structuralSystemModelCP->GetDgnDb().Elements().GetForEdit<CategorySelector>(viewDefCPtr->GetCategorySelectorId());
        for (auto categoryId : m_dgnCategoriesSeen)
            {
            categorySelectorPtr->AddCategory(categoryId);
            }
        categorySelectorPtr->Update();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::CreateGraphicsRelationship(BentleyM0200::Dgn::PhysicalElementR structuralBimElementR, Dgn::DgnElementId graphicsElementId, Utf8String relClassName)
    {
    ECInstanceKey insKey;
    if (relClassName.IsNullOrEmpty(relClassName.c_str()) || !graphicsElementId.IsValid())
        return;

    if (DbResult::BE_SQLITE_OK != structuralBimElementR.GetDgnDb().InsertLinkTableRelationship(insKey,
                                                                                     *structuralBimElementR.GetDgnDb().Schemas().GetClass(BIS_ECSCHEMA_NAME, relClassName)->GetRelationshipClassCP(),
                                                                                     ECInstanceId(graphicsElementId.GetValue()),
                                                                                     ECInstanceId(structuralBimElementR.GetElementId().GetValue())))
        {
        BeAssert(false);
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::GetBimBridgeFromToParams(Bentley::ObmNET::BridgeR bridgeR, double & start, double & end)
    {
    auto bridgeUnits = bridgeR.GetBridgeUnits();
    bvector<double> startLocations, endLocations;
    while (bridgeUnits->MoveNext())
        {
        auto currentBridgeUnit = bridgeUnits->GetCurrent();
        startLocations.push_back(currentBridgeUnit->GetStartSupportLine()->GetDistAlong());
        endLocations.push_back(currentBridgeUnit->GetEndSupportLine()->GetDistAlong());
        }

    double minStart = startLocations.at(0);
    double maxEnd = endLocations.at(0);
    for (int i = 0; i < startLocations.size(); i++)
        {
        if (startLocations.at(i) < minStart)
            minStart = startLocations.at(i);

        if (endLocations.at(i) > maxEnd)
            maxEnd = endLocations.at(i);
        }

    start = minStart;
    end = maxEnd;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertSerializedProperties(Bentley::WString serializedProperties, Dgn::GeometricElement3d& geometricElementR)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr elementPropertiesXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, serializedProperties.c_str(), serializedProperties.size());
    BeXmlNodeP elementPropertiesRootNodeP = elementPropertiesXmlDom->GetRootElement();
    if (elementPropertiesRootNodeP == nullptr)
        return;

    Utf8String aspectName = elementPropertiesRootNodeP->GetName();
    auto aspectNameCopy = aspectName.copy();
    aspectName.append("Aspect");
    StandaloneECInstancePtr  aspectInstancePtr;
    auto ecClassP = m_converter.GetDgnDb().Schemas().GetClass(OBM_SCHEMA_NAME, aspectName);
    if (ecClassP != nullptr)
        aspectInstancePtr = ecClassP->GetDefaultStandaloneEnabler()->CreateInstance();
    if (aspectInstancePtr == nullptr)
        return;

    BeXmlNodeP currentNode = elementPropertiesRootNodeP->GetFirstChild();
    Utf8CP propertyName;
    Utf8String propertyNameUtf8String;
    Utf8String propertyValue;
    do
        {
        propertyName = currentNode->GetName();
        propertyNameUtf8String = Utf8String(propertyName);
        if (propertyNameUtf8String.Contains("ID"))
            {
            aspectNameCopy.append(Utf8String("ID"));
            propertyName = aspectNameCopy.c_str();
            }
        currentNode->GetContent(propertyValue, nullptr);
        
        BentleyM0200::ECN::ECValue ecPropertyValue = CreatePropertyECValue(propertyName, propertyValue);
        aspectInstancePtr->SetValue(propertyName, ecPropertyValue);
        currentNode = (BeXmlNodeP) currentNode->next;
        } while (currentNode != nullptr);
    Dgn::DgnElement::GenericUniqueAspect::SetAspect(geometricElementR, *aspectInstancePtr, ecClassP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertSerializedProperties(Bentley::WString serializedProperties, Dgn::GeometricElement3d& geometricElementR, BentleyM0200::Utf8String parentAspectName, StandaloneECInstancePtr parentAspectInstancePtr)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr elementPropertiesXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, serializedProperties.c_str(), serializedProperties.size());
    BeXmlNodeP elementPropertiesRootNodeP = elementPropertiesXmlDom->GetRootElement();
    if (elementPropertiesRootNodeP == nullptr)
        return;

    Utf8String aspectName = elementPropertiesRootNodeP->GetName();
    auto aspectNameCopy = aspectName.copy();
    aspectName.append("Aspect");
    StandaloneECInstancePtr  aspectInstancePtr;
    BentleyM0200::ECN::ECClassCP ecClassP = nullptr;
    if (!aspectName.Equals(parentAspectName))
        {
        ecClassP = m_converter.GetDgnDb().Schemas().GetClass(OBM_SCHEMA_NAME, aspectName);
        if (ecClassP != nullptr)
            aspectInstancePtr = ecClassP->GetDefaultStandaloneEnabler()->CreateInstance();
        }
    else
        aspectInstancePtr = parentAspectInstancePtr;

    if (aspectInstancePtr == nullptr)
        return;

    BeXmlNodeP currentNode = elementPropertiesRootNodeP->GetFirstChild();
    Utf8CP propertyName;
    Utf8String propertyNameUtf8String;
    Utf8String propertyValue;
    do
        {
        propertyName = currentNode->GetName();
        propertyNameUtf8String = Utf8String(propertyName);
        if (propertyNameUtf8String.Contains("ID"))
            {
            aspectNameCopy.append(Utf8String("ID"));
            propertyName = aspectNameCopy.c_str();
            }
        currentNode->GetContent(propertyValue, nullptr);

        BentleyM0200::ECN::ECValue ecPropertyValue = CreatePropertyECValue(propertyName, propertyValue);
        aspectInstancePtr->SetValue(propertyName, ecPropertyValue);
        currentNode = (BeXmlNodeP) currentNode->next;
        } while (currentNode != nullptr);
        Dgn::DgnElement::GenericUniqueAspect::SetAspect(geometricElementR, *aspectInstancePtr, ecClassP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OBMConverter::ConvertProperties(Bentley::DgnECInstanceCR ecInstanceCR, Dgn::GeometricElement3d& geometricalElementR)
    {
    DgnECInstanceR ecInstance = const_cast<DgnECInstanceR>(ecInstanceCR);

    auto ecClassName = ecInstance.GetClass().GetName();
    auto aspectName = ecClassName.append(WString("Aspect").c_str());
    auto ecClassP = m_converter.GetDgnDb().Schemas().GetClass(OBM_SCHEMA_NAME, BentleyM0200::Utf8String(aspectName.c_str()));
    if (ecClassP == nullptr)
        return;

    StandaloneECInstancePtr aspectInstancePtr = ecClassP->GetDefaultStandaloneEnabler()->CreateInstance();
    if (aspectInstancePtr.IsNull())
        return;

    Bentley::ECN::ECPropertyIterable ecProperties = ecInstance.GetClass().GetProperties(false);
    for (Bentley::ECN::ECPropertyIterable::const_iterator propertyIterator = ecProperties.begin(); propertyIterator != ecProperties.end(); ++propertyIterator)
        {
        auto propertyName = (*propertyIterator)->GetName();
        auto propertyType = (*propertyIterator)->GetTypeName();
        auto propertyDisplayLabel = (*propertyIterator)->GetDisplayLabel();
        Bentley::ECN::ECValue v;
        ecInstance.GetValue(v, propertyName.c_str());
        auto valueP = &v;
        auto valueB0200P = (BentleyM0200::ECN::ECValueP)valueP;
        auto value = v.ToString();
        if (propertyName.Contains(Bentley::WString("SerializedProperties")))
            {
            ConvertSerializedProperties(value, geometricalElementR, BentleyM0200::Utf8String(aspectName.c_str()), aspectInstancePtr);
            }
        else if (propertyName.Contains(Bentley::WString("AttributesXmlFragment")))
            {
            ConvertSerializedProperties(value, geometricalElementR);
            }
        else
            {
            auto propNameUtf8 = Utf8String(propertyName.c_str());
            aspectInstancePtr->SetValue(propNameUtf8.c_str(), *valueB0200P);
            }
        }

    Dgn::DgnElement::GenericUniqueAspect::SetAspect(geometricalElementR, *aspectInstancePtr, ecClassP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyM0200::ECN::ECValue OBMConverter::CreatePropertyECValue(BentleyM0200::Utf8String propertyName, BentleyM0200::Utf8String propertyValue)
    {
    Bentley::Utf8String propertyType = ObmApplicationSchemaCreator::DeterminePropertyType(propertyName);
    if (propertyType.Equals("double"))
        {
        double value = std::stod(propertyValue.c_str());
        return ECValue(value);
        }
    else
        return ECValue(propertyValue.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::ECN::ECRelationshipClassCP OBMConverter::FindRelationshipClass(DgnFileP dgnFile, WCharCP schemaName, WCharCP className)
    {
    Bentley::ECN::ECClassCP ecClass = FindClass(dgnFile, schemaName, className);
    if (nullptr == ecClass)
        {
        return nullptr;
        }
    return static_cast<Bentley::ECN::ECRelationshipClassCP>(ecClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::ECN::ECClassCP OBMConverter::FindClass(DgnFileP dgnFile, WCharCP schemaName, WCharCP className)
    {
    auto& mgr = DgnPlatform::DgnECManager::GetManager();
    DgnPlatform::SchemaInfo schemaInfo(Bentley::ECN::SchemaKey(schemaName, (uint32_t) CIF_MAJOR_SCHEMA_VERSION_NO, (uint32_t) CIF_MINOR_SCHEMA_VERSION_NO), *dgnFile);
    Bentley::ECN::ECSchemaPtr schema = mgr.LocateSchemaInDgnFile(schemaInfo, Bentley::ECN::SCHEMAMATCHTYPE_Latest);
    return schema->GetClassCP(className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatform::DgnECRelationshipIterable OBMConverter::FindRelationships(DgnECInstanceCR instance, Bentley::ECN::ECRelationshipClassCP relClass, Bentley::ECN::ECRelatedInstanceDirection dir)
    {
    auto& mgr = DgnPlatform::DgnECManager::GetManager();
    DgnPlatform::QueryRelatedClassSpecifierPtr classSpec = DgnPlatform::QueryRelatedClassSpecifier::Create(*relClass, nullptr, dir);
    return mgr.FindRelationships(instance, *classSpec, Bentley::Cif::Dgn::CifDgnECManager::GetQualifiedPepCreateContext());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename BimElementType>
void OBMConverter::CreateBimStructuralElement(Bentley::ObmNET::BMSolidR obmSolidR, Dgn::PhysicalModelCP structuralSystemModelCP, Dgn::PhysicalElementPtr parentStructureP, Bentley::WStringP serializedPropertiesP)
    {
    auto structuralSystemModelP = const_cast<Dgn::PhysicalModelP>(structuralSystemModelCP);
    auto bimElementPtr = BimElementType::Create(*structuralSystemModelP);
    if (bimElementPtr.IsValid())
        {
        if(parentStructureP != nullptr)
            bimElementPtr->SetParentId(parentStructureP->GetElementId(), parentStructureP->GetElementClassId());

        Dgn::DgnElementId graphicsElementId;
        Utf8String relClassName;
        CreateBimElementGeometry(obmSolidR, *bimElementPtr, graphicsElementId, relClassName);
        bimElementPtr->Insert();

        if (m_obmSchemaCreatorP != nullptr)
            m_obmSchemaCreatorP->UpdateOBMApplicationSchema(obmSolidR.GetECInstance());

        auto graphicalElementPtr = parentStructureP->GetDgnDb().Elements().GetForEdit<GraphicalElement3d>(graphicsElementId);
        ConvertProperties(obmSolidR.GetECInstance(), *graphicalElementPtr);
        graphicalElementPtr->Update();
        
        CreateGraphicsRelationship(*bimElementPtr, graphicsElementId, relClassName);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename BimElementType, typename BimElementTypePtr>
BimElementTypePtr OBMConverter::CreateBimStructuralElement1(Bentley::ObmNET::BMSolidR obmSolidR, Dgn::PhysicalElementP parentStructureP, Dgn::DgnCategoryId categoryId, Dgn::PhysicalModelCP structuralSystemModelCP)
    {
    auto structuralSystemModelP = const_cast<Dgn::PhysicalModelP>(structuralSystemModelCP);
    auto bimElementPtr = BimElementType::Create(*structuralSystemModelP, categoryId);
    if (bimElementPtr.IsValid())
        {
        if(parentStructureP != nullptr)
            bimElementPtr->SetParentId(parentStructureP->GetElementId(), parentStructureP->GetElementClassId());
        Dgn::DgnElementId graphicsElementId;
        Utf8String relClassName;
        CreateBimElementGeometry(obmSolidR, *bimElementPtr, graphicsElementId, relClassName);
        bimElementPtr->Insert();
        CreateGraphicsRelationship(*bimElementPtr, graphicsElementId, relClassName);
        }
    return bimElementPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename BimElementType, typename BimElementTypePtr>
BimElementTypePtr OBMConverter::CreateBimStructuralElement2(Bentley::ObmNET::BMSolidR obmSolidR, Dgn::PhysicalModelCP structuralSystemModelCP, Dgn::PhysicalElementPtr parentStructurePtr, Bentley::WStringP serializedPropertiesP)
    {
    auto structuralSystemModelP = const_cast<Dgn::PhysicalModelP>(structuralSystemModelCP);
    auto bimElementPtr = BimElementType::Create(*structuralSystemModelP);
    if (bimElementPtr.IsValid())
        {
        if (parentStructurePtr != nullptr)
            bimElementPtr->SetParentId(parentStructurePtr->GetElementId(), parentStructurePtr->GetElementClassId());

        Dgn::DgnElementId graphicsElementId;
        Utf8String relClassName;
        CreateBimElementGeometry(obmSolidR, *bimElementPtr, graphicsElementId, relClassName);
        bimElementPtr->Insert();

        if (m_obmSchemaCreatorP != nullptr)
            m_obmSchemaCreatorP->UpdateOBMApplicationSchema(obmSolidR.GetECInstance());

        auto graphicalElementPtr = structuralSystemModelP->GetDgnDb().Elements().GetForEdit<GraphicalElement3d>(graphicsElementId);
        ConvertProperties(obmSolidR.GetECInstance(), *graphicalElementPtr);
        graphicalElementPtr->Update();
        
        CreateGraphicsRelationship(*bimElementPtr, graphicsElementId, relClassName);
        }
    return bimElementPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ObmApplicationSchemaCreator::ObmApplicationSchemaCreator(BentleyM0200::BeFileName schemaFileName) : m_schemaFileName(schemaFileName)
    {
    m_xmlDom = BeXmlDom::CreateEmpty();
    //InitializeSchema();

    InitializePhrases();
    m_propertiesToIgnore.insert("AssociatedComponents");
    m_propertiesToIgnore.insert("Sections");

    WString schemaFileNameCopy = WString(schemaFileName.copy());
    schemaFileNameCopy.ReplaceAll(WCharP(L".ecschema.xml"), WCharP(L""));
    schemaFileNameCopy.AppendA("_Updated.ecschema.xml");
    m_updatedSchemaFileName = BeFileName(schemaFileNameCopy);
    ReadSchemaFromFile();
    InitializeLocalizableStringsMap();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ObmApplicationSchemaCreator::InitializeSchema()
    {
    m_rootNodeP = m_xmlDom->AddNewElement("ECSchema", nullptr, nullptr);
    m_rootNodeP->AddAttributeStringValue("schemaName", "OpenBridgeModelerCE");
    m_rootNodeP->AddAttributeStringValue("alias", "obmce");
    m_rootNodeP->AddAttributeStringValue("version", "01.00.00");
    m_rootNodeP->AddAttributeStringValue("description", "Application schema for data specific to OpenBridgeModelerCE.");
    m_rootNodeP->AddAttributeStringValue("xmlns", "http://www.bentley.com/schemas/Bentley.ECXML.3.1");

    m_currentNodeP = m_xmlDom->AddNewElement("ECSchemaReference", nullptr, m_rootNodeP);
    m_currentNodeP->AddAttributeStringValue("name", "CoreCustomAttributes");
    m_currentNodeP->AddAttributeStringValue("version", "01.00.00");
    m_currentNodeP->AddAttributeStringValue("alias", "CoreCA");

    m_currentNodeP = m_xmlDom->AddNewElement("ECSchemaReference", nullptr, m_rootNodeP);
    m_currentNodeP->AddAttributeStringValue("name", "BisCore");
    m_currentNodeP->AddAttributeStringValue("version", "01.00.00");
    m_currentNodeP->AddAttributeStringValue("alias", "bis");

    m_currentNodeP = m_xmlDom->AddNewElement("ECSchemaReference", nullptr, m_rootNodeP);
    m_currentNodeP->AddAttributeStringValue("name", "StructuralPhysical");
    m_currentNodeP->AddAttributeStringValue("version", "01.00.00");
    m_currentNodeP->AddAttributeStringValue("alias", "sp");

    m_currentNodeP = m_xmlDom->AddNewElement("KindOfQuantity", nullptr, m_rootNodeP);
    m_currentNodeP->AddAttributeStringValue("typeName", "LENGTH");
    m_currentNodeP->AddAttributeStringValue("displayLabel", "Bridge Length");
    m_currentNodeP->AddAttributeStringValue("persistenceUnit", "M(DefaultReal)");
    m_currentNodeP->AddAttributeStringValue("presentationUnits", "M(real2u);FT(real2u)");
    m_currentNodeP->AddAttributeStringValue("relativeError", "0.0001");
    
    m_xmlDom->ToFile(WCharP(L"C:/Users/Matt.Balnis/Documents/Demos/Civil Sprint Review 11-2-2018/schemaGenerationTest.xml"), (BentleyM0200::BeXmlDom::ToStringOption)(BentleyM0200::BeXmlDom::TO_STRING_OPTION_Formatted | BentleyM0200::BeXmlDom::TO_STRING_OPTION_Indent), BentleyM0200::BeXmlDom::FILE_ENCODING_Utf8);
    }

BentleyM0200::bset<BentleyM0200::Utf8String> ObmApplicationSchemaCreator::s_doublePropertyPhrases = BentleyM0200::bset<BentleyM0200::Utf8String>();
BentleyM0200::bset<BentleyM0200::Utf8String> ObmApplicationSchemaCreator::s_stringPropertyPhrases = BentleyM0200::bset<BentleyM0200::Utf8String>();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ObmApplicationSchemaCreator::InitializePhrases()
    {
    s_doublePropertyPhrases.insert("Length");
    s_doublePropertyPhrases.insert("Height");
    s_doublePropertyPhrases.insert("Width");
    s_doublePropertyPhrases.insert("Depth");
    s_doublePropertyPhrases.insert("Margin");
    s_doublePropertyPhrases.insert("Offset");
    s_doublePropertyPhrases.insert("Thickness");
    s_doublePropertyPhrases.insert("Elevation");
    s_doublePropertyPhrases.insert("Radius");
    s_doublePropertyPhrases.insert("W1");
    s_doublePropertyPhrases.insert("W2");
    s_doublePropertyPhrases.insert("D1");
    s_doublePropertyPhrases.insert("D2");
    s_doublePropertyPhrases.insert("Angle");
    s_doublePropertyPhrases.insert("Weight");
    s_doublePropertyPhrases.insert("Overhang");
    s_doublePropertyPhrases.insert("SegmentStart");
    s_doublePropertyPhrases.insert("DistanceFrom");

    s_stringPropertyPhrases.insert("Type");

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ObmApplicationSchemaCreator::InitializeLocalizableStringsMap()
    {
    BeXmlStatus xmlStatus;
    //TODO: Find a place to put the localizable strings map
    BeXmlDomPtr localizableStringsXmlPtr = BeXmlDom::CreateAndReadFromFile(xmlStatus, L"C:/Users/Matt.Balnis/Documents/Demos/Civil Sprint Review 11-2-2018/LocalizableStrings.xml");
    auto rootNode = localizableStringsXmlPtr->GetRootElement();
    auto xPathContextPtr = localizableStringsXmlPtr->AcquireXPathContext(rootNode);
    BentleyM0200::BeXmlDom::IterableNodeSet iterableXmlNodes;
    localizableStringsXmlPtr->SelectNodes(iterableXmlNodes, "//*", xPathContextPtr);
    for (BeXmlNodeP& currentNodeP : iterableXmlNodes)
        {
        BentleyM0200::Utf8String currentNodeType = currentNodeP->GetName();
        if (!currentNodeType.Equals("data"))
            continue;

        BentleyM0200::Utf8String currentNodeName;
        currentNodeP->GetAttributeStringValue(currentNodeName, "name");
        if (currentNodeName.IsNullOrEmpty(currentNodeName.c_str()))
            continue;

        auto childNodeP = currentNodeP->GetFirstChild();
        BentleyM0200::Utf8String value;
        childNodeP->GetContent(value);
        m_localizableStringsMap.Insert(currentNodeName, value);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BeXmlNodeP ObmApplicationSchemaCreator::AddNewAspect(BeXmlNodeP parentNodeP, Bentley::Utf8String aspectName, Bentley::Utf8String aspectModifier, Bentley::Utf8String aspectDisplayLabel, Bentley::Utf8String aspectDescription)
    {
    if ((m_elementTypeNames.find(aspectName) != m_elementTypeNames.end()) || DoesSchemaContainAspect(Bentley::WString(aspectName.c_str())))
        {
        return nullptr;
        }

    m_xmlDom->ToFile(WCharP(L"C:/Users/Matt.Balnis/Documents/Demos/Civil Sprint Review 11-2-2018/test.xml"), (BentleyM0200::BeXmlDom::ToStringOption)(BentleyM0200::BeXmlDom::TO_STRING_OPTION_Formatted | BentleyM0200::BeXmlDom::TO_STRING_OPTION_Indent), BentleyM0200::BeXmlDom::FILE_ENCODING_Utf8);

    m_elementTypeNames.insert(aspectName);

    m_currentNodeP = m_xmlDom->AddNewElement("ECEntityClass", nullptr, parentNodeP);
    m_currentNodeP->AddAttributeStringValue("typeName", aspectName.c_str());
    m_currentNodeP->AddAttributeStringValue("modifier", aspectModifier.c_str());
    m_currentNodeP->AddAttributeStringValue("displayLabel", aspectDisplayLabel.c_str());
    m_currentNodeP->AddAttributeStringValue("description", aspectDescription.c_str());

    m_xmlDom->AddNewElement("BaseClass", WCharCP(L"bis:ElementUniqueAspect"), m_currentNodeP);

    BeXmlNodeP relationshipNodeP = m_xmlDom->AddNewElement("ECRelationshipClass", nullptr, parentNodeP);
    Utf8String relationshipTypeName = Utf8String("ElementOwns");
    relationshipTypeName.append(aspectName.c_str());
    relationshipNodeP->AddAttributeStringValue("typeName", relationshipTypeName.c_str());
    relationshipNodeP->AddAttributeStringValue("strength", "embedding");
    relationshipNodeP->AddAttributeStringValue("modifier", aspectModifier.c_str());

    BeXmlNodeP relationshipChildNodeP = m_xmlDom->AddNewElement("BaseClass", WCharCP(L"bis:ElementOwnsUniqueAspect"), relationshipNodeP);
    relationshipChildNodeP = m_xmlDom->AddNewElement("Source", nullptr, relationshipNodeP);
    relationshipChildNodeP->AddAttributeStringValue("multiplicity", "(1..1)");
    relationshipChildNodeP->AddAttributeStringValue("polymorphic", "true");
    relationshipChildNodeP->AddAttributeStringValue("roleLabel", "owns");
    BeXmlNodeP relationshipChildChildNodeP = m_xmlDom->AddNewElement("Class", nullptr, relationshipChildNodeP);
    relationshipChildChildNodeP->AddAttributeStringValue("class", "sp:StructuralMember");

    relationshipChildNodeP = m_xmlDom->AddNewElement("Target", nullptr, relationshipNodeP);
    relationshipChildNodeP->AddAttributeStringValue("multiplicity", "(0..1)");
    relationshipChildNodeP->AddAttributeStringValue("polymorphic", "false");
    relationshipChildNodeP->AddAttributeStringValue("roleLabel", "is owned by");
    relationshipChildChildNodeP = m_xmlDom->AddNewElement("Class", nullptr, relationshipChildNodeP);
    relationshipChildChildNodeP->AddAttributeStringValue("class", aspectName.c_str());

    WriteXmlDomToFile();

    return m_currentNodeP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ObmApplicationSchemaCreator::AddNewAspectProperty(BeXmlNodeP parentNodeP, Bentley::Utf8String propertyName, Bentley::Utf8String propertyTypeName, Bentley::Utf8String propertyDisplayLabel, Bentley::Utf8String kindOfQuantity)
    {
    if (DoesAspectContainProperty(propertyName.c_str(), parentNodeP) == true)
        return;

    BeXmlNodeP propertyNodeP = m_xmlDom->AddNewElement("ECProperty", nullptr, parentNodeP);
    propertyNodeP->AddAttributeStringValue("propertyName", propertyName.c_str());
    propertyNodeP->AddAttributeStringValue("typeName", propertyTypeName.c_str());
    propertyNodeP->AddAttributeStringValue("displayLabel", propertyDisplayLabel.c_str());

    if(!kindOfQuantity.IsNullOrEmpty(kindOfQuantity.c_str()))
        propertyNodeP->AddAttributeStringValue("kindOfQuantity", kindOfQuantity.c_str());
    
    WriteXmlDomToFile();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BeXmlNodeP ObmApplicationSchemaCreator::FindAspectByName(BentleyM0200::Utf8String aspectName)
    {
    /*auto xPathContextPtr =*/ m_xmlDom->AcquireXPathContext(NULL);
    m_xmlDom->GetRootElement();
    auto iter = m_aspectNameToSchemaNodeMap.find(aspectName);
    if (iter != m_aspectNameToSchemaNodeMap.end())
        {
        return iter->second;
        }

    /*
    BentleyM0200::BeXmlDom::IterableNodeSet iterableXmlNodes;
    m_xmlDom->SelectNodes(iterableXmlNodes, "//*", xPathContextPtr);
    for (BeXmlNodeP& currentSchemaNodeP : iterableXmlNodes)
        {
        BentleyM0200::Utf8String currentAspectName;
        currentSchemaNodeP->GetAttributeStringValue(currentAspectName, "typeName");
        if (currentAspectName.Contains(aspectName))
            return currentSchemaNodeP;
        }
        */
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ObmApplicationSchemaCreator::ReadSchemaFromFile()
    {
    BeXmlStatus xmlStatus;
    m_xmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, m_schemaFileName.c_str());
    m_rootNodeP = m_xmlDom->GetRootElement();

    auto xPathContextPtr = m_xmlDom->AcquireXPathContext(m_rootNodeP);
    BentleyM0200::BeXmlDom::IterableNodeSet iterableXmlNodes;
    m_xmlDom->SelectNodes(iterableXmlNodes, "//*", xPathContextPtr);
    for (BeXmlNodeP& currentSchemaNodeP : iterableXmlNodes)
        {
        BentleyM0200::Utf8String nodeName = currentSchemaNodeP->GetName();
        if (!nodeName.Equals("ECEntityClass"))
            continue;

        BentleyM0200::Utf8String currentAspectName;
        currentSchemaNodeP->GetAttributeStringValue(currentAspectName, "typeName");
        if (!currentAspectName.IsNullOrEmpty(currentAspectName.c_str()))
            m_aspectNameToSchemaNodeMap.Insert(currentAspectName, currentSchemaNodeP);
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ObmApplicationSchemaCreator::WriteXmlDomToFile()
    {
    /* Updated OBM Application schema gets written to {AssetsDir}\ECSchemas\Application\OpenBridgeModelerCE_Updated.ecschema.xml */
    m_xmlDom->ToFile(m_updatedSchemaFileName.c_str(), (BentleyM0200::BeXmlDom::ToStringOption)(BentleyM0200::BeXmlDom::TO_STRING_OPTION_Formatted | BentleyM0200::BeXmlDom::TO_STRING_OPTION_Indent), BentleyM0200::BeXmlDom::FILE_ENCODING_Utf8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ObmApplicationSchemaCreator::DoesSchemaContainAspect(Bentley::WString aspectName)
    {
    BentleyM0200::WString schemaXmlString("");
    Bentley::WString searchString("\"");
    searchString.append(aspectName);
    searchString.append(Bentley::WString("\""));
    m_xmlDom->ToString(schemaXmlString, (BentleyM0200::BeXmlDom::ToStringOption)(BentleyM0200::BeXmlDom::TO_STRING_OPTION_Formatted | BentleyM0200::BeXmlDom::TO_STRING_OPTION_Indent));

    if (schemaXmlString.Contains(searchString.c_str()))
        return true;
    else
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ObmApplicationSchemaCreator::DoesAspectContainProperty(BentleyM0200::Utf8String propertyName, BeXmlNodeP aspectNodeP)
    {
    BentleyM0200::Utf8String aspectPropertyName;
    BeXmlNodeP childNodeP = aspectNodeP->GetFirstChild();
    
    while (childNodeP != nullptr)
        {
        childNodeP->GetAttributeStringValue(aspectPropertyName, "propertyName");
        if (aspectPropertyName.Equals(propertyName))
            return true;
        childNodeP = childNodeP->GetNextSibling();
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::Utf8String ObmApplicationSchemaCreator::DetermineKindOfQuantity(BentleyM0200::Utf8String propertyName, BentleyM0200::Utf8String propertyTypeName)
    {
    if (propertyTypeName.Contains("double"))
        {
        if (propertyName.Contains("Angle"))
            return "rralign:ANGLE";
        else if (propertyName.Contains("Weight"))
            return "";
        else
            return "LENGTH";
        }
    else    
        return "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::Utf8String ObmApplicationSchemaCreator::DeterminePropertyType(BentleyM0200::Utf8String propertyName)
    {
    for (BentleyM0200::Utf8String stringPhrase : s_stringPropertyPhrases)
        {
        if (propertyName.Contains(stringPhrase))
            return "string";
        }
    for (BentleyM0200::Utf8String doublePhrase : s_doublePropertyPhrases)
        {
        if (propertyName.Contains(doublePhrase))
            return "double";
        }
    return "string";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::Utf8String ObmApplicationSchemaCreator::DetermineDisplayLabel(BentleyM0200::Utf8String propertyName)
    {
    BentleyM0200::Utf8String displayLabel = "";
    bool endsWithID = false;
    Utf8Char* spaceChar(" ");
    if (propertyName.EndsWith("ID"))
        {
        endsWithID = true;
        propertyName.ReplaceAll("ID", "");
        }

    propertyName.ReplaceAll("_", "");
    
    auto iterator = m_localizableStringsMap.find(propertyName);
    if (iterator != m_localizableStringsMap.end() && !endsWithID)
        {
        displayLabel = iterator->second;
        return displayLabel.c_str();
        }

    BentleyM0200::Utf8P propertyNameP = propertyName.begin();

    while (propertyNameP != propertyName.end())
        {
        if (std::isupper(*(propertyNameP), std::locale::classic()) && propertyNameP != propertyName.begin() && (*(propertyNameP-1) != *spaceChar))
            {
            displayLabel.append(" ");
            }
        displayLabel.append(BentleyM0200::Utf8String(1, *propertyNameP));
        propertyNameP++;
        }

    if (endsWithID)
        displayLabel.append(" ID");

    displayLabel.ReplaceAll("Prop ", "");
    
    return Bentley::Utf8String(displayLabel.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ObmApplicationSchemaCreator::ApplySchemaFormatting()
    {
    auto xPathContextPtr = m_xmlDom->AcquireXPathContext(NULL);

    BentleyM0200::BeXmlDom::IterableNodeSet iterableXmlNodes;
    m_xmlDom->SelectNodes(iterableXmlNodes, "//text()", xPathContextPtr);
    for (BeXmlNodeP& currentSchemaNode : iterableXmlNodes)
        {
        BentleyM0200::Utf8String nodeContent;
        currentSchemaNode->GetContent(nodeContent);
        if (nodeContent.Contains("\n"))
            {
            auto parentNodeP = currentSchemaNode->GetParentNode();
            parentNodeP->RemoveChildNode(currentSchemaNode);
            }
        }
    WriteXmlDomToFile();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ObmApplicationSchemaCreator::ParseSerializedProperties(Bentley::WString serializedProperties)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr elementPropertiesXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, serializedProperties.c_str(), serializedProperties.size());
    BeXmlNodeP elementPropertiesRootNodeP = elementPropertiesXmlDom->GetRootElement();
    if (elementPropertiesRootNodeP == nullptr)
        return;

    Utf8String aspectName = elementPropertiesRootNodeP->GetName();
    auto aspectNameCopy = aspectName.copy();
    aspectNameCopy.append("Aspect");
    BeXmlNodeP aspectNode = AddNewAspect(m_rootNodeP, aspectNameCopy.c_str(), Bentley::Utf8String("sealed"), elementPropertiesRootNodeP->GetName(), elementPropertiesRootNodeP->GetName());
    if (aspectNode == nullptr)
        return;
    BeXmlNodeP currentNode = elementPropertiesRootNodeP->GetFirstChild();

    Utf8CP propertyName;
    Utf8String propertyNameUtf8String;
    Utf8String propertyValue;
    do
        {
        propertyName = currentNode->GetName();
        propertyNameUtf8String = Utf8String(propertyName);
        if (m_propertiesToIgnore.end() != m_propertiesToIgnore.find(propertyNameUtf8String))
            {
            currentNode = (BeXmlNodeP) currentNode->next;
            continue;
            }
            
        if (propertyNameUtf8String.Contains("ID"))
            {
            aspectName.append(Utf8String("ID"));
            propertyName = aspectName.c_str();
            }
        Bentley::Utf8String propertyType = DeterminePropertyType(propertyName);
        Bentley::Utf8String kindOfQuantity = DetermineKindOfQuantity(propertyNameUtf8String, propertyType.c_str());
        Bentley::Utf8String displayLabel = DetermineDisplayLabel(propertyName);
        AddNewAspectProperty(aspectNode, propertyName, propertyType, displayLabel, kindOfQuantity);
        currentNode = (BeXmlNodeP) currentNode->next;
        } while (currentNode != nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ObmApplicationSchemaCreator::ParseSerializedProperties(Bentley::WString serializedProperties, BentleyM0200::Utf8String parentAspectName, BeXmlNodeP parentAspectNodeP)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr elementPropertiesXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, serializedProperties.c_str(), serializedProperties.size());
    BeXmlNodeP elementPropertiesRootNodeP = elementPropertiesXmlDom->GetRootElement();
    if (elementPropertiesRootNodeP == nullptr)
        return;

    BeXmlNodeP aspectNodeP;
    Utf8String aspectName = elementPropertiesRootNodeP->GetName();
    auto aspectNameCopy = aspectName.copy();
    aspectNameCopy.append("Aspect");
    if (!parentAspectName.Equals(aspectNameCopy.c_str()))
        {
        aspectNodeP = AddNewAspect(m_rootNodeP, aspectNameCopy.c_str(), Bentley::Utf8String("sealed"), elementPropertiesRootNodeP->GetName(), elementPropertiesRootNodeP->GetName());
        }
    else
        {
        aspectNodeP = parentAspectNodeP;
        }

    if (aspectNodeP == nullptr)
        return;
    BeXmlNodeP currentNode = elementPropertiesRootNodeP->GetFirstChild();

    Utf8CP propertyName;
    Utf8String propertyNameUtf8String;
    Utf8String propertyValue;
    do
        {
        propertyName = currentNode->GetName();
        propertyNameUtf8String = Utf8String(propertyName);
        if (m_propertiesToIgnore.end() != m_propertiesToIgnore.find(propertyNameUtf8String))
            {
            currentNode = (BeXmlNodeP) currentNode->next;
            continue;
            }

        if (propertyNameUtf8String.Contains("ID"))
            {
            aspectName.append(Utf8String("ID"));
            propertyName = aspectName.c_str();
            }
        Bentley::Utf8String propertyType = DeterminePropertyType(propertyName);
        Bentley::Utf8String kindOfQuantity = DetermineKindOfQuantity(propertyNameUtf8String, propertyType.c_str());
        Bentley::Utf8String displayLabel = DetermineDisplayLabel(propertyName);
        AddNewAspectProperty(aspectNodeP, propertyName, propertyType, displayLabel, kindOfQuantity);
        currentNode = (BeXmlNodeP) currentNode->next;
        } while (currentNode != nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ObmApplicationSchemaCreator::UpdateOBMApplicationSchema(Bentley::DgnECInstanceCR ecInstanceCR)
    {
    DgnECInstanceR ecInstance = const_cast<DgnECInstanceR>(ecInstanceCR);

    auto ecClassName = ecInstance.GetClass().GetName();
    auto ecClassNameCopy = ecClassName.copy();
    auto aspectName = ecClassName.append(WString("Aspect").c_str());
    BeXmlNodeP aspectNodeP = AddNewAspect(m_rootNodeP, Bentley::Utf8String(aspectName.c_str()), Bentley::Utf8String("sealed"), Bentley::Utf8String(ecInstance.GetClass().GetDisplayLabel().c_str()), Bentley::Utf8String(ecClassName.c_str()));
    if (aspectNodeP == nullptr)
        {
        //We've already seen this type of element, but we need to check the serialized properties and attributes fragment to see if the specific type of element exists in the schema
        aspectNodeP = FindAspectByName(BentleyM0200::Utf8String(aspectName.c_str()));
        Bentley::ECN::ECValue serializedProperties;
        auto serializedPropertiesName = ecClassNameCopy.copy().append((WString("_SerializedProperties")).c_str());
        ecInstance.GetValue(serializedProperties, serializedPropertiesName.c_str());
        if (!serializedProperties.IsNull())
            ParseSerializedProperties(serializedProperties.ToString(), BentleyM0200::Utf8String(aspectName.c_str()), aspectNodeP);
        else
            {
            Bentley::WString beamSegmentAttributesName("Prop");
            serializedPropertiesName = ecClassNameCopy.copy().append((WString("AttributesXmlFragment")).c_str());
            beamSegmentAttributesName.append(serializedPropertiesName);
            ecInstance.GetValue(serializedProperties, beamSegmentAttributesName.c_str());
            if (!serializedProperties.IsNull())
                ParseSerializedProperties(serializedProperties.ToString());
            }
        return;
        }

    Bentley::ECN::ECPropertyIterable ecProperties = ecInstance.GetClass().GetProperties(false);
    for (Bentley::ECN::ECPropertyIterable::const_iterator propertyIterator = ecProperties.begin(); propertyIterator != ecProperties.end(); ++propertyIterator)
        {
        auto propertyName = (*propertyIterator)->GetName();
        auto propertyType = (*propertyIterator)->GetTypeName();
        auto propertyDisplayLabel = (*propertyIterator)->GetDisplayLabel();
        if (propertyName.Contains(Bentley::WString("SerializedProperties")))
            {
            Bentley::ECN::ECValue serializedProperties;
            ecInstance.GetValue(serializedProperties, propertyName.c_str());
            ParseSerializedProperties(serializedProperties.ToString(), BentleyM0200::Utf8String(aspectName.c_str()), aspectNodeP);
            }
        else if (propertyName.Contains(Bentley::WString("AttributesXmlFragment")))
            {
            Bentley::ECN::ECValue serializedProperties;
            ecInstance.GetValue(serializedProperties, propertyName.c_str());
            ParseSerializedProperties(serializedProperties.ToString());
            }
        else
            {
            Bentley::Utf8String kindOfQuantity = DetermineKindOfQuantity(BentleyM0200::Utf8String(propertyName.c_str()), BentleyM0200::Utf8String(propertyType.c_str()));
            auto displayLabel = DetermineDisplayLabel(BentleyM0200::Utf8String(propertyDisplayLabel.c_str()));
            AddNewAspectProperty(aspectNodeP, Bentley::Utf8String(propertyName.c_str()), Bentley::Utf8String(propertyType.c_str()), displayLabel, kindOfQuantity);
            }
        }
    }


END_ORDBRIDGE_NAMESPACE


