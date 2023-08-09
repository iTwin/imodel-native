/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once


#include <Bentley/BeAssert.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/DgnModel.h>
#include <DgnPlatform/ElementHandler.h>

#define BIS_CLASS_LinkElement           "LinkElement"
#define BIS_CLASS_LinkModel             "LinkModel"
#define BIS_CLASS_LinkPartition         "LinkPartition"
#define BIS_CLASS_UrlLink               "UrlLink"
#define BIS_CLASS_EmbeddedFileLink      "EmbeddedFileLink"
#define BIS_CLASS_RepositoryLink        "RepositoryLink"
#define BIS_REL_ElementHasLinks         "ElementHasLinks"

#define LINK_ECSQL_PREFIX "link"
#define SOURCE_ECSQL_PREFIX "source"

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_ElementHandler { struct UrlLinkHandler; struct RepositoryLinkHandler; struct EmbeddedFileLinkHandler; struct LinkPartitionHandler; struct SynchronizationConfigLinkHandler; }

//=======================================================================================
//! A LinkPartition provides a starting point for a LinkModel hierarchy
//! @note LinkPartition elements only reside in the RepositoryModel
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinkPartition : InformationPartitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_LinkPartition, InformationPartitionElement);
    friend struct dgn_ElementHandler::LinkPartitionHandler;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnSubModelInsert(DgnModelCR model) const override;
    explicit LinkPartition(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a new LinkPartition
    //! @param[in] parentSubject The new LinkPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this LinkPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static LinkPartitionPtr Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
    //! Create and insert a new LinkPartition
    //! @param[in] parentSubject The new LinkPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this LinkPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static LinkPartitionCPtr CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
};

//=======================================================================================
//! A model which contains only links - LinkElement-s and classes derived from it.
//! @ingroup GROUP_DgnModel
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinkModel : InformationModel
{
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_LinkModel, InformationModel);

public:
    struct CreateParams : Dgn::InformationModel::CreateParams
    {
    DEFINE_T_SUPER(Dgn::InformationModel::CreateParams);

    protected:
        CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnClassId classId, DgnElementId modeledElementId, bool isPrivate = false)
            : T_Super(dgndb, classId, modeledElementId, isPrivate)
            {}

    public:
        //! @private
        //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
        CreateParams(Dgn::DgnModel::CreateParams const& params) : T_Super(params) {}

        //! Parameters to create a new instance of a LinkModel.
        //! @param[in] dgndb The DgnDb for the new DgnModel
        //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
        //! @param[in] isPrivate Optional parameter specifying that this model should @em not appear in lists shown to the user
        CreateParams(Dgn::DgnDbR dgndb, DgnElementId modeledElementId, bool isPrivate = false) :
            T_Super(dgndb, LinkModel::QueryClassId(dgndb), modeledElementId, isPrivate)
            {}
    };

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsertElement(DgnElementR element) override;
public:
    explicit LinkModel(CreateParams const& params) : T_Super(params) {}

    static LinkModelPtr Create(CreateParams const& params) { return new LinkModel(params); }

    //! Gets the LinkModel by Id. If the model is not loaded, it loads it, but does not fill it with contained elements.
    static LinkModelPtr Get(Dgn::DgnDbCR dgndb, DgnModelId id) { return dgndb.Models().Get<LinkModel>(id); }

    //! Query the DgnClassId of the dgn.LinkModel ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the dgn.LinkModel class - it does @em not return the class of a specific instance.
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_LinkModel)); }
};

