/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/DTMAddIn.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "stdafx.h"
#include    <vcclr.h>
#include    "DTMAddIn.h"
#include    "LandXMLImportForm.h"
#include    "dtmcommandstool.h"
#include "commandsdefs.h"
//#include <BentleyManagedUtil\StringInterop.h>

using namespace System::Xml;

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT;

void startAnnotateContoursCommand (CommandNumber cmdNumber, int cmdName) ;
void startAnnotateSpotsCommand (CommandNumber cmdNumber, int cmdName) ;
void StartMainAnnotateSpotsCommand (ElementHandleCR dtm, ::UInt64 cmdNumer);
void StartMainAnnotateContoursCommand (ElementHandleCR dtm, ::UInt64 cmdNumer);
void hook_labelContoursTextHeightWidth (DialogItemMessage* dimP);

void init();

#define MUST_REFERENCE_MATCHING_TEMPLATE (TRUE)
#define INCLUDE_REFS    (true)

extern void EngageWordlib (void);

BEGIN_BENTLEY_TERRAINMODEL_COMMANDS_NAMESPACE

/// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
struct FindDTMHelper
{
private: ElementHandle m_dtm;
private: WCharCP       m_name;
private: WString       m_tempName;

private: static StatusInt TraverseCB (ElementHandleCR eh, void *userArgs)
        {
        FindDTMHelper *caller = reinterpret_cast<FindDTMHelper*>(userArgs);
        if (SUCCESS == DTMElementHandlerManager::GetName (eh, caller->m_tempName) && caller->m_tempName == caller->m_name)
            {
            BeAssert (!caller->m_dtm.IsValid ());
            caller->m_dtm = eh;
            return ERROR;
            }
        return SUCCESS;
        }

private: FindDTMHelper (wchar_t const name[]) : m_name (name)
        {
        }

public: static ElementHandle Find (wchar_t const name[], DgnModelRefP modelRef, bool includeRefs)
        {
        FindDTMHelper fh (name);
        DTMElementHandlerManager::Traverse (modelRef, includeRefs, TraverseCB, &fh);
        return fh.m_dtm;
        }
}; // End FindDTMHelper struct

AddIn::AddIn(System::IntPtr mdlDescIn) : Bentley::MstnPlatformNET::AddIn(mdlDescIn)
    {
    m_mdlDesc = mdlDescIn;
    s_app = this;
//ToDo    init();
    }

int AddIn::Run(array<System::String^>^ commandLine)
    {
//    mdlSystem_setMdlAppClass ( NULL, APPLICATION_MSREQD );
    return 0;
    }

void AddIn::AnnotateContours (WCharCP unparsed)
    {
    if (!WString::IsNullOrEmpty (unparsed))
        {
        ElementHandle dtm = FindDTMHelper::Find (unparsed, ACTIVEMODEL, INCLUDE_REFS);
        if (dtm.IsValid ())
            {
            StartMainAnnotateContoursCommand (dtm, CMD_TERRAINMODEL_LABEL_CONTOURS);
            return;
            }
        }
    startAnnotateContoursCommand (CMD_TERRAINMODEL_LABEL_CONTOURS, CMDNAME_LabelContours);
    }

void AddIn::AnnotateSpots (WCharCP unparsed)
    {
    EngageWordlib ();
    if (!WString::IsNullOrEmpty (unparsed))
        {
        ElementHandle dtm = FindDTMHelper::Find (unparsed, ACTIVEMODEL, INCLUDE_REFS);
        if (dtm.IsValid ())
            {
            StartMainAnnotateSpotsCommand (dtm, CMD_TERRAINMODEL_LABEL_SPOTS);
            return;
            }
        }
    startAnnotateSpotsCommand (CMD_TERRAINMODEL_LABEL_SPOTS, CMDNAME_LabelSpot);
    }

/// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
inline StatusInt mdlElementTemplate_AttachAndApplyParams (EditElementHandleR el, ElementTemplateNodeR elmTemplate)
    {
    if (MstnElementTemplateMgr::ApplyTemplateDefaultsToElement (el, elmTemplate, true))
        return SUCCESS;
    return ERROR;
    }

