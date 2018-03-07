/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/src/DgnDbToBimConverter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BimTeleporter/BisJson1Importer.h>
#include <BimTeleporter/BisJson1Exporter0601.h>
#include <Logging/bentleylogging.h>
#include <BimTeleporter/DgnDbToBimConverter.h>

#include <folly/futures/Future.h>
#include <folly/ProducerConsumerQueue.h>
#include <Bentley/BeThread.h>
#include <folly/BeFolly.h>

USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY
USING_DGNDB_TO_BIM_NAMESPACE
using namespace BentleyB0200::Dgn::BimTeleporter;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
folly::Future<bool> ExportDgnDb(BisJson1Exporter0601* exporter)
    {
    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=] ()
        {
        return exporter->ExportDgnDb();
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
bool DgnDbToBimConverter::Convert(WCharCP inputPath, WCharCP outputPath)
    {
    BisJson1Exporter0601 exporter(inputPath);
    auto logFunc = [] (TeleporterLoggingSeverity severity, const char* message)
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("DgnDbToBimConverter")->message((SEVERITY) severity, message);
        };
    exporter.SetLogger(logFunc);

    auto perfLogFunc = [] (const char* message)
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("DgnDbToBimConverter.Performance")->info(message);
        };
    exporter.SetPerformanceLogger(perfLogFunc);

    BisJson1Importer importer(outputPath);
    exporter.SetQueueWrite([&importer] (const char* jsonEntry)
        {
        importer.AddToQueue(jsonEntry);
        });

    StopWatch totalTimer(true);
    std::thread consumer([&importer] { importer.CreateBim(); });
    auto future = ExportDgnDb(&exporter);
    bool stat = future.get();
    importer.SetDone();
    consumer.join();
    totalTimer.Stop();
    Utf8PrintfString message("Total teleportation|%.0f millisecs", totalTimer.GetElapsedSeconds() * 1000.0);
    BentleyApi::NativeLogging::LoggingManager::GetLogger("DgnDbToBimConverter.Performance")->info(message.c_str());

    return true;
    }

