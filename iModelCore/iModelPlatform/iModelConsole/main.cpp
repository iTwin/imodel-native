/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <DgnPlatform/PlatformLib.h>
#include "iModelConsole.h"
#include <Bentley/Logging.h>

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <mutex>

// Maximum size for input files (8MB)
#define MAX_INPUT_SIZE (8 * 1024 * 1024)

#ifdef COMMENT_OUT_UNUSED_VARIABLE
static WCharCP s_configFileName = L"logging.config.xml";
#endif

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE_EC

// Function declaration
extern "C" __declspec(dllexport) void fuzz(char *bimFilePath, char *sqlFilePath);

void SafeLog(const std::string& message) {
    std::mutex& consoleMutex = IModelConsole::GetConsoleMutex();
    std::lock_guard<std::mutex> lock(consoleMutex);
    std::cerr << message << std::endl;
}

std::string convertWCharToString(WCharCP wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void IModelConsoleBeAssertHandler(wchar_t const* message, wchar_t const* file, unsigned line, BeAssertFunctions::AssertType atype)
    {
    WString errorMessage;
    errorMessage.Sprintf(L"ASSERTION FAILURE: %ls (%ls:%d)\n", message, file, line);

    NativeLogging::CategoryLogger("BeAssert").errorv(errorMessage.c_str());
    IModelConsole::WriteErrorLine(Utf8String(errorMessage).c_str());
    }

bool TryGetLogConfigPath(BeFileNameR logConfigPath, BeFileNameCR exeDir);

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InitLogging(BeFileNameCR exeDir)
    {
    }

// Fuzzing target function
void fuzz(char *bimFilePath, char *sqlFilePath) {
    if (!bimFilePath || !sqlFilePath) {
        SafeLog("Error: Null input path(s)");
        return;
    }

    // SafeLog("INFO: Processing BIM file: " + std::string(bimFilePath) + ", SQL file: " + std::string(sqlFilePath));
    
    FILE *fp = NULL;
    char *sample_bytes = NULL;
    
    // Open and read SQL file
    if (fopen_s(&fp, sqlFilePath, "rb") != 0 || !fp) {
        // SafeLog("Error opening SQL file: " + std::string(sqlFilePath));
        return;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Safety checks
    if (file_size <= 0) {
        // SafeLog("Error: Empty or invalid SQL file");
        fclose(fp);
        return;
    }

    if (file_size > MAX_INPUT_SIZE) {
        // SafeLog("Error: SQL file too large");
        fclose(fp);
        return;
    }

    // Allocate buffer and read file
    sample_bytes = (char *)malloc(file_size);
    if (!sample_bytes) {
        // SafeLog("Error: Memory allocation failed");
        fclose(fp);
        return;
    }

    size_t bytes_read = fread(sample_bytes, 1, file_size, fp);
    if (bytes_read != (size_t)file_size) {
        // SafeLog("Error: Failed to read entire SQL file");
        free(sample_bytes);
        fclose(fp);
        return;
    }

    // SafeLog("INFO: Processing SQL of size: " + std::to_string(file_size) + " bytes");

    // Execute query with both BIM file path and SQL query
    IModelConsole::Singleton().ExecuteSampleQuery(bimFilePath, sample_bytes);

    // Clean up resources in reverse order of allocation
    free(sample_bytes);     // Free memory first
    fclose(fp);             // Close file last
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int wmain(int argc, WCharCP argv[])
    {
#ifdef _WIN32
#if defined (UNICODE_OUTPUT_FOR_TESTING)
    // turning this on makes it so we can show unicode characters, but screws up piped output for programs like python.
    _setmode(_fileno(stdout), _O_U16TEXT);  // so we can output any and all unicode to the console
    _setmode(_fileno(stderr), _O_U16TEXT);  // so we can output any and all unicode to the console
#endif
#endif

    if (argc != 3) {
        // SafeLog("Usage: " + convertWCharToString(argv[0]) + " <bim_file_path> <sql_file_path>");
        return 1;
    }

    BeFileName exeDir = Desktop::FileSystem::GetExecutableDir();
    if (!exeDir.DoesPathExist()) {
        // SafeLog("Error: Executable directory not found");
        return 1;
    }

    InitLogging(exeDir);

    // C++ programs start-up with the "C" locale in effect by default, and the "C" locale does not support conversions of any characters outside
    // the "basic character set". ... The call to setlocale() says "I want to use the user's default narrow string encoding". This encoding is
    // based on the Posix-locale for Posix environments. In Windows, this encoding is the ACP, which is based on the system-locale.
    // However, the success of this code is dependent on two things:
    //      1) The narrow encoding must support the wide character being converted.
    //      2) The font/gui must support the rendering of that character.
    // In Windows, #2 is often solved by setting cmd.exe's font to Lucida Console."
    // (http://cboard.cprogramming.com/cplusplus-programming/145590-non-english-characters-cout-2.html)
    setlocale(LC_CTYPE, "");

    //set customized assert handler as default handler causes exception which makes iModelConsole crash.
    BeAssertFunctions::SetBeAssertHandler(IModelConsoleBeAssertHandler);

    Dgn::PlatformLib::Initialize(IModelConsole::Singleton());

    // Convert both file paths
    std::string bimFilePath = convertWCharToString(argv[1]);
    std::string sqlFilePath = convertWCharToString(argv[2]);
    
    // std::cerr << "INFO: BIM file path: " << bimFilePath << std::endl;
    // std::cerr << "INFO: SQL file path: " << sqlFilePath << std::endl;

    // For Jackalope fuzzing - handle @@ placeholder
    if (sqlFilePath == "@@") {
        // SafeLog("Error: Direct @@ placeholder passed. This should be replaced by Jackalope.");
        return 1;
    }

    fuzz(const_cast<char*>(bimFilePath.c_str()), const_cast<char*>(sqlFilePath.c_str()));
    return 0;
}

#ifdef __unix__
int main(int argc, char** argv) {
    BentleyApi::bvector<WCharCP> argv_w_ptrs;
    for (int i = 0; i < argc; i++) {
        BentleyApi::WString argw(argv[i], BentleyApi::BentleyCharEncoding::Utf8);
        auto argp = new wchar_t[argw.size() + 1];
        wcscpy(argp, argw.data());
        argv_w_ptrs.push_back(argp);
    }

    return wmain(argc, argv_w_ptrs.data());
}
#endif
