/*--------------------------------------------------------------------------------------+
|
|     $Source: all/gra/hrf/Test/HRFGeoTiffCompressedTable/test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFGeoTiffCoordSysTable.h>



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int main(int argc, char* argv[])
    {
    int ExitCode = 0;


    HRFGeoTiffCoordSysTable GeoTiffCoordSysTable;

    cout << "Table definition" << endl << endl;

    cout << GeoTiffCoordSysTable.GetColumnCount() << " columns " << endl;

    HRFGeoTiffCompressedTable::ColumnsInfo TableColumnsInfo = GeoTiffCoordSysTable.GetColumnsInfo();

    HRFGeoTiffCompressedTable::ColumnsInfo::const_iterator Itr(TableColumnsInfo.begin());
    vector<string> SearchColumn;
    while (Itr != TableColumnsInfo.end())
        {
        cout << Itr->m_ColumnStartPos << "  " << Itr->m_ColumnSize << "   " << Itr->m_ColumnName << endl;
        SearchColumn.push_back(Itr->m_ColumnName);
        Itr++;
        }

    char Command[1024];

    char ColumnName[1024];
    char SearchKey[1024];
    GeoTiffCoordSysTable.LockTable();
    vector<string> SearchResult;

    cout << "Valid command are 's' for select and 'q' for quit" << endl << endl;
    cout << "command : ";
    gets(Command);

    while (BeStringUtilities::Stricmp(Command, "Q") != 0)
        {
        if (BeStringUtilities::Stricmp(Command, "S") == 0)
            {
            cout << "Column Name : ";
            gets(ColumnName);
            cout << "Search key : ";
            gets(SearchKey);

            cout << endl;

            GeoTiffCoordSysTable.GetValues(ColumnName, SearchKey, SearchColumn, &SearchResult);

            vector<string>::const_iterator ResultItr(SearchResult.begin());
            while (ResultItr != SearchResult.end())
                {
                cout << *ResultItr << "  ";
                ResultItr++;
                }
            cout << endl;
            }

        cout << "Command : ";
        gets(Command);
        }
    GeoTiffCoordSysTable.ReleaseTable();

    return ExitCode;
    }