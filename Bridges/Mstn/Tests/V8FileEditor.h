/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ConverterInternal.h"
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
    void SetActiveLevel(EditElementHandleR eeh) {eeh.GetElementP()->ehdr.level = m_activeLevel;}

    // Geometry
    void AddLine(DgnV8Api::ElementId* eid = nullptr, DgnV8ModelP v8model = nullptr, DPoint3d offset = DPoint3d::FromZero());
    void AddModel (DgnV8Api::ModelId& modelid, Bentley::WStringCR modelName);
    void AddView (DgnV8Api::ElementId& elementId, Bentley::WStringCR viewName);
    void AddLevel (DgnV8Api::LevelId& levelid, Bentley::WStringCR levelName);
    };
