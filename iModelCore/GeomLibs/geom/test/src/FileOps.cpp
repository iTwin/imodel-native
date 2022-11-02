/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    BeTest::GetHost().GetOutputRoot(fileName);
    if (directoryName)
        {
        fileName.AppendToPath(directoryName);
        if (!BeFileName::IsDirectory(fileName.c_str()))
            BeFileName::CreateNewDirectory(fileName.c_str());
        }

    if (nameB)
        fileName.AppendToPath(nameB);
    if (nameC)
        fileName.AppendToPath(nameC);
    if (extension)
        fileName.AppendExtension(extension);

    BeFile file;
    if (BeFileStatus::Success == file.Create(fileName.c_str(), true))
        {
        uint32_t bytesWritten = 0;
        file.Write(&bytesWritten, string.c_str(), (uint32_t)string.size());
        file.Close();
        return true;
        }
    return false;
    }

bool GTestFileOps::WriteByteArrayToTextFile(bvector<Byte> &bytes, WCharCP directoryName, WCharCP nameB, WCharCP nameC, WCharCP extension)
    {
    BeFileName fileName;
    BeTest::GetHost().GetOutputRoot(fileName);
    if (directoryName)
        {
        fileName.AppendToPath(directoryName);
        if (!BeFileName::IsDirectory(fileName.c_str()))
            BeFileName::CreateNewDirectory(fileName.c_str());
        }

    if (nameB)
        fileName.AppendToPath(nameB);
    if (nameC)
        fileName.AppendToPath(nameC);
    if (extension)
        fileName.AppendExtension(extension);

    BeFile file;
    if (BeFileStatus::Success == file.Create(fileName.c_str(), true))
        {
        uint32_t bytesWritten = 0;
        static char const * beginArray = "[\n";
        static char const * endArray= "]\n";
        static char const * comma = ",";
        static char const * newLine = "\n";
        file.Write(&bytesWritten, beginArray, (uint32_t)strlen (beginArray));
        char string[1024];
        uint32_t bytesOnLine = 0;
        static uint32_t maxBytesOnLine = 120;
        for (size_t i = 0; i < bytes.size (); i++)
            {
            sprintf (string, "%d", bytes[i]);
            uint32_t newBytes = (uint32_t)strlen(string);
            if (newBytes + 1 + bytesOnLine > maxBytesOnLine)
                {
                file.Write(&bytesWritten, newLine, (uint32_t)strlen(newLine));
                bytesOnLine = 0;
                }
            file.Write(&bytesWritten, string, (uint32_t)strlen(string));
            if (i + 1 != bytes.size ())
                {
                file.Write(&bytesWritten, comma, (uint32_t)strlen(comma));
                }
            bytesOnLine += newBytes + 1;
            }
        file.Write(&bytesWritten, endArray, (uint32_t)strlen(endArray));
        file.Close();
        return true;
        }
    return false;
    }