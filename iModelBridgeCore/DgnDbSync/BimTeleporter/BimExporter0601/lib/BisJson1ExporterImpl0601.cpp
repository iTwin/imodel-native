/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/BimExporter0601/lib/BisJson1ExporterImpl0601.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Base64Utilities.h>
#include "BisJson1ExporterImpl0601.h"
#include <limits>

DGNDB06_USING_NAMESPACE_BENTLEY
DGNDB06_USING_NAMESPACE_BENTLEY_SQLITE
DGNDB06_USING_NAMESPACE_BENTLEY_SQLITE_EC
DGNDB06_USING_NAMESPACE_BENTLEY_EC
DGNDB06_USING_NAMESPACE_BENTLEY_DGN

static Utf8CP const JSON_TYPE_KEY = "Type";
static Utf8CP const JSON_OBJECT_KEY = "Object";
static Utf8CP const JSON_ACTION_KEY = "Action";
static Utf8CP const JSON_ACTION_INSERT = "Insert";
static Utf8CP const JSON_TYPE_Font = "Font";
static Utf8CP const JSON_TYPE_FontFaceData = "FontFaceData";
static Utf8CP const JSON_TYPE_LineStyleProperty = "LineStyleProperty";
static Utf8CP const JSON_TYPE_Model = "Model";
static Utf8CP const JSON_TYPE_CategorySelector = "CategorySelector";
static Utf8CP const JSON_TYPE_ModelSelector = "ModelSelector";
static Utf8CP const JSON_TYPE_DisplayStyle = "DisplayStyle";
static Utf8CP const JSON_TYPE_DictionaryModel = "DictionaryModel";
static Utf8CP const JSON_TYPE_CodeSpec = "CodeSpec";
static Utf8CP const JSON_TYPE_Schema = "Schema";
static Utf8CP const JSON_TYPE_Element = "Element";
static Utf8CP const JSON_TYPE_GeometricElement2d = "GeometricElement2d";
static Utf8CP const JSON_TYPE_GeometricElement3d = "GeometricElement3d";
static Utf8CP const JSON_TYPE_GeometryPart = "GeometryPart";
static Utf8CP const JSON_TYPE_Subject = "Subject";
static Utf8CP const JSON_TYPE_Partition = "Partition";
static Utf8CP const JSON_TYPE_Category = "Category";
static Utf8CP const JSON_TYPE_SubCategory = "SubCategory";
static Utf8CP const JSON_TYPE_ViewDefinition3d = "ViewDefinition3d";
static Utf8CP const JSON_TYPE_ViewDefinition2d = "ViewDefinition2d";
static Utf8CP const JSON_TYPE_ElementRefersToElement = "ElementRefersToElement";
static Utf8CP const JSON_TYPE_ElementGroupsMembers = "ElementGroupsMembers";
static Utf8CP const JSON_TYPE_ElementHasLinks = "ElementHasLinks";
static Utf8CP const JSON_TYPE_AnnotationTextStyle = "AnnotationTextStyle";
static Utf8CP const JSON_TYPE_Texture = "Texture";

static Utf8CP const  BIS_ELEMENT_PROP_CodeSpec="CodeSpec";
static Utf8CP const  BIS_ELEMENT_PROP_CodeScope="CodeScope";
static Utf8CP const  BIS_ELEMENT_PROP_CodeValue = "CodeValue";
static Utf8CP const  BIS_ELEMENT_PROP_Model = "Model";
static Utf8CP const  BIS_ELEMENT_PROP_Parent = "Parent";

static Utf8CP const  BIS_MODEL_PROP_ModeledElement = "ModeledElement";

static Utf8CP const JSON_INSTANCE_ID = "id";
static Utf8CP const JSON_CLASSNAME = "className";

#define EMBEDDED_FACE_DATA_PROP_NS "dgn_Font"
#define EMBEDDED_FACE_DATA_PROP_NAME "EmbeddedFaceData"
static const PropertySpec EMBEDDED_FACE_DATA_PROPERTY_SPEC(EMBEDDED_FACE_DATA_PROP_NAME, EMBEDDED_FACE_DATA_PROP_NS);

BEGIN_BIM_EXPORTER_NAMESPACE

static const wchar_t s_spinner[] = L" /-\\|";
static const size_t s_spinnerSize = _countof(s_spinner) - 1;

//=======================================================================================
// A quick and dirty progress meter.
// @bsiclass                                                    Sam.Wilson  07/14
//=======================================================================================
struct PrintfProgressMeter : BentleyApi::Dgn::DgnProgressMeter
    {
    DEFINE_T_SUPER(DgnProgressMeter)

    protected:
        Utf8String m_stepName;
        Utf8String m_taskName;
        Utf8String m_lastMessage;
        double m_timeOfLastUpdate;
        double m_timeOfLastSpinnerUpdate;
        uint32_t m_spinCount;

        void UpdateDisplay0(Utf8StringCR msgLeft)
            {
            m_lastMessage = msgLeft;

            // Display the number of tasks remaining. Not all major tasks have a task count.
            Utf8String tbd;
            if (m_stepsRemaining || m_tasksRemaining)
                tbd = Utf8PrintfString(":%d/%d", m_stepsRemaining, m_tasksRemaining);

            // Display the spinner and the task.
            Utf8PrintfString msg("[%c] %-123.123s %-16.16s", s_spinner[m_spinCount%s_spinnerSize], msgLeft.c_str(), tbd.c_str());
            printf("%s\r", msg.c_str());
            }

        void UpdateDisplay()
            {
            auto now = BeTimeUtilities::QuerySecondsCounter();

            if ((now - m_timeOfLastUpdate) < 1.0)
                return;

            m_timeOfLastUpdate = now;

            UpdateDisplay0(FmtMessage());
            }

        bool HasDescription() const
            {
            return m_taskName.find(':') != Utf8String::npos;
            }

        Utf8String FmtMessage() const
            {
            Utf8String msg(m_stepName);
            msg.append(": ");
            msg.append(m_taskName);
            return msg;
            }

        void ForceNextUpdateToDisplay() { m_timeOfLastUpdate = m_timeOfLastSpinnerUpdate = 0; }
        virtual void _Hide() override
            {
            Utf8PrintfString msg("    %-123.123s %-16.16s", "", "");
            printf("%s\r", msg.c_str());
            }

        virtual Abort _ShowProgress() override
            {
            if (m_aborted)
                return ABORT_Yes;

            auto now = BeTimeUtilities::QuerySecondsCounter();

            if ((now - m_timeOfLastSpinnerUpdate) < 0.25) // don't do printf's more than a few times per second -- too slow and not useful
                return ABORT_No;

            m_timeOfLastSpinnerUpdate = now;

            m_spinCount++;

            bool justShowSpinner = false;

            if ((now - m_timeOfLastUpdate) < 0.5)
                justShowSpinner = true;         // don't push out full messages more than a couple times per second -- too slow and not useful
            else
                justShowSpinner = (FmtMessage() == m_lastMessage);

            if (justShowSpinner)
                {
                printf("[%c]\r", s_spinner[m_spinCount%s_spinnerSize]);
                return ABORT_No;
                }

            ForceNextUpdateToDisplay();
            UpdateDisplay();
            return ABORT_No;
            }

        virtual void _SetCurrentStepName(Utf8CP stepName) override
            {
            T_Super::_SetCurrentStepName(stepName); // decrements step count

            if (NULL == stepName)
                {
                m_stepName.clear();
                return;
                }
            if (m_stepName.Equals(stepName))
                return;

            m_stepName = stepName;
            m_taskName.clear();
            m_spinCount = 0;
            ForceNextUpdateToDisplay();
            UpdateDisplay();
            }

        virtual void _SetCurrentTaskName(Utf8CP taskName) override
            {
            T_Super::_SetCurrentTaskName(taskName); // decrements task count

            if (taskName && m_taskName == taskName)
                return;

            m_taskName = taskName ? taskName : "";
            m_spinCount = 0;
            ForceNextUpdateToDisplay();
            UpdateDisplay();
            }

    public:
        PrintfProgressMeter() : BentleyApi::Dgn::DgnProgressMeter(), m_timeOfLastUpdate(0), m_timeOfLastSpinnerUpdate(0), m_spinCount(0) {}
    };

static const size_t ID_STRINGBUFFER_LENGTH = std::numeric_limits<uint64_t>::digits + 1; //+1 for the trailing 0 character

Utf8String IdToString(int64_t id)
    {
    Utf8Char idStrBuffer[ID_STRINGBUFFER_LENGTH];
    BeStringUtilities::FormatUInt64(idStrBuffer, ID_STRINGBUFFER_LENGTH, id, (HexFormatOptions) ((int) HexFormatOptions::IncludePrefix | (int) HexFormatOptions::Uppercase));
    return Utf8String(idStrBuffer);
    }

Utf8String IdToString(uint64_t id)
    {
    Utf8Char idStrBuffer[ID_STRINGBUFFER_LENGTH];
    BeStringUtilities::FormatUInt64(idStrBuffer, ID_STRINGBUFFER_LENGTH, id, (HexFormatOptions) ((int) HexFormatOptions::IncludePrefix | (int) HexFormatOptions::Uppercase));
    return Utf8String(idStrBuffer);
    }

