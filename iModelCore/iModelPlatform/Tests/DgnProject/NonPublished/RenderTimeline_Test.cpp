/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"


//=======================================================================================
// @bsistruct
//=======================================================================================
struct RenderTimelineTests : public DgnDbTestFixture
{
    Json::Value m_script;

    void SetUp() final
        {
        SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true);
        m_script = MakeScript(1);
        }

    static Json::Value MakeScript(int batchId)
        {
        Json::Value model;
        model["modelId"] = "0x123";

        Json::Value elem1;
        elem1["elementIds"] = "+2+3*5+1";
        elem1["batchId"] = batchId;
        model["elementTimelines"].append(elem1);

        Json::Value elem2;
        elem2["elementIds"].append("0x456");
        elem2["elementIds"].append("0xfed");
        elem2["batchId"] = 2;
        model["elementTimelines"].append(elem2);

        Json::Value script;
        script.append(model);
        return script;
        }

    DgnDbR CloseAndReopenDb()
        {
        auto filename = BeFileName(m_db->GetDbFileName());
        m_db->CloseDb();
        OpenDb(m_db, filename, BeSQLite::Db::OpenMode::ReadWrite);
        return *m_db;
        }

    RenderTimelineCPtr InsertTimeline() const
        {
        auto& dict = m_db->GetDictionaryModel();
        DgnClassId classId(m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_RenderTimeline));
        EXPECT_TRUE(classId.IsValid());

        RenderTimelinePtr timeline(new RenderTimeline(RenderTimeline::CreateParams(*m_db, dict.GetModelId(), classId)));
        EXPECT_TRUE(timeline.IsValid());

        timeline->SetScript(m_script);
        auto persistent = timeline->Insert();
        EXPECT_TRUE(persistent.IsValid());

        m_db->SaveChanges();
        return static_cast<RenderTimelineCP>(persistent.get());
        }

    DgnElementId InsertDisplayStyle(Json::Value script)
        {
        auto style = CreateDisplayStyle();
        if (!script.isNull())
            style->SetStyle("scheduleScript", script);

        return WriteDisplayStyle(*style);
        }

    DgnElementId InsertDisplayStyle(DgnElementId timelineId)
        {
        auto style = CreateDisplayStyle();
        style->SetRenderTimelineId(timelineId);
        return WriteDisplayStyle(*style);
        }

    DisplayStyle3dPtr CreateDisplayStyle()
        {
        return new DisplayStyle3d(m_db->GetDictionaryModel(), "");
        }

    DgnElementId WriteDisplayStyle(DisplayStyle3dR style)
        {
        auto persistent = style.Insert();
        EXPECT_TRUE(persistent.IsValid());
        m_db->SaveChanges();
        return persistent->GetElementId();
        }

    void ExpectScript(DgnElementId id, Json::Value expected) const
        {
        auto host = RenderTimeline::GetScriptHost(id, *m_db);
        EXPECT_EQ(host.GetScript(), expected);
        }

    void ExpectElementIds(RenderTimelineCR timeline, Json::Value opts, bool expectPresent)
        {
        Json::Value json;
        timeline.ToJson(json, opts);

        auto scriptStr = json["script"];
        EXPECT_TRUE(scriptStr.isString());

        auto script = Json::Reader::DoParse(scriptStr.asString());
        EXPECT_FALSE(script.isNull());

        if (expectPresent)
            {
            EXPECT_EQ(script, m_script);
            }
        else
            {
            auto model = script[0];
            EXPECT_EQ(model["modelId"].asString(), "0x123");

            auto elems = model["elementTimelines"];
            EXPECT_EQ(elems.size(), 2);
            EXPECT_EQ(elems[0]["batchId"].asInt(), 1);
            EXPECT_EQ(elems[0]["elementIds"].asString(), "");
            EXPECT_EQ(elems[1]["batchId"].asInt(), 2);
            EXPECT_EQ(elems[1]["elementIds"].asString(), "");
            }
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RenderTimelineTests, Create)
    {
    DgnElementId elemId;
        {
        auto timeline = InsertTimeline();
        elemId = timeline->GetElementId();
        m_db->SaveChanges();

        auto update = timeline->MakeCopy<RenderTimeline>();
        update->SetDescription("my timeline");
        timeline = static_cast<RenderTimelineCP>(update->UpdateAndGet().get());
        ASSERT_TRUE(timeline.IsValid());
        m_db->SaveChanges();
        }

    auto& db = CloseAndReopenDb();
    auto timeline = db.Elements().Get<RenderTimeline>(elemId);
    ASSERT_TRUE(timeline.IsValid());
    EXPECT_TRUE(timeline->GetDescription().Equals("my timeline"));
    Json::Value script;
    timeline->GetScript(script);
    EXPECT_FALSE(script.isNull());
    EXPECT_EQ(script, m_script);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RenderTimelineTests, ObtainScriptFromSourceId)
    {
    auto timeline = InsertTimeline();
    ExpectScript(timeline->GetElementId(), m_script);

    auto noScript = InsertDisplayStyle(DgnElementId());
    ExpectScript(noScript, Json::nullValue);

    auto embedded = InsertDisplayStyle(m_script);
    ExpectScript(embedded, m_script);

    // We expect the caller to supply the Id of the element that hosts the script - we do not check for pointers to other elements.
    auto separate = InsertDisplayStyle(timeline->GetElementId());
    ExpectScript(separate, Json::nullValue);

    ExpectScript(DgnElementId((uint64_t)0xbaadf00d), Json::nullValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RenderTimelineTests, OmitElementIds)
    {
    auto timeline = InsertTimeline();
    ExpectElementIds(*timeline, Json::nullValue, true);

    Json::Value opts;
    opts["renderTimeline"] = Json::nullValue;
    ExpectElementIds(*timeline, opts, true);

    opts["renderTimeline"]["omitScriptElementIds"] = false;
    ExpectElementIds(*timeline, opts, true);

    opts["renderTimeline"]["omitScriptElementIds"] = true;
    ExpectElementIds(*timeline, opts, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RenderTimelineTests, CachedScript)
    {
    struct JsConst : BeJsConst { JsValueRef const* GetValue() const { return m_val.get(); } };
    auto isSameObject = [](BeJsConst a, BeJsConst b) { return static_cast<JsConst&>(a).GetValue() == static_cast<JsConst&>(b).GetValue(); };

    auto timeline = InsertTimeline();
    auto script1 = timeline->GetScript();
    EXPECT_FALSE(script1.isNull());
    auto script2 = timeline->GetScript();
    EXPECT_TRUE(isSameObject(script1, script2));

    auto edit = timeline->MakeCopy<RenderTimeline>();
    Json::Value newScript = MakeScript(42);
    edit->SetScript(newScript);
    auto script3 = edit->GetScript();
    EXPECT_FALSE(isSameObject(script3, script2));
    EXPECT_EQ(BeJsConst(newScript), script3);
    EXPECT_FALSE(isSameObject(script3, newScript));
    EXPECT_TRUE(isSameObject(script3, edit->GetScript()));

    EXPECT_EQ(DgnDbStatus::Success, edit->Update());
    ExpectScript(timeline->GetElementId(), newScript);
    }

