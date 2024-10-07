/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct ECDbIdSetVirtualTableTestFixture : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbIdSetVirtualTableTestFixture, IdSetModuleTest) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("vtab.ecdb"));
    m_ecdb.ExecuteSql("analyze");
    m_ecdb.SaveChanges();
    ReopenECDb();
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[1,2,3,4,5]')"));

        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 5);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[1,2,3,4,5]') where id = 1"));

        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 1);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[1,2,3,4,5]') where id > 1"));

        int i = 1;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 5);
    }
    {
        std::vector<Utf8String> hexIds = std::vector<Utf8String>{"0x1", "0x2", "0x3", "4", "5"};

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i =0;i<hexIds.size();i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindText(hexIds[i].c_str(), IECSqlBinder::MakeCopy::No));
        }
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ(BeStringUtilities::ParseHex(hexIds[i++].c_str()), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, hexIds.size());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[\"0x1\",\"0x2\",3,4,5]')"));
    
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i), stmt.GetValueInt64(0));
            i++;
        }
        ASSERT_EQ(i, 5);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[0x1,0x2,\"3\",\"4\",\"5\"]')"));

        // Should fail while converting to json array because hex values with quotes are required so should be empty table and should throw error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[1,\"2\",3, 4.0, 5.0]')"));
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i), stmt.GetValueInt64(0));
            i++;
        }
        ASSERT_EQ(i, 5);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[1,\"2\",3, 4.5, 5.6]')"));
        
        // Will not take into account 4.5 and 5.6 because they are decimal values so should be empty table and throw error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        ASSERT_EQ(ECSqlStatus::Error,stmt.BindText(1, "[1,\"2\",3, \"abc\"]", IECSqlBinder::MakeCopy::No));

        // no binding as we use array ecsql binder so need to call AddArrayElement first
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));

        // no binding so no data
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Error, elementBinder.BindText( "[1,\"2\",3, \"abc\"]", IECSqlBinder::MakeCopy::No));

        // EmptyArray is Binded so should be empty table and should throw error because the ultimate json text which will be binded will be an empty json array and that is not allowed
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 1;i<=10;i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindInt(i));
        }

        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i), stmt.GetValueInt64(0));
            i++;
        }
        ASSERT_EQ(i, 10);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 1;i<=10;i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(i));
        }

        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i), stmt.GetValueInt64(0));
            i++;
        }
        ASSERT_EQ(i, 10);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindNull());
        for(int i = 1;i<=10;i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(i));
        }

        // having null as an element so should be empty table and should throw error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        DPoint2d pArrayOfST1_P2D[] = {DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35)};
        DPoint3d pArrayOfST1_P3D[] = {DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16)};
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 0;i<=2;i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Error, elementBinder.BindPoint2d(pArrayOfST1_P2D[i]));
        }
        for(int i = 0;i<=2;i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Error, elementBinder.BindPoint3d(pArrayOfST1_P3D[i]));
        }

        // EmptyArray is Binded so should be empty table and should throw error because the ultimate json text which will be binded will be an empty json array and that is not allowed
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        DPoint2d pArrayOfST1_P2D[] = {DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35)};
        DPoint3d pArrayOfST1_P3D[] = {DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16)};
        double pST1P_ST2P_D2 = 431231.3432;
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 0;i<=2;i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Error, elementBinder["D1"].BindDouble(pST1P_ST2P_D2));
            ASSERT_EQ(ECSqlStatus::Error, elementBinder["P2D"].BindPoint2d(pArrayOfST1_P2D[i]));
        }
        for(int i = 0;i<=2;i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Error, elementBinder["D1"].BindDouble(pST1P_ST2P_D2));
            ASSERT_EQ(ECSqlStatus::Error, elementBinder["P3D"].BindPoint3d(pArrayOfST1_P3D[i]));
        }

        // EmptyArray is Binded so should be empty table and should throw error because the ultimate json text which will be binded will be an empty json array and that is not allowed
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        const std::vector<std::vector<uint8_t>> bi_array = {
            {0x48, 0x65, 0x6},
            {0x48, 0x65, 0x6},
            {0x48, 0x65, 0x6}
        };
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(auto& m : bi_array)
            ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindBlob((void const*)&m[0], (int)m.size(), IECSqlBinder::MakeCopy::No));

        // Binary is Binded so should be empty table and error should be thrown
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        const auto dt = DateTime(DateTime::Kind::Unspecified, 2017, 1, 17, 0, 0);
        const auto dtUtc = DateTime(DateTime::Kind::Utc, 2018, 2, 17, 0, 0);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 0;i<=1;i++)
        {
            ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindDateTime(dt));
            ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindDateTime(dtUtc));
        }

        // EmptyArray is Binded so should be empty table and should throw error because the ultimate json text which will be binded will be an empty json array and that is not allowed
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        auto geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 0;i<=1;i++)
        {
            ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGeometry(*geom));
        }

        // Binary is Binded because BindGeometry internally calls so should be empty table and error should be thrown
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 0;i<=1;i++)
        {
            ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindText("ABC",IECSqlBinder::MakeCopy::No));
        }

        // EmptyArray is Binded so should be empty table and should throw error because the ultimate json text which will be binded will be an empty json array and that is not allowed
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 0;i<=1;i++)
        {
            ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindText("[abc]",IECSqlBinder::MakeCopy::No));
        }

        // EmptyArray is Binded so should be empty table and should throw error because the ultimate json text which will be binded will be an empty json array and that is not allowed
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 0;i<=1;i++)
        {
            ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindText("[\"abc\"]",IECSqlBinder::MakeCopy::No));
        }

        // EmptyArray is Binded so should be empty table and should throw error because the ultimate json text which will be binded will be an empty json array and that is not allowed
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT x FROM ECVLib.IdSet('[1,2,3,4,5]'), (with cte(x) as(select ECInstanceId from meta.ECClassDef) select x from cte) where id = x group by x"));
        int i = 1;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 6);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT x FROM  (with cte(x) as(select ECInstanceId from meta.ECClassDef) select x from cte), ECVLib.IdSet('[1,2,3,4,5]') where id = x group by x"));
        int i = 1;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 6);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ECVLib.IdSet('[1,2,3,4,5]'), meta.ECClassDef where ECInstanceId = id group by ECInstanceId"));
        ASSERT_EQ(true, m_ecdb.ExplainQuery(stmt.GetNativeSql(), true).Contains("SCAN main.ec_Class"));
        ASSERT_EQ(true, m_ecdb.ExplainQuery(stmt.GetNativeSql(), true).Contains("SCAN IdSet VIRTUAL TABLE INDEX 1"));
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId FROM ECVLib.IdSet('[1,2,3,4,5]'), meta.ECClassDef where ECClassId = id group by ECClassId"));
        ASSERT_EQ(true, m_ecdb.ExplainQuery(stmt.GetNativeSql(), true).Contains("SCAN IdSet VIRTUAL TABLE INDEX 1"));
        ASSERT_EQ(true, m_ecdb.ExplainQuery(stmt.GetNativeSql(), true).Contains("SCAN main.ec_Class USING COVERING INDEX ix_ec_Class_Name"));
    }
    {
        std::vector<Utf8String> hexIds = std::vector<Utf8String>{"0x1", "0x2", "0x3", "4", "5"};

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ECVLib.IdSet(?), meta.ECClassDef where ECInstanceId = id group by ECInstanceId"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i =0;i<hexIds.size();i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindText(hexIds[i].c_str(), IECSqlBinder::MakeCopy::No));
        }
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ(BeStringUtilities::ParseHex(hexIds[i++].c_str()), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, hexIds.size());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Reset());
        ASSERT_EQ(ECSqlStatus::Success, stmt.ClearBindings());

        IECSqlBinder& arrayBinder_two = stmt.GetBinder(1);
        for(int i =0;i<hexIds.size();i++)
        {
            IECSqlBinder& elementBinder = arrayBinder_two.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindText(hexIds[i].c_str(), IECSqlBinder::MakeCopy::No));
        }
        i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ(BeStringUtilities::ParseHex(hexIds[i++].c_str()), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, hexIds.size());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT ECClassId FROM ECVLib.IdSet(?,?), meta.ECClassDef where ECClassId = id group by ECClassId"));
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[1,1,1,1]')"));
        
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ(i+1, stmt.GetValueInt64(0));
            i++;
        }
        ASSERT_EQ(i, 1);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[-1,-2,3,-4,5]')"));
        // negative values are not allowed so empty table and should throw error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        std::vector<int> ids = std::vector<int>{0,1,1,2};
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& binder = stmt.GetBinder(1);

        for(auto& i : ids)
        {
            ASSERT_EQ(ECSqlStatus::Success, binder.AddArrayElement().BindInt(i));
        }
        // 0 is not allowed so empty table and dhould throw error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[\"-1\",\"-2\",\"3\",\"-4\",\"5\"]')"));
        // negative values are not allowed so empty table and should throw error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        std::vector<Utf8String> stringIds = std::vector<Utf8String>{"-1", "-2", "3", "-4", "5"};
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& binder = stmt.GetBinder(1);

        for(auto& i : stringIds)
        {
            if(i.EqualsIAscii("3") || i.EqualsIAscii("5"))
                ASSERT_EQ(ECSqlStatus::Success, binder.AddArrayElement().BindText(i.c_str(), IECSqlBinder::MakeCopy::No));
            else
                ASSERT_EQ(ECSqlStatus::Error, binder.AddArrayElement().BindText(i.c_str(), IECSqlBinder::MakeCopy::No));
        }
        // Binding negative values will fail so for the negative values binder.AddArrayElement().BindText() will be empty array element which are not allowed so empty table and should throw error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[\"0xFFFFFFFF\",3,4,5]')"));
        int i = 3;
        while (i<=5)
        {
            ASSERT_EQ(BE_SQLITE_ROW,  stmt.Step());
            ASSERT_EQ(i, stmt.GetValueInt64(0));
            i++;
        }
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[\"0x0\",3,4,5]')"));
        // 0 values are not allowed so empty table and should throw error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        std::vector<Utf8String> hexIds = std::vector<Utf8String>{"0x1", "0x2", "0x3", "4", "5"};

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ECVLib.IdSet(?), meta.ECClassDef where ECInstanceId = id group by ECInstanceId"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i =0;i<hexIds.size();i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindText(hexIds[i].c_str(), IECSqlBinder::MakeCopy::No));
        }
        int i = 0;
        while (i<=1)
        {
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            ASSERT_EQ(BeStringUtilities::ParseHex(hexIds[i++].c_str()), stmt.GetValueInt64(0));
        }

        ASSERT_EQ(ECSqlStatus::Success, stmt.ClearBindings());

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(BeStringUtilities::ParseHex(hexIds[i++].c_str()), stmt.GetValueInt64(0));

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
}


END_ECDBUNITTESTS_NAMESPACE