Utf8String IdToString(Utf8CP idString)
    {
    uint64_t id;
    BeStringUtilities::ParseUInt64(id, idString);
    return IdToString(id);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
BisJson1ExporterImpl::BisJson1ExporterImpl(wchar_t const* dbPath) : m_dbPath(dbPath)
    {
    m_meter = new PrintfProgressMeter();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
BisJson1ExporterImpl::~BisJson1ExporterImpl()
    {
    delete m_meter;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
L10N::SqlangFiles BisJson1ExporterImpl::_SupplySqlangFiles()
    {
    BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath(L"sqlang");
    sqlangFile.AppendToPath(L"DgnClientFx_en.sqlang.db3");

    return L10N::SqlangFiles(sqlangFile);
    }

void BisJson1ExporterImpl::LogMessage(TeleporterLoggingSeverity severity, Utf8CP message, ...)
    {
    va_list args;
    va_start(args, message);
    Utf8PrintfString msg(message, args);
    va_end(args);
    m_logger(severity, msg.c_str());
    }

void BisJson1ExporterImpl::LogMessage(TeleporterLoggingSeverity severity, WCharCP message, ...)
    {
    va_list args;
    va_start(args, message);
    WPrintfString msg(message, args);
    va_end(args);
    m_logger(severity, Utf8String(msg.c_str()).c_str());
    }

void BisJson1ExporterImpl::LogPerformanceMessage(StopWatch& stopWatch, Utf8CP description, ...)
    {
    stopWatch.Stop();
    va_list args;
    va_start(args, description);
    Utf8String formattedDescription;
    formattedDescription.VSprintf(description, args);
    va_end(args);

    Utf8PrintfString message("%s|%.0f millisecs", formattedDescription.c_str(), stopWatch.GetElapsedSeconds() * 1000.0);
    m_performanceLog(message.c_str());
    }

void BisJson1ExporterImpl::SendToQueue(Utf8CP json)
    {
    Utf8String jsonStr(json);
    jsonStr.ReplaceAll("dgn.", "BisCore.");
    (QueueJson)(jsonStr.c_str());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool BisJson1ExporterImpl::OpenDgnDb()
    {
    DgnPlatformLib::Initialize(*this, false);
    DbResult dbStatus;
    DgnDb::OpenParams openParams(DgnDb::OpenMode::Readonly);

    m_dgndb = DgnDb::OpenDgnDb(&dbStatus, m_dbPath, openParams);

    if (!m_dgndb.IsValid())
        {
        LogMessage(TeleporterLoggingSeverity::LOG_ERROR, L"Failed to open DgnDb '%ls'", m_dbPath.GetName());
        return false;
        }

    m_nextAvailableId = DgnElementId(*m_dgndb, "dgn_Element", "Id");
    m_nextAvailableId.UseNext(*m_dgndb);

    m_viewDefinition3dClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "ViewDefinition3d");
    m_viewDefinition2dClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "ViewDefinition2d");
    m_spatialViewDefinitionId = m_dgndb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "SpatialViewDefinition");
    m_cameraViewDefinitionClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "CameraViewDefinition");
    m_sheetViewDefinitionClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "SheetViewDefinition");
    m_drawingViewDefinitionClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "DrawingViewDefinition");
    m_subCategoryClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "SubCategory");
    m_categoryClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "Category");
    m_linkModelClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "LinkModel");
    m_annotationTextStyle = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "AnnotationTextStyle");
    m_textureClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "Texture");
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
bool BisJson1ExporterImpl::ExportDgnDb()
    {
    if (!OpenDgnDb())
        return false;

    Json::Value tableData(Json::ValueType::arrayValue);
    m_meter->AddSteps(8);

    SetStepName(BisJson1ExporterImpl::ProgressMessage::STEP_EXPORT_SCHEMAS());
    StopWatch timer(true);
    StopWatch totalTimer(true);
    BentleyStatus stat = SUCCESS;
    if (SUCCESS != (stat = ExportSchemas(tableData)))
        return false;

    LogPerformanceMessage(timer, "Export Schemas");

    timer.Start();
    if (SUCCESS != (stat = ExportFonts(tableData)))
        return false;
    LogPerformanceMessage(timer, "Export Fonts");

    timer.Start();
    if (SUCCESS != (stat = ExportAuthorities(tableData)))
        return false;
    LogPerformanceMessage(timer, "Export Authorities");

    timer.Start();
    if (SUCCESS != (stat = ExportModels(tableData)))
        return false;
    LogPerformanceMessage(timer, "Export Models");

    timer.Start();
    if (SUCCESS != (stat = ExportViews(tableData)))
        return false;
    LogPerformanceMessage(timer, "Export Views");

    timer.Start();
    if (SUCCESS != (stat = ExportCategories(tableData)))
        return false;
    LogPerformanceMessage(timer, "Export Categories");

    timer.Start();
    if (SUCCESS != (stat = ExportGeometryParts(tableData)))
        return false;
    LogPerformanceMessage(timer, "Export GeomParts");

    timer.Start();
    if (SUCCESS != (stat = ExportLineStyles(tableData)))
        return false;
    LogPerformanceMessage(timer, "Export LineStyles");

    timer.Start();
    if (SUCCESS != (stat = ExportTextures(tableData)))
        return false;
    LogPerformanceMessage(timer, "Export Textures");

    timer.Start();
    if (SUCCESS != (stat = ExportElements(tableData)))
        return false;
    ReportProgress();
    LogPerformanceMessage(timer, "Export Elements");

    timer.Start();
    if (SUCCESS != (stat = ExportNamedGroups(tableData)))
        return false;
    LogPerformanceMessage(timer, "Export Named Groups");

    timer.Start();
    if (SUCCESS != (stat = ExportElementHasLinks(tableData)))
        return false;
    LogPerformanceMessage(timer, "Export ElementHasLinks");
	
	timer.Start();
    if (SUCCESS != (stat = ExportV8Relationships(tableData)))
        return false;

    LogPerformanceMessage(timer, "Export EC Relationships");

    LogPerformanceMessage(totalTimer, "Total export time");

    m_dgndb->CloseDb();
    m_dgndb = nullptr;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportFonts(Json::Value& out)
    {
    Statement stmt;
    Utf8PrintfString sql("SELECT Id,SubId,StrData,RawSize FROM be_Prop WHERE Namespace='%s' AND Name='%s'", EMBEDDED_FACE_DATA_PROP_NS, EMBEDDED_FACE_DATA_PROP_NAME);
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql.c_str())))
        {
        LogMessage(TeleporterLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve font properties");
        return ERROR;
        }
    while (BE_SQLITE_ROW == stmt.Step())
        {
        auto& entry = out.append(Json::ValueType::objectValue);
        entry[JSON_TYPE_KEY] = JSON_TYPE_FontFaceData;
        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto& row = entry[JSON_OBJECT_KEY];
        row.clear();
        int64_t id = stmt.GetValueInt64(0);
        row[JSON_INSTANCE_ID] = IdToString(id).c_str();
        row["SubId"] = IdToString(stmt.GetValueInt64(1)).c_str();
        row["StrData"] = stmt.GetValueText(2);
        int rawSize = stmt.GetValueInt(3);
        row["RawSize"] = rawSize;

        bvector<Byte> data;
        data.resize(rawSize);
        m_dgndb->QueryProperty(&data[0], (uint32_t) data.size(), EMBEDDED_FACE_DATA_PROPERTY_SPEC, id, 0);
        Utf8String str;
        Base64Utilities::Encode(str, data.data(), rawSize);
        row["Base64EncodedData"] = str.c_str();
        row["Base64EncodedSize"] = str.SizeInBytes();
        (QueueJson) (entry.toStyledString().c_str());
        }

    Statement stmt2;
    Utf8String sql2("SELECT Id,Type,Name,Metadata FROM dgn_Font");
    if (BE_SQLITE_OK != (stmt2.Prepare(*m_dgndb, sql2.c_str())))
        {
        LogMessage(TeleporterLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve fonts");
        return ERROR;
        }

    while (BE_SQLITE_ROW == stmt2.Step())
        {
        auto& entry = out.append(Json::ValueType::objectValue);
        entry[JSON_TYPE_KEY] = JSON_TYPE_Font;
        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto& row = entry[JSON_OBJECT_KEY];
        row.clear();
        row[JSON_INSTANCE_ID] = IdToString(stmt2.GetValueInt64(0)).c_str();
        row["Type"] = stmt2.GetValueInt(1);
        row["Name"] = stmt2.GetValueText(2);

        ByteCP data = static_cast<ByteCP>(stmt2.GetValueBlob(3));
        size_t dataSize = stmt2.GetColumnBytes(3);
        Utf8String str;
        Base64Utilities::Encode(str, data, dataSize);
        row["Metadata"] = str.c_str();
        (QueueJson) (entry.toStyledString().c_str());
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportGeometryParts(Json::Value& out)
    {
    // These need to come first as the LineStyle components contain mapped ids.  And LineStyleElements using LineStyle components.
    return ExportElements(out, DGN_ECSCHEMA_NAME, "GeometryPart", m_dgndb->GetDictionaryModel().GetModelId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportTextures(Json::Value& out)
    {
    // These need to come first as the GeomParts contain mapped ids
    return ExportElements(out, DGN_ECSCHEMA_NAME, "Texture", m_dgndb->GetDictionaryModel().GetModelId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportLineStyles(Json::Value& out)
    {
    Statement stmt;
    Utf8String sql("SELECT Id, Name, StrData FROM be_Prop WHERE Namespace='dgn_LStyle'");
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql.c_str())))
        {
        LogMessage(TeleporterLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve line style properties");
        return ERROR;
        }
    while (BE_SQLITE_ROW == stmt.Step())
        {
        auto& entry = out.append(Json::ValueType::objectValue);
        entry[JSON_TYPE_KEY] = JSON_TYPE_LineStyleProperty;
        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto& row = entry[JSON_OBJECT_KEY];
        row.clear();
        int64_t id = stmt.GetValueInt64(0);
        row[JSON_INSTANCE_ID] = IdToString(id).c_str();
        row["Name"] = stmt.GetValueText(1);
        row["StrData"] = stmt.GetValueText(2);
        (QueueJson) (entry.toStyledString().c_str());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportAuthorities(Json::Value& out)
    {
    Statement stmt;
    Utf8PrintfString sql("SELECT Id, Properties FROM dgn_Authority");
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql.c_str())))
        {
        LogMessage(TeleporterLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve authorities");
        return ERROR;
        }
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnAuthorityId id = stmt.GetValueId<DgnAuthorityId>(0);
        DgnAuthorityCPtr authority = m_dgndb->Authorities().GetAuthority(id);
        
        //All of the Dgn* authorities have been changed.
        if (authority->GetName().Equals("DgnComponent") ||
            authority->GetName().Equals("DgnModels") ||
            authority->GetName().Equals("DgnCategories") ||
            authority->GetName().Equals("DgnResources") ||
            authority->GetName().Equals("DgnColors"))
            continue;

        auto& entry = out.append(Json::ValueType::objectValue);

        ECClassCP authorityClass = m_dgndb->Schemas().GetECClass(authority->GetClassId());

        entry[JSON_TYPE_KEY] = JSON_TYPE_CodeSpec;
        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto& row = entry[JSON_OBJECT_KEY];
        row.clear();
        if (authorityClass->GetName().Equals("NamespaceAuthority"))
            row["CodeSpecType"] = 1;
        else 
            row["CodeSpecType"] = 1;
        row[JSON_INSTANCE_ID] = IdToString(id.GetValue()).c_str();
        if (authority->GetName().Equals("Local"))
            row["Name"] = "bis:NullCodeSpec";
        else if (authority->GetName().Equals("DgnGeometryPart"))
            row["Name"] = "bis:GeometryPart";
        else if (authority->GetName().Equals("DgnMaterials"))
            row["Name"] = "bis:MaterialElement";
        else
            row["Name"] = authority->GetName().c_str();
        row["Properties"] = stmt.GetValueText(1);
        (QueueJson)(entry.toStyledString().c_str());
        }

    // Need to make some "fake" authorities that were added in BisCore
    CreateCodeSpec(out, 2, "bis:AnnotationFrameStyle");
    CreateCodeSpec(out, 2, "bis:AnnotationLeaderStyle");
    CreateCodeSpec(out, 2, "bis:AnnotationTextStyle");
    CreateCodeSpec(out, 2, "bis:CategorySelector");
    CreateCodeSpec(out, 2, "bis:ColorBook");
    CreateCodeSpec(out, 2, "bis:DisplayStyle");
    CreateCodeSpec(out, 2, "bis:Drawing");
    CreateCodeSpec(out, 2, "bis:DrawingCategory");
    CreateCodeSpec(out, 2, "bis:GraphicalType2d");
    CreateCodeSpec(out, 2, "bis:LineStyle");
    CreateCodeSpec(out, 2, "bis:LinkElement");
    CreateCodeSpec(out, 2, "bis:ModelSelector");
    CreateCodeSpec(out, 2, "bis:PhysicalType");
    CreateCodeSpec(out, 2, "bis:Sheet");
    CreateCodeSpec(out, 2, "bis:SpatialCategory");
    CreateCodeSpec(out, 2, "bis:SpatialLocationType");
    CreateCodeSpec(out, 2, "bis:TextAnnotationSeed");
    CreateCodeSpec(out, 2, "bis:Texture");
    CreateCodeSpec(out, 2, "bis:ViewDefinition");


    CreateCodeSpec(out, 3, "bis:InformationPartitionElement");
    CreateCodeSpec(out, 3, "bis:Subject");
    CreateCodeSpec(out, 3, "bis:SubCategory");

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId BisJson1ExporterImpl::CreateCodeSpec(Json::Value& out, uint8_t codeSpecType, Utf8CP name)
    {
    auto& entry = out.append(Json::ValueType::objectValue);
    entry[JSON_TYPE_KEY] = JSON_TYPE_CodeSpec;
    entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& row = entry[JSON_OBJECT_KEY];
    row.clear();
    row["CodeSpecType"] = codeSpecType;
    row["Name"] = name;
    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
    row[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue()).c_str();

    m_authorityIds[name] = IdToString(m_nextAvailableId.GetValue());
    (QueueJson)(entry.toStyledString().c_str());
    return m_nextAvailableId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2016
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String BisJson1ExporterImpl::RemapResourceAuthority(Json::Value& obj, ECN::ECClassCP elementClass)
    {
    Utf8String authority;
    if (elementClass->GetName().Equals("AnnotationTextStyle"))
        authority = m_authorityIds["bis:AnnotationTextStyle"];
    else if (elementClass->GetName().Equals("AnnotationFrameStyle"))
        authority = m_authorityIds["bis:AnnotationFrameStyle"];
    else if (elementClass->GetName().Equals("AnnotationLeaderStyle"))
        authority = m_authorityIds["bis:AnnotationLeaderStyle"];
    else if (elementClass->GetName().Equals("TextAnnotationSeed"))
        authority = m_authorityIds["bis:TextAnnotationSeed"];
    else if (elementClass->GetName().Equals("Texture"))
        authority = m_authorityIds["bis:Texture"];
    else if (elementClass->GetName().Equals("TrueColor"))
        authority = m_authorityIds["bis:TrueColor"];
    else if (elementClass->GetName().Equals("ViewDefinition"))
        authority = m_authorityIds["bis:ViewDefinition"];
    else if (elementClass->GetName().Equals("LineStyle"))
        authority = m_authorityIds["bis:LineStyle"];
    if (!Utf8String::IsNullOrEmpty(authority.c_str()))
        return authority;

    for (ECN::ECClassCP base : elementClass->GetBaseClasses())
        {
        authority = RemapResourceAuthority(obj, base);
        if (!Utf8String::IsNullOrEmpty(authority.c_str()))
            return authority;
        }
    return authority;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId BisJson1ExporterImpl::CreateDisplayStyle(Json::Value& out, ViewControllerCR vc, Utf8CP name, bool is3d)
    {
    Render::ViewFlags viewFlags = vc.GetViewFlags();

    auto& displayStyle = out.append(Json::ValueType::objectValue);
    displayStyle[JSON_TYPE_KEY] = JSON_TYPE_DisplayStyle;
    displayStyle[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    displayStyle[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& row = displayStyle[JSON_OBJECT_KEY];
    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
    row[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue());
    row["Name"] = name;
    row["ViewFlags"] = Json::Value(Json::ValueType::objectValue);

    // ViewFlags Json has changed in bim0200, so we need to manually create the Json
    auto& vf = row["ViewFlags"];
    viewFlags.ToBaseJson(vf);
    vf.removeMember("construct");
    vf.removeMember("fill");
    if (!viewFlags.m_constructions) vf["noConstruct"] = true;
    if (!viewFlags.m_fill) vf["noFill"] = true;

    if (is3d)
        {
        viewFlags.To3dJson(vf);
        vf.removeMember("ignoreLighting");
        vf.removeMember("noClipVol");
        if (!viewFlags.m_noClipVolume) vf["clipVol"] = true;
        if (viewFlags.m_ignoreLighting) vf["noLighting"] = true;
        }

    row["BackgroundColor"] = vc.GetBackgroundColor().GetValue();
    row["Is3d"] = is3d;
    (QueueJson)(displayStyle.toStyledString().c_str());

    return m_nextAvailableId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId BisJson1ExporterImpl::CreateCategorySelector(Json::Value& out, ViewControllerCR vc, Utf8CP name)
    {
    // Need to export the categories before we can create the category selector
    for (DgnCategoryId id : vc.GetViewedCategories())
        {
        if (m_insertedElements.end() != m_insertedElements.find(id))
            continue;

        DgnCategoryCPtr cat = DgnCategory::QueryCategory(id, *m_dgndb);
        if (!cat.IsValid())
            continue;
        Utf8PrintfString whereClause(" AND ECInstanceId=%" PRIu64, cat->GetElementId().GetValue());
        ExportElements(out, cat->GetElementClass()->GetSchema().GetName().c_str(), cat->GetElementClass()->GetName().c_str(), cat->GetModelId(), whereClause.c_str());
        auto& entry = out[out.size() - 1][JSON_OBJECT_KEY];
        auto& codeSpec = entry[BIS_ELEMENT_PROP_CodeSpec] = Json::Value(Json::ValueType::objectValue);
        if (vc.IsDrawingView() || vc.IsSheetView())
            {
            entry[JSON_CLASSNAME] = "BisCore.DrawingCategory";
            codeSpec["id"] = m_authorityIds["bis:DrawingCategory"].c_str();
            }
        else
            {
            entry[JSON_CLASSNAME] = "BisCore.SpatialCategory";
            codeSpec["id"] = m_authorityIds["bis:SpatialCategory"].c_str();
            }
        }

    auto& categorySelector = out.append(Json::ValueType::objectValue);
    categorySelector[JSON_TYPE_KEY] = JSON_TYPE_CategorySelector;
    categorySelector[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    categorySelector[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& row = categorySelector[JSON_OBJECT_KEY];
    row.clear();
    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
    row[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue());
    row["Name"] = name;
    row["Categories"] = Json::Value(Json::ValueType::arrayValue);
    DgnElementId categorySelectorId = m_nextAvailableId;
    for (DgnCategoryId id : vc.GetViewedCategories())
        {
        Json::Value category(Json::ValueType::uintValue);
        category = IdToString(id.GetValue());
        row["Categories"].append(category);
        }
    (QueueJson)(categorySelector.toStyledString().c_str());
    return categorySelectorId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId BisJson1ExporterImpl::CreateModelSelector(Json::Value& out, ViewControllerCR vc, Utf8CP name)
    {
    auto& modelSelector = out.append(Json::ValueType::objectValue);
    modelSelector[JSON_TYPE_KEY] = JSON_TYPE_ModelSelector;
    modelSelector[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    modelSelector[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& row = modelSelector[JSON_OBJECT_KEY];
    row.clear();
    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
    row[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue());
    row["Name"] = name;
    row["Models"] = Json::Value(Json::ValueType::arrayValue);
    for (DgnModelId id : vc.GetViewedModels())
        {
        Json::Value model(Json::ValueType::uintValue);
        model = IdToString(id.GetValue());
        row["Models"].append(model);
        }
    (QueueJson)(modelSelector.toStyledString().c_str());
    return m_nextAvailableId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId BisJson1ExporterImpl::CreateDrawingElement(Json::Value& out, Utf8CP name)
    {
    auto& drawingElement = out.append(Json::ValueType::objectValue);
    drawingElement[JSON_TYPE_KEY] = JSON_TYPE_Element;
    drawingElement[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    drawingElement[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& obj = drawingElement[JSON_OBJECT_KEY];
    obj.clear();
    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);

    obj[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue()).c_str();
    obj[JSON_CLASSNAME] = "BisCore.Drawing";

    Utf8String codeNS = IdToString(m_documentListModelId.GetValue());
    MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeSpec, m_authorityIds["bis:Drawing"].c_str());
    MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeScope, codeNS.c_str());

    obj[BIS_ELEMENT_PROP_CodeValue] = name;
    obj["UserLabel"] = name;
    MakeNavigationProperty(obj, BIS_ELEMENT_PROP_Model, codeNS.c_str());
    (QueueJson)(drawingElement.toStyledString().c_str());
    return m_nextAvailableId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId BisJson1ExporterImpl::CreateSheetElement(Json::Value& out, DgnModelCR model)
    {
    auto& sheetElement = out.append(Json::ValueType::objectValue);
    sheetElement[JSON_TYPE_KEY] = JSON_TYPE_Element;
    sheetElement[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    sheetElement[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& obj = sheetElement[JSON_OBJECT_KEY];
    obj.clear();
    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);

    obj[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue()).c_str();
    obj[JSON_CLASSNAME] = "BisCore.Sheet";

    Utf8String codeNS = IdToString(m_sheetListModelId.GetValue());
    MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeSpec, m_authorityIds["bis:Sheet"].c_str());
    MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeScope, codeNS.c_str());
    obj[BIS_ELEMENT_PROP_CodeValue] = model.GetName().c_str();
    obj["UserLabel"] = model.GetName().c_str();
    MakeNavigationProperty(obj, BIS_ELEMENT_PROP_Model, codeNS.c_str());

    obj["Height"] = model.ToSheetModel()->GetSize().y;
    obj["Width"] = model.ToSheetModel()->GetSize().x;

    (QueueJson)(sheetElement.toStyledString().c_str());
    return m_nextAvailableId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportViews(Json::Value& out)
    {
    auto iter = ViewDefinition::MakeIterator(*m_dgndb);
    for (auto const& entry : iter)
        {
        ViewDefinitionCPtr view = ViewDefinition::QueryView(entry.GetId(), *m_dgndb);
        if (!view.IsValid())
            continue;

        ViewControllerPtr vc = ViewDefinition::LoadViewController(entry.GetId(), *m_dgndb);
        if (!vc.IsValid())
            continue;

        DgnElementId categorySelectorId = CreateCategorySelector(out, *vc, view->GetName().c_str());
        DgnElementId displayStyle = CreateDisplayStyle(out, *vc, view->GetName().c_str(), view->GetElementClass()->Is(m_viewDefinition3dClass));
        DgnElementId modelSelectorId;
        if (view->GetElementClass()->Is(m_viewDefinition3dClass))
            modelSelectorId = CreateModelSelector(out, *vc, view->GetName().c_str());

        Utf8PrintfString whereClause(" AND ECInstanceId=%" PRIu64, view->GetElementId().GetValue());
        ExportElements(out, view->GetElementClass()->GetSchema().GetName().c_str(), view->GetElementClass()->GetName().c_str(), view->GetModelId(), whereClause.c_str(), false);

        auto& row = out[out.size() - 1];
        auto& obj = row[JSON_OBJECT_KEY];
        MakeNavigationProperty(obj, "CategorySelector", categorySelectorId.GetValue());
        MakeNavigationProperty(obj, "DisplayStyle", displayStyle.GetValue());
        MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeSpec, m_authorityIds["bis:ViewDefinition"].c_str());
        if (obj.isMember("Source"))
            {
            if ((DgnViewSource) (obj["Source"].asInt()) == DgnViewSource::Private)
                obj["IsPrivate"] = true;
            else
                obj["IsPrivate"] = false;
            obj.removeMember("Source");
            }

        if (view->GetElementClass()->Is(m_viewDefinition3dClass))
            {
            row[JSON_TYPE_KEY] = JSON_TYPE_ViewDefinition3d;
            obj.removeMember("BaseModelId");

            MakeNavigationProperty(obj, "ModelSelector", modelSelectorId.GetValue());
            obj["Origin"] = Json::Value(Json::ValueType::objectValue);
            obj["Origin"]["x"] = vc->GetOrigin().x;
            obj["Origin"]["y"] = vc->GetOrigin().y;
            obj["Origin"]["z"] = vc->GetOrigin().z;

            obj["Extents"] = Json::Value(Json::ValueType::objectValue);
            obj["Extents"]["x"] = vc->GetDelta().x;
            obj["Extents"]["y"] = vc->GetDelta().y;
            obj["Extents"]["z"] = vc->GetDelta().z;

            RotMatrix rot = vc->GetRotation();
            YawPitchRollAngles ypr;
            YawPitchRollAngles::TryFromRotMatrix(ypr, rot);
            obj["Yaw"] = ypr.GetYaw().Degrees();
            obj["Pitch"] = ypr.GetPitch().Degrees();
            obj["Roll"] = ypr.GetRoll().Degrees();

            if (view->GetElementClass()->Is(m_cameraViewDefinitionClass))
                {
                CameraViewControllerCP cvc = dynamic_cast<CameraViewControllerCP>(vc.get());
                obj[JSON_CLASSNAME] = "BisCore.SpatialViewDefinition";

                obj["EyePoint"] = Json::Value(Json::ValueType::objectValue);
                obj["EyePoint"]["x"] = cvc->GetEyePoint().x;
                obj["EyePoint"]["y"] = cvc->GetEyePoint().y;
                obj["EyePoint"]["z"] = cvc->GetEyePoint().z;

                obj["FocusDistance"] = cvc->GetFocusDistance();
                obj["LensAngle"] = cvc->GetLensAngle();
                obj["IsCameraOn"] = cvc->IsCameraOn();
                }
            else
                {
                obj[JSON_CLASSNAME] = "BisCore.OrthographicViewDefinition";
                }
            }
        else if (view->GetElementClass()->Is(m_viewDefinition2dClass))
            {
            row[JSON_TYPE_KEY] = JSON_TYPE_ViewDefinition2d;
            Json::Value tmp(obj["BaseModelId"]);
            MakeNavigationProperty(obj, "BaseModel", tmp);

            obj.removeMember("BaseModelId");
            obj["Origin"] = Json::Value(Json::ValueType::objectValue);
            obj["Origin"]["x"] = vc->GetOrigin().x;
            obj["Origin"]["y"] = vc->GetOrigin().y;

            obj["Extents"] = Json::Value(Json::ValueType::objectValue);
            obj["Extents"]["x"] = vc->GetDelta().x;
            obj["Extents"]["y"] = vc->GetDelta().y;

            DVec3d xColumn; 
            RotMatrix rot = vc->GetRotation();
            rot.GetColumn(xColumn, 0);
            obj["RotationAngle"] = atan2(xColumn.y, xColumn.x);

            if (view->GetElementClass()->Is(m_drawingViewDefinitionClass))
                obj[JSON_CLASSNAME] = "BisCore.DrawingViewDefinition";
            else if (view->GetElementClass()->Is(m_sheetViewDefinitionClass))
                obj[JSON_CLASSNAME] = "BisCore.SheetViewDefinition";
            }
        (QueueJson)(row.toStyledString().c_str());
        }
        return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportCategories(Json::Value& out)
    {
    ExportCategories(out, "dgn_GeometricElement2d", "BisCore.DrawingCategory", m_authorityIds["bis:DrawingCategory"].c_str());
    ExportCategories(out, "dgn_GeometricElement3d", "BisCore.SpatialCategory", m_authorityIds["bis:SpatialCategory"].c_str());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportCategories(Json::Value& out, Utf8CP tableName, Utf8CP bisClassName, Utf8CP bisAuthorityStr)
    {
    Utf8PrintfString sql("select DISTINCT g.CategoryId, e.ModelId,  s.NAME, c.NAME FROM ec_Schema s, ec_Class c, %s g, dgn_Element e WHERE g.CategoryId = e.Id AND e.ECClassId = c.[Id] and c.[SchemaId] = s.[Id]", tableName);
    Statement stmt;
    auto stat = stmt.Prepare(*m_dgndb, sql.c_str());
    if (DbResult::BE_SQLITE_OK != stat)
        {
        Utf8String error;
        error.Sprintf("Failed get prepared SQL statement for SELECTing categories: %s", sql.c_str());
        LogMessage(TeleporterLoggingSeverity::LOG_WARNING, error.c_str());
        return ERROR;
        }

    while (BE_SQLITE_ROW == stmt.Step())
        {
        ECInstanceId categoryId = stmt.GetValueId<ECInstanceId>(0);
        if (m_insertedElements.find(DgnElementId(categoryId.GetValue())) != m_insertedElements.end())
            continue;
        Utf8PrintfString whereClause(" AND ECInstanceId=%" PRIu64, categoryId.GetValue());
        ExportElements(out, stmt.GetValueText(2), stmt.GetValueText(3), stmt.GetValueId<DgnModelId>(1), whereClause.c_str());
        auto& entry = out[out.size() - 1][JSON_OBJECT_KEY];
        entry[JSON_CLASSNAME] = bisClassName;

        MakeNavigationProperty(entry, BIS_ELEMENT_PROP_CodeSpec, bisAuthorityStr);
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportSchemas(Json::Value& out) const
    {
    bvector<Utf8String> knownSchemas = {"Bentley_Standard_Classes", "Bentley_Standard_CustomAttributes", "CoreCustomAttributes", "ECDbMap", "ECDbFileInfo", "ECDbSystem","ECDb_FileInfo", "ECDb_System", "EditorCustomAttributes", "Generic", "MetaSchema", "dgn", "PointCloud", "Raster", "ThreeMx", "ECv3ConversionAttributes"};
    bvector<ECSchemaCP> ecSchemas = m_dgndb->Schemas().GetECSchemas();
    ECSchemaCP dgnSchema = m_dgndb->Schemas().GetECSchema("dgn");
    ECSchemaP nonConstDgn = const_cast<ECSchemaP>(dgnSchema);
    ECSchemaCP genericSchema = m_dgndb->Schemas().GetECSchema("generic");
    ECClassCP defaultBase = genericSchema->GetClassCP("ElementRefersToElement");
    ECClassCP defaultAspectBase = genericSchema->GetClassCP("ElementOwnsMultiAspect");
    ECClassCP aspectClass = dgnSchema->GetClassCP("ElementMultiAspect");
    bvector<Utf8String> alreadyVisited;

    ECN::ECSchemaReadContextPtr schemaReadContext;
    schemaReadContext = ECN::ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(m_dgndb->GetSchemaLocater());

    for (ECN::ECSchemaCP ischema : ecSchemas)
        {
        bvector<ECSchemaCP> orderedSchemas;
        if (knownSchemas.end() != std::find(knownSchemas.begin(), knownSchemas.end(), ischema->GetName()))
            continue;
        ischema->FindAllSchemasInGraph(orderedSchemas);

        for (ECN::ECSchemaCP schema : orderedSchemas)
            {
            if (knownSchemas.end() != std::find(knownSchemas.begin(), knownSchemas.end(), schema->GetName()))
                continue;
            if (alreadyVisited.end() != std::find(alreadyVisited.begin(), alreadyVisited.end(), schema->GetName()))
                continue;
            Utf8String copyXml;
            if (ECN::SchemaWriteStatus::Success != schema->WriteToXmlString(copyXml, 3, 0))
                continue;

            ECSchemaPtr copied = nullptr;
            ECN::SchemaReadStatus readStat = ECN::ECSchema::ReadFromXmlString(copied, copyXml.c_str(), *schemaReadContext);
            if (ECN::SchemaReadStatus::Success != readStat)
                continue;

            if (!ECN::ECSchema::IsSchemaReferenced(*copied, *dgnSchema))
                {
                copied->AddReferencedSchema(*nonConstDgn);
                }

            for (ECN::ECClassCP ecClass : copied->GetClasses())
                {
                ECN::ECClassP nonConstClass = const_cast<ECClassP>(ecClass);
                
                // Calculated ECProperties are not read-only in bis
                for (ECN::ECPropertyP prop : nonConstClass->GetProperties(false))
                    {
                    if (prop->IsCalculated())
                        prop->SetIsReadOnly(false);
                    // BisCore renames several properties on core classes.  This could cause a conflict with an inherited class's property, so we need to rename it
                    if (prop->GetName().EqualsIAscii("Model") || prop->GetName().EqualsIAscii("Parent") || prop->GetName().EqualsIAscii("Category"))
                        {
                        nonConstClass->RenameConflictProperty(prop, true);
                        }
                    }

                if (!ecClass->IsRelationshipClass())
                    continue;
                if (nonConstClass->GetBaseClasses().size() < 2)
                    {
                    if (!(*nonConstClass->GetBaseClasses().begin())->Is(defaultBase))
                        continue;
                    nonConstClass->RemoveBaseClass(*(*nonConstClass->GetBaseClasses().begin()));
                    }
                for (ECN::ECClassCP base : nonConstClass->GetBaseClasses())
                    nonConstClass->RemoveBaseClass(*base);

                ECClassCP base = defaultBase;
                for (ECN::ECEntityClassP constraint : nonConstClass->GetRelationshipClassP()->GetTarget().GetClasses())
                    {
                    if (constraint->Is(aspectClass))
                        {
                        base = defaultAspectBase;
                        break;
                        }
                    }
                nonConstClass->AddBaseClass(*base);
                }
            Utf8String schemaXml;
            if (ECN::SchemaWriteStatus::Success != copied->WriteToXmlString(schemaXml, 3, 0))
                continue;
            schemaXml.ReplaceAll("dgn:", "bis:");
            schemaXml.ReplaceAll("name=\"dgn\" version=\"02.00.00\" prefix=\"dgn\"", "name=\"BisCore\" version=\"01.00\" prefix=\"bis\"");
            schemaXml.ReplaceAll("xmlns=\"dgn.02.00\"", "xmlns=\"BisCore.01.00\"");
            schemaXml.ReplaceAll("name=\"ECDbMap\" version=\"01.00.01\"", "name=\"ECDbMap\" version=\"02.00\"");
            schemaXml.ReplaceAll("generic:ElementRefersToElement", "bis:ElementRefersToElements");
            schemaXml.ReplaceAll("ECStructArrayProperty", "ECProperty");
            schemaXml.ReplaceAll("generic:PhysicalObject", "bis:PhysicalElement");
            schemaXml.ReplaceAll("generic:SpatialGroup", "generic:Group");


            auto& entry = out.append(Json::ValueType::objectValue);
            entry[JSON_TYPE_KEY] = JSON_TYPE_Schema;
            entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
            entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
            auto& obj = entry[JSON_OBJECT_KEY];
            obj.clear();
            obj[schema->GetFullSchemaName().c_str()] = schemaXml.c_str();
            alreadyVisited.push_back(schema->GetName());
            (QueueJson)(entry.toStyledString().c_str());
            }
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId BisJson1ExporterImpl::InitListModel(Json::Value& out, Utf8CP name)
    {
    DgnElementId partitionId = CreatePartitionElement(name, "DocumentPartition", m_jobSubjectId, out);
    auto& entry = out.append(Json::ValueType::objectValue);
    entry[JSON_TYPE_KEY] = JSON_TYPE_Model;
    entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& obj = entry[JSON_OBJECT_KEY];
    obj.clear();
    obj[JSON_CLASSNAME] = "BisCore.DocumentListModel";

    MakeNavigationProperty(obj, BIS_MODEL_PROP_ModeledElement, partitionId.GetValue());

    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
    obj[JSON_INSTANCE_ID] = IdToString(partitionId.GetValue()).c_str();
    (QueueJson)(entry.toStyledString().c_str());
    return partitionId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::InitDrawingListModel(Json::Value& out)
    {
    m_documentListModelId = InitListModel(out, "Converted Drawings");
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::InitSheetListModel(Json::Value& out)
    {
    m_sheetListModelId = InitListModel(out, "Converted Sheets");
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportModels(Json::Value& out)
    {
    Utf8PrintfString subjectName("DgnDb0601: %s", Utf8String(m_dgndb->GetFileName().GetFileNameWithoutExtension().c_str()).c_str());
    m_jobSubjectId = CreateSubjectElement(subjectName.c_str(), out);
    InitDrawingListModel(out);
    InitSheetListModel(out);

    Statement stmt;
    Utf8PrintfString sql("SELECT s.NAME, c.NAME from ec_Schema s, ec_Class c WHERE s.ID = c.[SchemaId] and c.Id in (SELECT DISTINCT ECClassId FROM %s_%s)", DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Model);
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql.c_str())))
        {
        LogMessage(TeleporterLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve DgnModels");
        return ERROR;
        }

    BentleyStatus stat = SUCCESS;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        if (SUCCESS != (stat = ExportModel(out, stmt.GetValueText(0), stmt.GetValueText(1))))
            break;
        }
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportModel(Json::Value& out, Utf8CP schemaName, Utf8CP className, Utf8CP whereClause)
    {
    Utf8PrintfString ecSql("SELECT ECInstanceId, * FROM ONLY %s.%s", schemaName, className);
    if (nullptr != whereClause)
        ecSql.append(whereClause);

    CachedECSqlStatementPtr statement = m_dgndb->GetPreparedECSqlStatement(ecSql.c_str());
    if (!statement.IsValid())
        {
        LogMessage(TeleporterLoggingSeverity::LOG_FATAL, "Unable to get cached statement ptr.");
        return ERROR;
        }
    JsonECSqlSelectAdapter jsonAdapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));

    while (BE_SQLITE_ROW == statement->Step())
        {
        // First need to create a SubjectElement that will be used for the ModelModelsElement relationship
        ECInstanceId actualElementId = statement->GetValueId<ECInstanceId>(0);
        if (m_insertedModels.find(DgnModelId(actualElementId.GetValue())) != m_insertedModels.end())
            continue;

        DgnModelPtr model = m_dgndb->Models().GetModel(DgnModelId(actualElementId.GetValue()));
        DgnElementId modeledElementId;
        if (model->IsSheetModel())
            modeledElementId = CreateSheetElement(out, *model);
        else if (model->Is2dModel())
            modeledElementId = CreateDrawingElement(out, model->GetName().c_str());
        else if (!model->IsDictionaryModel())
            modeledElementId = CreatePartitionElement(*model, DgnElementId(), out);
        
        auto& entry = out.append(Json::ValueType::objectValue);
        if (model->IsDictionaryModel())
            entry[JSON_TYPE_KEY] = JSON_TYPE_DictionaryModel;
        else
            entry[JSON_TYPE_KEY] = JSON_TYPE_Model;

        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto& obj = entry[JSON_OBJECT_KEY];
        obj.clear();
        jsonAdapter.GetRowInstance(obj);

        // Certain classes are now abstract in BisCore, so they need to be converted to the derived concrete classes
        if (model->IsGroupInformationModel())
            obj[JSON_CLASSNAME] = "Generic.GroupModel";
        else if (model->IsSpatialModel())
            {
            Utf8String tmp(obj["$ECClassKey"].asString());
            tmp.ReplaceAll("dgn.SpatialModel", "BisCore.PhysicalModel");
            obj[JSON_CLASSNAME] = tmp.c_str();
            }
        else if (model->IsSheetModel())
            {
            Utf8String tmp(obj["$ECClassKey"].asString());
            tmp.ReplaceAll("dgn.SheetModel", "BisCore.SheetModel");
            obj[JSON_CLASSNAME] = tmp.c_str();
            obj.removeMember("SheetSize");
            }
        else if (nullptr != model->ToGeometricModel2d())
            { 
            Utf8String tmp(obj["$ECClassKey"].asString());
            tmp.ReplaceAll("dgn.GeometricModel2d", "BisCore.DrawingModel");
            obj[JSON_CLASSNAME] = tmp.c_str();
            }
        else
            {
            Utf8String tmp(obj["$ECClassKey"].asString());
            tmp.ReplaceAll("dgn.", "BisCore.");
            obj[JSON_CLASSNAME] = tmp.c_str();
            }

        obj.removeMember("Code");
        obj.removeMember("Label");
        obj.removeMember("DependencyIndex");
        obj.removeMember("$ECClassKey");
        obj.removeMember("$ECInstanceLabel");
        obj.removeMember("$ECClassId");
        obj.removeMember("$ECClassLabel");

        obj[JSON_INSTANCE_ID] = IdToString(obj["$ECInstanceId"].asString().c_str()).c_str();

        obj.removeMember("$ECInstanceId");

        if (obj.isMember("Visibility"))
            {
            obj["IsPrivate"] = obj["Visibility"].asInt() == 0;
            obj.removeMember("Visibility");
            }

        if (obj.isMember("Properties"))
            {
            if (!obj["Properties"].isNull() && !Utf8String::IsNullOrEmpty(obj["Properties"].asString().c_str()))
                obj["JsonProperties"] = obj["Properties"].asString().c_str();
            obj.removeMember("Properties");
            }

        if (modeledElementId.IsValid())
            MakeNavigationProperty(obj, BIS_MODEL_PROP_ModeledElement, modeledElementId.GetValue());

        m_insertedModels[model->GetModelId()] = 1;
        (QueueJson)(entry.toStyledString().c_str());

        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportElements(Json::Value& out)
    {
    BentleyStatus stat = SUCCESS;
    DgnModels::Iterator modelsIterator = m_dgndb->Models().MakeIterator();
    for (DgnModels::Iterator::Entry modelEntry : modelsIterator)
        {
        DgnModelPtr model = m_dgndb->Models().GetModel(modelEntry.GetModelId());
        if (SUCCESS != (stat = ExportElements(out, model->GetModelId())))
            break;
        }
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportElements(Json::Value& out, DgnModelId parentModel)
    {
    Statement stmt;
    Utf8CP sql = "SELECT s.NAME, c.NAME from ec_Schema s, ec_Class c WHERE s.ID = c.[SchemaId] and c.Id in (SELECT DISTINCT ECClassId FROM dgn_Element WHERE ModelId=?)";
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql)))
        {
        LogMessage(TeleporterLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve element classes for model");
        return ERROR;
        }
    stmt.BindId(1, parentModel);

    BentleyStatus stat = SUCCESS;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        if (SUCCESS != (stat = ExportElements(out, stmt.GetValueText(0), stmt.GetValueText(1), parentModel)))
            break;
        }
    return stat;
    }

void RenameConflictMembers(Json::Value& obj, Utf8CP prefix, Utf8CP member)
    {
    for (auto const& id : obj.getMemberNames())
        {
        if (0 == stricmp(id.c_str(), member))
            {
            Utf8PrintfString tmp("%s_%s_", prefix, member);
            obj[tmp.c_str()] = obj[id];
            obj.removeMember(id);
            return;
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportElements(Json::Value& out, Utf8CP schemaName, Utf8CP className, DgnModelId parentModel, Utf8CP whereClause, bool sendToQueue)
    {
    Utf8PrintfString ecSql("SELECT ECInstanceId, * FROM ONLY %s.%s WHERE ModelId=?", schemaName, className);
    if (nullptr != whereClause)
        ecSql.append(whereClause);
    CachedECSqlStatementPtr statement = m_dgndb->GetPreparedECSqlStatement(ecSql.c_str());
    if (!statement.IsValid())
        {
        LogMessage(TeleporterLoggingSeverity::LOG_FATAL, "Unable to get cached statement ptr.");
        return ERROR;
        }
    statement->BindId(1, parentModel);
    JsonECSqlSelectAdapter jsonAdapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    jsonAdapter.SetStructArrayAsString(true);
    Utf8String prefix = m_dgndb->Schemas().GetECSchema(schemaName)->GetNamespacePrefix();

    while (BE_SQLITE_ROW == statement->Step())
        {
        ECInstanceId actualElementId = statement->GetValueId<ECInstanceId>(0);
        if (m_insertedElements.end() != m_insertedElements.find(DgnElementId(actualElementId.GetValue())))
            continue;

        DgnElementCPtr element = m_dgndb->Elements().GetElement(DgnElementId(actualElementId.GetValue()));
        if (m_insertedModels.find(element->GetModelId()) == m_insertedModels.end())
            {
            ECClassCP modelClass = m_dgndb->Schemas().GetECClass(element->GetModel()->GetClassId());
            Utf8PrintfString whereClause(" WHERE ECInstanceId=%" PRIu64, element->GetModelId().GetValue());
            if (SUCCESS != ExportModel(out, modelClass->GetSchema().GetName().c_str(), modelClass->GetName().c_str(), whereClause.c_str()))
                return ERROR;
            }

        if (element->GetParentId().IsValid() && m_insertedElements.end() == m_insertedElements.find(element->GetParentId()))
            {
            DgnElementCPtr parent = m_dgndb->Elements().GetElement(element->GetParentId());
            Utf8PrintfString whereClause(" AND ECInstanceId=%" PRIu64, parent->GetElementId().GetValue());
            if (SUCCESS != ExportElements(out, parent->GetElementClass()->GetSchema().GetName().c_str(), parent->GetElementClass()->GetName().c_str(), parent->GetModelId(), whereClause.c_str()))
                return ERROR;
            }

        auto& entry = out.append(Json::ValueType::objectValue);
        entry[JSON_TYPE_KEY] = JSON_TYPE_Element;
        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        Json::Value obj = Json::Value(Json::ValueType::objectValue);
        obj.clear();
        jsonAdapter.GetRowInstance(obj);

        obj[JSON_INSTANCE_ID] = IdToString(obj["$ECInstanceId"].asString().c_str()).c_str();
        obj.removeMember("$ECInstanceId");

        MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeSpec, IdToString(obj["Code"]["AuthorityId"].asString().c_str()).c_str());

        // Most things will get scoped to the dictionary model.  There are a few exceptions that will override thi
        MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeScope, m_dgndb->GetDictionaryModel().GetModelId().GetValue());
        obj[BIS_ELEMENT_PROP_CodeValue] = obj["Code"]["Value"];
        obj.removeMember("LastMod");
        obj["UserLabel"] = obj["Label"];
        obj.removeMember("Label");
        obj["JsonProperties"] = Utf8String(obj["UserProperties"].asString().c_str()).c_str();
        obj.removeMember("UserProperties");

        RenameConflictMembers(obj, prefix.c_str(), "Model");
        RenameConflictMembers(obj, prefix.c_str(), "Description");
        RenameConflictMembers(obj, prefix.c_str(), "Parent");
        RenameConflictMembers(obj, prefix.c_str(), "Category");

        if (obj.isMember("ParentId"))
            {
            if (!obj["ParentId"].isNull())
                MakeNavigationProperty(obj, BIS_ELEMENT_PROP_Parent, IdToString(obj["ParentId"].asString().c_str()).c_str());
            obj.removeMember("ParentId");
            }

        if (obj.isMember("ModelId"))
            {
            if (!obj["ModelId"].isNull())
                MakeNavigationProperty(obj, BIS_ELEMENT_PROP_Model, IdToString(obj["ModelId"].asString().c_str()).c_str());

            obj.removeMember("ModelId");
            }

        if (obj.isMember("CategoryId"))
            {
            if (!obj["CategoryId"].isNull())
                MakeNavigationProperty(obj, "Category", IdToString(obj["CategoryId"].asString().c_str()).c_str());
            obj.removeMember("CategoryId");
            }

        if (obj.isMember("Descr"))
            {
            obj["Description"] = obj["Descr"].asString();
            obj.removeMember("Descr");
            }
        if (element->GetElementClass()->Is(m_subCategoryClass))
            {
            entry[JSON_TYPE_KEY] = JSON_TYPE_SubCategory;
            DgnSubCategoryCPtr sub = m_dgndb->Elements().Get<DgnSubCategory>(element->GetElementId());
            if (sub->IsDefaultSubCategory())
                entry["IsDefaultSubCategory"] = true;
            MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeScope, element->GetParentId());
            }
        else if (element->GetElementClass()->Is(m_categoryClass))
            {
            entry[JSON_TYPE_KEY] = JSON_TYPE_Category;
            obj[JSON_CLASSNAME] = "BisCore.SpatialCategory";
            obj[BIS_ELEMENT_PROP_CodeSpec]["id"] = m_authorityIds["bis:SpatialCategory"].c_str();
            obj.removeMember("Scope");
            }
        else if (element->IsGeometricElement())
            {
            if (element->Is2d())
                entry[JSON_TYPE_KEY] = JSON_TYPE_GeometricElement2d;
            else
                entry[JSON_TYPE_KEY] = JSON_TYPE_GeometricElement3d;
            // The GeomStream retrieved directly from the database is apparently incomplete.  Need to actually get it off the element
            GeometryStreamCR geom = element->ToGeometrySource()->GetGeometryStream();
            if (geom.HasData())
                {
                Utf8String encode;
                if (SUCCESS != Base64Utilities::Encode(encode, geom.GetData(), geom.GetSize()))
                    obj.removeMember("GeometryStream");
                else
                    obj["GeometryStream"] = encode.c_str();

                }
            }
        else if (nullptr != element->ToGeometryPart())
            {
            entry[JSON_TYPE_KEY] = JSON_TYPE_GeometryPart;
            GeometryStreamCR geom = element->ToGeometryPart()->GetGeometryStream();
            Utf8String encode;
            if (SUCCESS != Base64Utilities::Encode(encode, geom.GetData(), geom.GetSize()))
                obj.removeMember("GeometryStream");
            else
                obj["GeometryStream"] = encode.c_str();
            }
        else if (element->IsGroupInformationElement())
            {
            Utf8String tmp = obj["$ECClassKey"].asString();
            tmp.ReplaceAll("Generic.SpatialGroup", "Generic.Group");
            tmp.ReplaceAll("Generic.GraphicGroup2d", "Generic.Group");
            obj[JSON_CLASSNAME] = tmp.c_str();
            }
        else if (element->GetElementClass()->Is(m_textureClass))
            {
            DgnTextureCPtr texture = m_dgndb->Elements().Get<DgnTexture>(element->GetElementId());
            Render::ImageSourceCR data = texture->GetImageSource();
            ByteStream const& stream = data.GetByteStream();
            Utf8String encode;
            if (SUCCESS != Base64Utilities::Encode(encode, stream.GetData(), stream.GetSize()))
                obj.removeMember("Data");
            else
                obj["Data"] = encode.c_str();
            entry[JSON_TYPE_KEY] = JSON_TYPE_Texture;
            obj["Height"] = texture->GetHeight();
            obj["Width"] = texture->GetWidth();
            obj["Format"] = (uint32_t) data.GetFormat();
            }

        if (element->GetCodeAuthority()->GetName().Equals("DgnResources"))
            {
            Utf8String authority = RemapResourceAuthority(obj, element->GetElementClass());
            if (!Utf8String::IsNullOrEmpty(authority.c_str()))
                obj[BIS_ELEMENT_PROP_CodeSpec]["id"] = authority.c_str();
            }
        else if (element->GetCodeAuthority()->GetName().Equals("DgnV8"))
            {
            Utf8PrintfString value("%s-%s", obj["Code"]["Namespace"].asString(), obj["Code"]["Value"].asString());
            obj[BIS_ELEMENT_PROP_CodeValue] = value.c_str();
            }

        if (obj.isMember("Data"))
            {
            if (element->GetElementClass()->GetName().EqualsI("MaterialElement"))
                {
                Utf8PrintfString assets("{\"materialAssets\":%s}", obj["Data"].asString().c_str());
                assets.ReplaceAll("RenderMaterial", "renderMaterial");
                obj["JsonProperties"] = assets.c_str();
                obj.removeMember("Data");

                // This is a bit of a hack.  But we sometimes end up with duplicate MaterialElements that differ by CodeNamespace.
                if (!Utf8String::IsNullOrEmpty(element->GetCode().GetNamespace().c_str()))
                    {
                    Utf8PrintfString value("%s-%s", obj["Code"]["Namespace"].asString(), obj["Code"]["Value"].asString());
                    obj[BIS_ELEMENT_PROP_CodeValue] = value.c_str();
                    }
                }
            else if (element->GetElementClass()->Is(m_annotationTextStyle))
                {
                entry[JSON_TYPE_KEY] = JSON_TYPE_AnnotationTextStyle;
                HandleAnnotationTextStyle(obj, element->GetElementId());
                obj.removeMember("Data");
                }
            }
        obj.removeMember("Code");
        if (element->GetElementClass()->GetName().EqualsI("MaterialElement"))
            obj[JSON_CLASSNAME] = "BisCore.RenderMaterial";

        if (obj.isMember("$ECClassKey") && !obj.isMember(JSON_CLASSNAME))
            {
            Utf8String tmp = obj["$ECClassKey"].asString();
            obj[JSON_CLASSNAME] = tmp.c_str();
            }

        if (obj.isMember("MODEL"))
            obj.removeMember("MODEL");
        obj.removeMember("$ECClassKey");
        obj.removeMember("$ECClassId");
        obj.removeMember("$ECClassLabel");
        obj.removeMember("$ECInstanceLabel");
        entry[JSON_OBJECT_KEY] = obj;
        m_insertedElements[element->GetElementId()] = 1;
        if (sendToQueue)
            SendToQueue(entry.toStyledString().c_str());
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
void BisJson1ExporterImpl::HandleAnnotationTextStyle(Json::Value& obj, DgnElementId id)
    {
    AnnotationTextStyleCPtr ats = m_dgndb->Elements().Get<AnnotationTextStyle>(id);
    if (!ats.IsValid())
        return;

    obj["ExplicitProperties"] = Json::Value(Json::ValueType::objectValue);
    auto& props = obj["ExplicitProperties"];
    props["AnnotationColorType"] = (int) ats->GetColorType();
    props["ColorValue"] = ats->GetColorValue().GetValue();
    props["FontId"] = IdToString(ats->GetFontId().GetValue()).c_str();
    props["Height"] = ats->GetHeight();
    props["LineSpacingFactor"] = ats->GetLineSpacingFactor();
    props["IsBold"] = ats->IsBold();
    props["IsItalic"] = ats->IsItalic();
    props["IsUnderlined"] = ats->IsUnderlined();
    props["StackedFractionScale"] = ats->GetStackedFractionScale();
    props["StackedFractionType"] = (int) ats->GetStackedFractionType();
    props["SubScriptOffsetFactor"] = ats->GetSubScriptOffsetFactor();
    props["SubScriptOffsetSclae"] = ats->GetSubScriptScale();
    props["WidthFactor"] = ats->GetWidthFactor();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId BisJson1ExporterImpl::CreateSubjectElement(Utf8CP subjectName, Json::Value& out)
    {
    auto& entry = out.append(Json::ValueType::objectValue);
    entry[JSON_TYPE_KEY] = JSON_TYPE_Subject;
    entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& subject = entry[JSON_OBJECT_KEY];
    subject.clear();

    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
    subject[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue()).c_str();
    subject["Label"] = subjectName;
    subject["Parent"] = -1;
    (QueueJson)(entry.toStyledString().c_str());

    return m_nextAvailableId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId BisJson1ExporterImpl::CreatePartitionElement(DgnModelCR model, DgnElementId subject, Json::Value& out)
    {
    Utf8String partitionType = nullptr;
    if (model.IsGroupInformationModel())
        partitionType = "GroupInformationPartition";
    else if (model.IsSpatialModel())
        partitionType = "PhysicalPartition";
    else if (model.IsSheetModel() || model.Is2dModel())
        partitionType = "DocumentPartition";
    else if (m_dgndb->Schemas().GetECClass(model.GetClassId())->Is(m_linkModelClass))
        partitionType = "LinkPartition";
    else if (model.IsDefinitionModel())
        partitionType = "DefinitionPartition";
    else
        {
        LogMessage(TeleporterLoggingSeverity::LOG_WARNING, "Unknown model type %s", typeid(model).name());
        }

    return CreatePartitionElement(model.GetName().c_str(), partitionType.c_str(), subject, out);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId BisJson1ExporterImpl::CreatePartitionElement(Utf8CP partitionName, Utf8CP partitionType, DgnElementId subject, Json::Value& out)
    {
    auto& entry = out.append(Json::ValueType::objectValue);
    entry[JSON_TYPE_KEY] = JSON_TYPE_Partition;
    entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& partition = entry[JSON_OBJECT_KEY];
    partition.clear();

    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
    partition[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue()).c_str();
    partition["Label"] = partitionName;
    if (subject.IsValid())
        partition["Subject"] = IdToString(subject.GetValue()).c_str();
    partition["PartitionType"] = partitionType;
    (QueueJson)(entry.toStyledString().c_str());

    return m_nextAvailableId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportNamedGroups(Json::Value& out)
    {
    Statement stmt;
    Utf8CP sql = "SELECT GroupId, MemberId, MemberPriority from dgn_ElementGroupsMembers";
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql)))
        {
        LogMessage(TeleporterLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve element groups elements instances");
        return ERROR;
        }

    auto& elemGroupsMembers = out.append(Json::ValueType::objectValue);
    elemGroupsMembers[JSON_TYPE_KEY] = JSON_TYPE_ElementGroupsMembers;
    elemGroupsMembers[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::arrayValue);
    elemGroupsMembers[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& groups = elemGroupsMembers[JSON_OBJECT_KEY];
    while (BE_SQLITE_ROW == stmt.Step())
        {
        Json::Value group(Json::ValueType::objectValue);
        group["GroupId"] = IdToString(stmt.GetValueId<DgnElementId>(0).GetValue()).c_str();
        group["MemberId"] = IdToString(stmt.GetValueId<DgnElementId>(1).GetValue()).c_str();
        group["MemberPriority"] = stmt.GetValueInt(2);
        groups.append(group);
        }
    (QueueJson)(elemGroupsMembers.toStyledString().c_str());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportElementHasLinks(Json::Value& out)
    {

    Statement stmt;
    Utf8CP sql = "SELECT ElementId, LinkId from dgn_ElementHasLinks";
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql)))
        {
        LogMessage(TeleporterLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve element has links instances");
        return ERROR;
        }

    auto& elemHasLinks = out.append(Json::ValueType::objectValue);
    elemHasLinks[JSON_TYPE_KEY] = JSON_TYPE_ElementHasLinks;
    elemHasLinks[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::arrayValue);
    elemHasLinks[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& groups = elemHasLinks[JSON_OBJECT_KEY];
    while (BE_SQLITE_ROW == stmt.Step())
        {
        Json::Value group(Json::ValueType::objectValue);
        group["ElementId"] = IdToString(stmt.GetValueId<DgnElementId>(0).GetValue()).c_str();
        group["LinkId"] = IdToString(stmt.GetValueId<DgnElementId>(1).GetValue()).c_str();
        groups.append(group);
        }
    (QueueJson) (elemHasLinks.toStyledString().c_str());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ExporterImpl::ExportV8Relationships(Json::Value& out)
    {
    Statement stmt;
    Utf8CP sql = "SELECT s.Name, c.Name, r.SourceECInstanceId, r.TargetECInstanceId FROM generic_ElementRefersToElement r, ec_Class c, ec_Schema s WHERE c.[Id] = r.ECClassId AND s.Id = c.[SchemaId]";
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql)))
        {
        LogMessage(TeleporterLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve generic relationship instances");
        return ERROR;
        }

    auto& elemRefersToElem = out.append(Json::ValueType::objectValue);
    elemRefersToElem[JSON_TYPE_KEY] = JSON_TYPE_ElementRefersToElement;
    elemRefersToElem[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::arrayValue);
    elemRefersToElem[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& relationships = elemRefersToElem[JSON_OBJECT_KEY];
    while (BE_SQLITE_ROW == stmt.Step())
        {
        Json::Value relationship(Json::ValueType::objectValue);
        relationship["Schema"] = stmt.GetValueText(0);
        relationship["Class"] = stmt.GetValueText(1);
        relationship["SourceId"] = IdToString(stmt.GetValueId<ECInstanceId>(2).GetValue()).c_str();
        relationship["TargetId"] = IdToString(stmt.GetValueId<ECInstanceId>(3).GetValue()).c_str();
        relationships.append(relationship);
        }
    (QueueJson)(elemRefersToElem.toStyledString().c_str());
    return SUCCESS;
    }

void BisJson1ExporterImpl::ReportProgress() const
    {
    if (m_meter && DgnProgressMeter::ABORT_Yes == m_meter->ShowProgress())
        {
        //OnFatalError();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void BisJson1ExporterImpl::SetStepName(ProgressMessage::StringId stringNum, ...) const
    {
    if (nullptr == m_meter)
        return;

    Utf8String fmt = ProgressMessage::GetString(stringNum);
    if (fmt.length() == 0)
        return;

    va_list args;
    va_start(args,stringNum);

    Utf8String value;
    value.VSprintf(fmt.c_str(), args);
    va_end(args);

    m_meter->SetCurrentStepName(value.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
void BisJson1ExporterImpl::MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, BeInt64Id id)
    {
    MakeNavigationProperty(out, propertyName, id.GetValue());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
void BisJson1ExporterImpl::MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, uint64_t id)
    {
    MakeNavigationProperty(out, propertyName, IdToString(id).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
void BisJson1ExporterImpl::MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, Utf8CP id)
    {
    auto& navObj = out[propertyName] = Json::Value(Json::ValueType::objectValue);
    navObj["id"] = id;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
void BisJson1ExporterImpl::MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, Json::Value& id)
    {
    auto& navObj = out[propertyName] = Json::Value(Json::ValueType::objectValue);
    navObj["id"] = id;
    }

END_BIM_EXPORTER_NAMESPACE