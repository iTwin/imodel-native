//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFSubDirManager.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HFCPtr.h>
#include <STMInternal/Storage/HTGFFDirectory.h>
#include <STMInternal/Storage/HTGFFSubDirIdIter.h>

namespace HTGFF {

class SubDirIdIter;

template <typename EditorT>
class SubDirIter;




/*---------------------------------------------------------------------------------**//**
* @description
* @see          Directory
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class SubDirManagerBase : protected DirectoryManager
    {
private:
    friend class                        DirectoryImpl;
    friend class                        Directory; // TDORAY: Try remove this

    template <typename T>
    friend class                        SubDirIter2;

    typedef vector<DirectoryPtr>        SubDirectoriesList;

    SubDirectoriesList                  m_SubDirectories;

    struct                              Impl;

    explicit                            SubDirManagerBase              (Directory& pi_rDir) : DirectoryManager(pi_rDir) {}


    bool                                OnDirectoryClosed              ();
    bool                                OnDirectorySaved               ();
    bool                                OnDirectoryRemoved             ();

     Directory*                   AddToCached                    (Directory*              pi_pSubDir);

protected:
    typedef Directory::CreateConfig     CreateConfig;

    // Note: Not meant to be used polymorphically
                                        ~SubDirManagerBase             () {}

    template <typename EditorT>
    struct EditorBaseTypeTrait          { typedef typename EditorT::base_type    type; };


     static size_t                GetNoSubDirIndex               ();


     bool                         GetSubDirsAttribute            (DirectoryTagID          pi_SubDirsTagID,
                                                                        DirectoryID*&           po_rpDirectoryIds,
                                                                        size_t&                 po_rDirectoryQty) const;

    // TDORAY: Should return const T. Change it when HFCPtr becomes const wise
    template <typename T>
    T*                                  GetFast                        (DirectoryTagID          pi_SubDirsTagID,
                                                                        size_t                  pi_SubDirIndex,
                                                                        DirectoryID             pi_SubDirID,
                                                                        const UserOptions*      pi_pUserOptions = 0) const;

    template <typename T>
    T*                                  GetFast                        (DirectoryTagID          pi_SubDirsTagID,
                                                                        size_t                  pi_SubDirIndex,
                                                                        DirectoryID             pi_SubDirID,
                                                                        const UserOptions*      pi_pUserOptions = 0);


    template <typename T>
    T*                                  Initialize                     (DirectoryTagID          pi_SubDirTagID,
                                                                        size_t                  pi_SubDirIndex,
                                                                        DirectoryID             pi_DirectoryID,
                                                                        const CreateConfig*     pi_pCreateConfig,
                                                                        const UserOptions*      pi_pUserOptions);

     Directory*                   FindBySubDirTagID              (DirectoryTagID          pi_SubDirTagID) const;
     Directory*                   FindByID                       (DirectoryID             pi_DirectoryID) const;


     SubDirIdIter                 beginId                        (DirectoryTagID          pi_SubDirsTagID) const;
     SubDirIdIter                 endId                          (DirectoryTagID          pi_SubDirsTagID) const;
public:

     bool                         Has                            (DirectoryTagID          pi_SubDirsTagID) const;
     bool                         Has                            (DirectoryTagID          pi_SubDirsTagID,
                                                                        size_t                  pi_SubDirIndex) const;  

     bool                         HasEx                          (DirectoryTagID          pi_SubDirsTagID,
                                                                        uint32_t&                 po_rVersion) const;
     bool                         HasEx                          (DirectoryTagID          pi_SubDirsTagID,
                                                                        size_t                  pi_SubDirIndex,
                                                                        uint32_t&                 po_rVersion) const;  

     size_t                       GetCount                       (DirectoryTagID          pi_SubDirsTagID) const;

     bool                         RemoveSubDirs                  (DirectoryTagID          pi_SubDirsTagID);
     bool                         RemoveSubDirs                  ();



    
    template <typename EditorT>
    const SubDirIterManager<EditorT>&   SubDirIterMgr                  () const;
    template <typename EditorT>
    SubDirIterManager<EditorT>&         SubDirIterMgr                  ();

    };


/*---------------------------------------------------------------------------------**//**
* @description
* @see          Directory
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
class SubDirManager : public SubDirManagerBase
    {
    typedef T*                          CDirP; //TDORAY: Change to real const as HFCPtr become const wise
    typedef T*                          DirP;

    // NOTE: Never constructed
    explicit                            SubDirManager                  ();
    // Note: Not meant to be used polymorphically
    ~SubDirManager                 () {}

public:


    CDirP Get (DirectoryTagID          pi_SubDirsTagID,
              size_t                  pi_SubDirIndex,
              const UserOptions*      pi_pUserOptions = 0) const;
    DirP  Get (DirectoryTagID          pi_SubDirsTagID,
              size_t                  pi_SubDirIndex,
              const UserOptions*      pi_pUserOptions = 0);

    CDirP Get (DirectoryTagID          pi_SubDirsTagID,
              const SubDirIdIter&     pi_SubDirIdIt,
              const UserOptions*      pi_pUserOptions = 0) const;
    DirP  Get (DirectoryTagID          pi_SubDirsTagID,
              const SubDirIdIter&     pi_SubDirIdIt,
              const UserOptions*      pi_pUserOptions = 0);

    CDirP Get (DirectoryTagID          pi_SubDirTagID,
              const UserOptions*      pi_pUserOptions = 0) const;

    DirP  Get (DirectoryTagID          pi_SubDirTagID,
              const UserOptions*      pi_pUserOptions = 0);

    DirP  Add (DirectoryTagID          pi_SubDirsTagID,
              const CreateConfig&     pi_rCreateConfig,
              const UserOptions*      pi_pUserOptions = 0);

    DirP  Create (DirectoryTagID          pi_SubDirsTagID,
                 size_t                  pi_SubDirIndex,
                 const CreateConfig&     pi_rCreateConfig,
                 const UserOptions*      pi_pUserOptions = 0);


    DirP  Create (DirectoryTagID          pi_SubDirTagID,
                 const CreateConfig&     pi_rCreateConfig,
                 const UserOptions*      pi_pUserOptions = 0);

    };

template <typename T>
const SubDirManager<T>&                 AsTypedMgr                     (const SubDirManagerBase&
                                                                                                pi_rBase);
template <typename T>
SubDirManager<T>&                       AsTypedMgr                     (SubDirManagerBase&      pi_rBase);

/*---------------------------------------------------------------------------------**//**
* @description  
* @see          Directory
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
class SubDirIterManager : public SubDirManagerBase
    {
    typedef typename EditorBaseTypeTrait<EditorT>::type 
                                        editor_base_type;

    // NOTE: Never constructed
    explicit                            SubDirIterManager              ();    
    // Note: Not meant to be used polymorphically
                                        ~SubDirIterManager             () {}

public:
    SubDirIter<const EditorT>           begin                          (DirectoryTagID          pi_SubDirsTagID,
                                                                        const editor_base_type& pi_rEditorBase) const;
    SubDirIter<const EditorT>           end                            (DirectoryTagID          pi_SubDirsTagID,
                                                                        const editor_base_type& pi_rEditorBase) const;

    SubDirIter<EditorT>                 begin                          (DirectoryTagID          pi_SubDirsTagID,
                                                                        editor_base_type&       pi_rEditorBase);
    SubDirIter<EditorT>                 end                            (DirectoryTagID          pi_SubDirsTagID,
                                                                        editor_base_type&       pi_rEditorBase);
    };


template <typename EditorT>
const SubDirIterManager<EditorT>&       AsTypedIterMgr                 (const SubDirManagerBase&
                                                                                                pi_rBase);
template <typename EditorT>
SubDirIterManager<EditorT>&             AsTypedIterMgr                 (SubDirManagerBase&      pi_rBase);

#include <STMInternal/Storage/HTGFFSubDirManager.hpp>

} //End namespace HTGFF