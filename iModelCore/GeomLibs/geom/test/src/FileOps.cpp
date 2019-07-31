/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "FileOps.h"
#include <Bentley/BeTest.h>

bool GTestFileOps::ReadAsString (BeFileName &filename, Utf8String &string)
    {

    string.clear ();
    BeFile file;
    if (BeFileStatus::Success == file.Open (filename, BeFileAccess::Read))
        {
        bvector<Byte> bytes;
        if (BeFileStatus::Success == file.ReadEntireFile (bytes))
            {
            for (auto b : bytes)
                string.push_back (b);
            return true;
            }
        }
    return false;
    }

bool GTestFileOps::WriteToFile(Utf8String &string, WCharCP directoryName, WCharCP nameB, WCharCP nameC, WCharCP extension)
    {
    BeFileName fileName;
    BeTest::GetHost ().GetOutputRoot (fileName);
    if (directoryName)
        {
        fileName.AppendToPath (directoryName);
        if (!BeFileName::IsDirectory (fileName.c_str ()))
            BeFileName::CreateNewDirectory (fileName.c_str ());
        }

    if (nameB)
        fileName.AppendToPath (nameB);
    if (nameC)
        fileName.AppendToPath (nameC);
    if (extension)
        fileName.AppendExtension (extension);

    BeFile file;
    if (BeFileStatus::Success == file.Create (fileName.c_str (), true))
        {
        uint32_t bytesWritten = 0;
        file.Write(&bytesWritten, string.c_str(), (uint32_t)string.size());
        file.Close ();
        return true;
        }
    return false;
    }