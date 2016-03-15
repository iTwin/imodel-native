/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Curl/NotificationPipe.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "NotificationPipe.h"

#include <Bentley/WString.h>
#include "../WebLogging.h"

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GetLastNativeSocketErrorForLog ()
    {
#if defined (BENTLEY_WIN32)
    return Utf8PrintfString ("WSAGetLastError %d", WSAGetLastError ()); // WIP: linker
#else
    return Utf8PrintfString ("errno %d", errno);
#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void AssertLastError ()
    {
    LOG.error (GetLastNativeSocketErrorForLog ().c_str ());
    BeAssert (false);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
NotificationPipe::NotificationPipe ()
#if !defined (BENTLEY_WIN32)
: m_writeStream (nullptr), m_readStream (nullptr)
#endif
    {
    m_pipe[0] = 0;
    m_pipe[1] = 0;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
NotificationPipe& NotificationPipe::GetDefault ()
    {
    static NotificationPipe s_notificationPipe = NotificationPipe ();

    return s_notificationPipe;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (BENTLEY_WIN32)
BentleyStatus NotificationPipe::MakeSocketPair (SOCKET fds[2])
    {
    struct sockaddr_in inaddr;
    struct sockaddr addr;

    memset (&inaddr, 0, sizeof(inaddr));
    memset (&addr, 0, sizeof(addr));

    inaddr.sin_family = AF_INET;
    inaddr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
    inaddr.sin_port = 0;

    SOCKET lst = ::socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);

    int yes = 1;
    setsockopt (lst, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));
    bind (lst, (struct sockaddr *)&inaddr, sizeof(inaddr));
    listen (lst, 1);
    int len = sizeof(inaddr);
    getsockname (lst, &addr, &len);
    fds[0] = ::socket (AF_INET, SOCK_STREAM, 0);
    connect (fds[0], &addr, len);
    fds[1] = accept (lst, 0, 0);
    closesocket (lst);

    if (0 == fds[0] || 0 == fds[1])
        {
        return ERROR;
        }
    return SUCCESS;
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus NotificationPipe::Open ()
    {
    // m_pipe[0] - read, m_pipe[1] - write
#if defined (BENTLEY_WIN32)
    if (SUCCESS != MakeSocketPair (m_pipe))
        {
        AssertLastError ();
        return ERROR;
        }
#else
    int status = pipe (m_pipe);
    if (0 != status)
        {
        AssertLastError ();
        return ERROR;
        }
    m_writeStream = fdopen (m_pipe[1], "w");
    m_readStream = fdopen (m_pipe[0], "r");
#endif
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus NotificationPipe::Close ()
    {
#if defined (BENTLEY_WIN32)
    closesocket (m_pipe[0]);
    closesocket (m_pipe[1]);
#else
    fclose (m_writeStream);
    fclose (m_readStream);
#endif
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus NotificationPipe::Send ()
    {
    if (m_pipe[1] == 0)
        {
        return ERROR;
        }
#if defined (BENTLEY_WIN32)
    int bytesSent = send (m_pipe[1], "n", 1, 0);
    if (bytesSent < 1)
        {
        AssertLastError ();
        return ERROR;
        }
#else
    fputc ('n', m_writeStream);
    fflush (m_writeStream);
#endif
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus NotificationPipe::Read ()
    {
    if (m_pipe[0] == 0)
        {
        return ERROR;
        }
#if defined (BENTLEY_WIN32)
    char buffer[1];
    int bytesReceived = recv (m_pipe[0], buffer, 1, 0);
    if (bytesReceived < 1)
        {
        AssertLastError ();
        return ERROR;
        }
#else
    fgetc (m_readStream);
#endif
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void NotificationPipe::AddListenFdToFdSet (fd_set& fdSetInOut, int& maxFdInOut)
    {
    int rFile = (int) m_pipe[0];
    FD_SET (rFile, &fdSetInOut);
    if (rFile > maxFdInOut)
        {
        maxFdInOut = rFile;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus NotificationPipe::ClearNotifications ()
    {
    int countReady = -1;
    while (countReady != 0)
        {
        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        int maxFd = -1;
        fd_set readFdSet;
        FD_ZERO (&readFdSet);

        AddListenFdToFdSet (readFdSet, maxFd);

        countReady = select (maxFd + 1, &readFdSet, NULL, NULL, &timeout);
        if (countReady < 0)
            {
            AssertLastError ();
            return ERROR;
            }

        if (countReady > 0)
            {
            if (SUCCESS != Read ())
                {
                return ERROR;
                }
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus NotificationPipe::WaitForNotifications ()
    {
    int maxfd = -1;
    fd_set readFdSet;
    FD_ZERO (&readFdSet);

    AddListenFdToFdSet (readFdSet, maxfd);

    int countReady = select (maxfd + 1, &readFdSet, NULL, NULL, NULL);
    if (countReady < 0)
        {
        AssertLastError ();
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus NotificationPipe::Notify ()
    {
    return Send ();
    }