#ifdef todo
class LandXMLImporter
    {
    private:
        WString m_fileName;
/*
        gcroot<System::Collections::Generic::List<System::String^>^> m_surfaces;
        gcroot<System::Collections::Generic::Dictionary<System::String^,System::String^>^>  m_surfaceNames; 
        gcroot<System::Collections::Generic::Dictionary<System::String^,System::String^>^> m_elementTemplates;
        gcroot<System::String^> m_fileName;
        static LandXMLImporter* s_instance;
        gcroot<System::String^> m_subUnit;
        bool m_isMetric;
*/

    public: LandXMLImporter (WCharCP fileName)
                {
                m_fileName = fileName;
                }

/*
        public: System::Collections::Generic::List<System::String^>^ ListSurfaces()
                {
                m_surfaces = gcnew System::Collections::Generic::List<System::String^>();

                // if the file does not exist, assume were using DTM Export
                if (System::IO::File::Exists (m_fileName))
                    {
                    bool isInUnits = false;
                    bool isInSurfaces = false;
                    System::Xml::XmlReader^ reader = System::Xml::XmlReader::Create (m_fileName);

                    try {
                    // Parse the file and display each of the nodes.
                    while (reader->Read ())
                        {
                        switch (reader->NodeType)
                            {
                            case System::Xml::XmlNodeType::Element:
                                if (isInUnits)
                                    {
                                    if (reader->Name == "Imperial")
                                        {
                                        m_subUnit = reader->GetAttribute ("linearUnit");
                                        }
                                    else if (reader->Name == "Metric")
                                        {
                                        m_subUnit = reader->GetAttribute ("linearUnit");
                                        }
                                    }
                                else if (isInSurfaces)
                                    {
                                    if (reader->Name == "Surface")
                                        m_surfaces->Add(reader->GetAttribute ("name"));
                                    }
                                else if (reader->Name == "Units")
                                    isInUnits = true;
                                else if (reader->Name == "Surfaces")
                                    isInSurfaces = true;
                                break;
                            case System::Xml::XmlNodeType::EndElement:
                                if (reader->Name == "Surfaces")
                                    isInSurfaces = false;
                                else if (reader->Name == "Units")
                                    isInUnits = false;
                                break;
                            }
                        }
                    } // try
                    catch (System::Exception^ ex)
                        {
                        System::Diagnostics::Debug::WriteLine (ex->Message);
                        }
                    delete reader;
                    }

                return m_surfaces;
                }
*/
    public: void ImportSurfaces ( array<LandXMLImportForm::SurfaceData^>^ surfaces )
                {
                s_instance = this;
                mdlUndo_startGroup ();
                int ret = 0;
                m_surfaces = gcnew System::Collections::Generic::List<System::String^>();
                m_elementTemplates = gcnew System::Collections::Generic::Dictionary<System::String^,System::String^> ();
                m_surfaceNames = gcnew System::Collections::Generic::Dictionary<System::String^,System::String^> ();
                for each ( LandXMLImportForm::SurfaceData^ surface in surfaces)
                    {
                    m_surfaces->Add ( surface->OriginalSurface );
                    m_surfaceNames->Add( surface->OriginalSurface, surface->Surface );
                    m_elementTemplates->Add ( surface->OriginalSurface, surface->ElementTemplate );
                    }

                ret = bcDTMLandXml_browseLandXmlFileForSurfaces ( fileNameSS.Ansi (), nullptr, &LandXMLImporter::addDTMToModelCallback, &LandXMLImporter::browsingTestSurfacesCallback );
                mdlUndo_endGroup ();
                s_instance = nullptr;
                }

    private: void AddSurface (System::String^ surfaceName)
                 {
                 m_surfaces->Add (surfaceName);
                 }

    private: bool ImportSurface (System::String^ surfaceName)
                 {
                 return m_surfaces->Contains(surfaceName);
                 }

    private: static bool browsingGetSurfacesCallback (char* surfaceName, void* userArg)
                 {
                 s_instance->AddSurface (gcnew System::String(surfaceName));
                 return false;
                 }

    private: static bool browsingTestSurfacesCallback (char* surfaceName, void* userArg)
                 {
                 return s_instance->ImportSurface (gcnew System::String(surfaceName));
                 return false;
                 }

    private: static int addDTMToModelCallback(BcDTMP bcDtm, char* surfaceName, void* userArg)
                 {
                 try
                     {
                     return s_instance->addDTMToModel (bcDtm, surfaceName);
                     }
                 catch(...)
                     {
                     char msg[1024], format[1024];

                     if (SUCCESS != mdlResource_loadFromStringList (format, NULL, STRINGID_Message_Main, RESULT_ImportFailedDTM))
                         strcpy_s (format, "Failed to import Terrain model %s");
                     sprintf_s (msg, format, surfaceName);
                     mdlOutput_messageCenter (OutputMessagePriority::Error, msg, NULL, FALSE);
                     }
                 return DTM_ERROR;
                 }

    private: int addDTMToModel (BcDTMP bcDtm, char* surfaceName )
                 {
                 EditElementHandle refEl, el;
                 String^ tempName;

                 if (!m_surfaceNames->TryGetValue(gcnew String(surfaceName), tempName))
                    tempName = gcnew String(surfaceName);

                 if (bcDtm->triangulate() != DTM_SUCCESS)
                     {
                     char msg[1024], format[1024];

                     if (SUCCESS != mdlResource_loadFromStringList (format, NULL, STRINGID_Message_Main, RESULT_ImportFailedDTM))
                         strcpy_s (format, "Failed to import Terrain model %s");
                     sprintf_s (msg, format, surfaceName);
                     mdlOutput_messageCenter (OutputMessagePriority::Error, msg, NULL, FALSE);
                     return DTM_SUCCESS;
                     }

                 el.SetModelRef(ACTIVEMODEL);
                 refEl.SetModelRef(ACTIVEMODEL);

                 // We need to work out the units transformation, first get UORs to meters.
                 double uorPerMeter = mdlModelRef_getUorPerMeter (ACTIVEMODEL);

                 double uorPerUnit = uorPerMeter;

                 // Now convert from meters to the DTM coordinate system.
                 if (Millimeter)
                     {
                     uorPerUnit *= 0.001;
                     }
                 else if (Centimeter)
                     {
                     uorPerUnit *= 0.01;
                     }
                 else if (Meter)
                     {
                     uorPerUnit *= 1;
                     }
                 else if (Kilometer ==  (System::String^)m_subUnit)
                     {
                     uorPerUnit *= 1000;
                     }
                 else if (Foot)
                     {
                     uorPerUnit *= 0.3048;
                     }
                 else if (USSurveyFoot)
                     {
                     uorPerUnit *= 0.30480061;
                     }
                 else if (Inch)
                     {
                     uorPerUnit *= 0.0254;
                     }
                 else if (Mile)
                     {
                     uorPerUnit *= 1609.347218694;
                     }
                 else
                     {
                     // Unknown.
                     }

                 DPoint3d ptGO;
                 mdlModelRef_getGlobalOrigin (ACTIVEMODEL, &ptGO);
                 mdlCurrTrans_begin ();
                 mdlCurrTrans_identity ();

                 mdlCurrTrans_scale (uorPerUnit, uorPerUnit, uorPerUnit);
                 mdlCurrTrans_translateOriginWorld (&ptGO);

                 StatusInt status = Bentley::TerrainModel::Element::DTMElementHandlerManager::ScheduleFromDtm(el, bcDtm->GetIDTM(), true);
                 mdlCurrTrans_end ();

                 if (status == SUCCESS)
                     {
                     el.AddToModel (el.GetModelRef());

                     pin_ptr<const wchar_t> p = PtrToStringChars (tempName);
                     Bentley::TerrainModel::Element::DTMElementHandlerManager::SetName (el.GetElementRef(), WString(p));

                     System::Collections::Generic::Dictionary<System::String^,System::String^> ^elementTemplates = m_elementTemplates;
                     System::String ^surfName = gcnew String (surfaceName);
                     System::String ^elmTemplateName = nullptr;
                     if (nullptr != elementTemplates && elementTemplates->TryGetValue (surfName, elmTemplateName))
                         {
                         if (elmTemplateName == GET_LOCALIZED ("ITEM_None"))
                             {
                             ElementPropertyUtils::ApplyActiveSettings (element);
                             }
                         else if (elmTemplateName == GET_LOCALIZED ("ITEM_Active"))
                             {
                             ElementTemplateP elmTemplate = nullptr;

                             if (SUCCESS == mdlElementTemplate_loadActive (&elmTemplate))
                                 {
                                 WChar path[512] = {0}; 

                                 ASSERT (NULL != elmTemplate && "Expected Active Template");
                                 mdlElementTemplate_getPath (elmTemplate, path, _countof (path));
                                 mdlElementTemplate_free (&elmTemplate);

                                 if (SUCCESS == mdlElementTemplate_getExistingTemplateInFile(&elmTemplate, path, mdlDgnFileObj_getMasterFile()))
                                     {
                                     mdlElementTemplate_AttachAndApplyParams (el, elmTemplate);
                                     }
                                 mdlElementTemplate_free (&elmTemplate);
                                 }
                             }
                         else
                             {
                             pin_ptr<wchar_t const> elmTemplatePath = PtrToStringChars (elmTemplateName);
                             ElementTemplateP elmTemplate = nullptr;
                            
                             if (SUCCESS == mdlElementTemplate_loadByPath (&elmTemplate, elmTemplatePath))
                                 {
                                 mdlElementTemplate_free (&elmTemplate);
                                 if (SUCCESS == mdlElementTemplate_getExistingTemplateInFile (&elmTemplate, elmTemplatePath, mdlDgnFileObj_getMasterFile()))
                                     {
                                     mdlElementTemplate_AttachAndApplyParams (el, elmTemplate);
                                     }
                                 mdlElementTemplate_free (&elmTemplate);
                                 }
                             }
                         }
                     else
                         {
                         ElementTemplateP elmTemplate = nullptr;

                         mdlTemplateManager_attachToElement (el.GetElementRef());
                         if (SUCCESS == mdlElementTemplate_loadFromElement (&elmTemplate, el.GetElementRef(), el.GetModelRef()))
                             {
                             if (elmTemplate)
                                 {
                                 mdlElementTemplate_applyElementParams (elmTemplate, el.GetElementRef(), el.GetModelRef(), MUST_REFERENCE_MATCHING_TEMPLATE);
                                 mdlElementTemplate_free (&elmTemplate);
                                 }
                             }
                         }
                     char msg[1024], format[1024];

                     if (SUCCESS != mdlResource_loadFromStringList (format, NULL, STRINGID_Message_Main, RESULT_ImportDTM))
                         strcpy_s (format, "Terrain model %s imported successfully");
                     sprintf_s (msg, format, surfaceName);
                     mdlOutput_messageCenter (OutputMessagePriority::Info, msg, NULL, FALSE);
                     return SUCCESS;
                     }
                 else
                     {
                     char msg[1024], format[1024];

                     if (SUCCESS != mdlResource_loadFromStringList (format, NULL, STRINGID_Message_Main, RESULT_ImportFailedDTM))
                         strcpy_s (format, "Failed to import Terrain model %s");
                     sprintf_s (msg, format, surfaceName);
                     mdlOutput_messageCenter (OutputMessagePriority::Error, msg, NULL, FALSE);
                     return ERROR;
                     }
                 }
    };