//=======================================================================================
//! LinkElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinkElement : InformationReferenceElement
{
    DEFINE_T_SUPER(InformationReferenceElement)

protected:
    //! Constructor
    explicit LinkElement(CreateParams const& params) : T_Super(params) {}

    //! Called when an element is about to be inserted into the DgnDb.
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;

public:
    //! Add the link to a source element
    //! @param[in] sourceElementId Id of the host element.
    //! @remarks The source may contain one or more links. It's also possible that there may be links that aren't in any source element at all
    DGNPLATFORM_EXPORT BentleyStatus AddToSource(DgnElementId sourceElementId) const;

    //! Add the link to a source element
    //! @param[in] dgndb DgnDb
    //! @param[in] linkId DgnElementId of the link
    //! @param[in] sourceElementId DgnElementId of the source element.
    //! @remarks The source may contain one or more links. It's also possible that there may be links that aren't in any source element at all
    DGNPLATFORM_EXPORT static BentleyStatus AddToSource(DgnDbR dgndb, DgnElementId linkId, DgnElementId sourceElementId);

    //! Returns true if the supplied element is one of the sources of this link
    //! @param[in] sourceElementId Id of the host element.
    DGNPLATFORM_EXPORT bool IsFromSource(DgnElementId sourceElementId) const;

    //! Returns true if the supplied element is one of the sources of the supplied link
    //! @param[in] dgndb DgnDb
    //! @param[in] linkId DgnElementId of the link
    //! @param[in] sourceElementId DgnElementId of the source element.
    DGNPLATFORM_EXPORT static bool IsFromSource(DgnDbR dgndb, DgnElementId linkId, DgnElementId sourceElementId);

    //! Removes this link from a source element
    //! @param[in] sourceElementId Id of the source element. If invalid id is passed in, removes the specified link from all hosts.
    //! @remarks The removed links are NOT deleted. Use @ref DgnElement::Delete to delete the link.
    DGNPLATFORM_EXPORT BentleyStatus RemoveFromSource(DgnElementId sourceElementId) const;

    //! Removes this link from a source element
    //! @param[in] dgndb DgnDb
    //! @param[in] linkId DgnElementId of the link
    //! @param[in] sourceElementId Id of the source element. If invalid id is passed in, removes the specified link from all hosts.
    //! @remarks The removed links are NOT deleted. Use @ref DgnElement::Delete to delete the link.
    DGNPLATFORM_EXPORT static BentleyStatus RemoveFromSource(DgnDbR dgndb, DgnElementId linkId, DgnElementId sourceElementId);

    //! Query all the source elements that contain this link
    DGNPLATFORM_EXPORT DgnElementIdSet QuerySources();

    DGNPLATFORM_EXPORT static BentleyStatus DoRemoveAllFromSource(DgnDbR dgndb, DgnElementId sourceElementId, Utf8CP schemaName, Utf8CP className, DgnElementIdSet const& targetElements);
    DGNPLATFORM_EXPORT static BentleyStatus DoPurgeOrphaned(DgnDbCR dgndb, Utf8CP schemaName, Utf8CP className, DgnElementIdSet const& unusedIds);
    };

//=======================================================================================
//! LinkElementBase
//=======================================================================================
template<class LINK_SUBTYPE>
struct ILinkElementBase
{
private:
    static BeSQLite::EC::CachedECSqlStatementPtr GetQueryBySourceStatement(DgnDbR dgndb, DgnElementId sourceElementId)
        {
        BeAssert(sourceElementId.IsValid());

        Utf8CP ecSqlFmt = "SELECT " LINK_ECSQL_PREFIX ".ECInstanceId FROM ONLY %s.%s " LINK_ECSQL_PREFIX " " \
            "JOIN " BIS_SCHEMA(BIS_CLASS_Element) " " SOURCE_ECSQL_PREFIX " USING " BIS_SCHEMA(BIS_REL_ElementHasLinks) " " \
            "WHERE " SOURCE_ECSQL_PREFIX ".ECInstanceId=?";

        Utf8PrintfString ecSql(ecSqlFmt, LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyHandlerECClassName());

        BeSQLite::EC::CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(ecSql.c_str());
        BeAssert(stmt.IsValid());

        stmt->BindId(1, sourceElementId);
        return stmt;
        }

protected:
    static DgnElementIdSet CollectElementIds(BeSQLite::EC::ECSqlStatement& stmt)
        {
        DgnElementIdSet idSet;
        BeSQLite::DbResult result;
        while ((result = stmt.Step()) == BeSQLite::DbResult::BE_SQLITE_ROW)
            idSet.insert(stmt.GetValueId<DgnElementId>(0));
        return idSet;
        }

public:
    //! Get a read only copy of the LinkElement from the DgnDb
    //! @return Invalid if the UrlLink does not exist
    static RefCountedCPtr<LINK_SUBTYPE> Get(Dgn::DgnDbCR dgndb, Dgn::DgnElementId linkElementId) { return dgndb.Elements().Get<LINK_SUBTYPE>(linkElementId); }

