/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

//--------------------------------------------------------------------------------------+
// Schema definitions for ECDbFileInfo.02.00.00.ecschema.xml
//--------------------------------------------------------------------------------------+

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

#define SCHEMA_ECDbFileInfo                                 "ECDbFileInfo"

#define CLASS_FileInfo                                      "FileInfo"
#define CLASS_FileInfo_PROPERTY_Name                        "Name"
#define CLASS_FileInfo_PROPERTY_Size                        "Size"
#define CLASS_FileInfo_PROPERTY_Description                 "Description"
#define CLASS_FileInfo_PROPERTY_LastModified                "LastModified"

#define CLASS_ExternalFileInfo                              "ExternalFileInfo"
#define CLASS_ExternalFileInfo_PROPERTY_RootFolder          "RootFolder"
#define CLASS_ExternalFileInfo_PROPERTY_RelativePath        "RelativePath"

#define CLASS_FileInfoOwnership                             "FileInfoOwnership"

#define ECSql_ExternalFileInfoClass                         "[ecdbf].[" CLASS_ExternalFileInfo "]"
#define ECSql_FileInfoOwnership                             "[ecdbf].[" CLASS_FileInfoOwnership "]"

END_BENTLEY_WEBSERVICES_NAMESPACE