LandXMLImporter* LandXMLImporter::s_instance = nullptr;

#undef WIDE
#undef __WIDE
#define __WIDE(txt) L##txt
#define WIDE(txt) __WIDE(txt)

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void SetCurrentCommandName ( ::UInt32 msgList, ::UInt32 msgId, wchar_t const whenNotFound[], RscFileHandle rscf = 0 )
    {
    wchar_t cmd[1024] = WIDE("");

    if ( SUCCESS == mdlResource_loadFromStringListW ( cmd, rscf, msgList, msgId ) )
        mdlState_setCurrentCommandName ( cmd );
    else
        mdlState_setCurrentCommandName ( whenNotFound );
    }
#endif

/// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
private ref class LandXMLLoggedImport
{
private: System::String ^m_fileName;
private: System::Collections::Generic::List<LandXMLImportForm::SurfaceData^> ^m_surfaces;
private: LandXMLLoggedImport (void) : m_fileName (nullptr), m_surfaces (nullptr)
        {}

public: static LandXMLLoggedImport^ CreateParsed (System::String ^inputFile)
        {
        try
            {
            LandXMLLoggedImport ^o = gcnew LandXMLLoggedImport ();
            XmlTextReader reader (inputFile);
            reader.ReadToFollowing ("LandXMLFile");
            reader.MoveToFirstAttribute ();
            o->m_fileName = reader.GetAttribute ("Path");
            while (reader.ReadToFollowing ("Surface"))
                {
                reader.MoveToFirstAttribute ();
                System::String ^surface = reader.GetAttribute ("Name");
                if (nullptr == surface)
                    continue;
                System::String ^elementTemplate = reader.GetAttribute ("ElementTemplate");
                if (nullptr == elementTemplate)
                    elementTemplate = GET_LOCALIZED ("ITEM_None");
                if (nullptr == o->m_surfaces)
                    o->m_surfaces = gcnew System::Collections::Generic::List<LandXMLImportForm::SurfaceData^> ();
                o->m_surfaces->Add (gcnew LandXMLImportForm::SurfaceData (surface, surface, elementTemplate));
                }
            return o;
            }
        catch (Exception ^)
            {
            return nullptr;
            }
        }

public: property array<LandXMLImportForm::SurfaceData^>^ Surfaces
        {
        array<LandXMLImportForm::SurfaceData^>^ get ( void ) { return nullptr == m_surfaces ? nullptr : m_surfaces->ToArray (); }
        }

public: property System::String^ FileName
        {
        System::String^ get ( void ) { return m_fileName; }
        }

}; // End LandXMLLoggedImport class


