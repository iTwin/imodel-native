/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include "json/BeJsValue.h"

struct ExternalSourceTest : public GenericDgnModel2dTestFixture
{
};

static ExternalSourceCPtr createAndInsertExternalSource(RepositoryLinkCR link, ExternalSource::SourceProperties const& sprops, ExternalSource::ConnectorProperties const& cprops)
    {
    DgnCode code = ExternalSource::CreateCode(link, sprops.name.c_str()); // in this test, we use the rlink as the scope and the name as the value
    ExternalSource::Properties props(sprops, cprops, code);
    DgnDbStatus status;
    auto xsenp = ExternalSource::Create(&status, props, link);
    EXPECT_TRUE(xsenp.IsValid());
    return xsenp.IsValid()? xsenp->InsertT<ExternalSource>(): nullptr;
    }

static ExternalSourceGroupCPtr createAndInsertExternalSourceGroup(DgnDbR db, Utf8StringCR name, BeJsConst jsonProperties)
    {
    DgnCode code = ExternalSourceGroup::CreateCode(*db.Elements().GetRootSubject(), name.c_str()); // in this test, we use the root subject as the scope and the name as the value
    DgnDbStatus status;
    auto xsenp = ExternalSourceGroup::Create(&status, db, {{}, {}, DgnCode()}, nullptr, nullptr, jsonProperties);
    EXPECT_TRUE(xsenp.IsValid());
    return xsenp.IsValid()? xsenp->InsertT<ExternalSourceGroup>(): nullptr;
    }

static RepositoryLinkCPtr createAndInsertRepositoryLink(DgnDbR db, Utf8StringCR url, Utf8StringCR label)
    {
    auto model = db.GetRepositoryModel();
    RepositoryLinkPtr link = RepositoryLink::Create(*model, url.c_str(), label.c_str(), "");
    EXPECT_TRUE(link.IsValid());
    return link.IsValid()? link->InsertT<RepositoryLink>(): nullptr;
    }

static ExternalSourceAttachment::PlacementProperties makePProps(TransformCR t)
    {
    ExternalSourceAttachment::PlacementProperties pprops;
    pprops.SetTransform(t);
    return pprops;
    }

static ExternalSourceAttachment::Properties makeProps(Utf8String refName, TransformCR t, ExternalSourceAttachment::Role role = ExternalSourceAttachment::Role::SpecifyPart)
    {
    ExternalSourceAttachment::Properties props = {refName, role, makePProps(t)};
    return props;
    }

static DPoint3d getTranslation(TransformCR t)
    {
    DPoint3d pt;
    t.GetTranslation(pt);
    return pt;
    }

