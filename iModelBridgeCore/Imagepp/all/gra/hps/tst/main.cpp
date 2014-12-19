//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/tst/main.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Asks for HPS file name.
// Asks for HOD destination file name.
// Asks for cloning or referencing.
// Open the store for that file, saves all objects to the other store.

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <fstream>
#include <sstream>
#include <Imagepp/all/h/HPSParser.h>
#include <Imagepp/all/h/HODDatabase.h>
#include <Imagepp/all/h/HFCLocalBlockStream.h>
#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>
#include <Imagepp/all/h/HPSObjectStore.h>
#include <Imagepp/all/h/HPMClassFilter.h>

#define USE_HPSSTORE 1

//---------------------------------------------------------------------------
#include <Imagepp/all/h/HRARaster.h>
#include <Imagepp/all/h/HPMPersistentObject.h>

class ImageShadow : public HPMPersistentObject, public HFCShareableObject
    {
    HPM_DECLARE_CLASS(1384)

public:
    // constructors and destructors
    ImageShadow();
    ImageShadow(const HFCPtr<HRARaster>& pi_pRaster);
    ~ImageShadow();

private:

    HFCPtr<HRARaster> m_pRaster;
    };

HPM_REGISTER_CLASS(ImageShadow, HPMPersistentObject)

ImageShadow::ImageShadow()
    {
    }

ImageShadow::ImageShadow(const HFCPtr<HRARaster>& pi_pRaster)
    {
    m_pRaster = pi_pRaster;
    }

ImageShadow::~ImageShadow()
    {
    }
//---------------------------------------------------------------------------

char* pInputBuffer =
    "FirstPicture = IMAGEFILE(\"file://e:\\bb\\misc\\f_zip.hmr\")\n"           \
    "SecondPicture = IMAGEFILE(\"file://e:\\bb\\misc\\96060_25.hmr\")\n"       \
    "FinalMosaic = MOSAIC(FirstPicture, SecondPicture)\n"                      \
    "PAGES (FirstPicture, SecondPicture, FinalMosaic)";

void main(int argc, char** argv)
    {
    string SourceName;
    string DestName;
    string AnswerCloning;
    if (argc == 3)
        {
        SourceName = argv[1];
        DestName = argv[2];
        }
    else
        {
        cout << "Fichier HPS a traiter: ";
        cin >> SourceName;
        cout << endl << "Fichier HOD a produire: ";
        cin >> DestName;
        cout << endl << "C pour copier, R pour referer: ";
        cin >> AnswerCloning;
        cout << endl;
        }

#if USE_HPSSTORE

        {
        HFCPtr<HFCURL> pURL = new HFCURLFile(string("file://" + SourceName));
        HFCPtr<HPSObjectStore> pSourceStore = new HPSObjectStore(&g_DefaultLog, pURL);
        HPMClassFilter<HRARaster> Filter(pSourceStore);
        HFCPtr<HODDatabase> pDatabase = new HODDatabase(&g_DefaultLog, new HFCLocalBlockStream(DestName, HFC_READ_WRITE_CREATE));

        pDatabase->Save(pSourceStore->GetWorldCluster());

        if ((AnswerCloning == "C") || (AnswerCloning == "c"))
            {
            while (Filter.ObjectAvailable())
                {
                HFCPtr<HRARaster> pRaster = ((HRARaster*)(Filter.Load()))->Clone(pDatabase);
                ImageShadow* pShadow = new ImageShadow(pRaster);
                pDatabase->Save(pShadow);
                delete pShadow;
                }
            }
        else
            {
            list<HFCPtr<ImageShadow> > ToDelete;
            while (Filter.ObjectAvailable())
                {
                HFCPtr<ImageShadow> pShadow = new ImageShadow((HRARaster*)(Filter.Load()));
                pDatabase->Save(pShadow);
                ToDelete.push_back(pShadow);
                }
            }
        }

#else

        {
        HPSParser Parser;
        HPSNode* pNode;
        if (SourceName != ".")
            {
            ifstream InputFile(SourceName.c_str());
            pNode = (HPSNode*)(Parser.Parse(InputFile));
            }
        else
            {
            wistringstream InputFile(pInputBuffer);
            pNode = (HPSNode*)(Parser.Parse(InputFile));
            }

        HFCPtr<HODDatabase> pDatabase = new HODDatabase(&g_DefaultLog, new HFCLocalBlockStream(DestName, HFC_READ_WRITE_CREATE));
        pDatabase->Save(pNode->GetWorldCluster());

        if ((AnswerCloning == "C") || (AnswerCloning == "c"))
            {
            HPSNode::ObjectList::const_iterator itr = pNode->GetObjectList().begin();
            while (itr != pNode->GetObjectList().end())
                {
                HFCPtr<HRARaster> pRaster = ((HRARaster*)(*itr))->Clone(pDatabase);
                ImageShadow* pShadow = new ImageShadow(pRaster);
                pDatabase->Save(pShadow);
                delete pShadow;
                ++itr;
                }
            }
        else
            {
            HPSNode::ObjectList::const_iterator itr = pNode->GetObjectList().begin();
            while (itr != pNode->GetObjectList().end())
                {
                ImageShadow* pShadow = new ImageShadow((HRARaster*)(*itr));
                pDatabase->Save(pShadow);
                delete pShadow;
                ++itr;
                }
            }
        }

#endif
    }