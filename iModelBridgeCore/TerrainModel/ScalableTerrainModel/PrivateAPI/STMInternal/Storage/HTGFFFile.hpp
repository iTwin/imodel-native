//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFFile.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------



template <typename RootDirT>
RootDirT* File::GetRootDir () const
    {
    return const_cast<File&>(*this).GetRootDir<RootDirT>();
    }

template <typename RootDirT>
RootDirT* File::GetRootDir ()
    {
    // Return stored root dir
    Directory* pRootDir = GetRootDir();
    if (0 != pRootDir)
        return static_cast<RootDirT*>(pRootDir);

    // If root dir was not yet initialized, do it.
    HFCPtr<RootDirT> pDir(new RootDirT());
    if (!pDir->InitializeRoot(GetFileHandle(), 0, 0))
        {
        HASSERT(!"Unable to initialize Root directory");
        return 0;
        }
    SetRoot(pDir.GetPtr());

    return pDir.GetPtr();
    }
