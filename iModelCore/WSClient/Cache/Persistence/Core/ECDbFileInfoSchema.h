/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Core/ECDbFileInfoSchema.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//--------------------------------------------------------------------------------------+
// Schema definitions for ECDbFileInfo.02.00.00.ecschema.xml
//--------------------------------------------------------------------------------------+

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

#define SCHEMA_ECDbFileInfo                                 "ECDbFileInfo"

#define CLASS_FileInfo                                      "FileInfo"
#define CLASS_FileInfo_PROPERTY_Name                        "name"
#define CLASS_FileInfo_PROPERTY_Size                        "size"
#define CLASS_FileInfo_PROPERTY_Description                 "description"
#define CLASS_FileInfo_PROPERTY_LastModified                "lastModified"

#define CLASS_ExternalFileInfo                              "ExternalFileInfo"
#define CLASS_ExternalFileInfo_PROPERTY_RootFolder          "rootFolder"
#define CLASS_ExternalFileInfo_PROPERTY_RelativePath        "relativePath"

#define CLASS_FileInfoOwnership                             "FileInfoOwnership"

#define ECSql_ExternalFileInfoClass                         "[ecdbf].[" CLASS_ExternalFileInfo "]"
#define ECSql_FileInfoOwnership                             "[ecdbf].[" CLASS_FileInfoOwnership "]"

END_BENTLEY_WEBSERVICES_NAMESPACE
