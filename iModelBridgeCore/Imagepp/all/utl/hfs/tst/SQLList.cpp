//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfs/tst/SQLList.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <Imagepp/all/h/HFSSQLListLister.h>
#include <Imagepp/all/h/HFSHIBPSQLLister.h>
#include <Imagepp/all/h/HFCInternetConnection.h>
#include <Imagepp/all/h/HFCHTTPConnection.h>
#include <Imagepp/all/h/HFCSocketConnection.h>


int main(int argc, char** argv)
    {
    if (argc != 3)
        {
        printf ("Usage : SQLList <ServerUrl> <Query>\n");
        return 1;
        }

    bool Connected;
    string Server(argv[1]);
    HFCPtr<HFCInternetConnection> pConnection;
    try
        {
        string Protocol(argv[1], 3);
        if (BeStringUtilities::Stricmp(Protocol.c_str(), "iip") == 0)
            pConnection = new HFCSocketConnection(argv[1]);
        else
            pConnection = new HFCHTTPConnection(argv[1]);

        Connected = pConnection->Connect(string(""), string(""), 30000);


        }
    catch(...)
        {
        printf("Invalid Server URL\n");
        }

    if (Connected)
        {
        try
            {
            HFSHIBPSQLLister Lister(pConnection);
            Lister.CanUseHIBP();
            if (Lister.List(argv[2]))
                {
                if (Lister.GotoFirst())
                    {
                    do
                        {
                        printf("%s\n", Lister.GetEntry().GetEntryName().c_str());
                        }
                    while (Lister.GotoNext());
                    }
                }
            }
        catch(...)
            {
            printf("Invalid SQL Command\n");
            }
        }
    else
        printf ("Enable to connect\n");

    return 0;
    }