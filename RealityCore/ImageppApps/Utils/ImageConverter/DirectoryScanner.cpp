/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageConverter/DirectoryScanner.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ImageConverterPch.h"
#include <tchar.h>
#include "DirectoryScanner.h"
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <stdio.h>
#include <io.h>



DirectoryScanner::DirectoryScanner(bool pi_ScanSubdir)
{
    m_ScanSubdir = pi_ScanSubdir;
}

DirectoryScanner::~DirectoryScanner()
{
    
}

DirectoryScanner::FILELIST DirectoryScanner::GetFileList(const Utf8String& pi_rPath)
{
    struct _wfinddata_t FileInfo;

    intptr_t hSearch;

    FILELIST List;
    EXTENSIONLIST ExtList;
    
    Utf8String FindPath;
    Utf8String CurDir;
   
    WChar Drive[_MAX_DRIVE];
    WChar Dir[_MAX_DIR];
    WChar FileName[_MAX_FNAME];
    WChar Ext[_MAX_EXT];

    _tsplitpath(WString(pi_rPath.c_str(), BentleyCharEncoding::Utf8).c_str(), Drive, Dir, FileName, Ext);

    // All supported files
    if( Ext[0] == 0x00 || Utf8String(Ext) == ".*")
    {
        HRFRasterFileFactory::Creators  ListFile = HRFRasterFileFactory::GetInstance()->GetCreators(HFC_READ_ONLY);

        for(size_t i=0; i < ListFile.size(); i++)
        {
            if (((HRFRasterFileCreator*)ListFile[i])->SupportsScheme(HFCURLFile::s_SchemeName()) &&
                ((HRFRasterFileCreator*)ListFile[i])->GetExtensions().c_str() != 0)
            {
                Utf8String ImageExt = ((HRFRasterFileCreator*)ListFile[i])->GetExtensions().c_str();
                
                if( ImageExt.find_last_of(';') != ImageExt.size() - 1 )
                    ImageExt += ";";
                
                AddExtension(ImageExt, ExtList);
            }
        }
    }
    else
    {
        Utf8String Extension = Utf8String(Ext) + ";";

        AddExtension(Extension, ExtList);
    }

    CurDir  = Utf8String(Drive);
    CurDir += Utf8String(Dir);

    FindPath = CurDir;
    FindPath += "*.*";

    hSearch = _tfindfirst(const_cast<WChar*>(WString(FindPath.c_str(), BentleyCharEncoding::Utf8).c_str()), &FileInfo);
	    
    if( hSearch != -1 )
	{
        // Insert all the file into the list
        do
        {
            // skip "." and ".."
            if( (_tcscmp(FileInfo.name, _TEXT(".")) != 0) && 
                (_tcscmp(FileInfo.name, _TEXT("..")) != 0) )
            {
                Utf8String Name;
                
                Name = CurDir;
                Name += Utf8String(FileInfo.name);

                if( (FileInfo.attrib & _A_SUBDIR) != _A_SUBDIR )
                {
                    if( IsCompliant(ExtList, Name) )
                        List.push_back(Name);
                }
                else if( m_ScanSubdir )
                {
                    if( Ext[0] != 0x00 )
                    {
                        Name += "\\*";
                        Name += Utf8String(Ext);
                    }
                    else
                        Name += "\\";

                    FILELIST NewList = GetFileList(Name);
                    List.merge(NewList);
                }
            }
        } while( _tfindnext(hSearch, &FileInfo) == 0 );
    	
        _findclose(hSearch);
	}

    return List;
}

bool DirectoryScanner::IsCompliant(const EXTENSIONLIST& pi_rList, const Utf8String& pi_rName)
{
	EXTENSIONLIST::const_iterator Itr = pi_rList.begin();
    bool Done = false;
    WChar Ext[_MAX_EXT];

    _tsplitpath(WString(pi_rName.c_str(), BentleyCharEncoding::Utf8).c_str(), 0, 0, 0, Ext);

    Utf8String LowerExt(Ext + 1);
    WString LowerExtW(LowerExt.c_str(), BentleyCharEncoding::Utf8);
    CaseInsensitiveStringToolsW().ToLower(LowerExtW);
    

    while( Itr != pi_rList.end() && !Done )
    {
        if( (*Itr) == LowerExt )
            Done = true;
        else
            Itr++;
    }

    return Done;
}

void DirectoryScanner::AddExtension(Utf8String& pi_rExt, EXTENSIONLIST& pi_rList)
{
    WString ExtWide(pi_rExt.c_str(), BentleyCharEncoding::Utf8);
    CaseInsensitiveStringToolsW().ToLower(ExtWide);

    Utf8String::size_type Pos = 0;
    Utf8String::size_type StartPos = Pos;

    Pos = pi_rExt.find(";", StartPos);

    // Check the case there is only one pattern
    if( Pos == Utf8String::npos && pi_rExt.size() > 0 )
    {
        // Keep only the pattern, eleminate 
        // the * and . if found
        Utf8String Pattern;
        Utf8String::size_type DotPos = pi_rExt.find('.');

        if( DotPos != Utf8String::npos )
            Pattern = pi_rExt.substr(DotPos + 1, pi_rExt.size() - (DotPos + 1));
        else
            Pattern = pi_rExt;
        
        pi_rList.push_back(Pattern);
    }

    while( Pos != Utf8String::npos )
    {
        Utf8String CurrentPattern = pi_rExt.substr(StartPos, (Pos - StartPos));
        
        // Keep only the pattern, eleminate 
        // the * and . if found
        Utf8String::size_type DotPos = CurrentPattern.find('.');
        if( DotPos != Utf8String::npos )
            CurrentPattern = CurrentPattern.substr(DotPos + 1, CurrentPattern.size() - (DotPos + 1));
        
        pi_rList.push_back(CurrentPattern);

        StartPos = Pos + 1;
        Pos = pi_rExt.find(";", StartPos);
    }
}
