/*----------------------------------------------------------------------+
|
|
|   $Source: BaseGeoCoord/EllipsoidEditingPanel.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/


// NOTE: This file is #included from BaseManagedGCS.cpp
/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
ref class    GeoEllipsoidStatusPanel : SWF::Panel
{
private:
array<SWF::Label^>^     m_lines;
SD::Font^               m_font;
SD::Size                m_size;
SD::Color               m_bgColor;
Ellipsoid^              m_ellipsoid;
ECI::ECInstanceList^    m_instanceList;
SWF::Button^            m_okButton;
String^                 m_originalName;

static const int       LINE_X     = 2;
static const int       LINE_H     = 14;
static const int       LINE_W     = 400;
static const int       LINE_Y     = 0;
static const int       NUMLINES   = 2;

/*------------------------------------------------------------------------------------**/
/// <author>Barry.Bentley</author>                              <date>04/2007</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
SWF::Label^      CreateLine
(
int             yPosition
)
    {
    SWF::Label^ line = gcnew SWF::Label();
    line->Font       = m_font;
    line->Size       = m_size;
    line->Location   = SD::Point (LINE_X, yPosition);
    line->BackColor  = m_bgColor;
    line->Anchor     = SWF::AnchorStyles::Left | SWF::AnchorStyles::Top | SWF::AnchorStyles::Right;
    return line;
    }

