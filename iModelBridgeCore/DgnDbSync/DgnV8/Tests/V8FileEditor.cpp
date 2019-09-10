/*------------------------------------ConverterApp--------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "V8FileEditor.h"
#include <Bentley/BeThread.h>
#include <VersionedDgnV8Api/DgnPlatform/ECXAProvider.h>
#include <VersionedDgnV8Api/DgnPlatform/ECXAInstance.h>
#include <VersionedDgnV8Api/DgnPlatform/ECInstanceHolderHandler.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::Open(BentleyApi::BeFileNameCR inputFile)
    {
    //uint64_t startTime = BeTimeUtilities::QueryMillisecondsCounter();
    BentleyApi::BeThreadUtilities::BeSleep(1); // make sure that the lastmodified time of V8 elements and of the V8 file changes! NB: element last mod time granularity is 1 millisecond.
    //EXPECT_GE(BeTimeUtilities::QueryMillisecondsCounter() , (startTime + 1));

    DgnV8Api::DgnFileStatus openStatus;
    auto doc = DgnV8Api::DgnDocument::CreateFromFileName(openStatus, inputFile, nullptr, Bentley::DEFDGNFILE_ID, DgnV8Api::DgnDocument::FetchMode::Read, DgnV8Api::DgnDocument::FetchOptions::Default);
    ASSERT_NE(doc , nullptr);
    m_file= DgnV8Api::DgnFile::Create(*doc, DgnV8Api::DgnFileOpenMode::ReadWrite);
    ASSERT_TRUE( m_file.IsValid() );
    auto loadFileStatus = m_file->LoadDgnFile(nullptr);
    ASSERT_EQ(BentleyApi::SUCCESS , loadFileStatus);

    GetAndLoadModel(m_defaultModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::Save()
    {
    m_file->ProcessChanges(DgnV8Api::DgnSaveReason::ApplInitiated);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::GetAndLoadModel(DgnV8ModelP& v8Model, DgnV8Api::ModelId mid)
    {
    if (-2 == mid)
        mid = m_file->GetDefaultModelId();

    v8Model = m_file->LoadRootModelById(nullptr, mid, true, true, false);
    ASSERT_NE( nullptr , v8Model );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::AddLine(DgnV8Api::ElementId* eid, DgnV8ModelP v8model, DPoint3d offset)
    {
    if (nullptr == v8model)
        v8model = m_defaultModel;

    double mm = v8model->GetModelInfo().GetUorPerStorage(); // our seed file has UORs in millimeters.

    Bentley::DSegment3d pts;
    pts.point[0] = offset;
    pts.point[1] = Bentley::DPoint3d::From(1000*mm, 0, 0);   // make the line 1 meter long
    DgnV8Api::EditElementHandle eh;
    DgnV8Api::LineHandler::CreateLineElement(eh, nullptr, pts, v8model->Is3D(), *v8model);
    SetActiveLevel(eh);
    ASSERT_EQ( BentleyApi::SUCCESS , eh.AddToModel() );

    if (nullptr != eid)
        *eid = eh.GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::AddAttachment(BentleyApi::BeFileNameCR attachmentFileName, DgnV8ModelP v8model, Bentley::DPoint3d origin, wchar_t const* attachedModelName)
    {
    if (nullptr == v8model)
        v8model = m_defaultModel;

    //  Add refV8File as an attachment to v8File
    Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(attachmentFileName.c_str());
    DgnV8Api::DgnAttachment* attachment;
    ASSERT_EQ( BentleyApi::SUCCESS, v8model->CreateDgnAttachment(attachment, *moniker, attachedModelName, true) );
    ASSERT_TRUE(nullptr != attachment);
    attachment->SetRefOrigin(origin);
    ASSERT_EQ( BentleyApi::SUCCESS, attachment->WriteToModel() );
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::AddFileLink(DgnV8Api::ElementId elementId, BentleyApi::BeFileName& linkFile, DgnV8ModelP v8model)
    {
    if (nullptr == v8model)
        v8model = m_defaultModel;

    DgnV8Api::ElementRefBase* elemRef = v8model->FindByElementId(elementId);
    EXPECT_TRUE(NULL != elemRef);

    DgnV8Api::DgnLinkTreeSpecPtr spec = DgnV8Api::DgnLinkManager::CreateTreeSpec(elemRef);
    EXPECT_TRUE(spec.IsValid());

    DgnV8Api::DgnLinkTreePtr linkTree = DgnV8Api::DgnLinkManager::ReadLinkTree(*spec, true);
    ASSERT_TRUE(linkTree.IsValid()) << "Fatal Error: Link tree is not valid";

    DgnLinkTreeBranchR root = linkTree->GetRootR();

    StatusInt status;
    DgnV8Api::TempDgnLinkTreeLeafOwner fileLinkLeaf = DgnV8Api::DgnLinkManager::CreateLink(status, DGNLINK_TYPEKEY_File);
    ASSERT_TRUE(SUCCESS == status);

    DgnV8Api::DgnFileLink* fileLink = static_cast <DgnV8Api::DgnFileLink*> (fileLinkLeaf->GetLinkP());

    Bentley::DgnDocumentMonikerPtr  linkMoniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(linkFile.c_str());

    ASSERT_TRUE(linkMoniker.IsValid());
    fileLink->SetMoniker(linkMoniker.get());
    fileLinkLeaf->SetName(L"FileLinkLeaf");
    root.AddChild(*fileLinkLeaf, -1);

    DgnV8Api::DgnLinkManager::WriteLinkTree(*linkTree);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat 09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::AddTextElement(DgnTextStyleCR textStyle, bool addToModel)
    {
    DgnV8Api::EditElementHandle textEEH;
    ASSERT_EQ(DgnV8Api::TEXTBLOCK_TO_ELEMENT_RESULT_Success, AddText(textEEH, textStyle));
    if (addToModel)
        ASSERT_TRUE(SUCCESS == textEEH.AddToModel());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat 09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::TextBlockToElementResult V8FileEditor::AddText(EditElementHandleR eeh, DgnTextStyleCR textStyle, DgnV8ModelP v8model)
    {
    if (nullptr == v8model)
        v8model = m_defaultModel;
    DgnV8Api::TextBlockPtr textBlock = DgnV8Api::TextBlock::Create(textStyle ,*v8model);
    EXPECT_TRUE(textBlock.IsValid());
    DPoint3d origin = DPoint3d::From(100, 200, 300);
    textBlock->SetUserOrigin (origin);
    textBlock->AppendText(L"Run1");
    textBlock->AppendText(L"Run2");
    return DgnV8Api::TextHandlerBase::CreateElement(eeh, NULL, *textBlock);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Umar.Hayat                          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::CreateCell(EditElementHandleR eeh, WCharCP cellName, bool addToModel, DgnV8ModelP v8model)
    {
    if (nullptr == v8model)
        v8model = m_defaultModel;

    ///////////////////////////////////////////////////////////////////////////
    //Create a named Cell.
    ///////////////////////////////////////////////////////////////////////////
    DPoint3d origin = DPoint3d::FromXYZ(100, 100, 0);
    RotMatrix rotationAndScale = RotMatrix::FromRowValues(1, 0, 0, 0, 1, 0, 0, 0, 1);

    DgnV8Api::NormalCellHeaderHandler::CreateCellElement(eeh, cellName, origin, rotationAndScale, v8model->Is3d(), *v8model);
    SetActiveLevel(eeh);
    if (addToModel)
        ASSERT_EQ(BentleyApi::SUCCESS, eeh.AddToModel());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Umar.Hayat                          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::CreateArc(EditElementHandleR eeh, bool addToModel, DgnV8ModelP v8model) // has no instances from our provider
    {
    if (nullptr == v8model)
        v8model = m_defaultModel;

    DPoint3d center = DPoint3d::FromXYZ(0, 0, 0);
    RotMatrix rot;
    rot.InitIdentity();

    DgnV8Api::ArcHandler::CreateArcElement(eeh, NULL, center, 150, 100, rot, 0.5, 1.0, v8model->Is3d(), *v8model);
    SetActiveLevel(eeh);
    if (addToModel)
        ASSERT_EQ(BentleyApi::SUCCESS, eeh.AddToModel());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             02/17
//---------------------------------------------------------------------------------------
void V8FileEditor::AddCellWithTwoArcs(DgnV8Api::ElementId* cellId, WCharCP cellName, DgnV8ModelP v8model)
    {
    if (nullptr == v8model)
        v8model = m_defaultModel;

    BentleyStatus status = ERROR;
    DgnV8Api::EditElementHandle arcEEH1, arcEEH2;
    CreateArc(arcEEH1, false, v8model);
    CreateArc(arcEEH2, false, v8model);

    DgnV8Api::EditElementHandle cellEEH;
    CreateCell(cellEEH, cellName, false, v8model);

    status = DgnV8Api::NormalCellHeaderHandler::AddChildElement(cellEEH, arcEEH1);
    EXPECT_TRUE(SUCCESS == status);
    status = DgnV8Api::NormalCellHeaderHandler::AddChildElement(cellEEH, arcEEH2);
    EXPECT_TRUE(SUCCESS == status);
    status = DgnV8Api::NormalCellHeaderHandler::AddChildComplete(cellEEH);
    EXPECT_TRUE(SUCCESS == status);

    EXPECT_TRUE( SUCCESS == cellEEH.AddToModel());

    if (nullptr != cellId)
        *cellId = cellEEH.GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Umar.Hayat                          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::CreateBSplineCurve(EditElementHandleR eeh, bool addToModel, DgnV8ModelP v8model) // has no instances from our provider
    {
    if (nullptr == v8model)
        v8model = m_defaultModel;

    DPoint3d center = DPoint3d::FromXYZ(0, 0, 0);
    RotMatrix rot;
    rot.InitIdentity();
    Bentley::MSBsplineCurve bCurve;
    static int const TEST_POINTNUM = 4;
    DPoint3d    points[4];
    points[0].Init(-10000 , 0, 0);
    points[1].Init(-10000, 1000, 0);
    points[2].Init(-10000, 2000, 0);
    points[3].Init(-10000, 3000, 0);
    bCurve.InitFromPoints( points, TEST_POINTNUM);

    DgnV8Api::BSplineStatus status = DgnV8Api::BSplineCurveHandler::CreateBSplineCurveElement(eeh, nullptr,  bCurve, v8model->Is3d(), *v8model);
    ASSERT_TRUE(DgnV8Api::BSPLINE_STATUS_Success == status);
    SetActiveLevel(eeh);
    if (addToModel)
        ASSERT_EQ(BentleyApi::SUCCESS, eeh.AddToModel());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Umar.Hayat                          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::CreatePointString(EditElementHandleR eeh, bool addToModel, DgnV8ModelP v8model)
    {
    if (nullptr == v8model)
        v8model = m_defaultModel;
    DPoint3d    points[4];
    points[0].Init(-10000, 0, 0);
    points[1].Init(-10000, 1000, 0);
    points[2].Init(-10000, 2000, 0);
    points[3].Init(-10000, 3000, 0);

    ASSERT_EQ(SUCCESS, DgnV8Api::PointStringHandler::CreatePointStringElement(eeh, NULL, points, NULL, 4, true, v8model->Is3d(), *v8model));
    SetActiveLevel(eeh);
    if (addToModel)
        ASSERT_EQ(BentleyApi::SUCCESS, eeh.AddToModel());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Umar.Hayat                          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::CreateEllipse(EditElementHandleR eeh, bool addToModel, DgnV8ModelP v8model)
    {
    if (nullptr == v8model)
        v8model = m_defaultModel;
    DPoint3d point[] = { 0, 0, 0 };
    double axis1 = 1000*PI / 4;
    double axis2 = 1000 * PI / 6;
    double rotAngle = 1000 * PI / 8;

    ASSERT_EQ(SUCCESS, DgnV8Api::EllipseHandler::CreateEllipseElement(eeh, NULL, *point, axis1, axis2, rotAngle, v8model->Is3d(), *v8model));
    SetActiveLevel(eeh);
    if (addToModel)
        ASSERT_EQ(BentleyApi::SUCCESS, eeh.AddToModel());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Umar.Hayat                          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::CreateCone(EditElementHandleR eeh, bool addToModel, DgnV8ModelP v8model)
    {
    if (nullptr == v8model)
        v8model = m_defaultModel;
    DPoint3d top = { 1000, 1000, 0 };
    DPoint3d bottom = { 0, 0, 0 };

    double const TOP_RADIUS = 1000*5.0;
    double const BOTTOM_RADIUS = 1000*8.0;

    RotMatrix           rMatrix;
    rMatrix.InitIdentity();

    ASSERT_EQ(SUCCESS, DgnV8Api::ConeHandler::CreateConeElement(eeh, NULL, TOP_RADIUS, BOTTOM_RADIUS, top, bottom, rMatrix, v8model->Is3d(), *v8model));
    SetActiveLevel(eeh);
    if (addToModel)
        ASSERT_EQ(BentleyApi::SUCCESS, eeh.AddToModel());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Umar.Hayat                          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::CreateComplex(EditElementHandleR eeh, bool addToModel, DgnV8ModelP v8model)
    {
    if (nullptr == v8model)
        v8model = m_defaultModel;
    DgnV8Api::ChainHeaderHandler::CreateChainHeaderElement(eeh, NULL, true, v8model->Is3d(), *v8model);
    SetActiveLevel(eeh);
    DgnV8Api::EditElementHandle line;
    DSegment3d  segment;
    segment.point[0].Zero();
    segment.point[1] = DPoint3d::FromXYZ(1000, 1000, 0);

    EXPECT_EQ(SUCCESS, DgnV8Api::LineHandler::CreateLineElement(line, NULL, segment, v8model->Is3d(), *v8model));
    SetActiveLevel(line);
    DgnV8Api::ChainHeaderHandler::AddComponentElement(eeh, line);
    DgnV8Api::ChainHeaderHandler::AddComponentComplete(eeh);
    line.Invalidate();
    if (addToModel)
        ASSERT_EQ(BentleyApi::SUCCESS, eeh.AddToModel());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Umar.Hayat                          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::CreateMesh(EditElementHandleR eeh, bool addToModel, DgnV8ModelP v8model )
    {
    PolyfaceHeaderPtr header0 = PolyfaceHeader::New ();
    header0->SetMeshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
    header0->SetTwoSided (false);
    header0->SetNumPerFace (0);
    DPoint3d points[] =
        {
        {1,1,0},
        {2,1,0},
        {2,2,0},
        {1,2,0},
        };
    int indices [] =
        {
        1,2,-3,0,
        3,4,-1,0
        };

    header0->Point().SetActive (true);
    header0->PointIndex().SetActive (true);
    for (int i = 0; i < _countof (indices); i++)
        header0->PointIndex().push_back (indices[i]);

    for (int i = 0; i < _countof (points); i++)
        header0->Point ().push_back (points[i]);
    StatusInt stat1 = DgnV8Api::MeshHeaderHandler::CreateMeshElement(eeh, NULL, *header0, v8model->Is3d(), *v8model);
    ASSERT_TRUE(SUCCESS == stat1);
    SetActiveLevel(eeh);
    if (addToModel)
        ASSERT_EQ(BentleyApi::SUCCESS, eeh.AddToModel());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Umar.Hayat                          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void V8FileEditor::CreateGroupHole(EditElementHandleR eeh, bool addToModel, DgnV8ModelP v8model)
    {
    if (nullptr == v8model)
        v8model = m_defaultModel;
    DgnV8Api::EditElementHandle solid;
    DgnV8Api::EditElementHandle shapeEeh;
    DgnV8Api::EditElementHandle holeEeh;
    DgnV8Api::ElementAgenda holes;

    DPoint3d point2[] = {5.0, 5.0, 5.0};
    ASSERT_EQ(SUCCESS, DgnV8Api::EllipseHandler::CreateEllipseElement(holeEeh, NULL, *point2, PI / 6, PI / 7, NULL, v8model->Is3D(), *v8model));

    DPoint3d point[] = {0.0, 0.0, 0.0};
    ASSERT_EQ(SUCCESS, DgnV8Api::EllipseHandler::CreateEllipseElement(shapeEeh, NULL, *point, PI / 4, PI / 5, NULL, v8model->Is3D(), *v8model));

    //MSElementCP holeElm = holeEeh.GetElementCP ();
    //MSElementCP shapeElm = shapeEeh.GetElementCP ();

    holes.Insert (holeEeh);

    ASSERT_EQ(SUCCESS, DgnV8Api::GroupedHoleHandler::CreateGroupedHoleElement(eeh, shapeEeh, holes));

    if (addToModel)
        ASSERT_EQ(BentleyApi::SUCCESS, eeh.AddToModel());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            06/2018
//---------------+---------------+---------------+---------------+---------------+-------
Bentley::BentleyStatus V8FileEditor::CreateInstance(DgnV8Api::DgnElementECInstancePtr &createdDgnECInstance, DgnV8ModelP targetModel, WCharCP schemaName, WCharCP className)
    {
    if (nullptr == targetModel)
        targetModel = m_defaultModel;

    DgnV8Api::EditElementHandle eeh;
    DgnV8Api::ECInstanceHolderHandler::CreateECInstanceHolderElement(eeh, *targetModel->GetDgnModelP());

    // Add the element to the dgnRepository. The element must be added before we write the XAttributes.
    if (SUCCESS != eeh.AddToModel())
        return ERROR;

    return CreateInstanceOnElement(createdDgnECInstance, eeh, targetModel, schemaName, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Umar.Hayat                          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::BentleyStatus V8FileEditor::CreateInstanceOnElement(DgnV8Api::DgnElementECInstancePtr &createdDgnECInstance, Bentley::ElementHandleCR eh, DgnV8ModelP targetModel, WCharCP schemaName, WCharCP className, bool markInterinsic)
    {
    BentleyStatus status = SUCCESS;
    Bentley::DgnFileR targetFile = *(targetModel->GetDgnFileP());
    Bentley::DgnECInstanceEnablerP   m_elementECInstanceEnabler = NULL;
    
    m_elementECInstanceEnabler = DgnV8Api::ECXAProvider::GetProvider().ObtainInstanceEnablerByName(schemaName, className, DgnV8Api::ECXAProvider::GetProvider().GetPerFileCache(targetFile));

    if (NULL == m_elementECInstanceEnabler)
        {
        return ERROR;
        }

    ECObjectsV8::StandaloneECInstanceR wipInstance = m_elementECInstanceEnabler->GetSharedWipInstance();

    DgnV8Api::DgnECInstanceStatus ecInstanceStatus;

    ecInstanceStatus = m_elementECInstanceEnabler->CreateInstanceOnElement (&createdDgnECInstance, wipInstance, eh);

    if (DgnV8Api::DGNECINSTANCESTATUS_Success != ecInstanceStatus)
        {
        return ERROR;
        }

    if (createdDgnECInstance.IsValid ())
        {
            if (markInterinsic)
            {
            DgnV8Api::ECXAInstance*  ecxaInstance = dynamic_cast<DgnV8Api::ECXAInstance*> (createdDgnECInstance.get());
            if (ecxaInstance)
                ecxaInstance->MarkIntrinsic(DgnV8Api::DgnECProviderId_Element);
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECInstancePtr V8FileEditor::QueryECInstance(ElementHandleCR eh, WCharCP schemaName, WCharCP className)
    {
    DgnV8Api::FindInstancesScopeOption scopeOption;
    DgnV8Api::FindInstancesScopePtr ehScope = DgnV8Api::FindInstancesScope::CreateScope(eh, scopeOption);
    auto query = DgnV8Api::ECQuery::CreateQuery(schemaName, className, true);
    auto iter = DgnV8Api::DgnECManager::GetManager().FindInstances(*ehScope, *query);
    auto i = iter.begin();
    if (i != iter.end())
        return (*i);
    return nullptr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Umar.Hayat                              04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::LevelId V8FileEditor::AddV8Level(BentleyApi::Utf8CP levelname)
    {
    DgnV8Api::FileLevelCache& lc = *dynamic_cast<DgnV8Api::FileLevelCache*>(&m_file->GetLevelCacheR());
    auto level = lc.CreateLevel(Bentley::WString(levelname,Bentley::BentleyCharEncoding::Utf8).c_str(), LEVEL_NULL_CODE, DGNV8_LEVEL_NULL_ID);
    if (!level.IsValid())
        return -1;
    if (DgnV8Api::LevelCacheErrorCode::None != lc.Write())
        return -1;
    return level.GetLevelId();
    }
