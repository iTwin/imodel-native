/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnTexture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId DgnTextures::Insert(Texture& tx, DgnDbStatus* outResult)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(result, outResult);
    DgnTextureId newId;

    auto status = m_dgndb.GetServerIssuedId(newId, DGN_TABLE(DGN_CLASSNAME_Texture), "Id");
    if (BE_SQLITE_OK != status)
        {
        result = DgnDbStatus::ForeignKeyConstraint;
        return DgnTextureId();
        }

    TextureData const& data = tx.GetData();
    Statement stmt(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Texture) " (Id,Name,Descr,Data,Format,Width,Height,Flags) Values(?,?,?,?,?,?,?,?)");
    stmt.BindId(1, newId);
    stmt.BindText(2, tx.GetName().empty() ? nullptr : tx.GetName().c_str(), Statement::MakeCopy::No);  // NB: BindText() with an empty Utf8String is NOT equivalent to BindText() with a null Utf8CP...
    stmt.BindText(3, tx.GetDescription(), Statement::MakeCopy::No);
    stmt.BindBlob(4, &data.GetData()[0], static_cast<int>(data.GetData().size()), Statement::MakeCopy::No);
    stmt.BindInt(5, static_cast<int>(data.GetFormat()));
    stmt.BindInt(6, static_cast<int>(data.GetWidth()));
    stmt.BindInt(7, static_cast<int>(data.GetHeight()));
    stmt.BindInt(8, static_cast<int>(data.GetFlags()));

    status = stmt.Step();
    if (BE_SQLITE_DONE != status)
        {
        result = DgnDbStatus::DuplicateName;
        return DgnTextureId();
        }

    result = DgnDbStatus::Success;
    tx.m_id = newId;
    return newId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnTextures::Update(Texture const& tx) const
    {
    if (!tx.IsValid())
        return DgnDbStatus::InvalidId;

    auto const& data = tx.GetData();
    Statement stmt(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Texture) " SET Descr=?,Data=?,Format=?,Width=?,Height=?,Flags=? WHERE Id=?");
    stmt.BindText(1, tx.GetDescription(), Statement::MakeCopy::No);
    stmt.BindBlob(2, &data.GetData()[0], static_cast<int>(data.GetData().size()), Statement::MakeCopy::No);
    stmt.BindInt(3, static_cast<int>(data.GetFormat()));
    stmt.BindInt(4, static_cast<int>(data.GetWidth()));
    stmt.BindInt(5, static_cast<int>(data.GetHeight()));
    stmt.BindInt(6, static_cast<int>(data.GetFlags()));
    stmt.BindId(7, tx.GetId());

    DbResult status = stmt.Step();
    BeDataAssert(BE_SQLITE_DONE == status);
    return (BE_SQLITE_DONE == status) ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnTextures::Format extractFormat(int value)
    {
    auto fmt = static_cast<DgnTextures::Format>(value);
    switch (fmt)
        {
        case DgnTextures::Format::JPEG:
        case DgnTextures::Format::RAW:
        case DgnTextures::Format::PNG:
        case DgnTextures::Format::TIFF:
            return fmt;
        default:
            return DgnTextures::Format::Unknown;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnTextures::Delete(DgnTextureId textureId)
    {
    Statement stmt;
    stmt.Prepare(m_dgndb, "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_Texture) " WHERE Id=?");
    stmt.BindId(1, textureId);
    const auto status = stmt.Step();
    return (BE_SQLITE_DONE == status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextures::Texture DgnTextures::Query(DgnTextureId id) const
    {
    Texture tx;
    if (!id.IsValid())
        return tx;

    BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock;
    CachedStatementPtr stmt;
    m_dgndb.GetCachedStatement(stmt, "SELECT Name,Descr,Data,Format,Width,Height,Flags FROM " DGN_TABLE(DGN_CLASSNAME_Texture) " WHERE Id=?");
    stmt->BindId(1, id);

    if (BE_SQLITE_ROW == stmt->Step())
        {
        tx.m_id = id;
        tx.m_name.AssignOrClear(stmt->GetValueText(0));
        tx.m_descr.AssignOrClear(stmt->GetValueText(1));

        size_t dataSize = static_cast<size_t>(stmt->GetColumnBytes(2));
        BeAssert(dataSize > 0);
        Byte const* data = static_cast<Byte const*>(stmt->GetValueBlob(2));
        tx.m_data.m_data.insert(tx.m_data.m_data.begin(), data, data + dataSize);

        tx.m_data.m_format = extractFormat(stmt->GetValueInt(3));
        tx.m_data.m_width = static_cast<uint32_t>(stmt->GetValueInt(4));
        tx.m_data.m_height = static_cast<uint32_t>(stmt->GetValueInt(5));
        tx.m_data.m_flags = static_cast<DgnTextures::Flags>(stmt->GetValueInt(6));
        }

    return tx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId DgnTextures::QueryTextureId(Utf8StringCR name) const
    {
    Statement stmt(m_dgndb, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Texture) " WHERE Name=?");
    stmt.BindText(1, name, Statement::MakeCopy::No);
    DgnTextureId id;
    if (BE_SQLITE_ROW == stmt.Step())
        id = stmt.GetValueId<DgnTextureId>(0);

    return id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextures::Iterator::const_iterator DgnTextures::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sql = MakeSqlString("SELECT Id,Name,Descr,Data,Format,Width,Height,Flags FROM " DGN_TABLE(DGN_CLASSNAME_Texture));
        m_db->GetCachedStatement(m_stmt, sql.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnTextures::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE(DGN_CLASSNAME_Texture));
    Statement sql;
    sql.Prepare(*m_db, sqlString.c_str());
    return (BE_SQLITE_ROW == sql.Step()) ? static_cast<size_t>(sql.GetValueInt(0)) : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId DgnTextures::Iterator::Entry::GetId() const            { Verify(); return m_sql->GetValueId<DgnTextureId>(0); }
Utf8CP DgnTextures::Iterator::Entry::GetName() const                { Verify(); return m_sql->GetValueText(1); }
Utf8CP DgnTextures::Iterator::Entry::GetDescr() const               { Verify(); return m_sql->GetValueText(2); }
size_t DgnTextures::Iterator::Entry::GetDataSize() const            { Verify(); return static_cast<size_t> (m_sql->GetColumnBytes(3)); }
Byte const* DgnTextures::Iterator::Entry::GetDataBytes() const           { Verify(); return static_cast<Byte const*> (m_sql->GetValueBlob(3)); }
DgnTextures::Format DgnTextures::Iterator::Entry::GetFormat() const { Verify(); return extractFormat(m_sql->GetValueInt(4)); }
uint32_t DgnTextures::Iterator::Entry::GetWidth() const             { Verify(); return static_cast<uint32_t> (m_sql->GetValueInt(5)); }
uint32_t DgnTextures::Iterator::Entry::GetHeight() const            { Verify(); return static_cast<uint32_t> (m_sql->GetValueInt(6)); }
DgnTextures::Flags DgnTextures::Iterator::Entry::GetFlags() const   { Verify(); return static_cast<DgnTextures::Flags> (m_sql->GetValueInt(7)); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t DgnTextures::AddQvTextureId(DgnTextureId TextureId) const 
    { 
    static uintptr_t s_nextQvTextureId;
    return (m_qvTextureIds[TextureId] = ++s_nextQvTextureId); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t DgnTextures::GetQvTextureId(DgnTextureId TextureId) const
    {
    auto const& found = m_qvTextureIds.find(TextureId);
    return (found == m_qvTextureIds.end()) ? 0 : found->second; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnTextures::Texture::GetImage(bvector<Byte>& image) const
    {
    TextureData const&              textureData = GetData();
    ImageUtilities::RgbImageInfo    imageInfo;
    BentleyStatus                   status = ERROR;

    memset(&imageInfo, 0, sizeof (imageInfo));
    switch (textureData.GetFormat())
        {
        case DgnTextures::Format::RAW:
            image = textureData.GetData();
            return SUCCESS;

        case DgnTextures::Format::PNG:  
            status = ImageUtilities::ReadImageFromPngBuffer(image, imageInfo, &textureData.GetData().front(), textureData.GetData().size());
            break;

        case DgnTextures::Format::JPEG:
            {
            ImageUtilities::RgbImageInfo    jpegInfo;

            jpegInfo.width = textureData.GetWidth();
            jpegInfo.height = textureData.GetHeight();
            jpegInfo.hasAlpha = true;
            jpegInfo.isBGR = false;
            jpegInfo.isTopDown = true;
            
            status = ImageUtilities::ReadImageFromJpgBuffer(image, imageInfo, &textureData.GetData().front(), textureData.GetData().size(), jpegInfo);
            break;
            }
    
        default:
            BeAssert(false);
            return ERROR;
        }

    if (SUCCESS != status ||
        imageInfo.width != textureData.GetWidth() ||
        imageInfo.height != textureData.GetHeight() ||
        !imageInfo.hasAlpha)
        {
        BeAssert(false);
        return ERROR;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId DgnTextures::ImportTexture(DgnImportContext& context, DgnDbR sourceDb, DgnTextureId source)
    {
    Texture sourceTexture = sourceDb.Textures().Query(source);
    if (!sourceTexture.IsValid())
        {
        BeAssert(!source.IsValid() && "look up should fail only for an invalid Textureid");
        return DgnTextureId();
        }

    // If the destination Db already contains a Texture by this name, then remap to it. Don't create another copy.
    DgnTextureId destTextureId = context.GetDestinationDb().Textures().QueryTextureId (sourceTexture.GetName());
    if (destTextureId.IsValid())
        return destTextureId;

    //  Must copy and remap the source material.
    Texture destTexture(sourceTexture);

    Insert (destTexture);

    return context.AddTextureId(source, destTexture.GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId DgnImportContext::RemapTextureId(DgnTextureId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnTextureId dest = FindTextureId (source);
    if (dest.IsValid())
        return dest;

    return GetDestinationDb().Textures().ImportTexture(*this, GetSourceDb(), source);
    }
