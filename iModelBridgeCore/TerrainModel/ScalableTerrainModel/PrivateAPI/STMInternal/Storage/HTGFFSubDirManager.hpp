//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFSubDirManager.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


/*---------------------------------------------------------------------------------**//**
* @description
* @param        pi_SubDirTagID   The tag id used to identify this directory as a
*                                   sub directory of its parent.
* @param        pi_SubDirIndex      The index of this directory as a sub directory. Only
*                                   specified when the directory is part of a sub
*                                   directory collection. Default value NO_SUB_DIR_INDEX
*                                   indicates that this directory is not part of a sub
*                                   directory collection (has no sub directory index).
* @param        pi_DirectoryID      The directory id for this directory.

* @return
* @bsimethod                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
T* SubDirManagerBase::Initialize   (DirectoryTagID          pi_SubDirTagID,
                                    size_t                  pi_SubDirIndex,
                                    DirectoryID             pi_DirectoryID,
                                    const CreateConfig*     pi_pCreateConfig,
                                    const UserOptions*      pi_pUserOptions)
    {
    // Directory not found. Create it and add it to our sub directories list.
    HFCPtr<T> pDir(Directory::CreateDir<T>());
    if (!pDir->Initialize(GetDir().GetFileHandle(), pi_SubDirTagID, pi_SubDirIndex, pi_DirectoryID, pi_pCreateConfig, pi_pUserOptions))
        return 0;

    return static_cast<T*>(AddToCached(pDir.GetPtr()));
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
T* SubDirManagerBase::GetFast      (DirectoryTagID      pi_SubDirsTagID,
                                    size_t              pi_SubDirIndex,
                                    DirectoryID         pi_SubDirID,
                                    const UserOptions*  pi_pUserOptions) const
    {
    return const_cast<SubDirManagerBase&>(*this).GetFast<T>(pi_SubDirsTagID, pi_SubDirIndex, pi_SubDirID, pi_pUserOptions);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
T* SubDirManagerBase::GetFast  (DirectoryTagID      pi_SubDirsTagID,
                                size_t              pi_SubDirIndex,
                                DirectoryID         pi_SubDirID,
                                const UserOptions*  pi_pUserOptions)
    {
    //TDORAY: This method should use a lock for thread safety
    HPRECONDITION(0 != pi_SubDirID);

    Directory* pFoundDir = FindByID(pi_SubDirID);
    if (0 != pFoundDir)
        return static_cast<T*>(pFoundDir);


    return Initialize<T>(pi_SubDirsTagID, pi_SubDirIndex, pi_SubDirID, 0, pi_pUserOptions);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>  
const SubDirManager<T>& AsTypedMgr (const SubDirManagerBase& pi_rBase)
    {
    return static_cast<const SubDirManager<T>&>(pi_rBase);
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
SubDirManager<T>& AsTypedMgr (SubDirManagerBase& pi_rBase)
    {
    return static_cast<SubDirManager<T>&>(pi_rBase);
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
const SubDirIterManager<EditorT>& AsTypedIterMgr (const SubDirManagerBase& pi_rBase)
    {
    return static_cast<const SubDirIterManager<EditorT>&>(pi_rBase);
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
SubDirIterManager<EditorT>& AsTypedIterMgr (SubDirManagerBase& pi_rBase)
    {
    return static_cast<SubDirIterManager<EditorT>&>(pi_rBase);
    }




/*---------------------------------------------------------------------------------**//**
* @description  Return a sub directory for the specified directory tag ID/index
*               and of T.
* @param        pi_SubDirsTagID   the directory tag id identifying the requested
*                                   directory.
* @param        pi_SubDirIndex      Index of the desired sub directory. See GetSubDirQty.
* @return       A pointer to the sub directory for specified directory tag ID and sub
*               directory index or NULL on failure.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
typename SubDirManager<T>::CDirP SubDirManager<T>::Get (DirectoryTagID      pi_SubDirsTagID,
                                                        size_t              pi_SubDirIndex,
                                                        const UserOptions*  pi_pUserOptions) const
    {
    return const_cast<SubDirManager&>(*this).template Get<T>(pi_SubDirsTagID, pi_SubDirIndex, pi_pUserOptions);
    }


/*---------------------------------------------------------------------------------**//**
* @description  Return a sub directory for the specified directory tag ID/index
*               and of T.
* @param        pi_SubDirsTagID   the directory tag id identifying the requested
*                                   directory.
* @param        pi_SubDirIndex      Index of the desired sub directory. See GetSubDirQty.
* @return       A pointer to the sub directory for specified directory tag ID and sub
*               directory index or NULL on failure.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
typename SubDirManager<T>::DirP SubDirManager<T>::Get  (DirectoryTagID      pi_SubDirsTagID,
                                                        size_t              pi_SubDirIndex,
                                                        const UserOptions*  pi_pUserOptions)
    {
    //TDORAY: This method should use a lock for thread safety

    HPRECONDITION(IsDirValid());
    HPRECONDITION(GetFile().IsVariableSizeTag(pi_SubDirsTagID));

    size_t          DirectoryQty = 0;
    DirectoryID*    pDirectoryIds = 0;
    if (!GetSubDirsAttribute(pi_SubDirsTagID, pDirectoryIds, DirectoryQty) || DirectoryQty <= pi_SubDirIndex)
        return 0;

    const DirectoryID& rDirectoryId = pDirectoryIds[pi_SubDirIndex];
    if (0 == rDirectoryId)
        return 0;

    return GetFast<T>(pi_SubDirsTagID, pi_SubDirIndex, rDirectoryId, pi_pUserOptions);
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>     
typename SubDirManager<T>::CDirP SubDirManager<T>::Get (DirectoryTagID          pi_SubDirsTagID,
                                                        const SubDirIdIter&     pi_SubDirIdIt,
                                                        const UserOptions*      pi_pUserOptions) const
{
    return GetFast<T>(pi_SubDirsTagID, pi_SubDirIdIt.ConvertToIndex(), *pi_SubDirIdIt, pi_pUserOptions);
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>     
typename SubDirManager<T>::DirP SubDirManager<T>::Get  (DirectoryTagID          pi_SubDirsTagID,
                                                        const SubDirIdIter&     pi_SubDirIdIt,
                                                        const UserOptions*      pi_pUserOptions)
{
    return GetFast<T>(pi_SubDirsTagID, pi_SubDirIdIt.ConvertToIndex(), *pi_SubDirIdIt, pi_pUserOptions);
    }



/*---------------------------------------------------------------------------------**//**
* @description  Return a sub directory for the specified directory tag ID and
*               of T.
* @param        pi_SubDirTagID   the directory tag id identifying the requested
*                                   directory.
* @return       A pointer to the sub directory for specified directory tag ID or NULL
*               on failure.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
typename SubDirManager<T>::CDirP SubDirManager<T>::Get (DirectoryTagID      pi_SubDirTagID,
                                                        const UserOptions*  pi_pUserOptions) const
    {
    return const_cast<SubDirManager<T>&>(*this).Get(pi_SubDirTagID, pi_pUserOptions);
    }


/*---------------------------------------------------------------------------------**//**
* @description  Return a sub directory for the specified directory tag ID and
*               of T.
* @param        pi_SubDirTagID   the directory tag id identifying the requested
*                                   directory.
* @return       A pointer to the sub directory for specified directory tag ID or NULL
*               on failure.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
typename SubDirManager<T>::DirP SubDirManager<T>::Get  (DirectoryTagID      pi_SubDirTagID,
                                                        const UserOptions*  pi_pUserOptions)
    {
    //TDORAY: This method should use a lock for thread safety

    HPRECONDITION(IsDirValid());
    HPRECONDITION(!GetFile().IsVariableSizeTag(pi_SubDirTagID));

    Directory* pFoundDir = FindBySubDirTagID(pi_SubDirTagID);
    if (0 != pFoundDir)
        return static_cast<DirP>(pFoundDir);


    DirectoryID DirectoryId = 0;
    if (!GetFile().GetField(pi_SubDirTagID, &DirectoryId))
        return 0;

    return Initialize<T>(pi_SubDirTagID, GetNoSubDirIndex(), DirectoryId, 0, pi_pUserOptions);
    }


/*---------------------------------------------------------------------------------**//**
* @description  Add a sub directory to the specified tag.
* @param        pi_SubDirsTagID   the directory tag id identifying the requested
*                                   directory. NOTE: Tag must be a array of variable
*                                   size.
* @param        po_pSubDirIndex     Index of the added sub directory.
* @return       A pointer to the added sub directory or NULL
*               on failure.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
typename SubDirManager<T>::DirP SubDirManager<T>::Add  (DirectoryTagID      pi_SubDirsTagID,
                                                        const CreateConfig& pi_rCreateConfig,
                                                        const UserOptions*  pi_pUserOptions)
    {
    HPRECONDITION(IsDirValid());

    size_t subDirID = 0;

    const DirectoryID directoryID = GetFile().AddSubDir(pi_SubDirsTagID, subDirID);
    if (0 == directoryID)
        return 0;

    return Initialize<T>(pi_SubDirsTagID, subDirID, directoryID, &pi_rCreateConfig, pi_pUserOptions);
    }


/*---------------------------------------------------------------------------------**//**
* @description  Add a sub directory to the specified tag.
* @param        pi_SubDirsTagID   the directory tag id identifying the requested
*                                   directory. NOTE: Tag must be a array of variable
*                                   size.
* @param        po_pSubDirIndex     Index of the added sub directory.
* @return       A pointer to the added sub directory or NULL
*               on failure.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
typename SubDirManager<T>::DirP SubDirManager<T>::Create   (DirectoryTagID      pi_SubDirsTagID,
                                                            size_t              pi_SubDirIndex,
                                                            const CreateConfig& pi_rCreateConfig,
                                                            const UserOptions*  pi_pUserOptions)
    {
    HPRECONDITION(IsDirValid());

    const DirectoryID directoryID = GetFile().CreateSubDir(pi_SubDirsTagID, pi_SubDirIndex);
    if (0 == directoryID)
        return 0;

    return Initialize<T>(pi_SubDirsTagID, pi_SubDirIndex, directoryID, &pi_rCreateConfig, pi_pUserOptions);
    }


/*---------------------------------------------------------------------------------**//**
* @description  Create a sub directory for the specified directory tag ID and
*               of T.
* @param        pi_SubDirTagID   the directory tag id identifying the requested
*                                   directory.
* @param        pi_rCreateConfig    Create configuration if the directory has to
*                                   be created.
* @return       A pointer to the sub directory for specified directory tag ID or NULL
*               on failure.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
typename SubDirManager<T>::DirP SubDirManager<T>::Create   (DirectoryTagID          pi_SubDirTagID,
                                                            const CreateConfig&     pi_rCreateConfig,
                                                            const UserOptions*      pi_pUserOptions)
    {
    HPRECONDITION(IsDirValid());

    const DirectoryID directoryID = GetFile().CreateSubDir(pi_SubDirTagID);
    if (0 == directoryID)
        return 0;

    return Initialize<T>(pi_SubDirTagID, GetNoSubDirIndex(), directoryID, &pi_rCreateConfig, pi_pUserOptions);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
SubDirIter<const EditorT> SubDirIterManager<EditorT>::begin (DirectoryTagID          pi_SubDirsTagID,
                                                             const editor_base_type& pi_rEditorBase) const
{
    return SubDirIter<const EditorT>::Create(pi_SubDirsTagID, beginId(pi_SubDirsTagID), pi_rEditorBase);
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
SubDirIter<const EditorT> SubDirIterManager<EditorT>::end (DirectoryTagID          pi_SubDirsTagID,
                                                           const editor_base_type& pi_rEditorBase) const
{
    return SubDirIter<const EditorT>::Create(pi_SubDirsTagID, endId(pi_SubDirsTagID), pi_rEditorBase);
}


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
SubDirIter<EditorT> SubDirIterManager<EditorT>::begin (DirectoryTagID       pi_SubDirsTagID,
                                                       editor_base_type&    pi_rEditorBase)
{
    return SubDirIter<EditorT>::Create(pi_SubDirsTagID, beginId(pi_SubDirsTagID), pi_rEditorBase);
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
SubDirIter<EditorT> SubDirIterManager<EditorT>::end (DirectoryTagID     pi_SubDirsTagID,
                                                     editor_base_type&  pi_rEditorBase)
{
    return SubDirIter<EditorT>::Create(pi_SubDirsTagID, endId(pi_SubDirsTagID), pi_rEditorBase);
}
