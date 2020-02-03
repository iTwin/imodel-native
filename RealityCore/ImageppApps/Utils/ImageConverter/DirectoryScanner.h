/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageConverter/DirectoryScanner.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef __DirectoryScanner_H__
#define __DirectoryScanner_H__

#include <Imagepp/h/ImageppAPI.h>


class DirectoryScanner
{
    public:
        
        typedef list<Utf8String, allocator<Utf8String> > FILELIST;
        typedef FILELIST::iterator FILELIST_ITR;
        typedef FILELIST EXTENSIONLIST;
        typedef EXTENSIONLIST::iterator EXTENSIONLIST_ITR;

        // Construction - destruction
            DirectoryScanner(bool pi_ScanSubdir = false);
        virtual 
            ~DirectoryScanner();

        FILELIST
            GetFileList(const Utf8String& pi_rPath);


    protected:
    
    private:
        
        // Not implemented
            DirectoryScanner(const DirectoryScanner&);
            DirectoryScanner& operator=(const DirectoryScanner&);

        void 
            AddExtension(Utf8String& pi_rExt, EXTENSIONLIST& pi_rList);

        bool
            IsCompliant(const EXTENSIONLIST& pi_rList, const Utf8String& pi_rName);

        bool m_ScanSubdir;
};

#endif

