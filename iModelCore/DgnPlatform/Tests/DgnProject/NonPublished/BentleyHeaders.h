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
#include <Bentley/ReleaseMarshaller.h>
//#include <Bentley/ScopedArray.h>
#include <Bentley/ValueFormat.h>
#include <Bentley/WString.h>
#include <Bentley/btree/btree.h>
#include <Bentley/btree/btree_container.h>
#include <Bentley/btree/btree_map.h>
#include <Bentley/btree/btree_set.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                        Umar.Hayat                05/15
//---------------------------------------------------------------------------------------
void GetBentleyStructList(bvector<TypeNamePair>& list)
{
    list.push_back(TypeNamePair(sizeof(Base64Utilities), L"Base64Utilities"));
    list.push_back(TypeNamePair(sizeof(BeAssertFunctions), L"BeAssertFunctions"));
    list.push_back(TypeNamePair(sizeof(BeAtomic<uint64_t>), L"BeAtomic<uint64_t>"));
    list.push_back(TypeNamePair(sizeof(BeAtomic<uint32_t>), L"BeAtomic<uint32_t>"));
    list.push_back(TypeNamePair(sizeof(BeAtomic<int>), L"BeAtomic<int>"));
    list.push_back(TypeNamePair(sizeof(BeConsole), L"BeConsole"));
    list.push_back(TypeNamePair(sizeof(BeDebugLogFunctions), L"BeDebugLogFunctions"));
    list.push_back(TypeNamePair(sizeof(FileNamePattern), L"FileNamePattern"));
    list.push_back(TypeNamePair(sizeof(BeDirectoryIterator), L"BeDirectoryIterator"));
    list.push_back(TypeNamePair(sizeof(BeFile), L"BeFile"));
    list.push_back(TypeNamePair(sizeof(BeFileListIterator), L"BeFileListIterator"));
    list.push_back(TypeNamePair(sizeof(BeFileName), L"BeFileName"));
    list.push_back(TypeNamePair(sizeof(BeIconUtilities), L"BeIconUtilities"));
    list.push_back(TypeNamePair(sizeof(BeNumerical), L"BeNumerical"));
    list.push_back(TypeNamePair(sizeof(BeSharedMutex), L"BeSharedMutex"));
    list.push_back(TypeNamePair(sizeof(BeSharedMutexHolder), L"BeSharedMutexHolder"));
    list.push_back(TypeNamePair(sizeof(BeStringUtilities), L"BeStringUtilities"));
    list.push_back(TypeNamePair(sizeof(BeSystemInfo), L"BeSystemInfo"));
    list.push_back(TypeNamePair(sizeof(BeTest), L"BeTest"));
    list.push_back(TypeNamePair(sizeof(BeMutex), L"BeMutex"));
    list.push_back(TypeNamePair(sizeof(BeMutexHolder), L"BeMutexHolder"));
    list.push_back(TypeNamePair(sizeof(BeConditionVariable), L"BeConditionVariable"));
    list.push_back(TypeNamePair(sizeof(BeThreadUtilities), L"BeThreadUtilities"));
    list.push_back(TypeNamePair(sizeof(BeThread), L"BeThread"));
    list.push_back(TypeNamePair(sizeof(BeSystemMutexHolder), L"BeSystemMutexHolder"));
    list.push_back(TypeNamePair(sizeof(BeThreadLocalStorage), L"BeThreadLocalStorage"));
    list.push_back(TypeNamePair(sizeof(BeTimeUtilities), L"BeTimeUtilities"));
    list.push_back(TypeNamePair(sizeof(StopWatch), L"StopWatch"));
    list.push_back(TypeNamePair(sizeof(BeVersion), L"BeVersion"));
    list.push_back(TypeNamePair(sizeof(bmap<WString,WString>), L"bmap<WString,WString>"));
    list.push_back(TypeNamePair(sizeof(bmultimap<WString, WString>), L"bmultimap<WString, WString>"));
    list.push_back(TypeNamePair(sizeof(bset<WString, WString>), L"bset<WString, WString>"));
    list.push_back(TypeNamePair(sizeof(bvector<WString>), L"bvector<WString>"));
    list.push_back(TypeNamePair(sizeof(DateTime), L"DateTime"));
    list.push_back(TypeNamePair(sizeof(GlobalHandleContainer), L"GlobalHandleContainer"));
    list.push_back(TypeNamePair(sizeof(Iota), L"Iota"));
    list.push_back(TypeNamePair(sizeof(NonCopyableClass), L"NonCopyableClass"));
    list.push_back(TypeNamePair(sizeof(ReleaseMarshaller), L"ReleaseMarshaller"));
    list.push_back(TypeNamePair(sizeof(DoubleFormatterBase), L"DoubleFormatterBase"));
    list.push_back(TypeNamePair(sizeof(DoubleFormatter), L"DoubleFormatter"));
    //list.push_back(TypeNamePair(sizeof(WStringddddddddddd), L"WStringddddddddddd"));
    list.push_back(TypeNamePair(sizeof(WString), L"WString"));

}



