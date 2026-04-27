/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../NonPublished/ECDbPublishedTests.h"
#include "ECSqlFuzzSchema.h"
#include "ECSqlFuzzGenerator.h"
#include <atomic>
#include <chrono>
#include <thread>

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Fuzz test fixture: creates an ECDb with the rich FuzzSchema and seed data.
// All fuzz tests use a deterministic RNG for reproducibility.
// @bsiclass
//=======================================================================================
struct ECSqlFuzzTestFixture : ECDbTestFixture
    {
    protected:
        uint32_t m_seed = 0;

        void SetUp() override
            {
            ECDbTestFixture::SetUp();
            ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ecsql_fuzz.ecdb", ECSqlFuzzSchema::Get()));
            ECSqlFuzzSchema::PopulateSeedData(m_ecdb, GetHelper());

            // Use GTest random seed for reproducibility, or fallback to a fixed seed
            m_seed = (uint32_t)::testing::UnitTest::GetInstance()->random_seed();
            if (m_seed == 0)
                m_seed = 42;
            }

        //! Core fuzz harness: generate ECSQL, prepare it, optionally step.
        //! The test passes if no crash/hang occurs. InvalidECSql is an acceptable outcome.
        void RunFuzzIteration(ECSqlFuzzGenerator& gen, Utf8StringCR ecsql, bool allowStep = true)
            {
            ScopedDisableFailOnAssertion disableAssert;
            ECSqlStatement stmt;
            ECSqlStatus status = stmt.Prepare(m_ecdb, ecsql.c_str());

            if (status == ECSqlStatus::Success && allowStep)
                {
                // Watchdog: interrupt Step() if it runs >500ms (e.g. infinite recursive CTE).
                // Use a condition variable so the watchdog exits immediately when Step() finishes
                // normally — avoids paying 500ms overhead on every iteration.
                std::mutex mtx;
                std::condition_variable cv;
                bool stepDone = false;

                std::thread watchdog([&]()
                    {
                    std::unique_lock<std::mutex> lk(mtx);
                    cv.wait_for(lk, std::chrono::milliseconds(500), [&]{ return stepDone; });
                    if (!stepDone)
                        m_ecdb.Interrupt();
                    });

                stmt.Step();

                {
                std::unique_lock<std::mutex> lk(mtx);
                stepDone = true;
                cv.notify_one();
                }
                watchdog.join();
                stmt.Finalize();
                }
            // Both Success and InvalidECSql are acceptable — we just must not crash
            }

        //! Run N iterations of a generator function, logging seed + ECSQL on failure
        template<typename GenFunc>
        void RunFuzzLoop(GenFunc genFunc, int iterations)
            {
            ECSqlFuzzGenerator gen(m_seed);
            for (int i = 0; i < iterations; i++)
                {
                Utf8String ecsql = genFunc(gen);
                ASSERT_NO_FATAL_FAILURE(RunFuzzIteration(gen, ecsql))
                    << "Fuzz failure at iteration " << i
                    << " | seed=" << m_seed
                    << " | ecsql=" << ecsql.c_str();
                }
            }

        //! Run N iterations for mutated valid ECSQL
        void RunMutatedFuzzLoop(int iterations)
            {
            ECSqlFuzzGenerator gen(m_seed);
            for (int i = 0; i < iterations; i++)
                {
                // Generate valid ECSQL, then mutate it
                Utf8String valid = gen.GenAnyStatement();
                Utf8String mutated = gen.Mutate(valid);
                ASSERT_NO_FATAL_FAILURE(RunFuzzIteration(gen, mutated))
                    << "Mutation fuzz failure at iteration " << i
                    << " | seed=" << m_seed
                    << " | original=" << valid.c_str()
                    << " | mutated=" << mutated.c_str();
                }
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzSelectSimple)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectSimple(); }, 5000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzSelectComplex)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectComplex(); }, 5000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzSelectRelationships)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectRelationship(); }, 5000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzSelectSetOperations)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectSetOperation(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzSelectAggregates)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectAggregate(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzSelectFunctions)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectFunctions(); }, 3000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzSelectECSqlSpecific)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectECSqlSpecific(); }, 3000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzInsert)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenInsert(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzUpdate)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenUpdate(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzDelete)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenDelete(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzPragma)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenPragma(); }, 1000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzAliases)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectWithAliases(); }, 3000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzBracketQuoted)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectBracketQuoted(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzSubqueryPositions)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectSubqueryPositions(); }, 3000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzInsertSelect)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenInsertSelect(); }, 1000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzNaturalJoin)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectNaturalJoin(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzBitwise)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectBitwise(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzCollate)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectCollate(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzAdvancedWindowFrame)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectAdvancedWindowFrame(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzCaseAdvanced)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectCaseAdvanced(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzRecursiveCTE)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectRecursiveCTE(); }, 1000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzTypePredicate)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectTypePredicate(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzPropertyPath)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectPropertyPath(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzAdvancedAggregate)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectAdvancedAggregate(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzChainedSetOps)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectChainedSetOps(); }, 1000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzPragmaAdvanced)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenPragmaAdvanced(); }, 1000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzSchemaImport)
    {
    ECSqlFuzzGenerator gen(m_seed);
    for (int i = 0; i < 500; i++)
        {
        Utf8String schemaXml = gen.GenSchemaXml();
        ScopedDisableFailOnAssertion disableAssert;
        ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
        ECSchemaPtr schema;
        // Try to deserialize — we just must not crash
        ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *ctx);
        // If valid, try to import into a fresh ecdb
        if (schema.IsValid())
            {
            ECDb testDb;
            BeFileName tempPath;
            BeTest::GetHost().GetTempDir(tempPath);
            Utf8String dbName;
            dbName.Sprintf("fuzz_schema_%d.ecdb", i);
            tempPath.AppendToPath(WString(dbName.c_str(), true).c_str());
            if (testDb.CreateNewDb(tempPath) == BE_SQLITE_OK)
                {
                bvector<ECSchemaCP> schemas;
                schemas.push_back(schema.get());
                testDb.Schemas().ImportSchemas(schemas);
                testDb.CloseDb();
                BeFileName::BeDeleteFile(tempPath);
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzUpdateAdvanced)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenUpdateAdvanced(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzDeleteAdvanced)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenDeleteAdvanced(); }, 1000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzParameters)
    {
    // Parameters: only prepare (no step since bindings are missing)
    ECSqlFuzzGenerator gen(m_seed);
    for (int i = 0; i < 2000; i++)
        {
        Utf8String ecsql = gen.GenSelectWithParameters();
        ASSERT_NO_FATAL_FAILURE(RunFuzzIteration(gen, ecsql, false))
            << "Parameter fuzz failure at iteration " << i
            << " | seed=" << m_seed
            << " | ecsql=" << ecsql.c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzComments)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectWithComments(); }, 1000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzLiteralEdgeCases)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectLiteralEdgeCases(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzNavigationAdvanced)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectNavigationAdvanced(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzValues)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectValues(); }, 1000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzOrderByAdvanced)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenSelectOrderByAdvanced(); }, 2000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzMutatedValid)
    {
    RunMutatedFuzzLoop(10000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzBoundaryInputs)
    {
    ECSqlFuzzGenerator gen(m_seed);
    for (int i = 0; i < 1000; i++)
        {
        Utf8String ecsql = gen.GenBoundaryInput();
        ASSERT_NO_FATAL_FAILURE(RunFuzzIteration(gen, ecsql, false))
            << "Boundary fuzz failure at iteration " << i
            << " | seed=" << m_seed
            << " | ecsql=" << ecsql.c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzDeepNesting)
    {
    ECSqlFuzzGenerator gen(m_seed);
    for (int i = 0; i < 500; i++)
        {
        Utf8String ecsql = gen.GenDeepNesting();
        ASSERT_NO_FATAL_FAILURE(RunFuzzIteration(gen, ecsql, false))
            << "Deep nesting fuzz failure at iteration " << i
            << " | seed=" << m_seed
            << " | ecsql=" << ecsql.c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlFuzzTestFixture, FuzzCrossStatementType)
    {
    RunFuzzLoop([](ECSqlFuzzGenerator& g) { return g.GenAnyStatement(); }, 5000);
    }

END_ECDBUNITTESTS_NAMESPACE
