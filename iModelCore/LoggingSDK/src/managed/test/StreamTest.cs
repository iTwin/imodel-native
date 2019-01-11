/*--------------------------------------------------------------------------------------+
|
|     $Source: src/managed/test/StreamTest.cs $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.IO;
using Bentley.Logging;

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

            string configuration =
                        "<configuration>"
                        + "<log4net>"
                            + " <appender name=\"ConsoleAppender\" type=\"log4net.Appender.ConsoleAppender\">"
                            + "  <layout type=\"log4net.Layout.PatternLayout\">"
                            + "      <param name=\"ConversionPattern\" value=\"STREAM CONFIGURATION - %m%n\" />"
                            + "   </layout>"
                            + " </appender>"

                            + " <root>"
                            + "  <level value=\"TRACE\" />"
                            + "  <appender-ref ref=\"ConsoleAppender\" />"
                            + " </root>"
                         + "</log4net>"
                        + "</configuration>";

             MemoryStream memStream = new MemoryStream();
             StreamWriter sw = new StreamWriter(memStream);
             sw.Write(configuration);
             sw.Flush();

             //Return to the beginning of the stream
             memStream.Seek(0, SeekOrigin.Begin);
             Log4netProvider logProvider = new Log4netProvider(memStream);
             LoggerConfig.registerLogInterface ( logProvider );


            ILogger logger = LoggerFactory.createLogger ( "MyNamspace" );

            logger.message ( SEVERITY.ERROR, "Some error message that I don't care about" );

            logger.message ( SEVERITY.TRACE, "Some trace message that I don't care about" );

            logger.message ( SEVERITY.DEBUG, "Some debug message that I don't care about" );

            logger.message ( SEVERITY.WARNING, "Some warning message that I don't care about" );

            logger.message ( SEVERITY.FATAL, "Some fatal message that I don't care about" );

            logger.message ( SEVERITY.FATAL, "Some fatal message {0} that I don't care about", 2134523 );

            logger.info("Some information message that I don't care about");

            logger.error ( "Another error mesasge" );

            logger.error ( "Another {0} error mesasge", "\"inserted string\"" );

            //
            // TODO: Add code to start application here
            //

            //finished
            sw.Close();
        }
    }
}
