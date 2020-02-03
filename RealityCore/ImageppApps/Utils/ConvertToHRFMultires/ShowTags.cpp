//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/ConvertToHRFMultires/ShowTags.cpp $
//:>    $RCSfile: ConvertToHRFMultires.cpp,v $
//:>   $Revision: 1.4 $
//:>       $Date: 2012/02/27 13:15:34 $
//:>     $Author: Donald.Morissette $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <Imagepp/h/ImageppAPI.h>

#include <Imagepp/all/h/HRFMacros.h>

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HFCURLFile.h>

#include <Imagepp/all/h/HRFGeoTiffFile.h>
#include <Imagepp/all/h/HRFJpegFile.h>
#include <Imagepp/all/h/HRFTiffFile.h>

USING_NAMESPACE_IMAGEPP

struct MyImageppLibHost : ImagePP::ImageppLib::Host 
{
    virtual void _RegisterFileFormat() override 
        {
        HOST_REGISTER_FILEFORMAT(HRFTiffCreator)
        HOST_REGISTER_FILEFORMAT(HRFGeoTiffCreator)
        HOST_REGISTER_FILEFORMAT(HRFJpegCreator)
        }

}; // MyImageppLibHost


//-----------------------------------------------------------------------------
// This function print out the usage for the current program.
//-----------------------------------------------------------------------------
void ShowUsage()
{
    // Check that we have the right number of parameters.
    cout << "ShowTags V1.0 --- " << __DATE__ << endl
         <<  endl
         << "Usage:  ShowTags <drive:\\SourceFile>" << endl
         <<  endl;
}

//--------------------------------------------------------------------------------
// Main
//--------------------------------------------------------------------------------
void main(int argc, char *argv[])
{

    //Initialize ImagePP host
    ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());

    {
    // Source          
    HFCPtr<HRFRasterFile>           pSource;     
    HFCPtr<HFCURLFile>              SrcFileName;

    // Check that we have the right number of parameters.
    if (argc != 2)
        {
        ShowUsage();
        exit(1);
        }

    // Open the source file
    Utf8String path = Utf8String(argv[1]);
    Utf8String fileName(HFCURLFile::s_SchemeName() + "://" + path);
    SrcFileName  = new HFCURLFile(fileName);
    pSource = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)SrcFileName); 
    
    if (pSource == 0)
        {
        cout << "Can't open the source file." << endl;
        exit(1);
        }

    // Image descriptor
    HFCPtr<HRFPageDescriptor>       pSrcPageDescriptor = pSource->GetPageDescriptor(0);

    // Display each tag.
    HPMAttributeSet::HPMASiterator TagIterator;
    for (TagIterator = pSrcPageDescriptor->GetTags().begin(); TagIterator != pSrcPageDescriptor->GetTags().end(); TagIterator++)
        {
        HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);

        printf ("%s : %s\n", pTag->GetName().c_str(), pTag->GetDataAsString().c_str());
        }

        // Extract data value
        HRFAttributeGPSLatitude const* pGPSValueTag = pSrcPageDescriptor->FindTagCP<HRFAttributeGPSLatitude>();
        HRFAttributeBrightnessValue const* pBrightValueTag = pSrcPageDescriptor->FindTagCP<HRFAttributeBrightnessValue>();
        if (pGPSValueTag != NULL)
        {
            vector<double> GPS(pGPSValueTag->GetData());
            for (int i=0; i<GPS.size(); ++i)
                printf("GPS Value : %d: %lf\n", i, GPS[i]);
        }

        if (pBrightValueTag != NULL)
        {
            double val = pBrightValueTag->GetData();
            printf("Bright Value : %lf\n", val);
        }


    } // Close the file                                               


    //Terminate ImagePP lib host
    ImagePP::ImageppLib::GetHost().Terminate(true);

}
