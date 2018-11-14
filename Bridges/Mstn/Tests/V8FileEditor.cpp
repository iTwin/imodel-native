/*------------------------------------ConverterApp--------------------------------------------------+
|
|     $Source: Tests/V8FileEditor.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
void V8FileEditor::AddAttachment(BentleyApi::BeFileNameCR attachmentFileName, DgnV8ModelP v8model, Bentley::DPoint3d origin)
    {
    if (nullptr == v8model)
        v8model = m_defaultModel;

    //  Add refV8File as an attachment to v8File
    Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(attachmentFileName.c_str());
    DgnV8Api::DgnAttachment* attachment;
    ASSERT_EQ( BentleyApi::SUCCESS, v8model->CreateDgnAttachment(attachment, *moniker, nullptr, true) );
    ASSERT_TRUE(nullptr != attachment);
    attachment->SetRefOrigin(origin);
    ASSERT_EQ( BentleyApi::SUCCESS, attachment->WriteToModel() );
    }