void ImportSurfaces (TerrainImporter& reader, array<LandXMLImportForm::SurfaceData^>^ surfaceDatas, bool triangulate)
    {
    bvector<WString> surfaceNames;

    for each (LandXMLImportForm::SurfaceData^ data in surfaceDatas)
        {
        pin_ptr<const wchar_t> dtmName = PtrToStringChars (data->OriginalSurface);
        surfaceNames.push_back (dtmName);
        }

    ImportedTerrainList surfaces = reader.ImportTerrains (surfaceNames);

    for each (LandXMLImportForm::SurfaceData^ data in surfaceDatas)
        {
        bool imported = false;
        BcDTMPtr dtm;

        WString dtmName;
        for (ImportedTerrainList::const_iterator it = surfaces.begin(); it != surfaces.end(); it++)
            {
            pin_ptr<const wchar_t> wDtmName = PtrToStringChars (data->OriginalSurface);
            if (it->GetName() == (WCharCP)wDtmName)
                {
                dtmName = wDtmName;
                dtm = it->GetTerrain ();
                break;
                }
            }
        if (dtm.IsValid())
            {
            if (!triangulate || dtm->Triangulate() == DTM_SUCCESS)
                {
                EditElementHandle element;
                element.SetModelRef (mdlModelRef_getActive());

                // We need to work out the units transformation, first get UORs to meters.
                double uorPerMeter = dgnModel_getUorPerMeter (mdlModelRef_getActive()->GetDgnModelP());

                double uorPerUnit = uorPerMeter;
                FileUnit linearUnit = reader.GetFileUnit();
                // Now convert from meters to the DTM coordinate system.
                switch (linearUnit)
                    {
                    case FileUnit::Millimeter:
                        uorPerUnit *= 0.001;
                        break;
                    case FileUnit::Centimeter:
                        uorPerUnit *= 0.01;
                        break;
                    case FileUnit::Meter:
                        uorPerUnit *= 1;
                        break;
                    case FileUnit::Kilometer:
                        uorPerUnit *= 1000;
                        break;
                    case FileUnit::Foot:
                        uorPerUnit *= 0.3048;
                        break;
                    case FileUnit::USSurveyFoot:
                        uorPerUnit *= 0.30480061;
                        break;
                    case FileUnit::Inch:
                        uorPerUnit *= 0.0254;
                        break;
                    case FileUnit::Mile:
                        uorPerUnit *= 1609.347218694;
                        break;
                    default:
                        // unknown;
                        break;
                    }
                DPoint3d ptGO;
                dgnModel_getGlobalOrigin (element.GetDgnModelP(), &ptGO);

                Transform trf;
                trf.InitIdentity ();
                trf.setTranslation (&ptGO);
                trf.ScaleMatrixColumns (uorPerUnit, uorPerUnit, uorPerUnit);

                if (Bentley::TerrainModel::Element::DTMElementHandlerManager::ScheduleFromDtm (element, nullptr, *dtm, trf, *element.GetModelRef(), true) == SUCCESS)
                    {
                    Bentley::MstnPlatform::Element::ElementPropertyUtils::ApplyActiveSettings (element);
                    if (data->ElementTemplate == GET_LOCALIZED ("ITEM_None"))
                        {
                        }
                    else
                        {
                        pin_ptr<const wchar_t> p = nullptr;
                        
                        if (data->ElementTemplate != GET_LOCALIZED ("ITEM_Active"))
                            p = PtrToStringChars (data->ElementTemplate);
                        ElementTemplateNodePtr elementTemplate;
                        MstnElementTemplateMgr::CreateFromExisting (elementTemplate, p, element.GetDgnFileP(), true, nullptr);

                        if (elementTemplate.IsValid())
                            MstnElementTemplateMgr::ApplyTemplateDefaultsToElement (element, *elementTemplate.get(), true);
                        }
                    if (element.AddToModel () == SUCCESS)
                        {
                        imported = true;
                        pin_ptr<const wchar_t> p = PtrToStringChars (data->Surface);
                        Bentley::TerrainModel::Element::DTMElementHandlerManager::SetName (element.GetElementRef(), p);
                        }
                    }
                }
            }


        if (imported)
            {
            WString msg;
            WChar format[1024];

            if (SUCCESS != mdlResource_loadFromStringList (format, NULL, STRINGID_Message_Main, RESULT_ImportDTM))
                wcscpy_s (format, L"Terrain model %s imported successfully");
            WString::Sprintf (msg, format, dtmName.GetWCharCP());
            mdlOutput_messageCenter (OutputMessagePriority::Info, msg.GetWCharCP(), NULL, OutputMessageAlert::None);
            }
        else
            {
            WString msg;
            WChar format[1024];

            if (SUCCESS != mdlResource_loadFromStringList (format, NULL, STRINGID_Message_Main, RESULT_ImportFailedDTM))
                wcscpy_s (format, L"Terrain model %s imported successfully");
            WString::Sprintf (msg, format, dtmName.GetWCharCP());
            mdlOutput_messageCenter (OutputMessagePriority::Info, msg.GetWCharCP(), NULL, OutputMessageAlert::None);
            }
        }
    }

