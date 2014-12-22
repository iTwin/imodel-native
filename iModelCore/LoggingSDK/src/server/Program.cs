/*--------------------------------------------------------------------------------------+
|
|     $Source: logging/server/Program.cs $
|
|  $Copyright: (c) 2008 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------+
|
|   Usings
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Collections.Generic;
using System.ServiceProcess;
using System.Text;

namespace BentleyLoggingServer
{
    static class Program
    {
        /// *****************************************************************************
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        ///  @author                                             AnthonyFalcone 01/08
        /// *****************************************************************************
        static void Main()
        {
            ServiceBase[] ServicesToRun;

            // More than one user Service may run within the same process. To add
            // another service to this process, change the following line to
            // create a second service object. For example,
            //
            //   ServicesToRun = new ServiceBase[] {new Service1(), new MySecondUserService()};
            //
            ServicesToRun = new ServiceBase[] { new Service() };

            ServiceBase.Run(ServicesToRun);
        }
    }
}