    //! Get an editable copy of the LinkElement from the DgnDb
    //! @return Invalid if the UrlLink does not exist, or if it cannot be edited.
    static RefCountedPtr<LINK_SUBTYPE> GetForEdit(Dgn::DgnDbR dgndb, Dgn::DgnElementId linkElementId) { return dgndb.Elements().GetForEdit<LINK_SUBTYPE>(linkElementId); }

    //! Query all the links in the specified source element
    static DgnElementIdSet QueryBySource(DgnDbR dgndb, DgnElementId sourceElementId)
        {
        BeSQLite::EC::CachedECSqlStatementPtr stmt = GetQueryBySourceStatement(dgndb, sourceElementId);
        return CollectElementIds(*stmt);
        }

    //! Returns true if the supplied element is the source of any links
    static bool ElementHasLinks(DgnDbR dgndb, DgnElementId elementId)
        {
        BeSQLite::EC::CachedECSqlStatementPtr stmt = GetQueryBySourceStatement(dgndb, elementId);
        return (BeSQLite::DbResult::BE_SQLITE_ROW == stmt->Step());
        }

    //! Query links by where clause
    //! @param dgndb DgnDb
    //! @param whereClause Optional where clause. e.g., LINK_ECSQL_PREFIX ".Label LIKE 'MyLabel%'"
    static DgnElementIdSet QueryByWhere(DgnDbR dgndb, Utf8CP whereClause)
        {
        Utf8PrintfString ecSql("SELECT " LINK_ECSQL_PREFIX ".ECInstanceId FROM ONLY %s.%s " LINK_ECSQL_PREFIX, LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyHandlerECClassName());

        if (whereClause)
            {
            ecSql.append(" WHERE ");
            ecSql.append(whereClause);
            }

        BeSQLite::EC::CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(ecSql.c_str());
        BeAssert(stmt.IsValid());

        return CollectElementIds(*stmt);
        }

    //! Query all the links in the specified link model
    static DgnElementIdSet QueryByModel(DgnDbR dgndb, DgnModelId linkModelId)
        {
        BeAssert(linkModelId.IsValid());

        Utf8PrintfString ecSql("SELECT " LINK_ECSQL_PREFIX ".ECInstanceId FROM ONLY %s.%s " LINK_ECSQL_PREFIX, LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyHandlerECClassName());

        ecSql.append(" WHERE " LINK_ECSQL_PREFIX ".Model.Id=?");

        BeSQLite::EC::CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(ecSql.c_str());
        BeAssert(stmt.IsValid());

        stmt->BindId(1, linkModelId);

        return CollectElementIds(*stmt);
        }

    //! Removes all links from the specified source
    //! @remarks The removed links are NOT deleted. Use @ref DgnElement::Delete to delete the link.
    static BentleyStatus RemoveAllFromSource(DgnDbR dgndb, DgnElementId sourceElementId)
        {
        return LinkElement::DoRemoveAllFromSource(dgndb, sourceElementId, LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyHandlerECClassName(),
                                                QueryBySource(dgndb, sourceElementId));
        }

