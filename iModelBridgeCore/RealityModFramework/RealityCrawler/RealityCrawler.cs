using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

// For the web crawler
using Abot.Crawler;
using Abot.Poco;
using Abot.Core;
using System.Net;
using log4net.Config;
using Bentley.RealityPlatform.RealityCrawler;
using System.Diagnostics;
using System.Configuration;

// For threading
//using System.Threading;

namespace Bentley.RealityPlatform.RealityCrawler
    {
    public class CustomCrawlDecisionMaker : CrawlDecisionMaker
        {
        public override CrawlDecision ShouldCrawlPage (PageToCrawl pageToCrawl, CrawlContext crawlContext)
            {
            CrawlDecision decision = base.ShouldCrawlPage (pageToCrawl, crawlContext);
            if ( decision.Allow && !pageToCrawl.IsRoot )
                {
                if ( pageToCrawl.Uri.Authority == "google.com" )
                    decision = new CrawlDecision
                    {
                        Allow = false, Reason = "Ignoring google links"
                    };
                }

            return decision;
            }

        public override CrawlDecision ShouldCrawlPageLinks (CrawledPage crawledPage, CrawlContext crawlContext)
            {
            CrawlDecision decision = base.ShouldCrawlPageLinks (crawledPage, crawlContext);
            //if (decision.Allow && !crawledPage.IsRoot)
            //{
            //    if (crawledPage.Uri.Authority == "google.com")
            //        decision = new CrawlDecision { Allow = false, Reason = "Ignoring google links" };
            //}

            return decision;
            }
        }

    class RealityCrawler
        {
        // For threading
        //private System.Object lockThis = new System.Object();

        static void Main (string[] args)
            {
            Console.Title = "GIS Crawler";
            //m_dictionaryWMS = new Dictionary();
            log4net.Config.XmlConfigurator.Configure ();
            //PrintDisclaimer();

            Console.WriteLine ("GIS Crawler");
            Console.WriteLine ("Press 'u' key to update or any key to crawl.");
            ConsoleKeyInfo cki = Console.ReadKey ();
            Console.Clear ();
            Stopwatch timer = new Stopwatch ();
            
            if ( cki.KeyChar == 'u' )
                {
                Console.WriteLine ("----------------------");
                Console.WriteLine ("| Updating Database. |");
                Console.WriteLine ("----------------------");
                Console.WriteLine ();
                timer.Start ();
                Bentley.RealityPlatform.RealityCrawler.ServerWMS.Update ();
                }
            else
                {
                Console.WriteLine ("---------------------");
                Console.WriteLine ("| Crawling the Web. |");
                Console.WriteLine ("---------------------");
                Console.WriteLine ();
                SeedReader seeds = new SeedReader ();
                timer.Start ();
                foreach ( Seed seed in seeds.seedList )
                    {
                    Console.ForegroundColor = ConsoleColor.DarkGray;
                    Console.WriteLine ("Time: {0}, {1}", DateTime.Now.ToLongDateString(), DateTime.Now.ToLongTimeString ());
                    Console.WriteLine ("About to crawl seed {0}", seed.uri.AbsoluteUri);
                    Console.ResetColor ();

                    // Load from app.config then tweek the abot crawler.
                    CrawlConfiguration crawlConfig = AbotConfigurationSectionHandler.LoadFromXml().Convert();
                    crawlConfig.IsExternalPageCrawlingEnabled = seed.isExternalPageCrawlingEnabled;
                    crawlConfig.IsExternalPageLinksCrawlingEnabled = seed.isExternalPageCrawlingEnabled;
                    crawlConfig.MaxCrawlDepth = 3;
                    crawlConfig.MaxPagesToCrawl = 100000;

                    IWebCrawler crawler;

                    crawler = GetDefaultWebCrawler (crawlConfig);

                    crawler.PageCrawlStartingAsync += crawler_ProcessPageCrawlStarting;
                    crawler.PageCrawlCompletedAsync += crawler_ProcessPageCrawlCompleted;
                    crawler.PageCrawlDisallowedAsync += crawler_PageCrawlDisallowed;
                    crawler.PageLinksCrawlDisallowedAsync += crawler_PageLinksCrawlDisallowed;

                    // Log the crawl
                    XmlConfigurator.Configure ();

                    //Start the crawl
                    //This is a synchronous call
                    //CrawlResult result = crawler.Crawl(uriToCrawl);

                    CrawlResult result = crawler.Crawl (seed.uri);
                    Console.ForegroundColor = ConsoleColor.DarkGray;
                    System.Console.WriteLine ("Seed Elapsed Time:{0} milliseconds", result.Elapsed.TotalMilliseconds);
                    Console.ResetColor ();
                    Console.WriteLine ();
                    }
                }
            timer.Stop ();
            // ToDo : Mesure the download speed.
            TimeSpan timeTaken = timer.Elapsed;
            Console.WriteLine ();
            Console.WriteLine ("Total Crawling elapsed time: {0} Day(s), {1} hour(s) {2} minute(s) {3} second(s) {4} milisecond(s)", timeTaken.Days, timeTaken.Hours, timeTaken.Minutes, timeTaken.Seconds, timeTaken.Milliseconds);

            //Now go view the log.txt file that is in the same directory as this executable. It has
            //all the statements that you were trying to read in the console window :).
            //Not enough data being logged? Change the app.config file's log4net log level from "INFO" TO "DEBUG"

            //PrintDisclaimer();

            // Keep the console window open in debug mode.
            Console.WriteLine ("Press any key to exit.");
            Console.ReadKey ();
            }

        private static IWebCrawler GetDefaultWebCrawler (CrawlConfiguration crawlConfig)
            {
            //CustomCrawlDecisionMaker myCustomCrawlerInstance = new CustomCrawlDecisionMaker ();
            //return new PoliteWebCrawler();
            //Will use the manually created crawlConfig object created above
            return new  PoliteWebCrawler(crawlConfig, null, null, null, null, null, null, null, null);
            //return new PoliteWebCrawler (null, myCustomCrawlerInstance, null, null, null, null, null, null, null);
            }

        private static void PrintDisclaimer ()
            {
            PrintAttentionText ("The demo is configured to only crawl a total of 2000 pages and will wait 1 second in between http requests. This is to avoid getting you blocked by your isp or the sites you are trying to crawl. You can change these values in the app.config or Abot.Console.exe.config file.");
            }

        private static void PrintAttentionText (string text)
            {
            ConsoleColor originalColor = System.Console.ForegroundColor;
            System.Console.ForegroundColor = ConsoleColor.Yellow;
            System.Console.WriteLine (text);
            System.Console.ForegroundColor = originalColor;
            }

        static void crawler_ProcessPageCrawlStarting (object sender, PageCrawlStartingArgs e)
            {
            PageToCrawl pageToCrawl = e.PageToCrawl;
            Console.WriteLine ("About to crawl link {0} which was found on page {1}", pageToCrawl.Uri.AbsoluteUri, pageToCrawl.ParentUri.AbsoluteUri);
            }

        /// <summary>
        ///
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        /// <remarks>
        /// ToDo: - Manage multi threading.
        /// - Filter results.
        /// </remarks>
        static void crawler_ProcessPageCrawlCompleted (object sender, PageCrawlCompletedArgs e)
            {
            CrawledPage crawledPage = e.CrawledPage;

            //if ( crawledPage.WebException != null || crawledPage.HttpWebResponse.StatusCode != HttpStatusCode.OK )
            //    Console.WriteLine ("Crawl of page failed {0}", crawledPage.Uri.AbsoluteUri);
            //else
            //    Console.WriteLine ("Crawl of page succeeded {0}", crawledPage.Uri.AbsoluteUri);

            if ( string.IsNullOrEmpty (crawledPage.Content.Text) )
                Console.WriteLine ("Page had no content {0}", crawledPage.Uri.AbsoluteUri);
            else
                {                
                Console.WriteLine ("Analyzing the links of the PAGE {0}", crawledPage.Uri.AbsoluteUri);

                KeywordDictionary dictionaryWMS = new KeywordDictionary ("DictionaryWMS.xml");
                //SeedCreator foundSeed = new SeedCreator ();
                Dictionary<int, string>CrawledServerList = ServerWMS.GenerateCrawledServerList();
                if (crawledPage.ParsedLinks != null)
                    {                    
                    foreach ( Uri foundLink in crawledPage.ParsedLinks )
                        {
                        if ( dictionaryWMS.FindKeyword (foundLink) )
                            {
                            if ( !CrawledServerList.ContainsValue (foundLink.AbsoluteUri.Split ('?')[0]) )
                                {
                                ServerWMS WMSKeywordURL = new ServerWMS (foundLink);
                                if ( WMSKeywordURL.isValid () )
                                    {
                                    Console.ForegroundColor = ConsoleColor.DarkGreen;
                                    Console.WriteLine ("Saving the WMS server link: {0}", foundLink.AbsoluteUri);
                                    Console.ResetColor ();
                                    }
                                WMSKeywordURL.Save ();
                                //RRHandler Test = new RRHandler(foundLink);
                                //foundSeed.Add(foundLink);
                                //Test.GetXMLStream();
                                }
                            else
                                {
                                Console.ForegroundColor = ConsoleColor.DarkRed;
                                Console.WriteLine ("Link already in the database: {0}", foundLink.AbsoluteUri);
                                Console.ResetColor ();
                                }
                            }
                        }
                    }
                //foundSeed.Write();
                }
            }

        static void crawler_PageLinksCrawlDisallowed (object sender, PageLinksCrawlDisallowedArgs e)
            {
            CrawledPage crawledPage = e.CrawledPage;
            Console.WriteLine ("Did not crawl the links on page {0} due to {1}", crawledPage.Uri.AbsoluteUri, e.DisallowedReason);
            }

        static void crawler_PageCrawlDisallowed (object sender, PageCrawlDisallowedArgs e)
            {
            PageToCrawl pageToCrawl = e.PageToCrawl;
            Console.WriteLine ("Did not crawl page {0} due to {1}", pageToCrawl.Uri.AbsoluteUri, e.DisallowedReason);
            }

        }
    }
