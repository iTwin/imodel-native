/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Curl/NotificationPipe.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#if defined (BENTLEY_WIN32)

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

#include <BeHttp/Http.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GetLastNativeSocketErrorForLog ();

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct NotificationPipe
    {
private:
#if defined (BENTLEY_WIN32)
    // Pipes do not work with select() on Windows. We use two sockets to fake pipe.
    SOCKET m_pipe[2];
#else
    // Unix style pipe
    int m_pipe[2];

    FILE* m_writeStream;
    FILE* m_readStream;
#endif

private:
#if defined (BENTLEY_WIN32)
    BentleyStatus MakeSocketPair (SOCKET fds[2]);
#endif
    BentleyStatus Send ();
    BentleyStatus Read ();
public:
    //! NotificationPipe is used to notify thread that is waiting with select()
    //! All methds should be called from same thread unless specified otherwise
    NotificationPipe ();

    static NotificationPipe& GetDefault ();

    BentleyStatus Open ();
    BentleyStatus Close ();

    //! Add listen FD to FD_SET to use it for select()
    void AddListenFdToFdSet (fd_set& fdSetInOut, int& maxFdInOut);

    BentleyStatus ClearNotifications ();
    BentleyStatus WaitForNotifications ();

    //! This is only method that can be called from other thread
    BentleyStatus Notify ();
    };

END_BENTLEY_HTTP_NAMESPACE
