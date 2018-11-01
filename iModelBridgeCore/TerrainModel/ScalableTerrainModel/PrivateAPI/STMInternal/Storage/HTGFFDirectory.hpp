//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFDirectory.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


template <typename T>
T& UserOptions::SafeReinterpretAs ()
    {
    HSTATICASSERT((BentleyApi::ImagePP::IsBaseOfTrait<UserOptions, T>::value));
    // Ensure no pointer shift is required on cast (as with multiple-inheritance
    HPRECONDITION(((void*)8) == static_cast<UserOptions*>((T*)8));
    // Ensure that the cast is to the right derived type
    HPRECONDITION(0 != dynamic_cast<T*>(this));
    return static_cast<T&>(*this);
    }

template <typename T>
const T& UserOptions::SafeReinterpretAs () const
    {
    HSTATICASSERT((BentleyApi::ImagePP::IsBaseOfTrait<UserOptions, T>::value));
    // Ensure no pointer shift is required on cast (as with multiple-inheritance
    HPRECONDITION(((void*)8) == static_cast<UserOptions*>((T*)8));
    // Ensure that the cast is to the right derived type
    HPRECONDITION(0 != dynamic_cast<const T*>(this));
    return static_cast<const T&>(*this);
    }


inline const DirectoryImpl& Directory::GetImpl () const
    {
    HPRECONDITION(0 != m_pImpl.get());
    return *m_pImpl;
    }

inline DirectoryImpl& Directory::GetImpl ()
    {
    HPRECONDITION(0 != m_pImpl.get());
    return *m_pImpl;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Returns whether the directory is associated with a valid/open file
*               implementation.
* @param        true when m_pImpl is valid(not null), false otherwise.
* @return
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool Directory::IsValid () const
    {
    return 0 != m_pImpl.get();
    }


/*---------------------------------------------------------------------------------**//**
* @description  Returns whether the current directory is read-only.
* @return       true when read-only, false otherwise.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool Directory::IsReadOnly () const
    {
    return !GetAccessMode().m_HasCreateAccess && !GetAccessMode().m_HasWriteAccess;
    }


inline const Directory::AccessMode& Directory::GetAccessMode () const
    {
    return GetFile().GetAccessMode();
    }


/*---------------------------------------------------------------------------------**//**
* @description Trait that can select between using the specific directory's Create
*              function or the generic Create based on the presence of Create as part
*              of DirT.
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/

template <typename DirT, bool DirImplementsCreator>
struct DirCreator_Impl
{
    static DirT* Create () {return DirT::Create();}
};

template <typename DirT>
struct DirCreator_Impl <DirT, false>        
{
    static DirT* Create () {return new DirT;}
};

template <typename DirT>
class Directory::DirCreator
    {
    template <typename DirTx>
    class ImplementsCreator
        {
        typedef int         _TRUE_TYPE;
        typedef char        _FASE_TYPE;

        static _TRUE_TYPE   _TestCreator (const DirTx*);
        static _FASE_TYPE   _TestCreator (...);

    public:
        enum
            {
            value = (sizeof(_TestCreator(DirTx::Create())) == sizeof(_TRUE_TYPE))
            };
        };
public:
    static DirT* Create () 
        {
        return DirCreator_Impl <DirT, ImplementsCreator<DirT>::value >::Create();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  Create a directory from its Create class member or via the operator
*               new. This decision is based on the presence of the Create class member.
* @see Directory::DirCreator
* @bsimethod                                                  Raymond.Gauthier   3/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
T* Directory::CreateDir ()
    {
    return DirCreator<T>::Create();
    }

//

/*---------------------------------------------------------------------------------**//**
* @description  Dummy Create used to evaluate whether to use a directory's
*               specific creator or the generic way (using operator new).
* @see Directory::DirCreator
* @bsimethod                                                  Raymond.Gauthier   3/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline Byte* Directory::Create ()
    {
    return 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
const PacketIterManager<EditorT>& Directory::PacketIterMgr () const
    {
    return static_cast<const PacketIterManager<EditorT>&>(PacketMgr());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
PacketIterManager<EditorT>& Directory::PacketIterMgr ()
    {
    return static_cast<PacketIterManager<EditorT>&>(PacketMgr());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
const HTGFF::SubDirManager<T>& Directory::SubDirMgr () const
    {
    return static_cast<const SubDirManager<T>&>(SubDirMgr());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
HTGFF::SubDirManager<T>& Directory::SubDirMgr ()
    {
    return static_cast<SubDirManager<T>&>(SubDirMgr());
    }

/*---------------------------------------------------------------------------------**//**
* @description     
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
const SubDirIterManager<EditorT>& Directory::SubDirIterMgr () const
    {
    return static_cast<const SubDirIterManager<EditorT>&>(SubDirMgr());
    }

/*---------------------------------------------------------------------------------**//**
* @description     
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
SubDirIterManager<EditorT>& Directory::SubDirIterMgr ()
    {
    return static_cast<SubDirIterManager<EditorT>&>(SubDirMgr());
    }