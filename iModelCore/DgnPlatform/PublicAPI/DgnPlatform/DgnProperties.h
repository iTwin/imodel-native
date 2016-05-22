/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnProperties.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnPlatform.h>

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

BEGIN_BENTLEY_DGN_NAMESPACE

typedef BeSQLite::PropertySpec DbPropSpec;
typedef DbPropSpec::Mode DbPropTxnMode;

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/12
//=======================================================================================
struct DgnProjectProperty
{
    struct Spec : DbPropSpec
        {
        Spec (Utf8CP name, DbPropTxnMode setting) : DbPropSpec (name, PROPERTY_APPNAME_DgnProject, setting) {}
        };

    struct ProjectProperty : Spec {ProjectProperty (Utf8CP name) : Spec(name, DbPropSpec::Mode::Normal){}};
    struct ProjectSetting  : Spec {ProjectSetting (Utf8CP name)  : Spec(name, DbPropSpec::Mode::Setting){}};

    static ProjectProperty SchemaVersion()   {return ProjectProperty("SchemaVersion");}
    static ProjectProperty Name()            {return ProjectProperty("Name");}
    static ProjectProperty Description()     {return ProjectProperty("Description");}
    static ProjectProperty Client()          {return ProjectProperty("Client");}
    static ProjectProperty SeedProjectId()   {return ProjectProperty("SeedProjectId");}
    static ProjectProperty ProjectType()     {return ProjectProperty("ProjectType");}
    static ProjectProperty LastEditor()      {return ProjectProperty("LastEditor");}
    static ProjectProperty Extents()         {return ProjectProperty("Extents");}
    static ProjectProperty IsSpatialRedline() {return ProjectProperty("IsSpatialRedline");}
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

    struct ProjectProperty : Spec {ProjectProperty (Utf8CP name) : Spec(name, DbPropSpec::Mode::Normal){}};
    struct ProjectSetting  : Spec {ProjectSetting (Utf8CP name)  : Spec(name, DbPropSpec::Mode::Setting){}};

    static ProjectProperty DgnProjectAssociationData() {return ProjectProperty("DgnProjectAssociationData");}
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson   04/13
//=======================================================================================
struct RedlineModelProperty
{
    struct Spec : DbPropSpec
        {
        Spec (Utf8CP name, DbPropTxnMode setting, DbPropSpec::Compress compress) : DbPropSpec (name, PROPERTY_APPNAME_RedlineModel, setting, compress) {}
        };

    struct ProjectProperty : Spec {ProjectProperty (Utf8CP name, bool compressed=true) : Spec(name, DbPropSpec::Mode::Normal, compressed? DbPropSpec::Compress::Yes: DbPropSpec::Compress::No){}};
    struct ProjectSetting  : Spec {ProjectSetting (Utf8CP name) : Spec(name, DbPropSpec::Mode::Setting, DbPropSpec::Compress::Yes){}};

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
        Spec (Utf8CP name, DbPropTxnMode setting) : DbPropSpec (name, PROPERTY_APPNAME_DgnEmbeddedProject, setting, DbPropSpec::Compress::No) {}
        };

    struct ProjectProperty : Spec {ProjectProperty (Utf8CP name) : Spec(name, DbPropSpec::Mode::Normal){}};
    struct ProjectSetting  : Spec {ProjectSetting (Utf8CP name)  : Spec(name, DbPropSpec::Mode::Setting){}};

    static ProjectProperty SchemaVersion()   {return ProjectProperty("SchemaVersion");}
    static ProjectProperty Name()            {return ProjectProperty("Name");}
    static ProjectProperty Description()     {return ProjectProperty("Description");}
    static ProjectProperty Client()          {return ProjectProperty("Client");}
    static ProjectProperty LastEditor()      {return ProjectProperty("LastEditor");}
    static ProjectProperty Thumbnail()       {return ProjectProperty("Thumbnail");}
    static ProjectProperty CreationDate()    {return ProjectProperty("CreationDate");}
    
    // Added by Keith Bertram. 3/27/2015. It was decided that it was advantageous to have the LatestChangeSetId property 
    // available within the imodel so the application does not have to extract the idgndb just to determine if change sets
    // are available for the imodel.
    static ProjectProperty LatestChangeSetId()    {return ProjectProperty("LatestChangeSetId");}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/12
//=======================================================================================
struct DgnViewProperty
{
    struct Spec : DbPropSpec
        {
        Spec (Utf8CP name, DbPropTxnMode setting, Compress compress=Compress::Yes) : DbPropSpec (name, PROPERTY_APPNAME_DgnView, setting, compress) {}
        };

    struct ViewProperty : Spec {ViewProperty (Utf8CP name, Compress compress=Compress::Yes) : Spec(name, DbPropSpec::Mode::Normal, compress){}};
    struct ViewSetting  : Spec {ViewSetting (Utf8CP name)  : Spec(name, DbPropSpec::Mode::Setting){}};

    static ViewSetting Settings()     {return ViewSetting("Settings");}
    static ViewSetting DefaultView()  {return ViewSetting("DefaultView");}
    static ViewProperty Thumbnail()   {return ViewProperty("Thumbnail", DbPropSpec::Compress::No);}
    static ViewProperty Thumbnail2()   {return ViewProperty("Thumbnail2", DbPropSpec::Compress::No);}
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    10/12
//=======================================================================================
struct LineStyleProperty
{
    struct Spec : DbPropSpec
        {
        Spec (Utf8CP name, DbPropTxnMode setting, Compress compress=Compress::Yes) : DbPropSpec (name, PROPERTY_APPNAME_LineStyle, setting, compress) {}
        };

    struct ComponentProperty : Spec {ComponentProperty (Utf8CP name, Compress compress=Compress::Yes) : Spec(name, DbPropSpec::Mode::Normal, compress){}};

    static ComponentProperty Compound()         {return ComponentProperty("CompoundV1");}
    static ComponentProperty LineCode()         {return ComponentProperty("LineCodeV1");}
    static ComponentProperty LinePoint()        {return ComponentProperty("LinePointV1");}
    static ComponentProperty PointSym()         {return ComponentProperty("PointSymV1");}
    static ComponentProperty RasterComponent()  {return ComponentProperty("RasterComponentV1");}
    static ComponentProperty RasterImage()      {return ComponentProperty("RasterImageV1");}
};
END_BENTLEY_DGN_NAMESPACE

/** @endcond */
