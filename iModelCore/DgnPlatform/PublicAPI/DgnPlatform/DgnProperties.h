/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnProperties.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnPlatform.h>

#define PROPERTY_APPNAME_LineStyle   "dgn_LStyle"
#define PROPERTY_APPNAME_DgnView     "dgn_View"

#define PROPERTY_APPNAME_DgnDb          "dgn_Db"        // replaces PROPERTY_APPNAME_DgnProject="dgn_Proj" from releases older than 2.0
#define PROPERTY_APPNAME_DgnEmbeddedDb  "pkge_dgnDb"    // replaces PROPERTY_APPNAME_DgnEmbeddedProject="pkge_dgnProj" from releases older than 2.0

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
        Spec(Utf8CP name, DbPropTxnMode setting) : DbPropSpec(name, PROPERTY_APPNAME_DgnDb, setting) {}
    };

    struct ProjectProperty : Spec {ProjectProperty(Utf8CP name) : Spec(name, DbPropSpec::Mode::Normal){}};

    static ProjectProperty ProfileVersion()  {return ProjectProperty("SchemaVersion");}
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
    static ProjectProperty LastSchemaUpgradeInPhase() { return ProjectProperty("LastSchemaUpgradeInPhase"); }
    static ProjectProperty ConnectedContextId() {return ProjectProperty("ConnectedContextId");} // Optional: the stringified GUID of the associated Connected Context
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    03/13
//=======================================================================================
struct DgnEmbeddedProjectProperty
{
    struct Spec : DbPropSpec
    {
        Spec(Utf8CP name, DbPropTxnMode setting) : DbPropSpec(name, PROPERTY_APPNAME_DgnEmbeddedDb, setting, DbPropSpec::Compress::No) {}
    };

    struct ProjectProperty : Spec {ProjectProperty(Utf8CP name) : Spec(name, DbPropSpec::Mode::Normal){}};

    static ProjectProperty ProfileVersion() { return ProjectProperty("SchemaVersion"); }
    static ProjectProperty Name()            {return ProjectProperty("Name");}
    static ProjectProperty Description()     {return ProjectProperty("Description");}
    static ProjectProperty Client()          {return ProjectProperty("Client");}
    static ProjectProperty LastEditor()      {return ProjectProperty("LastEditor");}
    static ProjectProperty Thumbnail()       {return ProjectProperty("Thumbnail");}
    static ProjectProperty CreationDate()    {return ProjectProperty("CreationDate");}
    
    // Added by Keith Bertram. 3/27/2015. It was decided that it was advantageous to have the LatestChangeSetId property 
    // available within the imodel so the application does not have to extract the ibim just to determine if change sets
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
        Spec(Utf8CP name, DbPropTxnMode setting, Compress compress=Compress::Yes) : DbPropSpec(name, PROPERTY_APPNAME_DgnView, setting, compress) {}
    };

    struct ViewProperty : Spec {ViewProperty(Utf8CP name, Compress compress=Compress::Yes) : Spec(name, DbPropSpec::Mode::Normal, compress){}};

    static ViewProperty DefaultView()    {return ViewProperty("DefaultView");}
    static ViewProperty ViewThumbnail() {return ViewProperty("Thumbnail", DbPropSpec::Compress::No);}
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    10/12
//=======================================================================================
struct LineStyleProperty
{
    struct Spec : DbPropSpec
    {
        Spec(Utf8CP name, DbPropTxnMode setting, Compress compress=Compress::Yes) : DbPropSpec(name, PROPERTY_APPNAME_LineStyle, setting, compress) {}
    };

    struct ComponentProperty : Spec {ComponentProperty(Utf8CP name, Compress compress=Compress::Yes) : Spec(name, DbPropSpec::Mode::Normal, compress){}};

    static ComponentProperty Compound()         {return ComponentProperty("CompoundV1");}
    static ComponentProperty LineCode()         {return ComponentProperty("LineCodeV1");}
    static ComponentProperty LinePoint()        {return ComponentProperty("LinePointV1");}
    static ComponentProperty PointSym()         {return ComponentProperty("PointSymV1");}
    static ComponentProperty RasterComponent()  {return ComponentProperty("RasterComponentV1");}
    static ComponentProperty RasterImage()      {return ComponentProperty("RasterImageV1");}
};
END_BENTLEY_DGN_NAMESPACE

/** @endcond */
