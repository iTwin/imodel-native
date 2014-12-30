/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeFileListIterator.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#ifndef BENTLEY_NO_BEFILENAME

#include "Bentley.h"
#include "BeFileName.h"
#include "WString.h"

BEGIN_BENTLEY_NAMESPACE

/*=================================================================================**//**
* An iterator for walking through a semicolon-delimited list of file paths, each entry of which
* may include wildcards. For example, a file path list might look like this: "\a\*;*.exe;c:\temp\Ab*", etc. 
* The iterator returns one filename at a time.
*
* Here are a few things to keep in mind when using BeFileListIterator:
* \li A BeFileListIterator does not accept environment or configuration variables in the list.
* \li A BeFileListIterator only returns filenames that actually exist. So, if the list is just the full name of a file which is not present,
* this iterator will return ERROR without returning anything.
*
* <h4>Recursion</h4>
* There is also an option to recursively return entries from subdirectories. This is most useful when the list includes the name of one
* or more directories. 
* @note The interaction between wildcards and recursion can be confusing. If the list includes both, the result is that
* if the wildcard returns a directory, all of its entries are returned. The wildcard is applied only at the first level, \em not recursively.
* So, if the filepath "\123\a*" is passed with recursion on, the file "\123\abc.txt" is returned, as well as \em all of the files
* in the directory "\123\apples\" and all of its subdirectories, but none of the files in "\123\def\" are returned, even if they start with "a".
* @note: This class does NOT accept environment or configuration variables in the list.
* @note It returns only filenames that actually exist. So, if the list is just the full name of a file which is not present,
*       this iterator will return ERROR without returning anything. 
* @ingroup BeFileGroup
* @bsiclass                                                     Keith.Bentley   02/09
+===============+===============+===============+===============+===============+======*/
class BeFileListIterator
    {
    class BeFileNameIterator* m_finder;
    WString     m_paths;
    size_t      m_curr;
    bool        m_recursive;
    void Init (bool recursive) {m_finder = NULL; m_recursive = recursive; m_curr = 0;}

public:
    //! Construct an instance of a BeFileListIterator.
    //! @param[in] filePathList A list of semicolon delimited file paths, including wildcards, to iterate.
    //! @param[in] recursive If the list contatains the name of a directory, or if a directory is returned from a wildcard, return their content, recursively.
    BeFileListIterator (WCharCP filePathList, bool recursive) {Init(recursive); m_paths.assign(filePathList);}
    //! Construct an instance of a BeFileListIterator.
    //! @param[in] filePathList A list of semicolon delimited file paths, including wildcards, to iterate.
    //! @param[in] recursive If the list contatains the name of a directory, or if a directory is returned from a wildcard, return their content, recursively.
    BeFileListIterator (WStringCR filePathList, bool recursive) {Init(recursive); m_paths.assign(filePathList.c_str());}
    //! Construct an instance of a BeFileListIterator.
    //! @param[in] filePathList A list of semicolon delimited file paths <em>locale-encoded</em>.
    //! @param[in] recursive If the list contatains the name of a directory, or if a directory is returned from a wildcard, return their content, recursively.
    BeFileListIterator (CharCP filePathList, bool recursive)  {Init(recursive); m_paths.AssignA(filePathList);}
    BENTLEYDLL_EXPORT ~BeFileListIterator ();

    //! Retrieve the next filename found from the path list.
    //! @param[out] name The name of the next valid file from the path list.
    //! @return SUCCESS if the name returned is valid, ERROR if there are no more files.
    BENTLEYDLL_EXPORT BentleyStatus GetNextFileName (BeFileName& name);
    };

END_BENTLEY_NAMESPACE

#endif // BENTLEY_NO_BEFILENAME
