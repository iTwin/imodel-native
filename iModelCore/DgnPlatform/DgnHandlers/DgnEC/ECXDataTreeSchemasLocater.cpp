/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnEC/ECXDataTreeSchemasLocater.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE


// The following detailingsymbol strings should not be translated because
// they are used as string-based indices to retrieve data stored in dgn files., Hence DO NOT MOVE THEM FROM HERE
// NOTE: Although "Marker" has been replaced with "Callout" throughout the source code,
// but resources contain Marker terminology from the beginning, so do not alter the macro values for any reasons.
// Define Types
#define DETAILINGSYMBOL_TYPE_TITLETEXT                          L"TitleText"
#define DETAILINGSYMBOL_TYPE_DRAWINGTITLE                       L"DrawingViewMarker"
#define DETAILINGSYMBOL_TYPE_ELEVATIONCALLOUT                   L"ElevationMarker"
#define DETAILINGSYMBOL_TYPE_DETAILCALLOUT                      L"DetailMarker"
#define DETAILINGSYMBOL_TYPE_SECTIONCALLOUT                     L"SectionMarker"

// Define parts of a detailing symbol
#define DETAILINGSYMBOL_PART_TITLE                              L"Title"
#define DETAILINGSYMBOL_PART_DETAILAREA                         L"DetailArea"
#define DETAILINGSYMBOL_PART_MAINTERMINATOR                     L"MainTerminator"
#define DETAILINGSYMBOL_PART_MAINWING                           L"MainWing"
#define DETAILINGSYMBOL_PART_TAILTERMINATOR                     L"TailTerminator"
#define DETAILINGSYMBOL_PART_TAILWING                           L"TailWing"
#define DETAILINGSYMBOL_PART_LEADER                             L"Leader"

// Define Property Types
#define DETAILINGSYMBOL_PROPERTYTYPE_LEVEL                      L"Level"
#define DETAILINGSYMBOL_PROPERTYTYPE_CELL                       L"Cell"
#define DETAILINGSYMBOL_PROPERTYTYPE_COLOR                      L"Color"
#define DETAILINGSYMBOL_PROPERTYTYPE_STYLE                      L"Style"
#define DETAILINGSYMBOL_PROPERTYTYPE_WEIGHT                     L"Weight"

// Define properties
#define DETAILINGSYMBOL_PROPERTY_LEVEL                          L"Level"
#define DETAILINGSYMBOL_PROPERTY_CELL                           L"Cell"
#define DETAILINGSYMBOL_PROPERTY_COLOR                          L"Color"
#define DETAILINGSYMBOL_PROPERTY_STYLE                          L"Style"
#define DETAILINGSYMBOL_PROPERTY_WEIGHT                         L"Weight"