void DoImport (WCharCP unparsed, bool onlyLandXML)
    {
    WString fileName;
    if (WString::IsNullOrEmpty (unparsed))
        {
        FileOpenParams fileOpenParams;
        WChar titleString[1024];
        WChar filterInfoStr[1024];

        memset (&fileOpenParams, 0, sizeof fileOpenParams );
        mdlResource_loadFromStringList (titleString, NULL, STRINGID_Message_Main, DIALOGTITLE_SelectLandXML);
        fileOpenParams.openCreate       = FILELISTATTR_DEFAULT;

        if (onlyLandXML)
            {
            fileOpenParams.defaultFilterP   = L"*.xml";
            mdlResource_loadFromStringList (filterInfoStr, NULL, STRINGID_Message_Main, DIALOGTITLE_LandXMLFileFilter);
            fileOpenParams.filterInfoStrP = filterInfoStr;
            }
        else
            {
            fileOpenParams.defaultFilterP = L"*.xml;*.fil;*.tin;*.bcdtm;*.dat;*.xyz;*.las";
            fileOpenParams.filterInfoStrP   = L"*.xml,LandXML [*.xml],*.fil,MX Model File [*.fil],*.tin;*.bcdtm,Geopak Tin File [*.tin;*.bcdtm],*.dat,Geopak Dat File [*.dat],XYZ File [*.xyz],Lidar File [*.las],*.las";
            }
        fileOpenParams.titleP           = titleString;
        fileOpenParams.dialogId         = DIALOGID_ExtendedFileOpen;
        fileOpenParams.defFileId        = DEFSCHEDULELINKER_ID;

        BeFileName  wcFileName;
    
        if (SUCCESS != mdlDialog_fileOpenExt (wcFileName, NULL, &fileOpenParams, 0))
            return;

        fileName = wcFileName.GetName();
        }
    else if (unparsed[0] == '@')
        {
        if (unparsed[1] == '@')
            {
            array<System::String^>^ surfaceNames = gcnew array<System::String^> (1);
            surfaceNames[0] = "Temp Surface";
            LandXMLImportForm form (surfaceNames);

            form.ShowDialog (gcnew Bentley::MstnPlatformNET::WinForms::MicroStationWin32 ());
            return;
            }
        LandXMLLoggedImport ^collectedData = LandXMLLoggedImport::CreateParsed (gcnew System::String (&unparsed[1]));

        if (nullptr == collectedData)
            {
            WChar msg[1000];

            mdlOutput_error ( SUCCESS == mdlResource_loadFromStringList ( msg, NULL, STRINGID_Message_Main, ERROR_UnreadableInputFile) ? msg : L"<Unreadable Input File!>" );
            return;
            }

        pin_ptr<const wchar_t> p = PtrToStringChars (collectedData->FileName);
        fileName = p;

        TerrainImporterPtr reader = TerrainImporter::CreateImporter (fileName.GetWCharCP());

        if (reader.IsValid ())
            ImportSurfaces (*reader.get(), collectedData->Surfaces, onlyLandXML);
        return;
        }

    if (NULL != mdlSystem_findMdlDesc (L"FEATURETRACKING"))
        mdlInput_sendKeyin (L"TERRAINMODEL FEATURETRACK", 0, INPUTQ_HEAD, L"TERRAINMODELCOMMANDS");

    TerrainImporterPtr reader;
    WString extension = BeFileName::GetExtension (fileName.GetWCharCP());

    if (!onlyLandXML || extension.CompareToI (L"xml") == 0)
        reader = TerrainImporter::CreateImporter (fileName.GetWCharCP());
    else
        reader = nullptr;

    if (reader.IsValid ())
        {
        const TerrainInfoList& surfaces = reader->GetTerrains ();

        if (surfaces.size() > 0)
            {
            array<System::String^>^ surfaceNames = gcnew array<System::String^>((int)surfaces.size());
            int i = 0;
            for(TerrainInfoList::const_iterator it = surfaces.begin(); it != surfaces.end(); i++, it++)
                surfaceNames[i] = gcnew System::String (it->GetName().GetWCharCP());

            LandXMLImportForm form (surfaceNames);

            if (form.ShowDialog ( gcnew Bentley::MstnPlatformNET::WinForms::MicroStationWin32() ) == System::Windows::Forms::DialogResult::OK)
                {
                ImportSurfaces (*reader.get(), form.SelectedSurfaces, onlyLandXML);
                }
            mdlState_startDefaultCommand ();
            }
        else
            {
            WChar msg[1000] = L"";
            mdlDialog_openMessageBox ( DIALOGID_MsgBoxOK, \
                ( SUCCESS == mdlResource_loadFromStringList ( msg, 0, STRINGID_Message_Main, ERROR_NoSurfaceDetected ) && msg[0] ) ? msg : L"<No surface detected>", \
                MessageBoxIconType::Warning );
            }
        }
    else
        {
        WChar msg[1000] = L"";
        mdlDialog_openMessageBox (DIALOGID_MsgBoxOK, \
                                  (SUCCESS == mdlResource_loadFromStringList (msg, 0, STRINGID_Message_Main, ERROR_InvalidFile) && msg[0]) ? msg : L"<Invalid file>", \
                                  MessageBoxIconType::Warning);
        }

    }

