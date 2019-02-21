/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/DirectoryObserver.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <Bentley/Tasks/WorkerThread.h>
#include <Bentley/BeFile.h>

BEGIN_BENTLEY_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
enum class FileChanges
    {
    FileAdded     = 1, //!< A file was added to an observed directory.
    FileMofified  = 2, //!< IOS!
    FileRemoved   = 3,
#if defined(ENABLE_RENAME)
    FileRenamed_oldName = 4,
    FileRenamed_newName = 5,
#endif
    };

typedef std::function<void(BeFileNameCR changedDir, FileChanges changeStatus)> ObserverCallback;
typedef RefCountedPtr<struct DirectoryObserver> DirectoryObserverPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct DirectoryObserver : RefCountedBase
    {
private:
    ObserverCallback    m_callback;
    Tasks::WorkerThreadPtr     m_waitingThread;
    Tasks::WorkerThreadPtr     m_callbackThread;

    BeAtomic<bool>      m_stoppedObserving;
    BeConditionVariable m_stoppedObservingLock;

protected:
    bset<BeFileName>    m_directories;
    BeAtomic<bool>      m_isObserving;

    BeAtomic<bool>      m_startedObserving;
    BeConditionVariable m_startedObservingLock;

protected:
    DirectoryObserver(bset<BeFileName> const& directories, ObserverCallback callback);
    DirectoryObserver(ObserverCallback callback);
    virtual ~DirectoryObserver();

    void RunCallbackAsync(BeFileNameCR changedDir, FileChanges changeStatus);

    virtual void _StartObserving() = 0;
    virtual void _StopObserving() = 0;
    virtual void _Observe() = 0;
    virtual void _OnObservingStopped() {}

public:

    BENTLEYDLL_EXPORT static DirectoryObserverPtr Create(const bset<BeFileName>& directories, ObserverCallback callback);
    BENTLEYDLL_EXPORT static DirectoryObserverPtr Create(ObserverCallback callback);


    //! Returns a set of directories monitored.
    BENTLEYDLL_EXPORT bset<BeFileName>const& GetObservedDirectories() const;
   
    //! Returns status of the observer.
    BENTLEYDLL_EXPORT bool GetObserverStatus() const;

    //! Adds a directory to the monitored directory list.
    //! @param[in] fileNameRef directory to be added.
    //! @note Adding a directory restarts observer routine, thus some events might be lost in a proccess
    //! @note Prefer using @ref AddObservedDirectories instead of this as the restart is performed once per changeset instead of once per directory
    BENTLEYDLL_EXPORT BentleyStatus AddObservedDirectory(BeFileNameCR fileNameRef);
    
    //! Adds a directory to the monitored directory list.
    //! @param[in] fileNames directories to be added.
    //! @note Adding a directory restarts observer routine, thus some events might be lost in a proccess
    //! @note Prefer using this instead of @ref AddObservedDirectory as the restart is performed once per changeset instead of once per directory
    BENTLEYDLL_EXPORT BentleyStatus AddObservedDirectories(bset<BeFileName>const& fileNames);

    //! Removes a directory from the monitored directory list.
    //! @param[in] fileNameRef directory to be removed.
    //! @note Removing a directory restarts observer routine, thus some events might be lost in a proccess
    //! @note Prefer using @ref RemoveObservedDirectories instead of this as the restart is performed once per changeset instead of once per directory
    BENTLEYDLL_EXPORT BentleyStatus RemoveObservedDirectory(BeFileNameCR fileNameRef);
    
    //! Removes a directory from the monitored directory list.
    //! @param[in] fileNames directories to be removed.
    //! @note Removing a directory restarts observer routine, thus some events might be lost in a proccess
    //! @note Prefer using this instead of @ref RemoveObservedDirectory as the restart is performed once per changeset instead of once per directory
    BENTLEYDLL_EXPORT BentleyStatus RemoveObservedDirectories(bset<BeFileName>const& fileNames);
    
    //! Removes all directories from the monitored directory list and stops the observer.
    //! @see StartObserving
    BENTLEYDLL_EXPORT BentleyStatus RemoveAllObservedDirectories();

    //! Starts the observer with current observed directory list.
    //! @note Fails if monitor is already started or observed directory list is empty
    BENTLEYDLL_EXPORT BentleyStatus StartObserving();
    
    //! Stops the observer without changing the observed directory list.
    BENTLEYDLL_EXPORT BentleyStatus StopObserving();
    };

END_BENTLEY_NAMESPACE
//__PUBLISH_SECTION_END__
