//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSLogicalPaths.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "HFCMonitor.h"

//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Properties>
inline HCSLogicalPaths<Properties>::HCSLogicalPaths()
    {
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Properties>
inline HCSLogicalPaths<Properties>::~HCSLogicalPaths()
    {
    }



//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Properties>
inline HCSLogicalPaths<Properties>::HCSLogicalPaths(const HCSLogicalPaths& pi_rObj)
    {
    HFCMonitor Monitor(pi_rObj.m_Key);

    m_LogicalPathList = pi_rObj.m_LogicalPathList;
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Properties>
inline HCSLogicalPaths<Properties>& HCSLogicalPaths<Properties>::operator= (const HCSLogicalPaths& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCMonitor MyMonitor(m_Key);
        HFCMonitor ItsMonitor(pi_rObj.m_Key);

        m_LogicalPathList = pi_rObj.m_LogicalPathList;
        }

    return (*this);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Properties>
inline void HCSLogicalPaths<Properties>::GetLogicalPath(uint32_t    pi_Index,
                                                        WString*    po_pLogical,
                                                        WString*    po_pPhysical,
                                                        Properties* po_pProperties) const
    {
    HPRECONDITION(pi_Index < CountLogicalPaths());
    HPRECONDITION(po_pLogical);
    HPRECONDITION(po_pPhysical);
    HFCMonitor Monitor(m_Key);

    // get the logical path information
    *po_pLogical  = m_LogicalPathList[pi_Index].GetName();
    *po_pPhysical = m_LogicalPathList[pi_Index].GetPhysicalPath();
    if (po_pProperties != 0)
        *po_pProperties = m_LogicalPathList[pi_Index].GetProperties();
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Properties>
inline WString HCSLogicalPaths<Properties>::FindPhysicalFor(const WString&  pi_rLogical,
                                                            Properties*     po_pProperties) const
    {
    WString Result;
    WString Logical(pi_rLogical);
    HFCMonitor Monitor(m_Key);

    // Format the logical path
    HCSLogicalPath<Properties>::FormatLogicalPath(&Logical);

    // Parse until end or found
    PathList::const_iterator Itr = m_LogicalPathList.begin();
    while ((Result.empty()) &&
           (Itr != m_LogicalPathList.end()) )
        {
        if (BeStringUtilities::Wcsicmp(Logical.c_str(), (*Itr).GetName().c_str()) == 0)
            {
            if (po_pProperties != 0)
                *po_pProperties = (*Itr).GetProperties();
            Result = (*Itr).GetPhysicalPath();
            }
        Itr++;
        }

    return (Result);
    }

//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Properties>
inline void HCSLogicalPaths<Properties>::RemoveLogicalPath(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < CountLogicalPaths());
    HFCMonitor Monitor(m_Key);

    // get the iterator to the indexed entry
    PathList::iterator Itr = m_LogicalPathList.begin() + pi_Index;

    // erase that entry
    m_LogicalPathList.erase(Itr);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Properties>
inline void HCSLogicalPaths<Properties>::RemoveLogicalPath(const WString& pi_rLogical)
    {
    HFCMonitor Monitor(m_Key);

    // Format the logical path
    WString Logical(pi_rLogical);
    HCSLogicalPath<Properties>::FormatLogicalPath(&Logical);

    // Parse until end or found
    bool FoundEntry = false;
    for (PathList::iterator Itr = m_LogicalPathList.begin();
         (!FoundEntry) && (Itr != m_LogicalPathList.end());)
        {
        if (FoundEntry = (BeStringUtilities::Wcsicmp(Logical.c_str(), (*Itr).GetName().c_str()) == 0))
            Itr = m_LogicalPathList.erase(Itr);
        else
            ++Itr;
        }
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Properties>
inline void HCSLogicalPaths<Properties>::ClearLogicalPaths()
    {
    HFCMonitor Monitor(m_Key);

    m_LogicalPathList.clear();
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Properties>
inline void HCSLogicalPaths<Properties>::AddLogicalPath(const WString&    pi_rLogical,
                                                        const WString&    pi_rPhysical,
                                                        const Properties& pi_rProperties)
    {
    HFCMonitor Monitor(m_Key);
    WString Logical(pi_rLogical);
    WString Physical(pi_rPhysical);

    // Format the paths
    HCSLogicalPath<Properties>::FormatLogicalPath(&Logical);
    HCSLogicalPath<Properties>::FormatPhysicalPath(&Physical);

    // Create the HCSLogicalPath struct
    HCSLogicalPath<Properties> Path(Logical, Physical, pi_rProperties);

    // Verify that the logical path does not already exists.  If so, remove
    // the previous copy.  In this case, we will remove the current version and
    // replace it with the new one.  Since the paths are saved in the registry
    // as sub-keys to the Logical Paths key, the logical name of the path must
    // be unique.  This can only happen if a path is defined in both the OLD
    // style and the NEW style of logical paths persistence in the registry.
    // Therefore, if a path is specified in both, we use the version in the NEW
    // style persistence.
    PathList::iterator Itr = find(m_LogicalPathList.begin(), m_LogicalPathList.end(), Path);
    HASSERT(Itr == m_LogicalPathList.end());
    if (Itr != m_LogicalPathList.end())
        m_LogicalPathList.erase(Itr);

    // verify that no '/' are at the beginning of the path, except for the root
    HASSERT((Path.GetName().compare(L"/") == 0) || (Path.GetName().find(L'/') != 0));

    // add it to the list
    m_LogicalPathList.push_back(Path);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Properties>
inline void HCSLogicalPaths<Properties>::SetLogicalPaths(const PathList& pi_rList)
    {
    HFCMonitor Monitor(m_Key);

    // Clear the current list
    m_LogicalPathList.clear();

    // copy the logical paths
    PathList::const_iterator Itr = pi_rList.begin();
    while (Itr != pi_rList.end())
        {
        // Add the current logical path
        AddLogicalPath((*Itr).GetName(), (*Itr).GetPhysicalPath(), (*Itr).GetProperties());

        // Proceed to the next logical path
        Itr++;
        }
    }


//-----------------------------------------------------------------------------
// Static
//-----------------------------------------------------------------------------
template<class Properties>
inline void HCSLogicalPaths<Properties>::SplitPath(const WString&   pi_rPath,
                                                   WString*         po_pDirectory,
                                                   WString*         po_pFileName) const
    {
    HPRECONDITION(po_pDirectory);
    HPRECONDITION(po_pFileName);
    WString::size_type Pos;
    WString Path(pi_rPath);

    // Make sure that the file doesn't have "\\"
    while ((Pos = Path.find(L'\\', 0)) != WString::npos)
        Path[Pos] = L'/';

    // To separate the file name from the directory, we'll
    // find the last "/".  If a last "/" is found, the information
    // to the right is the directory, and to the left is the file name.
    //
    // If no last "/" is found, the whole string is the file name
    //
    if ((Pos = Path.find_last_of(L'/')) != string::npos)
        {
        // The directory part of the includes all until the last "/"
        // The file name includes all after the last "/"
        *po_pDirectory = WString(Path, 0, Pos);
        *po_pFileName = WString(Path, Pos + 1, Path.size());
        }
    else
        {
        po_pDirectory->resize(0);
        *po_pFileName = Path;
        }
    }


//-----------------------------------------------------------------------------
// This method converts a logical path into a physical path.  Note that the
// first parameter is a path without the file name.
//-----------------------------------------------------------------------------
template<class Properties>
bool HCSLogicalPaths<Properties>::GetPhysicalPath(const WString&   pi_rLogical,
                                                   WString*         po_pPhysical,
                                                   WString*         po_pLogicalPath,
                                                   Properties*      po_pProperties) const
    {
    HPRECONDITION(po_pPhysical);

    bool         Result = false;
    WString       Logical(pi_rLogical); // formatted copy of param 1
    WString       Physical;             // mapped physical if found
    WString       SubLogical;           // current sub-level being tested
    WString       LogicalRemain;        // sub-dir from logical sub-level

    //--------------------------------------
    // Initialize
    //--------------------------------------

    // Copy the given logical path
    WString::size_type Pos;
    while ((Pos = Logical.find(L'\\', 0)) != string::npos)
        Logical[Pos] = L'/';

    // Insure that the logical path starts with "/" and that it
    // does not end with "/", except for the root "/"
    if (Logical[0] != L'/')
        Logical.insert(0, L"/");
    while ((Logical.compare(L"/") != 0) &&
           (Logical[Logical.size() - 1] == L'/') )
        Logical.resize(Logical.size() - 1);


    //--------------------------------------
    // Find a physical path
    //--------------------------------------

    // Try to find a physical path for each sub-dir level in the
    // logical path starting with the longest one.
    //
    //  Example:
    //
    //      /images/quebec/satellite    -> not found, remove one level
    //      /images/quebec              -> c:\Quebecimages
    //      /images                     -> no need to search
    //      //                          -> no need to search
    //

    SubLogical = Logical;
    do
        {
        // try to map the current logical path.  if one is not found,
        // remove one level to the directory
        Physical = FindPhysicalFor(SubLogical, po_pProperties);
        if (Physical.empty())
            {
            // if the tested sub-level logical isn't mapped, remove one
            // level which is saved in the remaining logical
            //
            //  example:
            //
            //      SubLevel:   /images/quebec  ->  /images
            //      Remain:     satellite       ->  /quebec/satellite
            //

            WString::size_type Pos;
            if ((SubLogical.compare(L"/") != 0) &&
                ((Pos = SubLogical.find_last_of(L'/')) != string::npos) )
                {
                // Build the new logical remain
                WString NewRemain(SubLogical.substr(Pos));
                NewRemain += LogicalRemain;
                LogicalRemain = NewRemain;

                // if the last slash is the first character, keep the slash
                // in the current SubLogical
                if (Pos == 0)
                    Pos += 1;

                // resize the current sub-logical level
                SubLogical.resize(Pos);
                }
            else
                SubLogical.resize(0);
            }
        }
    while ((Physical.empty()) && (!SubLogical.empty()));


    //--------------------------------------
    // Generate the full physical
    //--------------------------------------

    if (!Physical.empty())
        {
        // Initialize the resulting string
        *po_pPhysical = L"";

        // remove all / at the end
        while (Physical[Physical.length() - 1] == L'/')
            Physical.resize(Physical.length() - 1);

        // remove all / at the beginning
        if (!LogicalRemain.empty())
            {
            WString::size_type Pos = 0;
            while (LogicalRemain[Pos] == L'/')
                Pos++;

            LogicalRemain = LogicalRemain.substr(Pos);
            }

        // the full physical path will included
        //
        //  "Physical/Remaining of Logical
        *po_pPhysical = Physical;

        // If there is a logical remain add it to the physical path.
        if (!LogicalRemain.empty())
            *po_pPhysical += L"/" + LogicalRemain;

        if (po_pLogicalPath != 0)
            {
            *po_pLogicalPath = SubLogical;
            HCSLogicalPath<Properties>::FormatLogicalPath(po_pLogicalPath);
            }

        Result = true;
        }

    return (Result);
    }


//-----------------------------------------------------------------------------
// Returns an URL of the physical name of a file (with a logical path)
// Uses the
//-----------------------------------------------------------------------------
template<class Properties>
inline bool HCSLogicalPaths<Properties>::GetPhysicalFileName(const WString&    pi_rImageName,
                                                              WString*          po_pPhysical,
                                                              Properties*       po_pProperties) const
    {
    HPRECONDITION(po_pPhysical);
    bool Result = false;

    // Copy and format the image name
    WString ImageName(pi_rImageName);
    HCSLogicalPath<Properties>::FormatLogicalPath(&ImageName);

    // extract the directory part of the file name part
    WString Directory, FileName;
    SplitPath(ImageName, &Directory, &FileName);
    HCSLogicalPath<Properties>::FormatLogicalPath(&Directory);

    // Find a physical path for the given logical directory
    if (GetPhysicalPath(Directory, po_pPhysical, 0, po_pProperties))
        {
        *po_pPhysical += L"/";
        *po_pPhysical += FileName;
        Result = true;
        }

    return (Result);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Properties>
inline HFCExclusiveKey& HCSLogicalPaths<Properties>::GetLogicalPathsKey() const
    {
    return (m_Key);
    }




//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Properties>
inline uint32_t HCSLogicalPaths<Properties>::CountLogicalPaths() const
    {
    HFCMonitor Monitor(m_Key);

    return m_LogicalPathList.size();
    }
