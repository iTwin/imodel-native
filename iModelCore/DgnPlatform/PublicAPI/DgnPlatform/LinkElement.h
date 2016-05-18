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
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/ECSqlStatementIterator.h>

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
protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element) override;
public:
    explicit LinkModel(CreateParams const& params) : T_Super(params) {}

    static LinkModelPtr Create(CreateParams const& params) { return new LinkModel(params); }
};

//=======================================================================================
//! LinkElement
//=======================================================================================
struct LinkElement : InformationElement
{
    DEFINE_T_SUPER(InformationElement)

protected:
    //! Constructor
    explicit LinkElement(CreateParams const& params) : T_Super(params) {}

    //! Called when an element is about to be inserted into the DgnDb.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert() override;

public:
    //! Add the link from a source element
    //! @param[in] sourceElementId Id of the host element. 
    //! @remarks The source may contain one or more links. It's also possible that there may be links that aren't in any source element at all
    DGNPLATFORM_EXPORT BentleyStatus AddToSource(DgnElementId sourceElementId) const;

    //! Removes this link from the hosted element
    //! @param[in] sourceElementId Id of the host element. If invalid id is passed in, removes the specified link from all hosts.
    //! @remarks The removed links are NOT deleted. Use @ref DgnElement::Delete to delete the link. 
    DGNPLATFORM_EXPORT BentleyStatus RemoveFromSource(DgnElementId sourceElementId) const;

    //! Query all the host elements that contain this link
    DGNPLATFORM_EXPORT DgnElementIdSet QuerySources();
};

//=======================================================================================
//! LinkElementBase
//=======================================================================================
template<class LINK_SUBTYPE>
struct ILinkElementBase
    {
    static DgnElementIdSet CollectElementIds(BeSQLite::EC::ECSqlStatement& stmt)
        {
        DgnElementIdSet idSet;
        BeSQLite::DbResult result;
        while ((result = stmt.Step()) == BeSQLite::DbResult::BE_SQLITE_ROW)
            idSet.insert(stmt.GetValueId<DgnElementId>(0));
        return idSet;
        }

    //! Query all the links in the specified source element
    static DgnElementIdSet QueryBySource(DgnDbR dgnDb, DgnElementId sourceElementId)
        {
        BeAssert(sourceElementId.IsValid());

        Utf8CP ecSqlFmt = "SELECT " LINK_ECSQL_PREFIX ".ECInstanceId FROM %s.%s " LINK_ECSQL_PREFIX " " \
            "JOIN " DGN_SCHEMA(DGN_CLASSNAME_Element) " " SOURCE_ECSQL_PREFIX " USING " DGN_SCHEMA(DGN_RELNAME_ElementHasLinks) " " \
            "WHERE " SOURCE_ECSQL_PREFIX ".ECInstanceId=?";

        Utf8PrintfString ecSql(ecSqlFmt, LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyECClassName());

        BeSQLite::EC::CachedECSqlStatementPtr stmt = dgnDb.GetPreparedECSqlStatement(ecSql.c_str());
        BeAssert(stmt.IsValid());

        stmt->BindId(1, sourceElementId);

        return CollectElementIds(*stmt);
        }

    //! Query links by where clause
    //! @param dgnDb DgnDb
    //! @param whereClause Optional where clause. e.g., LINK_ECSQL_PREFIX ".Label LIKE 'MyLabel%'"
    static DgnElementIdSet Query(DgnDbR dgnDb, Utf8CP whereClause = nullptr)
        {
        Utf8PrintfString ecSql("SELECT " LINK_ECSQL_PREFIX ".ECInstanceId FROM %s.%s " LINK_ECSQL_PREFIX, LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyECClassName());

        if (whereClause)
            {
            ecSql.append(" WHERE ");
            ecSql.append(whereClause);
            }
            
        BeSQLite::EC::CachedECSqlStatementPtr stmt = dgnDb.GetPreparedECSqlStatement(ecSql.c_str());
        BeAssert(stmt.IsValid());

        return CollectElementIds(*stmt);
        }

    //! Query all the links in the specified link model
    static DgnElementIdSet QueryByModel(DgnDbR dgnDb, DgnModelId linkModelId)
        {
        BeAssert(linkModelId.IsValid());

        Utf8PrintfString ecSql("SELECT " LINK_ECSQL_PREFIX ".ECInstanceId FROM %s.%s " LINK_ECSQL_PREFIX, LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyECClassName());

        ecSql.append(" WHERE " LINK_ECSQL_PREFIX ".ModelId=?");

        BeSQLite::EC::CachedECSqlStatementPtr stmt = dgnDb.GetPreparedECSqlStatement(ecSql.c_str());
        BeAssert(stmt.IsValid());

        stmt->BindId(1, linkModelId);

        return CollectElementIds(*stmt);
        }

    //! Removes all links from the specified source
    //! @remarks The removed links are NOT deleted. Use @ref DgnElement::Delete to delete the link. 
    static BentleyStatus RemoveAllFromSource(DgnDbR dgnDb, DgnElementId sourceElementId)
        {
        BeAssert(sourceElementId.IsValid());

        // Note: We have to work around the ECSql limitation of not allowing JOIN clauses with DELETE statements. 

        DgnElementIdSet removeLinkIds = QueryBySource(dgnDb, sourceElementId);

        Utf8CP ecSqlFmt = "DELETE FROM ONLY " DGN_SCHEMA(DGN_RELNAME_ElementHasLinks) " WHERE InVirtualSet(?, TargetECInstanceId)";
        Utf8PrintfString ecSql(ecSqlFmt, LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyECClassName());

        BeSQLite::EC::CachedECSqlStatementPtr stmt = dgnDb.GetPreparedECSqlStatement(ecSql.c_str());
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

    //! Deletes all links that do not have a source specified
    static BentleyStatus PurgeUnused(DgnDbR dgnDb)
        {
        // Note: We have to work around the ECSql limitation of not allowing JOIN clauses with DELETE statements. 

        Utf8CP ecSqlFmt = "SELECT link.ECInstanceId FROM ONLY %s.%s link WHERE link.ECInstanceId NOT IN (SELECT TargetECInstanceId FROM " DGN_SCHEMA(DGN_RELNAME_ElementHasLinks) ")";
        Utf8PrintfString ecSql(ecSqlFmt, LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyECClassName());
        
        BeSQLite::EC::CachedECSqlStatementPtr stmt = dgnDb.GetPreparedECSqlStatement(ecSql.c_str());
        BeAssert(stmt.IsValid());

        DgnElementIdSet unusedIds = CollectElementIds(*stmt);
        if (unusedIds.empty())
            return SUCCESS;

        Utf8CP ecSqlDelFmt = "DELETE FROM ONLY %s.%s WHERE InVirtualSet(?, ECInstanceId)";
        Utf8PrintfString ecSqlDel(ecSqlDelFmt, LINK_SUBTYPE::MyECSchemaName(), LINK_SUBTYPE::MyECClassName());

        stmt = dgnDb.GetPreparedECSqlStatement(ecSqlDel.c_str());
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

    //! Get a read only copy of the UrlLink from the DgnDb
    //! @return Invalid if the UrlLink does not exist
    static UrlLinkCPtr Get(Dgn::DgnDbCR dgnDb, Dgn::DgnElementId linkElementId) { return dgnDb.Elements().Get<UrlLink>(linkElementId); }

    //! Get an editable copy of the UrlLink from the DgnDb
    //! @return Invalid if the UrlLink does not exist, or if it cannot be edited.
    static UrlLinkPtr GetForEdit(Dgn::DgnDbR dgnDb, Dgn::DgnElementId linkElementId) { return dgnDb.Elements().GetForEdit<UrlLink>(linkElementId); }

    //! Update the persistent state of the UrlLink in the DgnDb from this modified copy of it. 
    DGNPLATFORM_EXPORT UrlLinkCPtr Update();

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
    static Utf8CP MyECSchemaName() { return DGN_ECSCHEMA_NAME; }

    //! Query the DgnClassId of the UrlLink ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the dgn.UrlLink class - it does @em not return the class of a specific instance.
    DGNPLATFORM_EXPORT static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb);
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

    explicit CreateParams(Dgn::DgnElement::CreateParams const& params, Utf8CP name = nullptr) : T_Super(params)
        {
        m_name.AssignOrClear(name); 
        }

    //! Constructor
    //! @param[in] linkModel Model that should contain the link
    //! @param[in] name Name used to create the embedded file. 
    //! @see DbEmbeddedFileTable for details on embedding file in a DgnDb
    DGNPLATFORM_EXPORT CreateParams(LinkModelR linkModel, Utf8CP name);
    };

private:
    Utf8String m_name;

    static void AddClassParams(ECSqlClassParamsR params);
    Dgn::DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& statement);

