/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/LinkElement.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/BeAssert.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/DgnModel.h>
#include <DgnPlatform/ElementHandler.h>

#define DGN_CLASSNAME_LinkElement           "LinkElement"
#define DGN_CLASSNAME_LinkModel             "LinkModel"
#define DGN_CLASSNAME_UrlLink               "UrlLink"
#define DGN_CLASSNAME_EmbeddedFileLink      "EmbeddedFileLink"
#define DGN_RELNAME_ElementHasLinks         "ElementHasLinks"

#define LINK_ECSQL_PREFIX "link"
#define SOURCE_ECSQL_PREFIX "source"

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_ElementHandler { struct UrlLinkHandler; struct EmbeddedFileLinkHandler; }

//=======================================================================================
//! A model which contains only links - LinkElement-s and classes derived from it.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                  Ramanujam.Raman   05/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinkModel : InformationModel
{
    DGNMODEL_DECLARE_MEMBERS(DGN_CLASSNAME_LinkModel, InformationModel);

public:
    struct CreateParams : Dgn::InformationModel::CreateParams
    {
    DEFINE_T_SUPER(Dgn::InformationModel::CreateParams);

    protected:
        CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnClassId classId, Dgn::DgnCode code, Utf8CP label = nullptr, bool inGuiList = true)
            : T_Super(dgndb, classId, code, label, inGuiList)
            {}

    public:
        //! @private
        //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
        CreateParams(Dgn::DgnModel::CreateParams const& params) : T_Super(params) {}

        //! Parameters to create a new instance of a LinkModel.
        //! @param[in] dgndb The DgnDb for the new DgnModel
        //! @param[in] code The Code for the DgnModel
        //! @param[in] label Label of the new DgnModel
        //! @param[in] inGuiList Controls the visibility of the new DgnModel in model lists shown to the user
        CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnCode code, Utf8CP label = nullptr, bool inGuiList = true) :
            T_Super(dgndb, LinkModel::QueryClassId(dgndb), code, label, inGuiList)
            {}
    };

protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element) override;
public:
    explicit LinkModel(CreateParams const& params) : T_Super(params) {}

    static LinkModelPtr Create(CreateParams const& params) { return new LinkModel(params); }

    //! Gets the LinkModel by Id. If the model is not loaded, it loads it, but does not fill it with contained elements. 
    static LinkModelPtr Get(Dgn::DgnDbCR dgndb, DgnModelId id) { return dgndb.Models().Get<LinkModel>(id); }

    //! Query the DgnClassId of the dgn.LinkModel ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the dgn.LinkModel class - it does @em not return the class of a specific instance.
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, DGN_CLASSNAME_LinkModel)); }
};

//=======================================================================================
//! LinkElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinkElement : InformationElement
{
    DEFINE_T_SUPER(InformationElement)

protected:
    //! Constructor
    explicit LinkElement(CreateParams const& params) : T_Super(params) {}

    //! Called when an element is about to be inserted into the DgnDb.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert() override;

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
            "JOIN " DGN_SCHEMA(DGN_CLASSNAME_Element) " " SOURCE_ECSQL_PREFIX " USING " DGN_SCHEMA(DGN_RELNAME_ElementHasLinks) " " \
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

        ecSql.append(" WHERE " LINK_ECSQL_PREFIX ".ModelId=?");

        BeSQLite::EC::CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(ecSql.c_str());
        BeAssert(stmt.IsValid());

        stmt->BindId(1, linkModelId);

        return CollectElementIds(*stmt);
        }

    //! Removes all links from the specified source
    //! @remarks The removed links are NOT deleted. Use @ref DgnElement::Delete to delete the link. 
    static BentleyStatus RemoveAllFromSource(DgnDbR dgndb, DgnElementId sourceElementId)
        {
        BeAssert(sourceElementId.IsValid());

        // Note: We have to work around the ECSql limitation of not allowing JOIN clauses with DELETE statements. 

        DgnElementIdSet removeLinkIds = QueryBySource(dgndb, sourceElementId);

        Utf8CP ecSqlFmt = "DELETE FROM ONLY " DGN_SCHEMA(DGN_RELNAME_ElementHasLinks) " WHERE InVirtualSet(?, TargetECInstanceId)";
        Utf8PrintfString ecSql(ecSqlFmt, LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyHandlerECClassName());

        BeSQLite::EC::CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(ecSql.c_str());
        BeAssert(stmt.IsValid());

        stmt->BindInt64(1, (int64_t) &removeLinkIds);

        BeSQLite::DbResult stepStatus = stmt->Step();
        if (BeSQLite::DbResult::BE_SQLITE_DONE != stepStatus)
            {
            BeAssert(false);
            return ERROR;
            }

        return SUCCESS;
        }

    //! Finds all links that do not have a source specified
    static DgnElementIdSet FindOrphaned(DgnDbCR dgndb)
        {
        Utf8CP ecSqlFmt = "SELECT link.ECInstanceId FROM ONLY %s.%s link WHERE link.ECInstanceId NOT IN (SELECT TargetECInstanceId FROM " DGN_SCHEMA(DGN_RELNAME_ElementHasLinks) ")";
        Utf8PrintfString ecSql(ecSqlFmt, LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyHandlerECClassName());

        BeSQLite::EC::CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(ecSql.c_str());
        BeAssert(stmt.IsValid());

        return CollectElementIds(*stmt);
        }

    //! Deletes all links that do not have a source specified
    static BentleyStatus PurgeOrphaned(DgnDbCR dgndb)
        {
        // Note: We have to work around the ECSql limitation of not allowing JOIN clauses with DELETE statements. 

        DgnElementIdSet unusedIds = FindOrphaned(dgndb);
        if (unusedIds.empty())
            return SUCCESS;

        Utf8CP ecSqlFmt = "DELETE FROM ONLY %s.%s WHERE InVirtualSet(?, ECInstanceId)";
        Utf8PrintfString ecSql(ecSqlFmt, LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyHandlerECClassName());

        BeSQLite::EC::CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(ecSql.c_str());
        BeAssert(stmt.IsValid());

        stmt->BindInt64(1, (int64_t) &unusedIds);

        BeSQLite::DbResult stepStatus = stmt->Step();
        if (BeSQLite::DbResult::BE_SQLITE_DONE != stepStatus)
            {
            BeAssert(false);
            return ERROR;
            }

        return SUCCESS;
        }

    //! Query the DgnClassId of the LinkElement ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the LinkElement class - it does @em not return the class of a specific instance.
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb)
        {
        return dgndb.Schemas().GetECClassId(LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyHandlerECClassName());
        }
    };