    //! Finds all links that do not have a source specified
    static DgnElementIdSet FindOrphaned(DgnDbCR dgndb)
        {
        Utf8CP ecSqlFmt = "SELECT link.ECInstanceId FROM ONLY %s.%s link WHERE link.ECInstanceId NOT IN (SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_ElementHasLinks) ")";
        Utf8PrintfString ecSql(ecSqlFmt, LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyHandlerECClassName());

        BeSQLite::EC::CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(ecSql.c_str());
        BeAssert(stmt.IsValid());

        return CollectElementIds(*stmt);
        }

    //! Deletes all links that do not have a source specified
    static BentleyStatus PurgeOrphaned(DgnDbCR dgndb)
        {
        return LinkElement::DoPurgeOrphaned(dgndb, LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyHandlerECClassName(), FindOrphaned(dgndb));
        }

    //! Query the DgnClassId of the LinkElement ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the LinkElement class - it does @em not return the class of a specific instance.
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb)
        {
        return dgndb.Schemas().GetClassId(LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyHandlerECClassName());
        }
    };

//=======================================================================================
//! An element containing an URL link
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE UrlLink : LinkElement, ILinkElementBase<UrlLink>
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_UrlLink, LinkElement)
    friend struct dgn_ElementHandler::UrlLinkHandler;

private:
    Utf8String m_url;
    Utf8String m_description;

protected:
    DGNPLATFORM_EXPORT void _CopyFrom(Dgn::DgnElementCR source, CopyFromOptions const&) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT void _ToJson(BeJsValue out, BeJsConst opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(BeJsConst props) override;

public:
    //! Parameters used to construct a UrlLink
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(UrlLink::T_Super::CreateParams);

    Utf8String m_url;
    Utf8String m_description;

    explicit CreateParams(Dgn::DgnElement::CreateParams const& params, Utf8CP url = nullptr, Utf8CP description = nullptr) : T_Super(params)
        {
        m_url.AssignOrClear(url);
        m_description.AssignOrClear(description);
        }

    //! Constructor
    //! @param[in] linkModel Model that should contain the link
    //! @param[in] url URL that the link points to
    //! @param[in] label Display label of the link
    //! @param[in] description Description of the link
    DGNPLATFORM_EXPORT explicit CreateParams(InformationModelR linkModel, Utf8CP url = nullptr, Utf8CP label = nullptr, Utf8CP description = nullptr);
    };

    BE_JSON_NAME(url);
    BE_JSON_NAME(description);

    //! Constructor
    explicit UrlLink(CreateParams const& params) : T_Super(params), m_url(params.m_url), m_description(params.m_description) {}

    //! Create an UrlLink
    static UrlLinkPtr Create(CreateParams const& params) { return new UrlLink(params); }

    //! Insert the UrlLink in the DgnDb
    DGNPLATFORM_EXPORT UrlLinkCPtr Insert(DgnDbStatus* stat=nullptr);

    //! Update the persistent state of the UrlLink in the DgnDb from this modified copy of it.
    DGNPLATFORM_EXPORT UrlLinkCPtr Update(DgnDbStatus* stat=nullptr);

    //! Query the UrlLink-s by it's properties
    //! @param[in] dgndb DgnDb
    //! @param[in] url URL that the link points to (optional)
    //! @param[in] label Label of the link (optional)
    //! @param[in] description Description of the link (optional)
    //! @param[in] limitCount Limits the count of the returned set (optional - if <=0, no limit is set)
    //! @remarks If any of the parameters are nullptr-s, they are omitted from the query. If they are empty strings, the query is made for empty/null values
    DGNPLATFORM_EXPORT static DgnElementIdSet Query(DgnDbCR dgndb, Utf8CP url = nullptr, Utf8CP label = nullptr, Utf8CP description = nullptr, int limitCount = -1);

    //! Set the url the link points to
    void SetUrl(Utf8CP url) { m_url.AssignOrClear(url); }

    //! Get the url the link points to
    Utf8CP GetUrl() const { return m_url.c_str(); }

    //! Set the description the link points to
    void SetDescription(Utf8CP description) { m_description.AssignOrClear(description); }

    //! Get the description the link points to
    Utf8CP GetDescription() const { return m_description.c_str(); }

    //! Get the schema name for the UrlLink class
    //! @note This is a static method that always returns the schema name of the dgn.UrlLink class - it does @em not return the schema of a specific instance.
    static Utf8CP MyECSchemaName() { return BIS_ECSCHEMA_NAME; }
};

