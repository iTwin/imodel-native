/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/V8FileEditor.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "../ConverterInternal.h"
#include <Bentley/BeTest.h>

//=======================================================================================
// @bsiclass                                    Sam.Wilson                      04/15
//=======================================================================================
struct V8FileEditor
    {
    Bentley::DgnFilePtr m_file;
    DgnV8ModelP m_defaultModel;
    DgnV8Api::LevelId m_activeLevel;

    V8FileEditor() : m_defaultModel(nullptr), m_activeLevel(DGNV8_LEVEL_DEFAULT_LEVEL_ID) {;}
    ~V8FileEditor() {Save();}

    void Open(BentleyApi::BeFileNameCR);
    void Save();
    void GetAndLoadModel(DgnV8ModelP&, DgnV8Api::ModelId = -2);
    void AddAttachment(BentleyApi::BeFileNameCR, DgnV8ModelP v8model = nullptr, Bentley::DPoint3d origin = Bentley::DPoint3d::FromZero());
    void AddFileLink(DgnV8Api::ElementId elementId, BentleyApi::BeFileName& linkFile, DgnV8ModelP v8model = nullptr);
    void AddTextElement(DgnTextStyleCR textStyle, bool addToModel=true);
    DgnV8Api::LevelId AddV8Level(BentleyApi::Utf8CP levelname);
    void SetActiveLevel(DgnV8Api::LevelId l) {m_activeLevel=l;}
    DgnV8Api::LevelId GetActiveLevel() const {return m_activeLevel;}
    void SetActiveLevel(EditElementHandleR eeh) {eeh.GetElementP()->ehdr.level = m_activeLevel;}
    DgnV8Api::LevelId GetLevelByName(WCharCP levelName)
        {
        DgnV8Api::FileLevelCache& lc = m_file->GetLevelCacheR();
        return lc.GetLevelByName(levelName).GetLevelId();
        }

    // ECData 
    Bentley::BentleyStatus CreateInstance(DgnV8Api::DgnElementECInstancePtr &createdDgnECInstance, DgnV8ModelP targetModel, WCharCP schemaName, WCharCP className);
    Bentley::BentleyStatus CreateInstanceOnElement(DgnV8Api::DgnElementECInstancePtr &createdDgnECInstance, Bentley::ElementHandleCR eh, DgnV8ModelP targetModel, WCharCP schemaName, WCharCP className, bool markInterinsic=false);
    DgnECInstancePtr QueryECInstance(ElementHandleCR eh, WCharCP schemaName, WCharCP className);

    // Geometry
    void AddLine(DgnV8Api::ElementId* eid = nullptr, DgnV8ModelP v8model = nullptr, DPoint3d offset = DPoint3d::FromZero());
    void AddCellWithTwoArcs(DgnV8Api::ElementId* cellId, WCharCP cellName, DgnV8ModelP v8model = nullptr);
    void CreateCell(EditElementHandleR eeh, WCharCP cellName, bool addToModel = true, DgnV8ModelP v8model = nullptr);
    void CreateArc(EditElementHandleR eeh, bool addToModel=true, DgnV8ModelP v8model = nullptr);
    void CreateBSplineCurve(EditElementHandleR eeh, bool addToModel=true, DgnV8ModelP v8model = nullptr);
    void CreatePointString(EditElementHandleR eeh, bool addToModel=true, DgnV8ModelP v8model = nullptr);
    void CreateEllipse(EditElementHandleR eeh, bool addToModel=true, DgnV8ModelP v8model = nullptr);
    void CreateMesh(EditElementHandleR eeh, bool addToModel=true, DgnV8ModelP v8model = nullptr);
    void CreateCone(EditElementHandleR eeh, bool addToModel=true, DgnV8ModelP v8model = nullptr);
    void CreateComplex(EditElementHandleR eeh, bool addToModel = true, DgnV8ModelP v8model = nullptr);
    DgnV8Api::TextBlockToElementResult AddText(EditElementHandleR eeh, DgnTextStyleCR textStyle, DgnV8ModelP v8model = nullptr);
    void CreateGroupHole(EditElementHandleR eeh, bool addToModel = true, DgnV8ModelP v8model = nullptr);
    };
