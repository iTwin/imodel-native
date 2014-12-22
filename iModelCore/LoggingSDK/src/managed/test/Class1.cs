/*--------------------------------------------------------------------------------------+
|
|     $Source: src/managed/test/Class1.cs $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using Bentley.Logging;
using Bentley.Logging.Provider;

namespace test
{
    /// <summary>
    /// Summary description for Class1.
    /// </summary>
    class Class1
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            //NativeInteropProvider logProvider = new NativeInteropProvider();
            Log4netProvider logProvider = new Log4netProvider("example_log4net_configuration.xml");

            LoggerConfig.registerLogInterface ( logProvider );

            ILogger logger = LoggerFactory.createLogger ( "MyNamspace" );

            logger.message ( SEVERITY.ERROR, "Some error message that I don't care about" );

            logger.message ( SEVERITY.TRACE, "Some trace message that I don't care about" );

            ILogger logger2 = LoggerFactory.createLogger ("AnotherNameSpace");

            logger2.message ( SEVERITY.DEBUG, "Some debug message that I don't care about" );

            logger2.message ( SEVERITY.WARNING, "Some warning message that I don't care about" );

            logger.message ( SEVERITY.FATAL, "Some fatal message that I don't care about" );

            logger.message ( SEVERITY.FATAL, "Some fatal message {0} that I don't care about", 2134523 );

            logger.info("Some information message that I don't care about");

            logger.error ( "Another error message" );

            logger.error ( "Another {0} error mesasge", "\"inserted string\"" );

            //
            // TODO: Add code to start application here
            //
        }
    }
}
