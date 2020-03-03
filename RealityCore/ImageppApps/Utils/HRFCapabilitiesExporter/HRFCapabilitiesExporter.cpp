/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HRFCapabilitiesExporter/HRFCapabilitiesExporter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "HTMLExporter.h"
#include "TextExporter.h"
#include "CreatorParser.h"

#include <ImagePP/all/h/HRFFileFormats.h>
#include <Imagepp/all/h/ImageppLib.h>

using namespace std;

// Forward declarations
void InitImagePP();
void DisplayTemplateUsage();
void DisplayUsage(TCHAR* exeName);
void GenerateHTMLReport(const HRFRasterFileFactory::Creators& creators, 
                        otstream& output, 
                        tfstream& templateInput);
void GenerateHTMLReportWithFrames(HRFRasterFileFactory::Creators const& creators, 
                                  WString const framesPath);
void GenerateTextReport(const HRFRasterFileFactory::Creators& creators, otstream& output);



/*---------------------------------------------------------------------------------**//**
* @bsifunction                                                  Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int _tmain(int argc, TCHAR *argv[])
{
    bool isHTMLReport = false;
    bool pauseBeforeExit = false;
    bool silentMode = false;
    WString filename = _T("");
    WString templateFilename = _T("");
    WString framesPath = _T("");

    // Looping through arguments
    for(int i = 1; i < argc; ++i)
    {
        // HTML Report
        if (_tcscmp(argv[i], _T("-H")) == 0 || _tcsicmp(argv[i], _T("--html")) == 0)
        {
            isHTMLReport = true;
        }
        else
        // HTML Report with Frames
        if (_tcscmp(argv[i], _T("-f")) == 0 || _tcsicmp(argv[i], _T("--frames")) == 0)
        {
            ++i;
            if (i < argc)
            {
                isHTMLReport = true;
                framesPath += argv[i];
            }
            else
            {
                _tcerr << _T("Error: You must specify a path for the --frames option.\n");
                exit(EXIT_FAILURE);
            }
        }
        else    
        // Help
        if (_tcscmp(argv[i], _T("-h")) == 0 || _tcsicmp(argv[i], _T("--help")) == 0)
        {
            ++i;
            if (i < argc && _tcsicmp(argv[i], _T("template")) == 0)
            {
                // Template help
                DisplayTemplateUsage();
            }
            else
            {
                // Program Usage
                DisplayUsage(argv[0]);
            }
            exit(EXIT_SUCCESS);
        }
        else
        // Output to file
        if (_tcscmp(argv[i], _T("-o")) == 0 || _tcsicmp(argv[i], _T("--output")) == 0)
        {
            ++i;
            if (i < argc)
                filename += argv[i];
            else
            {
                _tcerr << _T("Error: You must specify a filename for the --output option.\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        // Pause at the end
        if (_tcscmp(argv[i], _T("-p")) == 0 || _tcsicmp(argv[i], _T("--pause")) == 0)
        {
            pauseBeforeExit = true;
        }
        else
        // Silent mode
        if (_tcscmp(argv[i], _T("-q")) == 0 || _tcsicmp(argv[i], _T("--quiet")) == 0)
        {
            silentMode = true;
        }
        else
        // Template file
        if (_tcscmp(argv[i], _T("-t")) == 0 || _tcsicmp(argv[i], _T("--template")) == 0)
        {
            ++i;
            if (i < argc)
                templateFilename += argv[i];
        }
        else    // Unknown parameter
        {
            // We display an error followed by the usage dialog
            _tcerr << _T("Error: ") << argv[i] << _T(" is an unknown parameter\n");
            DisplayUsage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    
    // Template option must be used with the HTML parameter
    if (!templateFilename.empty() && !isHTMLReport)
    {
        _tcerr << _T("Error: The --html option must be used when specifiying a template file (--template option).\n");
        exit(EXIT_FAILURE);
    }
    
    // Template option option cannot be used with Frames option
    if (!templateFilename.empty() && !framesPath.empty())
    {
        _tcerr << _T("Error: You cannot specify a template when using the --frames option.\n");
        exit(EXIT_FAILURE);
    }
    
    // Check for use of Frames and output filename
    if (!filename.empty() && !framesPath.empty())
    {
        _tcerr << _T("Error: You cannot specify an output file when using the --frames option.\n");
        exit(EXIT_FAILURE);
    }

    // Image++ Initialization
    InitImagePP();

    // Retrieving the available creators
    const HRFRasterFileFactory::Creators& Creators = HRFRasterFileFactory::GetInstance()->GetCreators(HFC_READ_WRITE);
    
    // Output to file
    tfstream filestr;
    if (!filename.empty())
    {
        filestr.open(filename.c_str(), ios_base::out);
        // Check for errors
        if (!filestr.is_open())
        {
            _tcerr << _T("Cannot open output file ") << filename.c_str() << _T(".\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Template file
    tfstream templatestr;
    if (!templateFilename.empty())
    {
        templatestr.open(templateFilename.c_str(), ios_base::in);
        // Check for errors
        if (!templatestr.is_open())
        {
            _tcerr << _T("Cannot open template file ") << templateFilename.c_str() << _T(".\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Frames path
    if (!framesPath.empty())
    {   
        //check if the directory exists, if it doesn't create it
        if(!PathFileExists(framesPath.c_str()))
            SHCreateDirectoryEx(NULL, framesPath.c_str(), NULL);
            
        
        if (PathIsDirectory(framesPath.c_str()))
        {
            // Checking write access for the specified path
            if (_taccess(framesPath.c_str(), 2) != 0)
            {
                _tcerr << _T("Error: You do not have write access to the specified directory");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            _tcerr << _T("Error: The specified path is not a directory.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Generating report
    if (isHTMLReport)
    {
        // HTML output
        if (!silentMode)
        {
            if (framesPath.empty())
                _tcout << _T("Generating HTML report...\n");
            else
                _tcout << _T("Generating HTML Frames report...\n");
        }
            
        if (filestr.is_open())
            GenerateHTMLReport(Creators, filestr, templatestr);
        else
        {
            if (!framesPath.empty())
            {
                // Verifying that the last character is a '\'
                size_t pos = framesPath.find_last_of(_T("\\"));
                if (pos != framesPath.length() - 1)
                    framesPath += _T("\\");
                    
                GenerateHTMLReportWithFrames(Creators, framesPath);
            }
            else
                GenerateHTMLReport(Creators, _tcout, templatestr);
        }
    } 
    else
    {   
        // Text output
        if (!silentMode)
            _tcout << _T("Generating text report...\n");
        if (filestr.is_open())
            GenerateTextReport(Creators, filestr);
        else
            GenerateTextReport(Creators, _tcout);
    }
    
    // Closing output file
    if (filestr.is_open())
        filestr.close();
        
    // Closing the template file
    if (templatestr.is_open())
        templatestr.close();
    
    if (!silentMode)    
        _tcerr << _T("\nReport generated successfully.\n");
    
    // Pause if necessary
    if (pauseBeforeExit)
    {
        _tcout << _T("\nPress a key to continue...");
        _getch();
    }
    //Terminate ImagePP lib host
    ImagePP::ImageppLib::GetHost().Terminate(true);
    
    return EXIT_SUCCESS;
}

IMPLEMENT_DEFAULT_IMAGEPP_LIBHOST(MyImageppLibHost)


/*---------------------------------------------------------------------------------**//**
* @bsifunction                                                  Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void InitImagePP()
{
#ifdef IPP_USING_STATIC_LIBRARIES
    #error TODO resource support.
    // Image++ Resource Loader initialization
    //HINSTANCE hInst = GetModuleHandle(NULL);
    //HFCResourceLoader::GetInstance()->SetModuleInstance(hInst);
#endif
//Initialize ImagePP host
ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());

}

/*---------------------------------------------------------------------------------**//**
* @bsifunction                                                  Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayTemplateUsage()
{
    _tcerr << _T("Bentley Systems HRF Capabilities Explorer.\n\n");
    _tcerr << _T("To create a custom report using an HTML template, use the --template (or -t)\n");
    _tcerr << _T("combined with the HTML generator switch (--html).\n\n");
    _tcerr << _T("Your template may contain the following tags:\n\n");
    _tcerr << _T(" ") << TEMPLATE_TAG_HEADER;
    _tcerr << _T("           : Will contain the list of all supported formats with \n");
    _tcerr << _T("                          clickable links to specifications tables.\n");
    _tcerr << _T(" ") << TEMPLATE_TAG_CONTENT;
    _tcerr << _T("          : The tables containing the formats specifications will\n");
    _tcerr << _T("                          replace this tag.\n");
    _tcerr << _T(" ") << TEMPLATE_TAG_FOOTER;
    _tcerr << _T("           : Will contain the footer of the document.\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsifunction                                                  Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayUsage(TCHAR* exeName)
{
    _tcerr << _T("Bentley Systems HRF Capabilities Explorer.\n");
    _tcerr << _T("Generates a specification sheet for all image formats available in Image++.\n\n");
    _tcerr << _T("usage: ") << exeName <<  _T(" [options]\n\n");
    _tcerr << _T("where options are:\n\n");
    _tcerr << _T(" -H            : Generates an HTML report of the available capabilities\n");
    _tcerr << _T("                 (also --html)\n");
    _tcerr << _T(" -t            : Allows the use of a template file when using the HTML option.\n");
    _tcerr << _T("                 (also --template)\n");
    _tcerr << _T(" -f <path>     : Generates an HTML report using Frames. The files are created\n");
    _tcerr << _T("                 in the specified path (also --frames <path> ).\n");
    _tcerr << _T("                 Use --help template for more information.\n");
    _tcerr << _T(" -o <filename> : Specifies where to save the report (also --output).\n");
    _tcerr << _T(" -p            : Pauses the application once the report is generated.\n");
    _tcerr << _T(" -q            : Quiet mode. No messages are printed when using this flag.\n");
    _tcerr << _T("                 The report is still printed if -o is not specified.\n");
    _tcerr << _T(" -h            : Print this help message and exit (also --help).\n\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsifunction                                                  Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void GenerateTextReport(HRFRasterFileFactory::Creators const& creators, otstream& output)
{
    TextExporter textExporter(output);
    
    textExporter.PrintHeader();
    // Looping through creators and printing their information
    for (uint32_t index = 0; index < creators.size(); index++)
    {
        HRFRasterFileCreator* pCreator = (HRFRasterFileCreator*)creators[index];
        HASSERT(pCreator != NULL);

        textExporter.SetRasterCreator(*pCreator);
        textExporter.PrintElement();
    }
    textExporter.PrintFooter();
}

/*---------------------------------------------------------------------------------**//**
* @bsifunction                                                  Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ParseCapabilities(const HRFRasterFileFactory::Creators& creators, CreatorParser& parser)
{
    // Looping through creators and printing their information
    for (uint32_t index = 0; index < creators.size(); index++)
    {
        HRFRasterFileCreator* pCreator = (HRFRasterFileCreator*)creators[index];
        HASSERT(pCreator != NULL);
        
        parser.ScanFileCreator(*pCreator);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsifunction                                                  Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void GenerateHTMLReport(const HRFRasterFileFactory::Creators& creators, otstream& output, tfstream& templateInput)
{
    bool useTemplate = templateInput.is_open();
    
    HTMLExporter htmlExporter;
    CreatorParser creatorParser;
    
    ParseCapabilities(creators, creatorParser);
    
    ImageFormatMap const imageFormats = creatorParser.GetImageFormats();
    
    // Generating the HTML report
    if (!useTemplate)
        {
        // Generating the HTML report with complete header & footer
        htmlExporter.Export(imageFormats, output);
    }
    else
    {
        htmlExporter.Export(imageFormats, templateInput, output);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsifunction                                                  Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void GenerateHTMLReportWithFrames(HRFRasterFileFactory::Creators const& creators, WString const framesPath)
{
    HTMLExporter htmlExporter;
    CreatorParser creatorParser;
    
    ParseCapabilities(creators, creatorParser);
    
    ImageFormatMap const imageFormats = creatorParser.GetImageFormats();
    htmlExporter.Export(imageFormats, framesPath);
}
