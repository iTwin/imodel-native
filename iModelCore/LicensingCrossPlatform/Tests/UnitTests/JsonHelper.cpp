/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "JsonHelper.h"

BEGIN_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE

Json::Value ReadJsonFile(BeFileNameCR filename)
{
	if (filename.DoesPathExist())
	{
		BeFileStatus status;
		BeFile jsonFile;
		ByteStream stream;
		status = jsonFile.Open(filename, BeFileAccess::Read);
		if (status == BeFileStatus::Success)
		{
			status = jsonFile.ReadEntireFile(stream);
			if (status == BeFileStatus::Success)
			{
				Utf8String jsonString((Utf8CP)stream.GetData(), stream.GetSize());
				return Json::Value(jsonString);
			}
		}
	}
	return Json::Value("");
}

END_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE