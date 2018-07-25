/*--------------------------------------------------------------------------------------+
|
|     $Source: docs/samplecode/Formats.sample.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ Overview_Formats_Include.sampleCode
#include <ECObjects/ECObjectsAPI.h>
//__PUBLISH_EXTRACT_END__

USING_NAMESPACE_BENTLEY_EC

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 07/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus CreateFormat()
	{
	//__PUBLISH_EXTRACT_START__ Overview_Formats_CreateFormat.sampleCode
    ECSchemaPtr schema;
    ECFormatP format;
    ECSchema::CreateSchema(schema, "FormatSchema", "format", 5, 0, 6);
    Formatting::NumericFormatSpec spec = Formatting::NumericFormatSpec();
    schema->CreateFormat(format, "TestFormat", "TestDisplayLabel", "TestDescription", &spec);
    //__PUBLISH_EXTRACT_END__
    return BentleyStatus::SUCCESS;
	}