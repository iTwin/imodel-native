/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Curl/NotificationPipe.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

#include <WinSock2.h>
#include <fcntl.h>
#include <io.h>

#else

#include <fstream>
#include <ostream>
#include <unistd.h>
#include <errno.h>

#endif

#include <Bentley/Bentley.h>
#include <Bentley/BeThread.h>

#include <BeHttp/Http.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GetLastNativeSocketErrorForLog ();

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
* NotificationPipe is used to notify thread that is waiting with select()
* All methds should be called from same thread unless specified otherwise
+---------------+---------------+---------------+---------------+---------------+------*/
struct NotificationPipe
    {
private:
    BeMutex m_notifyMutex;

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    // Pipes do not work with select() on Windows. We use two sockets to fake pipe.
    SOCKET m_pipe[2];
#else
    // Unix style pipe
    int m_pipe[2];

    FILE* m_writeStream;
    FILE* m_readStream;
#endif

private:
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    BentleyStatus MakeSocketPair (SOCKET fds[2]);
#endif
    BentleyStatus Send ();
    BentleyStatus Read ();
    void Clear();

public:
    //! Create new pipe. Internal, use GetDefault() for most cases.
    NotificationPipe ();

    //! Get default pipe.
    static NotificationPipe& GetDefault ();

    //! Open pipe if not yet open or closed.
    BentleyStatus Open ();
    //! Close pipe if open.
    BentleyStatus Close ();
    //! Check if pipe is open now.
    bool IsOpen ();

    //! Add listen FD to FD_SET to use it for select(). Listens for Notify() notifications.
    void AddListenFdToFdSet (fd_set& fdSetInOut, int& maxFdInOut);

    //! Clear notifications added by Notify()
    BentleyStatus ClearNotifications ();
    //! Wait until new notifications are added by Notify()
    BentleyStatus WaitForNotifications ();

    //! Notify listeners. This is only method that can be called from other thread.
    BentleyStatus Notify ();
    };

END_BENTLEY_HTTP_NAMESPACE
