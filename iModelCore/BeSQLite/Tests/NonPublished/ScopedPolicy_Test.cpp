/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BeSQLiteNonPublishedTests.h"
#include <BeSQLite/ScopedPolicy.h>
#include <BeSQLite/ChangeSet.h>
#include <cstring>
#include <stdexcept>

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ScopedPolicyTest : public ::testing::Test
{
    Db m_db;

    void SetUp() override
        {
        BeFileName tempDir;
        BeTest::GetHost().GetTempDir(tempDir);
        BeSQLiteLib::Initialize(tempDir);

        BeFileName fileName;
        BeTest::GetHost().GetOutputRoot(fileName);
        fileName.AppendToPath(L"ScopedPolicy.db");
        if (BeFileName::DoesPathExist(fileName))
            BeFileName::BeDeleteFile(fileName);

        ASSERT_EQ(BE_SQLITE_OK, m_db.CreateNewDb(fileName.GetNameUtf8().c_str()));
        ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TABLE users(id INTEGER PRIMARY KEY, name TEXT, secret TEXT, data BLOB)"));
        ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO users VALUES(1,'alice','s1',x'0102'),(2,'bob','s2',x'0304')"));
        ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TABLE audit(id INTEGER PRIMARY KEY, msg TEXT)"));
        ASSERT_EQ(BE_SQLITE_OK, m_db.SaveChanges());
        }

    void TearDown() override
        {
        if (m_db.IsDbOpen())
            {
            m_db.AbandonChanges();
            m_db.CloseDb();
            }
        }

    DbResult Exec(Utf8CP sql) {return m_db.TryExecuteSql(sql);}

    int Count(Utf8CP table)
        {
        Statement stmt;
        Utf8PrintfString sql("SELECT count(*) FROM %s", table);
        EXPECT_EQ(BE_SQLITE_OK, stmt.TryPrepare(m_db, sql.c_str()));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        return stmt.GetValueInt(0);
        }

    DbResult PrepareResult(Utf8CP sql)
        {
        Statement stmt;
        return stmt.TryPrepare(m_db, sql);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, DenyIsRemovedByDestructor)
    {
        {
        ScopedPolicy policy(m_db);
        policy.DenyDelete().DenyInsert();
        EXPECT_TRUE(policy.IsActive());
        EXPECT_EQ(BE_SQLITE_OK, policy.GetStatus());
        EXPECT_EQ(BE_SQLITE_AUTH, Exec("DELETE FROM users"));
        EXPECT_EQ(BE_SQLITE_AUTH, Exec("INSERT INTO audit(msg) VALUES('x')"));
        EXPECT_EQ(BE_SQLITE_OK, Exec("UPDATE users SET name='carol' WHERE id=2"));
        ASSERT_NE(nullptr, policy.GetLastDenial());
        EXPECT_EQ(AuthorizerAction::Insert, policy.GetLastDenial()->m_action);
        EXPECT_STREQ("audit", policy.GetLastDenial()->m_arg1.value().c_str());
        }
    EXPECT_EQ(2, Count("users"));
    EXPECT_EQ(BE_SQLITE_OK, Exec("DELETE FROM audit"));
    EXPECT_EQ(BE_SQLITE_OK, Exec("INSERT INTO audit(msg) VALUES('x')"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, ConstraintAddedLaterTakesEffect)
    {
    ScopedPolicy policy(m_db);
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.TryPrepare(m_db, "SELECT name FROM users"));
    policy.DenyTable("USERS");
    EXPECT_EQ(BE_SQLITE_AUTH, stmt.Step()) << "prepared statement must be re-authorized";
    EXPECT_EQ(BE_SQLITE_AUTH, PrepareResult("SELECT * FROM main.users"));
    EXPECT_EQ(BE_SQLITE_OK, PrepareResult("SELECT * FROM audit"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, NestedScopeReplacesAndRestores)
    {
    ScopedPolicy outer(m_db);
    outer.DenyDelete();
    EXPECT_EQ(BE_SQLITE_AUTH, Exec("DELETE FROM audit"));
        {
        ScopedPolicy inner(m_db);
        inner.DenyUpdate();
        EXPECT_FALSE(outer.IsActive());
        EXPECT_TRUE(inner.IsActive());
        EXPECT_EQ(BE_SQLITE_OK, Exec("DELETE FROM audit"));
        EXPECT_EQ(BE_SQLITE_AUTH, Exec("UPDATE users SET name='x'"));
        }
    EXPECT_TRUE(outer.IsActive());
    EXPECT_EQ(BE_SQLITE_AUTH, Exec("DELETE FROM audit"));
    EXPECT_EQ(BE_SQLITE_OK, Exec("UPDATE users SET name='x' WHERE id=1"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, CustomCallbackSeesRequest)
    {
    Utf8String table, column, schema;
    ScopedPolicy policy(m_db, [&](AuthorizerRequest const& r)
        {
        if (r.m_action == AuthorizerAction::Read && r.m_arg2 && 0 == strcmp(r.m_arg2, "secret"))
            {
            table = r.m_arg1;
            column = r.m_arg2;
            schema = r.m_database ? r.m_database : "";
            return AuthorizerResult::Deny;
            }
        return AuthorizerResult::Allow;
        });
    EXPECT_EQ(BE_SQLITE_AUTH, PrepareResult("SELECT secret FROM users"));
    EXPECT_STREQ("users", table.c_str());
    EXPECT_STREQ("secret", column.c_str());
    EXPECT_STREQ("main", schema.c_str());
    EXPECT_EQ(BE_SQLITE_OK, PrepareResult("SELECT name FROM users"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, ReadOnly)
    {
    ScopedPolicy policy(m_db);
    policy.ReadOnly();
    EXPECT_EQ(2, Count("users"));
    EXPECT_EQ(BE_SQLITE_OK, PrepareResult("PRAGMA table_info(users)"));
    EXPECT_EQ(BE_SQLITE_OK, PrepareResult("SELECT * FROM pragma_table_info('users')"));
    EXPECT_EQ(BE_SQLITE_AUTH, Exec("INSERT INTO audit(msg) VALUES('x')"));
    EXPECT_EQ(BE_SQLITE_AUTH, Exec("UPDATE users SET name='x'"));
    EXPECT_EQ(BE_SQLITE_AUTH, Exec("DELETE FROM users"));
    EXPECT_EQ(BE_SQLITE_AUTH, Exec("CREATE TABLE t2(a)"));
    EXPECT_EQ(BE_SQLITE_AUTH, Exec("PRAGMA user_version=5"));
    EXPECT_EQ(BE_SQLITE_AUTH, PrepareResult("PRAGMA database_list"));
    EXPECT_EQ(BE_SQLITE_AUTH, Exec("ATTACH ':memory:' AS aux"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, IgnoreRedactsOnlyUnderReadOnly)
    {
    auto redactSecret = [](AuthorizerRequest const& r)
        {
        bool hit = r.m_arg2 && 0 == strcmp(r.m_arg2, "secret");
        return hit ? AuthorizerResult::Ignore : AuthorizerResult::Allow;
        };

        {
        ScopedPolicy policy(m_db, redactSecret);
        // Without ReadOnly(), Ignore would silently corrupt writes; it becomes Deny.
        EXPECT_EQ(BE_SQLITE_AUTH, PrepareResult("SELECT secret FROM users"));
        EXPECT_EQ(BE_SQLITE_AUTH, Exec("UPDATE users SET secret='zz'"));
        }

    ScopedPolicy policy(m_db, redactSecret);
    policy.ReadOnly();
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.TryPrepare(m_db, "SELECT name, secret FROM users WHERE id=1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("alice", stmt.GetValueText(0));
    EXPECT_TRUE(stmt.IsColumnNull(1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, AllowOnlyIntersects)
    {
    ScopedPolicy policy(m_db);
    policy.AllowOnly({AuthorizerAction::Select, AuthorizerAction::Read, AuthorizerAction::Insert, AuthorizerAction::Function});
    EXPECT_EQ(2, Count("users"));
    EXPECT_EQ(BE_SQLITE_OK, Exec("INSERT INTO audit(msg) VALUES('x')"));
    EXPECT_EQ(BE_SQLITE_AUTH, Exec("DELETE FROM audit"));
    policy.AllowOnly({AuthorizerAction::Select, AuthorizerAction::Read, AuthorizerAction::Function});
    EXPECT_EQ(BE_SQLITE_AUTH, Exec("INSERT INTO audit(msg) VALUES('y')"));
    EXPECT_EQ(2, Count("users"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, DenyTableMatching)
    {
    ScopedPolicy policy(m_db);
    policy.DenyTable("Users", "main");
    EXPECT_EQ(BE_SQLITE_AUTH, PrepareResult("SELECT name FROM users"));
    EXPECT_EQ(BE_SQLITE_AUTH, PrepareResult("SELECT count(*) FROM users"));
    EXPECT_EQ(BE_SQLITE_AUTH, Exec("CREATE INDEX idx_users ON users(name)"));
    EXPECT_EQ(BE_SQLITE_OK, PrepareResult("SELECT msg FROM audit"));

    ScopedPolicy schemaPolicy(m_db);
    schemaPolicy.DenyTable("sqlite_schema");
    EXPECT_EQ(BE_SQLITE_AUTH, PrepareResult("SELECT name FROM sqlite_master"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, DenyTableThroughView)
    {
    ASSERT_EQ(BE_SQLITE_OK, Exec("CREATE TEMP VIEW v_users AS SELECT name FROM users"));
    ScopedPolicy policy(m_db);
    policy.DenyTable("users");
    EXPECT_EQ(BE_SQLITE_AUTH, PrepareResult("SELECT * FROM v_users"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, DenyColumn)
    {
    ScopedPolicy policy(m_db);
    policy.DenyColumn("users", "secret").DenyColumn("users", "id");
    EXPECT_EQ(BE_SQLITE_AUTH, PrepareResult("SELECT secret FROM users"));
    EXPECT_EQ(BE_SQLITE_AUTH, Exec("UPDATE users SET secret='x'"));
    EXPECT_EQ(BE_SQLITE_AUTH, PrepareResult("SELECT rowid FROM users")) << "rowid is reported as the INTEGER PRIMARY KEY";
    EXPECT_EQ(BE_SQLITE_OK, PrepareResult("SELECT name FROM users"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, DenyFunctionAndPragma)
    {
    ScopedPolicy policy(m_db);
    policy.DenyFunction("UPPER").DenyPragma("user_version");
    EXPECT_NE(BE_SQLITE_OK, PrepareResult("SELECT upper(name) FROM users"));
    ASSERT_NE(nullptr, policy.GetLastDenial());
    EXPECT_EQ(AuthorizerAction::Function, policy.GetLastDenial()->m_action);
    EXPECT_STREQ("upper", policy.GetLastDenial()->m_arg2.value().c_str());
    EXPECT_EQ(BE_SQLITE_OK, PrepareResult("SELECT lower(name) FROM users"));
    EXPECT_EQ(BE_SQLITE_AUTH, PrepareResult("PRAGMA user_version"));
    EXPECT_EQ(BE_SQLITE_OK, PrepareResult("PRAGMA page_size"));
    policy.ClearLastDenial();
    EXPECT_EQ(nullptr, policy.GetLastDenial());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, BlobIO)
    {
    BlobIO blob;
        {
        ScopedPolicy policy(m_db);
        policy.ReadOnly();
        EXPECT_EQ(BE_SQLITE_OK, blob.Open(m_db, "users", "data", 1, false));
        blob.Close();
        EXPECT_EQ(BE_SQLITE_AUTH, blob.Open(m_db, "users", "data", 1, true));
        }
        {
        ScopedPolicy policy(m_db);
        policy.DenyColumn("users", "data");
        EXPECT_EQ(BE_SQLITE_AUTH, blob.Open(m_db, "users", "data", 1, false));
        }
        {
        ScopedPolicy policy(m_db, [](AuthorizerRequest const&) {return AuthorizerResult::Ignore;});
        policy.ReadOnly();
        EXPECT_EQ(BE_SQLITE_AUTH, blob.Open(m_db, "users", "data", 1, false)) << "Ignore is Deny for blobs";
        }
    EXPECT_EQ(BE_SQLITE_OK, blob.Open(m_db, "users", "data", 1, true));
    blob.Close();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, ChangesetForbidsRedaction)
    {
    struct Tracker : ChangeTracker
        {
        Tracker(DbR db) {SetDb(&db);}
        OnCommitStatus _OnCommit(bool, Utf8CP) override {return OnCommitStatus::Commit;}
        };
    Tracker tracker(m_db);
    tracker.EnableTracking(true);
    ASSERT_EQ(BE_SQLITE_OK, Exec("UPDATE users SET name='zed', secret='s9' WHERE id=1"));

    ScopedPolicy policy(m_db, [](AuthorizerRequest const& r)
        {
        bool hit = r.m_arg2 && 0 == strcmp(r.m_arg2, "secret");
        return hit ? AuthorizerResult::Ignore : AuthorizerResult::Allow;
        });
    policy.ReadOnly();
    ChangeSet changeset;
    EXPECT_NE(BE_SQLITE_OK, changeset.FromChangeTrack(tracker)) << "redaction must not leak NULLs into a changeset";
    tracker.EndTracking();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, HardeningIsRestored)
    {
    ASSERT_EQ(BE_SQLITE_OK, Exec("CREATE TRIGGER trg AFTER INSERT ON users BEGIN INSERT INTO audit(msg) VALUES('trg'); END"));
        {
        ScopedPolicy policy(m_db);
        policy.TriggersEnabled(false).MaxAttached(0).Defensive();
        EXPECT_EQ(BE_SQLITE_OK, policy.GetStatus());
        EXPECT_EQ(BE_SQLITE_OK, Exec("INSERT INTO users(name) VALUES('t1')"));
        EXPECT_EQ(0, Count("audit"));
        EXPECT_NE(BE_SQLITE_OK, Exec("ATTACH ':memory:' AS aux"));
        }
    EXPECT_EQ(BE_SQLITE_OK, Exec("INSERT INTO users(name) VALUES('t2')"));
    EXPECT_EQ(1, Count("audit"));
    EXPECT_EQ(BE_SQLITE_OK, Exec("ATTACH ':memory:' AS aux"));
    EXPECT_EQ(BE_SQLITE_OK, Exec("DETACH aux"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, HardeningOnInactiveGuardFailsClosed)
    {
    ScopedPolicy outer(m_db);
        {
        ScopedPolicy inner(m_db);
        outer.Defensive();
        }
    EXPECT_EQ(BE_SQLITE_MISUSE, outer.GetStatus());
    EXPECT_TRUE(outer.IsFailClosed());
    EXPECT_EQ(BE_SQLITE_AUTH, PrepareResult("SELECT name FROM users"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, ErrorsFailClosed)
    {
        {
        ScopedPolicy policy(m_db, AuthorizerCallback());
        EXPECT_EQ(BE_SQLITE_MISUSE, policy.GetStatus());
        EXPECT_TRUE(policy.IsFailClosed());
        EXPECT_EQ(BE_SQLITE_AUTH, PrepareResult("SELECT name FROM users"));
        }
        {
        ScopedPolicy policy(m_db, [](AuthorizerRequest const& r) -> AuthorizerResult
            {
            if (r.m_action == AuthorizerAction::Delete)
                throw std::runtime_error("boom");
            return AuthorizerResult::Allow;
            });
        EXPECT_EQ(BE_SQLITE_AUTH, Exec("DELETE FROM audit"));
        ASSERT_NE(nullptr, policy.GetLastDenial());
        EXPECT_TRUE(policy.GetLastDenial()->m_callbackFailed);
        }
    EXPECT_EQ(2, Count("users"));

    Db closed;
    ScopedPolicy inert(closed);
    EXPECT_EQ(BE_SQLITE_ERROR_NOTOPEN, inert.GetStatus());
    EXPECT_FALSE(inert.IsActive());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScopedPolicyTest, CloseDbDetachesGuards)
    {
    BeFileName fileName(m_db.GetDbFileName(), true);
    ScopedPolicy outer(m_db);
    outer.DenyDelete();
    ScopedPolicy inner(m_db);
    inner.ReadOnly();
    m_db.CloseDb();
    EXPECT_FALSE(outer.IsActive());
    EXPECT_FALSE(inner.IsActive());

    ASSERT_EQ(BE_SQLITE_OK, m_db.OpenBeSQLiteDb(fileName, Db::OpenParams(Db::OpenMode::ReadWrite)));
    EXPECT_EQ(BE_SQLITE_OK, Exec("DELETE FROM audit")) << "a reopened Db has no policy";
    }
