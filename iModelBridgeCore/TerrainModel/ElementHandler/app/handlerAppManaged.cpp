/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/app/handlerAppManaged.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma unmanaged

#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/idtm.h>
#include <TerrainModel/Core/bcdtmclass.h>
#include <TerrainModel/ElementHandler/DTMElementHandlerManager.h>
#include <TerrainModel/ElementHandler/TMElementDisplayHandler.h>
#include <DgnPlatform/TerrainModel/TMSymbologyOverrideManager.h>
#include <DgnPlatform\TerrainModel\TMElementHandler.h>
#include <RmgrTools/Tools/RscFileManager.h>
#include "handlerAppDefs.h"
#include <Mstn\MdlApi\mdl.h>
#include <Mstn\basetype.h>
#include <Mstn\MdlApi\msdisplaypath.h>

#include <DgnPlatform\DgnECTypeAdapters.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

    DTMELEMENT_EXPORT ECSchemaCP GetPresentationSchema ();

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

#pragma managed

#using <System.dll>
#using <System.Windows.Forms.dll>
#using <System.Drawing.dll>
#using <Bentley.ECSystem3.dll>
#using <Bentley.ECObjects3.dll>
#using <Bentley.EC.AbstractUI3.dll>
#using <Bentley.UI.dll>
#using <Bentley.DgnPlatformNET.dll>
#using <Bentley.Platform.dll>
#using <Bentley.Microstation.Winforms.Controls.dll>
#using <bentley.microstation.winforms.ecpropertypane.dll>
#using <ustation.dll>
#using <bentley.terrainmodelNET.Element.dll>
#using <bentley.microstation.templates.support.dll>

namespace   SWFD = System::Windows::Forms::Design;
namespace   SCM = System::ComponentModel;
namespace   ECI = Bentley::ECObjects::Instance;
namespace   ECUI = Bentley::ECObjects::UI;
namespace   BDPN = Bentley::DgnPlatformNET;
namespace   BDPNDT = Bentley::DgnPlatformNET::XDataTree;
namespace   ECXDT = Bentley::MstnPlatformNET::XDataTree;
using namespace Bentley::MstnPlatformNET::XDataTree;
using namespace Bentley::TerrainModelNET::Element;

#include<vcclr.h>   // for PtrToStringChars()

using namespace Bentley::ECObjects;
using namespace Bentley::ECObjects::Instance;
using namespace Bentley::ECSystem::Extensibility;
using namespace Bentley::EC::AbstractUI;
using namespace Bentley::DgnPlatformNET::DgnEC;
using namespace Bentley::ECSystem::Session;
using namespace Bentley::ECSystem::Repository;
using namespace Bentley::Collections;
using namespace System::Collections;
using namespace Bentley::ECObjects::UI; 

RscFileManager::DllRsc*  g_TMHandlersResources;

WString          GetString (UInt stringId)
    {
    return g_TMHandlersResources->GetString (stringId);
    }

