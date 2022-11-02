/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Bentley/BeNumerical.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_LOGGING


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool areTransformsEqual(Transform const& t1, Transform const& t2)
    {
    auto matrixTolerance = Angle::TinyAngle();

    DPoint3d x1, x2;
    t1.GetTranslation(x1);
    t2.GetTranslation(x2);

    double maxCoord = std::max<double>(x1.MaxAbs(), x2.MaxAbs());
    double empericalTolernace = 100 * DoubleOps::SmallCoordinateRelTol();

    if (fabs(x1.MaxDiff(x2)) > DoubleOps::SmallCoordinateRelTol())
        LOG.tracev("Existing distance  %f x 1e-10 is greater than SmallCoordinateRelTol for model duplicate detection", x1.MaxDiff(x2) * 1.0e10);
    auto xlatTolerance = std::max<double>(empericalTolernace, BentleyApi::BeNumerical::NextafterDelta(maxCoord));

    return t1.IsEqual(t2, matrixTolerance, xlatTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus addXSA(ExternalSourceR element, Utf8StringCR sourceModelId, Utf8StringCR sourceModelName)
    {
    DgnDbR db = element.GetDgnDb();
    auto aspectClass = db.Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_ExternalSourceAspect);
    if (NULL == aspectClass)
        return DgnDbStatus::MissingDomain;

    auto instance = aspectClass->GetDefaultStandaloneEnabler()->CreateInstance();
    instance->SetValue("Scope", ECN::ECValue(element.GetParentId()));
    instance->SetValue("Identifier", ECN::ECValue(sourceModelId.c_str()));
    instance->SetValue("Kind", ECN::ECValue(BIS_EXTERNAL_SOURCE_ASPECT_KIND_ExternalSource));

    if (!sourceModelName.empty())
        {
        BeJsDocument json;
        json["name"] = sourceModelName;
        instance->SetValue("JsonProperties", ECN::ECValue(json.Stringify().c_str()));
        }

    DgnDbStatus status;
    DgnElement::GenericMultiAspect::AddAspect(element, *instance, &status);
    return status;
    }

#pragma region "ExternalSource"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static CodeSpecCPtr getCodeSpec(DgnDbR db, Utf8String name, CodeScopeSpec codeScopeSpec)
    {
    auto codeSpec = db.CodeSpecs().GetCodeSpec(name.c_str());
    if (codeSpec.IsValid())
        return codeSpec;

    auto newCodeSpec = CodeSpec::Create(db, name.c_str(), codeScopeSpec);
    if (!newCodeSpec.IsValid())
        {
        LOG.errorv("failed to create CodeSpec with name=%s", name.c_str());
        return nullptr;
        }
    auto status = newCodeSpec->Insert();
    if (DgnDbStatus::Success != status)
        {
        LOG.errorv("failed to insert CodeSpec with name=%s status=0x%lx", name.c_str(), status);
        return nullptr;
        }

    return db.CodeSpecs().GetCodeSpec(newCodeSpec->GetCodeSpecId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecCPtr ExternalSource::GetCodeSpec(DgnDbR db) {return getCodeSpec(db, BIS_CODESPEC_ExternalSource, CodeScopeSpec::CreateRepositoryScope());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode ExternalSource::CreateCode(DgnElementCR scope, Utf8StringCR name)
    {
    auto codeSpec = GetCodeSpec(scope.GetDgnDb());
    return codeSpec.IsValid()? codeSpec->CreateCode(scope, name): DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalSource::SetConnectorProperties(ConnectorProperties const& cprops)
    {
    auto storedprops = GetConnectorProperties();
    if (!storedprops.name.empty() && !storedprops.name.Equals(cprops.name))
        {
        LOG.warningv("ExternalSource::SetConnectorProperties is changing connector name from %s to %s", storedprops.name.c_str(), cprops.name.c_str());
        }

    SetPropertyValue("ConnectorName", ECN::ECValue(cprops.name.c_str()));
    SetPropertyValue("ConnectorVersion", ECN::ECValue(cprops.version.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalSource::ConnectorProperties ExternalSource::GetConnectorProperties() const
    {
    ConnectorProperties cprops;

    ECN::ECValue v;
    GetPropertyValue(v, "ConnectorName");
    cprops.name = v.GetUtf8CP();

    GetPropertyValue(v, "ConnectorVersion");
    cprops.version = v.GetUtf8CP();

    return cprops;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalSourcePtr ExternalSource::Create(DgnDbStatus* statusOut, Properties const& props, RepositoryLinkCR rlink, DgnModelCP model, BeJsConst jsonProperties)
    {
    DgnDbR db = rlink.GetDgnDb();
    DgnModelId modelId = model ? model->GetModelId() : DgnModel::RepositoryModelId();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::ExternalSource::GetHandler());
    // Note that an ExternalSource does not have a parent

    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);

    if (!rlink.GetElementId().IsValid())
        {
        BeAssert(false);
        LOG.errorv("ExternalSource::Create - must supply a valid RepositoryLink");
        status = DgnDbStatus::BadArg;
        return nullptr;
        }

    if (props.m_code.IsValid() && props.m_code.GetCodeSpecId() != GetCodeSpec(db)->GetCodeSpecId())
        {
        BeAssert(false);
        status = DgnDbStatus::InvalidCode;
        return nullptr;
        }

    auto xse = new ExternalSource(ExternalSource::CreateParams(db, modelId, classId, props.m_code));

    auto userLabel = !props.m_userLabel.empty()? props.m_userLabel.c_str(): props.m_source.name.c_str();
    xse->SetUserLabel(userLabel);

    xse->SetConnectorProperties(props.m_connector);

    auto relClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ExternalSourceIsInRepository);
    xse->SetPropertyValue("Repository", rlink.GetElementId(), relClassId);

    addXSA(*xse, props.m_source.identifier, props.m_source.name);

    if (!jsonProperties.isNull())
        xse->SetJsonProperties(json_externalSource(), jsonProperties);

    return xse;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryLinkId ExternalSource::GetRepositoryLinkId() const
    {
    ECN::ECValue value;
    GetPropertyValue(value, "Repository");
    if (value.IsNull())
        return RepositoryLinkId();

    BeAssert(value.IsNavigation());
    ECN::ECValue::NavigationInfo const& info = value.GetNavigationInfo();
    return info.GetId<RepositoryLinkId>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryLinkCPtr ExternalSource::GetRepository() const
    {
    return GetDgnDb().Elements().Get<RepositoryLink>(GetRepositoryLinkId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<ExternalSourceCPtr> ExternalSource::FindByRepository(DgnDbR db, RepositoryLinkId targetId)
    {
    std::vector<ExternalSourceCPtr> nodes;

    auto stmt = db.GetPreparedECSqlStatement("select sourceecinstanceid from " BIS_SCHEMA(BIS_REL_ExternalSourceIsInRepository) " where targetecinstanceid=?");
    stmt->BindId(1, targetId);
    while (BeSQLite::BE_SQLITE_ROW == stmt->Step())
        {
        auto xse = db.Elements().Get<ExternalSource>(stmt->GetValueId<DgnElementId>(0));
        if (!xse.IsValid())
            {
            LOG.errorv("iModelBridge::FindBySourceRepositoryLink cannot get element %lld", stmt->GetValueId<DgnElementId>(0).GetValue());
            return nodes;
            }
        nodes.push_back(xse);
        }
    return nodes;
    }

std::vector<ExternalSourceCPtr> ExternalSource::FindByRepository(RepositoryLinkCR rlink)
    {
    return FindByRepository(rlink.GetDgnDb(), RepositoryLinkId(rlink.GetElementId().GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalSourceCPtr ExternalSource::FindBySourceIdentifier(DgnDbR db, RepositoryLinkId rlinkId, Utf8StringCR sourceId)
    {
    auto stmt = db.GetPreparedECSqlStatement("select xse.ecinstanceid from " BIS_SCHEMA(BIS_CLASS_ExternalSource) " xse, " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " xsa "
                                                " where xse.Repository.Id = ? "
                                                "   and xse.ecinstanceid = xsa.Element.Id"
                                                "   and xsa.kind='" BIS_EXTERNAL_SOURCE_ASPECT_KIND_ExternalSource "'"
                                                "   and xsa.Identifier = ?");
    stmt->BindId(1, rlinkId);
    stmt->BindText(2, sourceId.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;

    auto xse = db.Elements().Get<ExternalSource>(stmt->GetValueId<DgnElementId>(0));
    if (!xse.IsValid())
        {
        BeAssert(false);
        LOG.errorv("ExternalSource::FindByExternalSourceIdentifier - unexpected element type or element not found %lld %s %lld", rlinkId.GetValue(), sourceId.c_str(), stmt->GetValueId<DgnElementId>(0).GetValue());
        }
    return xse;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalSourceCPtr ExternalSource::FindBySourceIdentifier(RepositoryLinkCR rlink, Utf8StringCR sourceId)
    {
    return FindBySourceIdentifier(rlink.GetDgnDb(), RepositoryLinkId(rlink.GetElementId().GetValue()), sourceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<ExternalSourceCPtr> ExternalSource::FindByFilename(DgnDbR db, BeFileNameCR fn)
    {
    std::vector<ExternalSourceCPtr> sources;

    if (!fn.StartsWithI(L"http:") && !fn.StartsWithI(L"https:") && !fn.StartsWithI(L"pw:"))
        {
        auto stmt = db.GetPreparedECSqlStatement(
        "SELECT xse.ECInstanceId "
        " FROM "
            BIS_SCHEMA(BIS_CLASS_ExternalSource) " xse, "
            BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " xsa "
        " WHERE xse.Repository.Id = xsa.Element.Id"
        "   AND xsa.Kind = 'DocumentWithBeGuid'"
        "   AND json_extract(xsa.JsonProperties, '$.fileName') LIKE ?"
        );
        stmt->BindText(1, Utf8String(fn).c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
        while (BE_SQLITE_ROW == stmt->Step())
            {
            auto xse = db.Elements().Get<ExternalSource>(stmt->GetValueId<DgnElementId>(0));
            if (xse.IsValid())
                sources.push_back(xse);
            }
        return sources;
        }

    // For URLS, like won't work. We must do a brute force search.
    auto stmt = db.GetPreparedECSqlStatement(
        "SELECT xse.ECInstanceId, json_extract(xsa.JsonProperties, '$.fileName')"
        "FROM "
            BIS_SCHEMA(BIS_CLASS_ExternalSource) " xse, "
            BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " xsa "
        " WHERE xse.Repository.Id = xsa.Element.Id"
        "   AND xsa.Kind = 'DocumentWithBeGuid'"
        );
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto elid = stmt->GetValueId<DgnElementId>(0);
        BeFileName rlinkFn(stmt->GetValueText(1), true);
        if (rlinkFn.EqualsI(fn))
            {
            auto xse = db.Elements().Get<ExternalSource>(elid);
            if (xse.IsValid())
                sources.push_back(xse);
            }
        }
    return sources;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalSource::SourceProperties ExternalSource::GetSourceProperties() const
    {
    auto stmt = GetDgnDb().GetPreparedECSqlStatement("select xsa.identifier from " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " xsa "
                                                        " where xsa.kind='" BIS_EXTERNAL_SOURCE_ASPECT_KIND_ExternalSource "'"
                                                        "   and xsa.Element.Id = ?");
    stmt->BindId(1, GetElementId());
    if (BE_SQLITE_ROW != stmt->Step())
        {
        BeAssert(false);
        LOG.errorv("ExternalSource::GetSourceProperties - ExternalSourceAspect not found for element %lld", GetElementId().GetValue());
        return {};
        }

    SourceProperties sprops;
    sprops.name = GetUserLabel();
    sprops.identifier = stmt->GetValueText(0);
    return sprops;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalSource::ForEachAttachment(T_ExternalSourceAttachmentProc proc) const
    {
    auto childIds = QueryChildren();
    for (auto childId : childIds)
        {
        auto attachment = GetDgnDb().Elements().Get<ExternalSourceAttachment>(childId);
        if (attachment.IsValid())
            if (!proc(*attachment))
                break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeJsConst ExternalSource::GetJsonProperties() const
    {
    return DgnElement::GetJsonProperties(json_externalSource());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeJsConst ExternalSource::GetAttachmentPath() const
    {
    return GetJsonProperties()[json_attachmentPath()];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalSource::SetAttachmentPath(BeJsConst path)
    {
    auto props = T_Super::GetJsonPropertiesR(json_externalSource());
    props[json_attachmentPath()].From(path);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename PT>
static Utf8String fmtPoint(PT const& pt)
    {
    return Utf8PrintfString("(%lf,%lf,%lf)", pt.x, pt.y, pt.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String fmtYPR(YawPitchRollAngles const& ypr)
    {
    return Utf8PrintfString("(%lf,%lf,%lf)", ypr.GetYaw().Degrees(), ypr.GetPitch().Degrees(), ypr.GetRoll().Degrees());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String fmtTransform(TransformCR t)
    {
    if (t.Matrix().IsIdentity())
        return fmtPoint(t.Translation());

    BeJsDocument json;
    BeJsGeomUtils::TransformToJson(json, t);
    return json.Stringify();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ExternalSourceAttachment::PlacementProperties::Fmt() const
    {
    Utf8String msg;
    if (!rotation.IsIdentity())
        msg.append("ypr=").append(fmtYPR(rotation)).append(" ");

    if (!translation.AlmostEqual(DPoint3d::FromZero()))
        msg.append("xlat=").append(fmtPoint(translation)).append(" ");

    if (!scale.AlmostEqual(DVec3d::From(1.0,1.0,1.0)))
        msg.append("scale=").append(fmtPoint(scale));

    return msg.empty()? "coincident": msg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ExternalSource::Fmt() const
    {
    auto sprops = GetSourceProperties();
    auto cprops = GetConnectorProperties();
    auto repo = GetRepository();
    return Utf8PrintfString("[0x%llx] \"%s\" userlabel=%s sprops={%s,%s} cprops={%s,%s} repo=0x%llx",
        GetElementId().GetValue(),  GetUserLabel(),
        GetUserLabel()? GetUserLabel(): "",
        sprops.identifier.c_str(), sprops.name.c_str(),
        cprops.name.c_str(), cprops.version.c_str(), repo.IsValid()? repo->GetElementId().GetValue(): 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalSource::LogTree(std::map<ExternalSourceCPtr, std::vector<Transform>>& xsesFound, TransformCR t, int indentLevel, T_Filter filter) const
    {
    if (!filter(*this))
        return;

    std::string indent(indentLevel, '\t');
    auto sprops = GetSourceProperties();
    auto cprops = GetConnectorProperties();
    auto repo = GetRepository();
    LOG.infov("%sXSE %s [transform=%s]", indent.c_str(), Fmt().c_str(), fmtTransform(t).c_str());

    auto& transformsFound = xsesFound[this];
    for (auto const& tFound : transformsFound)
        {
        if (t.IsEqual(tFound))
            {
            // we've already visited this node
            LOG.infov("%s(redundant)", indent.c_str());
            return;
            }
        }
    transformsFound.push_back(t);

    ForEachAttachment([&](ExternalSourceAttachmentCR attachment)
        {
        attachment.LogTree(xsesFound, t, indentLevel+1, filter);
        return true;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ExternalSourceAttachment::Fmt() const
    {
    auto pprops = GetPlacementProperties();
    return Utf8PrintfString("[0x%llx] \"%s\" role:%d placement:%s ExternalSource:0x%llx", GetElementId().GetValue(), GetUserLabel(), GetRole(), pprops.Fmt().c_str(), GetAttachedExternalSourceId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalSourceAttachment::LogTree(std::map<ExternalSourceCPtr, std::vector<Transform>>& xsesFound, TransformCR t, int indentLevel, ExternalSource::T_Filter filter) const
    {
    auto target = GetAttachedExternalSource();
    if (!target.IsValid())
        return;

    if (!filter(*target))
        return;

    std::string indent(indentLevel, '\t');
    LOG.infov("%sattachment %s", indent.c_str(), Fmt().c_str());

    auto attachedTransform = Transform::FromProduct(t, GetPlacementProperties().transform);
    target->LogTree(xsesFound, attachedTransform, indentLevel+1, filter);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalSourceAttachmentCPtr ExternalSource::FindTransformedAttachmentTo(ExternalSourceCR xseToFind, TransformCR absoluteTransformToMatch, TransformCR absoluteTransformToThisXse, int indentLevel) const
    {
    Utf8String indent(indentLevel, '\t');
    LOG.tracev("%s%s %s HasTransformedAttachmentTo? %s %s", indent.c_str(),
        Fmt().c_str(), fmtTransform(absoluteTransformToThisXse).c_str(),
        xseToFind.Fmt().c_str(), fmtTransform(absoluteTransformToMatch).c_str());

    ExternalSourceAttachmentCPtr foundAttachment;
    ForEachAttachment([&](auto const& attachment)
        {
        auto attached = attachment.GetAttachedExternalSource();
        if (attached != &xseToFind)
            return true; // keep looking

        indentLevel++;
        Utf8String indent(indentLevel, '\t');

        auto absoluteChildTransform = Transform::FromProduct(absoluteTransformToThisXse, attachment.GetPlacementProperties().transform);

        LOG.tracev("%sattachmentTransform=%s", indent.c_str(), fmtTransform(absoluteChildTransform).c_str());

        if ((attached == &xseToFind) && areTransformsEqual(absoluteChildTransform, absoluteTransformToMatch))
            {
            foundAttachment = &attachment;
            LOG.tracev("%sYES", indent.c_str());
            return false; // stop the iteration
            }

        foundAttachment = attached->FindTransformedAttachmentTo(xseToFind, absoluteTransformToMatch, absoluteChildTransform, indentLevel);
        if (foundAttachment.IsValid())
            {
            LOG.tracev("%s(Found in nested attachment)", indent.c_str());
            return false; // stop the iteration
            }

        return true; // keep looking
        });

    return foundAttachment;
    }

#pragma region "ExternalSourceGroup"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalSourceGroupPtr ExternalSourceGroup::Create(DgnDbStatus* statusOut, DgnDbR db, Properties const& props, RepositoryLinkCP rlink, DgnModelCP model, BeJsConst jsonProperties)
    {
    DgnModelId modelId = model ? model->GetModelId() : DgnModel::RepositoryModelId();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::ExternalSourceGroup::GetHandler());
    // DgnElementId parentId; // no parent
    // DgnClassId parentRelClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);

    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);

    if (!classId.IsValid())
        {
        BeAssert(false);
        LOG.errorv("ExternalSourceGroup::Create - invalid parameter");
        status = DgnDbStatus::BadArg;
        return nullptr;
        }

    auto xse = new ExternalSourceGroup(ExternalSourceGroup::CreateParams(db, modelId, classId, props.m_code));

    if (!props.m_source.name.empty())
        xse->SetUserLabel(props.m_source.name.c_str());

    xse->SetConnectorProperties(props.m_connector);

    if (nullptr != rlink)
        {
        auto relClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ExternalSourceIsInRepository);
        xse->SetPropertyValue("Repository", rlink->GetElementId(), relClassId);
        }

    if (!props.m_source.identifier.empty())
        addXSA(*xse, props.m_source.identifier, props.m_source.name);

    if (!jsonProperties.isNull())
        xse->SetJsonProperties(json_externalSource(), jsonProperties);

    return xse;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ExternalSourceGroup::Add(ExternalSourceCR member, int priority) const
    {
    if (!GetElementId().IsValid() || !member.GetElementId().IsValid())
        return DgnDbStatus::InvalidId; // elements must be inserted to form link table relationship

    CachedECSqlStatementPtr statement = GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "INSERT INTO " BIS_SCHEMA(BIS_REL_ExternalSourceGroupGroupsSources)
        " (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId,MemberPriority) VALUES(?,?,?,?,?)", GetDgnDb().GetECCrudWriteToken());

    if (!statement.IsValid())
        return DgnDbStatus::BadRequest;

    statement->BindId(1, GetElementClassId());
    statement->BindId(2, GetElementId());
    statement->BindId(3, member.GetElementClassId());
    statement->BindId(4, member.GetElementId());
    statement->BindInt(5, priority);
    DbResult result = statement->Step();
    if (BE_SQLITE_DONE == result)
        return DgnDbStatus::Success;
    if (BE_SQLITE_CONSTRAINT_UNIQUE == result)
        return DgnDbStatus::ConstraintNotUnique;
    return DgnDbStatus::BadRequest;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ExternalSourceGroup::Remove(ExternalSourceCR member) const
    {
    CachedECSqlStatementPtr statement = GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "DELETE FROM " BIS_SCHEMA(BIS_REL_ExternalSourceGroupGroupsSources) " WHERE SourceECInstanceId=? AND TargetECInstanceId=?", GetDgnDb().GetECCrudWriteToken());

    if (!statement.IsValid())
        return DgnDbStatus::BadRequest;

    statement->BindId(1, GetElementId());
    statement->BindId(2, member.GetElementId());
    return (BE_SQLITE_DONE == statement->Step()) ? DgnDbStatus::Success : DgnDbStatus::BadRequest;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExternalSourceGroup::HasMember(ExternalSourceCR member) const
    {
    CachedECSqlStatementPtr statement = GetDgnDb().GetPreparedECSqlStatement(
        "SELECT SourceECInstanceId FROM " BIS_SCHEMA(BIS_REL_ExternalSourceGroupGroupsSources) " WHERE SourceECInstanceId=? AND TargetECInstanceId=? LIMIT 1");

    if (!statement.IsValid())
        return false;

    statement->BindId(1, GetElementId());
    statement->BindId(2, member.GetElementId());
    return (BE_SQLITE_ROW == statement->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet ExternalSourceGroup::QueryMembers() const
    {
    CachedECSqlStatementPtr statement = GetDgnDb().GetPreparedECSqlStatement(
        "SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_ExternalSourceGroupGroupsSources) " WHERE SourceECInstanceId=?");

    if (!statement.IsValid())
        return DgnElementIdSet();

    statement->BindId(1, GetElementId());

    DgnElementIdSet elementIdSet;
    while (BE_SQLITE_ROW == statement->Step())
        elementIdSet.insert(statement->GetValueId<DgnElementId>(0));

    return elementIdSet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int ExternalSourceGroup::QueryMemberPriority(ExternalSourceCR member) const
    {
    CachedECSqlStatementPtr statement = GetDgnDb().GetPreparedECSqlStatement(
        "SELECT MemberPriority FROM " BIS_SCHEMA(BIS_REL_ExternalSourceGroupGroupsSources) " WHERE SourceECInstanceId=? AND TargetECInstanceId=? LIMIT 1");

    if (!statement.IsValid())
        return -1;

    statement->BindId(1, GetElementId());
    statement->BindId(2, member.GetElementId());
    return (BE_SQLITE_ROW == statement->Step()) ? statement->GetValueInt(0) : -1;
    }

#pragma region "ExternalSourceAttachment"

CodeSpecCPtr ExternalSourceAttachment::GetCodeSpec(DgnDbR db) {return getCodeSpec(db, BIS_CODESPEC_ExternalSourceAttachment, CodeScopeSpec::CreateParentElementScope());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode ExternalSourceAttachment::CreateCode(ExternalSourceCR parent, Utf8StringCR name)
    {
    auto codeSpec = GetCodeSpec(parent.GetDgnDb());
    return codeSpec.IsValid()? codeSpec->CreateCode(parent, name): DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalSourceAttachment::SetPlacementProperties(PlacementProperties const& props)
    {
    auto angles = props.rotation;
    SetPropertyValue("Yaw", ECN::ECValue(angles.GetYaw().Degrees()));
    SetPropertyValue("Pitch", ECN::ECValue(angles.GetPitch().Degrees()));
    SetPropertyValue("Roll", ECN::ECValue(angles.GetRoll().Degrees()));
    SetPropertyValue("Translation", ECN::ECValue(props.translation));
    SetPropertyValue("Scale", ECN::ECValue(props.scale));

    BeJsDocument tjson;
    BeJsGeomUtils::TransformToJson(tjson, props.transform);
    GetJsonPropertiesR(json_externalSourceAttachment())[json_transform()].From(tjson);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalSourceAttachment::PlacementProperties ExternalSourceAttachment::GetPlacementProperties() const
    {
    PlacementProperties pprops;

    ECN::ECValue v;
    GetPropertyValue(v, "Yaw");
    if (!v.IsNull())
        pprops.rotation.SetYaw(AngleInDegrees::FromDegrees(v.GetDouble()));
    GetPropertyValue(v, "Pitch");
    if (!v.IsNull())
        pprops.rotation.SetPitch(AngleInDegrees::FromDegrees(v.GetDouble()));
    GetPropertyValue(v, "Roll");
    if (!v.IsNull())
        pprops.rotation.SetRoll(AngleInDegrees::FromDegrees(v.GetDouble()));

    GetPropertyValue(v, "Translation");
    if (!v.IsNull())
        pprops.translation = v.GetPoint3d();
    else
        pprops.translation.Zero();

    GetPropertyValue(v, "Scale");
    if (!v.IsNull())
        pprops.scale.Init(v.GetPoint3d());
    else
        pprops.scale.Zero();

    auto json = GetJsonProperties(json_externalSourceAttachment());
    JsonUtils::TransformFromJson(pprops.transform, Json::Value::From(json[json_transform()].Stringify()));

    return pprops;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalSourceAttachment::PlacementProperties::SetTransform(TransformCR t)
    {
    transform = t;

    YawPitchRollAngles::TryFromTransform(translation, rotation, transform); // (may fail)

    double relativeScale{};
    if (transform.IsRigidScale(relativeScale))
        {
        scale.Init(relativeScale,relativeScale,relativeScale);
        }
    else
        {
        RotMatrix r;
        r.NormalizeColumnsOf(transform.Matrix(), scale);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalSourceAttachment::Role ExternalSourceAttachment::GetRole() const
    {
    Properties props;
    ECN::ECValue v;
    GetPropertyValue(v, "Role");
    BeAssert(v.IsInteger());
    return (Role)v.GetInteger();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalSourceAttachment::SetRole(Role r)
    {
    SetPropertyValue("Role", ECN::ECValue((int)r));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeJsConst ExternalSourceAttachment::GetAttachmentPath() const
    {
    return GetJsonProperties(json_externalSourceAttachment())[json_path()];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalSourceAttachment::SetAttachmentPath(BeJsConst path)
    {
    GetJsonPropertiesR(json_externalSourceAttachment())[json_path()].From(path);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalSourceAttachmentCPtr ExternalSourceAttachment::Find(ExternalSourceCR parent, TransformCR transform, ExternalSourceCR target, Utf8StringCR label)
    {
    auto& db = parent.GetDgnDb();
    auto stmt = db.GetPreparedECSqlStatement("select ecinstanceid from " BIS_SCHEMA(BIS_CLASS_ExternalSourceAttachment) " where Parent.Id=? and Attaches.Id=? and UserLabel=?");
    stmt->BindId(1, parent.GetElementId());
    stmt->BindId(2, target.GetElementId());
    stmt->BindText(3, label.c_str(), EC::IECSqlBinder::MakeCopy::No);
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto attachment = db.Elements().Get<ExternalSourceAttachment>(stmt->GetValueId<DgnElementId>(0));
        if (!attachment.IsValid())
            {
            BeAssert(false);
            continue;
            }
        auto props = attachment->GetPlacementProperties();
        if (areTransformsEqual(props.transform, transform))
            return attachment;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalSourceAttachmentPtr ExternalSourceAttachment::Create(ExternalSourceCR parent, Properties const& props, ExternalSourceCR xsrc)
    {
    DgnDbR db = parent.GetDgnDb();
    DgnModelId modelId = parent.GetModelId();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::ExternalSourceAttachment::GetHandler());
    DgnClassId attachesRelClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ExternalSourceAttachmentAttachesSource);
    DgnClassId parentRelClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ExternalSourceOwnsAttachments);
    DgnElementId parentId = parent.GetElementId();

    if (!classId.IsValid() || !parentId.IsValid() || !parentRelClassId.IsValid() || !xsrc.GetElementId().IsValid())
        {
        BeAssert(false);
        LOG.errorv("ExternalSourceAttachment::Create - invalid parameter");
        return nullptr;
        }

    auto att = new ExternalSourceAttachment(ExternalSourceAttachment::CreateParams(db, modelId, classId, DgnCode(), nullptr, parentId, parentRelClassId));
    att->SetPlacementProperties(props.m_placement);
    att->SetRole(props.m_role);
    att->SetUserLabel(props.m_userLabel.c_str());
    att->SetPropertyValue("Attaches", xsrc.GetElementId(), attachesRelClassId);
    if (!props.m_referenceNum.empty())
        att->GetJsonPropertiesR(json_externalSourceAttachment())[json_referenceNum()] = props.m_referenceNum;

    return att;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ExternalSourceAttachment::GetAttachedExternalSourceId() const
    {
    ECN::ECValue value;
    GetPropertyValue(value, "Attaches");
    if (value.IsNull())
        return DgnElementId();

    BeAssert(value.IsNavigation());
    ECN::ECValue::NavigationInfo const& info = value.GetNavigationInfo();
    return info.GetId<DgnElementId>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalSourceCPtr ExternalSourceAttachment::GetAttachedExternalSource() const
    {
    return GetDgnDb().Elements().Get<ExternalSource>(GetAttachedExternalSourceId());
    }

/*---------------------------------------------------------------------------------**//**
*               ExternalSourceAspect
* Element <---  .Element.Id
*               .Source ---> ExternalSource <--attaches-- ExternalSourceAttachment
*                                         1               *
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<ExternalSourceAttachmentCPtr> ExternalSourceAttachment::FindAttachmentsTo(DgnElementCR el, Utf8CP externalSourceAspectKind)
    {
    auto& db = el.GetDgnDb();

    std::vector<BentleyApi::Dgn::ExternalSourceAttachmentCPtr> attachments;

    auto stmt = db.GetPreparedECSqlStatement(
        "select attachment.ecinstanceid from "
        BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " xsa, "
        BIS_SCHEMA(BIS_CLASS_ExternalSource) " xse, "
        BIS_SCHEMA(BIS_CLASS_ExternalSourceAttachment) " attachment "
        " where attachment.attaches.id = xse.ecinstanceid"
        " and xsa.source.id = xse.ecinstanceid"
        " and xsa.element.id=? and xsa.kind=?"
        );
    stmt->BindId(1, el.GetElementId());
    stmt->BindText(2, externalSourceAspectKind, IECSqlBinder::MakeCopy::No);
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        auto attachment = db.Elements().Get<BentleyApi::Dgn::ExternalSourceAttachment>(stmt->GetValueId<BentleyApi::Dgn::DgnElementId>(0));
        attachments.push_back(attachment);
        }
    return attachments;
    }

#pragma region "SynchronizationConfigLink"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SynchronizationConfigLinkPtr SynchronizationConfigLink::Create(InformationModelR model, BeGuidCR federationGuid, Utf8StringCR url, Utf8StringCR description, BeJsConst props)
    {
    auto& db = model.GetDgnDb();
    UrlLink::CreateParams params(model, url.c_str(), description.c_str());
    params.m_federationGuid = federationGuid;
    params.m_classId = db.Domains().GetClassId(dgn_ElementHandler::SynchronizationConfigLinkHandler::GetHandler());
    params.m_description = description; // (Tricky! The CreateParams constructor above does not set description. It sets userLabel.)
    auto link = new SynchronizationConfigLink(params);
    auto code = DgnCode::CreateEmpty(); // sync config link does not need a code, but its code cannot be invalid. Construct an empty code, to help the caller who plans to update an existing element.
    link->SetCode(code);
    link->SetJsonProperties(json_synchronizationConfigLink(), props);
    return link;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SynchronizationConfigLinkCPtr SynchronizationConfigLink::FindByGuid(DgnDbR db, BeGuidCR guid)
    {
    auto stmt = db.GetPreparedECSqlStatement("select ecinstanceid from " BIS_SCHEMA(BIS_CLASS_SynchronizationConfigLink) " where federationGuid=?");
    stmt->BindGuid(1, guid);
    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;
    auto eid = stmt->GetValueId<DgnElementId>(0);
    return db.Elements().Get<SynchronizationConfigLink>(eid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SynchronizationConfigLinkCPtr SynchronizationConfigLink::FindByUrl(DgnDbR db, Utf8StringCR url)
    {
    auto stmt = db.GetPreparedECSqlStatement("select ecinstanceid from " BIS_SCHEMA(BIS_CLASS_SynchronizationConfigLink) " where url=?");
    stmt->BindText(1, url.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;
    auto eid = stmt->GetValueId<DgnElementId>(0);
    return db.Elements().Get<SynchronizationConfigLink>(eid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime SynchronizationConfigLink::GetLastSuccessfulRun() const
    {
    ECN::ECValue v;
    GetPropertyValue(v, "LastSuccessfulRun");
    return v.GetDateTime();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SynchronizationConfigLink::SetLastSuccessfulRun(DateTime dt)
    {
    ECN::ECValue v(dt);
    SetPropertyValue("LastSuccessfulRun", v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SynchronizationConfigLink::AddRoot(ExternalSourceCR member) const
    {
    if (!GetElementId().IsValid() || !member.GetElementId().IsValid())
        {
        BeAssert(false);
        LOG.error("SynchronizationConfigLink::AddSource - must bge persistent");
        return DgnDbStatus::InvalidId; // elements must be inserted to form link table relationship
        }

    CachedECSqlStatementPtr statement = GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "INSERT INTO " BIS_SCHEMA(BIS_REL_SynchronizationConfigSpecifiesRootSources)
        " (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)", GetDgnDb().GetECCrudWriteToken());

    if (!statement.IsValid())
        return DgnDbStatus::BadRequest;

    statement->BindId(1, GetElementClassId());
    statement->BindId(2, GetElementId());
    statement->BindId(3, member.GetElementClassId());
    statement->BindId(4, member.GetElementId());
    DbResult result = statement->Step();
    if (BE_SQLITE_DONE == result)
        return DgnDbStatus::Success;
    if (BE_SQLITE_CONSTRAINT_UNIQUE == result)
        return DgnDbStatus::ConstraintNotUnique;
    return DgnDbStatus::BadRequest;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<ExternalSourceCPtr> SynchronizationConfigLink::GetSources(Utf8CP relClass) const
    {
    std::vector<ExternalSourceCPtr> roots;

    if (!GetElementId().IsValid())
        {
        BeAssert(false);
        LOG.error("SynchronizationConfigLink::GetSources - must be persistent");
        return roots;
        }
    auto syncroot = GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("select targetecinstanceid from %s where sourceecinstanceid = ?", relClass).c_str());
    syncroot->BindId(1, GetElementId());
    while (BE_SQLITE_ROW == syncroot->Step())
        {
        auto elid = syncroot->GetValueId<DgnElementId>(0);
        auto xse = GetDgnDb().Elements().Get<ExternalSource>(elid);
        if (!xse.IsValid())
            {
            BeDataAssert(false);
            LOG.errorv("SynchronizationConfigLink::GetSources - Element 0x%llx is a source of synchronization 0x%llx, but it is either not an ExternalSource or it it cannot be loaded", elid.GetValue(), GetElementId().GetValue());
            return roots;
            }
        roots.push_back(xse);
        }

    return roots;
    }

std::vector<ExternalSourceCPtr> SynchronizationConfigLink::GetRoots() const {return GetSources(BIS_SCHEMA(BIS_REL_SynchronizationConfigSpecifiesRootSources));}
std::vector<ExternalSourceCPtr> SynchronizationConfigLink::GetSources() const {return GetSources(BIS_SCHEMA(BIS_REL_SynchronizationConfigProcessesSources));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SynchronizationConfigLink::RemoveSource(ExternalSourceCR member, Utf8CP relClass) const
    {
    if (!GetElementId().IsValid() || !member.GetElementId().IsValid())
        {
        BeAssert(false);
        LOG.error("SynchronizationConfigLink::AddSource - must bge persistent");
        return DgnDbStatus::InvalidId; // elements must be inserted to form link table relationship
        }

    CachedECSqlStatementPtr statement = GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "DELETE FROM " BIS_SCHEMA(BIS_REL_SynchronizationConfigProcessesSources) " WHERE (SourceECInstanceId=? AND TargetECInstanceId=?)", GetDgnDb().GetECCrudWriteToken());

    if (!statement.IsValid())
        return DgnDbStatus::BadRequest;

    statement->BindId(1, GetElementId());
    statement->BindId(2, member.GetElementId());
    DbResult result = statement->Step();
    if (BE_SQLITE_DONE == result)
        return DgnDbStatus::Success;
    return DgnDbStatus::BadRequest;
    }

DgnDbStatus SynchronizationConfigLink::RemoveRoot(ExternalSourceCR member) const {return RemoveSource(member, BIS_SCHEMA(BIS_REL_SynchronizationConfigSpecifiesRootSources));}
DgnDbStatus SynchronizationConfigLink::RemoveSource(ExternalSourceCR member) const {return RemoveSource(member, BIS_SCHEMA(BIS_REL_SynchronizationConfigProcessesSources));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SynchronizationConfigLink::AddSource(ExternalSourceCR member) const
    {
    if (!GetElementId().IsValid() || !member.GetElementId().IsValid())
        {
        BeAssert(false);
        LOG.error("SynchronizationConfigLink::AddSource - must bge persistent");
        return DgnDbStatus::InvalidId; // elements must be inserted to form link table relationship
        }

    CachedECSqlStatementPtr statement = GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "INSERT INTO " BIS_SCHEMA(BIS_REL_SynchronizationConfigProcessesSources)
        " (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)", GetDgnDb().GetECCrudWriteToken());

    if (!statement.IsValid())
        return DgnDbStatus::BadRequest;

    statement->BindId(1, GetElementClassId());
    statement->BindId(2, GetElementId());
    statement->BindId(3, member.GetElementClassId());
    statement->BindId(4, member.GetElementId());
    DbResult result = statement->Step();
    if (BE_SQLITE_DONE == result)
        return DgnDbStatus::Success;
    if (BE_SQLITE_CONSTRAINT_UNIQUE == result)
        return DgnDbStatus::ConstraintNotUnique;
    return DgnDbStatus::BadRequest;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SynchronizationConfigLink::HasSource(ExternalSourceCR member, Utf8CP relClass) const
    {
    if (!GetElementId().IsValid() || !member.GetElementId().IsValid())
        {
        LOG.error("SynchronizationConfigLink::HasSource - must bge persistent");
        BeAssert(false);
        return false;
        }

    auto statement = GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT * FROM %s WHERE SourceECInstanceId=? AND TargetECInstanceId=?", relClass).c_str());
    statement->BindId(1, GetElementId());
    statement->BindId(2, member.GetElementId());
    return BE_SQLITE_ROW == statement->Step();
    }

bool SynchronizationConfigLink::HasRoot(ExternalSourceCR member) const {return HasSource(member, BIS_SCHEMA(BIS_REL_SynchronizationConfigSpecifiesRootSources));}
bool SynchronizationConfigLink::HasSource(ExternalSourceCR member) const {return HasSource(member, BIS_SCHEMA(BIS_REL_SynchronizationConfigProcessesSources));}
