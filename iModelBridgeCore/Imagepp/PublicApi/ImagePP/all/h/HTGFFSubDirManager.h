//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTGFFSubDirManager.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HFCPtr.h>
#include <ImagePP/all/h/HTGFFDirectory.h>
#include <ImagePP/all/h/HTGFFSubDirIdIter.h>

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

    _HDLLg Directory*                   AddToCached                    (Directory*              pi_pSubDir);

protected:
    typedef Directory::CreateConfig     CreateConfig;

    // Note: Not meant to be used polymorphically
                                        ~SubDirManagerBase             () {}

    template <typename EditorT>
    struct EditorBaseTypeTrait          { typedef typename EditorT::base_type    type; };


    _HDLLg static size_t                GetNoSubDirIndex               ();


    _HDLLg bool                         GetSubDirsAttribute            (DirectoryTagID          pi_SubDirsTagID,
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

    _HDLLg Directory*                   FindBySubDirTagID              (DirectoryTagID          pi_SubDirTagID) const;
    _HDLLg Directory*                   FindByID                       (DirectoryID             pi_DirectoryID) const;


    _HDLLg SubDirIdIter                 beginId                        (DirectoryTagID          pi_SubDirsTagID) const;
    _HDLLg SubDirIdIter                 endId                          (DirectoryTagID          pi_SubDirsTagID) const;
public:

    _HDLLg bool                         Has                            (DirectoryTagID          pi_SubDirsTagID) const;
    _HDLLg bool                         Has                            (DirectoryTagID          pi_SubDirsTagID,
                                                                        size_t                  pi_SubDirIndex) const;  

    _HDLLg bool                         HasEx                          (DirectoryTagID          pi_SubDirsTagID,
                                                                        uint32_t&                 po_rVersion) const;
    _HDLLg bool                         HasEx                          (DirectoryTagID          pi_SubDirsTagID,
                                                                        size_t                  pi_SubDirIndex,
                                                                        uint32_t&                 po_rVersion) const;  

    _HDLLg size_t                       GetCount                       (DirectoryTagID          pi_SubDirsTagID) const;

    _HDLLg bool                         RemoveSubDirs                  (DirectoryTagID          pi_SubDirsTagID);
    _HDLLg bool                         RemoveSubDirs                  ();



    
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

#include <ImagePP/all/h/HTGFFSubDirManager.hpp>

} //End namespace HTGFF