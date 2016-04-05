/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityCrawler/RealityCrawler.cs $
| 
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

// For the web crawler
using System.Net;
using System.Diagnostics;
using System.Configuration;
using BentleyCrawlerLib;

namespace Bentley.RealityPlatform.RealityCrawler
    {
    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette       1/2016
    //=====================================================================================
    class CrawlerObserver : BentleyCrawlerLib.ICrawlerObserverNet
        {
        public void OnPageCrawled(BentleyCrawlerLib.PageContentNet pageContent)
            {
            crawler_ProcessPageCrawlCompleted(pageContent);
            }
        
        private void crawler_ProcessPageCrawlCompleted(BentleyCrawlerLib.PageContentNet pageContent)
            {
            if (null == pageContent)
                return;

            if (string.IsNullOrEmpty (pageContent.GetText()))
                Console.WriteLine ("Page had no content {0}", pageContent.GetUrl().GetUrl().AbsoluteUri);
            else
                {
                Console.WriteLine("Analyzing the links of the PAGE {0}", pageContent.GetUrl().GetUrl().AbsoluteUri);
            
                KeywordDictionary dictionaryWMS = new KeywordDictionary ("DictionaryWMS.xml");
                Dictionary<int, string> crawledServerList = ServerWMS.GenerateCrawledServerList();
                if (pageContent.GetLinks() != null)
                    {
                    foreach (UrlNet foundLink in pageContent.GetLinks())
                        {
                        if (dictionaryWMS.FindKeyword(foundLink.GetUrl()))
                            {
                            if (!crawledServerList.ContainsValue(foundLink.GetUrl().AbsoluteUri.Split('?')[0]))
                                {
                                ServerWMS WMSKeywordURL = new ServerWMS (foundLink.GetUrl());
                                if ( WMSKeywordURL.isValid () )
                                    {
                                    Console.ForegroundColor = ConsoleColor.DarkGreen;
                                    Console.WriteLine ("Saving the WMS server link: {0}", foundLink.GetUrl().AbsoluteUri);
                                    Console.ResetColor ();
                                    }
                                WMSKeywordURL.Save ();
                                }
                            else
                                {
                                Console.ForegroundColor = ConsoleColor.DarkRed;
                                Console.WriteLine("Link already in the database: {0}", foundLink.GetUrl().AbsoluteUri);
                                Console.ResetColor ();
                                }
                            }
                        }
                    }
                }
            }
        }

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette       1/2016
    //=====================================================================================
    class RealityCrawler
        {
        static void Main (string[] args)
            {
            Console.Title = "GIS Crawler";            
            Stopwatch timer = new Stopwatch ();
            
            if(args.Length > 0)
                {
                if("-u" == args[0])
                    {
                    Console.WriteLine("----------------------");
                    Console.WriteLine("| Updating Database. |");
                    Console.WriteLine("----------------------");
                    Console.WriteLine();
                    timer.Start();
                    if (args.Length > 1)
                        Bentley.RealityPlatform.RealityCrawler.ServerWMS.Update(args[1]);
                    else
                        Bentley.RealityPlatform.RealityCrawler.ServerWMS.Update();
                    }
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
                    //crawlConfig.IsExternalPageCrawlingEnabled = seed.isExternalPageCrawlingEnabled;
                    //crawlConfig.IsExternalPageLinksCrawlingEnabled = seed.isExternalPageCrawlingEnabled;

                    // Create crawler.
                    CrawlerObserver observer = new CrawlerObserver();
                    BentleyCrawlerLibNet crawler = BentleyCrawlerLibNet.Create(5);

                    // Set options.
                    crawler.SetObserver(observer);
                    crawler.SetAcceptExternalLinks(true);
                    crawler.SetAcceptLinksInExternalLinks(true);
                    crawler.SetCrawlLinksWithHtmlTagRelNoFollow(true);
                    crawler.SetCrawlLinksFromPagesWithNoFollowMetaTag(true);
                    crawler.SetMaximumCrawlDepth(3);
                    crawler.SetMaxNumberOfLinkToCrawl(100000);

                    // Run.
                    crawler.Crawl(seed.uri);
                    
                    Console.ResetColor ();
                    Console.WriteLine ();
                    }
                }
            timer.Stop ();
            // ToDo : Mesure the download speed.
            TimeSpan timeTaken = timer.Elapsed;
            Console.WriteLine ();
            Console.WriteLine ("Total Crawling elapsed time: {0} Day(s), {1} hour(s) {2} minute(s) {3} second(s) {4} milisecond(s)", timeTaken.Days, timeTaken.Hours, timeTaken.Minutes, timeTaken.Seconds, timeTaken.Milliseconds);

            // Keep the console window open in debug mode.
            Console.WriteLine ("Press any key to exit.");
            Console.ReadKey ();
            }
        }
    }