// NEEDSWORK - shouldn't turn this warning off, but file is in transition.
#pragma warning(disable:4189)  // Variable initialized but not used

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDataTreeSchemasLocater::ECXDataTreeSchemasLocater() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ECXDataTreeSchemasLocater::RegisterSchemas ()
    {
    static ECXDataTreeSchemasLocater* s_singletonECXDataTreeSchemasLocater = NULL;
    if (NULL != s_singletonECXDataTreeSchemasLocater)
        return;
        
    s_singletonECXDataTreeSchemasLocater = new ECXDataTreeSchemasLocater();
    
    s_singletonECXDataTreeSchemasLocater->CreateElementTemplateSchema ();
    s_singletonECXDataTreeSchemasLocater->CreateCustomInterfaceSchema ();
    s_singletonECXDataTreeSchemasLocater->CreateDetailingSymbolStyleSchema ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static ECClassP        GenerateStruct (WCharCP name, ECClassP baseClass, ECSchemaR schema)
    {
    ECClassP ecStruct;
    schema.CreateClass (ecStruct, name);

    if (baseClass)
        ecStruct->AddBaseClass(*baseClass);

    ecStruct->SetIsStruct (true);
    return ecStruct;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static StructECPropertyP    CreateStructProperty (WCharCP name, ECClassP structType, ECClassP parentClass, Int32 priority, WCharCP category=NULL)
    {
    StructECPropertyP  structProp;
    parentClass->CreateStructProperty(structProp, name, *structType);

    //ECUI::ECPropertyPane::SetCategory (structProp, category);
    // need to set priority for grid

    return structProp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static PrimitiveECPropertyP  CreatePrimitiveProperty (WCharCP name, PrimitiveType primitiveType, ECClassP ecStruct, Int32 priority, WCharCP category=NULL)
    {
    PrimitiveECPropertyP  ecProperty;
    ecStruct->CreatePrimitiveProperty(ecProperty, name, primitiveType);

    // need to set priority for grid

    return ecProperty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static ArrayECPropertyP  CreateStructArrayProperty (WCharCP name, ECClassP ecClass, ECClassP ecStruct, Int32 priority, WCharCP category=NULL)
    {
    ArrayECPropertyP ecStructArray;
    ecClass->CreateArrayProperty (ecStructArray, name, ecStruct);

    // need to set priority for grid
    return ecStructArray;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void  ECXDataTreeSchemasLocater::CreateElementTemplateSchema ()
    {
    DgnECManager::GetManager().CreateSchema (m_elementTemplateSchemaHolder, L"Ustn_ElementParams", 1, 0, NULL);
    //WIP: Abeesh TODO register this schema in SharedSchemaCache?
    m_elementTemplateSchemaHolder->SetNamespacePrefix (L"tbep");

    ECClassP baseClass;

    m_elementTemplateSchemaHolder->CreateClass (baseClass, L"Ustn_ElementParams");
    baseClass->SetIsStruct (true);

    ECClassP              criteriaStruct    = GenerateStruct (L"CriteriaStruct", NULL, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  criteriaProp      = CreatePrimitiveProperty(L"Criteria", PRIMITIVETYPE_String, criteriaStruct, 0);

    ECClassP              stringParamsStruct = GenerateStruct (L"StringParam", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  stringProp         = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_String, stringParamsStruct, 99000);

    ECClassP              booleanParamsStruct = GenerateStruct (L"BoolParam", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  booleanProp         = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_Boolean, booleanParamsStruct, 99000);

    ECClassP              scaleParamsStruct = GenerateStruct (L"ScaleParam", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  scaleProp         = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_Double, scaleParamsStruct, 99000);
    // scaleProp->MinimumValue = System::Double (s_minScaleValue);

    ///////////////////////////////////////////////////////////
    //  General Settings
    ///////////////////////////////////////////////////////////
    // --- Level ---
    ECClassP              levelNameStruct     = GenerateStruct (L"Level", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  levelNameProperty   = CreatePrimitiveProperty (L"Value", PRIMITIVETYPE_String, levelNameStruct, 99000);
    ArrayECPropertyP      levelsArrayProperty = CreateStructArrayProperty (L"Levels", baseClass, levelNameStruct, 100000, L"GeneralCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (levelNameProperty);
    //ECUI::ECPropertyPane::SetExtendedType (levelNameProperty, s_LevelNameET); // --- used to display value in Reports ---
    // ECUI::ECPropertyPane::SetExtendedType (levelsArrayProperty, s_ShowArrayAsStringET);
    // ECUI::ECPropertyPane::SetMemberExtendedType (levelsArrayProperty, s_LevelStructExtendedType);

    // --- Color ---
    // install the type for ECElementColor. We use it to store template colors.
    //    IECType^        colorType;
    //    if (nullptr == (colorType = ECO::ECObjects::GetInstalledType (L"ECElementColor")))
    //        ECO::ECObjects::AddInstalledType (L"ECElementColor", colorType = DECP::ECElementColorType::Instance);
    ECClassP              colorStruct      = GenerateStruct (L"Color", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  colorProperty    = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_Binary, colorStruct, 99000); // use binary since Installed type not supported
    ArrayECPropertyP      colorStructArray = CreateStructArrayProperty(L"Colors", baseClass, colorStruct, 99000, L"GeneralCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (colorProperty);
    //ECUI::ECPropertyPane::SetExtendedType (colorArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (colorArrayProperty, s_ColorStructExtendedType);

    // --- line style ---
    ECClassP              lineStyleStruct   = GenerateStruct (L"LineStyle", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  lineStyleProperty = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_String, lineStyleStruct, 99000);
    ArrayECPropertyP lineStyleStructArray   = CreateStructArrayProperty(L"LineStyles", baseClass, lineStyleStruct, 98000, L"GeneralCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (lineStyleProperty);
    //ECUI::ECPropertyPane::SetExtendedType (lineStyleArrayProperty, s_ShowLineStyleArrayAsStringET);
    //ECUI::ECPropertyPane::SetExtendedType (lineStyleProperty, s_LineStyleNameExtendedType);  // --- used to display value in Reports ---
    //ECUI::ECPropertyPane::SetMemberExtendedType (lineStyleArrayProperty, s_LineStyleStructExtendedType); // --- used to display and edit values in property pane ---

    // --- Weight ---
    ECClassP              weightStruct      = GenerateStruct (L"Weight", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  weightProperty    = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_Integer, weightStruct, 99000);
    ArrayECPropertyP      weightStructArray = CreateStructArrayProperty (L"Weights", baseClass, weightStruct, 97000, L"GeneralCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (weightProperty);
    //ECUI::ECPropertyPane::SetExtendedType (weightArrayProperty, s_ShowLineWeightArrayAsStringET);
    //ECUI::ECPropertyPane::SetExtendedType (weightProperty, s_WeightExtendedType); // --- used to display value in Reports ---
    //ECUI::ECPropertyPane::SetMemberExtendedType (weightArrayProperty, s_WeightStructExtendedType); // --- used to display and edit values in property pane ---

    // --- Class ---
    ECClassP              classStruct      = GenerateStruct (L"ElementClass", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  classProperty    = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_Integer, classStruct, 99000);
    ArrayECPropertyP      classStructArray = CreateStructArrayProperty (L"ElementClasses", baseClass, classStruct, 96000, L"GeneralCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (classProperty);
    //ECUI::ECPropertyPane::SetExtendedType (classArrayProperty, s_ShowClassArrayAsStringET);
    //ECUI::ECPropertyPane::SetExtendedType (classProperty, s_ElementClassNameExtendedType); // --- used to display value in Reports ---
    //ECUI::ECPropertyPane::SetMemberExtendedType (classArrayProperty, s_ClassStructExtendedType); // --- used to display and edit values in property pane ---

    // --- Transparency ---
    ECClassP              transparencyStruct      = GenerateStruct (L"Transparency", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  transparencyProperty    = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_Double, transparencyStruct, 99000);
    ArrayECPropertyP      transparencyStructArray = CreateStructArrayProperty (L"Transparencies", baseClass, transparencyStruct, 95000, L"GeneralCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (transparencyProperty);
    //ECUI::ECPropertyPane::SetExtendedType (transparencyArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (transparencyArrayProperty, s_TransparencyStructExtendedType);

    // --- Priority ---
    ECClassP              priorityStruct      = GenerateStruct (L"Priority", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  priorityProperty    = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_Integer, priorityStruct, 99000);
    ArrayECPropertyP      priorityStructArray = CreateStructArrayProperty(L"Priorities", baseClass, priorityStruct, 94000, L"GeneralCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (priorityProperty);
    //ECUI::ECPropertyPane::SetExtendedType (priorityArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (priorityArrayProperty, s_PriorityStructExtendedType);

    // --- LineStyleParam ---
    ECClassP              lineStyleParamStruct  = GenerateStruct (L"LineStyleParam", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  lsscaleProp             = CreatePrimitiveProperty(L"LSScale",   PRIMITIVETYPE_Double, lineStyleParamStruct, 100000);
    //scaleProp->MinimumValue = System::Double (s_minScaleValue);
    PrimitiveECPropertyP  dashScaleProp         = CreatePrimitiveProperty(L"DashScale", PRIMITIVETYPE_Double, lineStyleParamStruct, 99000);
    //dashScaleProp->MinimumValue = System::Double (s_minScaleValue);
    PrimitiveECPropertyP  gapScaleProp          = CreatePrimitiveProperty(L"GapScale",  PRIMITIVETYPE_Double, lineStyleParamStruct, 98000);
    //gapScaleProp->MinimumValue = System::Double (s_minScaleValue);
    PrimitiveECPropertyP  startWidthProperty    = CreatePrimitiveProperty(L"StartWidth",  PRIMITIVETYPE_Double, lineStyleParamStruct, 97000);
    //startWidthProperty->MinimumValue = System::Double (0.0);
    //ECUI::ECPropertyPane::SetExtendedType (startWidthProperty, s_SimpleDistanceET);
    PrimitiveECPropertyP  endWidthProperty      = CreatePrimitiveProperty(L"EndWidth",  PRIMITIVETYPE_Double, lineStyleParamStruct, 96000);
    //endWidthProperty->MinimumValue = System::Double (0.0);
    //ECUI::ECPropertyPane::SetExtendedType (endWidthProperty, s_SimpleDistanceET);
    PrimitiveECPropertyP  shiftProperty         = CreatePrimitiveProperty(L"ShiftDistance",  PRIMITIVETYPE_Double, lineStyleParamStruct, 95000);
    //ECUI::ECPropertyPane::SetExtendedType (shiftProperty, s_SimpleDistanceET);
    PrimitiveECPropertyP  shiftFractionProperty = CreatePrimitiveProperty(L"ShiftFraction",  PRIMITIVETYPE_Double, lineStyleParamStruct, 94000);
    ArrayECPropertyP  lineStyleParamStructArray = CreateStructArrayProperty(L"LineStyleParams", baseClass, lineStyleParamStruct, 93500, L"GeneralCategory");

    ///////////////////////////////////////////////////////////
    //  Linear Settings
    ///////////////////////////////////////////////////////////
    // --- AreaMode ---
    ECClassP              areaModeStruct        = GenerateStruct (L"AreaMode", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  areaModeProperty      = CreatePrimitiveProperty (L"Value", PRIMITIVETYPE_Integer, areaModeStruct, 99000);
    ArrayECPropertyP      areaModeArrayProperty = CreateStructArrayProperty (L"AreaModes", baseClass, areaModeStruct, 100000, L"LinearCategory");
    //ECUI::ECPropertyPane::SetExtendedType (areaModeProperty, s_AreaModeExtendedType); // --- used to display value in Reports ---
    //ECUI::ECPropertyPane::SetMemberExtendedType (areaModeArrayProperty, s_AreaModeStructExtendedType);  // --- used to display and edit values in property pane ---

    // --- FillMode ---
    ECClassP              fillStruct        = GenerateStruct (L"FillMode", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  fillProperty      = CreatePrimitiveProperty (L"Value", PRIMITIVETYPE_Integer, fillStruct, 99000);
    ArrayECPropertyP      fillArrayProperty = CreateStructArrayProperty (L"FillModes", baseClass, fillStruct, 99000, L"LinearCategory");
    //ECUI::ECPropertyPane::SetExtendedType (fillArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetExtendedType (fillProperty, s_AreaFillExtendedType); // --- used to display value in Reports ---
    //ECUI::ECPropertyPane::SetMemberExtendedType (fillArrayProperty, s_FillStructExtendedType); // --- used to display and edit values in property pane ---

    // --- FillColor ---
    // install the type for ECElementFillColor. We use it to store template fillcolors.
    //IECType^        fillcolorType;
    //if (nullptr == (fillcolorType = ECO::ECObjects::GetInstalledType (L"ECElementFillColor")))
    //    ECO::ECObjects::AddInstalledType (L"ECElementFillColor", fillcolorType = DECP::ECElementFillColorType::Instance);
    ECClassP              fillcolorStruct        = GenerateStruct (L"FillColor", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  fillcolorProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_Binary, fillcolorStruct, 99000); // use binary since Installed type not supported
    ArrayECPropertyP      fillcolorArrayProperty = CreateStructArrayProperty(L"FillColors", baseClass, fillcolorStruct, 98000, L"LinearCategory");
    // ECUI::ECPropertyPane::SetRequiresRefresh (fillcolorProperty);
    //ECUI::ECPropertyPane::SetExtendedType (fillcolorArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (fillcolorArrayProperty, s_FillcolorStructExtendedType);

    // =================== Cell Settings ===============
    // --- CellName ---
    ECClassP              cellNameStruct        = GenerateStruct (L"CellName", stringParamsStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  cellNameProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_String, cellNameStruct, 99000);
    ArrayECPropertyP      cellNameArrayProperty = CreateStructArrayProperty(L"CellNames", baseClass, cellNameStruct, 100000, L"CellCategory");
    //ECUI::ECPropertyPane::SetExtendedType (cellNameArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (cellNameArrayProperty, s_CellNameStructExtendedType);

    // --- Scale ---
    ECClassP              scaleStruct        = GenerateStruct (L"Scale", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  scaleProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_Point3D, scaleStruct, 99000); 
    ArrayECPropertyP      scaleArrayProperty = CreateStructArrayProperty(L"Scales", baseClass, scaleStruct, 99500, L"CellCategory");
    // ECUI::ECPropertyPane::SetRequiresRefresh (scaleProperty);
    //ECUI::ECPropertyPane::SetExtendedType (scaleArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (scaleArrayProperty, s_NonZeroPointStructET);

    // --- Terminator ---
    ECClassP              terminatorStruct        = GenerateStruct (L"Terminator", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  terminatorProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_String, terminatorStruct, 99000); 
    ArrayECPropertyP      terminatorArrayProperty = CreateStructArrayProperty(L"Terminators", baseClass, terminatorStruct, 98000, L"CellCategory");
    // ECUI::ECPropertyPane::SetRequiresRefresh (terminatorProperty);
    //ECUI::ECPropertyPane::SetExtendedType (terminatorArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (terminatorArrayProperty, s_TerminatorStructExtendedType);

    // --- Terminator Scale ---
    ECClassP              terminatorScaleStruct        = GenerateStruct (L"TerminatorScale", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  terminatorScaleProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_Double, terminatorScaleStruct, 99000); 
    ArrayECPropertyP      terminatorScaleArrayProperty = CreateStructArrayProperty(L"TerminatorScales", baseClass, terminatorScaleStruct, 97000, L"CellCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (terminatorScaleProperty);
    //terminatorScaleProperty->MinimumValue = System::Double (s_minScaleValue);
    //ECUI::ECPropertyPane::SetExtendedType (terminatorScaleArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (terminatorScaleArrayProperty, s_TerminatorScaleStructExtendedType);

    // --- ActivePoints  type/value/criteria---
    ECClassP              activePointStruct        = GenerateStruct (L"ActivePoint", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  pointTypeProperty        = CreatePrimitiveProperty(L"Type", PRIMITIVETYPE_Integer, activePointStruct, 99000);  //0 = zero length line, 1 = symbol, 2 = cell
    PrimitiveECPropertyP  pointValueProperty       = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_String, activePointStruct, 98000);
    ArrayECPropertyP      activePointArrayProperty = CreateStructArrayProperty(L"ActivePoints", baseClass, activePointStruct, 99000, L"ActivePointCategory");
    //ECUI::ECPropertyPane::SetExtendedType (activePointArrayProperty, s_ShowActivePointArrayAsStringET);
    //ECI::IECInstance^ s_ActivePointExtendedValue = ECUI::ECPropertyPane::CreateExtendedType(L"ElementParams_ActivePointValue");  // --- used to display value in Reports ---
    //ECUI::ECPropertyPane::SetExtendedType (pointValueProperty, s_ActivePointExtendedValue);
    //ECUI::ECPropertyPane::SetMemberExtendedType (activePointArrayProperty, s_ActivePointStructET); // --- used to display and edit values in property pane ---

    // =================== Text Settings ===============
    // --- TextStyles ---
    ECClassP              textStyleStruct        = GenerateStruct (L"TextStyle", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  textStyleProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_String, textStyleStruct, 99000); 
    ArrayECPropertyP      textStyleArrayProperty = CreateStructArrayProperty(L"TextStyles", baseClass, textStyleStruct, 100000, L"TextCategory");
    //ECUI::ECPropertyPane::SetExtendedType (textStyleProperty, s_TextStyleET); // --- used to display value in Reports ---
    //ECUI::ECPropertyPane::SetExtendedType (textStyleArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (textStyleArrayProperty, s_TextStyleStructET);

    // ----- TextStyleOverrides ----
    ECClassP             textstyleOverrideStruct        = GenerateStruct            (L"TextStyleOverride",  criteriaStruct,        *m_elementTemplateSchemaHolder);                 
    PrimitiveECPropertyP textHeightProperty             = CreatePrimitiveProperty   (L"TextHeight",         PRIMITIVETYPE_Double,  textstyleOverrideStruct, 99000); 
    PrimitiveECPropertyP textWidthProperty              = CreatePrimitiveProperty   (L"TextWidth",          PRIMITIVETYPE_Double,  textstyleOverrideStruct, 98000); 
    PrimitiveECPropertyP textFontProperty               = CreatePrimitiveProperty   (L"Font",               PRIMITIVETYPE_String,  textstyleOverrideStruct, 97000); 
    PrimitiveECPropertyP textSlantProperty              = CreatePrimitiveProperty   (L"TextSlant",          PRIMITIVETYPE_Double,  textstyleOverrideStruct, 96000); 
    PrimitiveECPropertyP textJustificationProperty      = CreatePrimitiveProperty   (L"TextJustification",  PRIMITIVETYPE_Integer, textstyleOverrideStruct, 95000); 
    PrimitiveECPropertyP nodeJustificationProperty      = CreatePrimitiveProperty   (L"NodeJustification",  PRIMITIVETYPE_Integer, textstyleOverrideStruct, 94000); 
    PrimitiveECPropertyP linelengthProperty             = CreatePrimitiveProperty   (L"LineLength",         PRIMITIVETYPE_Integer, textstyleOverrideStruct, 93000); 
    PrimitiveECPropertyP lineSpacingProperty            = CreatePrimitiveProperty   (L"LineSpacing",        PRIMITIVETYPE_Double,  textstyleOverrideStruct, 92000); 
    PrimitiveECPropertyP characterSpacingProperty       = CreatePrimitiveProperty   (L"CharacterSpacing",   PRIMITIVETYPE_Double,  textstyleOverrideStruct, 91000); 
    PrimitiveECPropertyP useFractionsProperty           = CreatePrimitiveProperty   (L"UseFractions",       PRIMITIVETYPE_Boolean, textstyleOverrideStruct, 90000); 
    PrimitiveECPropertyP useVerticalTextProperty        = CreatePrimitiveProperty   (L"UseVerticalText",    PRIMITIVETYPE_Boolean, textstyleOverrideStruct, 89000); 
    PrimitiveECPropertyP underlineTextProperty          = CreatePrimitiveProperty   (L"UnderlineText",      PRIMITIVETYPE_Boolean, textstyleOverrideStruct, 88000); 
    ArrayECPropertyP     textStyleOverrideArrayProperty = CreateStructArrayProperty (L"TextStyleOverrides", baseClass, textstyleOverrideStruct, 99000, L"TextCategory"); 
    //ECUI::ECPropertyPane::SetExtendedType (textHeightProperty,      s_NonZeroDistanceET); 
    //ECUI::ECPropertyPane::SetExtendedType (textWidthProperty, s_NonZeroDistanceET);
    //ECI::IECInstance^ s_FontNameET = ECUI::ECPropertyPane::CreateExtendedType(L"ElementParams_FontName");
    //ECUI::ECPropertyPane::SetExtendedType (fontNameProperty, s_FontNameET);
    //ECUI::ECPropertyPane::SetExtendedType (textSlantProperty, ECUI::ECPropertyPane::PropertyExtendedType::Angle);
    //ECUI::ECPropertyPane::SetExtendedType (textJustificationProperty, s_TextJustET);
    //ECUI::ECPropertyPane::SetExtendedType (nodeJustificationProperty, s_NodeJustET);
    //ECUI::ECPropertyPane::SetExtendedType (lineSpacingProperty, s_SimpleDistanceET);
    //ECUI::ECPropertyPane::SetExtendedType (characterSpacingProperty, s_SimpleDistanceET);

    // --- PatternCells ---
    ECClassP              patternCellStruct        = GenerateStruct (L"PatternCell", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  patternCellProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_String, patternCellStruct, 99000); 
    ArrayECPropertyP      patternCellArrayProperty = CreateStructArrayProperty(L"PatternCells", baseClass, patternCellStruct, 99000, L"PatternCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (patternCellProperty);
    //ECUI::ECPropertyPane::SetExtendedType (patternCellArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (patternCellArrayProperty, s_patternCellStructET);

    // ----- Cell PatternScales ----
    ECClassP              patternScaleStruct   = GenerateStruct (L"PatternScale", scaleParamsStruct, *m_elementTemplateSchemaHolder);
    ArrayECPropertyP      patternScaleProperty = CreateStructArrayProperty(L"PatternScales", baseClass, patternScaleStruct, 98000, L"PatternCategory");
    //ECUI::ECPropertyPane::SetExtendedType (patternScaleArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (patternScaleArrayProperty, s_PatternScaleStructET);

    // ----- shared PatternDeltas structure ----
    ECClassP              patternDeltaStruct        = GenerateStruct (L"PatternDelta", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  patternDeltaXProperty     = CreatePrimitiveProperty(L"X", PRIMITIVETYPE_Double, patternDeltaStruct, 99000); 
    PrimitiveECPropertyP  patternDeltaYProperty     = CreatePrimitiveProperty(L"Y", PRIMITIVETYPE_Double, patternDeltaStruct, 98000); 
    //ECUI::ECPropertyPane::SetExtendedType (patternDeltaXProperty, ECUI::ECPropertyPane::PropertyExtendedType::Distance);
    //ECUI::ECPropertyPane::SetExtendedType (patternDeltaYProperty, ECUI::ECPropertyPane::PropertyExtendedType::Distance);
    //ECUI::ECPropertyPane::SetRequiresRefresh (patternDeltaXProperty);
    //ECUI::ECPropertyPane::SetRequiresRefresh (patternDeltaYProperty);

    // ----- Cell AreaPatternDeltas ----
    ArrayECPropertyP      areaPatternDeltaArrayProperty = CreateStructArrayProperty(L"AreaPatternDeltas", baseClass, patternDeltaStruct, 97000, L"PatternCategory");
    //ECUI::ECPropertyPane::SetExtendedType (areaPatternDeltaArrayProperty, s_ShowDeltaXYArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (areaPatternDeltaArrayProperty, s_PatternDeltaStructET);

    // ----- Cell AreaPatternAngle ----
    ECClassP              areaPatAngleStruct        = GenerateStruct (L"AreaPatternAngle", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  areaPatAngleProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_Double, areaPatAngleStruct, 97000); 
    ArrayECPropertyP      areaPatAngleArrayProperty = CreateStructArrayProperty(L"AreaPatternAngles", baseClass, areaPatAngleStruct, 96000, L"PatternCategory");
    //ECUI::ECPropertyPane::SetExtendedType (areaPatAngleProperty, ECUI::ECPropertyPane::PropertyExtendedType::Angle);
    //ECUI::ECPropertyPane::SetRequiresRefresh (areaPatAngleProperty);
    //ECUI::ECPropertyPane::SetExtendedType (areaPatAngleArrayProperty, s_ShowAngleArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (areaPatAngleArrayProperty, s_AreaPatAngleStructET);

    // ----- Hatch PatternDeltas ----
    ArrayECPropertyP      patternDeltaArrayProperty = CreateStructArrayProperty(L"PatternDeltas", baseClass, patternDeltaStruct, 94000, L"PatternCategory");
    //ECUI::ECPropertyPane::SetExtendedType (patternDeltaArrayProperty, s_ShowDeltaXYArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (patternDeltaArrayProperty, s_PatternDeltaStructET);

    // ----- Hatch PatternAngles ----
    ECClassP              patternAngleStruct        = GenerateStruct (L"PatternAngle", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  patternAngle1Property     = CreatePrimitiveProperty(L"Angle1", PRIMITIVETYPE_Double, patternAngleStruct, 99000); 
    PrimitiveECPropertyP  patternAngle2Property     = CreatePrimitiveProperty(L"Angle2", PRIMITIVETYPE_Double, patternAngleStruct, 98000); 
    ArrayECPropertyP      patternAngleArrayProperty = CreateStructArrayProperty(L"PatternAngles", baseClass, patternAngleStruct, 96000, L"PatternCategory");
    //ECUI::ECPropertyPane::SetMemberExtendedType (patternAngle1Property, ECUI::ECPropertyPane::PropertyExtendedType::Angle);
    //ECUI::ECPropertyPane::SetMemberExtendedType (patternAngle2Property, ECUI::ECPropertyPane::PropertyExtendedType::Angle);
    //ECUI::ECPropertyPane::SetRequiresRefresh (patternAngle1Property);
    //ECUI::ECPropertyPane::SetRequiresRefresh (patternAngle2Property);
    //ECUI::ECPropertyPane::SetExtendedType (patternAngleArrayProperty, s_ShowPatternAngleArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (patternAngleArrayProperty, s_PatternAngleStructET);

    // =================== Multi-line Settings ===============
    // ----- Multi-line Style ----
    ECClassP              mlineStyleStruct        = GenerateStruct (L"MlineStyle", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  mlineStyleProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_String, mlineStyleStruct, 99000); 
    ArrayECPropertyP      mlineStyleArrayProperty = CreateStructArrayProperty(L"MlineStyles", baseClass, mlineStyleStruct, 100000, L"MultilineCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (mlineStyleProperty);
    //ECUI::ECPropertyPane::SetExtendedType (mlineStyleProperty, s_MlineStyleET); // --- used to display value in Reports ---
    //ECUI::ECPropertyPane::SetExtendedType (mlineStyleArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (mlineStyleArrayProperty, s_MlineStyleStructET);

    // =================== Dimension Settings ===============
    // ----- DimensionStyle ----
    ECClassP              dimStyleStruct        = GenerateStruct (L"DimensionStyle", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  dimStyleProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_String, dimStyleStruct, 99000); 
    ArrayECPropertyP      dimStyleArrayProperty = CreateStructArrayProperty(L"DimensionStyles", baseClass, dimStyleStruct, 100000, L"DimensionCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (dimStyleProperty);
    //ECUI::ECPropertyPane::SetExtendedType (dimStyleProperty, s_DimStyleET); // --- used to display value in Reports ---
    //ECUI::ECPropertyPane::SetExtendedType (dimStyleArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (dimStyleArrayProperty, s_DimStyleStructET);

    // =================== Material Settings ===============
    // ----- Material ----
    ECClassP              materialStruct        = GenerateStruct (L"Material", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  materialProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_String, materialStruct, 99000); 
    ArrayECPropertyP      materialArrayProperty = CreateStructArrayProperty(L"Materials", baseClass, materialStruct, 100000, L"MaterialCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (materialProperty);
    //ECUI::ECPropertyPane::SetExtendedType (materialArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (materialArrayProperty, s_MaterialStructET);

#ifdef WIP_NO_FEATUREASPECTS_IN_DGNPLATFORM
    // @fadoc Hide/show Material Array property    
    if (!FeatureAspects::IsAllowed(AspectID::Standards_Templates_Materials))
        ECUI::ECPropertyPane::HideProperty(materialArrayProperty);
#endif

    // =================== Detailing Symbols Settings ===============
    // ----- Detailing Symbol - Title Text ----
    ECClassP              dsttStruct        = GenerateStruct (L"TitleTextStyle", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  dsttProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_String, dsttStruct, 99000); 
    ArrayECPropertyP      dsttArrayProperty = CreateStructArrayProperty(L"TitleTextStyles", baseClass, dsttStruct, 100000, L"DetailingSymbolsCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (dsProperty);
    //ECUI::ECPropertyPane::SetExtendedType (dsProperty, s_dsTitleTextET); // --- used to display value in Reports ---
    //ECUI::ECPropertyPane::SetExtendedType (dsStructArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (dsStructArrayProperty, s_dsTitleTextStructET);

    // ----- Detailing Symbol - Drawing View Marker ----
    ECClassP              dsdtStruct        = GenerateStruct (L"DrawingTitleStyle", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  dsdtProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_String, dsdtStruct, 99000); 
    ArrayECPropertyP      dsdtArrayProperty = CreateStructArrayProperty(L"DrawingTitleStyles", baseClass, dsdtStruct, 99000, L"DetailingSymbolsCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (dsProperty);
    //ECUI::ECPropertyPane::SetExtendedType (dsProperty, s_dsDrawingViewMarkerET);  // --- used to display value in Reports ---
    //ECUI::ECPropertyPane::SetExtendedType (dsStructArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (dsStructArrayProperty, s_dsDrawingViewMarkerStructET);

    // ----- Detailing Symbol - Detail Marker ----
    ECClassP              dscoStruct        = GenerateStruct (L"DetailCalloutStyle", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  dscoProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_String, dscoStruct, 99000); 
    ArrayECPropertyP      dscoArrayProperty = CreateStructArrayProperty(L"DetailCalloutStyles", baseClass, dscoStruct, 98000, L"DetailingSymbolsCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (dsProperty);
    //ECUI::ECPropertyPane::SetExtendedType (dsProperty, s_dsDetailCalloutET);  // --- used to display value in Reports ---
    //ECUI::ECPropertyPane::SetExtendedType (dsStructArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (dsStructArrayProperty, s_dsDetailCalloutStructET);

    // ----- Detailing Symbol - Elevation Marker ----
    ECClassP              dsecoStruct        = GenerateStruct (L"ElevationCalloutStyle", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  dsecoProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_String, dsecoStruct, 99000); 
    ArrayECPropertyP      dsecoArrayProperty = CreateStructArrayProperty(L"ElevationCalloutStyles", baseClass, dsecoStruct, 97000, L"DetailingSymbolsCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (dsProperty);
    //ECUI::ECPropertyPane::SetExtendedType (dsProperty, s_dsDetailCalloutET);  // --- used to display value in Reports ---
    //ECUI::ECPropertyPane::SetExtendedType (dsStructArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (dsStructArrayProperty, s_dsDetailCalloutStructET);

    // ----- Detailing Symbol - Section Marker ----
    ECClassP              dsscoStruct        = GenerateStruct (L"SectionCalloutStyle", criteriaStruct, *m_elementTemplateSchemaHolder);
    PrimitiveECPropertyP  dsscoProperty      = CreatePrimitiveProperty(L"Value", PRIMITIVETYPE_String, dsscoStruct, 99000); 
    ArrayECPropertyP      dsscoArrayProperty = CreateStructArrayProperty(L"SectionCalloutStyles", baseClass, dsscoStruct, 97000, L"DetailingSymbolsCategory");
    //ECUI::ECPropertyPane::SetRequiresRefresh (dsProperty);
    //ECUI::ECPropertyPane::SetExtendedType (dsProperty, s_dsDetailCalloutET);  // --- used to display value in Reports ---
    //ECUI::ECPropertyPane::SetExtendedType (dsStructArrayProperty, s_ShowArrayAsStringET);
    //ECUI::ECPropertyPane::SetMemberExtendedType (dsStructArrayProperty, s_dsDetailCalloutStructET);

//    WString ecSchemaXml;
//    schema->WriteXmlToString (ecSchemaXml);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void   ECXDataTreeSchemasLocater::CreateCustomInterfaceSchema ()
    {
    DgnECManager::GetManager().CreateSchema (m_customInterfaceSchemaHolder, L"Ustn_CustomTool", 1, 0, NULL);
    //WIP: Abeesh TODO register this schema in SharedSchemaCache?
    m_customInterfaceSchemaHolder->SetNamespacePrefix (L"tbui");

    ECClassP modificationDataStruct = GenerateStruct (L"ModificationDataStruct", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP menuPathProperty         = CreatePrimitiveProperty (L"MenuPath",         PRIMITIVETYPE_String,  modificationDataStruct, 99900, L""); 
        PrimitiveECPropertyP relativeItemProperty     = CreatePrimitiveProperty (L"RelativeItem",     PRIMITIVETYPE_String,  modificationDataStruct, 99800, L""); 
        PrimitiveECPropertyP menuIdProperty           = CreatePrimitiveProperty (L"MenuId",           PRIMITIVETYPE_Integer, modificationDataStruct, 99700, L""); 
        PrimitiveECPropertyP itemSearchIdProperty     = CreatePrimitiveProperty (L"ItemSearchId",     PRIMITIVETYPE_Integer, modificationDataStruct, 99600, L""); 
        PrimitiveECPropertyP relativeLocationProperty = CreatePrimitiveProperty (L"RelativeLocation", PRIMITIVETYPE_Integer, modificationDataStruct, 99500, L""); 
        }                    

    ECClassP bitMapIconStruct = GenerateStruct (L"BitMapIcon", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP bitMapTypeProperty = CreatePrimitiveProperty (L"BitMapType", PRIMITIVETYPE_Integer, bitMapIconStruct, 99900, L""); 
        PrimitiveECPropertyP bitMapNameProperty = CreatePrimitiveProperty (L"BitMapName", PRIMITIVETYPE_String,  bitMapIconStruct, 99800, L""); 
        }

   ECClassP rscIconStruct = GenerateStruct (L"RscIcon", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP rscTypeProperty   = CreatePrimitiveProperty (L"RscType",   PRIMITIVETYPE_Integer, rscIconStruct, 99900, L""); 
        PrimitiveECPropertyP ownerProperty     = CreatePrimitiveProperty (L"Owner",     PRIMITIVETYPE_String,  rscIconStruct, 99800, L""); 
        PrimitiveECPropertyP iconOwnerProperty = CreatePrimitiveProperty (L"IconOwner", PRIMITIVETYPE_String,  rscIconStruct, 99700, L""); 
        PrimitiveECPropertyP rscIdProperty     = CreatePrimitiveProperty (L"RscId",     PRIMITIVETYPE_Integer, rscIconStruct, 99600, L""); 
        PrimitiveECPropertyP itemArgProperty   = CreatePrimitiveProperty (L"ItemArg",   PRIMITIVETYPE_Integer, rscIconStruct, 99500, L""); 
        PrimitiveECPropertyP iconTypeProperty  = CreatePrimitiveProperty (L"IconType",  PRIMITIVETYPE_Integer, rscIconStruct, 99400, L""); 
        PrimitiveECPropertyP iconIdProperty    = CreatePrimitiveProperty (L"IconID",    PRIMITIVETYPE_Integer, rscIconStruct, 99300, L""); 
        PrimitiveECPropertyP cmdNumProperty    = CreatePrimitiveProperty (L"CmdNum",    PRIMITIVETYPE_Integer, rscIconStruct, 99200, L""); 
        }                    

    ECClassP iconStruct = GenerateStruct (L"Icon", NULL, *m_customInterfaceSchemaHolder); 
        {
        PrimitiveECPropertyP namedToolTypeProperty = CreatePrimitiveProperty (L"NamedToolType", PRIMITIVETYPE_String, iconStruct, 99900, L""); 
        StructECPropertyP rscIconProperty          = CreateStructProperty (L"RscIcon",          rscIconStruct,        iconStruct, 99800, L""); 
        StructECPropertyP bitMapIconProperty       = CreateStructProperty (L"BitMapIcon",       bitMapIconStruct,     iconStruct, 99700, L""); 
        }

    ECClassP processingDataStruct = GenerateStruct (L"ProcessingData", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP cmdStringProperty         = CreatePrimitiveProperty (L"CmdString",         PRIMITIVETYPE_String,   processingDataStruct, 99900, L""); 
        PrimitiveECPropertyP balloonTextProperty       = CreatePrimitiveProperty (L"BalloonText",       PRIMITIVETYPE_String,   processingDataStruct, 99800, L""); 
        PrimitiveECPropertyP helpUrlProperty           = CreatePrimitiveProperty (L"HelpUrl",           PRIMITIVETYPE_String,   processingDataStruct, 99700, L""); 
        PrimitiveECPropertyP associateTemplateProperty = CreatePrimitiveProperty (L"AssociateTemplate", PRIMITIVETYPE_Integer,  processingDataStruct, 99600, L""); 
        PrimitiveECPropertyP templatePathProperty      = CreatePrimitiveProperty (L"TemplatePath",      PRIMITIVETYPE_String,   processingDataStruct, 99500, L""); 
        PrimitiveECPropertyP defaultTemplateProperty   = CreatePrimitiveProperty (L"DefaultTemplate",   PRIMITIVETYPE_String,   processingDataStruct, 99400, L""); 
        }                    
                                                                                                                                           
   ECClassP cmdMenuItemStruct = GenerateStruct (L"CmdMenuItemStruct", NULL, *m_customInterfaceSchemaHolder);
        {
        StructECPropertyP cmdMenuItemProperty    = CreateStructProperty (L"Icon",           iconStruct,           cmdMenuItemStruct, 99900, L""); 
        StructECPropertyP processingDataProperty = CreateStructProperty (L"ProcessingData", processingDataStruct,   cmdMenuItemStruct, 99800, L""); 
        }

   ECClassP toolMenuItemStruct = GenerateStruct (L"ToolMenuItemStruct", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP toolPathProperty = CreatePrimitiveProperty (L"ToolPath", PRIMITIVETYPE_String, toolMenuItemStruct, 99900, L""); 
        }

   ECClassP locksStruct = GenerateStruct (L"LocksStruct", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP associationProperty        = CreatePrimitiveProperty (L"Association",        PRIMITIVETYPE_Boolean, locksStruct, 99900, L""); 
        PrimitiveECPropertyP snapProperty               = CreatePrimitiveProperty (L"Snap",               PRIMITIVETYPE_Boolean, locksStruct, 99800, L""); 
        PrimitiveECPropertyP gripProperty               = CreatePrimitiveProperty (L"Grip",               PRIMITIVETYPE_Boolean, locksStruct, 99700, L""); 
        PrimitiveECPropertyP unitProperty               = CreatePrimitiveProperty (L"Unit",               PRIMITIVETYPE_Boolean, locksStruct, 99600, L""); 
        PrimitiveECPropertyP boreSiteProperty           = CreatePrimitiveProperty (L"BoreSite",           PRIMITIVETYPE_Boolean, locksStruct, 99500, L""); 
        PrimitiveECPropertyP angleProperty              = CreatePrimitiveProperty (L"Angle",              PRIMITIVETYPE_Boolean, locksStruct, 99400, L""); 
        PrimitiveECPropertyP textNodeProperty           = CreatePrimitiveProperty (L"TextNode",           PRIMITIVETYPE_Boolean, locksStruct, 99300, L""); 
        PrimitiveECPropertyP axisProperty               = CreatePrimitiveProperty (L"Axis",               PRIMITIVETYPE_Boolean, locksStruct, 99200, L""); 
        PrimitiveECPropertyP scaleProperty              = CreatePrimitiveProperty (L"Scale",              PRIMITIVETYPE_Boolean, locksStruct, 99100, L""); 
        PrimitiveECPropertyP graphicGroupProperty       = CreatePrimitiveProperty (L"GraphicGroup",       PRIMITIVETYPE_Boolean, locksStruct, 98900, L""); 
        PrimitiveECPropertyP levelProperty              = CreatePrimitiveProperty (L"Level",              PRIMITIVETYPE_Boolean, locksStruct, 98800, L""); 
        PrimitiveECPropertyP fenceOverlapProperty       = CreatePrimitiveProperty (L"FenceOverlap",       PRIMITIVETYPE_Boolean, locksStruct, 98700, L""); 
        PrimitiveECPropertyP fenceClipProperty          = CreatePrimitiveProperty (L"FenceClip",          PRIMITIVETYPE_Boolean, locksStruct, 98600, L""); 
        PrimitiveECPropertyP fenceVoidProperty          = CreatePrimitiveProperty (L"FenceVoid",          PRIMITIVETYPE_Boolean, locksStruct, 98500, L""); 
        PrimitiveECPropertyP cellStretchProperty        = CreatePrimitiveProperty (L"CellStretch",        PRIMITIVETYPE_Boolean, locksStruct, 98400, L""); 
        PrimitiveECPropertyP selectionProperty          = CreatePrimitiveProperty (L"Selection",          PRIMITIVETYPE_Boolean, locksStruct, 98300, L""); 
        PrimitiveECPropertyP constructionProperty       = CreatePrimitiveProperty (L"Construction",       PRIMITIVETYPE_Boolean, locksStruct, 98200, L""); 
        PrimitiveECPropertyP isometricProperty          = CreatePrimitiveProperty (L"Isometric",          PRIMITIVETYPE_Boolean, locksStruct, 98100, L""); 
        PrimitiveECPropertyP depthProperty              = CreatePrimitiveProperty (L"Depth",              PRIMITIVETYPE_Boolean, locksStruct, 98000, L""); 
        PrimitiveECPropertyP useAnnotationScaleProperty = CreatePrimitiveProperty (L"UseAnnotationScale", PRIMITIVETYPE_Boolean, locksStruct, 97900, L""); 
        PrimitiveECPropertyP sharedCellsProperty        = CreatePrimitiveProperty (L"SharedCells",        PRIMITIVETYPE_Boolean, locksStruct, 97800, L""); 
        PrimitiveECPropertyP trueScaleCellsProperty     = CreatePrimitiveProperty (L"TrueScaleCells",     PRIMITIVETYPE_Boolean, locksStruct, 97700, L""); 
        }                                                                                                                                     

   ECClassP settingStruct = GenerateStruct (L"SettingsStruct", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP keyPointProperty        = CreatePrimitiveProperty (L"KeyPoint",                PRIMITIVETYPE_Integer, settingStruct, 99900, L""); 
        PrimitiveECPropertyP snapModeProperty        = CreatePrimitiveProperty (L"SnapMode",                PRIMITIVETYPE_Integer, settingStruct, 99800, L""); 
        PrimitiveECPropertyP snapOverrideProperty    = CreatePrimitiveProperty (L"SnapOverride",            PRIMITIVETYPE_Integer, settingStruct, 99700, L""); 
        PrimitiveECPropertyP angleProperty           = CreatePrimitiveProperty (L"Angle",                   PRIMITIVETYPE_Double,  settingStruct, 99600, L""); 
        PrimitiveECPropertyP gridUnitProperty        = CreatePrimitiveProperty (L"GridUnit",                PRIMITIVETYPE_Double,  settingStruct, 99500, L""); 
        PrimitiveECPropertyP gridRefProperty         = CreatePrimitiveProperty (L"GridRef",                 PRIMITIVETYPE_Integer, settingStruct, 99400, L""); 
        PrimitiveECPropertyP unitRoundOffProperty    = CreatePrimitiveProperty (L"UnitRoundOff",            PRIMITIVETYPE_Double,  settingStruct, 99300, L""); 
        PrimitiveECPropertyP tagIncrementProperty    = CreatePrimitiveProperty (L"TagIncrement",            PRIMITIVETYPE_Integer, settingStruct, 99200, L""); 
        PrimitiveECPropertyP tabPropertyProperty     = CreatePrimitiveProperty (L"Tab",                     PRIMITIVETYPE_Integer, settingStruct, 99100, L""); 
        PrimitiveECPropertyP streamDeltaProperty     = CreatePrimitiveProperty (L"StreamDelta",             PRIMITIVETYPE_Double,  settingStruct, 98900, L""); 
        PrimitiveECPropertyP streamToleranceProperty = CreatePrimitiveProperty (L"StreamTolerance",         PRIMITIVETYPE_Double,  settingStruct, 98800, L""); 
        PrimitiveECPropertyP streamAngleProperty     = CreatePrimitiveProperty (L"StreamAngle",             PRIMITIVETYPE_Double,  settingStruct, 98700, L""); 
        PrimitiveECPropertyP streamAreaProperty      = CreatePrimitiveProperty (L"StreamArea",              PRIMITIVETYPE_Double,  settingStruct, 98600, L""); 
        PrimitiveECPropertyP axisAngleProperty       = CreatePrimitiveProperty (L"AxisAngle",               PRIMITIVETYPE_Double,  settingStruct, 98500, L""); 
        PrimitiveECPropertyP gridOrientationProperty = CreatePrimitiveProperty (L"GridOrientation",         PRIMITIVETYPE_Integer, settingStruct, 98400, L""); 
        PrimitiveECPropertyP axisOriginProperty      = CreatePrimitiveProperty (L"AxisOrigin",              PRIMITIVETYPE_Double,  settingStruct, 98300, L""); 
        PrimitiveECPropertyP capModeProperty         = CreatePrimitiveProperty (L"CapMode",                 PRIMITIVETYPE_Integer, settingStruct, 98200, L""); 
        PrimitiveECPropertyP cellLibraryProperty     = CreatePrimitiveProperty (L"CellLibrary",             PRIMITIVETYPE_String,  settingStruct, 98100, L""); 
        PrimitiveECPropertyP unitRoundRatioProperty  = CreatePrimitiveProperty (L"UnitRoundRatio",          PRIMITIVETYPE_Double,  settingStruct, 98000, L""); 
        PrimitiveECPropertyP igdsDimensionsProperty  = CreatePrimitiveProperty (L"IGDSCompatibleDimension", PRIMITIVETYPE_Boolean, settingStruct, 97900, L""); 
        PrimitiveECPropertyP igdsMlineProperty       = CreatePrimitiveProperty (L"IGDSCompatibleMline",     PRIMITIVETYPE_Boolean, settingStruct, 97800, L""); 
        PrimitiveECPropertyP isoPlaneModeProperty    = CreatePrimitiveProperty (L"IsoPlaneMode",            PRIMITIVETYPE_Integer, settingStruct, 97700, L""); 
        PrimitiveECPropertyP patternTolProperty      = CreatePrimitiveProperty (L"PatternTolerance",        PRIMITIVETYPE_Double,  settingStruct, 97600, L""); 
        }                    

   ECClassP itemSettingStruct = GenerateStruct (L"ItemSetting", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP nameProperty  = CreatePrimitiveProperty (L"Name",  PRIMITIVETYPE_String,  itemSettingStruct, 99900, L""); 
        PrimitiveECPropertyP typeProperty  = CreatePrimitiveProperty (L"Type",  PRIMITIVETYPE_Integer, itemSettingStruct, 99800, L""); 
        PrimitiveECPropertyP valueProperty = CreatePrimitiveProperty (L"Value", PRIMITIVETYPE_String,  itemSettingStruct, 99700, L""); 
        }                    

   ECClassP toolSettingStruct = GenerateStruct (L"Tool_Settings", NULL, *m_customInterfaceSchemaHolder);
        {
        StructECPropertyP    processingDataProperty    = CreateStructProperty      (L"Settings",     settingStruct,     toolSettingStruct, 99800, L""); 
        StructECPropertyP    locksProperty             = CreateStructProperty      (L"Locks",        locksStruct,       toolSettingStruct, 99800, L"");
        ArrayECPropertyP     itemSettingsArrayProperty = CreateStructArrayProperty (L"ItemSettings", toolSettingStruct, itemSettingStruct, 100000, L"");
        }

    ECClassP namedToolStruct = GenerateStruct (L"NamedTool", NULL, *m_customInterfaceSchemaHolder);
        {
        StructECPropertyP    iconProperty           = CreateStructProperty    (L"Icon",                  iconStruct,            namedToolStruct, 99500, L""); 
        StructECPropertyP    processingDataProperty = CreateStructProperty    (L"ProcessingData",        processingDataStruct,  namedToolStruct, 99800, L""); 
        StructECPropertyP    toolSettingsProperty   = CreateStructProperty    (L"Tool_Settings",         toolSettingStruct,     namedToolStruct, 99800, L""); 
        PrimitiveECPropertyP toolDisplayProperty    = CreatePrimitiveProperty (L"ToolDisplay",           PRIMITIVETYPE_Integer, namedToolStruct, 99600, L""); 
        PrimitiveECPropertyP toolTypeProperty       = CreatePrimitiveProperty (L"ToolType",              PRIMITIVETYPE_Integer, namedToolStruct, 99600, L""); 
        PrimitiveECPropertyP helpIDProperty         = CreatePrimitiveProperty (L"HelpID",                PRIMITIVETYPE_String,  namedToolStruct, 98700, L""); 
        PrimitiveECPropertyP dimensionProperty      = CreatePrimitiveProperty (L"Dimension",             PRIMITIVETYPE_Integer, namedToolStruct, 99400, L""); 
        PrimitiveECPropertyP showHideProperty       = CreatePrimitiveProperty (L"ShowHideCriteria",      PRIMITIVETYPE_String,  namedToolStruct, 99000, L""); 
        PrimitiveECPropertyP enableDisableProperty  = CreatePrimitiveProperty (L"EnableDisableCriteria", PRIMITIVETYPE_String,  namedToolStruct, 98900, L""); 
        PrimitiveECPropertyP toggleStateProperty    = CreatePrimitiveProperty (L"ToggleStateCriteria",   PRIMITIVETYPE_String,  namedToolStruct, 98900, L""); 
        PrimitiveECPropertyP productsProperty       = CreatePrimitiveProperty (L"Products",              PRIMITIVETYPE_String,  namedToolStruct, 98900, L""); 
        }                    

    ECClassP namedToolRefStruct = GenerateStruct (L"NamedToolRef", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP toolPathProperty = CreatePrimitiveProperty (L"ToolPath", PRIMITIVETYPE_String, namedToolRefStruct, 99900, L""); 
        }
     
    ECClassP taskRefStruct = GenerateStruct (L"TaskRef", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP taskPathProperty = CreatePrimitiveProperty (L"TaskPath", PRIMITIVETYPE_String, taskRefStruct, 99900, L""); 
        }

    ECClassP mainTaskRefStruct = GenerateStruct (L"MainTaskRef", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP mainTaskPathProperty = CreatePrimitiveProperty (L"MainTaskPath", PRIMITIVETYPE_String, mainTaskRefStruct, 99900, L""); 
        }

    ECClassP applicationToolRefStruct = GenerateStruct (L"ApplicationToolRef", NULL, *m_customInterfaceSchemaHolder);
        {
        StructECPropertyP    rscIconProperty  = CreateStructProperty    (L"RscIcon",               rscIconStruct,        applicationToolRefStruct, 99800, L""); 
        PrimitiveECPropertyP showHideProperty = CreatePrimitiveProperty (L"ShowHideCriteria",      PRIMITIVETYPE_String, applicationToolRefStruct, 99700, L""); 
        }

    ECClassP toolContainerStruct = GenerateStruct (L"ToolContainer", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP taskTypeProperty       = CreatePrimitiveProperty (L"TaskType",              PRIMITIVETYPE_Integer, toolContainerStruct, 99900, L""); 
        PrimitiveECPropertyP taskLayoutModeProperty = CreatePrimitiveProperty (L"TaskLayoutMode",        PRIMITIVETYPE_Integer, toolContainerStruct, 99800, L""); 
        PrimitiveECPropertyP cmdStringProperty      = CreatePrimitiveProperty (L"CmdString",             PRIMITIVETYPE_String,  toolContainerStruct, 99700, L""); 
        PrimitiveECPropertyP menuMnemonicProperty   = CreatePrimitiveProperty (L"MenuMnemonic",          PRIMITIVETYPE_String,  toolContainerStruct, 99600, L""); 
        StructECPropertyP    iconProperty           = CreateStructProperty    (L"Icon",                  iconStruct,            toolContainerStruct, 99500, L""); 
        PrimitiveECPropertyP dimensionProperty      = CreatePrimitiveProperty (L"Dimension",             PRIMITIVETYPE_Integer, toolContainerStruct, 99400, L""); 
        PrimitiveECPropertyP showIconProperty       = CreatePrimitiveProperty (L"ShowIcon",              PRIMITIVETYPE_Boolean, toolContainerStruct, 99300, L""); 
        PrimitiveECPropertyP verboseProperty        = CreatePrimitiveProperty (L"VerboseTask",           PRIMITIVETYPE_Boolean, toolContainerStruct, 99200, L""); 
        PrimitiveECPropertyP showInToolboxList      = CreatePrimitiveProperty (L"ShowInToolBoxList",     PRIMITIVETYPE_Boolean, toolContainerStruct, 99100, L""); 
        PrimitiveECPropertyP showHideProperty       = CreatePrimitiveProperty (L"ShowHideCriteria",      PRIMITIVETYPE_String,  toolContainerStruct, 99000, L""); 
        PrimitiveECPropertyP enableDisableProperty  = CreatePrimitiveProperty (L"EnableDisableCriteria", PRIMITIVETYPE_String,  toolContainerStruct, 98900, L""); 
        PrimitiveECPropertyP mainTaskPathProperty   = CreatePrimitiveProperty (L"MainTaskPath",          PRIMITIVETYPE_String,  toolContainerStruct, 98800, L""); 
        PrimitiveECPropertyP helpIDProperty         = CreatePrimitiveProperty (L"HelpID",                PRIMITIVETYPE_String,  toolContainerStruct, 98700, L""); 
        }                    

    ECClassP contextContainerStruct = GenerateStruct (L"ContextContainer", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP showHideProperty        = CreatePrimitiveProperty (L"ShowHideCriteria",      PRIMITIVETYPE_String,   contextContainerStruct, 99900, L""); 
        PrimitiveECPropertyP contextLoadModeProperty = CreatePrimitiveProperty (L"ContextFolderLoadMode", PRIMITIVETYPE_Integer,  contextContainerStruct, 99800, L""); 
        }                    

    ECClassP toolSeparatorStruct = GenerateStruct (L"ToolSeparator", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP showHideProperty      = CreatePrimitiveProperty (L"ShowHideCriteria",      PRIMITIVETYPE_String,   toolSeparatorStruct, 99900, L""); 
        }                    

    ECClassP appMenuStruct = GenerateStruct (L"AppMenu", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP dimensionProperty     = CreatePrimitiveProperty (L"Dimension",             PRIMITIVETYPE_Integer,  appMenuStruct, 99900, L""); 
        PrimitiveECPropertyP showHideProperty      = CreatePrimitiveProperty (L"ShowHideCriteria",      PRIMITIVETYPE_String,   appMenuStruct, 99800, L""); 
        PrimitiveECPropertyP enableDisableProperty = CreatePrimitiveProperty (L"EnableDisableCriteria", PRIMITIVETYPE_String,   appMenuStruct, 99700, L""); 
        StructECPropertyP    modDataProperty       = CreateStructProperty    (L"ModificationData",      modificationDataStruct, appMenuStruct, 99600, L""); 
        PrimitiveECPropertyP menuIdProperty        = CreatePrimitiveProperty (L"MenuId",                PRIMITIVETYPE_Integer,  appMenuStruct, 99500, L""); 
        PrimitiveECPropertyP attributesProperty    = CreatePrimitiveProperty (L"Attributes",            PRIMITIVETYPE_Integer,  appMenuStruct, 99400, L""); 
        PrimitiveECPropertyP helpInfoProperty      = CreatePrimitiveProperty (L"HelpInfo",              PRIMITIVETYPE_Integer,  appMenuStruct, 99300, L""); 
        PrimitiveECPropertyP helpTaskIdProperty    = CreatePrimitiveProperty (L"HelpTaskId",            PRIMITIVETYPE_String,   appMenuStruct, 99200, L""); 
        PrimitiveECPropertyP hookIdProperty        = CreatePrimitiveProperty (L"HookId",                PRIMITIVETYPE_Integer,  appMenuStruct, 99100, L""); 
        PrimitiveECPropertyP searchIdProperty      = CreatePrimitiveProperty (L"SearchId",              PRIMITIVETYPE_Integer,  appMenuStruct, 99000, L""); 
        PrimitiveECPropertyP hasIconProperty       = CreatePrimitiveProperty (L"HasIcon",               PRIMITIVETYPE_Integer,  appMenuStruct, 98900, L""); 
        PrimitiveECPropertyP iconIDProperty        = CreatePrimitiveProperty (L"IconID",                PRIMITIVETYPE_Integer,  appMenuStruct, 98800, L""); 
        PrimitiveECPropertyP iconTypeProperty      = CreatePrimitiveProperty (L"IconType",              PRIMITIVETYPE_Integer,  appMenuStruct, 98700, L""); 
        PrimitiveECPropertyP iconTaskIdProperty    = CreatePrimitiveProperty (L"IconTaskId",            PRIMITIVETYPE_String,   appMenuStruct, 98600, L""); 
        PrimitiveECPropertyP iconNameProperty      = CreatePrimitiveProperty (L"IconName",              PRIMITIVETYPE_String,   appMenuStruct, 98500, L""); 
        }                    

    ECClassP appMenuItemStruct = GenerateStruct (L"AppMenuItem", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP dimensionProperty     = CreatePrimitiveProperty (L"Dimension",             PRIMITIVETYPE_Integer,  appMenuItemStruct, 99900, L""); 
        PrimitiveECPropertyP showHideProperty      = CreatePrimitiveProperty (L"ShowHideCriteria",      PRIMITIVETYPE_String,   appMenuItemStruct, 99800, L""); 
        PrimitiveECPropertyP enableDisableProperty = CreatePrimitiveProperty (L"EnableDisableCriteria", PRIMITIVETYPE_String,   appMenuItemStruct, 99700, L""); 
        StructECPropertyP    modDataProperty       = CreateStructProperty    (L"ModificationData",      modificationDataStruct, appMenuItemStruct, 99600, L""); 
        PrimitiveECPropertyP helpInfoProperty      = CreatePrimitiveProperty (L"HelpInfo",              PRIMITIVETYPE_Integer,  appMenuItemStruct, 99500, L""); 
        PrimitiveECPropertyP helpTaskIdProperty    = CreatePrimitiveProperty (L"HelpTaskId",            PRIMITIVETYPE_String,   appMenuItemStruct, 99400, L""); 
        PrimitiveECPropertyP hookIdProperty        = CreatePrimitiveProperty (L"HookId",                PRIMITIVETYPE_Integer,  appMenuItemStruct, 99300, L""); 
        PrimitiveECPropertyP acceleratorProperty   = CreatePrimitiveProperty (L"Accelerator",           PRIMITIVETYPE_Integer,  appMenuItemStruct, 99200, L""); 
        PrimitiveECPropertyP enabledProperty       = CreatePrimitiveProperty (L"Enabled",               PRIMITIVETYPE_Integer,  appMenuItemStruct, 99100, L""); 
        PrimitiveECPropertyP markProperty          = CreatePrimitiveProperty (L"Mark",                  PRIMITIVETYPE_Integer,  appMenuItemStruct, 99000, L""); 
        PrimitiveECPropertyP subMenuTypeProperty   = CreatePrimitiveProperty (L"SubMenuType",           PRIMITIVETYPE_Integer,  appMenuItemStruct, 98900, L""); 
        PrimitiveECPropertyP subMenuIdProperty     = CreatePrimitiveProperty (L"SubMenuId",             PRIMITIVETYPE_Integer,  appMenuItemStruct, 98800, L""); 
        PrimitiveECPropertyP searchIdProperty      = CreatePrimitiveProperty (L"SearchId",              PRIMITIVETYPE_Integer,  appMenuItemStruct, 98700, L""); 
        PrimitiveECPropertyP commandNumberProperty = CreatePrimitiveProperty (L"CommandNumber",         PRIMITIVETYPE_Integer,  appMenuItemStruct, 98600, L""); 
        PrimitiveECPropertyP hasIconProperty       = CreatePrimitiveProperty (L"HasIcon",               PRIMITIVETYPE_Integer,  appMenuItemStruct, 98500, L""); 
        PrimitiveECPropertyP iconIDProperty        = CreatePrimitiveProperty (L"IconID",                PRIMITIVETYPE_Integer,  appMenuItemStruct, 98400, L""); 
        PrimitiveECPropertyP iconTypeProperty      = CreatePrimitiveProperty (L"IconType",              PRIMITIVETYPE_Integer,  appMenuItemStruct, 98300, L""); 
        PrimitiveECPropertyP iconTaskIdProperty    = CreatePrimitiveProperty (L"IconTaskId",            PRIMITIVETYPE_String,   appMenuItemStruct, 98200, L""); 
        PrimitiveECPropertyP iconNameProperty      = CreatePrimitiveProperty (L"IconName",              PRIMITIVETYPE_String,   appMenuItemStruct, 98100, L""); 
        PrimitiveECPropertyP commandTaskIdProperty = CreatePrimitiveProperty (L"CommandTaskId",         PRIMITIVETYPE_String,   appMenuItemStruct, 98000, L""); 
        PrimitiveECPropertyP unparsedProperty      = CreatePrimitiveProperty (L"Unparsed",              PRIMITIVETYPE_String,   appMenuItemStruct, 97900, L""); 
        }    

     ECClassP userMenuStruct = GenerateStruct (L"UserMenu", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP dimensionProperty      = CreatePrimitiveProperty (L"Dimension",             PRIMITIVETYPE_Integer,  userMenuStruct, 99900, L""); 
        PrimitiveECPropertyP showHideProperty       = CreatePrimitiveProperty (L"ShowHideCriteria",      PRIMITIVETYPE_String,   userMenuStruct, 99800, L""); 
        PrimitiveECPropertyP enableDisableProperty  = CreatePrimitiveProperty (L"EnableDisableCriteria", PRIMITIVETYPE_String,   userMenuStruct, 99700, L""); 
        StructECPropertyP    modDataProperty        = CreateStructProperty    (L"ModificationData",      modificationDataStruct, userMenuStruct, 99600, L""); 
        }                    

    ECClassP userMenuItemStruct = GenerateStruct (L"UserMenuItemStruct", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP dimensionProperty      = CreatePrimitiveProperty (L"Dimension",             PRIMITIVETYPE_Integer,      userMenuItemStruct, 99900, L""); 
        PrimitiveECPropertyP showHideProperty       = CreatePrimitiveProperty (L"ShowHideCriteria",      PRIMITIVETYPE_String,       userMenuItemStruct, 99800, L""); 
        PrimitiveECPropertyP enableDisableProperty  = CreatePrimitiveProperty (L"EnableDisableCriteria", PRIMITIVETYPE_String,       userMenuItemStruct, 99700, L""); 
        PrimitiveECPropertyP markExpressionProperty = CreatePrimitiveProperty (L"ShowMarkCriteria",      PRIMITIVETYPE_String,       userMenuItemStruct, 99600, L""); 
        PrimitiveECPropertyP acceleratorProperty    = CreatePrimitiveProperty (L"Accelerator",           PRIMITIVETYPE_Integer,      userMenuItemStruct, 99500, L""); 
        PrimitiveECPropertyP typeProperty           = CreatePrimitiveProperty (L"Type",                  PRIMITIVETYPE_Integer,      userMenuItemStruct, 99400, L""); 
        StructECPropertyP    modDataProperty        = CreateStructProperty    (L"ModificationData",      modificationDataStruct,     userMenuItemStruct, 99300, L""); 
        StructECPropertyP    cmdMenuItemProperty    = CreateStructProperty    (L"CmdMenuItem",           cmdMenuItemStruct,          userMenuItemStruct, 99200, L""); 
        StructECPropertyP    toolMenuItemProperty   = CreateStructProperty    (L"ToolMenuItem",          toolMenuItemStruct,         userMenuItemStruct, 99100, L""); 
        StructECPropertyP    appToolRefProperty     = CreateStructProperty    (L"ApplicationToolRef",    applicationToolRefStruct,   userMenuItemStruct, 99000, L""); 
        }                    
    
    ECClassP userDeleteMenuEntryStruct = GenerateStruct (L"UserDeleteMenuEntryStruct", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP menuPathProperty     = CreatePrimitiveProperty (L"MenuPath",     PRIMITIVETYPE_String,  userDeleteMenuEntryStruct, 99900, L""); 
        PrimitiveECPropertyP menuIdProperty       = CreatePrimitiveProperty (L"MenuId",       PRIMITIVETYPE_Integer, userDeleteMenuEntryStruct, 99898, L""); 
        PrimitiveECPropertyP itemSearchIdProperty = CreatePrimitiveProperty (L"ItemSearchId", PRIMITIVETYPE_Integer, userDeleteMenuEntryStruct, 99800, L""); 
        }                    

    ECClassP contextMenuStruct = GenerateStruct (L"ContextMenu", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP showHideProperty      = CreatePrimitiveProperty (L"ShowHideCriteria",      PRIMITIVETYPE_String,  contextMenuStruct, 99900, L""); 
        PrimitiveECPropertyP enableDisableProperty = CreatePrimitiveProperty (L"EnableDisableCriteria", PRIMITIVETYPE_String,  contextMenuStruct, 99898, L""); 
        PrimitiveECPropertyP priorityProperty      = CreatePrimitiveProperty (L"Priority",              PRIMITIVETYPE_Integer, contextMenuStruct, 99800, L""); 
        }                    

    ECClassP contextMenuItemStruct = GenerateStruct (L"ContextMenuItemStruct", NULL, *m_customInterfaceSchemaHolder);
        {
        PrimitiveECPropertyP showHideProperty       = CreatePrimitiveProperty (L"ShowHideCriteria",      PRIMITIVETYPE_String,     contextMenuItemStruct, 99900, L""); 
        PrimitiveECPropertyP enableDisableProperty  = CreatePrimitiveProperty (L"EnableDisableCriteria", PRIMITIVETYPE_String,     contextMenuItemStruct, 99898, L""); 
        PrimitiveECPropertyP markExpressionProperty = CreatePrimitiveProperty (L"MarkExpression",        PRIMITIVETYPE_String,     contextMenuItemStruct, 99897, L""); 
        PrimitiveECPropertyP priorityProperty       = CreatePrimitiveProperty (L"Priority",              PRIMITIVETYPE_Integer,    contextMenuItemStruct, 99800, L""); 
        PrimitiveECPropertyP typeProperty           = CreatePrimitiveProperty (L"Type",                  PRIMITIVETYPE_Integer,    contextMenuItemStruct, 99000, L""); 
        StructECPropertyP    cmdMenuItemProperty    = CreateStructProperty    (L"CmdMenuItem",           cmdMenuItemStruct,        contextMenuItemStruct, 91000, L""); 
        StructECPropertyP    toolMenuItemProperty   = CreateStructProperty    (L"ToolMenuItem",          toolMenuItemStruct,       contextMenuItemStruct, 91000, L""); 
        StructECPropertyP    appToolRefProperty     = CreateStructProperty    (L"ApplicationToolRef",    applicationToolRefStruct, contextMenuItemStruct, 97000, L""); 
        }                    

    // Generate the root "Ustn_CustomTool" class
    ECClassP rootClass = GenerateStruct (L"Ustn_CustomTool", NULL, *m_customInterfaceSchemaHolder);
    CreatePrimitiveProperty (L"UIType", PRIMITIVETYPE_String, rootClass, 100000, NULL);
    CreateStructProperty (L"NamedTool",            namedToolStruct,           rootClass, 99000, L"Category"); 
    CreateStructProperty (L"NamedToolRef",         namedToolRefStruct,        rootClass, 98000, L"Category");
    CreateStructProperty (L"TaskRef",              taskRefStruct,             rootClass, 97000, L"Category"); 
    CreateStructProperty (L"MainTaskRef",          mainTaskRefStruct,         rootClass, 96000, L"Category");
    CreateStructProperty (L"ApplicationToolRef",   applicationToolRefStruct,  rootClass, 95000, L"Category"); 
    CreateStructProperty (L"ToolContainer",        toolContainerStruct,       rootClass, 94000, L"Category");
    CreateStructProperty (L"ContextContainer",     contextContainerStruct,    rootClass, 93000, L"Category"); 
    CreateStructProperty (L"ToolSeparator",        toolSeparatorStruct,       rootClass, 92000, L"Category"); 
    CreateStructProperty (L"AppMenu",              appMenuStruct,             rootClass, 91000, L"Category"); 
    CreateStructProperty (L"AppMenuItem",          appMenuItemStruct,         rootClass, 90000, L"Category"); 
    CreateStructProperty (L"UserMenu",             userMenuStruct,            rootClass, 89000, L"Category"); 
    CreateStructProperty (L"UserMenuItem",         userMenuItemStruct,        rootClass, 88000, L"Category"); 
    CreateStructProperty (L"UserDeleteMenuEntry",  userDeleteMenuEntryStruct, rootClass, 87000, L"Category"); 
    CreateStructProperty (L"ContextMenu",          contextMenuStruct,         rootClass, 86000, L"Category"); 
    CreateStructProperty (L"ContextMenuItem",      contextMenuItemStruct,     rootClass, 85000, L"Category"); 

//    WString ecSchemaXml;
//    schema->WriteToXmlString (ecSchemaXml);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void   ECXDataTreeSchemasLocater::CreateDetailingSymbolStyleSchema ()
    {
    DgnECManager::GetManager().CreateSchema (m_detailingSymbolStyleSchemaHolder, L"DetailSymbolExtender", 1, 0, NULL);
    //WIP: Abeesh TODO register this schema in SharedSchemaCache?
    m_detailingSymbolStyleSchemaHolder->SetNamespacePrefix (L"dsep");

    // create shared structs for components of detailing symbols
    ECClassP titleStruct = GenerateStruct (DETAILINGSYMBOL_PART_TITLE, NULL, *m_detailingSymbolStyleSchemaHolder);
        {
        PrimitiveECPropertyP  cellProperty   = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_CELL, PRIMITIVETYPE_String, titleStruct, 99000, NULL);
        PrimitiveECPropertyP  colorProperty  = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_COLOR, PRIMITIVETYPE_Binary, titleStruct, 98000, NULL);
        PrimitiveECPropertyP  styleProperty  = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_STYLE, PRIMITIVETYPE_String, titleStruct, 97000, NULL);
        PrimitiveECPropertyP  weightProperty = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_WEIGHT, PRIMITIVETYPE_Integer, titleStruct, 96000, NULL);
        }

    ECClassP detailAreaStruct = GenerateStruct (DETAILINGSYMBOL_PART_DETAILAREA, NULL, *m_detailingSymbolStyleSchemaHolder);
        {
        PrimitiveECPropertyP  colorProperty  = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_COLOR, PRIMITIVETYPE_Binary, detailAreaStruct, 98000, NULL);
        PrimitiveECPropertyP  styleProperty  = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_STYLE, PRIMITIVETYPE_String, detailAreaStruct, 97000, NULL);
        PrimitiveECPropertyP  weightProperty = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_WEIGHT, PRIMITIVETYPE_Integer, detailAreaStruct, 96000, NULL);
        }

    ECClassP mainWingStruct = GenerateStruct (DETAILINGSYMBOL_PART_MAINWING, NULL, *m_detailingSymbolStyleSchemaHolder);
        {
        PrimitiveECPropertyP  cellProperty   = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_CELL, PRIMITIVETYPE_String, mainWingStruct, 99000, NULL);
        PrimitiveECPropertyP  colorProperty  = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_COLOR, PRIMITIVETYPE_Binary, mainWingStruct, 98000, NULL);
        PrimitiveECPropertyP  styleProperty  = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_STYLE, PRIMITIVETYPE_String, mainWingStruct, 97000, NULL);
        PrimitiveECPropertyP  weightProperty = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_WEIGHT, PRIMITIVETYPE_Integer, mainWingStruct, 96000, NULL);
        }

    ECClassP mainTerminatorStruct = GenerateStruct (DETAILINGSYMBOL_PART_MAINTERMINATOR, NULL, *m_detailingSymbolStyleSchemaHolder);
        {
        PrimitiveECPropertyP  cellProperty   = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_CELL, PRIMITIVETYPE_String, mainTerminatorStruct, 99000, NULL);
        PrimitiveECPropertyP  colorProperty  = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_COLOR, PRIMITIVETYPE_Binary, mainTerminatorStruct, 98000, NULL);
        PrimitiveECPropertyP  styleProperty  = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_STYLE, PRIMITIVETYPE_String, mainTerminatorStruct, 97000, NULL);
        PrimitiveECPropertyP  weightProperty = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_WEIGHT, PRIMITIVETYPE_Integer, mainTerminatorStruct, 96000, NULL);
        }

    ECClassP tailWingStruct = GenerateStruct (DETAILINGSYMBOL_PART_TAILWING, NULL, *m_detailingSymbolStyleSchemaHolder);
        {
        PrimitiveECPropertyP  cellProperty   = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_CELL, PRIMITIVETYPE_String, tailWingStruct, 99000, NULL);
        PrimitiveECPropertyP  colorProperty  = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_COLOR, PRIMITIVETYPE_Binary, tailWingStruct, 98000, NULL);
        PrimitiveECPropertyP  styleProperty  = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_STYLE, PRIMITIVETYPE_String, tailWingStruct, 97000, NULL);
        PrimitiveECPropertyP  weightProperty = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_WEIGHT, PRIMITIVETYPE_Integer, tailWingStruct, 96000, NULL);
        }

    ECClassP tailTerminatorStruct = GenerateStruct (DETAILINGSYMBOL_PART_TAILTERMINATOR, NULL, *m_detailingSymbolStyleSchemaHolder);
        {
        PrimitiveECPropertyP  cellProperty   = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_CELL, PRIMITIVETYPE_String, tailTerminatorStruct, 99000, NULL);
        PrimitiveECPropertyP  colorProperty  = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_COLOR, PRIMITIVETYPE_Binary, tailTerminatorStruct, 98000, NULL);
        PrimitiveECPropertyP  styleProperty  = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_STYLE, PRIMITIVETYPE_String, tailTerminatorStruct, 97000, NULL);
        PrimitiveECPropertyP  weightProperty = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_WEIGHT, PRIMITIVETYPE_Integer, tailTerminatorStruct, 96000, NULL);
        }

    ECClassP leaderStruct = GenerateStruct (DETAILINGSYMBOL_PART_LEADER, NULL, *m_detailingSymbolStyleSchemaHolder);
        {
        PrimitiveECPropertyP  colorProperty  = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_COLOR, PRIMITIVETYPE_Binary, leaderStruct, 98000, NULL);
        PrimitiveECPropertyP  styleProperty  = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_STYLE, PRIMITIVETYPE_String, leaderStruct, 97000, NULL);
        PrimitiveECPropertyP  weightProperty = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_WEIGHT, PRIMITIVETYPE_Integer, leaderStruct, 96000, NULL);
        }

    // create struct for each symbol type     
    ECClassP titleTextStruct         = GenerateStruct (DETAILINGSYMBOL_TYPE_TITLETEXT,        NULL, *m_detailingSymbolStyleSchemaHolder);
        {
        PrimitiveECPropertyP  levelProperty          = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_LEVEL, PRIMITIVETYPE_String, titleTextStruct, 99000, L"TitleTextCategory");
        StructECPropertyP     titleProperty          = CreateStructProperty (DETAILINGSYMBOL_PART_MAINTERMINATOR, titleStruct, titleTextStruct, 98000, L"TitleTextCategory");
        }

    ECClassP drawingViewMarkerStruct = GenerateStruct (DETAILINGSYMBOL_TYPE_DRAWINGTITLE,     NULL, *m_detailingSymbolStyleSchemaHolder);
        {
        PrimitiveECPropertyP  levelProperty          = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_LEVEL, PRIMITIVETYPE_String, drawingViewMarkerStruct, 99000, L"DrawingTitleCategory");
        StructECPropertyP     mainTerminatorProperty = CreateStructProperty (DETAILINGSYMBOL_PART_MAINTERMINATOR, mainTerminatorStruct, drawingViewMarkerStruct, 98000, L"DrawingTitleCategory");
        StructECPropertyP     mainWingProperty       = CreateStructProperty (DETAILINGSYMBOL_PART_MAINWING, mainWingStruct, drawingViewMarkerStruct, 97000, L"DrawingTitleCategory");
        StructECPropertyP     leaderProperty         = CreateStructProperty (DETAILINGSYMBOL_PART_LEADER, leaderStruct, drawingViewMarkerStruct, 96000, L"DrawingTitleCategory");
        StructECPropertyP     detailAreaProperty     = CreateStructProperty (DETAILINGSYMBOL_PART_DETAILAREA, detailAreaStruct, drawingViewMarkerStruct, 95000, L"DrawingTitleCategory");
        }

    ECClassP elevationMarkerStruct   = GenerateStruct (DETAILINGSYMBOL_TYPE_ELEVATIONCALLOUT, NULL, *m_detailingSymbolStyleSchemaHolder);
        {
        PrimitiveECPropertyP  levelProperty          = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_LEVEL, PRIMITIVETYPE_String, elevationMarkerStruct, 99000, L"ElevationCalloutCategory");
        StructECPropertyP     mainTerminatorProperty = CreateStructProperty (DETAILINGSYMBOL_PART_MAINTERMINATOR, mainTerminatorStruct, elevationMarkerStruct, 98000, L"ElevationCalloutCategory");
        StructECPropertyP     mainWingProperty       = CreateStructProperty (DETAILINGSYMBOL_PART_MAINWING, mainWingStruct, elevationMarkerStruct, 97000, L"ElevationCalloutCategory");
        }

    ECClassP detailMarkerStruct      = GenerateStruct (DETAILINGSYMBOL_TYPE_DETAILCALLOUT, NULL, *m_detailingSymbolStyleSchemaHolder);
        {
        PrimitiveECPropertyP  levelProperty          = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_LEVEL, PRIMITIVETYPE_String, detailMarkerStruct, 99000, L"DetailCalloutCategory");
        StructECPropertyP     mainTerminatorProperty = CreateStructProperty (DETAILINGSYMBOL_PART_MAINTERMINATOR, mainTerminatorStruct, detailMarkerStruct, 98000, L"DetailCalloutCategory");
        StructECPropertyP     titleProperty          = CreateStructProperty (DETAILINGSYMBOL_PART_TITLE, titleStruct, detailMarkerStruct, 97000, L"DetailCalloutCategory");
        }

    ECClassP sectionMarkerStruct = GenerateStruct (DETAILINGSYMBOL_TYPE_SECTIONCALLOUT, NULL, *m_detailingSymbolStyleSchemaHolder);
        {
        PrimitiveECPropertyP  levelProperty          = CreatePrimitiveProperty (DETAILINGSYMBOL_PROPERTY_LEVEL, PRIMITIVETYPE_String, sectionMarkerStruct, 99000, L"SectionCalloutCategory");
        StructECPropertyP     mainTerminatorProperty = CreateStructProperty (DETAILINGSYMBOL_PART_MAINTERMINATOR, mainTerminatorStruct, sectionMarkerStruct, 98000, L"SectionCalloutCategory");
        StructECPropertyP     mainWingProperty       = CreateStructProperty (DETAILINGSYMBOL_PART_MAINWING, mainWingStruct, sectionMarkerStruct, 97000, L"SectionCalloutCategory");
        StructECPropertyP     tailTerminatorProperty = CreateStructProperty (DETAILINGSYMBOL_PART_TAILTERMINATOR, tailTerminatorStruct, sectionMarkerStruct, 96000, L"SectionCalloutCategory");
        StructECPropertyP     tailWingProperty       = CreateStructProperty (DETAILINGSYMBOL_PART_TAILWING, tailWingStruct, sectionMarkerStruct, 95000, L"SectionCalloutCategory");
        StructECPropertyP     leaderProperty         = CreateStructProperty (DETAILINGSYMBOL_PART_LEADER, leaderStruct, sectionMarkerStruct, 94000, L"SectionCalloutCategory");
        }

    // Generate the root "DetailSymbolExtender" class
    ECClassP rootClass = GenerateStruct (L"DetailSymbolExtender", NULL, *m_detailingSymbolStyleSchemaHolder);
    CreateStructProperty (DETAILINGSYMBOL_TYPE_SECTIONCALLOUT,   sectionMarkerStruct,     rootClass, 92000, L"SectionCalloutCategory"  ); 
    CreateStructProperty (DETAILINGSYMBOL_TYPE_ELEVATIONCALLOUT, elevationMarkerStruct,   rootClass, 91000, L"ElevationCalloutCategory"); 
    CreateStructProperty (DETAILINGSYMBOL_TYPE_DETAILCALLOUT,    detailMarkerStruct,      rootClass, 90000, L"DetailCalloutCategory"   ); 
    CreateStructProperty (DETAILINGSYMBOL_TYPE_DRAWINGTITLE,     drawingViewMarkerStruct, rootClass, 89000, L"DrawingTitleCategory"    ); 
    CreateStructProperty (DETAILINGSYMBOL_TYPE_TITLETEXT,        titleTextStruct,         rootClass, 88000, L"TitleTextCategory"       ); 

//    WString ecSchemaXml;
//    schema->WriteToXmlString (ecSchemaXml);
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE

