/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/BentleyHeaders.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// Bentley API Headers
#include <Bentley/Base64Utilities.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeAtomic.h>
#include <Bentley/BeConsole.h>
#include <Bentley/BeDebugLog.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeFileListIterator.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeIconUtilities.h>
//Android only
//#include <Bentley/BeJStringUtilities.h>
#include <Bentley/Bentley.h>
#include <Bentley/BentleyAllocator.h>
#include <Bentley/BeNumerical.h>
#include <Bentley/BeSharedMutex.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeSystemInfo.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeThreadLocalStorage.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeVersion.h>
//#include <Bentley/bstdmap.h>
//#include <Bentley/bstdset.h>
#include <Bentley/bvector.h>
#include <Bentley/CatchNonPortable.h>
#include <Bentley/CodePages.h>
#include <Bentley/DateTime.h>
#include <Bentley/GlobalHandleContainer.h>
#include <Bentley/Iota.h>
#include <Bentley/NonCopyableClass.h>
#include <Bentley/RefCounted.h>
//#include <Bentley/ReleaseMarshaller.h>   // this header use windows.h
//#include <Bentley/ScopedArray.h>
#include <Bentley/ValueFormat.h>
#include <Bentley/WString.h>
#include <Bentley/btree/btree.h>
#include <Bentley/btree/btree_container.h>
#include <Bentley/btree/btree_map.h>
#include <Bentley/btree/btree_set.h>

#define MakeTypeInfo(typeName) \
    TypeInfo(sizeof(typeName), __alignof(typeName), WString(#typeName , true))

#define MakeTypeInfoAndAddToList(typeName) \
    list.push_back(MakeTypeInfo(typeName))

//---------------------------------------------------------------------------------------
// @bsimethod                                        Umar.Hayat                05/15
//---------------------------------------------------------------------------------------
void GetBentleyStructList(bvector<TypeInfo>& list)
{
    MakeTypeInfoAndAddToList(Base64Utilities);
    MakeTypeInfoAndAddToList(Base64Utilities);
    MakeTypeInfoAndAddToList(Base64Utilities);
    MakeTypeInfoAndAddToList(BeAssertFunctions);

    // typedef used to properply stringify the MACROs as data type may resolve into BeAtomic or std::atomic , in case of std::atomic it will not stringify in the macro properly , ( more details check for BENTLEY_HAVE_STD_ATOMIC in BeAtomic.h )
    typedef BeAtomic<uint64_t> BeAtomic_uint64_t;
    MakeTypeInfoAndAddToList(BeAtomic_uint64_t);
    typedef BeAtomic<uint32_t> BeAtomic_uint32_t;
    MakeTypeInfoAndAddToList(BeAtomic_uint32_t);
    typedef BeAtomic<int> BeAtomic_int;
    MakeTypeInfoAndAddToList(BeAtomic_int);

    MakeTypeInfoAndAddToList(BeConsole);
    MakeTypeInfoAndAddToList(BeDebugLogFunctions);
    MakeTypeInfoAndAddToList(FileNamePattern);
    MakeTypeInfoAndAddToList(BeDirectoryIterator);
    MakeTypeInfoAndAddToList(BeFile);
    MakeTypeInfoAndAddToList(BeFileListIterator);
    MakeTypeInfoAndAddToList(BeFileName);
    MakeTypeInfoAndAddToList(BeIconUtilities);
    MakeTypeInfoAndAddToList(BeNumerical);
    //MakeTypeInfoAndAddToList(BeSharedMutex);
    MakeTypeInfoAndAddToList(BeSharedMutexHolder);
    MakeTypeInfoAndAddToList(BeStringUtilities);
    MakeTypeInfoAndAddToList(BeSystemInfo);
    MakeTypeInfoAndAddToList(BeTest);
    //MakeTypeInfoAndAddToList(BeMutex);
    MakeTypeInfoAndAddToList(BeMutexHolder);
    //MakeTypeInfoAndAddToList(BeConditionVariable);
    MakeTypeInfoAndAddToList(BeThreadUtilities);
    //MakeTypeInfoAndAddToList(BeThread);
    MakeTypeInfoAndAddToList(BeSystemMutexHolder);
    //MakeTypeInfoAndAddToList(BeThreadLocalStorage);
    MakeTypeInfoAndAddToList(BeTimeUtilities);
    MakeTypeInfoAndAddToList(StopWatch);
    MakeTypeInfoAndAddToList(BeVersion);
    // Typedef are used to remove spaces and commas to resolve/strigify properly
    typedef bmap<WString, WString> bmap_WString_WString;
    MakeTypeInfoAndAddToList(bmap_WString_WString);
    typedef bmultimap<WString, WString> bmultimap_WString_WString;
    MakeTypeInfoAndAddToList(bmultimap_WString_WString);
    typedef bset<WString, WString> bset_WString_WString;
    MakeTypeInfoAndAddToList(bset_WString_WString);
    typedef bvector<WString> bvector_WString;
    MakeTypeInfoAndAddToList(bvector_WString);
    MakeTypeInfoAndAddToList(DateTime);
    MakeTypeInfoAndAddToList(GlobalHandleContainer);
    MakeTypeInfoAndAddToList(Iota);
    MakeTypeInfoAndAddToList(NonCopyableClass);
    //MakeTypeInfoAndAddToList(ReleaseMarshaller);
    MakeTypeInfoAndAddToList(DoubleFormatterBase);
    MakeTypeInfoAndAddToList(DoubleFormatter);
    MakeTypeInfoAndAddToList(WString);

}