//=======================================================================================
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RepositoryLink : UrlLink
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_RepositoryLink, UrlLink)
    friend struct dgn_ElementHandler::RepositoryLinkHandler;

protected:
    BeSQLite::BeGuid m_repositoryGuid;

    explicit RepositoryLink(CreateParams const& params) : T_Super(params) {}

    DGNPLATFORM_EXPORT void _CopyFrom(Dgn::DgnElementCR source, CopyFromOptions const&) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT void _ToJson(BeJsValue out, BeJsConst opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(BeJsConst props) override;

    BE_JSON_PROP_NAMESPACE(DocumentProperties);

    BE_JSON_NAME(repositoryGuid);

public:
    //! Create a DgnCode using the specified model (uniqueness scope) and name
    DGNPLATFORM_EXPORT static DgnCode CreateCode(InformationModelCR model, Utf8StringCR name);
    //! Create a unique DgnCode for an InformationPartitionElement with the specified Subject as its parent
    //! @param[in] model The uniqueness scope for the DgnCode
    //! @param[in] baseName The base name for the CodeValue. A suffix will be appended (if necessary) to make it unique within the specified scope.
    //! @private
    DGNPLATFORM_EXPORT static DgnCode CreateUniqueCode(InformationModelCR model, Utf8CP baseName);

    //! Create a RepositoryLink in memory
    //! @param[in] model Create the RepositoryLink in this DgnModel
    //! @param[in] url The URL of the RepositoryLink
    //! @param[in] name The name of the RepositoryLink that will be used to form its DgnCode
    //! @param[in] description The optional description of the RepositoryLink
    DGNPLATFORM_EXPORT static RepositoryLinkPtr Create(InformationModelR model, Utf8CP url, Utf8CP name, Utf8CP description = nullptr);

    //! Get the RepositoryGuid property value
    BeSQLite::BeGuid GetRepositoryGuid() const {return m_repositoryGuid;}

    //! Set the RepositoryGuid property value
    void SetRepositoryGuid(BeSQLite::BeGuid g) {m_repositoryGuid = g;}

    //! Get the Document Properties for this InformationPartitionElement. Document Properties provide additional information about
    //! the document to which this link refers.
    BeJsConst GetDocumentProperties() const {return GetJsonProperties(json_prop_namespace_DocumentProperties());}
    BeJsValue GetDocumentPropertiesR() {return GetJsonPropertiesR(json_prop_namespace_DocumentProperties());}

    //! Set the Document Properties of this InformationPartitionElement. Document Properties
    void SetDocumentProperties(BeJsConst value) {GetDocumentPropertiesR().From(value);}
};

//=======================================================================================
//! Link to the Configuration for an iModel Synchronization definition.
//! Every synchronization, local or online, should be identifiable by a stable GUID.
//! Groups a set of ExternalSources.
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct SynchronizationConfigLink : UrlLink
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_SynchronizationConfigLink, UrlLink)
    friend struct dgn_ElementHandler::SynchronizationConfigLinkHandler;

    explicit SynchronizationConfigLink(CreateParams const& params) : T_Super(params) {}

    std::vector<ExternalSourceCPtr> GetSources(Utf8CP relClass) const;
    DgnDbStatus RemoveSource(ExternalSourceCR member, Utf8CP relClass) const;
    bool HasSource(ExternalSourceCR member, Utf8CP relClass) const;

