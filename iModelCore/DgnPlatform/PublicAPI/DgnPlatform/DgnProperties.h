/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnProperties.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#define PROPERTY_APPNAME_DgnFile     "dgn_File"
#define PROPERTY_APPNAME_DgnFont     "dgn_Font"
#define PROPERTY_APPNAME_DgnLevel    "dgn_Level"
#define PROPERTY_APPNAME_LineStyle   "dgn_LStyle"
#define PROPERTY_APPNAME_DgnMaterial "dgn_Material"
#define PROPERTY_APPNAME_DgnModel    "dgn_Model"
#define PROPERTY_APPNAME_DgnProject  "dgn_Proj"
#define PROPERTY_APPNAME_DgnSession  "dgn_Sessn"
#define PROPERTY_APPNAME_DgnView     "dgn_View"
#define PROPERTY_APPNAME_Provenance  "dgn_Prov"
#define PROPERTY_APPNAME_DgnEmbeddedProject "pkge_dgnProj"
#define PROPERTY_APPNAME_DgnMarkupProject "dgnMarkup_Proj"
#define PROPERTY_APPNAME_RedlineModel "dgnMarkup_RedlineModel"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef BeSQLite::PropertySpec DbPropSpec;
typedef DbPropSpec::ProperyTxnMode DbPropTxnMode;

//=======================================================================================
// Persistent DgnFile-based properties and settings
// @bsiclass                                                    Keith.Bentley   02/12
//=======================================================================================
struct DgnFileProperty
{
    struct Spec : DbPropSpec
        {
        Spec (Utf8CP name, DbPropTxnMode setting) : DbPropSpec (name, PROPERTY_APPNAME_DgnFile, setting) {}
        };
    struct FileProperty : Spec {FileProperty (Utf8CP name) : Spec(name, DbPropSpec::TXN_MODE_Normal){}};

    struct CachedProperty : Spec {CachedProperty (Utf8CP name) : Spec(name, DbPropSpec::TXN_MODE_Cached){}};
    static CachedProperty NextGraphicGroup() {return CachedProperty("NextGG");}
    static CachedProperty NextTextNode()     {return CachedProperty("NextTN");}
    static FileProperty SchemaVersion()      {return FileProperty("SchemaVersion");}
};

//=======================================================================================
// Persistent model-based property names that are defined at the "system" level. Only properties created and maintained by
// DgnPlatform code should appear in this list. Applications should create a parallel property name list, but
// should choose an appropriate Namespace string (which of course should NOT start with "dgn_").
// @bsiclass                                                    Keith.Bentley   03/11
//=======================================================================================
struct DgnModelProperty
{
    struct Spec : DbPropSpec
        {
        Spec (Utf8CP name, DbPropTxnMode setting) : DbPropSpec (name, PROPERTY_APPNAME_DgnModel, setting) {}
        };

    struct ModelProperty : Spec {ModelProperty (Utf8CP name) : Spec(name, DbPropSpec::TXN_MODE_Normal){}};
    struct ModelSetting  : Spec {ModelSetting (Utf8CP name)  : Spec(name, DbPropSpec::TXN_MODE_Setting) {}};

    static ModelSetting  ModelSettings()       {return ModelSetting("ModelSettings");}
    static ModelProperty Author()              {return ModelProperty("Author");}
    static ModelProperty Title()               {return ModelProperty("Title");}
    static ModelProperty Subject()             {return ModelProperty("Subject");}
    static ModelProperty Keywords()            {return ModelProperty("Keywords");}
    static ModelProperty Comments()            {return ModelProperty("Comments");}
    static ModelProperty LastAuthor()          {return ModelProperty("LastAuthor");}
    static ModelProperty EditTime()            {return ModelProperty("EditTime");}
    static ModelProperty LastPrinted()         {return ModelProperty("LastPrinted");}
    static ModelProperty ModelProperties()     {return ModelProperty("ModelProps");}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/12
//=======================================================================================
struct DgnProjectProperty
{
    struct Spec : DbPropSpec
        {
        Spec (Utf8CP name, DbPropTxnMode setting) : DbPropSpec (name, PROPERTY_APPNAME_DgnProject, setting) {}
        };

    struct ProjectProperty : Spec {ProjectProperty (Utf8CP name) : Spec(name, DbPropSpec::TXN_MODE_Normal){}};
    struct ProjectSetting  : Spec {ProjectSetting (Utf8CP name)  : Spec(name, DbPropSpec::TXN_MODE_Setting){}};

    static ProjectProperty SchemaVersion()   {return ProjectProperty("SchemaVersion");}
    static ProjectProperty Name()            {return ProjectProperty("Name");}
    static ProjectProperty Description()     {return ProjectProperty("Description");}
    static ProjectProperty Client()          {return ProjectProperty("Client");}
    static ProjectProperty SeedProjectId()   {return ProjectProperty("SeedProjectId");}
    static ProjectProperty ColorTable()      {return ProjectProperty("ColorTable");}
    static ProjectProperty ProjectType()     {return ProjectProperty("ProjectType");}
    static ProjectProperty LastEditor()      {return ProjectProperty("LastEditor");}
    static ProjectProperty Extents()         {return ProjectProperty("Extents");}
    static ProjectProperty IsPhysicalRedline() {return ProjectProperty("IsPhysicalRedline");}
    static ProjectProperty Units()           {return ProjectProperty("Units");}
    static ProjectProperty DgnGCS()          {return ProjectProperty("DgnGCS");}
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson   04/13
//=======================================================================================
struct DgnMarkupProjectProperty
{
    struct Spec : DbPropSpec
        {
        Spec (Utf8CP name, DbPropTxnMode setting) : DbPropSpec (name, PROPERTY_APPNAME_DgnMarkupProject, setting) {}
        };

    struct ProjectProperty : Spec {ProjectProperty (Utf8CP name) : Spec(name, DbPropSpec::TXN_MODE_Normal){}};
    struct ProjectSetting  : Spec {ProjectSetting (Utf8CP name)  : Spec(name, DbPropSpec::TXN_MODE_Setting){}};

    static ProjectProperty DgnProjectAssociationData() {return ProjectProperty("DgnProjectAssociationData");}
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson   04/13
//=======================================================================================
struct RedlineModelProperty
{
    struct Spec : DbPropSpec
        {
        Spec (Utf8CP name, DbPropTxnMode setting, WantCompress compress) : DbPropSpec (name, PROPERTY_APPNAME_RedlineModel, setting, compress) {}
        };

    struct ProjectProperty : Spec {ProjectProperty (Utf8CP name, bool compressed=true) : Spec(name, DbPropSpec::TXN_MODE_Normal, compressed? DbPropSpec::COMPRESS_PROPERTY_Yes: DbPropSpec::COMPRESS_PROPERTY_No){}};
    struct ProjectSetting  : Spec {ProjectSetting (Utf8CP name) : Spec(name, DbPropSpec::TXN_MODE_Setting, DbPropSpec::COMPRESS_PROPERTY_Yes){}};

    static ProjectProperty RedlineECInstanceId() {return ProjectProperty("RedlineECInstanceId");}
    static ProjectProperty ViewId() {return ProjectProperty("ViewId");}
    static ProjectProperty ImageData2 () {return ProjectProperty("ImageData2",/*compressed*/false);}
    static ProjectProperty ImageData () {return ProjectProperty("ImageData",/*compressed*/true);}
    static ProjectProperty ImageDef() {return ProjectProperty("ImageDef");}
    static ProjectProperty RDLSheetDef(){return ProjectProperty("RDLSheetDef");}
    static ProjectProperty DgnProjectAssociationData() {return ProjectProperty("DgnProjectAssociationData");}
};


//=======================================================================================
// @bsiclass                                                    John.Gooding    03/13
//=======================================================================================
struct DgnEmbeddedProjectProperty
{
    struct Spec : DbPropSpec
        {
        Spec (Utf8CP name, DbPropTxnMode setting) : DbPropSpec (name, PROPERTY_APPNAME_DgnEmbeddedProject, setting, DbPropSpec::COMPRESS_PROPERTY_No) {}
        };

