/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnFileIO/DgnFileDb.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

USING_NAMESPACE_BENTLEY_SQLITE

enum
    {
    DB_ElementSignature00 = 0x00, // pre-Graphite06
    DB_ElementSignature01 = 0x01, // Graphite06 and forward
    };

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   11/10
//=======================================================================================
struct DbElementWriter
{
protected:
    ElementListHandler* m_table;
    UInt32    m_sequence;
    DgnModelP m_model;
    DgnProjectR m_project;
    BeSQLite::SnappyToBlob m_zipper;
    BeSQLite::CachedStatementPtr m_stmt;

    void ExecStatement();
    void PrepareInsertStatement();
    void PrepareUpdateStatement();
    void ZipDataAndSubElements (ElementRefP elRef, ByteCP elementData, UInt32 elementSize);
    void ZipElementRef (ElementRefP elRef);
    void SaveElementToRow (PersistentElementRefP el, bool isInsert);

public:
    DbElementWriter (DgnProjectR project, ElementListHandler& table) : m_project(project), m_table(&table)  {m_sequence = 0;}

    void SetSequence(int seq) {m_sequence = seq;}
    void InsertElement (DgnModelR, PersistentElementRefP);
    void UpdateElement (DgnModelR, PersistentElementRefP);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/11
//=======================================================================================
struct DbException
    {
    DgnFileStatus m_status;
    int           m_result;
    DbException (DgnFileStatus s, int r){m_status = s; m_result=r;}
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   11/10
//=======================================================================================
struct ElementBlobHeader
{
    UInt32  m_signature;    // write this so we can detect errors on read
    UInt32  m_bytes;
    UInt32  m_attrOff;
    UInt32  m_handlerId;
    ElementBlobHeader (ElementRefP elRef);
    ElementBlobHeader (BeSQLite::SnappyReader& in);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ElementBlobHeader::ElementBlobHeader (ElementRefP elRef)
    {
    m_signature = DB_ElementSignature01;
    m_handlerId = elRef->GetHandler()->GetHandlerId().GetId();

    DgnElementCP el = elRef->GetUnstableMSElementCP();
    m_bytes   = el->Size();
    m_attrOff = el->GetAttributeOffset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ElementBlobHeader::ElementBlobHeader (SnappyReader& in) {UInt32 actuallyRead; in._Read ((byte*) this, sizeof(*this), actuallyRead);}

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/11
//=======================================================================================
struct ControlElementRefList : PersistentElementRefList
{
    DEFINE_T_SUPER(PersistentElementRefList)
    ControlElementRefList(ElementListHandlerR handler, DgnModelP model) : PersistentElementRefList(handler, model) {}
};

//=======================================================================================
//! @bsiclass                                                     KeithBentley    12/00
//=======================================================================================
struct DictionaryElementRefList : PersistentElementRefList
{
    DEFINE_T_SUPER(PersistentElementRefList)
    DictionaryElementRefList(ElementListHandlerR handler, DgnModelP model) : PersistentElementRefList(handler, model) {}
};

//=======================================================================================
//! @bsiclass                                                     KeithBentley    12/00
//=======================================================================================
struct PhysicalElementRefList : GraphicElementRefList
{
    DEFINE_T_SUPER(GraphicElementRefList)
    PhysicalElementRefList(ElementListHandlerR handler, DgnModelP model) : GraphicElementRefList (handler, model) {}
};

//=======================================================================================
//! @bsiclass                                                     KeithBentley    12/00
//=======================================================================================
struct DrawingElementRefList : public GraphicElementRefList
{
    UInt32 m_nextSequence;
    DrawingElementRefList (ElementListHandlerR handler, DgnModelP model) : GraphicElementRefList (handler, model){m_nextSequence = 0;}
    DEFINE_T_SUPER(GraphicElementRefList)
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/11
//=======================================================================================
struct ElementTableHandler : TxnTableHandler
{
private:
    void AddLifecycleChange (TxnSummary::TxnLifecycleSet& elSet, BeSQLite::Changes::Change const& change, bool useNew) const;
    virtual Utf8CP _GetTableName() const override {return DGN_TABLE_Element;}
    virtual void _OnAdd(TxnSummary& summary, BeSQLite::Changes::Change const& change) const override    {AddLifecycleChange (summary.m_added, change, true);}
    virtual void _OnDelete(TxnSummary& summary, BeSQLite::Changes::Change const& change) const override {AddLifecycleChange (summary.m_deleted, change, false);}
    virtual void _OnUpdate(TxnSummary& summary, BeSQLite::Changes::Change const& change) const override;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/11
//=======================================================================================
struct XAttTableHandler : TxnTableHandler
{
private:
    void AddChange (TxnSummary::TxnXAttrSet& elSet, BeSQLite::Changes::Change const& change, bool useNew) const;
    virtual Utf8CP _GetTableName() const override {return DGN_TABLE_ElmXAtt;}
    virtual void _OnAdd(TxnSummary& summary, BeSQLite::Changes::Change const& change) const override    {AddChange (summary.m_xattAdded, change, true);}
    virtual void _OnDelete(TxnSummary& summary, BeSQLite::Changes::Change const& change) const override {AddChange (summary.m_xattDeleted, change, false);}
    virtual void _OnUpdate(TxnSummary& summary, BeSQLite::Changes::Change const& change) const override {AddChange (summary.m_xattModified, change, false);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/13
//=======================================================================================
struct GraphicListHandler : ElementListHandler
{
    virtual UInt32 _BindGraphicColumns(BeSQLiteStatementP stmt, DgnElementCR elData) const override
        {
        DRange3dCR range = elData.GetRange();
        bool is3d = elData.Is3d();

        // if the element has no range, store NULLs
        if (!range.IsNull())
            {
            DRange3dCR range = elData.GetRange();
            BeAssert (range.low.x<= range.high.x);
            BeAssert (range.low.y<= range.high.y);
            stmt->BindDouble(4, range.low.x);
            stmt->BindDouble(5, range.high.x);
            stmt->BindDouble(6, range.low.y);
            stmt->BindDouble(7, range.high.y);
            if (is3d)
                {
                BeAssert (range.low.z<= range.high.z);
                stmt->BindDouble(8, range.low.z);
                stmt->BindDouble(9, range.high.z);
                }
            }

        stmt->BindInt(10, elData.GetProperties());
        stmt->BindBlob(11, &elData.GetSymbology(), sizeof(Symbology), Statement::MAKE_COPY_No);
        if (!is3d)
            stmt->BindInt(12, elData.GetDisplayPriority());

        return  sizeof(DgnElement);
        }
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/11
//=======================================================================================
struct Element3dListHandler : GraphicListHandler
{
    virtual UInt32 _GetHeaderSize() const override {return sizeof(DgnElement);}
    virtual void _LoadGraphicColumns(BeSQLiteStatementR stmt, DgnElementP elData) const override
        {
        elData->SetProperties((Int16) stmt.GetValueInt(11));

        enum {FirstCol=5};
        DRange3dR range = elData->GetRangeR();
        if (stmt.IsColumnNull(FirstCol+0))
            {
            range = DRange3d::NullRange();
            }
        else
            {
            range.low.x  = stmt.GetValueDouble(FirstCol+0);
            range.high.x = stmt.GetValueDouble(FirstCol+1);
            range.low.y  = stmt.GetValueDouble(FirstCol+2);
            range.high.y = stmt.GetValueDouble(FirstCol+3);
            range.low.z  = stmt.GetValueDouble(FirstCol+4);
            range.high.z = stmt.GetValueDouble(FirstCol+5);
            }

        memcpy (&elData->GetSymbologyR(), stmt.GetValueBlob(12), sizeof(Symbology));
        }
    virtual bool _VerifyDimensionality (DgnElementCR el) override {return el.Is3d();}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/11
//=======================================================================================
struct PhysicalListHandler : Element3dListHandler
{
    DEFINE_T_SUPER(Element3dListHandler)
    virtual PersistentElementRefList* _CreateList(DgnModelP dgnModel) {return new PhysicalElementRefList(*this, dgnModel);}
    virtual ElementOwnerType _GetElementOwnerType() const override {return OWNER_Physical;}
    virtual DgnModelStatus _VerifyElemDescr (MSElementDescrR elemDescr) override;
};

//=======================================================================================
// @bsiclass   .                                                 Keith.Bentley   07/11
//=======================================================================================
struct Element2dListHandler : GraphicListHandler
{
    DEFINE_T_SUPER(GraphicListHandler)
    virtual PersistentElementRefList* _CreateList(DgnModelP dgnModel) {return new DrawingElementRefList(*this, dgnModel);}
    virtual ElementOwnerType _GetElementOwnerType() const override {return OWNER_Drawing;}

    virtual UInt32 _GetHeaderSize() const override {return sizeof(DgnElement);}
    virtual void _LoadGraphicColumns(BeSQLiteStatementR stmt, DgnElementP elData) const override
        {
        enum {FirstCol=5};
        DRange3dR range = elData->GetRangeR();
        if (stmt.IsColumnNull(FirstCol+0))
            {
            range = DRange3d::NullRange();
            }
        else
            {
            range.low.x  = stmt.GetValueDouble(FirstCol+0);
            range.high.x = stmt.GetValueDouble(FirstCol+1);
            range.low.y  = stmt.GetValueDouble(FirstCol+2);
            range.high.y = stmt.GetValueDouble(FirstCol+3);
            range.low.z = range.high.z = 0.0;
            }

        elData->SetProperties((Int16) stmt.GetValueInt(11));
        memcpy (&elData->GetSymbologyR(), stmt.GetValueBlob(12), sizeof(Symbology));
        elData->SetDisplayPriority(stmt.GetValueInt (13));
        }

    virtual bool _VerifyDimensionality (DgnElementCR el) override {return !el.Is3d();}
    virtual DgnModelStatus _VerifyElemDescr (MSElementDescrR elemDescr) override;
};


static PhysicalListHandler   s_physicalListHandler;
static Element2dListHandler    s_element2dListHandler;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProject::InitializeTableHandlers()
    {
    static ElementTableHandler  s_elementTableHandler;
    static XAttTableHandler     s_xattTableHandler;
    ITxnManager::AddTableHandler (s_elementTableHandler);
    ITxnManager::AddTableHandler (s_xattTableHandler);
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/12
//=======================================================================================
struct DbElementListReader : DbElementReader
{
protected:
    PersistentElementRefListR m_list;
    DgnModelP                 m_dgnModel;
    virtual DgnModelP _GetDgnModelForElement() override {return m_dgnModel;}
    virtual void _OnElementSelected (PersistentElementRefR, bool wasLoaded) override;

public:
    DbElementListReader (PersistentElementRefListR list) : DbElementReader(list.GetDgnProject()), m_list(list), m_dgnModel(list.GetDgnModelP()) {}
    UInt32 ReadList(bool graphic);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/12
//=======================================================================================
struct DbElementReloader : DbElementReader
{
private:
    PersistentElementRefR m_elRef;
public:
    DbElementReloader (PersistentElementRefR elRef) : DbElementReader(*elRef.GetDgnProject()), m_elRef(elRef) {}
    DgnModelStatus ReloadElement();
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/12
//=======================================================================================
struct DbElementLoader : DbElementReader
{
private:
    ElementId  m_id;
    virtual PersistentElementRefP _FindExistingElementRef (ElementId id) override {return NULL;}

public:
    DbElementLoader (DgnProjectCR project, ElementId id) : DbElementReader(project), m_id(id) {}
    PersistentElementRefP LoadElement();
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isElemRangeInvalid (MSElementDescrP descr, bool is3d)
    {
    if (descr->Element().Size() < sizeof (DgnElement))
        return true;

    DRange3dR range = descr->ElementR().GetRangeR();
    if (!is3d)
        range.low.z = range.high.z = 0.;

    return (range.low.x>range.high.x || range.low.y>range.high.y || range.low.z>range.high.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus PhysicalListHandler::_VerifyElemDescr (MSElementDescrR descr)
    {
    return isElemRangeInvalid(&descr, true) ? DGNMODEL_STATUS_InvalidRange : T_Super::_VerifyElemDescr(descr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus Element2dListHandler::_VerifyElemDescr (MSElementDescrR descr)
    {
    return isElemRangeInvalid(&descr, false) ? DGNMODEL_STATUS_InvalidRange : T_Super::_VerifyElemDescr(descr);
    }

ElementListHandlerR PhysicalModel::_GetGraphicElementHandler() const {return s_physicalListHandler;}
ElementListHandlerR DgnModel2d::_GetGraphicElementHandler() const {return s_element2dListHandler;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DbElementReader::GetSelectElementSql()
    {
    return "SELECT "
                      "Id,"              // 0
                      "ItemId,"          // 1
                      "HandlerId,"       // 2
                      "ModelId,"         // 3
                      "LevelId,"         // 4
                      "[Min.X],"         // 5
                      "[Max.X],"         // 6
                      "[Min.Y],"         // 7
                      "[Max.Y],"         // 8
                      "[Min.Z],"         // 9
                      "[Max.Z],"         // 10
                      "Props,"           // 11
                      "Symb,"            // 12
                      "Priority"         // 13
           " FROM " DGN_TABLE_Element " ";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnModels::IsElementIdUsed (ElementId id) const
    {
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_project.GetCachedStatement (stmt, "SELECT 1 FROM " DGN_TABLE_Element " WHERE Id=?"))
        {
        BeAssert (0);
        return false;
        }

    stmt->BindInt64(1, id.GetValueUnchecked());
    return BE_SQLITE_ROW == stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
ElementId DgnModels::GetHighestElementId ()
    {
    if (!m_highestElementId.IsValid())
        {
        BeLuid nextRepo(m_project.GetRepositoryId().GetNextRepositoryId().GetValue(),0);
        Statement stmt;
        
        stmt.Prepare (m_project, "SELECT max(Id) FROM " DGN_TABLE_Element " WHERE Id<?");
        stmt.BindInt64 (1,nextRepo.GetValue());

        DbResult result = stmt.Step();
        UNUSED_VARIABLE(result);

        BeAssert (result == BE_SQLITE_ROW);
        Int64 currMax = stmt.GetValueInt64(0);

        BeRepositoryBasedId firstId (m_project.GetRepositoryId(), 0);
        m_highestElementId.m_id = (currMax < firstId.m_id) ? firstId.m_id : currMax;

        m_lowestNewElementId = m_highestElementId;
        m_lowestNewElementId.UseNext();
        }

    return  m_highestElementId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
ElementId DgnModels::GetLowestNewElementId ()
    {
    GetHighestElementId();
    return m_lowestNewElementId;
    }

/*---------------------------------------------------------------------------------**//**
 ElementIds are 64 bits, divided into two 32 bit parts {high:low}. The high 32 bits are reserved for the
 repositoryId of the creator and the low 32 bits hold the identifier of the element. This scheme is
 designed to allow multiple users on differnt computers to create new elements without
 generating conflicting ids, since the repositoryId is intended to be unqiue for every copy of the project.
 We are allowed to make ElementIds in the range of [{repositoryid:1},{repositoryid+1:0}). So, find the highest currently
 used id in that range and add 1. If none, use the first id. If the highest possible id is already in use, search for an
 available id randomly.
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
ElementId DgnModels::MakeNewElementId()
    {
    GetHighestElementId();

    // see if the next id is the highest possible Id for our repositoryId. If not, use it. Otherwise try random ids until we find one that's
    // not currently in use.
    BeRepositoryBasedId lastId (m_project.GetRepositoryId().GetNextRepositoryId(), 0);
    if (m_highestElementId.m_id < lastId.m_id-100) // reserve a few ids for special meaning
        {
        m_highestElementId.UseNext();
        return m_highestElementId;
        }

    // highest id already used, try looking for a random available id
    BeLuid val;
    do
        {
        val.CreateRandom();
        val.m_luid.i[1] = m_project.GetRepositoryId().GetValue();
        } while (IsElementIdUsed(ElementId(val.m_luid.u)));

    return ElementId(val.m_luid.u);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnModels::DeleteElementFromDb (ElementId elementId)
    {
    // to delete an element, we delete it from the DGNELEMENT_TABLE_Data table. Foreign keys and triggers cause all of the related
    // tables to be cleaned up.
    CachedStatementPtr deleteStmt;
    DbResult status = m_project.GetCachedStatement(deleteStmt, "DELETE FROM " DGN_TABLE_Element " WHERE Id=?");
    if (BE_SQLITE_OK != status)
        return  status;

    deleteStmt->BindId(1, elementId);
    return deleteStmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefP DbElementLoader::LoadElement()
    {
    Utf8String sql (GetSelectElementSql());
    sql.append ("WHERE Id=?");

    m_project.GetCachedStatement(m_selectStmt, sql.c_str());
    m_selectStmt->BindId(1, m_id);

    PersistentElementRefP elRef;
    bool wasLoaded;
    if (BE_SQLITE_ROW != GetOneElement(elRef, wasLoaded))
        return  NULL;

    _OnElementSelected (*elRef, wasLoaded);
    return  elRef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefPtr DgnModels::GetElementById (ElementId id) const
    {
    if (!id.IsValid())
        return  NULL;

    PersistentElementRefP elRef = m_elements.FindElementById (id);
    if (NULL == elRef)
        {
        HighPriorityOperationBlock highPriority;  //  see comments in BeSQLite.h
        DbElementLoader loader (m_project, id);
        elRef = loader.LoadElement();
        }

    return  elRef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/14
+---------------+---------------+---------------+---------------+---------------+------*/
ElementIdSet DgnModels::GetElementsForItem(DgnItemId itemId)
    {
    ElementIdSet elems;
    CachedStatementPtr stmt;

    if (BE_SQLITE_OK != m_project.GetCachedStatement (stmt, "SELECT Id FROM " DGN_TABLE_Element " WHERE ItemId=?"))
        {
        BeAssert (0);
        return elems;
        }
    stmt->BindId (2, itemId);

    while (BE_SQLITE_ROW == stmt->Step())
        elems.insert(stmt->GetValueId<ElementId>(0));

    return elems;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnModel::ChangeElementHandlerInDb (ElementId elementId, ElementHandlerId handlerId)
    {
    Statement stmt;
    DbResult status = stmt.Prepare(GetDgnProject(), "UPDATE " DGN_TABLE_Element " SET HandlerId=? WHERE Id=?");
    if (BE_SQLITE_OK != status)
        return  status;

    stmt.BindInt64(1, handlerId.GetId());
    stmt.BindId(2, elementId);
    return  stmt.Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/13
//---------------------------------------------------------------------------------------
DbResult DgnModel::ChangeElementLevelInDb (ElementId elementId, LevelId levelId)
    {
    Statement stmt;
    DbResult status = stmt.Prepare (GetDgnProject(), "UPDATE " DGN_TABLE_Element " SET LevelId=? WHERE Id=?");
    if (BE_SQLITE_OK != status)
        return  status;

    stmt.BindInt(1, levelId.GetValueUnchecked());
    stmt.BindId(2, elementId);
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DbElementWriter::PrepareInsertStatement()
    {
    Utf8CP insertSql =
            "INSERT INTO " DGN_TABLE_Element "("
                "Id,"           // 1
                "ItemId,"       // 2
                "LevelId,"      // 3
                "[Min.X],"      // 4
                "[Max.X],"      // 5
                "[Min.Y],"      // 6
                "[Max.Y],"      // 7
                "[Min.Z],"      // 8
                "[Max.Z],"      // 9
                "Props,"        // 10
                "Symb,"         // 11
                "Priority,"     // 12
                "Size,"         // 13
                "Data,"         // 14
                "HandlerId,"    // 15
                "ModelId"       // 16
            ")VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";

    DbResult status = m_project.GetCachedStatement (m_stmt, insertSql);
    if (BE_SQLITE_OK != status)
        throw DbException(DGNPROJECT_ERROR_SQLSchemaError, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DbElementWriter::PrepareUpdateStatement()
    {
    Utf8CP updateSql =
            "UPDATE " DGN_TABLE_Element " SET "
                "ItemId=?2,"
                "LevelId=?3,"
                "[Min.X]=?4,"
                "[Max.X]=?5,"
                "[Min.Y]=?6,"
                "[Max.Y]=?7,"
                "[Min.Z]=?8,"
                "[Max.Z]=?9,"
                "Props=?10,"
                "Symb=?11,"
                "Priority=?12,"
                "Size=?13,"
                "Data=?14,"
                "HandlerId=?15"
            " WHERE Id=?1";

    DbResult status = m_project.GetCachedStatement (m_stmt, updateSql);
    if (BE_SQLITE_OK != status)
        throw DbException(DGNPROJECT_ERROR_SQLSchemaError, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DbElementWriter::ExecStatement()
    {
    DbResult status = m_stmt->Step();
    if (status != BE_SQLITE_DONE)
        {
        BeAssert(false);
        throw DbException(DGNOPEN_STATUS_CorruptFile, status);
        }
    m_stmt->Reset();
    m_stmt->ClearBindings();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DbElementWriter::SaveElementToRow (PersistentElementRefP el, bool isInsert)
    {
    ZipElementRef (el);
    UInt32 zipSize = m_zipper.GetCompressedSize();
    if (0 == zipSize)
        {
        BeAssert (false);
        return;
        }

    DgnElementCP elData = el->GetUnstableMSElementCP();
    ElementId elementId = elData->GetElementId();

    m_stmt->BindId(1, elementId);
    m_stmt->BindId(2, el->GetItemId());
    m_stmt->BindInt(3, elData->GetLevelValue());
    m_stmt->BindInt(13, m_zipper.GetUnCompressedSize());
    m_stmt->BindInt(15, el->GetHandler()->GetHandlerId().GetId());

    if (isInsert)
        m_stmt->BindId (16, m_model->GetModelId());

    if (1 == m_zipper.GetCurrChunk())
        {
        m_stmt->BindBlob (14, m_zipper.GetChunkData(0), zipSize, Statement::MAKE_COPY_No);
        ExecStatement();
        return;
        }

    m_stmt->BindZeroBlob (14, zipSize);

    ExecStatement();

    StatusInt status = m_zipper.SaveToRow (m_project, DGN_TABLE_Element, "Data", elementId.GetValue());
    if (SUCCESS != status)
        {
        BeAssert(false);
        throw DbException(DGNOPEN_STATUS_CorruptFile, status);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DbElementWriter::ZipDataAndSubElements (ElementRefP elRef, ByteCP elementData, UInt32 elementSize)
    {
    m_zipper.Write (elementData, elementSize);
    elRef->ClearDirty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DbElementWriter::ZipElementRef (ElementRefP elRef)
    {
    DgnElementCP elData = elRef->GetUnstableMSElementCP();
    m_zipper.Init();

    UInt32 hdrSize = m_table->_BindGraphicColumns(m_stmt.get(), *elData);
    ElementBlobHeader header (elRef);
    m_zipper.Write ((ByteCP) &header, sizeof (header));

    BeAssert(header.m_bytes >= hdrSize);
    ZipDataAndSubElements (elRef, ((ByteCP)elData)+hdrSize, header.m_bytes-hdrSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DbElementWriter::InsertElement (DgnModelR model, PersistentElementRefP elRef)
    {
    BeAssert (elRef->CanBeSavedToFile());

    m_model = &model;
    PrepareInsertStatement();
    SaveElementToRow (elRef, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DbElementWriter::UpdateElement (DgnModelR model, PersistentElementRefP elRef)
    {
    BeAssert (elRef->CanBeSavedToFile());

    m_model = &model;
    PrepareUpdateStatement();
    SaveElementToRow (elRef, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2012
//--------------+------------------------------------------------------------------------
void DbElementReader::_LoadElement (PersistentElementRefR elRef)
    {
    ElementId elementId = elRef.GetElementId();

    if (ZIP_SUCCESS != m_snappy.Init (m_project, DGN_TABLE_Element, "Data", elementId.GetValue()))
        throw DbException (DGNOPEN_STATUS_CorruptFile, 0);

    ElementBlobHeader header (m_snappy);
    if ((DB_ElementSignature01 != header.m_signature) && (DB_ElementSignature00 != header.m_signature))
        {
        BeAssert (false);
        throw DbException(DGNOPEN_STATUS_CorruptFile, 0);
        }

    ElementListHandlerR listHandler = elRef.GetListHandler();

    DgnElementP el = elRef.ReserveMemory(header.m_bytes);
    UInt32 hdrSize = listHandler._GetHeaderSize();

    memset (el, 0, hdrSize);

    elRef.SetItemId(m_selectStmt->GetValueId<DgnItemId>(1));
    el->SetSizeWords(header.m_bytes / 2);
    el->SetAttributeOffset(header.m_attrOff);
    el->SetLevel(m_selectStmt->GetValueInt(4));
    el->SetElementId(elementId);

    listHandler._LoadGraphicColumns(*m_selectStmt, el);

    UInt32 actuallyRead;
    m_snappy._Read (((byte*)el)+hdrSize, header.m_bytes-hdrSize, actuallyRead);

    if (actuallyRead != header.m_bytes-hdrSize)
        {
        BeAssert(false);
        throw DbException(DGNOPEN_STATUS_CorruptFile, 0);
        }

    ElementHandlerId handlerId ((UInt32)m_selectStmt->GetValueInt(2));

    Handler& elHandler = m_project.Domains().ResolveHandler(handlerId);
    elRef.SetHandler (&elHandler);

    m_project.SendElementLoadedEvent (elRef); // Notify listeners of newly loaded element...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DbElementListReader::_OnElementSelected (PersistentElementRefR elRef, bool wasLoaded)
    {
    if (wasLoaded || NULL == m_list.FindElementById(elRef.GetElementId()))
        m_list._OnElementAdded(elRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP DbElementReader::_GetDgnModelForElement()
    {
    return m_project.Models().GetModelById (m_selectStmt->GetValueId<DgnModelId>(3));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefP DbElementReader::_FindExistingElementRef (ElementId id)
    {
    return m_project.Models().ElementPool().FindElementById(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbElementReader::GetOneElement(PersistentElementRefP& elRef, bool& wasLoaded)
    {
    elRef = NULL;
    wasLoaded = false;

    DbResult result = m_selectStmt->Step();
    if (BE_SQLITE_ROW != result)
        return  result;

    ElementId id = m_selectStmt->GetValueId<ElementId>(0);
    DgnElementPool& elemPool = m_project.Models().ElementPool();

    if (true)
        {
        // since we can load elements on more than one thread, we need to check that the element doesn't already exist
        // *with the lock held* before we load it. This avoids a race condition where an element is loaded on more than one thread.
        BeDbMutexHolder _v(elemPool.m_mutex);

        elRef = _FindExistingElementRef(id);
        if (NULL != elRef)
            return BE_SQLITE_ROW;

        DgnModelP dgnModel = _GetDgnModelForElement();
        if (NULL == dgnModel)
            {
            BeAssert (false);
            return BE_SQLITE_ROW; // let them keep going
            }

        elRef = elemPool.AllocateElementRef(*dgnModel, id);
        // now that the elementref exists, we can release the lock on the pool.
        }

    _LoadElement(*elRef);
    wasLoaded = true;

    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 DbElementReader::ReadElements (ICheckStopP checkStop)
    {
    UInt32 count=0;
    PersistentElementRefP elRef;
    bool wasLoaded;

    while (BE_SQLITE_ROW == GetOneElement(elRef, wasLoaded))
        {
        if (NULL != elRef)
            {
            _OnElementSelected (*elRef, wasLoaded);
            ++count;
            }

        if (checkStop && checkStop->_CheckStop())
            break;
        }

    m_snappy.Finish();
    return  count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DbElementReloader::ReloadElement()
    {
    Utf8String sql (GetSelectElementSql());
    sql.append ("WHERE Id=?");

    m_project.GetCachedStatement(m_selectStmt, sql.c_str());
    m_selectStmt->BindId(1, m_elRef.GetElementId());

    DbResult result = m_selectStmt->Step();
    if (BE_SQLITE_ROW != result)
        return  DGNMODEL_STATUS_ElementNotFound;

    try
        {
        _LoadElement(m_elRef);
        }
    catch (DbException)
        {
        return DGNMODEL_STATUS_ElementReadError;
        }

    return  DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbFileIOElementReader::DgnDbFileIOElementReader (DgnProjectCR project, DgnModelId mid) : DbElementReader (project)
    {
    Utf8String sql (GetSelectElementSql());
    sql.append (Utf8PrintfString ("WHERE ModelId=%lld", mid.GetValue()));

    m_project.GetCachedStatement(m_selectStmt, sql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefPtr DgnDbFileIOElementReader::LoadElement()
    {
    PersistentElementRefP ref;
    bool wasLoaded;
    return (BE_SQLITE_ROW == GetOneElement(ref, wasLoaded)) ? ref : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 DbElementListReader::ReadList(bool graphic)
    {
    Utf8String sql (GetSelectElementSql());
    sql.append ("WHERE ModelId=? AND Symb IS ");

    if (graphic)
        sql.append ("NOT NULL");
    else
        sql.append ("NULL");

    m_project.GetCachedStatement(m_selectStmt, sql.c_str());
    m_selectStmt->BindId(1, m_dgnModel->GetModelId());

    m_list._SetFilled();
    return ReadElements(NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus ElementListHandler::AddElementToModel (DgnModelR dgnModel, MSElementDescrR descr, AddElementOptions const& opts)
    {
    // set this model into the incoming MSElementDesr (necessary to resolve handler).
    descr.SetDgnModel(dgnModel);

    // make sure we have a handler for every entry in descr
    opts._ResolveHandler(descr);

    DgnModelStatus stat = opts._VerifyElemDescr (*this, descr);
    if (DGNMODEL_STATUS_Success != stat)
        return  stat;

    opts._ValidateIds (dgnModel.GetDgnProject(), descr);

    // create a new PersistentElementRef (and sub-elements), and copy data from descriptor into DgnModel
    DgnElementRef& newElRef = CreateNewElementRef (dgnModel, descr, opts);

    DbElementWriter writer (dgnModel.GetDgnProject(), *this);

    try
        {
        writer.InsertElement (dgnModel, (PersistentElementRefP) descr.GetElementRef());
        }
    catch (DbException)
        {
        return DGNMODEL_STATUS_ElementWriteError;
        }

    // notify listeners of new element ref existence
    opts._OnAdded ((PersistentElementRefR) newElRef, true);
    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus ElementListHandler::ReplaceElement (MSElementDescrR newDescr, PersistentElementRefR oldElm, AddElementOptions const& opts)
    {
    DgnModelR dgnModel = *oldElm.GetDgnModelP();
    if (dgnModel.IsReadOnly())
        return  DGNMODEL_STATUS_ReadOnly;

    DgnModelStatus vStatus = opts._VerifyElemDescr (*this, newDescr);
    if (DGNMODEL_STATUS_Success != vStatus)
        return vStatus;

    DgnProjectR project = dgnModel.GetDgnProject();

    oldElm.TransferIdToDescr (newDescr);
    oldElm.ModifyFromDescr (dgnModel, newDescr);

    PersistentElementRefList* list = dgnModel.GetGraphicElementsP();
    if (list)
        list->_OnElementModified(oldElm);

    DbElementWriter writer (project, *this);
    writer.UpdateElement (dgnModel, &oldElm);

    return  DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementTableHandler::AddLifecycleChange(TxnSummary::TxnLifecycleSet& group, BeSQLite::Changes::Change const& change, bool useNew) const
    {
    Changes::Change::ValueStage stage = useNew ? Changes::Change::VALUE_New : Changes::Change::VALUE_Old;

    ElementId elementId(change.GetValue(0, stage).GetValueInt64());
    ElementHandlerId handlerId(static_cast <UInt32> (change.GetValue(4, stage).GetValueInt64()));
    if (!elementId.IsValid())
        {
        BeAssert (false);
        return;
        }

    group.insert (TxnSummary::LifecycleChange(elementId, handlerId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void XAttTableHandler::AddChange (TxnSummary::TxnXAttrSet& group, BeSQLite::Changes::Change const& change, bool useNew) const
    {
    Changes::Change::ValueStage stage = useNew ? Changes::Change::VALUE_New : Changes::Change::VALUE_Old;
    ElementId elementId(change.GetValue(0, stage).GetValueInt64());
    XAttributeHandlerId xaHandlerId (change.GetValue(1, stage).GetValueInt());
    UInt32 xaId = change.GetValue(2, stage).GetValueInt();

    if (!elementId.IsValid() || !xaHandlerId.IsValid())
        {
        BeAssert (false);
        return;
        }

    group.insert (TxnSummary::XAttrChange(elementId, xaHandlerId, xaId));
    }

static void compareLow (double& val, double test){if (test<val) val=test;}
static void compareHigh(double& val, double test){if (test>val) val=test;}
static void compareLow (double& val, DbValue dbVal) {if (dbVal.IsValid()) compareLow(val, dbVal.GetValueDouble());}
static void compareHigh(double& val, DbValue dbVal) {if (dbVal.IsValid()) compareHigh(val, dbVal.GetValueDouble());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void unionRange2d (TxnSummary& summary, BeSQLite::Changes::Change const& change, bool useNew)
    {
    Changes::Change::ValueStage stage = useNew ? Changes::Change::VALUE_New : Changes::Change::VALUE_Old;
    DRange2dR range = summary.GetDrawingRangeR();

    compareLow (range.low.x,  change.GetValue(1, stage));
    compareHigh(range.high.x, change.GetValue(2, stage));
    compareLow (range.low.y,  change.GetValue(3, stage));
    compareHigh(range.high.y, change.GetValue(4, stage));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void unionRange3d (TxnSummary& summary, BeSQLite::Changes::Change const& change, bool useNew) 
    {
    Changes::Change::ValueStage stage = useNew ? Changes::Change::VALUE_New : Changes::Change::VALUE_Old;
    DRange3dR range = summary.GetPhysicalRangeR();

    compareLow (range.low.x,  change.GetValue(1, stage));
    compareHigh(range.high.x, change.GetValue(2, stage));
    compareLow (range.low.y,  change.GetValue(3, stage));
    compareHigh(range.high.y, change.GetValue(4, stage));
    compareLow (range.low.z,  change.GetValue(5, stage));
    compareHigh(range.high.z, change.GetValue(6, stage));
    }

/*---------------------------------------------------------------------------------**//**
* Called when an element in the physical table has been modified. We want to union the pre-changed and post-changed
* range values into the transaction summary.
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementTableHandler::_OnUpdate(TxnSummary& summary, BeSQLite::Changes::Change const& change) const
    {
    // get the post-changed state of the element, loading it if neccessary
    ElementId elementId(change.GetOldValue(0).GetValueInt64());
    auto elRef = summary.GetDgnProject().Models().GetElementById(elementId);
    if (!elRef.IsValid())
        {
        BeAssert (false);
        return;
        }

    summary.m_modified.insert (elRef.get());

    // skip elements with invalid range
    DgnElementCP el = elRef->GetUnstableMSElementCP();
    if (!el->IsRangeValid3d())
        return;

    // union the post-changed range of the element
    if (!el->Is3d())
        {
        DRange2dR range = summary.GetDrawingRangeR();
        range.Extend(el->GetRange().low);
        range.Extend(el->GetRange().high);

        unionRange2d (summary, change, true);
        unionRange2d (summary, change, false);
        }
    else
        {
        DRange3d& range = summary.GetPhysicalRangeR();
        range.Extend(el->GetRange());

        unionRange3d (summary, change, true);
        unionRange3d (summary, change, false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ElementListHandlerR PersistentElementRef::GetListHandler() const
    {
    return m_dgnModel->_GetGraphicElementHandler();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus PersistentElementRef::ReloadFromDb ()
    {
    m_dgnModel->ElementChanged (*this, ELEMREF_CHANGE_REASON_Modify);

    DbElementReloader reloader (*this);
    DgnModelStatus stat = reloader.ReloadElement();

    PersistentElementRefListP list = m_dgnModel->GetGraphicElementsP();
    if (DGNMODEL_STATUS_Success == stat && list)
        list->_OnElementModified(*this);

    return  stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFileStatus DgnModel::FillModel()
    {
    if (IsFilled())
        return  DGNFILE_STATUS_Success;

    try
        {
        GraphicElementRefList* graphicElms = GetGraphicElementsP(true);
        if (NULL == graphicElms)
            return  DGNFILE_ERROR_ModelLoadError;

        DbElementListReader reader (*graphicElms);
        reader.ReadList(true);
        }
    catch (DbException e)
        {
        return  e.m_status;
        }

    SetReadOnly (m_project.IsReadonly());
    ModelFillComplete();

    return  DGNFILE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult PhysicalModel::_QueryModelRange (DRange3dR range)
    {
    Statement stmt;
    DbResult rc = stmt.Prepare (m_project, "SELECT min([Min.X]),max([Max.X]),min([Min.Y]),max([Max.Y]),min([Min.Z]),max([Max.Z]) FROM " DGN_TABLE_Element
                                           " WHERE ModelId=? AND [Min.X] IS NOT NULL");
    if (rc!=BE_SQLITE_OK)
        {
        BeAssert (false);
        return  rc;
        }

    stmt.BindId (1, GetModelId());
    rc = stmt.Step();
    BeAssert(rc==BE_SQLITE_ROW);
    range.low.x  = stmt.GetValueDouble(0);
    range.high.x = stmt.GetValueDouble(1);
    range.low.y  = stmt.GetValueDouble(2);
    range.high.y = stmt.GetValueDouble(3);
    range.low.z  = stmt.GetValueDouble(4);
    range.high.z = stmt.GetValueDouble(5);
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnModel2d::_QueryModelRange (DRange3dR range)
    {
    Statement stmt;
    DbResult rc = stmt.Prepare (m_project, "SELECT min([Min.X]),max([Max.X]),min([Min.Y]),max([Max.Y]) FROM " DGN_TABLE_Element
                                           " WHERE ModelId=? AND [Min.X] IS NOT NULL");
    if (rc!=BE_SQLITE_OK)
        {
        BeAssert (false);
        return  rc;
        }

    stmt.BindId (1, GetModelId());
    rc = stmt.Step();
    BeAssert(rc==BE_SQLITE_ROW);
    range.low.x  = stmt.GetValueDouble(0);
    range.high.x = stmt.GetValueDouble(1);
    range.low.y  = stmt.GetValueDouble(2);
    range.high.y = stmt.GetValueDouble(3);
    range.low.z = range.high.z = 0.0;
    return rc;
    }