public:
    BE_JSON_NAME(synchronizationConfigLink); //!< The namespace for the additional JsonProperties on a SynchronizationConfigLink element

    //! Create a non-persistent SynchronizationConfigLink
    //! @param model The model where the new link will be stored. Normally, this should be the RepositoryModel
    //! @param url The unique URL that identifies the synchronization definition. This may be the empty string for locally defined or one-off synchronizations.
    //! @param description The name by which the user knows the synchronization
    //! @param federationGuid The stable and unique identifier of the synchronization definition. Every synchronization, local or online, should be identifiable by a stable GUID.
    //! @param props Additional properties of the synchronization definition
    //! @return a new non-persistent SynchronizationConfigLink element
    DGNPLATFORM_EXPORT static SynchronizationConfigLinkPtr Create(InformationModelR model, BeSQLite::BeGuidCR federationGuid, Utf8StringCR url, Utf8StringCR description, BeJsConst props);
    DGNPLATFORM_EXPORT static SynchronizationConfigLinkCPtr FindByUrl(DgnDbR, Utf8StringCR url);
    DGNPLATFORM_EXPORT static SynchronizationConfigLinkCPtr FindByGuid(DgnDbR, BeSQLite::BeGuidCR guid);

    DGNPLATFORM_EXPORT DgnDbStatus AddRoot(ExternalSourceCR) const;
    DGNPLATFORM_EXPORT DgnDbStatus RemoveRoot(ExternalSourceCR) const;
    DGNPLATFORM_EXPORT std::vector<ExternalSourceCPtr> GetRoots() const;
    DGNPLATFORM_EXPORT bool HasRoot(ExternalSourceCR) const;

    DGNPLATFORM_EXPORT DgnDbStatus AddSource(ExternalSourceCR) const;
    DGNPLATFORM_EXPORT DgnDbStatus RemoveSource(ExternalSourceCR) const;
    DGNPLATFORM_EXPORT std::vector<ExternalSourceCPtr> GetSources() const;
    DGNPLATFORM_EXPORT bool HasSource(ExternalSourceCR) const;

    DGNPLATFORM_EXPORT DateTime GetLastSuccessfulRun() const;
    DGNPLATFORM_EXPORT void SetLastSuccessfulRun(DateTime);
};

//=======================================================================================
//! An element containing a link to an file embedded in the Db.
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EmbeddedFileLink : LinkElement, ILinkElementBase<EmbeddedFileLink>
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_EmbeddedFileLink, LinkElement)
    friend struct dgn_ElementHandler::EmbeddedFileLinkHandler;

public:
    //! Parameters used to construct a EmbeddedFileLink
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(EmbeddedFileLink::T_Super::CreateParams);

    Utf8String m_name;
    Utf8String m_description;

    explicit CreateParams(Dgn::DgnElement::CreateParams const& params, Utf8CP name = nullptr, Utf8CP description = nullptr) : T_Super(params)
        {
        m_name.AssignOrClear(name);
        m_description.AssignOrClear(description);
        }

    //! Constructor
    //! @param[in] linkModel Model that should contain the link
    //! @param[in] name Name used to create the embedded file.
    //! @param[in] label Label to identify the link.
    //! @param[in] description Description of the link
    //! @see DbEmbeddedFileTable for details on embedding file in a DgnDb
    DGNPLATFORM_EXPORT CreateParams(LinkModelR linkModel, Utf8CP name, Utf8CP label = nullptr, Utf8CP description = nullptr);
    };

private:
    Utf8String m_name;
    Utf8String m_description;

