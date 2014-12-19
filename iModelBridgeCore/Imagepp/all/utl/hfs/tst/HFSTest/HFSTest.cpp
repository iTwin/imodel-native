//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfs/tst/HFSTest/HFSTest.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFSHIB.h>
#include <Imagepp/all/h/HFSWinDrive.h>
#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCURLCommonInternet.h>
#include <Imagepp/all/h/HFSItemIterator.h>


void List(const HFCPtr<HFSItem>& pi_rpItem)
    {
    HAutoPtr<HFSItemIterator> pItr;
    HFCPtr<HFSItem> pFolderItem;
    if (pi_rpItem != 0)
        {
        if (pi_rpItem->IsFolder())
            {
            pItr = (HFSItemIterator*)pi_rpItem->CreateIterator();

            if ((pFolderItem = pItr->GetFirstItem()) != 0)
                {
                do
                    {
                    printf("%30s\t%s\n", pFolderItem->GetName().c_str(), pFolderItem->IsFolder()? "<DIR>":"<FILE>");
                    }
                while ((pFolderItem = pItr->GetNextItem()) != 0);
                }
            }
        else
            printf("%30s\t%s\n", pi_rpItem->GetName().c_str(), "<FILE>");
        }
    else
        printf("Invalid path\n");
    }

int32_t main(uint32_t pi_Argc, char* pi_ppArgv[])
    {

    if (pi_Argc != 2)
        {
        printf("Usage : HFSTest URL\n");
        exit(0);
        }

    HFCPtr<HFCURL> pUrl = HFCURL::Instanciate(pi_ppArgv[1]);
    if (pUrl == 0)
        {
        printf("Invalid URL\n");
        exit(0);
        }

    WORD wVersionRequested = MAKEWORD( 2, 2 );
    WSADATA wsaData;
    int err;
    err = WSAStartup( wVersionRequested, &wsaData );
    if (err != 0 )
        {
        printf("Winsock init error!\n");
        exit(0);
        }
#if 0
    if (!AfxSocketInit())
        {
        printf("Winsock init error!\n");
        exit(0);
        }
#endif


    HAutoPtr<HFSFileSystem> pFileSystem;


    if (pUrl->IsCompatibleWith(HFCURLFile::CLASS_ID))
        pFileSystem = new HFSWinDrive(pUrl, 0);
    else if (pUrl->IsCompatibleWith(HFCURLCommonInternet::CLASS_ID))
        pFileSystem = new HFSHIB(pUrl, 5000);

    char Command[128];
    string Path;
    HAutoPtr<HFSItemIterator> pItr;
    HFCPtr<HFSItem> pFolder;
    HFCPtr<HFSItem> pFolderItem;
    while (gets(Command) != 0 && BeStringUtilities::Stricmp(Command, "quit") != 0)
        {
        if (BeStringUtilities::Stricmp(Command, "dir") == 0)
            List(pFileSystem->GetCurrentFolder());
        else if (BeStringUtilities::Strnicmp(Command, "dir ", 4) == 0)
            {
            // validate the drive name, if exist
            List(pFileSystem->GetItem(Command + 4));
            }
        else if (BeStringUtilities::Stricmp(Command, "cd") == 0)
            printf("%s\n", pFileSystem->GetCurrentFolder()->GetPath().c_str());
        else if (BeStringUtilities::Strnicmp(Command, "cd ", 3) == 0)
            pFileSystem->ChangeFolder(Command + 3);
        else
            printf("Invalid command\n");


        printf("\n\n");
        }

    if (WSACleanup())
        {
        printf("Socket clean up error : %d, please contact technical support.\n", WSAGetLastError());
        }

    return 0;
    }