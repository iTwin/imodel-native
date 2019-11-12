/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "BeFileName.h"
#include "WString.h"

BEGIN_BENTLEY_NAMESPACE

struct DirectoryWalkerImpl;

//! Parses and matches file wildcard expressions. Expressions are expected to follow the Unix 'glob' rules, i.e., * and ?
//! @ingroup GROUP_File
struct FileNamePattern
    {
    //! Call this to parse an expression that might contain a combination of a filepath and a wildcard expression.
    //! @param[out] dir The directory portion of the path. Note that dir will not have a trailing path separator.
    //! @param[out] glob The portion of the path following the last separator
    //! @param[in] pathAndPattern The path and pattern expression to parse
    BENTLEYDLL_EXPORT static void Parse (BeFileName& dir, WString& glob, BeFileNameCR pathAndPattern);
    //! Call this to test if a filename is matched by a wildcard expression
    //! @param name the filename to test
    //! @param glob the wildcard expression to use
    BENTLEYDLL_EXPORT static bool MatchesGlob (BeFileNameCR name, WCharCP glob);
    };

//! Iterates the entries in a specified directory
//! @ingroup GROUP_File
struct BeDirectoryIterator
    {
private:
    DirectoryWalkerImpl* m_impl;

public:
    //! Create an iterator over the specified directory
    BENTLEYDLL_EXPORT BeDirectoryIterator (BeFileNameCR dir);

    BENTLEYDLL_EXPORT ~BeDirectoryIterator();

    //! Get the name of the current directory entry
    BENTLEYDLL_EXPORT StatusInt GetCurrentEntry (BeFileName& name, bool& isDir, bool fullPath=true);

    //! Move to the next directory entry
    BENTLEYDLL_EXPORT StatusInt ToNext();

    //! Return all filenames that match a wildcard expression in the specified directory
    static void WalkDirsAndMatch (bvector<BeFileName>& matches, BeFileNameCR topDir, WCharCP glob, bool recursive)
        {
        BeFileName topDirPrefix (topDir);
        topDirPrefix.AppendSeparator();

        BeFileName entryName;
        bool        isDir;
        for (BeDirectoryIterator dirs (topDir); dirs.GetCurrentEntry (entryName, isDir) == SUCCESS; dirs.ToNext())
            {
            if (FileNamePattern::MatchesGlob (entryName.GetBaseName(), glob))
                matches.push_back (entryName);

            if (isDir && recursive)
                WalkDirsAndMatch (matches, entryName, glob, true);
            }
        }

    }; // BeDirectoryIterator

END_BENTLEY_NAMESPACE
