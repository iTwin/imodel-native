/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnXf/DgnXfExport.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnXf/DgnDbXf.h>
#include <DgnPlatform/DgnXf/DgnProjectXf.h>
#include <DgnPlatform/DgnCore/DgnProject.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnCore/LineStyle.h>
#include <DgnPlatform/DgnCore/ViewController.h>
#include <DgnPlatform/DgnCore/XGraphics.h>
#include <DgnPlatform/DgnCore/DisplayAttribute.h>
#include <DgnPlatform/DgnCore/DgnMarkupProject.h>
#include <DgnPlatform/DgnHandlers/DgnECPersistence.h>
#include <BeSQLite/ECDb/ECDbApi.h>
#include <ECObjects/ECObjectsAPI.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
using namespace ProtoBuf;

typedef bvector<ECN::ECSchemaCP> ECSchemaVector;
typedef ECSchemaVector& ECSchemaVectorR;
typedef const ECSchemaVector& ECSchemaVectorCR;

typedef bvector<ECN::ECClassCP> ECClassVector;
typedef ECClassVector& ECClassVectorR;
typedef const ECClassVector& ECClassVectorCR;

//=======================================================================================
// Save a DgnDb as DgnXf.
// @bsiclass                                                    Keith.Bentley   09/13
//=======================================================================================
struct DgnDbXfWriter
{
private:
    static bool GetECInstanceIdFromECInstance (ECInstanceId& ecInstanceId, ECN::IECInstanceCR instance)
        {
        WString instanceIdStr = instance.GetInstanceId ();
        if (instanceIdStr.empty ())
            return false;

        return ECInstanceIdHelper::FromString (ecInstanceId, instanceIdStr.c_str ());
        }

public:
    DgnXfLib::IMessageProcessor& m_messageProcessor;
    DgnProjectR      m_project;
    //  For Graphite05 and earlier symbols are saved to elements and have
    //  model scope. Starting with Graphite06 they are saved to stamps and have project scope.
    //  When we stop exporting projects created in Graphite05 and older we can eliminate m_savedElementSymbols
    bset<ElementId>  m_savedElementSymbols;
    bset<DgnStampId> m_savedStampSymbols;
    int              m_nLoaded;
    bvector<ECN::ECSchemaCP> m_ecSchemas;

    DgnDbXfWriter(DgnProjectR project, DgnXfLib::IMessageProcessor& proc) : 
                  m_project(project), m_messageProcessor(proc), m_nLoaded(0) {}

    // each mesasge must be accompanied by size and type values before the actual data.
    void ProcessMessage(DgnXfLib::DgnXfMessageId messageType, ProtoBuf::MessageLite& msg)
        {
        m_messageProcessor.ProcessMessage (messageType, msg);
        }

    void WriteSectionStart(DgnXfLib::DgnXfSectionId sectionId);
    void WriteSectionEnd(DgnXfLib::DgnXfSectionId sectionId);

    void WriteDgnHeader();
    void WriteUnits();
    void WriteLevelTable();
    void WriteFontTable();
    void WriteColors();
    void WriteLineStyleComponents(DgnXfLib::Styles& xfStyles);
    void WriteStyleTable();
    void WriteRedlineModelProperties (DgnXfLib::Models_Model*);
    void WriteModelTable();
    void WriteModelSelectorTable();
    void WriteViews();
    void WriteElement (DgnModelId modelId, ElementId elId, bool is3d);

    void WritePhysicalModel(DgnModelId modelId);
    void WriteDrawingModel(DgnModelId modelId);
    void WritePhysicalElements();
    void WriteDrawingElements();
    void WriteSheetElements();
    void WriteRedlineElements();
    void WritePhysicalRedlineElements();

    //  For Graphite05 and earlier symbols are saved to elements and have
    //. model scope. Starting with Graphite06 they are saved to stamps and have project scope.
    void WriteElementSymbolDefinition(ElementId symbolId, DgnModelId modelId);
    //  The DgnstampId is project wide, but needs to be created for every model on the V8 side.
    //  This code assumes that is an importer issue.
    void WriteStampSymbolDefinition(DgnStampId symbolId, DgnModelId modelId);

    static bool ECSchemaCompare (ECN::ECSchemaCP schema1, ECN::ECSchemaCP schema2);
    void WriteECSchema (ECN::ECSchemaCP ecSchema);
    void SetupECInstanceKeysOnElement (DgnXfLib::GraphicElement &xfElement, ECInstanceKeyMultiMap& ecKeyMap, ElementHandleCR eh);
    void GatherExportableECClasses (ECClassVectorR ecClasses, ECClassVectorR ecRelationshipClasses);
    void WriteECInstancesOfClass (ECN::ECClassCP ecClass);

    void SaveDoubleArray(ProtoBuf::RepeatedField<double>* out, double const* in, int size) {out->Reserve(size); for (int i=0; i<size; ++i) out->AddAlreadyReserved(*(in+i));}
    void SaveRotMatrix(DgnXfLib::RotMatrix& out, RotMatrixCR in) {SaveDoubleArray (out.mutable_rot(), (double const*) &in, 9);}
    void SaveTransform(DgnXfLib::Transform& out, TransformCR in) {SaveDoubleArray (out.mutable_trans(), (double const*) &in, 12);}
    void SaveDRange3d(DgnXfLib::DRange3d& out, DRange3dCR in) {SaveDoubleArray (out.mutable_range(), (double const*) &in, 6);}
    void SaveDPoint2d(DgnXfLib::DPoint2d& out, DPoint2dCR in) {out.add_pt(in.x); out.add_pt(in.y);}
    void SaveDPoint3d(DgnXfLib::DPoint3d& out, DPoint3dCR in) {out.add_pt(in.x); out.add_pt(in.y); out.add_pt(in.z);}

    void ExportContextSection();
    void ExportECSchemaSection();
    void ExportElementSection();
    void ExportECInstancesSection();

    DgnXfLib::DgnXfStatus ExportToDgnXf();
};

