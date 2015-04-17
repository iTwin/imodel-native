/*----------------------------------------------------------------------+
|
|   $Source: DgnCore/BSISerializable.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RichardTrefz    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
BsiSerializable::BsiSerializable (uint32_t version)
    {
    m_blockTypeId = 0;
    m_version =
        m_highestVersionWritten =
            m_lastVersionWritten = version;
    m_extraDataSize = 0;
    m_extraData = 0;
    m_storeRead = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RichardTrefz    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
BsiSerializable::~BsiSerializable ()
    {
    delete m_extraData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RichardTrefz    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BsiSerializable::SerRead
(
Byte*           startOfStore,
int             sizeOfStore
)
    {
    m_storeRead = false;

    DataInternalizer    reader;

    reader.setBuffer (startOfStore, sizeOfStore);

    reader.get (&m_highestVersionWritten);
    reader.get (&m_lastVersionWritten);

    uint32_t zero = 0;
    reader.get (&zero);
    reader.get (&zero);

    StatusInt   status;
    if (BSISUCCESS != (status = SerReadFields (reader)))
        return status;

    if (m_extraData)
        {
        delete m_extraData;
        m_extraData = 0;
        }

    m_extraDataSize = static_cast<uint32_t>(reader.getRemainingSize());

    if (m_extraDataSize > 0)
        {
        m_extraData = new Byte [m_extraDataSize];
        reader.get (m_extraData, m_extraDataSize);
        }

    m_storeRead = true;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RichardTrefz    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BsiSerializable::SerWrite
(
int*            id,
uint32_t*         numBytes,
Byte**          block
)
    {
    *id = m_blockTypeId;

    *numBytes = 0;
    *block    = NULL;

    m_highestVersionWritten = MAX (m_highestVersionWritten, m_version);
    m_lastVersionWritten = m_version;

    DataExternalizer   writer;
    writer.rewind ();

    writer.put (m_highestVersionWritten);
    writer.put (m_lastVersionWritten);

    uint32_t zero = 0;
    writer.put (zero);
    writer.put (zero);

    SerWriteFields (writer);

    if (m_extraData  &&  m_extraDataSize)
        writer.put ((Byte*)m_extraData, m_extraDataSize);

    *numBytes = static_cast<uint32_t>(writer.getBytesWritten());

    if (*numBytes > 0)
        {
        *block = new Byte [*numBytes];
        memcpy (*block, writer.getBufRW(), *numBytes);
        }

    return BSISUCCESS;
    }

