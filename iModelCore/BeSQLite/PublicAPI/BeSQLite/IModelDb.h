/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "BeSQLite.h"

#define PROPERTY_APPNAME_Package "pkge_Main"
#define PROPERTY_APPNAME_IModel  "imodel"

BEGIN_BENTLEY_SQLITE_NAMESPACE
enum IModelSchemaValues
    {
    PACKAGE_CURRENT_VERSION_Major = 1,
    PACKAGE_CURRENT_VERSION_Minor = 1,
    PACKAGE_CURRENT_VERSION_Sub1  = 0,
    PACKAGE_CURRENT_VERSION_Sub2  = 0,

    PACKAGE_SUPPORTED_VERSION_Major = PACKAGE_CURRENT_VERSION_Major,  // oldest version of the package schema supported by current api
    PACKAGE_SUPPORTED_VERSION_Minor = PACKAGE_CURRENT_VERSION_Minor,
    PACKAGE_SUPPORTED_VERSION_Sub1  = 0,
    PACKAGE_SUPPORTED_VERSION_Sub2  = 0,
    };

//=======================================================================================
//! Standard properties that an i-model publisher program should add to the .imodel
//! and .ibim files that it creates.
// @bsiclass                                                    Shaun.Sewall    05/13
//=======================================================================================
struct IModelProperty
{
    struct Spec : PropertySpec
        {
        Spec(Utf8CP name) : PropertySpec(name, PROPERTY_APPNAME_IModel, Mode::Normal) {}
        };

    static Spec PublisherProgram()  {return Spec("PublisherProgram");}
    static Spec PublisherVersion()  {return Spec("PublisherVersion");}
    static Spec IModelType()        {return Spec("imodelType");}
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    03/13
//=======================================================================================
struct PackageProperty
{
    struct Spec : PropertySpec
        {
        Spec(Utf8CP name) : PropertySpec(name, PROPERTY_APPNAME_Package, Mode::Normal) {}
        };

    static Spec ProfileVersion()  {return Spec("SchemaVersion");}
    static Spec Name()            {return Spec("Name");}
    static Spec Description()     {return Spec("Description");}
    static Spec Client()          {return Spec("Client");}
};

END_BENTLEY_SQLITE_NAMESPACE
