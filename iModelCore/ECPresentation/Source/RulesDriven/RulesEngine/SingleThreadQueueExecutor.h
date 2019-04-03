/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/SingleThreadQueueExecutor.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include <Bentley/BeThread.h>
#include <folly/Executor.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                11/2017
//=======================================================================================
struct SingleThreadedQueueExecutor : folly::Executor
{
    struct ThreadRunner;

private:
    Utf8String m_name;
    ThreadRunner* m_runner;

private:
    void Init();

public:
    ECPRESENTATION_EXPORT SingleThreadedQueueExecutor(Utf8String name = "");
    ECPRESENTATION_EXPORT ~SingleThreadedQueueExecutor();
    ECPRESENTATION_EXPORT void add(folly::Func) override;
    void addWithPriority(folly::Func func, int8_t) override {add(std::move(func));}
    folly::Future<folly::Unit> Terminate();
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