void AddIn::ImportLandXML (WCharCP unparsed)
    {
    DoImport (unparsed, true);
    }

void AddIn::ImportDTM (WCharCP unparsed)
    {
    DoImport (unparsed, false);
    }

END_BENTLEY_TERRAINMODEL_COMMANDS_NAMESPACE

namespace BDC = Bentley::TerrainModel::Commands;

/// <author>Piotr.Slowinski</author>                            <date>04/2011</date>
void ImportCMD (WCharCP unparsed)
    {
    if ( !mdlModelRef_getActive ()->Is3d() )
        {
        mdlState_startDefaultCommand ();

        WString     tmpstr;
        short        relerr = 0;

        MstnResourceUtils::GetUstationError (tmpstr, &relerr, 3);

        /* save for user commands */
        tcb->relerr = relerr;
        mdlOutput_error (tmpstr.c_str ());
        }
    else if ( WORKMODE_DGN != mdlSystem_getWorkmode () )
        {
        WChar msg[1024] = L"";

        mdlState_startDefaultCommand ();
        mdlOutput_error ( SUCCESS == mdlResource_loadFromStringList ( msg, NULL, STRINGID_Message_Main, ERROR_UnacceptableWorkMode ) ? msg : L"<Unacceptable work mode!>" );
        }
    else switch ( tcb->last_parsed_command )
        {
        case CMD_TERRAINMODEL_IMPORT_LANDXML:
            BDC::AddIn::ImportLandXML (unparsed);
            break;

        case CMD_TERRAINMODEL_IMPORT_DTM:
            BDC::AddIn::ImportDTM ( nullptr );
            break;
        }
    }

/// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
void PrepareLandXMLCfg (WCharCP unparsed)
    {
    System::String ^fileName = gcnew System::String (unparsed);

    if (System::String::IsNullOrEmpty (fileName))
        return;

    TerrainImporterPtr reader = TerrainImporter::CreateImporter (unparsed);

    if (reader.IsNull ())
        return;

    const TerrainInfoList& surfaces = reader->GetTerrains ();

    if (surfaces.size() < 1)
        return;

    System::String ^configFile = fileName->Substring (0, fileName->LastIndexOf(L'.')) + "_config.xml";

    XmlTextWriter writer (configFile, System::Text::ASCIIEncoding::UTF8);
    writer.Formatting = Formatting::Indented;
    writer.Indentation = 4;
    writer.WriteStartDocument ();
    writer.WriteStartElement ("LandXMLFile");
    writer.WriteAttributeString ("Path", fileName);
    for (TerrainInfo info : surfaces)
        {
        writer.WriteStartElement ("Surface");
        writer.WriteAttributeString ("Name", gcnew System::String (info.GetName ().GetWCharCP ()));
        writer.WriteAttributeString ("ElementTemplate", "None");
        writer.WriteEndElement ();
        }
    writer.WriteEndElement ();
    writer.WriteEndDocument ();
    writer.Close ();
    }

