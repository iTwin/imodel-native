/*--------------------------------------------------------------------------------------+
|
|     $Source: src/managed/console/console.cs $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using Bentley.Logging;

namespace Bentley.Logging.Provider
{
    class ConsoleProviderContext : ILogProviderContext
    {
        string      m_nameSpace;
        SEVERITY     m_sev;

        public ConsoleProviderContext ( string nameSpace, SEVERITY sev )
        {
            m_nameSpace = nameSpace;
            m_sev = sev;
        }

        public string getNameSpace()
        {
            return m_nameSpace;
        }

        public SEVERITY getSeverity()
        {
            return m_sev;
        }

    }

    public class ConsoleProvider : ILogProvider
    {
        public ConsoleProvider()
        {
        }

        public int createLogger ( string nameSpace, out ILogProviderContext pContext )
        {
            pContext = new ConsoleProviderContext ( nameSpace, SEVERITY.TRACE );

            return 0;
        }

        public int destroyLogger ( Provider.ILogProviderContext pContext )
        {
            return 0;
        }

        public int logMessage ( ILogProviderContext context, SEVERITY sev, string msg )
        {
            ConsoleProviderContext consoleContext = (ConsoleProviderContext)context;

            Console.WriteLine ( "{0} {1} {2}", consoleContext.getNameSpace(), sev, msg );

            return 0;
        }

        public bool isSeverityEnabled ( ILogProviderContext context, SEVERITY sev )
        {
            ConsoleProviderContext consoleContext = (ConsoleProviderContext)context;

            return ( sev > consoleContext.getSeverity() );
        }

    }
}