    struct ProjectProperty : Spec {ProjectProperty (Utf8CP name) : Spec(name, DbPropSpec::TXN_MODE_Normal){}};
    struct ProjectSetting  : Spec {ProjectSetting (Utf8CP name)  : Spec(name, DbPropSpec::TXN_MODE_Setting){}};

    static ProjectProperty SchemaVersion()   {return ProjectProperty("SchemaVersion");}
    static ProjectProperty Name()            {return ProjectProperty("Name");}
    static ProjectProperty Description()     {return ProjectProperty("Description");}
    static ProjectProperty Client()          {return ProjectProperty("Client");}
    static ProjectProperty BoundProjectFormat() {return ProjectProperty("BoundProjectFormat");}
    static ProjectProperty LastEditor()      {return ProjectProperty("LastEditor");}
    static ProjectProperty Thumbnail()       {return ProjectProperty("Thumbnail");}
    static ProjectProperty CreationDate()    {return ProjectProperty("CreationDate");}
    static ProjectProperty ExpirationDate()  {return ProjectProperty("ExpirationDate");}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/12
//=======================================================================================
struct DgnViewProperty
{
    struct Spec : DbPropSpec
        {
        Spec (Utf8CP name, DbPropTxnMode setting, WantCompress compress=COMPRESS_PROPERTY_Yes) : DbPropSpec (name, PROPERTY_APPNAME_DgnView, setting, compress) {}
        };

    struct ViewProperty : Spec {ViewProperty (Utf8CP name, WantCompress compress=COMPRESS_PROPERTY_Yes) : Spec(name, DbPropSpec::TXN_MODE_Normal, compress){}};
    struct ViewSetting  : Spec {ViewSetting (Utf8CP name)  : Spec(name, DbPropSpec::TXN_MODE_Setting){}};

    static ViewSetting Settings()     {return ViewSetting("Settings");}
    static ViewSetting DefaultView()  {return ViewSetting("DefaultView");}
    static ViewProperty Thumbnail()   {return ViewProperty("Thumbnail", DbPropSpec::COMPRESS_PROPERTY_No);}
};

//=======================================================================================
// @bsiclass                                                    MattGooding     10/12
//=======================================================================================
struct DgnMaterialProperty
{
    struct Spec : DbPropSpec
        {
        Spec (Utf8CP name, DbPropTxnMode setting) : DbPropSpec (name, PROPERTY_APPNAME_DgnMaterial, setting) {}
        };

    struct MaterialProperty : Spec {MaterialProperty (Utf8CP name) : Spec(name, DbPropSpec::TXN_MODE_Normal){}};
    struct MaterialSetting  : Spec {MaterialSetting (Utf8CP name)  : Spec(name, DbPropSpec::TXN_MODE_Setting){}};

    static MaterialSetting XmlMaterial()            {return MaterialSetting("XmlMaterial");}
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    10/12
//=======================================================================================
struct LineStyleProperty
{
    struct Spec : DbPropSpec
        {
        Spec (Utf8CP name, DbPropTxnMode setting) : DbPropSpec (name, PROPERTY_APPNAME_LineStyle, setting) {}
        };

    struct ComponentProperty : Spec {ComponentProperty (Utf8CP name) : Spec(name, DbPropSpec::TXN_MODE_Normal){}};

    static ComponentProperty Compound()         {return ComponentProperty("CompoundV1");}
    static ComponentProperty LineCode()         {return ComponentProperty("LineCodeV1");}
    static ComponentProperty LinePoint()        {return ComponentProperty("LinePointV1");}
    static ComponentProperty PointSym()         {return ComponentProperty("PointSymV1");}
    static ComponentProperty SymbolElements()   {return ComponentProperty("SymbolEleV1");}
    static ComponentProperty SymbolXGraphics()  {return ComponentProperty("SymbolXGraphicsV1");}
};
END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
