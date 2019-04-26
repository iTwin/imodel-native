/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ConverterInternal.h"
#include <Bentley/BeTest.h>
#include "CodeScope.h"

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

//=======================================================================================
// @bsistruct                              
//=======================================================================================
struct CodeAssignerXDomain : XDomain
    {
    enum class Scope {Model, Related, Repository};

    int m_lineCount {};
    BentleyApi::Dgn::CodeSpecId m_codeSpecId;
    BentleyApi::Dgn::DgnElementId m_scopeId;
    CodeScope m_scope;
    Utf8String m_prefix;

    CodeAssignerXDomain(BentleyApi::Dgn::DgnElementId scopeId, Utf8CP prefix, CodeScope s) : m_scopeId(scopeId), m_scope(s), m_prefix(prefix) {}

    void InitCodeSpec(BentleyApi::Dgn::DgnDbR);
    BentleyApi::Dgn::DgnCode GenerateCode(BentleyApi::Dgn::DgnDbR);

    void _DetermineElementParams(DgnClassId&, DgnCode&, DgnCategoryId&, DgnV8Api::ElementHandle const&, Converter&, Bentley::ECN::IECInstance const*, ResolvedModelMapping const&) override;

    void _OnBeginConversion(Converter&, DgnV8ModelR rootModel) override {;}
    void _OnFinishConversion(Converter&) override {;}
    };