protected:
    DGNPLATFORM_EXPORT void _CopyFrom(Dgn::DgnElementCR source, CopyFromOptions const&) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT void _ToJson(BeJsValue out, BeJsConst opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(BeJsConst props) override;

public:
    BE_JSON_NAME(name);
    BE_JSON_NAME(description);

    //! Constructor
    explicit EmbeddedFileLink(CreateParams const& params) : T_Super(params), m_name(params.m_name), m_description(params.m_description) {}

    //! Create a UrlLink
    static EmbeddedFileLinkPtr Create(CreateParams const& params) { return new EmbeddedFileLink(params); }

    //! Insert the EmbeddedFileLink in the DgnDb
    DGNPLATFORM_EXPORT EmbeddedFileLinkCPtr Insert();

    //! Update the persistent state of the EmbeddedFileLink in the DgnDb from this modified copy of it.
    DGNPLATFORM_EXPORT EmbeddedFileLinkCPtr Update();

    //! Query the EmbeddedFileLink-s by it's properties
    //! @param[in] dgndb DgnDb
    //! @param[in] name Name used to embed the file (optional)
    //! @param[in] label Label of the link (optional)
    //! @param[in] description Description of the link (optional)
    //! @param[in] limitCount Limits the count of the returned set (optional - if <=0, no limit is set)
    //! @remarks If any of the parameters are nullptr-s, they are omitted from the query. If they are empty strings, the query is made for empty/null values
    DGNPLATFORM_EXPORT static DgnElementIdSet Query(DgnDbCR dgndb, Utf8CP name = nullptr, Utf8CP label = nullptr, Utf8CP description = nullptr, int limitCount = -1);

    //! Set the name used to embed the file
    //! @see DbEmbeddedFileTable for details on embedding file in a DgnDb
    void SetName(Utf8CP name) { m_name.AssignOrClear(name); }

    //! Get the name used to embed the file
    //! @see DbEmbeddedFileTable for details on embedding file in a DgnDb
    Utf8CP GetName() const { return m_name.c_str(); }

    //! Set the description the link points to
    void SetDescription(Utf8CP description) { m_description.AssignOrClear(description); }

    //! Get the description the link points to
    Utf8CP GetDescription() const { return m_description.c_str(); }

    //! Get the schema name for the UrlLink class
    //! @note This is a static method that always returns the schema name of the dgn.UrlLink class - it does @em not return the schema of a specific instance.
    static Utf8CP MyECSchemaName() { return BIS_ECSCHEMA_NAME; }
};

namespace dgn_ModelHandler
{
    //! The ModelHandler for LinkModel
    struct EXPORT_VTABLE_ATTRIBUTE Link : Information
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_LinkModel, LinkModel, Link, Information, DGNPLATFORM_EXPORT)
    };
}

namespace dgn_ElementHandler
{
    //! The handler for LinkPartition
    struct LinkPartitionHandler : InformationPartition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_LinkPartition, LinkPartition, LinkPartitionHandler, InformationPartition, DGNPLATFORM_EXPORT)
    };

    //! The handler for UrlLink elements
    struct EXPORT_VTABLE_ATTRIBUTE UrlLinkHandler : InformationContent
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_UrlLink, UrlLink, UrlLinkHandler, InformationContent, DGNPLATFORM_EXPORT)
        DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };

    //! The handler for EmbeddedFileLink elements
    struct EXPORT_VTABLE_ATTRIBUTE EmbeddedFileLinkHandler : InformationContent
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_EmbeddedFileLink, EmbeddedFileLink, EmbeddedFileLinkHandler, InformationContent, DGNPLATFORM_EXPORT)
        DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };

    //! The handler for RepositoryLink elements
    struct EXPORT_VTABLE_ATTRIBUTE RepositoryLinkHandler : UrlLinkHandler
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_RepositoryLink, RepositoryLink, RepositoryLinkHandler, UrlLinkHandler, DGNPLATFORM_EXPORT)
        DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };

    struct EXPORT_VTABLE_ATTRIBUTE SynchronizationConfigLinkHandler : UrlLinkHandler
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_SynchronizationConfigLink, SynchronizationConfigLink, SynchronizationConfigLinkHandler, UrlLinkHandler, DGNPLATFORM_EXPORT)
    };

}

END_BENTLEY_DGN_NAMESPACE