/*---------------------------------------------------------------------------------**//**
/// <author>Barry.Bentley</author>                              <date>04/2007</date>
+---------------+---------------+---------------+---------------+---------------+------*/
public:
GeoEllipsoidStatusPanel (Ellipsoid^ ellipsoid, SWF::Button^ okButton)
    {
    m_ellipsoid         = ellipsoid;
    m_instanceList      = nullptr;
    m_okButton          = okButton;
    m_originalName      = ellipsoid->Name;

    SuspendLayout();
    Dock                = SWF::DockStyle::Fill;
    Size                = SD::Size (400,25);
    int currentY        = LINE_Y;
    m_size              = SD::Size (LINE_W, LINE_H);
    m_font              = gcnew SD::Font (gcnew String ("Microsoft Sans Serif"), (float)12, (SD::FontStyle)0, SD::GraphicsUnit::Pixel, 0);
    m_bgColor           = SD::Color::FromArgb (245,245,245);
    m_lines             = gcnew array<SWF::Label^>(NUMLINES);
    for (int iLine=0; iLine < NUMLINES; iLine++)
        {
        m_lines[iLine] = CreateLine (currentY);
        currentY += LINE_H;
        }
    Controls->AddRange (m_lines);
    ResumeLayout();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SetListeners (ECI::ECInstanceList^ instanceList)
    {
    for each (ECI::IECInstance^ instance in instanceList)
        {
        instance->ECPropertyValueChanged += gcnew ECI::ECPropertyValueChangedHandler (this, &GeoEllipsoidStatusPanel::PropertyValueChangedHandler);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            RemoveListeners (ECI::ECInstanceList^ instanceList)
    {
    for each (ECI::IECInstance^ instance in instanceList)
        {
        instance->ECPropertyValueChanged -= gcnew ECI::ECPropertyValueChangedHandler (this, &GeoEllipsoidStatusPanel::PropertyValueChangedHandler);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SetInstanceList (ECI::ECInstanceList^ instanceList)
    {
    if (nullptr != m_instanceList)
        RemoveListeners (m_instanceList);

    if (nullptr != instanceList)
        SetListeners (instanceList);

    m_instanceList = instanceList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            PropertyValueChangedHandler (System::Object^ sender, ECI::ECPropertyValueChangedEventArgs^ args)
    {
    // every time any property changes, validate that the name is unique, and set enabled state of OK button.
    bool    inSystemLibrary;
    bool    isOk = true;
    
    // find the name
    // If the name hasn't changed, don't check for uniqueness
    if ( (nullptr != m_originalName) && (0 == String::Compare (m_ellipsoid->Name, m_originalName, true)) )
        isOk = true;
    else
        isOk = m_ellipsoid->NameUnique (inSystemLibrary);

    if (isOk)
        {
        m_lines[0]->Text = String::Empty;
        m_lines[1]->Text = String::Empty;
        }
    else
        {
        m_lines[0]->Text = GeoCoordinateLocalization::GetLocalizedString (inSystemLibrary ? "EllipsoidInSystemLibrary" : "EllipsoidInUserLibrary");
        m_lines[1]->Text = GeoCoordinateLocalization::GetLocalizedString ("SelectAnotherEllipsoidName");
        }

    if (nullptr != m_okButton)
        m_okButton->Enabled = isOk;
    }
};

/*=================================================================================**//**
* We subclass ECPropertyPane so we can override ReloadInstanceLists, which is called when
*  an IECProperty with the "RequiresReload" custom attribute is changed. In this case,
*  it's called when we change either the convert type.
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
ref class    GeoEllipsoidPropertyPane : ECUI::ECPropertyPane
{
Ellipsoid^                  m_ellipsoid;
GCSECObjectModel^           m_gcsECObjectModel;
GeoEllipsoidStatusPanel^    m_statusPanel;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/07
+---------------+---------------+---------------+---------------+---------------+------*/
GeoEllipsoidPropertyPane
(
Ellipsoid^              ellipsoid,
GCSECObjectModel^       gcsECObjectModel,
String^                 stateFileName
) : ECUI::ECPropertyPane (nullptr, nullptr, nullptr, L"EllipsoidProps", false, false, true, stateFileName, 1000)
    {
    m_ellipsoid         = ellipsoid;
    m_gcsECObjectModel  = gcsECObjectModel;
    m_statusPanel       = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/07
+---------------+---------------+---------------+---------------+---------------+------*/
protected:
virtual void         ReloadInstanceLists () override
    {
    ECI::ECInstanceListSet^ instanceListSet = gcnew ECI::ECInstanceListSet();
    ECI::ECInstanceList^    instanceList = nullptr;
    if (nullptr != m_ellipsoid)
        {
        instanceList    = m_gcsECObjectModel->GetEllipsoidProperties (m_ellipsoid);
        instanceListSet->Add (instanceList);
        }

    // When this is called, it's because a property is changed that requires us to reload the elements.
    SetInstanceListSet (instanceListSet);

    // make sure our status panel knows of any changes to the instance list.
    if (nullptr != m_statusPanel)
        m_statusPanel->SetInstanceList (instanceList);
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
property Ellipsoid^      Ellipsoid
    {
    Bentley::GeoCoordinatesNET::Ellipsoid^      get () { return m_ellipsoid; }
    void                                        set (Bentley::GeoCoordinatesNET::Ellipsoid^    value) { m_ellipsoid= value; ReloadInstanceLists(); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/07
+---------------+---------------+---------------+---------------+---------------+------*/
property GeoEllipsoidStatusPanel^   StatusPanel
    {
    GeoEllipsoidStatusPanel^    get () {return m_statusPanel;}
    void                        set (GeoEllipsoidStatusPanel^ value) {m_statusPanel = value;}
    }
};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
public ref class    GeoEllipsoidPropertiesPanel : SWF::Panel
{
private:
GeoEllipsoidPropertyPane^   m_propertyPane;
GeoEllipsoidStatusPanel^    m_statusPanel;

static const int        TOPPANEL_W  = 400;
static const int        TOPPANEL_H  = 400;
static const int        STATUS_H    = 35;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
GeoEllipsoidPropertiesPanel
(
Ellipsoid^          ellipsoid,
bool                forEditing,
SWF::Button^        okButton
)
    {
    Init (ellipsoid, forEditing, okButton);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    Init
(
Ellipsoid^          ellipsoid,
bool                editData,
SWF::Button^        okButton
)
    {
    GCSECObjectModel^ gcsECObjectModel      = gcnew GCSECObjectModel (false, false);
    m_propertyPane                          = gcnew GeoEllipsoidPropertyPane (nullptr, gcsECObjectModel, "GCSProperty");
    m_propertyPane->LayoutPreset            = BUCWG::GroupLayoutPreset::StrictTopDown;

    if (!editData)
        m_propertyPane->Dock                = SWF::DockStyle::Fill;
    else
        {
        Size                        = SD::Size (TOPPANEL_W, TOPPANEL_H);
        m_propertyPane->Size        = SD::Size (TOPPANEL_W, TOPPANEL_H - STATUS_H);
        m_propertyPane->Location    = SD::Point (0, 0);
        m_propertyPane->Anchor      = SWF::AnchorStyles::Right | SWF::AnchorStyles::Top | SWF::AnchorStyles::Left | SWF::AnchorStyles::Bottom;
        m_statusPanel               = gcnew GeoEllipsoidStatusPanel (ellipsoid, okButton);
        m_statusPanel->Location     = SD::Point (0, TOPPANEL_H - STATUS_H);
        m_statusPanel->Size         = SD::Size (TOPPANEL_W, STATUS_H);
        m_statusPanel->Anchor       = SWF::AnchorStyles::Right | SWF::AnchorStyles::Left | SWF::AnchorStyles::Bottom;
        m_propertyPane->StatusPanel = m_statusPanel;
        }
    m_propertyPane->Ellipsoid = ellipsoid;

    SuspendLayout();
    Controls->Add (m_propertyPane);
    if (editData)
        Controls->Add (m_statusPanel);
    ResumeLayout();
    }

};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
public ref class    EditEllipsoidFormContents
{
private:

SWF::Button^                    m_okButton;
SWF::Button^                    m_cancelButton;
SWF::Panel^                     m_bottomPanel;
GeoEllipsoidPropertiesPanel^    m_propertiesPanel;

static const int    BTN_Y       = 12;
static const int    PANELHEIGHT = 40;
static const int    PANELWIDTH  = 300;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
EditEllipsoidFormContents
(
SWF::Form^          form,
Ellipsoid^          ellipsoid,
bool                forEditing
)
    {
    InitializeComponent (form, ellipsoid, forEditing);
    }

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            InitializeComponent
(
SWF::Form^          form,
Ellipsoid^          ellipsoid,
bool                forEditing
)
    {
    m_okButton                  = gcnew SWF::Button();

    m_propertiesPanel           = gcnew GeoEllipsoidPropertiesPanel (ellipsoid, forEditing, m_okButton);
    m_propertiesPanel->Dock     = SWF::DockStyle::Fill;

    m_okButton->DialogResult    = SWF::DialogResult::OK;
    m_okButton->Location        = SD::Point(20, BTN_Y);
    m_okButton->Size            = SD::Size(96, 24);
    m_okButton->TabIndex        = 10;
    m_okButton->Text            = GeoCoordinateLocalization::GetLocalizedString ("Ok");

    if (forEditing)
        {
        m_cancelButton              = gcnew SWF::Button();
        m_cancelButton->DialogResult = SWF::DialogResult::Cancel;
        m_cancelButton->Location    = SD::Point(130, BTN_Y);
        m_cancelButton->Size        = SD::Size(96, 24);
        m_cancelButton->TabIndex    = 11;
        m_cancelButton->Text        = GeoCoordinateLocalization::GetLocalizedString ("Cancel");
        }

    m_bottomPanel               = gcnew SWF::Panel();
    m_bottomPanel->Size         = SD::Size(PANELWIDTH, PANELHEIGHT);
    m_bottomPanel->Dock         = SWF::DockStyle::Bottom;

    m_bottomPanel->SuspendLayout();
    m_bottomPanel->Controls->Add (m_okButton);
    if (forEditing)
        m_bottomPanel->Controls->Add (m_cancelButton);
    m_bottomPanel->ResumeLayout();

    form->SuspendLayout();
    form->Controls->Add (m_propertiesPanel);
    form->Controls->Add (m_bottomPanel);
    form->ResumeLayout();
    }

};


/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   05/10
+===============+===============+===============+===============+===============+======*/
public ref class   EllipsoidTreeNode : SWF::TreeNode
{
internal:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidTreeNode (String^ name, SWF::ContextMenu^ contextMenu) : SWF::TreeNode (name)
    {
    ContextMenu = contextMenu;
    InitializeImageKeys();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanRename
    {
    bool get () { return false; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanDeleteFromLibrary
    {
    bool get () { return false; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanPasteFromClipboard
    {
    bool get () { return false; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanCopyToClipboard
    {
    bool get () { return false; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanEdit
    {
    bool get () { return false; }
    }

virtual void    InitializeImageKeys () {}
};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   05/10
+===============+===============+===============+===============+===============+======*/
public ref class    EllipsoidNode : EllipsoidTreeNode
{
private:
Ellipsoid^      m_ellipsoid;
bool            m_fromUserLib;
internal:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidNode (String^ name, SWF::ContextMenu^ contextMenu, Ellipsoid^ ellipsoid, bool fromUserLib) : EllipsoidTreeNode (name, contextMenu)
    {
    m_fromUserLib   = fromUserLib;
    m_ellipsoid     = ellipsoid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
property Ellipsoid^     Ellipsoid
    {
    Bentley::GeoCoordinatesNET::Ellipsoid^  get () { return m_ellipsoid; }
    void                                    set (Bentley::GeoCoordinatesNET::Ellipsoid^    value) { m_ellipsoid = value; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanRename
    {
    bool get () override { return m_fromUserLib; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanDeleteFromLibrary
    {
    bool get () override { return m_fromUserLib; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanPasteFromClipboard
    {
    bool get () override { return true; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanCopyToClipboard
    {
    bool get () override { return true; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanEdit
    {
    bool get () override { return m_fromUserLib; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    InitializeImageKeys () override;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
property String^    KeyName
    {
    String^ get () { return m_ellipsoid->Name; }
    }
};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   05/10
+===============+===============+===============+===============+===============+======*/
public ref class    EllipsoidLibraryNode : EllipsoidTreeNode
{
private:
bool            m_isUserLib;
BGC::LibraryP   m_library;

internal:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidLibraryNode (String^ name, SWF::ContextMenu^ contextMenu, BGC::LibraryP sourceLibrary) : EllipsoidTreeNode (name, contextMenu)
    {
    m_library   = sourceLibrary;
    m_isUserLib = sourceLibrary->IsUserLibrary();

    // load all the members. It is not worth "delay loading", we are going to open the library nodes immediately anyway.
    uint32_t ellipsoidCount = (uint32_t) sourceLibrary->GetEllipsoidCount();
    for (uint32_t iEllipsoid=0; iEllipsoid < ellipsoidCount; iEllipsoid++)
        {
        WString ellipsoidName;
        if (BSISUCCESS == sourceLibrary->GetEllipsoidName (iEllipsoid, ellipsoidName))
            {
            CSEllipsoidDef* ellipsoidDef = sourceLibrary->GetEllipsoid (ellipsoidName.c_str());
            if (NULL != ellipsoidDef)
                {
                Ellipsoid^      geoEllipsoid = gcnew Ellipsoid (*ellipsoidDef, sourceLibrary);
                if (geoEllipsoid->Valid)
                    Nodes->Add (gcnew EllipsoidNode (geoEllipsoid->Name, contextMenu, geoEllipsoid, m_isUserLib));
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    InitializeImageKeys () override;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            DoPaste (SWF::IDataObject^ dataObject)
    {
    // NEEDSWORK
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            DeleteMember (EllipsoidNode^ node, SWF::Form^ hostForm)
    {
    // the node passed in represents the Ellipsoid to be deleted.
    // The call to Library->DeleteEllipsoid makes sure that the Ellipsoid is not in use by any of the GC's in this library (those are the only ones that can us it).
    BI::ScopedString    ellipsoidName (node->KeyName);
    StatusInt   status = m_library->DeleteEllipsoid (ellipsoidName.Uni());
    if (BGC::GEOCOORDERR_EllipsoidInUse == status)
        {
        SWF::MessageBox::Show (hostForm, GeoCoordinateLocalization::GetLocalizedString ("EllipsoidInUse"), 
                                GeoCoordinateLocalization::GetLocalizedString ("CannotDeleteEllipsoidTitle"), SWF::MessageBoxButtons::OK);
        return;
        }
    if (BGC::GEOCOORDERR_LibraryReadonly == status)
        {
        SWF::MessageBox::Show (hostForm, GeoCoordinateLocalization::GetLocalizedString ("LibraryReadOnly"), 
                                GeoCoordinateLocalization::GetLocalizedString ("CannotDeleteEllipsoidTitle"), SWF::MessageBoxButtons::OK);
        return;
        }
    Nodes->Remove (node);
    }

};



/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   05/10
+===============+===============+===============+===============+===============+======*/
private ref class    EllipsoidEditingPanel : SWF::Panel
{
private:
SWF::Form^                  m_hostForm;
SWF::Button^                m_doneButton;
SWF::Button^                m_editButton;
SWF::Button^                m_newButton;
SWF::Button^                m_deleteButton;
SWF::Panel^                 m_bottomPanel;

SWF::TreeView^              m_treeView;
GeoEllipsoidPropertyPane^   m_propertyPane;
GCSECObjectModel^           m_ecObjectModel;

SWF::ContextMenu^           m_contextMenu;
IHostFormProvider2^         m_hostFormProvider;
BGC::LibraryP               m_library;
String^                     m_userOrganizationName;

SWF::MenuItem^              m_cutMenuItem;
SWF::MenuItem^              m_copyMenuItem;
SWF::MenuItem^              m_pasteMenuItem;
SWF::MenuItem^              m_deleteFromLibraryMenuItem;
SWF::MenuItem^              m_editMenuItem;
bool                        m_needGCSReload;       // if datums are edited, the GCS's need to be reloaded, because otherwise they'll show stale parameters.

#if defined (DATUM_DRAGDROP)
// drag drop stuff
SelectionPanelTreeNode^     m_dragNode;
SD::Rectangle               m_dragRectangle;

static const int    SCROLLREGION    = 20;
static const int    SHIFT_KEY       = 4;    // from DragEventsArg documentation.
static const int    CTRL_KEY        = 8;
static const int    ALT_KEY         = 32;
#endif

public:
static String^        EllipsoidImageKey = "Ellipsoid";

private:
static const int    BTN_Y           = 12;
static const int    PANELHEIGHT     = 40;
static const int    PANELWIDTH      = 300;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    barry.bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidEditingPanel
(
SWF::Form^                  hostForm,
IHostFormProvider2^         hostFormProvider,
SWF::ImageList^             imageList,
BGC::LibraryP               library,
String^                     userOrganizationName
)
    {
    m_hostForm              = hostForm;
    m_hostFormProvider      = hostFormProvider;
    m_ecObjectModel         = gcnew GCSECObjectModel (false, false);
    m_library               = library;
    if (nullptr == imageList)
        imageList = GetDefaultImageList ();
    m_userOrganizationName  = userOrganizationName;
    InitializeComponent (imageList);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    barry.bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
~EllipsoidEditingPanel ()
    {
    // gets changed by compiler to Dispose method.
    // go through the treeview nodes and dispose of the BaseGeoCoordinateSystems within.
    for each (SWF::TreeNode^ node in m_treeView->Nodes)
        {
        GroupTreeNode^  groupNode;
        if (nullptr != (groupNode = dynamic_cast<GroupTreeNode^>(node)))
            delete groupNode;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
SWF::ImageList^ GetDefaultImageList ()
    {
    try
        {
        SWF::ImageList^ imageList   = gcnew SWF::ImageList();
        SD::Size        size        = imageList->ImageSize;

        imageList->ColorDepth       = SWF::ColorDepth::Depth32Bit;

        // get the Icon representing a closed folder and use it for the library, favorites, and group.
        SREF::Assembly^ assembly            = SREF::Assembly::GetExecutingAssembly ();
        SD::Icon^       folderClosedIcon    = nullptr;
        SD::Icon^       ellipsoidIcon       = nullptr;
        SIO::Stream^    stream;

        stream           = assembly->GetManifestResourceStream ("FolderClosed.ico");
        folderClosedIcon = gcnew SD::Icon (stream);

        stream       = assembly->GetManifestResourceStream ("GeoCoordEllipsoid.ico");
        ellipsoidIcon    = gcnew SD::Icon (stream);

        SWF::ImageList::ImageCollection^    images = imageList->Images;
        images->Add (GCSSelectionPanel::LibraryImageKey, folderClosedIcon);
        images->Add (EllipsoidImageKey,                  ellipsoidIcon);
        return imageList;
        }
    catch (System::Exception^)
        {
        return nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
property IHostFormProvider2^     HostFormProvider
{
IHostFormProvider2^ get() {return m_hostFormProvider;}
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
property bool                   NeedGCSReload
{
bool get() {return m_needGCSReload;}
}

private:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            InitializeComponent
(
SWF::ImageList^             imageList
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            SetSelectedEllipsoid (Ellipsoid^ ellipsoid)
    {
    m_propertyPane->Ellipsoid = ellipsoid;
    }

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            EditEllipsoid (Ellipsoid^ ellipsoid)
    {
    ellipsoid->CanEdit                  = true;
    SWF::Form^                form      = HostFormProvider->GetEditEllipsoidForm();
    gcnew EditEllipsoidFormContents (form, ellipsoid, true);
    SWF::DialogResult         result    = HostFormProvider->ShowModal (form);
    // put it back to uneditable so it doesn't show up as editable in the selection dialog.
    ellipsoid->CanEdit                      = false;
    delete form;

    // if the user hits OK, verify that the new Ellipsoid has a unique name.
    return (SWF::DialogResult::OK == result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            PopulateLibraryNode ()
    {
    // Go through the libraries and populate the tree.
    m_treeView->Nodes->Add (gcnew EllipsoidLibraryNode (gcnew String (m_library->GetGUIName().data()), m_contextMenu, m_library));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    NodeSelected
(
System::Object^,
SWF::TreeViewEventArgs^   eventArgs
)
    {
    EllipsoidNode^  ellipsoidNode;
    if (nullptr != (ellipsoidNode = dynamic_cast<EllipsoidNode^> (eventArgs->Node)))
        {
        m_propertyPane->Ellipsoid = ellipsoidNode->Ellipsoid;
        SetSelectedEllipsoid (ellipsoidNode->Ellipsoid);
        }
    else
        {
        SetSelectedEllipsoid (nullptr);
        }

    bool libraryReadOnly    = m_library->IsReadOnly();
    m_editButton->Enabled   = (nullptr != ellipsoidNode) && !libraryReadOnly;
    m_deleteButton->Enabled = (nullptr != ellipsoidNode) && !libraryReadOnly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void    NodeDoubleClicked
(
System::Object^                     sender,
SWF::TreeNodeMouseClickEventArgs^   eventArgs
)
    {
    EllipsoidNode^  ellipsoidNode;
    if (nullptr != (ellipsoidNode = dynamic_cast<EllipsoidNode^> (eventArgs->Node)))
        {
        m_propertyPane->Ellipsoid = ellipsoidNode->Ellipsoid;
        SetSelectedEllipsoid (ellipsoidNode->Ellipsoid);
        EditPropertiesClickEvent (sender, eventArgs);
        }
    else
        SetSelectedEllipsoid (nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void    NodeClicked
(
System::Object^,
SWF::TreeNodeMouseClickEventArgs^   eventArgs
)
    {
    // if it's a right click, we need to set the node selection, as it doesn't get set on right click otherwise.
    if (SWF::MouseButtons::Right == eventArgs->Button)
        m_treeView->SelectedNode = eventArgs->Node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            MenuPopupEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    // Here are the possibilities:
    // 1) EllipsoidLibraryNode that can't be editted.
    // 2) EllipsoidNode that can be editted. Ellipsoids in User Libraries are editable, Ellipsoids in the system library are not.

    SWF::ContextMenu^   menu;

    // sender should be the menu item.
    if (nullptr == (menu = dynamic_cast<SWF::ContextMenu^> (sender)) )
        return;

    if (nullptr == m_treeView)
        return;

    EllipsoidTreeNode^       treeNode;
    if (nullptr == (treeNode = dynamic_cast <EllipsoidTreeNode^> (m_treeView->SelectedNode)))
        return;

    SWF::Menu::MenuItemCollection^    menuItems = menu->MenuItems;
    menuItems->Clear();

    if (treeNode->CanCopyToClipboard)
        menuItems->Add (m_copyMenuItem);

    if (treeNode->CanPasteFromClipboard)
        menuItems->Add (m_pasteMenuItem);

    if (treeNode->CanDeleteFromLibrary)
        menuItems->Add (m_deleteFromLibraryMenuItem);

    if (treeNode->CanEdit)
        menuItems->Add (m_editMenuItem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            PasteClickEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    // get node and make sure it's a group (it should be)
    SWF::TreeNode^          selectedNode;
    if ( (nullptr == m_treeView) || (nullptr == (selectedNode = m_treeView->SelectedNode)) )
        return;

    EllipsoidLibraryNode^   libraryNode;
    if ( (nullptr == (libraryNode = dynamic_cast<EllipsoidLibraryNode^>(selectedNode))) &&
         (nullptr == (libraryNode = dynamic_cast<EllipsoidLibraryNode^>(selectedNode->Parent))) )
        return;

    libraryNode->DoPaste (SWF::Clipboard::GetDataObject());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            CopyMemberClickEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    // get node and make sure it's a group (it should be)
    EllipsoidNode^         ellipsoidNode;
    if ( (nullptr != m_treeView) && (nullptr != (ellipsoidNode = dynamic_cast<EllipsoidNode^>(m_treeView->SelectedNode))) )
        {
        SWF::Clipboard::SetDataObject (gcnew SWF::DataObject (gcnew MemberNodeInfo (ellipsoidNode->KeyName)));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            CutMemberClickEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    SWF::TreeNode^      selectedNode;
    if ( (nullptr == m_treeView) || (nullptr == (selectedNode = m_treeView->SelectedNode)) )
        return;

    EllipsoidNode^      ellipsoidNode;
    if ( (nullptr == (ellipsoidNode = dynamic_cast<EllipsoidNode^>(selectedNode))) )
        return;

    SWF::Clipboard::SetDataObject (gcnew SWF::DataObject (gcnew MemberNodeInfo (ellipsoidNode->KeyName)));

    // remove from the tree
    EllipsoidLibraryNode^  parentNode;
    if (nullptr != (parentNode = dynamic_cast<EllipsoidLibraryNode^>(ellipsoidNode->Parent)))
        parentNode->DeleteMember (ellipsoidNode, m_hostForm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DeleteFromLibraryClickEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    SWF::TreeNode^      selectedNode;
    if ( (nullptr == m_treeView) || (nullptr == (selectedNode = m_treeView->SelectedNode)) )
        return;

    EllipsoidNode^      ellipsoidNode;
    if ( (nullptr == (ellipsoidNode = dynamic_cast<EllipsoidNode^>(selectedNode))) )
        return;

    // remove from the tree
    EllipsoidLibraryNode^  parentNode;
    if (nullptr != (parentNode = dynamic_cast<EllipsoidLibraryNode^>(ellipsoidNode->Parent)))
        parentNode->DeleteMember (ellipsoidNode, m_hostForm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            EditPropertiesClickEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    SWF::TreeNode^      selectedNode;
    if ( (nullptr == m_treeView) || (nullptr == (selectedNode = m_treeView->SelectedNode)) )
        return;

    EllipsoidNode^      ellipsoidNode;
    if ( (nullptr == (ellipsoidNode = dynamic_cast<EllipsoidNode^>(selectedNode))) )
        return;

    // clone the original and edit it.
    Ellipsoid^      clone = gcnew Ellipsoid (ellipsoidNode->Ellipsoid);

    if (EditEllipsoid (clone))
        {
        // we need to update the node name (if the name changed), update the file, and update the info panel.
        StatusInt status = ellipsoidNode->Ellipsoid->ReplaceInLibrary (clone);
        if (BGC::GEOCOORDERR_EllipsoidIllegalName == status)
            {
            SWF::MessageBox::Show (m_hostForm, GeoCoordinateLocalization::GetLocalizedString ("IllegalEllipsoidName"),
                            GeoCoordinateLocalization::GetLocalizedString ("CannotReplaceEllipsoidTitle"), SWF::MessageBoxButtons::OK);
            return;
            }
        else if (BGC::GEOCOORDERR_LibraryReadonly == status)
            {
            SWF::MessageBox::Show (m_hostForm, GeoCoordinateLocalization::GetLocalizedString ("LibraryReadOnly"),
                                    GeoCoordinateLocalization::GetLocalizedString ("CannotReplaceEllipsoidTitle"), SWF::MessageBoxButtons::OK);
            return;
            }
        else if (BGC::GEOCOORDERR_EllipsoidNoUniqueName == status)
            {
            // we prevent this from happening in the editor.
            assert (false);
            return;
            }
        else if (BSISUCCESS != status)
            {
            assert (false);
            return;
            }
        
        ellipsoidNode->Ellipsoid = clone;
        ellipsoidNode->Text      = clone->Name;
        SetSelectedEllipsoid (ellipsoidNode->Ellipsoid);

        // the GCS's in the panel this came from will need to be refreshed.
        m_needGCSReload = true;
        }
    else
        {
        // restore the node to its original state.
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
Ellipsoid^          CreateNewEllipsoid (String^ nameTemplate)
    {
    BI::ScopedString    newNameTemplate (nameTemplate);
    CSEllipsoidDef*     createdEllipsoid;

    if (BSISUCCESS == m_library->CreateNewEllipsoid (createdEllipsoid, newNameTemplate.Uni()))
        {
        Ellipsoid^      newEllipsoid = gcnew Ellipsoid (*createdEllipsoid, m_library);
        newEllipsoid->Source         = m_userOrganizationName;
        newEllipsoid->Description    = GeoCoordinateLocalization::GetLocalizedString ("UserDefined");
        return newEllipsoid;
        }
    else
        {
        assert (false);
        return nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            NewButtonClicked
(
System::Object^         sender,
System::EventArgs^      eventArgs
)
    {
    String^         newNameTemplate = GeoCoordinateLocalization::GetLocalizedString ("User");
    Ellipsoid^      newEllipsoid    = CreateNewEllipsoid (newNameTemplate);

    if (nullptr == newEllipsoid)
        {
        assert (false);
        return;
        }

    // Allow the user to initialize the new Ellipsoid.
    if (EditEllipsoid (newEllipsoid))
        {
        // should have unique name since we enforce that while editing.
        StatusInt   status;
        if (BSISUCCESS != (status = newEllipsoid->AddToLibrary()))
            {
            if (BGC::GEOCOORDERR_EllipsoidIllegalName == status)
                {
                SWF::MessageBox::Show (m_hostForm, GeoCoordinateLocalization::GetLocalizedString ("IllegalEllipsoidName"), 
                                GeoCoordinateLocalization::GetLocalizedString ("CannotAddEllipsoidTitle"), SWF::MessageBoxButtons::OK);
                return;
                }
            else if (BGC::GEOCOORDERR_LibraryReadonly == status)
                {
                SWF::MessageBox::Show (m_hostForm, GeoCoordinateLocalization::GetLocalizedString ("LibraryReadOnly"), 
                                GeoCoordinateLocalization::GetLocalizedString ("CannotAddEllipsoidTitle"), SWF::MessageBoxButtons::OK);
                return;
                }
            else if (BGC::GEOCOORDERR_MaxUserLibraryEllipsoids == status)
                {
                SWF::MessageBox::Show (m_hostForm, GeoCoordinateLocalization::GetLocalizedString ("MaxUserEllipsoids"), 
                                GeoCoordinateLocalization::GetLocalizedString ("CannotAddEllipsoidTitle"), SWF::MessageBoxButtons::OK);
                return;
                }
            assert (false);
            return;
            }

        // make sure we can find it in the library.
        BI::ScopedString    ellipsoidName (newEllipsoid->Name);
        CSEllipsoidDef* ellipsoidDef;
        if (NULL == (ellipsoidDef = m_library->GetEllipsoid (ellipsoidName.Uni())))
            {
            assert (false);
            return;
            }

        Ellipsoid^      geoEllipsoid = gcnew Ellipsoid (*ellipsoidDef, m_library);
        m_treeView->Nodes[0]->Nodes->Add (gcnew EllipsoidNode (geoEllipsoid->Name, m_contextMenu, geoEllipsoid, true));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            CreateMenuItems ()
    {
    m_cutMenuItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("Cut"));
    m_cutMenuItem->Click += gcnew System::EventHandler (this, &EllipsoidEditingPanel::CutMemberClickEvent);

    m_copyMenuItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("Copy"));
    m_copyMenuItem->Click += gcnew System::EventHandler (this, &EllipsoidEditingPanel::CopyMemberClickEvent);

    m_pasteMenuItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("Paste"));
    m_pasteMenuItem->Click += gcnew System::EventHandler (this, &EllipsoidEditingPanel::PasteClickEvent);

    m_deleteFromLibraryMenuItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("DeleteFromLibrary"));
    m_deleteFromLibraryMenuItem->Click += gcnew System::EventHandler (this, &EllipsoidEditingPanel::DeleteFromLibraryClickEvent);

    m_editMenuItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("EditProperties"));
    m_editMenuItem->Click += gcnew System::EventHandler (this, &EllipsoidEditingPanel::EditPropertiesClickEvent);
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            EllipsoidEditingPanel::InitializeComponent
(
SWF::ImageList^             imageList
)
    {
    m_doneButton                        = gcnew SWF::Button();
    m_doneButton->DialogResult          = SWF::DialogResult::OK;
    m_doneButton->Location              = SD::Point(20, BTN_Y);
    m_doneButton->Size                  = SD::Size(96, 24);
    m_doneButton->TabIndex              = 10;
    m_doneButton->Text                  = GeoCoordinateLocalization::GetLocalizedString ("Done");

    m_editButton                        = gcnew SWF::Button();
    m_editButton->Location              = SD::Point(130, BTN_Y);
    m_editButton->Size                  = SD::Size(96, 24);
    m_editButton->TabIndex              = 12;
    m_editButton->Text                  = GeoCoordinateLocalization::GetLocalizedString ("Edit");
    m_editButton->Click                 += gcnew System::EventHandler (this, &EllipsoidEditingPanel::EditPropertiesClickEvent);
    m_editButton->Enabled               = false;

    m_newButton                         = gcnew SWF::Button();
    m_newButton->Location               = SD::Point(240, BTN_Y);
    m_newButton->Size                   = SD::Size(96, 24);
    m_newButton->TabIndex               = 13;
    m_newButton->Text                   = GeoCoordinateLocalization::GetLocalizedString ("New");
    m_newButton->Click                  += gcnew System::EventHandler (this, &EllipsoidEditingPanel::NewButtonClicked);

    m_deleteButton                      = gcnew SWF::Button();
    m_deleteButton->Location            = SD::Point(350, BTN_Y);
    m_deleteButton->Size                = SD::Size(96, 24);
    m_deleteButton->TabIndex            = 14;
    m_deleteButton->Text                = GeoCoordinateLocalization::GetLocalizedString ("Delete");
    m_deleteButton->Click               += gcnew System::EventHandler (this, &EllipsoidEditingPanel::DeleteFromLibraryClickEvent);
    m_deleteButton->Enabled             = false;

    m_hostForm->AcceptButton            = m_doneButton;

    m_bottomPanel                       = gcnew SWF::Panel();
    m_bottomPanel->Size                 = SD::Size(PANELWIDTH, PANELHEIGHT);
    m_bottomPanel->Dock                 = SWF::DockStyle::Bottom;

    m_bottomPanel->SuspendLayout();
    m_bottomPanel->Controls->Add (m_doneButton);
    m_bottomPanel->Controls->Add (m_editButton);
    m_bottomPanel->Controls->Add (m_newButton);
    m_bottomPanel->Controls->Add (m_deleteButton);
    m_bottomPanel->ResumeLayout();

    // Set up the Tree side of the SplitContainer
    m_treeView = gcnew SWF::TreeView ();
    m_treeView->HideSelection           = false;      // want selection hilited even when this treeView doesn't have focus.
    m_treeView->ImageList               = imageList;
    m_treeView->Dock                    = SWF::DockStyle::Fill;
    m_treeView->AfterSelect             += gcnew SWF::TreeViewEventHandler (this, &EllipsoidEditingPanel::NodeSelected);
    m_treeView->NodeMouseDoubleClick    += gcnew SWF::TreeNodeMouseClickEventHandler (this, &EllipsoidEditingPanel::NodeDoubleClicked);
    m_treeView->NodeMouseClick          += gcnew SWF::TreeNodeMouseClickEventHandler (this, &EllipsoidEditingPanel::NodeClicked);

    PopulateLibraryNode ();
    m_treeView->SelectedNode            = m_treeView->Nodes[0];
    m_treeView->Nodes[0]->Expand();

    m_propertyPane                      = gcnew GeoEllipsoidPropertyPane (nullptr, m_ecObjectModel, "GCSLibrary");
    m_propertyPane->LayoutPreset        = BUCWG::GroupLayoutPreset::StrictTopDown;
    m_propertyPane->Dock                = SWF::DockStyle::Fill;

    SWF::SplitContainer^  splitContainer = gcnew SWF::SplitContainer();
    splitContainer->Panel1->Controls->Add (m_treeView);
    splitContainer->Panel2->Controls->Add (m_propertyPane);
    splitContainer->Dock                = SWF::DockStyle::Fill;
    splitContainer->Size                = SD::Size(450,400);
    splitContainer->Panel1MinSize       = 150;
    splitContainer->Panel2MinSize       = 150;

    this->Dock                          = SWF::DockStyle::Fill;
    this->Size                          = SD::Size (450,450);

    // don't do the layout until we're done adding stuff.
    SuspendLayout();
    Controls->Add (splitContainer);
    Controls->Add (m_bottomPanel);
    ResumeLayout();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            EllipsoidNode::InitializeImageKeys ()
    {
    ImageKey            = EllipsoidEditingPanel::EllipsoidImageKey;
    SelectedImageKey    = EllipsoidEditingPanel::EllipsoidImageKey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void    EllipsoidLibraryNode::InitializeImageKeys ()
    {
    String^ key         = GCSSelectionPanel::LibraryImageKey;
    ImageKey            = key;
    SelectedImageKey    = key;
    }


