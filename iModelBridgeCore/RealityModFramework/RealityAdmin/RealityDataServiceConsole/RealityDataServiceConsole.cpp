/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/RealityDataServiceConsole/RealityDataServiceConsole.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#include <RealityAdmin/RealityDataServiceConsole.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

int main(int argc, char* argv[])
    {
    SetConsoleTitle("RealityDataService Navigator");

    char* substringPosition;
    std::string substring;
    BeFileName infile = BeFileName("");
    BeFileName outfile = BeFileName("");
    Utf8String server = "";
    Utf8String projectId = "";

    if (argc == 3)
        {
        for (int i = 0; i < argc; ++i)
            {
            if (strstr(argv[i], "-i:") || strstr(argv[i], "--infile:"))
                {
                substringPosition = strstr(argv[i], ":");
                substringPosition++;
                substring = std::string(substringPosition);
                infile = BeFileName(substring.c_str());
                }
            else if (strstr(argv[i], "-o:") || strstr(argv[i], "--outfile:"))
                {
                substringPosition = strstr(argv[i], ":");
                substringPosition++;
                substring = std::string(substringPosition);
                outfile = BeFileName(substring.c_str());
                }
            else if (strstr(argv[i], "-s:") || strstr(argv[i], "--server:"))
                {
                substringPosition = strstr(argv[i], ":");
                substringPosition++;
                server = std::string(substringPosition).c_str();
                }
            else if (strstr(argv[i], "-p:") || strstr(argv[i], "--projectId:"))
                {
                substringPosition = strstr(argv[i], ":");
                substringPosition++;
                projectId = std::string(substringPosition).c_str();
                }
            }
        }

    RealityDataConsole console = RealityDataConsole();
    if (!server.empty() && !projectId.empty())
        {
        console.Run(server, projectId);
        return 0;
        }
    else if (infile.empty() || !infile.DoesPathExist() || outfile.empty())
        {
        console.Run();
        return 0;
        }

    console.Run(infile, outfile);
    return 0;
    }
