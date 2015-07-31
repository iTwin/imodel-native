/*----------------------------------------------------------------------+
|
|   $Source: DgnGeoCoord/ReprojectionSettings.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma unmanaged
#include    <DgnGeoCoord\DgnGeoCoord.h>
#include    <DgnPlatform\IGeoCoordServices.h>
#include    <DgnGeoCoord\DgnGeoCoordApi.h>
#include    <DgnPlatform\DgnCoreAPI.h>
#include    <DgnPlatform\Tools\fileutil.h>
#include    <DgnPlatform\ElementHandle.h>

#pragma managed
#include    <vcclr.h>
#include    <BentleyManagedUtil\StringInterop.h>
#using      <System.dll>
#using      <System.Drawing.dll>
#using      <System.Windows.Forms.dll>
#using      <Bentley.General.dll>
#using      <Bentley.UI.dll>
#using      <Bentley.Platform.dll>
#using      <Bentley.ECObjects.dll>
#using      <Bentley.GeoCoord2.dll>
#using      <Bentley.ECXAttributes.dll>

namespace SIO   = System::IO;
namespace SWF   = System::Windows::Forms;
namespace SD    = System::Drawing;
namespace ECO   = Bentley::ECObjects;
namespace ECS   = Bentley::ECObjects::Schema;
namespace ECI   = Bentley::ECObjects::Instance;
namespace ECUI  = Bentley::ECObjects::UI;
namespace ECXML = Bentley::ECObjects::XML;
namespace BGC   = Bentley::GeoCoordinates;
namespace BGCN  = Bentley::GeoCoordinatesNET;
namespace BUIA  = Bentley::UI::Attributes;
namespace ECXA  = Bentley::ECXAttributes;

// this makes it so we can use just String, rather than System::String all over the place.
using   System::String;

#pragma unmanaged
USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_NAMESPACE

namespace GeoCoordinates {

/*=================================================================================**//**
* This class contains the reprojection settings used while reprojecting a cache from reference GCS to master GCS.
* It is compiled only to native code.
* @bsiclass                                                     Barry.Bentley   05/07
+===============+===============+===============+===============+===============+======*/
class       ReprojectionSettings : public DgnPlatform::IGeoCoordinateReprojectionSettings
{
private:
double                  m_strokeTolerance;
ReprojectionOption      m_doCellElementsIndividually;
ReprojectionOption      m_doMultilineTextElementsIndividually;
bool                    m_scaleText;
bool                    m_rotateText;
bool                    m_scaleCells;
bool                    m_rotateCells;
ReprojectionOption      m_strokeArcs;
ReprojectionOption      m_strokeEllipses;
ReprojectionOption      m_strokeCurves;
bool                    m_postStrokeLinear;
bool                    m_reprojectElevation;

public:
ReprojectionSettings
(
)
    {
    m_strokeTolerance                       = 0.1;
    m_doCellElementsIndividually            = ReprojectionOptionIfLarge;
    m_doMultilineTextElementsIndividually   = ReprojectionOptionIfLarge;
    m_scaleText                             = true;
    m_rotateText                            = true;
    m_scaleCells                            = true;
    m_rotateCells                           = true;
    m_strokeArcs                            = ReprojectionOptionIfLarge;
    m_strokeEllipses                        = ReprojectionOptionIfLarge;
    m_strokeCurves                          = ReprojectionOptionIfLarge;
    m_postStrokeLinear                      = false;
    m_reprojectElevation                    = false;
    }

ReprojectionSettings
(
ReprojectionSettings const &   source
)
    {
    m_strokeTolerance                       = source.m_strokeTolerance;
    m_doCellElementsIndividually            = source.m_doCellElementsIndividually;
    m_doMultilineTextElementsIndividually   = source.m_doMultilineTextElementsIndividually;
    m_scaleText                             = source.m_scaleText;
    m_rotateText                            = source.m_rotateText;
    m_scaleCells                            = source.m_scaleCells;
    m_rotateCells                           = source.m_rotateCells;
    m_strokeArcs                            = source.m_strokeArcs;
    m_strokeEllipses                        = source.m_strokeEllipses;
    m_strokeCurves                          = source.m_strokeCurves;
    m_postStrokeLinear                      = source.m_postStrokeLinear;
    m_reprojectElevation                    = source.m_reprojectElevation;
    }

virtual double                  StrokeTolerance() override                                          { return m_strokeTolerance; }
virtual ReprojectionOption      DoCellElementsIndividually() override                               { return m_doCellElementsIndividually; }
virtual ReprojectionOption      DoMultilineTextElementsIndividually() override                      { return m_doMultilineTextElementsIndividually; }
virtual bool                    ScaleText() override                                                { return m_scaleText; }
virtual bool                    RotateText() override                                               { return m_rotateText; }
virtual bool                    ScaleCells() override                                               { return m_scaleCells; }
virtual bool                    RotateCells() override                                              { return m_rotateCells; }
virtual ReprojectionOption      StrokeArcs() override                                               { return m_strokeArcs; }
virtual ReprojectionOption      StrokeEllipses() override                                           { return m_strokeEllipses; }
virtual ReprojectionOption      StrokeCurves() override                                             { return m_strokeCurves; }
virtual bool                    PostStrokeLinear() override                                         { return m_postStrokeLinear; }
virtual bool                    ReprojectElevation() override                                       { return m_reprojectElevation; }

void                            SetStrokeTolerance (double strokeTolerance)                         { m_strokeTolerance = strokeTolerance; }
void                            SetDoCellElementsIndividually (ReprojectionOption state)            { m_doCellElementsIndividually = state; }
void                            SetDoMultilineTextElementsIndividually (ReprojectionOption state)   { m_doMultilineTextElementsIndividually = state; }
void                            SetScaleText (bool state)                                           { m_scaleText = state; }
void                            SetRotateText (bool state)                                          { m_rotateText = state; }
void                            SetScaleCells (bool state)                                          { m_scaleCells = state; }
void                            SetRotateCells (bool state)                                         { m_rotateCells = state; }
void                            SetStrokeArcs (ReprojectionOption state)                            { m_strokeArcs = state; }
void                            SetStrokeEllipses (ReprojectionOption state)                        { m_strokeEllipses = state; }
void                            SetStrokeCurves (ReprojectionOption state)                          { m_strokeCurves = state; }
void                            SetPostStrokeLinear (bool state)                                    { m_postStrokeLinear = state; }
void                            SetReprojectElevation (bool state)                                  { m_reprojectElevation = state; }
};

}   // End of GeoCoordinates Namespace

#pragma managed

namespace GeoCoordinatesNET {

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
public ref class    ReprojectionSettingsPanel : SWF::Panel
{
private:
ECUI::ECPropertyPane^       m_propertyPane;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
public:
ReprojectionSettingsPanel
(
ECI::ECInstanceListSet^     settings
)
    {
    Initialize (settings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionSettingsPanel
(
ECI::IECInstance^           settings
)
    {
    ECI::ECInstanceList^                list        = gcnew ECI::ECInstanceList();
    ECI::ECInstanceListSet^             listSet     = gcnew ECI::ECInstanceListSet();
    list->Add (settings);
    listSet->Add (list);
    Initialize (listSet);
    }

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            Initialize
(
ECI::ECInstanceListSet^     settings
)
    {
    m_propertyPane = gcnew ECUI::ECPropertyPane (nullptr, nullptr, nullptr, "RepSettings", false, false, false, nullptr, 1000);
    m_propertyPane->Dock = SWF::DockStyle::Fill;
    m_propertyPane->SetInstanceListSet (settings);
    SuspendLayout();
    Controls->Add (m_propertyPane);
    ResumeLayout();
    }

};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
[BUIA::DialogInformation (OpenKeyin = "NEEDSWORK_OPENKEYIN", CloseKeyin = "", 
                        EnglishTitle = "Reprojection Settings", FeatureTrackingId = "d35dbd04-3aa8-11e4-983b-6894231a2840",
                        SourceFile = "$Source: DgnGeoCoord/ReprojectionSettings.cpp $")]
#if defined (NEEDSWORK_DGNPLATFORM_GUI)
public ref class    ReprojectionSettingsForm  : BMW::Adapter
#else
public ref class    ReprojectionSettingsForm  : SWF::Form
#endif
{
private:

SWF::Button^                m_okButton;
SWF::Button^                m_cancelButton;
SWF::Panel^                 m_bottomPanel;
ReprojectionSettingsPanel^  m_reprojectionSettingsPanel;


public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionSettingsForm
(
ECI::ECInstanceListSet^     settings,
String^                     nameKey
)
    {
    InitializeComponent (settings, nameKey);
    
    BUIA::DialogInformation::FeatureTrackOpen (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            InitializeComponent
(
ECI::ECInstanceListSet^     settings,
String^                     nameKey
)
    {
    Name                        = nameKey;
    Text                        = GeoCoordinateLocalization::GetLocalizedString (nameKey);

    m_reprojectionSettingsPanel = gcnew ReprojectionSettingsPanel (settings);
    m_reprojectionSettingsPanel->Dock = SWF::DockStyle::Fill;

    System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager (ReprojectionSettingsForm::typeid));

    m_okButton                  = gcnew SWF::Button();
    m_cancelButton              = gcnew SWF::Button();

    resources->ApplyResources (m_okButton, "OkButton");
    m_okButton->Anchor          = SWF::AnchorStyles::Bottom | SWF::AnchorStyles::Right;
    m_okButton->DialogResult    = SWF::DialogResult::OK;
    m_okButton->TabIndex        = 10;
    m_okButton->Text            = GeoCoordinateLocalization::GetLocalizedString ("Ok");

    resources->ApplyResources (m_cancelButton, "CancelButton");
    m_cancelButton->Anchor       = SWF::AnchorStyles::Bottom | SWF::AnchorStyles::Right;
    m_cancelButton->DialogResult = SWF::DialogResult::Cancel;
    m_cancelButton->TabIndex    = 11;
    m_cancelButton->Text        = GeoCoordinateLocalization::GetLocalizedString ("Cancel");

    m_bottomPanel               = gcnew SWF::Panel();
    resources->ApplyResources (m_bottomPanel, "BottomPanel");
    m_bottomPanel->Dock         = SWF::DockStyle::Bottom;

    m_bottomPanel->SuspendLayout();
    m_bottomPanel->Controls->Add (m_okButton);
    m_bottomPanel->Controls->Add (m_cancelButton);
    m_bottomPanel->ResumeLayout();

    SuspendLayout();
    Controls->Add (m_reprojectionSettingsPanel);
    Controls->Add (m_bottomPanel);
    Size = SD::Size (400,310);

    ResumeLayout();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
SWF::DialogResult           ShowSettingsDialog
(
)
    {
    return ShowDialog();
    }

};


/*=================================================================================**//**
* Managed class called from ReprojectionSettingsManager.
* @bsiclass                                                     Barry.Bentley   05/07
+===============+===============+===============+===============+===============+======*/
public ref class   ReprojectionSettingsECObjects : ECS::IECSchemaLocater
{
private:
ECS::IECSchema^                                 m_schema;
ECS::IECClass^                                  m_ecClass;
ECI::IECInstance^                               m_defaultRefSettingsInstance;
ECI::IECInstance^                               m_defaultReprojectionSettingsInstance;

static      ReprojectionSettingsECObjects^      s_instance;

static const String^                            s_ReferenceConfigVar    = "MS_GEOCOORDINATE_REFSETTINGS";
static const String^                            s_ReferenceFileName     = "refReprojectionSettings.ecxml";
static const String^                            s_ReprojectionConfigVar = "MS_GEOCOORDINATE_REPROJECTIONSETTINGS";
static const String^                            s_ReprojectionFileName  = "reprojectSettings.ecxml";



static System::String^   SETTINGS_SCHEMA_NAME = "GeoReprojectSettings";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   04/07
+---------------+---------------+---------------+---------------+---------------+------*/
public:
static property ReprojectionSettingsECObjects^   Instance
    {
    ReprojectionSettingsECObjects^ get()
        {
        if (nullptr == s_instance)
            {
            s_instance = gcnew ReprojectionSettingsECObjects();
            ECO::ECObjects::AddSchemaLocater (s_instance);
            }

        return s_instance;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
private:
ECS::ECProperty^   CreateProperty
(
String^                                     propertyName,
ECS::IECType^                               propertyType,
int                                         priority,
ECI::IECInstance^                           displayAttribute
)
    {
    ECS::ECProperty^ ecProperty = gcnew ECS::ECProperty (propertyName, propertyType);
    ecProperty->DisplayLabel = GeoCoordinateLocalization::GetLocalizedString (propertyName);
    ECUI::ECPropertyPane::SetPriority (ecProperty, priority);
    if (nullptr != displayAttribute)
        ecProperty->SetCustomAttribute (displayAttribute);
    return ecProperty;
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
property ECS::IECClass^ ECClass
    {
    ECS::IECClass^ get ()
        {
        if (nullptr == m_ecClass)
            {
            // creating the schema creates m_ecClass as a byproduct.
            /* ECS::IECSchema^ schema =*/  Schema;
            }
        return m_ecClass;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
property ECS::IECSchema^ Schema
    {
    ECS::IECSchema^ get ()
        {
        if (nullptr == m_schema)
            {
            ECI::IECInstance^   category                = ECUI::ECPropertyPane::CreateCategory ("ReprojectionSettings", GeoCoordinateLocalization::GetLocalizedString("ReprojectionSettings"), nullptr, ECUI::ECPropertyPane::CategorySortPriorityMedium, true);
            ECI::IECInstance^   yesNoAttribute          = ECUI::ECPropertyPane::CreateBooleanDisplayStringsAttribute (GeoCoordinateLocalization::GetLocalizedString ("Yes"), GeoCoordinateLocalization::GetLocalizedString ("No"));

            array<int>^         optionValues            = { (int)ReprojectionOptionNever, (int)ReprojectionOptionAlways, (int)ReprojectionOptionIfLarge};
            array<String^>^     optionDisplay           = { GeoCoordinateLocalization::GetLocalizedString("Never"), GeoCoordinateLocalization::GetLocalizedString("Always"), GeoCoordinateLocalization::GetLocalizedString("IfLarge")};
            ECI::IECInstance^   reprojOptionAttribute   = ECUI::ECPropertyPane::CreateStandardIntValuesAttribute (true, optionDisplay, optionValues);

            m_schema = gcnew ECS::ECSchema (SETTINGS_SCHEMA_NAME, 1, 0, "GeoReproj");

            m_ecClass  = gcnew ECS::ECClass ("Settings");
            ECUI::ECPropertyPane::SetCategory (m_ecClass, category);

            ECUI::ECPropertyPane::SetCategory (m_ecClass, category);
            m_schema->AddClass (m_ecClass);

            ECS::IECProperty^   ecProperty;
            ecProperty = CreateProperty ("StrokeTolerance", ECO::ECObjects::DoubleType, 2000, nullptr);
            m_ecClass->Add (ecProperty);

            ecProperty = CreateProperty ("CellElements", ECO::ECObjects::IntegerType, 1900, reprojOptionAttribute);
            m_ecClass->Add (ecProperty);

            ecProperty = CreateProperty ("MultilineTextElements", ECO::ECObjects::IntegerType, 1700, reprojOptionAttribute);
            m_ecClass->Add (ecProperty);

            ecProperty = CreateProperty ("RotateCells", ECO::ECObjects::BooleanType, 1600, yesNoAttribute);
            m_ecClass->Add (ecProperty);

            ecProperty = CreateProperty ("ScaleCells", ECO::ECObjects::BooleanType, 1500, yesNoAttribute);
            m_ecClass->Add (ecProperty);

            ecProperty = CreateProperty ("RotateText", ECO::ECObjects::BooleanType, 1300, yesNoAttribute);
            m_ecClass->Add (ecProperty);

            ecProperty = CreateProperty ("ScaleText", ECO::ECObjects::BooleanType, 1200, yesNoAttribute);
            m_ecClass->Add (ecProperty);

            ecProperty = CreateProperty ("StrokeArcs", ECO::ECObjects::IntegerType, 1100, reprojOptionAttribute);
            m_ecClass->Add (ecProperty);

            ecProperty = CreateProperty ("StrokeEllipses", ECO::ECObjects::IntegerType, 1000, reprojOptionAttribute);
            m_ecClass->Add (ecProperty);

            ecProperty = CreateProperty ("StrokeCurves", ECO::ECObjects::IntegerType, 900, reprojOptionAttribute);
            m_ecClass->Add (ecProperty);

            ecProperty = CreateProperty ("PostStrokeLinear", ECO::ECObjects::BooleanType, 600, yesNoAttribute);
            m_ecClass->Add (ecProperty);

            ecProperty = CreateProperty ("ReprojectElevation", ECO::ECObjects::BooleanType, 800, yesNoAttribute);
            m_ecClass->Add (ecProperty);
            }

        return m_schema;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ECS::IECSchema^     LocateSchema
(
String^                 schemaName,
int                     versionMajor,
int                     versionMinor,
ECS::SchemaMatchType    matchType,
ECS::IECSchema^         parentSchema,
System::Object^         context
)
    {
    return schemaName->Equals (SETTINGS_SCHEMA_NAME) ? Schema : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int                 GetPriority
(
System::Object^         context
)
    {
    return 100;
    }

/*=================================================================================**//**
* ECXAttributes arg
* @bsiclass                                                     Barry.Bentley   05/07
+===============+===============+===============+===============+===============+======*/
ref class       ECXATraverseArg
{
public:
ECI::IECInstance^    m_foundInstance;

ECXATraverseArg() { m_foundInstance = nullptr; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool                            FindECXAttribute
(
ECI::IECInstance^               ecInstance,
System::IntPtr                  xAttrHandle,
System::Object^                 userObj
)
    {
    // if we find it, we can stop.
    if (ecInstance->ClassDefinition == m_ecClass)
        {
        ECXATraverseArg^ traverseArg = static_cast <ECXATraverseArg^>(userObj);
        traverseArg->m_foundInstance = ecInstance;
        UpgradeInstance (ecInstance);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
ECI::IECInstance^               GetReprojectionSettingsInstance
(
DgnModelRefP    modelRef
)
    {
    DgnModelRefP    parentModelRef;
    DgnModelP       cache;
    DgnAttachmentP  refP;

    // if it's a root modelRef we're reprojecting to, we want to use the Default Reprojection Settings.
    if ( (NULL == modelRef) || (NULL == (parentModelRef = modelRef->GetParentModelRefP())) )
        return GetDefaultReprojectionSettingsInstance();

    if ( (NULL == (cache = parentModelRef->GetDgnModelP())) || (NULL == (refP = modelRef->AsDgnAttachmentP())) )
        return nullptr;

    Bentley::DgnPlatform::ElementRefBase*  elemRef;
    if (NULL == (elemRef = cache->FindElementByID (refP->GetElementId())))
        return nullptr;

    ElementHandle  elemHandle (elemRef, parentModelRef);
    ReprojectionSettingsECObjects::ECXATraverseArg^ arg = gcnew ReprojectionSettingsECObjects::ECXATraverseArg();
    ECXA::ECXAttributes::Traverse (gcnew ECXA::ECXAttributes::TraverseDelegate (this, &ReprojectionSettingsECObjects::FindECXAttribute),
                                    elemHandle, XAttributeHandle::INVALID_XATTR_ID, nullptr, arg);
    return arg->m_foundInstance;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
internal:
ECI::IECInstance^               InstanceFromSettings
(
IGeoCoordinateReprojectionSettingsP  settings
)
    {
    ECI::IECInstance^ ecSettings = ReprojectionSettingsECObjects::Instance->ECClass->CreateInstance();
    ecSettings["StrokeTolerance"]->NativeValue      = settings->StrokeTolerance();
    ecSettings["CellElements"]->IntValue            = settings->DoCellElementsIndividually();
    ecSettings["MultilineTextElements"]->IntValue   = settings->DoMultilineTextElementsIndividually();
    ecSettings["ScaleText"]->NativeValue            = settings->ScaleText();
    ecSettings["RotateText"]->NativeValue           = settings->RotateText();
    ecSettings["ScaleCells"]->NativeValue           = settings->ScaleCells();
    ecSettings["RotateCells"]->NativeValue          = settings->RotateCells();
    ecSettings["StrokeArcs"]->IntValue              = settings->StrokeArcs();
    ecSettings["StrokeEllipses"]->IntValue          = settings->StrokeEllipses();
    ecSettings["StrokeCurves"]->IntValue            = settings->StrokeCurves();
    ecSettings["PostStrokeLinear"]->NativeValue     = settings->PostStrokeLinear();
    ecSettings["ReprojectElevation"]->NativeValue   = settings->ReprojectElevation();

    return ecSettings;
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
ECI::IECInstance^               GetDefaultReferenceSettingsInstance
(
)
    {
    if (nullptr == m_defaultRefSettingsInstance)
        {
        if (nullptr == (m_defaultRefSettingsInstance = ReadSettings (s_ReferenceConfigVar, s_ReferenceFileName)))
            {
            // create default.
            BGC::ReprojectionSettings *settings = new BGC::ReprojectionSettings();
            m_defaultRefSettingsInstance = InstanceFromSettings (settings);
            delete settings;
            }
        }

    return m_defaultRefSettingsInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                            SetDefaultReferenceSettingsInstance
(
ECI::IECInstance^   newSettings,
bool                saveToDisk
)
    {
    // ignore any attempt to set to null
    if (nullptr == newSettings)
        return;

    ECI::IECInstance^ oldSettings;
    if (nullptr != (oldSettings = GetDefaultReferenceSettingsInstance()))
        {
        // if no difference, don't bother to save.
        ECI::ECInstanceDifferences::DifferenceList^ differenceList = ECI::ECInstanceDifferences::GetInstanceDifferences (oldSettings, newSettings);
        if (differenceList->IsEmpty)
            return;
        }

    m_defaultRefSettingsInstance = newSettings;

    if (saveToDisk)
        WriteSettings (m_defaultRefSettingsInstance, s_ReferenceConfigVar, s_ReferenceFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
ECI::IECInstance^               GetDefaultReprojectionSettingsInstance
(
)
    {
    if (nullptr == m_defaultReprojectionSettingsInstance)
        {
        if (nullptr == (m_defaultReprojectionSettingsInstance = ReadSettings (s_ReprojectionConfigVar, s_ReprojectionFileName)))
            {
            // create default.
            BGC::ReprojectionSettings *settings = new BGC::ReprojectionSettings();
            m_defaultReprojectionSettingsInstance = InstanceFromSettings (settings);
            delete settings;
            }
        }

    return m_defaultReprojectionSettingsInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                            SetDefaultReprojectionSettingsInstance
(
ECI::IECInstance^   newSettings,
bool                saveToDisk
)
    {
    // ignore any attempt to set to null
    if (nullptr == newSettings)
        return;

    ECI::IECInstance^ oldSettings;
    if (nullptr != (oldSettings = GetDefaultReprojectionSettingsInstance()))
        {
        // if no difference, don't bother to save.
        ECI::ECInstanceDifferences::DifferenceList^ differenceList = ECI::ECInstanceDifferences::GetInstanceDifferences (oldSettings, newSettings);
        if (differenceList->IsEmpty)
            return;
        }

    m_defaultReprojectionSettingsInstance = newSettings;

    if (saveToDisk)
        WriteSettings (m_defaultReprojectionSettingsInstance, s_ReprojectionConfigVar, s_ReprojectionFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
SIO::Stream^                    OpenSettingsStream
(
const String^       configVarName,
const String^       defaultFileName,
bool                create
)
    {
    pin_ptr<const wchar_t>  configVar   = ::PtrToStringChars (configVarName);
    pin_ptr<const wchar_t>  defaultFile = ::PtrToStringChars (defaultFileName);
    BeFileName              foundFile;
    int                     option = create ? UF_OPEN_CREATE | UF_NO_CUR_DIR : UF_NO_CUR_DIR;

    if (SUCCESS == util_findFile (NULL, &foundFile, NULL, configVar, defaultFile, option))
        {
        String^                 foundFileString = gcnew String (foundFile.GetName ());

        try
            {
            SIO::FileMode    fileMode   = create ? SIO::FileMode::Create  : SIO::FileMode::Open;
            SIO::FileAccess  fileAccess = create ? SIO::FileAccess::Write : SIO::FileAccess::Read;
            SIO::Stream^     fileStream = gcnew SIO::FileStream (foundFileString, fileMode, fileAccess);
            return fileStream;
            }
        catch (SIO::IOException^)
            {
            }
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
void                            UpgradeInstance
(
ECI::IECInstance^   ecSettings
)
    {
    // make sure all of the properties are set. We added ReprojectElevation after the others.
    ECI::IECPropertyValue^  propVal;
    if ( (nullptr == (propVal = ecSettings->GetPropertyValue ("ReprojectElevation"))) || propVal->IsNull )
        ecSettings["ReprojectElevation"]->IntValue = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
ECI::IECInstance^               ReadSettings
(
const String^       configVarName,
const String^       defaultFileName
)
    {
    SIO::Stream^ fileStream;
    if (nullptr != (fileStream = OpenSettingsStream (configVarName, defaultFileName, false)))
        {
        ECS::IECSchema^     settingsSchema = ReprojectionSettingsECObjects::Instance->Schema;
        ECI::IECInstance^   ecSettings = nullptr;
        try
            {
            ECI::ECInstanceBinaryReader^  reader   = gcnew ECI::ECInstanceBinaryReader();
            ecSettings = reader->DeserializeInstance (fileStream, settingsSchema);
            UpgradeInstance (ecSettings);
            }
        finally
            {
            fileStream->Close();
            }
        return ecSettings;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                            WriteSettings
(
ECI::IECInstance^   instance,
const String^       configVarName,
const String^       defaultFileName
)
    {
    SIO::Stream^ fileStream;
    if (nullptr != (fileStream = OpenSettingsStream (configVarName, defaultFileName, true)))
        {
        try
            {
            ECI::ECInstanceBinaryWriter^  writer   = gcnew ECI::ECInstanceBinaryWriter ();
            writer->SerializeInstance (fileStream, instance);
            }
        finally
            {
            fileStream->Close();
            }
        }
    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt                       PutSettingsOnAttachment
(
DgnModelRefP        modelRef,
ECI::IECInstance^   instance
)
    {
    DgnModelRefP    parentModelRef;
    DgnModelP       cache;
    DgnAttachmentP  refP;

    if ( (NULL == modelRef) || (NULL == (parentModelRef = modelRef->GetParentModelRefP())) || (NULL == (cache = parentModelRef->GetDgnModelP())) || (NULL == (refP = modelRef->AsDgnAttachmentP())) )
        return ERROR;

    Bentley::DgnPlatform::ElementRefBase*  elemRef;
    if (NULL == (elemRef = cache->FindElementByID (refP->GetElementId())))
        return ERROR;

    ECXA::ECXAttributes::ReplaceInstance (elemRef, instance, true, false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool                            HasDifferences
(
DgnModelRefP            modelRef,
ECI::IECInstance^   newSettings
)
    {
    ECI::IECInstance^   oldSettings;
    if (nullptr == (oldSettings = GetReprojectionSettingsInstance (modelRef)))
        return true;

    // there's really only one instance in the list, but need it for the comparison.
    ECI::ECInstanceDifferences::DifferenceList^ differenceList = ECI::ECInstanceDifferences::GetInstanceDifferences (oldSettings, newSettings);
    return (!differenceList->IsEmpty);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
internal:
StatusInt                       EditRefReprojectionSettings
(
DgnModelRefListP    modelRefList
)
    {
    if (NULL == modelRefList)
        return ERROR;

    int                                 refCount     = (int) modelRefList->size();
    ECI::ECInstanceListSet^             listSet      = gcnew ECI::ECInstanceListSet();
    ECI::IECInstance^                   defaultClone = static_cast <ECI::IECInstance^>(GetDefaultReferenceSettingsInstance()->Clone());
    IGeoCoordinateServicesP             gcsServices  = GeoCoordinationManager::GetServices();

    for (int iRef=0; iRef < refCount; iRef++)
        {
        // include only those references that we will reproject.
        DgnModelRefP        modelRef = modelRefList->at(iRef);
        ECI::IECInstance^   ecSettings;

        if (!gcsServices->WillReprojectToParent (modelRef))
            continue;

        if (nullptr == (ecSettings = GetReprojectionSettingsInstance (modelRef)))
            ecSettings = defaultClone;

        ECI::ECInstanceList^ list = gcnew ECI::ECInstanceList();
        list->Add (ecSettings);
        listSet->Add (list);
        }

    // have an instancelistset
    if (0 == listSet->Count)
        return ERROR;

    ReprojectionSettingsForm^   form    = gcnew ReprojectionSettingsForm (listSet, "RefReprojectionSettings");
    SWF::DialogResult           result  = form->ShowSettingsDialog();
    delete form;

    if (SWF::DialogResult::OK != result)
        return SUCCESS;

    // now we have to put the settings back into the DgnModels.
    for (int iRef=0, iIndex=0; iRef < refCount; iRef++)
        {
        DgnModelRefP            modelRef = modelRefList->at(iRef);

        if (!gcsServices->WillReprojectToParent (modelRef))
            continue;

        // iIndex keeps track of the index into the listSet, since some modelRefs won't have a corresponding entry in the list.
        ECI::ECInstanceList^    list     = listSet[iIndex++];
        ECI::IECInstance^       instance = list[0];

        if ( (instance == defaultClone) || HasDifferences (modelRef, instance) )
            PutSettingsOnAttachment (modelRef, instance);
        }

    return SUCCESS;
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt                       SaveDefaultSettingsToReference
(
DgnModelRefP    modelRef
)
    {
    return PutSettingsOnAttachment (modelRef, GetDefaultReferenceSettingsInstance());
    }

};

}   // End of GeoCoordinatesNET Namespace



namespace GeoCoordinates {

class   ReprojectionSettingsManager
{
private:
_int64                                                  m_defaultRefSettingsTime;
DgnPlatform::IGeoCoordinateReprojectionSettingsP        m_defaultRefSettings;

static  ReprojectionSettingsManager*                    s_instance;


public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
static ReprojectionSettingsManager* GetInstance ()
    {
    if (NULL == s_instance)
        s_instance = new ReprojectionSettingsManager();
    return s_instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
IGeoCoordinateReprojectionSettingsP     GetReprojectionSettings (DgnModelRefP  modelRef)
    {
    ECI::IECInstance^   ecSettings;
    if (nullptr != (ecSettings = BGCN::ReprojectionSettingsECObjects::Instance->GetReprojectionSettingsInstance (modelRef)))
        {
        IGeoCoordinateReprojectionSettingsP settings;
        if (NULL != (settings = SettingsFromInstance (ecSettings)))
            return settings;
        }

    // if we didn't find any, get a copy of the default reference settings.
    return new ReprojectionSettings (*(ReprojectionSettings*)GetDefaultReferenceSettings());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
IGeoCoordinateReprojectionSettingsP     SettingsFromInstance (ECI::IECInstance^ ecSettings)
    {
    ReprojectionSettings*   settings = new BGC::ReprojectionSettings();

    try
        {
        settings->SetStrokeTolerance                     ((double)ecSettings["StrokeTolerance"]->NativeValue);
        settings->SetDoCellElementsIndividually          ((ReprojectionOption)ecSettings["CellElements"]->IntValue);
        settings->SetDoMultilineTextElementsIndividually ((ReprojectionOption)ecSettings["MultilineTextElements"]->IntValue);
        settings->SetScaleText                           (0 != ecSettings["ScaleText"]->IntValue);
        settings->SetRotateText                          (0 != ecSettings["RotateText"]->IntValue);
        settings->SetScaleCells                          (0 != ecSettings["ScaleCells"]->IntValue);
        settings->SetRotateCells                         (0 != ecSettings["RotateCells"]->IntValue);
        settings->SetStrokeArcs                          ((ReprojectionOption)ecSettings["StrokeArcs"]->IntValue);
        settings->SetStrokeEllipses                      ((ReprojectionOption)ecSettings["StrokeEllipses"]->IntValue);
        settings->SetStrokeCurves                        ((ReprojectionOption)ecSettings["StrokeCurves"]->IntValue);
        settings->SetPostStrokeLinear                    (0 != ecSettings["PostStrokeLinear"]->IntValue);
        settings->SetReprojectElevation                  (0 != ecSettings["ReprojectElevation"]->IntValue);
        }
    catch (System::Exception^)
        {
        // give up on any exception.
        return NULL;
        }

    return settings;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt                               EditRefReprojectionSettings (DgnModelRefListP modelRefList)
    {
    return BGCN::ReprojectionSettingsECObjects::Instance->EditRefReprojectionSettings (modelRefList);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt                               SaveDefaultSettingsToReference (DgnModelRefP modelRef)
    {
    return BGCN::ReprojectionSettingsECObjects::Instance->SaveDefaultSettingsToReference (modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
IGeoCoordinateReprojectionSettingsP     GetDefaultReferenceSettings ()
    {
    // get rid of any settings structure older than 20 seconds and get a new one.
    System::TimeSpan    settingsAge (System::DateTime::Now.Ticks - m_defaultRefSettingsTime);
    if (settingsAge.TotalMilliseconds > 20000)
        {
        if (NULL != m_defaultRefSettings)
            delete m_defaultRefSettings;

        m_defaultRefSettings = NULL;
        }

    if (NULL == m_defaultRefSettings)
        {
        ECI::IECInstance^    ecSettings;
        if (nullptr != (ecSettings = BGCN::ReprojectionSettingsECObjects::Instance->GetDefaultReferenceSettingsInstance()))
            m_defaultRefSettings = SettingsFromInstance (ecSettings);
        else
            m_defaultRefSettings = new ReprojectionSettings();

        m_defaultRefSettingsTime = System::DateTime::Now.Ticks;
        }

    return m_defaultRefSettings;
    }
};

/*=================================================================================**//**
* ReprojectionSettingsManager methods
* @bsiclass                                                     Barry.Bentley   05/07
+===============+===============+===============+===============+===============+======*/
ReprojectionSettingsManager*    ReprojectionSettingsManager::s_instance;



}   // End of GeoCoordinates Namespace

END_BENTLEY_NAMESPACE

#pragma unmanaged
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORDMANAGED_EXPORTED DgnPlatform::IGeoCoordinateReprojectionSettingsP dgnGeoCoord_getRefReprojectionSettings (DgnModelRefP modelRef)
    {
    return Bentley::GeoCoordinates::ReprojectionSettingsManager::GetInstance()->GetReprojectionSettings (modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORDMANAGED_EXPORTED StatusInt       dgnGeoCoord_editRefReprojectionSettings
(
DgnModelRefListP modelRefList
)
    {
    return Bentley::GeoCoordinates::ReprojectionSettingsManager::GetInstance()->EditRefReprojectionSettings (modelRefList);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORDMANAGED_EXPORTED StatusInt       dgnGeoCoord_saveDefaultRefReprojectionSettings
(
DgnModelRefP        modelRef
)
    {
    return Bentley::GeoCoordinates::ReprojectionSettingsManager::GetInstance()->SaveDefaultSettingsToReference (modelRef);
    }


