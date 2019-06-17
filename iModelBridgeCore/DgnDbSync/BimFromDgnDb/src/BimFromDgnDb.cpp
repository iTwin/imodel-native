/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <BimFromDgnDb/BimFromJson.h>
#include <BimFromDgnDb/DgnDb0601ToJson.h>
#include <Logging/bentleylogging.h>
#include <BimFromDgnDb/BimFromDgnDbAPI.h>

#include <folly/futures/Future.h>
#include <folly/ProducerConsumerQueue.h>
#include <Bentley/BeThread.h>
#include <folly/BeFolly.h>

#include "InteropHostProvider.h"
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY

using namespace BentleyB0200::Dgn::BimFromDgnDb;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
folly::Future<bool> ExportDgnDb(DgnDb0601ToJson* exporter)
    {
    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=] ()
        {
        return exporter->ExportDgnDb();
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
bool BimFromDgnDb::Upgrade(WCharCP inputPath, WCharCP outputPath)
    {
    DgnDb0601ToJson exporter(inputPath, InteropHostProvider::GetTemporaryDirectory(), InteropHostProvider::GetAssetsDirectory());
    auto logFunc = [] (BimFromDgnDbLoggingSeverity severity, const char* message)
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("BimFromDgnDbUpgrader")->message((SEVERITY) severity, message);
        };
    exporter.SetLogger(logFunc);

    auto perfLogFunc = [] (const char* message)
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("BimFromDgnDbUpgrader.Performance")->info(message);
        };
    exporter.SetPerformanceLogger(perfLogFunc);

    BimFromJson importer(outputPath);
    if (!importer.CreateBim())
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("BimFromDgnDbUpgrader")->fatal("Failed to create bim.  Aborting");
        return false;
        }

    exporter.SetQueueWrite([&importer] (const char* jsonEntry)
        {
        importer.AddToQueue(jsonEntry);
        });

    StopWatch totalTimer(true);
    auto future = ExportDgnDb(&exporter);
    importer.ImportJson(future);
    
    totalTimer.Stop();
    Utf8PrintfString message("Total upgrade|%.0f millisecs", totalTimer.GetElapsedSeconds() * 1000.0);
    BentleyApi::NativeLogging::LoggingManager::GetLogger("BimFromDgnDbUpgrader.Performance")->info(message.c_str());

    return true;
    }

