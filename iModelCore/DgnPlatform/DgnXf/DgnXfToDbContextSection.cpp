/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnXf/DgnXfToDbContextSection.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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

#define WSTR(ASTR) WString((ASTR).c_str(), true).c_str()

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_FOREIGNFORMAT
using namespace ProtoBuf;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::ImportDgnHeader()
    {
    BeAssert (!m_imported.header);
    m_imported.header = true;

    // projectGuid - I don't think we should be setting the GUID! That's set when the app creates the project.
    // provenance - no such thing.
    // createdByApp - no such thing. The app's host determines the product name.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::ImportModel (DgnXfLib::Models_Model const& thisModel)
    {
    DgnModelType modelType = (DgnModelType)thisModel.type(); //  For now assume that Models_ModelType is same as DgnPlatform::ModelType

    //  Model name - make sure it's unique
    Utf8String modelName = thisModel.name();
    DgnModelId mid = m_importer.m_project.Models().QueryModelId (modelName.c_str());
    if (mid.IsValid())
        {
        WString uname;
        m_importer.m_project.Models().GetUniqueModelName (uname, WString(modelName.c_str(),true).c_str());
        modelName.Assign (uname.c_str());
        }

    DgnModels::Model newModelRow;
    newModelRow.SetModelType (modelType);
    newModelRow.SetName (modelName.c_str());
    newModelRow.SetDescription (thisModel.descr().c_str());
    newModelRow.SetVisibility (thisModel.hidden() ? (int) ModelIterate::All: 255);
    if (m_importer.m_project.Models().CreateNewModel (newModelRow) != DGNMODEL_STATUS_Success)
        {
        m_importer.ReportIssue (DgnProjectFromDgnXfMessageProcessor::IMPORT_ISSUE_ImportFailure, L"Model");
        return;
        }

    m_importer.m_remapper->RecordModelId (thisModel.id(), newModelRow.GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::ImportModels()
    {
    BeAssert (!m_imported.models);
    BeAssert (m_imported.units);

    m_imported.models = true;

    for (int i=0; i<m_modelsEntry.models_size(); ++i)
        {
        DgnXfLib::Models_Model const& thisModel = m_modelsEntry.models(i);
        ImportModel (thisModel);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::ImportUnits()
    {
    BeAssert (!m_imported.units);
    m_imported.units = true;

    DgnXfLib::StorageUnits const& physicalUnits = m_unitsEntry.physicalUnits();

    DgnXfLib::UnitDefinition const& unitDef = physicalUnits.units();
    UnitDefinition storageUnitsBase ((UnitBase)unitDef.base(), (UnitSystem)unitDef.system(), unitDef.numerator(), unitDef.denominator(), WSTR(unitDef.label()));

    StorageUnits projStorage (storageUnitsBase, physicalUnits.uorPerStorage(), physicalUnits.solidExtent());
    projStorage.SetGlobalOrigin (DgnProjectFromDgnXfMessageProcessor::GetDPoint3d (physicalUnits.globalOrigin()));

    DgnUnits& projUnits = m_importer.m_project.Units();
    projUnits.GetPhysicalUnits() = projStorage;
    projUnits.SetAzimuth (m_unitsEntry.azimuth());
    projUnits.SetOriginLatitude (m_unitsEntry.latitude());
    projUnits.SetOriginLongitude (m_unitsEntry.longitude());
    projUnits.Save();

    if (m_unitsEntry.has_extent())
        projUnits.SaveProjectExtents (DgnProjectFromDgnXfMessageProcessor::GetDRange3d (m_unitsEntry.extent()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::ImportLevel (DgnXfLib::Levels_Level const& xfLevel)
    {
    UInt32 color = m_importer.m_remapper->RemapElementColor (xfLevel.color());
    DgnStyleId lineStyle = m_importer.m_remapper->RemapLineStyle (xfLevel.style());
    DgnMaterialId material = m_importer.m_remapper->RemapMaterialId (xfLevel.material());

    //  Level name - make sure it's unique.
    Utf8String levelName (xfLevel.name().c_str());

    int index = 0;
    Utf8String baseName (levelName);
    while (m_importer.m_project.Levels().QueryLevelId(levelName.c_str()).IsValid())
        levelName = Utf8PrintfString("%s-%d", baseName.c_str(), ++index);

    //  LevelId - must be unique. Don't care what its value is.
    LevelId levelId = (LevelId)xfLevel.id();
    if (m_importer.m_project.Levels().QueryLevelById (levelId).IsValid())
        levelId = LevelId(1 + m_importer.m_project.Levels().QueryHighestId().GetValue());

    //  Should be able to create a new level with this unique name and id
    DgnLevels::Facet::Appearance graphics;
    graphics.SetColor(color);
    graphics.SetWeight(xfLevel.weight());
    graphics.SetStyle(lineStyle.GetValue());
    graphics.SetMaterial(material);
    graphics.SetTransparency(xfLevel.transparency());
    graphics.SetDisplayPriority(xfLevel.displayPriority());
    DgnLevels::Level level (levelId, levelName.c_str(), (DgnLevels::Scope) xfLevel.scope(), xfLevel.descr().c_str());
    m_importer.m_project.Levels().InsertLevel (level, graphics);

    m_importer.m_remapper->RecordLevelId (xfLevel.id(), levelId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::ImportLevels()
    {
    BeAssert (!m_imported.levels);
    BeAssert (m_imported.colors);
    /* *** WIP_DGNXFTOV8_LINESTYLES
    BeAssert (m_imported.lineStyles);
    BeAssert (m_imported.materials);
    */

    m_imported.levels = true;

    for (int i=0; i<m_levelsEntry.levels_size(); ++i)
        {
        ImportLevel (m_levelsEntry.levels(i));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::ImportDisplayStyle (DgnXfLib::Styles_Style const& thisStyle)
    {
    m_importer.ReportIssue(DgnProjectFromDgnXfMessageProcessor::IMPORT_ISSUE_TBDStyles, L"DisplayStyle");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::ImportStyles()
    {
    for (int i=0; i<m_stylesEntry.styles_size(); ++i)
        {
        DgnXfLib::Styles_Style const& thisStyle = m_stylesEntry.styles(i);

        switch (thisStyle.type())
            {
            case DgnXfLib::Styles_StyleType_Display:
                ImportDisplayStyle (thisStyle);
                break;

            case DgnXfLib::Styles_StyleType_Line:
                {
                m_importer.ReportIssue(DgnProjectFromDgnXfMessageProcessor::IMPORT_ISSUE_TBDStyles, L"LineStyle");
                break;
                }

            case DgnXfLib::Styles_StyleType_Text:
                {
                m_importer.ReportIssue(DgnProjectFromDgnXfMessageProcessor::IMPORT_ISSUE_TBDStyles, L"TextStyle");
                break;
                }

            default:
                BeAssert (false);
                m_importer.ReportIssue (DgnProjectFromDgnXfMessageProcessor::IMPORT_ISSUE_IncorrectData, WPrintfString(L"Style %d",thisStyle.type()));
                break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::ImportFont (DgnXfLib::Fonts_Font const& thisFont)
    {
    if (thisFont.type() != DgnXfLib::Fonts_FontType_DGNFONTTYPE_TrueType)
        {
        // *** WIP_FONT - We can't import RSC or SHX fonts, because the DgnXf file does not contain the font data that we'd need to embed.
        m_importer.ReportIssue (DgnProjectFromDgnXfMessageProcessor::IMPORT_ISSUE_TBDStyles, L"RSC Fonts");
        return;
        }
    m_importer.m_project.Fonts().InsertFontNumberMappingDirect(thisFont.id(), (DgnFontType)thisFont.type(), thisFont.name().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::ImportFonts()
    {
    for (int i=0; i<m_fontsEntry.fonts_size(); ++i)
        {
        ImportFont (m_fontsEntry.fonts(i));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::ImportColors()
    {
    BeAssert (!m_imported.colors);
    m_imported.colors = true;

    if (m_colorsEntry.colorMap_size() != DgnColorMap::INDEX_ColorCount)
        {
        m_importer.ReportIssue (DgnProjectFromDgnXfMessageProcessor::IMPORT_ISSUE_IncorrectData, L"Color");
        return;
        }

    //  ------------------------------------------------------
    //  256 indexed colors
    //      DgnXf format stores color map entries in internal order.
    //  ------------------------------------------------------
    UInt32* colors = m_importer.m_project.Colors().GetDgnColorMapP()->GetTbgrColorsP();
    for (int i=0; i<DgnColorMap::INDEX_ColorCount; ++i)
        {
        colors[i] = m_colorsEntry.colorMap(i);
        }

    //  ------------------------------------------------------
    //  Named colors (RGB and ColorBook)
    //  ------------------------------------------------------
    for (int i=0; i<m_colorsEntry.namedColors_size(); ++i)
        {
        DgnXfLib::Colors_NamedColor namedColor = m_colorsEntry.namedColors(i);

        UInt32 xfId = (UInt32)namedColor.id();
        Utf8String book = namedColor.book();
        Utf8String name = namedColor.name();

        IntColorDef intColor;
        intColor.m_int = (UInt32)namedColor.rgb();

        DgnTrueColorId newId = m_importer.m_project.Colors().InsertTrueColor(intColor.m_rgb, name.c_str(), book.c_str());

        m_importer.m_remapper->RecordExtendedColor (xfId, newId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::ImportModelSelectors ()
    {
    BeAssert (m_imported.models);
    BeAssert (!m_imported.modelSelectors);
    m_imported.modelSelectors = true;

    for (int i=0; i<m_modelSelectorsEntry.selectors_size(); ++i)
        {
        DgnXfLib::ModelSelectors_Selector const& selector = m_modelSelectorsEntry.selectors(i);

        DgnModelSelectorId selId (selector.id());

        //  Selector name - make sure it's unique
        Utf8String selName (selector.name());
        Utf8String baseName (selName);
        int index = 0;
        while (m_importer.m_project.ModelSelectors().QuerySelectorId (selName.c_str()).IsValid())
            selName = Utf8PrintfString ("%s-%d", baseName.c_str(), ++index);

        DgnModelSelectors::Selector row (selName.c_str(), selector.descr().c_str());
        m_importer.m_project.ModelSelectors().InsertSelector (row);

        m_importer.m_remapper->RecordModelSelectorId (selector.id(), row.GetId());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::SaveViewProperties (DgnViews::View const& entry, DgnXfLib::View const& view)
    {
    DgnXfLib::View_Flags const& xfFlags = view.flags();

    ViewFlags flags;
    memset (&flags, 0, sizeof(flags));
    flags.fast_text = xfFlags.fast_text();

    flags.line_wghts               = xfFlags.line_wghts();
    flags.patterns                 = xfFlags.patterns();
    flags.text_nodes               = xfFlags.text_nodes();
    flags.ed_fields                = xfFlags.ed_fields();
    flags.grid                     = xfFlags.grid();
    flags.constructs               = xfFlags.constructs();
    flags.dimens                   = xfFlags.dimens();
    flags.fill                     = xfFlags.fill();
    flags.auxDisplay               = xfFlags.auxDisplay();
    flags.renderMode               = xfFlags.renderMode();
    flags.textureMaps              = xfFlags.textureMaps();
    flags.transparency             = xfFlags.transparency();
    flags.inhibitLineStyles        = xfFlags.inhibitLineStyles();
    flags.patternDynamics          = xfFlags.patternDynamics();
    flags.renderDisplayEdges       = xfFlags.renderDisplayEdges();
    flags.renderDisplayHidden      = xfFlags.renderDisplayHidden();
    flags.overrideBackground       = xfFlags.overrideBgColor();
    flags.noFrontClip              = xfFlags.noFrontClip();
    flags.noBackClip               = xfFlags.noBackClip();
    flags.noClipVolume             = xfFlags.noClipVolume();
    flags.renderDisplayShadows     = xfFlags.renderDisplayShadows();
    flags.hiddenLineStyle          = xfFlags.hiddenLineStyle();
    flags.inhibitRenderMaterials   = xfFlags.inhibitRenderMaterials();
    flags.ignoreSceneLights        = xfFlags.ignoreSceneLights();

    // validate old data.
    if (flags.renderMode > (UInt32)MSRenderMode::Phong)
        flags.renderMode = (UInt32)MSRenderMode::Wireframe;

    ViewControllerPtr thisView = ViewController::CreateForView (m_importer.m_project, entry);
    if (DGNVIEW_TYPE_Physical == entry.GetDgnViewType())
        {
        DgnXfLib::View_View3d const& view3d = view.view3d();

        thisView->SetOrigin (DgnProjectFromDgnXfMessageProcessor::GetDPoint3d (view3d.origin()));
        thisView->SetDelta (DgnProjectFromDgnXfMessageProcessor::GetDVec3d (view3d.delta()));
        thisView->SetRotation (DgnProjectFromDgnXfMessageProcessor::GetRotMatrix (view3d.rot()));

        auto cameraView = thisView->ToCameraViewControllerP();
        if (cameraView != NULL)
            {
            cameraView->SetCameraOn (view3d.has_camera());
            if (view3d.has_camera())
                {
                DgnXfLib::View_Camera const& camera = view3d.camera();
                cameraView->SetLensAngle (camera.lensAngle());
                cameraView->SetFocusDistance (camera.focusDistance());
                cameraView->SetEyePoint (DgnProjectFromDgnXfMessageProcessor::GetDPoint3d (camera.eyePoint()));
                }
            }

        if (view3d.has_displayStyle())
            thisView->ToPhysicalViewControllerP()->SetDisplayStyle (m_importer.m_remapper->RemapDisplayStyleId (view3d.displayStyle()));
        }
    else
        {
        if (dynamic_cast<ViewController2d*>(thisView.get()) != NULL)
            {
            DgnXfLib::View_View2d const& view2d = view.view2d();

            thisView->SetOrigin (DPoint3d::From (DgnProjectFromDgnXfMessageProcessor::GetDPoint2d (view2d.origin())));
            thisView->SetDelta (DgnProjectFromDgnXfMessageProcessor::GetDVec3d (view2d.delta()));
            RotMatrix rot;
            rot.InitFromAxisAndRotationAngle (0, view2d.rotAngle());
            thisView->SetRotation (rot);
            }
        }

    if (view.has_bgColor())
        {
        flags.overrideBackground = true;
        IntColorDef intColor;
        intColor.m_int = view.bgColor();
        thisView->SetBackgroundColor (intColor.m_rgb);
        }

    thisView->GetViewFlagsR() = flags;
    thisView->Save();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::ImportView (DgnXfLib::View const& view)
    {
    //  For now, we know that the enum values for the following types are the same in DgnXf as they are in our .h files.
    DgnViewType viewType = (DgnViewType)view.type();
    DgnViewSource viewSource = (DgnViewSource)view.source();
    Utf8CP viewSubType = "";    // WIP_DGNXF ViewController -- DgnXf does not yet support view subtypes. Use legacy defaults.

    // Ignore viewid in DgnXf data. Nothing in the file depends on this ID.  Let DgnDb file issue a view id.

    //  View name - make sure it's unique.
    Utf8String viewName (view.name());
    Utf8String baseName (viewName);
    int index = 0;
    while (m_importer.m_project.Views().QueryViewId (viewName.c_str()).IsValid())
        viewName = Utf8PrintfString ("%s-%d", baseName.c_str(), ++index);


    DgnModelId baseModelId = m_importer.m_remapper->RemapModelId (view.baseModel());

    DgnModelSelectorId selectorId = m_importer.m_remapper->RemapModelSelectorId (view.modelSel());

    DgnViews::View row (viewType, viewSubType, baseModelId, viewName.c_str(), view.descr().c_str(), selectorId, viewSource);

    if (m_importer.m_project.Views().InsertView (row) != BE_SQLITE_OK)
        m_importer.ReportIssue (DgnProjectFromDgnXfMessageProcessor::IMPORT_ISSUE_ImportFailure, L"View");

    SaveViewProperties (row, view);

    m_importer.m_project.Views().SavePropertyString (row.GetId(), DgnViewProperty::LevelMask(), view.levels());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::ImportViews()
    {
    BeAssert (m_imported.levels);
    BeAssert (m_imported.models);
    BeAssert (m_imported.modelSelectors);
    //BeAssert (m_imported.displayStyles);  There may be no display styles

    BeAssert (!m_imported.views);
    m_imported.views = true;

    FOR_EACH (DgnXfLib::View view, m_viewEntries)
        ImportView (view);
    }

/*---------------------------------------------------------------------------------**//**
* This function generates style tables, etc. from context section messages.
* This function knows how to create styles in dependency order.
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::OnContextSectionEnd ()
    {
    // Process context sections *in dependency order*
    ImportDgnHeader();
    ImportUnits();
    ImportFonts();
    ImportColors();
    ImportStyles();
    ImportLevels();
    ImportModels();
    ImportModelSelectors();
    ImportViews();
    }

/*---------------------------------------------------------------------------------**//**
* This function handles messages that occur in the context section.
* It changes the state to neutral when it encounters a section-end message.
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbContextSection::ProcessContextSectionMessage (DgnXfLib::DgnXfMessageId entryType, ProtoBuf::MessageLite& msg)
    {
    BeAssert (m_importer.m_inSection == DgnProjectFromDgnXfMessageProcessor::IN_SECTION_Context);

    if (m_importer.m_imported.context)
        {
        BeDataAssert (false && "duplicate context sections");
        m_importer.ReportIssue (DgnProjectFromDgnXfMessageProcessor::IMPORT_ISSUE_IncorrectData, L"Context");
/*<==*/ return;
        }

    switch (entryType)
        {
        default:
            m_importer.ReportIssue (DgnProjectFromDgnXfMessageProcessor::IMPORT_ISSUE_UnrecognizedMessage, L"");
/*<==*/     return;

        //  Just make a copy of each message as it comes through. We have to wait until the end to process these messages.
        case DgnXfLib::MESSAGE_CONTEXT_DgnHeader:       m_hdrEntry      = (DgnXfLib::DgnHeader&)msg;            break;
        case DgnXfLib::MESSAGE_CONTEXT_Models:          m_modelsEntry   = (DgnXfLib::Models&)msg;               break;
        case DgnXfLib::MESSAGE_CONTEXT_ModelSelectors:  m_modelSelectorsEntry = (DgnXfLib::ModelSelectors&)msg; break;
        case DgnXfLib::MESSAGE_CONTEXT_Units:           m_unitsEntry    = (DgnXfLib::Units&)msg;                break;
        case DgnXfLib::MESSAGE_CONTEXT_Styles:          m_stylesEntry   = (DgnXfLib::Styles&)msg;               break;
        case DgnXfLib::MESSAGE_CONTEXT_Levels:          m_levelsEntry   = (DgnXfLib::Levels&)msg;               break;
        case DgnXfLib::MESSAGE_CONTEXT_Fonts:           m_fontsEntry    = (DgnXfLib::Fonts&)msg;                break;
        case DgnXfLib::MESSAGE_CONTEXT_Colors:          m_colorsEntry   = (DgnXfLib::Colors&)msg;               break;
        case DgnXfLib::MESSAGE_CONTEXT_View:            m_viewEntries.push_back ((DgnXfLib::View&)msg);         break;

        //  Now that we have all of the context-related messages, we can create style tables, etc. from them.
        case DgnXfLib::MESSAGE_SectionEnd:
            OnContextSectionEnd();
            m_importer.m_imported.context = true;
            m_importer.m_inSection = DgnProjectFromDgnXfMessageProcessor::IN_SECTION_Neutral;
            break;
        }
    }



