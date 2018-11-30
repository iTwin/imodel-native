//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/HTGFFSubDirManager.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/HTGFFSubDirManager.h>

#include <STMInternal/Storage/HTGFFAttributeManager.h>
#include <STMInternal/Storage/HTGFFSubDirIdIter.h>

namespace HTGFF {


struct SubDirManagerBase::Impl
    {
    struct HasTagID : public unary_function<const Directory*, bool>
        {
        DirectoryTagID m_tagID;
        explicit HasTagID (DirectoryTagID pi_tagID) : m_tagID(pi_tagID) {}

        bool operator() (const Directory* pi_pLeft) const
            {
            return pi_pLeft->GetTagID() == m_tagID;
            }
        };

    struct HasDirectoryID : public unary_function<const Directory*, bool>
        {
        DirectoryID m_directoryID;
        explicit HasDirectoryID (DirectoryID pi_directoryID) : m_directoryID(pi_directoryID) {}

        bool operator() (const Directory* pi_pLeft) const
            {
            return pi_pLeft->GetID() == m_directoryID;
            }
        };

    struct NotifyDirForSave
        {
        bool operator () (Directory* pi_pDirectory) const {
            return pi_pDirectory->OnSaved ();
            }
        };
    struct NotifyDirForClose
        {
        bool operator () (Directory* pi_pDirectory) const {
            return pi_pDirectory->OnClosed ();
            }
        };

    struct NotifyDirForRemoval
        {
        bool operator () (Directory* pi_pDirectory) const {
            return pi_pDirectory->OnRemoved ();
            }
        };
    };

/*---------------------------------------------------------------------------------**//**
* @description     
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SubDirManagerBase::Has    (DirectoryTagID  pi_SubDirsTagID) const
    {
    return 0 != GetDir().GetFile().GetSubDir(pi_SubDirsTagID);
    }

/*---------------------------------------------------------------------------------**//**
* @description     
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SubDirManagerBase::Has    (DirectoryTagID  pi_SubDirsTagID,
                                size_t          pi_SubDirIndex) const
    {
    return 0 != GetDir().GetFile().GetSubDir(pi_SubDirsTagID, pi_SubDirIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @description     
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SubDirManagerBase::HasEx  (DirectoryTagID  pi_SubDirsTagID,
                                uint32_t&         po_rVersion) const
    {
    return 0 != GetDir().GetFile().GetSubDirEx(pi_SubDirsTagID, po_rVersion);
    }

/*---------------------------------------------------------------------------------**//**
* @description     
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SubDirManagerBase::HasEx  (DirectoryTagID  pi_SubDirsTagID,
                                size_t          pi_SubDirIndex,
                                uint32_t&         po_rVersion) const
    {
    return 0 != GetDir().GetFile().GetSubDirEx(pi_SubDirsTagID, pi_SubDirIndex, po_rVersion);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t SubDirManagerBase::GetNoSubDirIndex ()
    {
    static const size_t NO_SUB_DIR_INDEX = -1;
    return NO_SUB_DIR_INDEX;
    }

bool SubDirManagerBase::GetSubDirsAttribute    (DirectoryTagID  pi_SubDirsTagID,
                                                DirectoryID*&   po_rpDirectoryIds,
                                                size_t&         po_rDirectoryQty) const
    {
    return GetDir().AttributeMgr().Get(pi_SubDirsTagID, po_rpDirectoryIds, po_rDirectoryQty);
    }



bool SubDirManagerBase::OnDirectoryClosed ()
    {
    // Notify all of our sub directories that the file was closed
    bool Success = (count_if(m_SubDirectories.begin(), m_SubDirectories.end(), Impl::NotifyDirForClose()) == m_SubDirectories.size());

    return Success;
    }

bool SubDirManagerBase::OnDirectorySaved ()
    {
    // Notify all of our sub directories that the file was saved
    bool Success = (count_if(m_SubDirectories.begin(), m_SubDirectories.end(), Impl::NotifyDirForSave()) == m_SubDirectories.size());

    return Success;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SubDirManagerBase::OnDirectoryRemoved ()
    {
    // Notify all of our sub directories that the directory was removed
    bool Success = (count_if(m_SubDirectories.begin(), m_SubDirectories.end(), Impl::NotifyDirForRemoval()) == m_SubDirectories.size());

    m_SubDirectories.clear();

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SubDirManagerBase::RemoveSubDirs (DirectoryTagID pi_SubDirsTagID)
    {
    // Partition sub directory array so that all sub directory with specified tag id come last.
    SubDirectoriesList::iterator newEndIt =
        partition(m_SubDirectories.begin(), m_SubDirectories.end(), not1(Impl::HasTagID(pi_SubDirsTagID)));

    const size_t removedQty = distance(newEndIt, m_SubDirectories.end());

    // Notify all of our cached sub directories that they should save their states
    bool Success = (count_if(newEndIt, m_SubDirectories.end(), Impl::NotifyDirForRemoval()) == removedQty);

    // Erase cached sub directories
    m_SubDirectories.erase(newEndIt, m_SubDirectories.end());

    // Remove sub directories from the file
    Success &= GetFile().RemoveSubDirs(pi_SubDirsTagID);

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SubDirManagerBase::RemoveSubDirs ()
    {
    // Notify all of our cached sub directories that they should save their states
    bool Success = (count_if(m_SubDirectories.begin(), m_SubDirectories.end(), Impl::NotifyDirForRemoval()) == m_SubDirectories.size());

    // Erase cached sub directories
    m_SubDirectories.clear();

    // Remove directory data from the file
    Success &= GetFile().RemoveSubDirs();

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SubDirIdIter SubDirManagerBase::beginId (DirectoryTagID pi_SubDirsTagID) const
    {
    HPRECONDITION(IsDirValid());
    return GetFile().SubDirIDsBegin(pi_SubDirsTagID);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SubDirIdIter SubDirManagerBase::endId (DirectoryTagID pi_SubDirsTagID) const
    {
    HPRECONDITION(IsDirValid());
    return GetFile().SubDirIDsEnd(pi_SubDirsTagID);
    }


/*---------------------------------------------------------------------------------**//**
* @description  Find a directory in our list of sub directories
* @param        pi_DirectoryTagID   The tag ID for the directory
* @return       Pointer to found directory, 0 when not found
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Directory* SubDirManagerBase::FindBySubDirTagID (DirectoryTagID pi_DirectoryTagID) const
    {
    // Search for existing sub directory
    SubDirectoriesList::const_iterator FoundIt =
        find_if(m_SubDirectories.begin(), m_SubDirectories.end(), Impl::HasTagID(pi_DirectoryTagID));

    if (FoundIt == m_SubDirectories.end())
        return 0;

    return (*FoundIt).GetPtr();
    }

Directory* SubDirManagerBase::FindByID (DirectoryID pi_DirectoryID) const
    {
    // Search for existing sub directory
    SubDirectoriesList::const_iterator FoundIt =
        find_if(m_SubDirectories.begin(), m_SubDirectories.end(), Impl::HasDirectoryID(pi_DirectoryID));

    if (FoundIt == m_SubDirectories.end())
        return 0;

    return (*FoundIt).GetPtr();
    }

size_t SubDirManagerBase::GetCount (DirectoryTagID pi_SubDirsTagID) const
    {
    return GetFile().GetSubDirCount(pi_SubDirsTagID);
    }


Directory* SubDirManagerBase::AddToCached (Directory* pi_pSubDir)
    {
    m_SubDirectories.push_back(pi_pSubDir);
    return pi_pSubDir;
    }

} //End namespace HTGFF