#define SAVE_OPTIONAL_ID(VAL,SETTER)     if((VAL).IsValid()){SETTER(VAL.GetValue());}
#define SAVE_OPTIONAL_STRING(VAL,SETTER) if(NULL!=(VAL)){SETTER(VAL);}
#define SAVE_OPTIONAL_DOUBLE(VAL,SETTER) if(0.0!=VAL){SETTER(VAL);}
#define SAVE_OPTIONAL_DRANGE(VAL,SETTER) if(!(VAL).IsNull()){SaveDRange3d(*SETTER(),VAL);}
#define SAVE_OPTIONAL_INT(VAL,SETTER)    if(0!=(VAL)){SETTER(VAL);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteSectionStart(DgnXfLib::DgnXfSectionId sectionId)
    {
    DgnXfLib::SectionStart msg; 
    msg.set_type(sectionId); 
    ProcessMessage(DgnXfLib::MESSAGE_SectionStart, msg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteSectionEnd(DgnXfLib::DgnXfSectionId sectionId)
    {
    DgnXfLib::SectionEnd msg; 
    msg.set_type(sectionId); 
    ProcessMessage(DgnXfLib::MESSAGE_SectionEnd, msg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteDgnHeader()
    {
    DgnXfLib::DgnHeader hdr;
    hdr.set_projectGuid(m_project.QueryMyProjectGuid().ToString());
    hdr.set_provenance (Utf8String(m_project.GetFileName()));
    hdr.set_createdByApp(Utf8String(T_HOST.GetProductName()));
    ProcessMessage (DgnXfLib::MESSAGE_CONTEXT_DgnHeader, hdr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteUnits()
    {
    DgnXfLib::Units units;

    DgnUnits& projUnits = m_project.Units();
    StorageUnits& projStorage = projUnits.GetPhysicalUnits();

    DgnXfLib::StorageUnits* storageUnits = units.mutable_physicalUnits();
    DgnXfLib::UnitDefinition* unitDef = storageUnits->mutable_units();
    unitDef->set_base((DgnXfLib::UnitBase) projStorage.GetBase());
    unitDef->set_system((DgnXfLib::UnitSystem) projStorage.GetSystem());
    unitDef->set_numerator(projStorage.GetNumerator());
    unitDef->set_denominator(projStorage.GetDenominator());
    unitDef->set_label(Utf8String(projStorage.GetLabel()));
    storageUnits->set_uorPerStorage(projStorage.GetUorPerStorage());
    storageUnits->set_solidExtent(projStorage.GetSolidExtent());
    SaveDPoint3d(*storageUnits->mutable_globalOrigin(), projStorage.GetGlobalOrigin());

    SAVE_OPTIONAL_DOUBLE(projUnits.GetAzimuth(), units.set_azimuth);
    SAVE_OPTIONAL_DOUBLE(projUnits.GetOriginLongitude(), units.set_longitude);
    SAVE_OPTIONAL_DOUBLE(projUnits.GetOriginLatitude(), units.set_latitude);
    SAVE_OPTIONAL_DRANGE(projUnits.GetProjectExtents(), units.mutable_extent);

    ProcessMessage (DgnXfLib::MESSAGE_CONTEXT_Units, units);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteLevelTable()
    {
    DgnXfLib::Levels levels;
    for (auto const& projLevel : m_project.Levels().MakeIterator())
        {
        DgnXfLib::Levels_Level* level = levels.add_levels();
        level->set_id(projLevel.GetLevelId().GetValue());
        level->set_name(projLevel.GetName());

        DgnLevels::Facet facet = m_project.Levels().QueryFacetById(LevelFacetId(projLevel.GetLevelId()));
        SAVE_OPTIONAL_INT(facet.GetAppearance().GetColor(), level->set_color);
        SAVE_OPTIONAL_INT(facet.GetAppearance().GetWeight(), level->set_weight);
        SAVE_OPTIONAL_INT(facet.GetAppearance().GetStyle(), level->set_style);
        SAVE_OPTIONAL_DOUBLE(facet.GetAppearance().GetTransparency(), level->set_transparency);
        SAVE_OPTIONAL_INT(facet.GetAppearance().GetDisplayPriority(), level->set_displayPriority);
        SAVE_OPTIONAL_STRING(projLevel.GetDescription(), level->set_descr);
        SAVE_OPTIONAL_ID(facet.GetAppearance().GetMaterial(), level->set_material);
        if (DgnLevels::Scope::Any != projLevel.GetScope())
            level->set_scope((DgnXfLib::Levels_Scope) projLevel.GetScope());
        if (DgnLevels::Rank::User != projLevel.GetRank())
            level->set_rank((DgnXfLib::Levels_Rank) projLevel.GetRank());
        }

    ProcessMessage(DgnXfLib::MESSAGE_CONTEXT_Levels, levels);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteFontTable()
    {
    DgnXfLib::Fonts fonts;
    for (auto const& projfont : m_project.Fonts().MakeIterator())
        {
        DgnXfLib::Fonts_Font* font = fonts.add_fonts();
        font->set_id(projfont.GetFontId());
        font->set_type((DgnXfLib::Fonts_FontType) projfont.GetFontType());
        font->set_name(projfont.GetName());
        }

    ProcessMessage(DgnXfLib::MESSAGE_CONTEXT_Fonts, fonts);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void DgnDbXfWriter::WriteLineStyleComponents(DgnXfLib::Styles& xfStyles)
    {
    bset<LsComponentId> ids;
    LsComponent::QueryComponentIds(ids, m_project, LsResourceType::Compound);
    LsComponent::QueryComponentIds(ids, m_project, LsResourceType::LineCode);
    LsComponent::QueryComponentIds(ids, m_project, LsResourceType::LinePoint);
    LsComponent::QueryComponentIds(ids, m_project, LsResourceType::PointSymbol);

    for (LsComponentId& componentId : ids)
        {
        switch((LsResourceType)componentId.m_type)
            {
            case LsResourceType::Compound:
                {
                LineStyleRscPtr ptr;
                if (LsCompoundComponent::GetRscFromDgnDb(ptr, m_project, componentId.m_id) != BSISUCCESS)
                    continue;
                LineStyleRsc* rsc = ptr.m_data;
                DgnXfLib::LsCompoundComponent* xfComp = xfStyles.add_compoundComponents();
                xfComp->set_thisId(componentId.m_id);
                if (rsc->descr[0] != 0)
                    xfComp->set_descr(rsc->descr);
                xfComp->set_auxType(rsc->auxType);
                for (unsigned i = 0; i < rsc->nComp; ++i)
                    {
                    ComponentInfo* rscComp = rsc->component+i;
                    DgnXfLib::LsCompoundComponent_ComponentInfo* compInfo = xfComp->add_components();
                    compInfo->set_type(rscComp->type);
                    compInfo->set_id(rscComp->id);
                    if (rscComp->offset != 0.0)
                        compInfo->set_offset(rscComp->offset);
                    }
                }
                break;
            case LsResourceType::LineCode:
                {
                LineCodeRscPtr ptr;
                if (LsStrokePatternComponent::GetRscFromDgnDb(ptr, m_project, componentId.m_id) != BSISUCCESS)
                    continue;
                LineCodeRsc*rsc = ptr.m_data;
                DgnXfLib::LsLineCodeComponent* xfComp = xfStyles.add_lineCodeComponents();
                xfComp->set_thisId(componentId.m_id);
                if (rsc->descr[0] != 0)
                    xfComp->set_descr(rsc->descr);
                xfComp->set_phase(rsc->phase);
                xfComp->set_options(rsc->options);
                xfComp->set_maxIterate(rsc->maxIterate);
                for (unsigned i = 0; i < rsc->nStrokes; i++)
                    {
                    StrokeData*rscSD = rsc->stroke+i;
                    DgnXfLib::LsLineCodeComponent_StrokeData*strokeData = xfComp->add_strokeData();
                    strokeData->set_length(rscSD->length);
                    strokeData->set_width(rscSD->width);
                    strokeData->set_endWidth(rscSD->endWidth);
                    strokeData->set_strokeMode(rscSD->strokeMode);
                    strokeData->set_widthMode(rscSD->widthMode);
                    strokeData->set_capMode(rscSD->capMode);
                    }
                }
                break;
            case LsResourceType::LinePoint:
                {
                LinePointRscPtr    ptr;
                if (LsPointComponent::GetRscFromDgnDb(ptr, m_project, componentId.m_id) != BSISUCCESS)
                    continue;
                LinePointRsc*rsc = ptr.m_data;
                DgnXfLib::LsLinePointComponent*xfComp = xfStyles.add_linePointComponents();
                xfComp->set_thisId(componentId.m_id);
                if (rsc->descr[0] != 0)
                    xfComp->set_descr(rsc->descr);
                xfComp->set_lcType(rsc->lcType);
                xfComp->set_lcId(rsc->lcID);
                for (unsigned i = 0; i < rsc->nSym; ++i)
                    {
                    PointSymInfo*rp = rsc->symbol+i;
                    DgnXfLib::LsLinePointComponent_PointSymbolInfo*xp = xfComp->add_symbols();
                    xp->set_symType(rp->symType);
                    xp->set_symId(rp->symID);
                    xp->set_strokeNum(rp->strokeNo);
                    xp->set_mod1(rp->mod1);
                    if (0.0 != rp->xOffset)
                        xp->set_xoffset(rp->xOffset);
                    if (0.0 != rp->yOffset)
                        xp->set_yoffset(rp->yOffset);
                    if (0.0 != rp->zOffset)
                        xp->set_zoffset(rp->zOffset);
                    }
                }
                break;

            case LsResourceType::PointSymbol:
                {
                PointSymRscPtr      ptr;

                if (LsSymbolComponent::GetRscFromDgnDb(ptr, m_project, componentId.m_id) != BSISUCCESS)
                    continue;

                PointSymRsc*        rsc = ptr.m_data;
                if (PointSymRsc::ST_XGraphics != rsc->GetSymbolType())
                    //  WIP report error or convert to XGraphics
                    continue;

                DgnXfLib::LsSymbolComponent*xfComp = xfStyles.add_symbolComponents();
                xfComp->set_thisId(componentId.m_id);
                if (rsc->header.descr[0] != 0)
                    xfComp->set_descr(rsc->header.descr);

                xfComp->set_scale(rsc->header.scale);
                xfComp->set_flags(rsc->symFlags);
                DRange3d range;
                range.low   = rsc->header.range.low;
                range.high  = rsc->header.range.high;
                SaveDRange3d(*xfComp->mutable_range(), range);
                xfComp->set_graphics(rsc->symBuf, rsc->nBytes);
                }
                break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteStyleTable()
    {
    DgnXfLib::Styles xfStyles;
    DgnStyles::Iterator styleTable (m_project);
    UInt32   lineStyleComponentMapIndex = 0;

    for (auto styleTableEntry : styleTable)
        {
        DgnXfLib::Styles_Style* xfStyle = xfStyles.add_styles();
        int dataSize = styleTableEntry.GetDataSize();
        void const* data = styleTableEntry.GetData();
        DgnXfLib::Styles_StyleType styleType = (DgnXfLib::Styles_StyleType)styleTableEntry.GetType();
        xfStyle->set_id(styleTableEntry.GetId().GetValue());
        xfStyle->set_type(styleType);
        xfStyle->set_name(styleTableEntry.GetName());
        xfStyle->set_desc(styleTableEntry.GetDescription());

        if (DgnXfLib::Styles_StyleType_Line == styleType)
            {
            //  Need to do something with the component data
            Json::Value  jsonDef(Json::objectValue);
            Json::Reader::Parse((Utf8CP)data, jsonDef);

            DgnXfLib::LineStyleComponentDescr* compMap = xfStyles.add_lineStyleComponentMap();
            compMap->set_thisId(lineStyleComponentMapIndex);
            compMap->set_unitDef(LsDefinition::GetUnitDef(jsonDef));
            compMap->set_attributes(LsDefinition::GetAttributes(jsonDef));
            compMap->set_componentId(LsDefinition::GetComponentId(jsonDef));
            compMap->set_componentType(LsDefinition::GetComponentType(jsonDef));

            xfStyle->set_data(&lineStyleComponentMapIndex, sizeof (lineStyleComponentMapIndex));
            lineStyleComponentMapIndex++;
            continue;
            }

        xfStyle->set_data(data, dataSize);
        }

    WriteLineStyleComponents(xfStyles);

    //  Testing styles_size is sufficient; there is no need to test _size of any
    //  of the line style component types.  If styles_size is 0 then nothing can
    //  use the components
    if (xfStyles.styles_size() != 0)
        ProcessMessage(DgnXfLib::MESSAGE_CONTEXT_Styles, xfStyles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteColors()
    {
    DgnXfLib::Colors  colors;

    // first save the 256 indexed colors
    colors.mutable_colorMap()->Reserve(256);
    DgnColorMapCP colorMap = m_project.Colors().GetDgnColorMap();
    for (int i=0; i<256; ++i)
        colors.add_colorMap(colorMap->GetColor(i).AsUInt32());

    // then save any named colors
    for (auto const& projcolor : m_project.Colors().MakeIterator())
        {
        DgnXfLib::Colors_NamedColor* color = colors.add_namedColors();
        color->set_id(projcolor.GetId().GetValue());
        color->set_rgb(IntColorDef(projcolor.GetColorValue()));
        SAVE_OPTIONAL_STRING(projcolor.GetName(), color->set_name);
        SAVE_OPTIONAL_STRING(projcolor.GetBookName(), color->set_book);
        }

    ProcessMessage(DgnXfLib::MESSAGE_CONTEXT_Colors, colors);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteRedlineModelProperties (DgnXfLib::Models_Model* model)
    {
    RedlineModel::ImageDef imageDef;
    bvector<UInt8>         imageData;
    if (RedlineModel::LoadImageData (imageDef, imageData, m_project, DgnModelId(model->id())) != BSISUCCESS)
        return;

    DgnXfLib::RedlineModelProperties* props = model->mutable_redlineProperties();

    DgnXfLib::RgbImage* rgbImage = props->mutable_backdrop();
    rgbImage->set_format (imageDef.m_format);
    rgbImage->set_pixelsPerRow (imageDef.m_sizeInPixels.x);
    rgbImage->set_pixelsPerCol (imageDef.m_sizeInPixels.y);
    rgbImage->set_data (&imageData.front(), imageData.size());

    DgnXfLib::RedlineModelProperties_Rectangle* imageRect = props->mutable_backdropArea();
    SaveDPoint2d (*imageRect->mutable_origin(), imageDef.m_origin);
    SaveDPoint2d (*imageRect->mutable_size(), imageDef.m_size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteModelTable()
    {
    DgnXfLib::Models models;

    for (auto const& projModel : m_project.Models().MakeIterator())    // write all models into the DgnXf        
        {
        DgnXfLib::Models_Model* model = models.add_models();
        model->set_id(projModel.GetModelId().GetValue());
        model->set_name(projModel.GetName());
        model->set_type((DgnXfLib::Models_ModelType) projModel.GetModelType());
        SAVE_OPTIONAL_STRING(projModel.GetDescription(), model->set_descr);
        
        int visibility = projModel.GetVisibility();
        if ((visibility & (int) ModelIterate::Gui) == 0)
            model->set_hidden (true);

        if (projModel.GetModelType() == DgnModelType::Redline)
            WriteRedlineModelProperties (model);
        }

    ProcessMessage(DgnXfLib::MESSAGE_CONTEXT_Models, models);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteModelSelectorTable()
    {
    DgnXfLib::ModelSelectors selectors;

    for (auto const& projSel : m_project.ModelSelectors().MakeIterator())
        {
        DgnXfLib::ModelSelectors_Selector* selector = selectors.add_selectors();
        selector->set_id(projSel.GetId().GetValue());
        selector->set_name(projSel.GetName());
        SAVE_OPTIONAL_STRING(projSel.GetDescription(), selector->set_descr);

        DgnModelSelection thisSel (m_project, projSel.GetId());
        thisSel.Load();
        for (auto const& modelId : thisSel)
            selector->add_models(modelId.GetValue());
        }

    ProcessMessage(DgnXfLib::MESSAGE_CONTEXT_ModelSelectors, selectors);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteViews()
    {
    for (auto const& projView : m_project.Views().MakeIterator())    // loop over all views in the project, exporting them
        {
        DgnXfLib::View thisView;

        DgnViewType viewType = projView.GetDgnViewType();
        DgnViewId   viewId   = projView.GetDgnViewId();

        auto vc = ViewController::CreateForView (m_project, projView);
        vc->Load();
        switch (viewType)
            {
            case DGNVIEW_TYPE_Physical:
                {
                auto phys = vc->ToPhysicalViewControllerP();

                // now save all of the physical view specific stuff
                DgnXfLib::View_View3d* frust3d = thisView.mutable_view3d();
                SaveDPoint3d (*frust3d->mutable_origin(), phys->GetOrigin());
                SaveDPoint3d (*frust3d->mutable_delta(), phys->GetDelta());
                SaveRotMatrix (*frust3d->mutable_rot(), phys->GetRotation());

                SAVE_OPTIONAL_ID (phys->GetDisplayStyleId(), frust3d->set_displayStyle);

                CameraViewControllerCP cameraView = phys->ToCameraViewController();
                if (cameraView != NULL && cameraView->IsCameraOn())
                    {
                    DgnXfLib::View_Camera* camera = frust3d->mutable_camera();
                    camera->set_lensAngle(cameraView->GetLensAngle());
                    camera->set_focusDistance(cameraView->GetFocusDistance());
                    SaveDPoint3d (*camera->mutable_eyePoint(), cameraView->GetEyePoint());
                    }

                // *** WIP_VIEW_CONTROLLER - what about the view-subclass-specific data?
                }
                break;

            case DGNVIEW_TYPE_Drawing:
                {
                auto drawing = vc->ToDrawingViewControllerP();

                // now save all of the drawing view specific stuff
                DgnXfLib::View_View2d* frust2d = thisView.mutable_view2d();
                SaveDPoint2d (*frust2d->mutable_origin(), drawing->GetOrigin2d());
                SaveDPoint2d (*frust2d->mutable_delta(), drawing->GetDelta2d());
                frust2d->set_rotAngle(drawing->GetRotAngle());
                }
                break;

            case DGNVIEW_TYPE_Sheet:
                {
                auto sheet = vc->ToSheetViewControllerP();

                // now save all of the sheet view specific stuff
                DgnXfLib::View_View2d* frust2d = thisView.mutable_view2d();
                SaveDPoint2d (*frust2d->mutable_origin(), sheet->GetOrigin2d());
                SaveDPoint2d (*frust2d->mutable_delta(), sheet->GetDelta2d());
                frust2d->set_rotAngle(sheet->GetRotAngle());
                }
                break;

            }

        if (!vc.IsValid()) // if we couldn't load the view, skip it.
            continue;

        // now save all of the information that is common to all view types
        thisView.set_id(viewId.GetValue());
        thisView.set_type((DgnXfLib::View_ViewType)viewType);
        // *** WIP_VIEWCONTROLLER thisView.set_typeid (thisView.GetTypeid().c_str());
        thisView.set_source((DgnXfLib::View_DgnViewSource)projView.GetDgnViewSource());
        SAVE_OPTIONAL_STRING(projView.GetName(), thisView.set_name);
        SAVE_OPTIONAL_STRING(projView.GetDescription(), thisView.set_descr);

        ViewFlags flags = vc->GetViewFlags();
        DgnXfLib::View_Flags* outFlags = thisView.mutable_flags();
        outFlags->set_fast_text(flags.fast_text);
        outFlags->set_line_wghts(flags.line_wghts);
        outFlags->set_patterns(flags.patterns);
        outFlags->set_text_nodes(flags.text_nodes);
        outFlags->set_ed_fields(flags.ed_fields);
        outFlags->set_grid(flags.grid);
        outFlags->set_constructs(flags.constructs);
        outFlags->set_dimens(flags.dimens);
        outFlags->set_fill(flags.fill);
        outFlags->set_auxDisplay(flags.auxDisplay);
        outFlags->set_renderMode(flags.renderMode);
        outFlags->set_textureMaps(flags.textureMaps);
        outFlags->set_transparency(flags.transparency);
        outFlags->set_patternDynamics(flags.patternDynamics);
        outFlags->set_renderDisplayEdges(flags.renderDisplayEdges);
        outFlags->set_renderDisplayHidden(flags.renderDisplayHidden);
        outFlags->set_overrideBgColor(flags.overrideBackground);
        outFlags->set_noFrontClip(flags.noFrontClip);
        outFlags->set_noBackClip(flags.noBackClip);
        outFlags->set_noClipVolume(flags.noClipVolume);
        outFlags->set_renderDisplayShadows(flags.renderDisplayShadows);
        outFlags->set_hiddenLineStyle(flags.hiddenLineStyle);
        outFlags->set_inhibitRenderMaterials(flags.inhibitRenderMaterials);
        outFlags->set_ignoreSceneLights(flags.ignoreSceneLights);
        if (flags.overrideBackground)
            thisView.set_bgColor (IntColorDef(vc->GetBackgroundColor()));

        Utf8String levelString;
        vc->GetLevelDisplayMask().ToString (levelString, 0);
        thisView.set_levels(levelString);
        thisView.set_baseModel(projView.GetBaseModelId().GetValue());

        // a view with no view selector shows only a single model.
        SAVE_OPTIONAL_ID(projView.GetSelectorId(), thisView.set_modelSel);

        // save this view to DgnXf
        ProcessMessage(DgnXfLib::MESSAGE_CONTEXT_View, thisView);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteElementSymbolDefinition(ElementId symbolId, DgnModelId modelId)
    {
    // see if we've already written this symbol
    if (m_savedElementSymbols.find(symbolId) != m_savedElementSymbols.end())
        return; // yes, we're done

    // save the fact that we have this symbol
    m_savedElementSymbols.insert(symbolId);

    DgnXfLib::GraphicSymbol thisSymb;
    thisSymb.set_symbolId(symbolId.GetValue());
    thisSymb.set_modelId(modelId.GetValue());

    XAttributeHandle xaTrans (symbolId, m_project, XAttributeHandlerId (XATTRIBUTEID_XGraphics, XGraphicsMinorId_SymbolTransform), 0);
    if (xaTrans.IsValid())
        thisSymb.set_transform(xaTrans.PeekData(), xaTrans.GetSize());

    XAttributeHandle xaData (symbolId, m_project, XAttributeHandlerId (XATTRIBUTEID_XGraphics, XGraphicsMinorId_Data), 0);
    if (xaData.IsValid())
        thisSymb.set_graphics (xaData.PeekData(), xaData.GetSize());

    ProcessMessage(DgnXfLib::MESSAGE_ELEMENT_GraphicSymbol, thisSymb);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
void DgnDbXfWriter::WriteStampSymbolDefinition(DgnStampId symbolId, DgnModelId modelId)
    {
    // see if we've already written this symbol
    if (m_savedStampSymbols.find(symbolId) != m_savedStampSymbols.end())
        return; // yes, we're done

    // save the fact that we have this symbol
    m_savedStampSymbols.insert(symbolId);

    DgnXfLib::GraphicSymbol thisSymb;
    thisSymb.set_symbolId(symbolId.GetValue());
    thisSymb.set_modelId(modelId.GetValue());

    XGraphicsSymbolStampPtr stamp = XGraphicsSymbolStamp::Get(m_project, symbolId);
    BeAssert(stamp.IsValid());
    if (!stamp.IsValid())
        return;

    Transform transform;
    if (stamp->GetTransform(transform))
        thisSymb.set_transform(&transform, sizeof transform);
    
    UInt32 xgraphicsSize;
    void const*xgraphicsData = stamp->GetXGraphicStream(xgraphicsSize);
    BeAssert(NULL != xgraphicsData && 0 != xgraphicsSize);
    if (NULL != xgraphicsData)
        thisSymb.set_graphics(xgraphicsData, xgraphicsSize);

    ProcessMessage(DgnXfLib::MESSAGE_ELEMENT_GraphicSymbol, thisSymb);
    }

/*---------------------------------------------------------------------------------**//**
* Returns true if schema1 < schema2, i.e., schema1 is before schema2 in dependency order
* @bsimethod                                  Ramanujam.Raman                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnDbXfWriter::ECSchemaCompare (ECN::ECSchemaCP schema1, ECN::ECSchemaCP schema2)
    {
    // Compare schema1 with references of schema2
    ECN::ECSchemaReferenceListCR referencedSchemas = schema2->GetReferencedSchemas();
    for (ECN::ECSchemaReferenceList::const_iterator it = referencedSchemas.begin(); it != referencedSchemas.end(); ++it)
        {
        // schema1 is a direct reference => schema1 < schema2
        if (it->second.get() == schema1)
            return true;

        // schema1 < reference of schema2 => schema1 < schema2
        if (ECSchemaCompare (schema1, it->second.get()))
            return true;
        }

    // Check if schema1 supplements schema2 (supplemental schemas should be imported first!)
    ECN::SupplementalSchemaMetaDataPtr metaData;
    if (ECN::SupplementalSchemaMetaData::TryGetFromSchema (metaData, *schema1) 
        && metaData.IsValid() 
        && metaData->IsForPrimarySchema (schema2->GetName(), 0, 0, ECN::SCHEMAMATCHTYPE_Latest))
        return true; 

    return false;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteECSchema (ECN::ECSchemaCP ecSchema)
    {
    Utf8String ecSchemaXml;
    ECN::SchemaWriteStatus writeStatus = ecSchema->WriteToXmlString (ecSchemaXml);
    BeAssert (writeStatus == ECN::SCHEMA_WRITE_STATUS_Success && "WriteToXmlString never had a code path returning an error");

    DgnXfLib::ECSchema xfSchema;
    xfSchema.set_name (Utf8String (ecSchema->GetName()));
    xfSchema.set_versionMajor (ecSchema->GetVersionMajor());
    xfSchema.set_versionMinor (ecSchema->GetVersionMinor());
    xfSchema.set_data (ecSchemaXml);

    for (ECN::ECClassCP ecClass : ecSchema->GetClasses())
        {
        DgnXfLib::ECSchema_ECClassMap* xfClass = xfSchema.add_classMap();
        xfClass->set_name (Utf8String (ecClass->GetName()));
        xfClass->set_classId (ecClass->GetId());
        }

    ProcessMessage (DgnXfLib::MESSAGE_EC_Schema, xfSchema);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::ExportECSchemaSection()
    {
    m_ecSchemas.clear();
    BentleyStatus status = m_project.GetEC().GetSchemaManager().GetECSchemas (m_ecSchemas, true);
    EXPECTED_CONDITION (status == SUCCESS);
    std::sort (m_ecSchemas.begin(), m_ecSchemas.end(), &ECSchemaCompare); // Sort in dependency order

    WriteSectionStart(DgnXfLib::SECTION_ECSchema);
    std::for_each (m_ecSchemas.begin(), m_ecSchemas.end(), [&](ECN::ECSchemaCP ecSchema) {WriteECSchema (ecSchema);});
    WriteSectionEnd(DgnXfLib::SECTION_ECSchema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::SetupECInstanceKeysOnElement (DgnXfLib::GraphicElement &xfElement, ECInstanceKeyMultiMap& ecKeyMap, ElementHandleCR eh)
    {
    ECInstanceKey ecPrimaryKey;
    if (DgnECPersistence::GetPrimaryInstanceOnElement (ecPrimaryKey, eh))
        {
        DgnXfLib::ECInstanceKey* xfPrimaryKey = xfElement.mutable_primaryECInstanceKey();
        xfPrimaryKey->set_classId (ecPrimaryKey.GetECClassId());
        xfPrimaryKey->set_instanceId (ecPrimaryKey.GetECInstanceId().GetValue());

        ecKeyMap.insert (ECInstanceKeyMultiMapPair(ecPrimaryKey.GetECClassId(), ecPrimaryKey.GetECInstanceId()));
        }

    bvector<ECInstanceKey> ecSecondaryKeys;
    DgnECPersistence::GetInstancesOnElement (ecSecondaryKeys, eh.GetElementId(), m_project, DgnECPersistence::AssociationType_Secondary);
    for (auto const& ecSecondaryKey : ecSecondaryKeys)
        {
        DgnXfLib::ECInstanceKey* xfSecondaryKey = xfElement.add_secondaryECInstanceKey();
        xfSecondaryKey->set_classId (ecSecondaryKey.GetECClassId());
        xfSecondaryKey->set_instanceId (ecSecondaryKey.GetECInstanceId().GetValue());

        ecKeyMap.insert (ECInstanceKeyMultiMapPair(ecSecondaryKey.GetECClassId(), ecSecondaryKey.GetECInstanceId()));
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteElement (DgnModelId modelId, ElementId elId, bool is3d)
    {
    PersistentElementRefPtr el = m_project.Models().GetElementById (elId);
    if (!el.IsValid())
        {
        BeAssert (false);
        return;
        }

    // TODO: we should convert OGREs into a single element

    XGraphicsContainer xgContainer;
    EditElementHandle eh(el.get());
    if (SUCCESS != xgContainer.ExtractFromElement (eh) && SUCCESS != xgContainer.CreateFromElement (eh))
        return;

    DgnXfLib::GraphicElement thisElem;

    thisElem.set_elementId(elId.GetValue());
    thisElem.set_modelId (modelId.GetValue());
    thisElem.set_levelId (el->GetLevel().GetValue());

    DgnElementCP eHdr = el->GetUnstableMSElementCP();

    SAVE_OPTIONAL_INT(eHdr->GetSymbology().color, thisElem.set_color);
    SAVE_OPTIONAL_INT(eHdr->GetSymbology().weight,thisElem.set_weight);
    SAVE_OPTIONAL_INT(eHdr->GetSymbology().style, thisElem.set_style);
    SAVE_OPTIONAL_INT(eHdr->IsInvisible(), thisElem.set_invisible);
    SAVE_OPTIONAL_ID (el->GetAssemblyId(), thisElem.set_assemblyId);
    SAVE_OPTIONAL_INT((DgnXfLib::GraphicElement_MSClass) eHdr->GetElementClassValue(), thisElem.set_msClass);

    ElemDisplayParams displayParams;
    eh.GetDisplayHandler()->GetElemDisplayParams (eh, displayParams, /*wantMaterials*/true);
    if (displayParams.GetFillDisplay() != FillDisplay::Never)
        {
        if (displayParams.GetGradient() != NULL)
            {
            // *** WIP_DGNXF_GRADIENT_FILL
            }
        else
            {
            SAVE_OPTIONAL_INT(displayParams.GetFillColor(), thisElem.set_fillColor);
            SAVE_OPTIONAL_INT((displayParams.GetFillDisplay() == FillDisplay::Always), thisElem.set_alwaysFilled);
            }
        }

    SAVE_OPTIONAL_DOUBLE(displayParams.GetTransparency(), thisElem.set_transparency);
    
    DRange3dCR range = el->GetRange();
    if (is3d)
        {
        if (eHdr->IsRangeValid3d())     // can be invalid for invisible elements
            {
            thisElem.set_xmin((Int64)range.low.x);
            thisElem.set_ymin((Int64)range.low.y);
            thisElem.set_zmin((Int64)range.low.z);
            thisElem.set_xmax((Int64)range.high.x);
            thisElem.set_ymax((Int64)range.high.y);
            thisElem.set_zmax((Int64)range.high.z);
            }
        }
    else
        {
        if (eHdr->IsRangeValid2d())
            {
            thisElem.set_xmin((Int64)range.low.x);
            thisElem.set_ymin((Int64)range.low.y);
            thisElem.set_xmax((Int64)range.high.x);
            thisElem.set_ymax((Int64)range.high.y);
            }
        SAVE_OPTIONAL_INT(eHdr->GetDisplayPriority(), thisElem.set_priority); // only valid for 2d elements
        }

    // TODO: Set basis transform, range

    thisElem.set_graphics(xgContainer.GetData(), xgContainer.GetDataSize());
    for (size_t i=0; i<xgContainer.GetSymbolCount(); ++i)
        {
        auto symbol = xgContainer.GetSymbolId(i);
        DgnStampId  stampId(symbol.GetValue());
        XGraphicsSymbolStampPtr stamp = XGraphicsSymbolStamp::Get(m_project, stampId);
        if (stamp.IsValid())
            WriteStampSymbolDefinition (stampId, modelId);
        else
            WriteElementSymbolDefinition (ElementId(symbol.GetValue()), modelId);

        thisElem.add_symbolIds(symbol.GetValue());
        }

    ECInstanceKeyMultiMap ecKeyMap;
    SetupECInstanceKeysOnElement (thisElem, ecKeyMap, eh);

    ProcessMessage (DgnXfLib::MESSAGE_ELEMENT_GraphicElement, thisElem);

    el = NULL;
    if (++m_nLoaded > 1000)  // every 1000 elements, purge all element memory
        {
        m_nLoaded = 0;
        m_project.Models().PurgeElementPool(0);
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WritePhysicalModel(DgnModelId modelId)
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT ElementId FROM " DGNELEMENT_TABLE_Data " WHERE ModelId=?1 AND Owner=4");
    stmt.BindId (1, modelId);

    while (BE_SQLITE_ROW == stmt.Step())
        WriteElement(modelId, stmt.GetValueId<ElementId>(0), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WritePhysicalElements()
    {
    if (DGNFILE_STATUS_Success != m_project.Models().FillDictionaryModel())
        {
        BeAssert (false);
        return;
        }

    for (auto const& projModel : m_project.Models().MakePhysicalIterator())
        WritePhysicalModel(projModel.GetModelId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteDrawingModel(DgnModelId modelId)
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT ElementId FROM " DGNELEMENT_TABLE_Data " WHERE ModelId=?1 AND Owner=5");
    stmt.BindId (1, modelId);

    while (BE_SQLITE_ROW == stmt.Step())
        WriteElement(modelId, stmt.GetValueId<ElementId>(0), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteDrawingElements()
    {
    for (auto const& projModel : m_project.Models().MakeDrawingIterator())
        WriteDrawingModel(projModel.GetModelId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteSheetElements()
    {
    for (auto const& projModel : m_project.Models().MakeSheetIterator())
        WriteDrawingModel(projModel.GetModelId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteRedlineElements()
    {
    for (auto const& projModel : m_project.Models().MakeRedlineIterator())
        WriteDrawingModel(projModel.GetModelId());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WritePhysicalRedlineElements()
    {
    for (auto const& projModel : m_project.Models().MakePhysicalIterator())
        WriteDrawingModel(projModel.GetModelId());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::GatherExportableECClasses (ECClassVectorR ecClasses, ECClassVectorR ecRelationshipClasses)
    {
    // TODO: Find a better way to identify classes of instances that won't have instances to be exported
    Utf8CP ecsqlTemplate = "SELECT NULL FROM ONLY %s";
    ECSqlStatement stmt;
    for (ECN::ECSchemaCP ecSchema : m_ecSchemas)
        {
        if (ecSchema->IsSystemSchema())
            continue;

        WCharCP schemaName = ecSchema->GetName().c_str();

        if (0 == wcscmp (L"dgn", schemaName) || 0 == wcscmp (L"Bentley_Standard_Classes", schemaName))
            continue; 
        
        for (ECN::ECClassCP ecClass : ecSchema->GetClasses())
            {
            if (ecClass->GetIsCustomAttributeClass())
                continue;
            
            stmt.Finalize ();
            Utf8String ecsql;
            ecsql.Sprintf (ecsqlTemplate, ECSqlBuilder::ToECSqlSnippet (*ecClass).c_str ());
            
            if (stmt.Prepare (m_project, ecsql.c_str ()) != ECSqlStatus::Success)
                continue;
            
            if (ecClass->GetRelationshipClassCP())
                ecRelationshipClasses.push_back (ecClass);
            else
                ecClasses.push_back (ecClass);
            }
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::WriteECInstancesOfClass (ECN::ECClassCP ecClass)
    {
    ECSqlSelectBuilder builder;
    builder.From (*ecClass, false).SelectAll();
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare (m_project, builder.ToString().c_str());
    EXPECTED_CONDITION (stat == ECSqlStatus::Success);

    ECInstanceECSqlSelectAdapter instanceAdapter (statement);
    while (statement.Step() == ECSqlStepStatus::HasRow)
        {
        ECN::IECInstancePtr instance = instanceAdapter.GetInstance();
        EXPECTED_CONDITION (instance.IsValid());

        ECInstanceId ecInstanceId;
        bool status = instanceAdapter.GetInstanceId (ecInstanceId);
        EXPECTED_CONDITION (status);
         
        Utf8String strInstance;
        ECN::InstanceWriteStatus writeStatus = instance->WriteToXmlString (strInstance, true, true);
        EXPECTED_CONDITION (writeStatus == ECN::INSTANCE_WRITE_STATUS_Success);
        
        bool isRelationship = (ecClass->GetRelationshipClassCP() != NULL);
        if (!isRelationship)
            {
            DgnXfLib::ECInstance xfInstance;

            DgnXfLib::ECInstanceKey* ecKey = xfInstance.mutable_key();
            ecKey->set_classId (ecClass->GetId());
            ecKey->set_instanceId (ecInstanceId.GetValue());

            xfInstance.set_data (strInstance);

            ProcessMessage (DgnXfLib::MESSAGE_EC_Instance, xfInstance);
            }
        else
            {
            DgnXfLib::ECRelationshipInstance xfRelationshipInstance;

            DgnXfLib::ECInstanceKey* ecKey = xfRelationshipInstance.mutable_key();
            ecKey->set_classId (ecClass->GetId());
            ecKey->set_instanceId (ecInstanceId.GetValue());
            
            ECN::IECRelationshipInstanceCP relInstance = dynamic_cast<ECN::IECRelationshipInstanceCP> (instance.get ());
            EXPECTED_CONDITION (relInstance != nullptr);

            auto sourceInstance = relInstance->GetSource ();
            ECInstanceId sourceECInstanceId;
            bool success = GetECInstanceIdFromECInstance (sourceECInstanceId, *sourceInstance);
            EXPECTED_CONDITION (success);
            const ECN::ECClassId sourceECClassId = sourceInstance->GetClass ().GetId ();

            DgnXfLib::ECInstanceKey* xfSourceKey = xfRelationshipInstance.mutable_sourceKey ();
            xfSourceKey->set_classId (sourceECClassId);
            xfSourceKey->set_instanceId (sourceECInstanceId.GetValue());

            auto targetInstance = relInstance->GetTarget ();
            ECInstanceId targetECInstanceId;
            success = GetECInstanceIdFromECInstance (targetECInstanceId, *targetInstance);
            EXPECTED_CONDITION (success);
            const ECN::ECClassId targetECClassId = targetInstance->GetClass ().GetId ();

            DgnXfLib::ECInstanceKey* xfTargetKey = xfRelationshipInstance.mutable_targetKey ();
            xfTargetKey->set_classId (targetECClassId);
            xfTargetKey->set_instanceId (targetECInstanceId.GetValue());

            xfRelationshipInstance.set_data (strInstance);

            ProcessMessage (DgnXfLib::MESSAGE_EC_RelationshipInstance, xfRelationshipInstance);
            }
        }
    }
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::ExportECInstancesSection()
    {
    bvector<ECN::ECClassCP> ecClasses, ecRelationshipClasses;
    GatherExportableECClasses (ecClasses, ecRelationshipClasses);

    WriteSectionStart(DgnXfLib::SECTION_ECInstances);
    std::for_each (ecClasses.begin(), ecClasses.end(), [&](ECN::ECClassCP ecClass) {WriteECInstancesOfClass (ecClass);});
    std::for_each (ecRelationshipClasses.begin(), ecRelationshipClasses.end(), [&](ECN::ECClassCP ecClass) {WriteECInstancesOfClass (ecClass);});
    WriteSectionEnd(DgnXfLib::SECTION_ECInstances);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::ExportContextSection()
    {
    WriteSectionStart(DgnXfLib::SECTION_Context);
    WriteDgnHeader();    
    WriteUnits();
    WriteColors();
    WriteLevelTable();
    WriteFontTable();
    WriteStyleTable();
    WriteModelTable();
    WriteModelSelectorTable();
    WriteViews();
    WriteSectionEnd(DgnXfLib::SECTION_Context);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbXfWriter::ExportElementSection()
    {
    WriteSectionStart(DgnXfLib::SECTION_Elements);
    WritePhysicalElements();
    WriteDrawingElements();
    WriteSheetElements();
    WriteRedlineElements();
    WritePhysicalRedlineElements();
    WriteSectionEnd(DgnXfLib::SECTION_Elements);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnXfLib::DgnXfStatus DgnDbXfWriter::ExportToDgnXf()
    {
    ExportContextSection();
    ExportECSchemaSection();
    ExportElementSection();
    ExportECInstancesSection();
    return  DgnXfLib::DGNXF_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnXfLib::DgnXfStatus DgnProjectToDgnXf::ProcessMessages (DgnXfLib::IMessageProcessor& proc, DgnProjectR project)
    {
    DgnDbXfWriter dgnXfWriter (project, proc);
    return dgnXfWriter.ExportToDgnXf(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/13
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnXfLib::DgnXfStatus doProcessMessages (DgnXfLib::IMessageProcessor& writer, BeFileNameCR dbFileName)
    {
    if (!DgnPlatformLib::GetHost().IsInitialized())
        {
        BeAssert (false);
        return DgnXfLib::DGNXF_STATUS_HostNotInitialized;
        }

    DgnFileStatus status;
    DgnProjectPtr proj = DgnProject::OpenProject (&status, dbFileName, DgnProject::OpenParams(Db::OPEN_Readonly));
    if (!proj.IsValid())
        return DgnXfLib::DGNXF_STATUS_CantOpen;

    return DgnPlatform::DgnProjectToDgnXf::ProcessMessages (writer, *proj);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnXfLib::DgnXfStatus DgnXfLib::DgnDbToDgnXf::ProcessMessages (DgnXfLib::IMessageProcessor& writer, BeFileNameCR dbFileName)
    {
    return doProcessMessages (writer, dbFileName);
    }

#ifdef BENTLEY_WIN32 // This host will be used only by Windows-based MicroStation

static void exporterAssertionFailureHandler (WCharCP _Message, WCharCP _File, unsigned _Line, BeAssertFunctions::AssertType atype)
    {
    // TBD report issue
    }

#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      02/2014
//=======================================================================================
struct TempHost : DgnPlatformLib::Host
{
    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override {return *new DgnPlatform::WindowsKnownLocationsAdmin();}
    virtual void _SupplyProductName (WStringR name) override {name.assign(L"");} // no product name
    virtual L10N::SqlangFiles _SupplySqlangFiles() {return L10N::SqlangFiles(BeFileName());} // no translatable strings
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnXfLib::DgnXfStatus DgnXfLib::DgnDbToDgnXf::ProcessMessagesWithTemporaryHost (DgnXfLib::IMessageProcessor& writer, BeFileNameCR dbFileName)
    {
    if (DgnPlatformLib::QueryHost() != NULL)
        return DgnXfLib::DGNXF_STATUS_HostAlreadyInitialized;

    TempHost host;
    DgnPlatformLib::Initialize(host, /*loadRscFiles*/true); // must pass true, as that tells the font manager to initialize. We'll need to look up fonts (incl. last resort) during the export.
    
    BeAssertFunctions::SetBeTestAssertHandler (exporterAssertionFailureHandler);

    DgnXfLib::DgnXfStatus status = doProcessMessages (writer, dbFileName);

    host.Terminate(false);

    return status;
    }
#endif