protected:
    DGNPLATFORM_EXPORT virtual void _CopyFrom(Dgn::DgnElementCR source) override;
    DGNPLATFORM_EXPORT virtual Dgn::DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT virtual Dgn::DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement) override;
    DGNPLATFORM_EXPORT virtual Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;

public:
    //! Constructor
    explicit EmbeddedFileLink(CreateParams const& params) : T_Super(params), m_name(params.m_name) {}

    //! Create a UrlLink
    static EmbeddedFileLinkPtr Create(CreateParams const& params) { return new EmbeddedFileLink(params); }

    //! Insert the EmbeddedFileLink in the DgnDb
    DGNPLATFORM_EXPORT EmbeddedFileLinkCPtr Insert();

    //! Get a read only copy of the EmbeddedFileLink from the DgnDb
    //! @return Invalid if the EmbeddedFileLink does not exist
    static EmbeddedFileLinkCPtr Get(Dgn::DgnDbCR dgnDb, Dgn::DgnElementId linkElementId) { return dgnDb.Elements().Get<EmbeddedFileLink>(linkElementId); }

    //! Get an editable copy of the EmbeddedFileLink from the DgnDb
    //! @return Invalid if the EmbeddedFileLink does not exist, or if it cannot be edited.
    static EmbeddedFileLinkPtr GetForEdit(Dgn::DgnDbR dgnDb, Dgn::DgnElementId linkElementId) { return dgnDb.Elements().GetForEdit<EmbeddedFileLink>(linkElementId); }

    //! Update the persistent state of the EmbeddedFileLink in the DgnDb from this modified copy of it. 
    DGNPLATFORM_EXPORT EmbeddedFileLinkCPtr Update();

    //! Set the name used to embed the file
    //! @see DbEmbeddedFileTable for details on embedding file in a DgnDb
    void SetName(Utf8CP name) { m_name.AssignOrClear(name); }

    //! Get the name used to embed the file
    //! @see DbEmbeddedFileTable for details on embedding file in a DgnDb
    Utf8CP GetName() const { return m_name.c_str(); }

    //! Get the schema name for the UrlLink class
    //! @note This is a static method that always returns the schema name of the dgn.UrlLink class - it does @em not return the schema of a specific instance.
    static Utf8CP MyECSchemaName() { return DGN_ECSCHEMA_NAME; }

    //! Query the DgnClassId of the EmbeddedFileLink ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the dgn.EmbeddedFileLink class - it does @em not return the class of a specific instance.
    DGNPLATFORM_EXPORT static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb);
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