namespace Bentley { namespace TerrainModelNET {

ref class SelectionListener : IEventHookHandler, IAUICommandManagerExtension, IAUICommandExecutorExtension
    {
    private: array<System::String^>^ m_applicationTypes;
    private: static SelectionListener^ s_instance;
    private: ECClassCP m_subDisplayClass;
    private: ECClassCP m_dtmElementClass;
    private: UInt32 m_displayParam;
    private: bool m_hasHighlight;
    private: ElementHandleP m_element;
    private: System::String^ m_overrideSymbologyCommand;

    public: static property SelectionListener^ Instance
                {
                SelectionListener^ get()
                    {
                    if (s_instance == nullptr)
                        s_instance = gcnew SelectionListener ();
                    return s_instance;
                    }
                }

    protected: SelectionListener ()
                   {
                   m_hasHighlight = false;
                   m_element = nullptr;
                   m_subDisplayClass = nullptr;
                   m_dtmElementClass = nullptr;
                   m_overrideSymbologyCommand = "Bentley.TerrainModel.Commands.OverrideSymbology";
                   }

               virtual ~SelectionListener ()
                   {
                   if (m_element) delete m_element;
                   }

    public: property virtual array<System::String^>^ ApplicationTypes
                {
                virtual array<System::String^>^ get() //override
                    {
                    if (m_applicationTypes == nullptr)
                        {
                        m_applicationTypes = gcnew array<System::String^> (1);
                        m_applicationTypes[0] = MultiSelectedEvent::APPLICATION;
                        }
                    return m_applicationTypes;
                    }
                }

    public: virtual bool EventCallback (System::String^ application, int hookCode, System::Object^ args, int% returnCode) // override
                {
                if (application == MultiSelectedEvent::APPLICATION && hookCode == KeyMultiSelectedEvent::EVENTCODE)
                    {
                    KeyMultiSelectedEvent^ multiSelectEvent = dynamic_cast<KeyMultiSelectedEvent^>(args);

                    bool highlight = false;
                    ElementHandle element;
                    UInt32 displayParam = 0;

                    if (multiSelectEvent)
                        {
                        if (multiSelectEvent->IsSubSelection)
                            {
                            if (multiSelectEvent->SelectionControl->SelectedDisplayItemsCount == 1)
                                {
                                for each (AUIDisplayItem^ item in multiSelectEvent->SelectionControl->SelectedDisplayItems)
                                    {
                                    IDgnECInstance^ clrInst = dynamic_cast<IDgnECInstance^>(item->IECInstance);
                                    if (clrInst && nullptr != clrInst->Element) // we only care about element-based instances...
                                        {
                                        // NEEDSWORK: converting between managed and native instances is expensive...if all we are doing is checking
                                        // the instance's ECClass maybe we can do that before or instead of converting to native...
                                        DgnECInstancePtr dgnInstance;
                                        DgnPlatformNET::DgnEC::DgnECManager::Manager->ConvertManagedInstanceToNative (System::IntPtr(&dgnInstance), clrInst);
                                        DgnElementECInstancePtr instance;
                                        if (dgnInstance.IsValid() && (instance = dgnInstance->GetAsElementInstance()).IsValid())
                                            {
                                            if (!m_subDisplayClass)
                                                m_subDisplayClass = Bentley::TerrainModel::Element::GetPresentationSchema()->GetClassCP (L"DTMSubElementDisplay");
                                            if (instance->GetClass().Is (m_subDisplayClass))
                                                {
                                                element = instance->GetElementHandle();
                                                displayParam = instance->GetLocalId();
                                                highlight = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                    if (m_hasHighlight)
                        {
                        if (displayParam == m_displayParam && m_element->GetElementRef() == element.GetElementRef())
                            highlight = false;
                        else
                            {
                            HiliteOrDehilite (*m_element, m_displayParam, false);
                            delete m_element;
                            m_element = nullptr;
                            m_hasHighlight = false;
                            }
                        }
                    if (highlight)
                        {
                        HiliteOrDehilite (element, displayParam, true);
                        m_hasHighlight = true;
                        m_displayParam = displayParam;
                        m_element = new ElementHandle (element);
                        }
                    return true;
                    }

                return false;
                }

    public: virtual AUICommandItemList^ GetCommandItems (ECSession^ session, AUIDisplayItem^ displayItem, IExtendedParameters^ context)
                {
                if (nullptr == displayItem)
                    return nullptr;

                IDgnECInstance^ clrInst = dynamic_cast<IDgnECInstance^>(displayItem->IECInstance);
                if (nullptr == clrInst)
                    return nullptr;

                DgnECInstancePtr nativeInstance;
                DgnPlatformNET::DgnEC::DgnECManager::Manager->ConvertManagedInstanceToNative (System::IntPtr(&nativeInstance), clrInst);
                if (nativeInstance.IsNull())
                    return nullptr;

                DgnElementECInstancePtr instance = nativeInstance->GetAsElementInstance();
                if (!instance.IsValid())
                    return nullptr;

                if (!m_dtmElementClass)
                    m_dtmElementClass = Bentley::TerrainModel::Element::GetPresentationSchema()->GetClassCP (L"DTMElement");
                if (instance->GetClass().Is (m_dtmElementClass))
                    {
                    ElementHandle element = instance->GetElementHandle();

                    if (TMSymbologyOverrideManager::CanHaveSymbologyOverride (element))
                        {
                        bool hasOverride = false;
                        ElementHandle elementHandle2;
                        if (TMSymbologyOverrideManager::GetElementForSymbology (element, elementHandle2, mdlModelRef_getActive ()))
                            hasOverride = elementHandle2.GetModelRef() == mdlModelRef_getActive();

                        AUICommandItemList^ list = gcnew AUICommandItemList ("", "");
                        AUICommandItem^ item;
                        if (hasOverride)
                            item = gcnew AUICommandItem (m_overrideSymbologyCommand, L"Remove Override Symbology", L"Removes the Override Symbology for this DTM", nullptr, false, true, true);
                        else
                            item = gcnew AUICommandItem (m_overrideSymbologyCommand, L"Override Symbology", L"Sets the Override Symbology for this DTM", nullptr, false, true, true);

                        item->CommandArgs = Bentley::Collections::ExtendedParameters::Create ();
                        item->CommandArgs ["ModelRefP"] = (System::IntPtr)element.GetModelRef();
                        item->CommandArgs ["ElementRefP"] = (System::IntPtr)element.GetElementRef();
                        item->CommandArgs ["ActiveModel"] = (System::IntPtr)ACTIVEMODEL;
                        list->Add (item);

                        return list;
                        }
                    }
                return nullptr;
                }

    private: void ProcessAdd (ElementHandleCR orgEh, DgnModelRefP destModelRef)
                 {
                 if (SUCCESS == TMSymbologyOverrideManager::CreateSymbologyOverride (orgEh, destModelRef))
                     {
                     }
                 }

    private: static void ProcessDel (ElementHandleCR orgEH, DgnModelRefP destModelRef)
                 {
                 TMSymbologyOverrideManager::DeleteSymbologyOverride (orgEH, destModelRef);
                 }

    private: static void DispPre (ElementHandleCR eh)
                 {
                 DisplayPathCP displayPath = mdlDisplayPath_new (eh.GetElementRef(), eh.GetModelRef());
                 mdlDisplayPath_drawInViews (displayPath, 0xffff, DRAW_MODE_Normal, DrawPurpose::ChangedPre);
                 mdlDisplayPath_release (displayPath);
                 }

    public: virtual bool ExecuteCommand (RepositoryConnection^ connection,System::String^ commandID, IExtendedParameters^ parameters)
                {
                if (commandID == m_overrideSymbologyCommand)
                    {
                    ElementHandle elementHandle ((ElementRefP)((System::IntPtr)parameters ["ElementRefP"]).ToPointer(), (DgnModelRefP)((System::IntPtr)parameters ["ModelRefP"]).ToPointer());
                    DgnModelRefP activeModel = (DgnModelRefP)((System::IntPtr)parameters ["ActiveModel"]).ToPointer();

                    if (TMSymbologyOverrideManager::CanHaveSymbologyOverride (elementHandle))
                        {
                        ElementHandle elementHandle2;
                        if (TMSymbologyOverrideManager::GetElementForSymbology (elementHandle, elementHandle2, activeModel))
                            {
                            if (elementHandle2.GetModelRef() == activeModel)
                                ProcessDel (elementHandle, activeModel);
                            else
                                ProcessAdd (elementHandle, activeModel);
                            }
                        else
                            ProcessAdd (elementHandle, activeModel);

                        DispPre (elementHandle);
                        }
                    return true;
                    }
                return false;
                }

                private:
                    void SetPathHiliteState (ElementHandleCR elem, int drawMode, int id)
                        {
                        ElementRefP elemRef = elem.GetElementRef();
                        ElementHiliteState  newState;

                        if (DRAW_MODE_Hilite == drawMode)
                            newState = HILITED_Bold;
                        else
                            newState = HILITED_None;

                        Bentley::TerrainModel::Element::DTMElementDisplayHandler::SetHighlight (elemRef, id, newState);
                        }

                    void HiliteOrDehilite (ElementHandle element, UInt32 displayParamId, bool hiliteIn)
                        {
                        //        TMSymbologyOverrideManager::GetReferencedElement (m_subElement->GetElement()->GetElemHandle(), elem);

                        SetPathHiliteState (element, hiliteIn ? DRAW_MODE_Hilite : DRAW_MODE_Normal, displayParamId);
                        DisplayPathCP    displayPath = mdlDisplayPath_new (element.GetElementRef(), element.GetModelRef());
                        mdlDisplayPath_drawInViews (displayPath, 0xffff, hiliteIn ? DRAW_MODE_Hilite : DRAW_MODE_Normal, hiliteIn ? DrawPurpose::Hilite : DrawPurpose::Unhilite);
                        mdlDisplayPath_release (displayPath);
                        }
                };

public ref class SymbologyPickerOptions : Bentley::MstnPlatformNET::WinForms::Controls::SymbologyPickerOptions
    {
    static SymbologyPickerOptions^ s_instance;
    System::String^ m_fromParent;

    SymbologyPickerOptions()
        {
        m_fromParent = gcnew System::String (GetString (MESSAGE_FromParent).GetWCharCP());
        }
    public: property static SymbologyPickerOptions^ Instance
        {
        SymbologyPickerOptions^ get()
            {
            if (s_instance == nullptr)
                s_instance = gcnew SymbologyPickerOptions();
            return s_instance;
            }
        }
    public: property virtual bool AllowByCell
                {
                virtual bool get() override
                    {
                    return true;
                    }
                }
    public: property virtual System::String^ ByCellDisplayString
                {
                virtual System::String^ get() override
                    {
                    return m_fromParent;
                    }
                }
    public: property virtual System::Drawing::Bitmap^ ByCellIcon
                {
                virtual System::Drawing::Bitmap^ get() override
                    {
                    return nullptr;
                    }
                }
    };

public ref class DgnECColorEditor : public Bentley::MstnPlatformNET::WinForms::ECPropertyPane::DgnECColorEditor
    {
    public: property Bentley::MstnPlatformNET::WinForms::Controls::SymbologyPickerOptions^ PickerOptions
                {
                virtual Bentley::MstnPlatformNET::WinForms::Controls::SymbologyPickerOptions^ get () override
                    {
                    return SymbologyPickerOptions::Instance;
                    }
                }
    };

public ref class LineStyleEditor : public Bentley::MstnPlatformNET::WinForms::ECPropertyPane::LineStyleEditor
    {
    public: property Bentley::MstnPlatformNET::WinForms::Controls::SymbologyPickerOptions^ PickerOptions
                {
                virtual Bentley::MstnPlatformNET::WinForms::Controls::SymbologyPickerOptions^ get () override
                    {
                    return SymbologyPickerOptions::Instance;
                    }
                }
    };

public ref class WeightEditor : public Bentley::MstnPlatformNET::WinForms::ECPropertyPane::WeightEditor
    {
    public: property Bentley::MstnPlatformNET::WinForms::Controls::SymbologyPickerOptions^ PickerOptions
                {
                virtual Bentley::MstnPlatformNET::WinForms::Controls::SymbologyPickerOptions^ get () override
                    {
                    return SymbologyPickerOptions::Instance;
                    }
                }
    };

public ref class DTMElementTemplateEditor : public Bentley::MstnPlatformNET::WinForms::ECPropertyPane::ElementTemplateEditor
    {
    public: virtual System::Object^ EditValue (SCM::ITypeDescriptorContext^ context, System::IServiceProvider^ provider, System::Object^ value, ECI::IECPropertyValue^ propertyValue, ECUI::ECEnumerablePropertyDescriptor^ propertyDescriptor) override 
        {
        // Obtain an SWFD.IWindowsFormsEditorService.
        SWFD::IWindowsFormsEditorService^ edSvc = dynamic_cast<SWFD::IWindowsFormsEditorService^>(provider->GetService(SWFD::IWindowsFormsEditorService::typeid));

        if (nullptr != edSvc)
            {
            System::String^ oldValue =  dynamic_cast<System::String^>(value);
            BDPN::Elements::Element^ element = GetInstanceElement (propertyValue);
            DTMElement^ dtmElement = dynamic_cast<DTMElement^>(element);
            if (dtmElement != nullptr)
                {
                BDPN::Elements::Element^ overrideElement = dtmElement->GetSymbologyOverrideElement();
                if (overrideElement != nullptr) element = overrideElement;
                }
//            BDPN::DgnFile^    file = GetInstanceDgnFile (propertyValue);
            BDPN::DgnFile^    file = element->DgnModel->GetDgnFile();
            bool allowImport = nullptr != file;
        
            Bentley::MstnPlatformNET::WinForms::ECPropertyPane::ElementTemplateEditorControl^ control = gcnew Bentley::MstnPlatformNET::WinForms::ECPropertyPane::ElementTemplateEditorControl (edSvc, oldValue, allowImport);
            edSvc->DropDownControl (control);

            value = control->SelectedElementTemplate;

            // Import into the file if the selected template is from a dgnlib
            // Do NOT import if we're editing a dummy instance for ECQueryForm
            BDPNDT::XDataTreeNode^ node;
            if (nullptr == file)
                file = Bentley::MstnPlatformNET::Session::Instance->GetActiveDgnFile();

            bool templateImported = false;
            if (nullptr != file && nullptr != (node = ElementTemplateMgr::GetExistingTemplate (control->SelectedElementTemplate, file, templateImported)) && templateImported)
                {
                value = control->SelectedElementTemplate;

                // if Element Templates dialog is open, update it.
                XDataTreeSessionMgr::RefreshExternalData (XDataTreeSessionMgr::TemplateDataTrees->HandlerID);
                }
            }

        return value;        
        }
    };





/*---------------------------------------------------------------------------------**//**
* @bsiclass                                Mathieu.St-Pierre                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
public ref class MrDTMViewFlagsTypeEditor : public Bentley::MstnPlatformNET::WinForms::ECPropertyPane::ViewFlagsTypeEditor
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
public ref class MrDTMViewPointDensityTypeEditor : public Bentley::MstnPlatformNET::WinForms::ECPropertyPane::DropDownEditor
    {
    public:
    MrDTMViewPointDensityTypeEditor () {}

    virtual System::String^ GetDropDownTitle () override
        {
        return L"Density";
        }
    };

ref class TMDgnElementColorStructTypeConverter : Bentley::MstnPlatformNET::Templates::Support::DgnElementColorStructTypeConverter
    {
    public:
    virtual System::String^ ConvertToString
        (
        IECPropertyValue^                       propVal,
        ECUI::ECEnumerablePropertyDescriptor^   propertyDescriptor,
        int                                     unusedIndex,
        System::Globalization::CultureInfo^     culture,
        System::Object^                         value
        ) override
        {
        IECStructValue^     structVal = safe_cast<IECStructValue^>(propVal);
        Bentley::DgnPlatformNET::ElementColor^ elementColor = ElementTemplateMgr::LoadElementColorFromPropertyValue (structVal);

        if (nullptr != elementColor)
            {
            if (elementColor->Source == Bentley::DgnPlatformNET::ColorSource::ByCell)
                return gcnew System::String (GetString (MESSAGE_FromParent).GetWCharCP ());
            return elementColor->ToString ();
            }

        return System::String::Empty;
        }

    };


}}

using namespace Bentley::TerrainModelNET;

static void dummy ()
    {
    ;
    }

void registerSelectionListener ()
    {
    BeFileName dllFileName;
    BeGetModuleFileName (dllFileName, (void*)&dummy);

#if defined (CREATE_STATIC_LIBRARIES)
    // We need to manually feed a fake name here so the automatic lookup works.
    // The MUIs are in the same location as you'd expect, but with a different file name base than whatever the statically linked application is.
    WString moduleDir = BeFileName::GetDirectoryName (dllFileName);
    dllFileName.BuildName (NULL, moduleDir.c_str (), DLL_NAME_FOR_RESOURCES, NULL);
#endif


    g_TMHandlersResources = new RscFileManager::DllRsc (dllFileName); // InitializeDgnCore has already called RscFileManager::StaticInitialize (<<culturename>>)

    EventHookManager::RegisterEventHook (SelectionListener::Instance);
    AUICommandManager::GetInstance ()->RegisterExtension (SelectionListener::Instance);
    AUICommandExecutor::GetInstance ()->RegisterExtension (SelectionListener::Instance);

    Bentley::ECObjects::Instance::IECInstance^ extendedType = Bentley::ECObjects::UI::ECPropertyPane::CreateExtendedType ("TMElementTemplate");
    Bentley::ECObjects::UI::ECPropertyPane::SetExtendedTypeWinFormUIEditor (extendedType, DTMElementTemplateEditor::typeid);

    extendedType = Bentley::ECObjects::UI::ECPropertyPane::CreateExtendedType ("TMColor");
    Bentley::ECObjects::UI::ECPropertyPane::SetExtendedTypeWinFormUIEditor (extendedType, DgnECColorEditor::typeid);
    Bentley::ECObjects::UI::ECPropertyPane::SetExtendedTypeTypeConverter (extendedType, TMDgnElementColorStructTypeConverter::typeid);

    extendedType = Bentley::ECObjects::UI::ECPropertyPane::CreateExtendedType ("TMStyle");
    Bentley::ECObjects::UI::ECPropertyPane::SetExtendedTypeWinFormUIEditor (extendedType, LineStyleEditor::typeid);

    extendedType = Bentley::ECObjects::UI::ECPropertyPane::CreateExtendedType ("TMWeight");
    Bentley::ECObjects::UI::ECPropertyPane::SetExtendedTypeWinFormUIEditor (extendedType, WeightEditor::typeid);

    // Shaded View Point Density - type editor and type converter
    extendedType = Bentley::ECObjects::UI::ECPropertyPane::CreateExtendedType ("MrDTMShadedViewPointDensity");
    Bentley::ECObjects::UI::ECPropertyPane::SetExtendedTypeWinFormUIEditor (extendedType, MrDTMViewPointDensityTypeEditor::typeid);
    ECPropertyPane::SetExtendedTypeTypeConverter (extendedType, Bentley::MstnPlatformNET::WinForms::ECPropertyPane::DropDownTypeConverter::typeid);

    // Unshaded View Point Density - type editor and type converter
    extendedType = Bentley::ECObjects::UI::ECPropertyPane::CreateExtendedType ("MrDTMUnshadedViewPointDensity");
    Bentley::ECObjects::UI::ECPropertyPane::SetExtendedTypeWinFormUIEditor (extendedType, MrDTMViewPointDensityTypeEditor::typeid);
    ECPropertyPane::SetExtendedTypeTypeConverter (extendedType, Bentley::MstnPlatformNET::WinForms::ECPropertyPane::DropDownTypeConverter::typeid);

    // View flags type editor
    extendedType = Bentley::ECObjects::UI::ECPropertyPane::CreateExtendedType ("MrDTMViewsFlags");
    Bentley::ECObjects::UI::ECPropertyPane::SetExtendedTypeWinFormUIEditor (extendedType, MrDTMViewFlagsTypeEditor::typeid);
    }