void CMD_TERRAINMODEL_annotateContours (WCharCP unparsed)
    {
    BDC::AddIn::AnnotateContours (unparsed);
    }

void CMD_TERRAINMODEL_annotateSpots (WCharCP unparsed)
    {
    BDC::AddIn::AnnotateSpots (unparsed);
    }

#pragma unmanaged

void FeatureTrack (WCharCP unparsed)
    {
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   Hook List                                                           |
|                                                                       |
+----------------------------------------------------------------------*/
static DialogHookInfo uHooks[] =
    {
        {HOOKITEMID_CommonOverriddenDistanceProp, (PFDialogHook)hook_labelContoursTextHeightWidth},
    };

static MdlCommandNumber s_commandNumbers [] =
    { 
        { FeatureTrack,                                      CMD_TERRAINMODEL_FEATURETRACK },
        { ImportCMD,                                         CMD_TERRAINMODEL_IMPORT_DTM },
        { ImportCMD,                                         CMD_TERRAINMODEL_IMPORT_LANDXML },
        { PrepareLandXMLCfg,                                 CMD_TERRAINMODEL_IMPORT_LANDXML_PREPARE_CONFIG },
        { CMD_TERRAINMODEL_annotateContours,                 CMD_TERRAINMODEL_LABEL_CONTOURS},
        { CMD_TERRAINMODEL_annotateSpots,                    CMD_TERRAINMODEL_LABEL_SPOTS},
    };

MdlDesc*    s_mdlDescLoader = nullptr;

dtmcommandsInfo *dtmcommandsInfoP;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Piotr.Slowinski                 02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" __declspec(dllexport) void MdlMain (int , WCharCP  argv[])
    {
    RscFileHandle   rscFileH;

    mdlResource_openFile (&rscFileH, NULL, 0);

     /* Publish the dialog item hooks */
    mdlDialog_hookPublish ( _countof ( uHooks ), uHooks);

    mdlSystem_registerCommandNumbers (s_commandNumbers);
    mdlParse_loadCommandTable (NULL);
    mdlState_registerStringIds (STRINGLISTID_Commands, STRINGLISTID_Prompts);

//    s_mdlDescLoader = mdlSystem_getCurrMdlDesc ();

    //WordlibControl &wlc = *(WordlibControl*)1;
    //mdlWordlib_initialize ( wlc );
    //mdlWordlib_terminate ( wlc );

    SymbolSet* setP;
    setP = mdlCExpression_initializeSet (VISIBILITY_DIALOG_BOX | VISIBILITY_CALCULATOR, 0, TRUE);
    dtmcommandsInfoP = (dtmcommandsInfo*)calloc (1, sizeof(*dtmcommandsInfoP));
    mdlDialog_publishComplexPtr (setP, "dtmcommandsInfo", "dtmcommandsInfoP", &dtmcommandsInfoP);

//    mdlSystem_loadMdlProgram ( ADDIN_ASSEMBLYNAME L".dll", ADDIN_NAME L",DEFAULTDOMAIN", nullptr );
    }

#pragma managed

struct mdlDesc {};