/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BentleyCrawlerLib;

namespace CrawlerLibNetTest
{
    class CrawlerTest : ICrawlerObserverNet
    {
        private BentleyCrawlerLibNet crawler;
        public CrawlerTest()
        {
            crawler = BentleyCrawlerLibNet.Create(25);
        }

        public void OnPageCrawled(PageContentNet page)
        {
            Console.WriteLine("OnPageCrawled");
        }

        public void Run()
        {
            Uri seed = new Uri("http://usgs.gov");
            crawler.SetObserver(this);
            crawler.Crawl(seed);
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            CrawlerTest Test = new CrawlerTest();
            Test.Run();
        }
    }
}