//=======================================================================================
//! An element containing an URL link
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE UrlLink : LinkElement, ILinkElementBase<UrlLink>
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_UrlLink, LinkElement)
    friend struct dgn_ElementHandler::UrlLinkHandler;

private:
    Utf8String m_url;
    Utf8String m_description;

    static void AddClassParams(ECSqlClassParamsR params);
    Dgn::DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& statement);

protected:
    DGNPLATFORM_EXPORT virtual void _CopyFrom(Dgn::DgnElementCR source) override;
    DGNPLATFORM_EXPORT virtual Dgn::DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT virtual Dgn::DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement) override;
    DGNPLATFORM_EXPORT virtual Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;

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
    DGNPLATFORM_EXPORT explicit CreateParams(LinkModelR linkModel, Utf8CP url = nullptr, Utf8CP label = nullptr, Utf8CP description = nullptr);
    };

    //! Constructor
    explicit UrlLink(CreateParams const& params) : T_Super(params), m_url(params.m_url), m_description(params.m_description) {}

    //! Create an UrlLink
    static UrlLinkPtr Create(CreateParams const& params) { return new UrlLink(params); }

    //! Insert the UrlLink in the DgnDb
    DGNPLATFORM_EXPORT UrlLinkCPtr Insert();

    //! Update the persistent state of the UrlLink in the DgnDb from this modified copy of it. 
    DGNPLATFORM_EXPORT UrlLinkCPtr Update();

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
//! An element containing a link to an file embedded in the Db.
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EmbeddedFileLink : LinkElement, ILinkElementBase<EmbeddedFileLink>
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_EmbeddedFileLink, LinkElement)
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

    static void AddClassParams(ECSqlClassParamsR params);
    Dgn::DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& statement);

protected:
    DGNPLATFORM_EXPORT virtual void _CopyFrom(Dgn::DgnElementCR source) override;
    DGNPLATFORM_EXPORT virtual Dgn::DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT virtual Dgn::DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement) override;
    DGNPLATFORM_EXPORT virtual Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;

public:
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
struct EXPORT_VTABLE_ATTRIBUTE Link : Model
{
    MODELHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_LinkModel, LinkModel, Link, Model, DGNPLATFORM_EXPORT)
};

}

namespace dgn_ElementHandler
{
//! The handler for UrlLink elements
struct EXPORT_VTABLE_ATTRIBUTE UrlLinkHandler : Element
{
    ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_UrlLink, UrlLink, UrlLinkHandler, Element, DGNPLATFORM_EXPORT)
    virtual void _GetClassParams(ECSqlClassParamsR params) override { T_Super::_GetClassParams(params); UrlLink::AddClassParams(params); }
};

//! The handler for EmbeddedFileLink elements
struct EXPORT_VTABLE_ATTRIBUTE EmbeddedFileLinkHandler : Element
{
    ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_EmbeddedFileLink, EmbeddedFileLink, EmbeddedFileLinkHandler, Element, DGNPLATFORM_EXPORT)
    virtual void _GetClassParams(ECSqlClassParamsR params) override { T_Super::_GetClassParams(params); EmbeddedFileLink::AddClassParams(params); }
};

}

END_BENTLEY_DGN_NAMESPACE

