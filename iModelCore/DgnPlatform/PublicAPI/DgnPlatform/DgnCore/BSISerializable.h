/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/BSISerializable.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma  once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <Bentley/Bentley.h>
#include "RmgrTools/Tools/DataExternalizer.h"

//=======================================================================================
//! @abstract   Base class for all settings block classes
//! @discussion The MStn settings element introduced in V9 is an element comprised of one
//!           one or more settings block. Each settings block is represented internally as
//!           a C++ class.  These classes are derived from BsiSerializable.
//!           <P>
//!               To create a settings block you must derive a C++ class from BsiSerializable
//!               and override these methods.
//!           </P>
//!           <TABLE id="Table1" cellSpacing="1" cellPadding="1" border="1">
//!               <TR>
//!                   <TD>
//!                       SerWriteFields
//!                   </TD>
//!                   <TD>
//!                       The derived class method should provide a m_writer.put call for each persistable field
//!                   </TD>
//!               </TR>
//!               <TR>
//!                   <TD>
//!                       SerReadFields
//!                   </TD>
//!                   <TD>
//!                       The derived class method should provide a m_reader.get call for each persistable field
//!                   </TD>
//!               </TR>
//!               <TR>
//!                   <TD>SerInitData</TD>
//!                   <TD>
//!                       This method should set all the class fields to reasonable values
//!                   </TD>
//!               </TR>
//!           </TABLE>
//! @bsiclass                                                     RichardTrefz    10/02
struct BsiSerializable
{

private:
    uint32_t    m_blockTypeId;
    uint32_t    m_version;
    uint32_t    m_highestVersionWritten;
    uint32_t    m_lastVersionWritten;
    bool        m_storeRead;
    uint32_t    m_extraDataSize;
    Byte*       m_extraData;

protected:
     //! @function    SerWriteFields
     //!              Virtual method which must be overriden in the derived class.
     //!              The derived class method should provide a m_writer.put call for each persistable field
     //! @result      BSISUCCESS or BSIERROR
    virtual StatusInt   SerWriteFields  (DataExternalizer&) = 0;

     //! @function    SerReadFields
     //!              Virtual method which must be overriden in the derived class.
     //!              The derived class method should provide a m_reader.get call for each persistable field
     //! @result      BSISUCCESS or BSIERROR
    virtual StatusInt   SerReadFields   (DataInternalizer&) = 0;

     //! @function    SerInitFields
     //!              Virtual method which must be overriden in the derived class.
     //!              This method should set all the fields in the class to reasonable default values, to handle
     //!              the situation that no settings block is found (or an older one with less fields is found).
    virtual void        SerInitFields   () = 0;

     //! @function    SerSetId
     //!              Sets the unique ID of the settings block.
    void    SerSetId                    (uint32_t id)   {m_blockTypeId = id;}

     //! @function    SerGetId
     //!              Returns the ID that uniquely identifies the settings block within the settings element.
     //! @result      Settings block ID
    uint32_t SerGetId                    () {return m_blockTypeId;}

     //! @function    SerWasStoreLoaded
     //!              Indicates whether the fields in the store were set by the initialization process or by loading a store.
     //! @result      Store loaded flag
    bool    SerWasStoreLoaded           () {return m_storeRead;}

     //! @function    SerGetHighestVersionWritten
     //!              Returns the (creation) version number of the settings block.
     //!              If V9 MStn updates a V10 settings element, the version created
     //!              might be 2 and the version written might be 1.
     //!              If V10 MStn updates a V9 settings element, the versions will be the same.
     //! @result      Version created
    uint32_t SerGetHighestVersionWritten () {return m_highestVersionWritten;}

     //! @function    SerGetVersionWritten
     //!              Returns the (last written) version number of the settings block.
     //!              If V9 MStn updates a V10 settings element, the version created
     //!              might be 2 and the version written might be 1.
     //!              If V10 MStn updates a V9 settings element, the versions will be the same.
     //! @result      Version written
    uint32_t SerGetLastVersionWritten    () {return m_lastVersionWritten;}

     //! @function    BsiSerializable
     //!              BsiSerializable constructor
     //! @param       version (IN) Version of settings block
    DGNPLATFORM_EXPORT BsiSerializable      (uint32_t version);

     //! @function    ~BsiSerializable
     //!              BsiSerializable destructor
    DGNPLATFORM_EXPORT virtual ~BsiSerializable     ();

public:
     //! @function    SerWrite
     //!              Appends a settings block to a settings element
     //! @param       id          (OUT) ID of the settings block created
     //! @param       numBytes    (OUT) Size of the settings block store
     //! @param       block       (OUT) Raw data of the settings block store
     //! @result      BSISUCCESS or BSIERROR
    DGNPLATFORM_EXPORT virtual StatusInt   SerWrite     (int* id, uint32_t* numBytes, Byte** block);

     //! @function    SerRead
     //!              Extracts a settings block from a raw byte buffer
     //! @param       startOfStore (IN) buffer to extract from
     //! @param       sizeOfStore  (IN) size in bytes of the buffer
     //! @result      The number of bytes consumed
    DGNPLATFORM_EXPORT virtual StatusInt   SerRead      (Byte* startOfStore, int sizeOfStore);

     //! @enum        PersistentDataTags
     //!              List of settings block ID's.
     //! @constant    PERSISTENT_DATA_DisplayPriority     ID for the display priority block.
    enum PersistentDataTags
    {
        PERSISTENT_DATA_UUID_Class                 = 1,
        PERSISTENT_DATA_DisplayPriority            = 1001,
        PERSISTENT_DATA_GradientSettings           = 1002,
        PERSISTENT_DATA_TransparencySettings       = 1005,
        PERSISTENT_DATA_CustomizeSettings          = 1006,
        PERSISTENT_DATA_DetailingSymbolSettings    = 1007,
    };
};