static DVec3d getScale(TransformCR t)
    {
    DVec3d s;
    double v{};
    if (t.IsRigidScale(v))
        {
        s.Init(v,v,v);
        }
    else
        {
        RotMatrix r;
        r.NormalizeColumnsOf(t.Matrix(), s);
        }
    return s;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void checkYpr(YawPitchRollAngles const& ypr, double y, double p, double r)
    {
    ASSERT_TRUE(ypr.GetYaw().AlmostEqual(AngleInDegrees::FromDegrees(y)));
    ASSERT_TRUE(ypr.GetPitch().AlmostEqual(AngleInDegrees::FromDegrees(p)));
    ASSERT_TRUE(ypr.GetRoll().AlmostEqual(AngleInDegrees::FromDegrees(r)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ExternalSourceTest, ExternalSourceCRUD)
    {
    DgnDbR db = *GetDgnDb(L"ExternalSourceCRUD");

    /*

        Suppose there are 2 repos.
        In repo #1, there are two sources, identified as 0 and 1.
        In repo #2, there is one source, identified as 0.
        Source 0 in repo 1 attaches both source 1 in repo 1 and source 0 in repo 2.

        xse10               -> repo1, "0"
            |
            +-@-> xse11     -> repo1, "1"
            |
            +-@-> xse20     -> repo2, "0"

    */

    DgnElementId repoId1, xse10Id;
    if (true)
        {
        auto repo1 = createAndInsertRepositoryLink(db, "http://www.bentley.com/repo1", "repo1");
        ASSERT_TRUE(repo1.IsValid());
        repoId1 = repo1->GetElementId();

        DgnDbStatus status;

        auto xse10 = createAndInsertExternalSource(*repo1, {"0","1,0"}, {"foo","v1"});
        ASSERT_TRUE(xse10.IsValid());
        xse10Id = xse10->GetElementId();

        { // quick test of cloning an ExternalSource
        DgnImportContext context(db,db);
        auto ed = xse10->CloneForImport(nullptr, db.GetDictionaryModel(), context);
        ASSERT_TRUE(ed.IsValid());
        DgnElement::Aspect::ImportAspects(*ed, *xse10, context); // copy (and remap) ExternalSourceAspects and all other aspects, too!
        auto xse10CC = ed->InsertT<ExternalSource>();
        ASSERT_TRUE(xse10CC.IsValid());

        auto sprops = xse10->GetSourceProperties();
        auto ccsprops = xse10CC->GetSourceProperties();
        auto cprops = xse10->GetConnectorProperties();
        auto cccprops = xse10CC->GetConnectorProperties();
        EXPECT_STREQ(ccsprops.name.c_str(), sprops.name.c_str());
        EXPECT_STREQ(ccsprops.identifier.c_str(), sprops.identifier.c_str());
        EXPECT_STREQ(cccprops.name.c_str(), cprops.name.c_str());
        EXPECT_STREQ(cccprops.version.c_str(), cprops.version.c_str());
        // NB: Code is copied only when copying between different DBs, so we can't check that.

        EXPECT_EQ(xse10CC->Delete(), DgnDbStatus::Success);
        }

        ASSERT_FALSE(createAndInsertExternalSource(*repo1, {"0","1,0"}, {"foo","v1"}).IsValid()) << " should fail to create a dup";

        auto xseFound = ExternalSource::FindBySourceIdentifier(*repo1, "0");
        ASSERT_TRUE(xseFound.IsValid());
        ASSERT_TRUE(xseFound->GetElementId() == xse10->GetElementId());

        BeTest::SetFailOnAssert(false);
        auto xseNoCode = ExternalSource::Create(&status, {{"0",""}, {"foo","v1"}, DgnCode()}, *repo1);  // try creating one with an empty name - that should be illegal
        ASSERT_TRUE(xseNoCode.IsValid()) << " code is NOT required";

        auto badcode = Subject::CreateCode(*db.Elements().GetRootSubject(), "foo"); // try creating one with the wrong codespec - that should be rejected
        BeTest::SetFailOnAssert(false);
        EXPECT_FALSE(ExternalSource::Create(&status, {{"0","1,0"}, {"foo","v1"}, badcode}, *repo1).IsValid());
        BeTest::SetFailOnAssert(true);
        EXPECT_EQ(status, DgnDbStatus::InvalidCode);
        }

    // Flush cache and re-check element
    db.Elements().ClearCache();
        {
        auto repo1 = db.Elements().Get<RepositoryLink>(repoId1);
        ASSERT_TRUE(repo1.IsValid());
        auto xse10 = db.Elements().Get<ExternalSource>(xse10Id);
        ASSERT_TRUE(xse10.IsValid());

        ASSERT_STREQ(xse10->GetUserLabel(), "1,0");

        auto link1 = xse10->GetRepository();
        ASSERT_TRUE(link1.IsValid());
        ASSERT_TRUE(link1->GetElementId() == repoId1);

        auto xses = ExternalSource::FindByRepository(*repo1);
        ASSERT_EQ(xses.size(), 1);
        ASSERT_TRUE(xses.front()->GetElementId() == xse10Id);

        auto sprops = xse10->GetSourceProperties();
        EXPECT_STREQ(sprops.name.c_str(), "1,0");
        EXPECT_STREQ(sprops.identifier.c_str(), "0");

        auto cprops = xse10->GetConnectorProperties();
        EXPECT_STREQ(cprops.name.c_str(), "foo");
        EXPECT_STREQ(cprops.version.c_str(), "v1");

        auto ed = xse10->MakeCopy<ExternalSource>();
        ed->SetConnectorProperties({"bar", "v2"});
        EXPECT_EQ(DgnDbStatus::Success, ed->Update());
        }

    db.Elements().ClearCache();
        {
        auto repo1 = db.Elements().Get<RepositoryLink>(repoId1);
        EXPECT_TRUE(repo1.IsValid());
        auto xseFound = ExternalSource::FindBySourceIdentifier(*repo1, "0");
        ASSERT_TRUE(xseFound.IsValid());

        auto cprops = xseFound->GetConnectorProperties();
        ASSERT_STREQ(cprops.name.c_str(), "bar");
        ASSERT_STREQ(cprops.version.c_str(), "v2");
        }

    Transform t1 = Transform::From(DPoint3d::From(1,1,1));
    DgnElementId a10_20_id, xse20Id, xse11Id;
        {
        auto repo1 = db.Elements().Get<RepositoryLink>(repoId1);
        auto xse10 = db.Elements().Get<ExternalSource>(xse10Id);

        auto xse11 = createAndInsertExternalSource(*repo1, {"1","1,1"}, {"foo","v1"}); // a second ExternalSource in the same repo
        ASSERT_TRUE(xse11.IsValid());
        EXPECT_EQ(xse11->GetRepository(), xse10->GetRepository());
        xse11Id = xse11->GetElementId();

        auto repo2 = createAndInsertRepositoryLink(db, "http://foo.com", "rlinklabel2"); // second repo
        ASSERT_TRUE(repo2.IsValid());
        auto xse20 = createAndInsertExternalSource(*repo2, {"0","2,0"}, {"foo","v1"}); // an ExternalSource in the second repo
        ASSERT_TRUE(xse20.IsValid());
        EXPECT_NE(xse20->GetRepository(), xse10->GetRepository());
        xse20Id = xse20->GetElementId();

        auto attachmentnp = ExternalSourceAttachment::Create(*xse10, makeProps("10->20", t1), *xse20);
        ASSERT_TRUE(attachmentnp.IsValid());
        auto a10_20 = attachmentnp->InsertT<ExternalSourceAttachment>();
        ASSERT_TRUE(a10_20.IsValid());
        ASSERT_EQ(a10_20->GetParentId(), xse10Id);
        ASSERT_EQ(a10_20->GetAttachedExternalSource(), xse20);
        a10_20_id = a10_20->GetElementId();
        }

    Transform t2 = Transform::From(DPoint3d::From(2,1,1));
    db.Elements().ClearCache();
        {
        auto a10_20 = db.Elements().Get<ExternalSourceAttachment>(a10_20_id);
        ASSERT_TRUE(a10_20.IsValid());

        EXPECT_EQ(a10_20->GetRole(), ExternalSourceAttachment::Role::SpecifyPart);

        auto pprops = a10_20->GetPlacementProperties();
        checkYpr(pprops.rotation, 0.0, 0.0, 0.0);
        EXPECT_TRUE(pprops.translation.IsEqual(getTranslation(t1)));
        EXPECT_TRUE(pprops.scale.IsEqual(getScale(t1)));
        EXPECT_TRUE(pprops.transform.IsEqual(t1));

        auto ed = a10_20->MakeCopy<ExternalSourceAttachment>();
        ed->SetRole(ExternalSourceAttachment::Role::ShowContext);
        pprops.SetTransform(t2);
        ed->SetPlacementProperties(pprops);
        EXPECT_EQ(DgnDbStatus::Success, ed->Update());
        }

    DgnElementId a10_11_id;
    db.Elements().ClearCache();
        {
        auto xse10 = db.Elements().Get<ExternalSource>(xse10Id);
        ASSERT_TRUE(xse10.IsValid());
        auto xse20 = db.Elements().Get<ExternalSource>(xse20Id);
        ASSERT_TRUE(xse20.IsValid());

        auto a10_20 = ExternalSourceAttachment::Find(*xse10, t2, *xse20, "10->20");
        ASSERT_TRUE(a10_20.IsValid());
        EXPECT_TRUE(a10_20->GetAttachedExternalSourceId() == xse20Id);
        EXPECT_TRUE(a10_20->GetAttachedExternalSource().IsValid());
        EXPECT_TRUE(a10_20->GetAttachedExternalSource()->GetElementId() == xse20Id);

        EXPECT_FALSE(ExternalSourceAttachment::Find(*xse10, t2, *xse10, "10->20").IsValid()) << " should not be able to find the a10_20 if we specified the wrong XSE as the target";

        auto pprops = a10_20->GetPlacementProperties();
        checkYpr(pprops.rotation, 0.0, 0.0, 0.0);
        EXPECT_TRUE(pprops.translation.IsEqual(getTranslation(t2)));
        EXPECT_TRUE(pprops.scale.IsEqual(getScale(t2)));
        EXPECT_TRUE(pprops.transform.IsEqual(t2));

        EXPECT_EQ(a10_20->GetRole(), ExternalSourceAttachment::Role::ShowContext);

        auto xse11 = db.Elements().Get<ExternalSource>(xse11Id);
        ASSERT_TRUE(xse11.IsValid());
        auto a11np = ExternalSourceAttachment::Create(*xse10, makeProps("10->11", t1), *xse11);
        ASSERT_TRUE(a11np.IsValid());
        auto a10_11 = a11np->InsertT<ExternalSourceAttachment>();
        ASSERT_TRUE(a10_11.IsValid());
        ASSERT_EQ(a10_11->GetParentId(), xse10Id);
        ASSERT_EQ(a10_11->GetAttachedExternalSource(), xse11);
        a10_11_id = a10_11->GetElementId();

        std::map<ExternalSourceAttachmentCPtr, ExternalSourceCPtr> tree;
        xse10->ForEachAttachment([&](ExternalSourceAttachmentCR a)
            {
            tree[&a] = a.GetAttachedExternalSource();
            return true;
            });
        ASSERT_EQ(tree.size(), 2);
        ASSERT_TRUE(tree[a10_20] == xse20);
        ASSERT_TRUE(tree[a10_11] == xse11);
        }

    BeJsDocument groupJsonProps("{\"foo\": true}");
    DgnElementId groupId;
        {
        auto group = createAndInsertExternalSourceGroup(db, "group", groupJsonProps);
        ASSERT_TRUE(group.IsValid());
        groupId = group->GetElementId();

        EXPECT_FALSE(group->GetRepository().IsValid());
        }

    db.Elements().ClearCache();
        {
        auto group = db.Elements().Get<ExternalSourceGroup>(groupId);
        ASSERT_TRUE(group.IsValid());

        auto storedJson = group->GetJsonProperties();
        EXPECT_TRUE(storedJson.isAlmostEqual(groupJsonProps));

        auto xse10 = db.Elements().Get<ExternalSource>(xse10Id);
        auto xse11 = db.Elements().Get<ExternalSource>(xse11Id);
        auto xse20 = db.Elements().Get<ExternalSource>(xse20Id);
        group->Add(*xse10);
        group->Add(*xse11);
        group->Add(*xse20);
        auto members = group->QueryMembers();
        ASSERT_EQ(members.size(), 3);
        ASSERT_TRUE(group->HasMember(*xse10));
        ASSERT_TRUE(group->HasMember(*xse11));
        ASSERT_TRUE(group->HasMember(*xse20));
        }


    if (true)
        {
        auto xse11 = db.Elements().Get<ExternalSource>(xse11Id);
        auto xse10 = db.Elements().Get<ExternalSource>(xse10Id);
        auto xse20 = db.Elements().Get<ExternalSource>(xse20Id);

        // Delete xse11
        EXPECT_EQ(xse11->Delete(), DgnDbStatus::Success);

        //  ... that should leave a dangling attachment pointing *to* it
        auto a10_11 = db.Elements().Get<ExternalSourceAttachment>(a10_11_id);
        ASSERT_TRUE(a10_11.IsValid()) << " Deleting a target XSE should not delete its referencing attachments";
        ASSERT_TRUE(a10_11->GetAttachedExternalSourceId() == xse11Id);
        ASSERT_FALSE(a10_11->GetAttachedExternalSource().IsValid()) << " missing attachment is not accessible";

        //  ... should remove pointer to it from any group that it was in
        auto group = db.Elements().Get<ExternalSourceGroup>(groupId);
        ASSERT_TRUE(group.IsValid()) << " group should still be there ... with a dangling pointer to one of its members";
        auto members = group->QueryMembers();
        ASSERT_EQ(members.size(), 2);
        ASSERT_TRUE(group->HasMember(*xse10));
        ASSERT_TRUE(group->HasMember(*xse20));

        //  Remove a group member explicitly
        group->Remove(*xse10);
        ASSERT_EQ(group->QueryMembers().size(), 1);
        ASSERT_FALSE(group->HasMember(*xse10));
        ASSERT_TRUE(group->HasMember(*xse20));

        // Delete the top-level XSE => deletes all of its attachments (but not the attached XSEs)
        EXPECT_EQ(xse10->Delete(), DgnDbStatus::Success);
        xse10 = nullptr;
        EXPECT_FALSE(db.Elements().Get<ExternalSource>(xse10Id).IsValid());
        EXPECT_FALSE(db.Elements().Get<ExternalSourceAttachment>(a10_20_id).IsValid()) << " Deleting an XSE should delete its child attachments";
        EXPECT_FALSE(db.Elements().Get<ExternalSourceAttachment>(a10_11_id).IsValid()) << " Deleting an XSE should delete its child attachments";

        // xse20 is still there
        EXPECT_TRUE(db.Elements().Get<ExternalSource>(xse20Id).IsValid());
        EXPECT_EQ(group->QueryMembers().size(), 1);
        ASSERT_TRUE(group->HasMember(*xse20));
        }
    }