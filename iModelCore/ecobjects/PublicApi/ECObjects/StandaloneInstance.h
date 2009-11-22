/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StandaloneInstance.h $
|
|   $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#pragma once

#include <ECObjects\ECObjects.h>

BEGIN_BENTLEY_EC_NAMESPACE
    
//! EC::StandaloneInstance is the native equivalent of a .NET "Heavyweight" ECInstance.
//! It holds the values in memory that it allocates... laid out according to the ClassLayout
//! @see MemoryEnabler, Instance
struct StandaloneInstance : MemoryBasedInstance
    {
private:
    MemoryEnablerPtr m_enabler; 
    std::wstring     m_instanceID;
    
    byte *           m_data;
    UInt32           m_bytesUsed;            
    UInt32           m_bytesAllocated;
    // WIP_FUSION: Unpublish most/all of this?
     
public: // These two must be public so that XDataEnabler can get at the guts of Standalone to copy it into and XAttribute     
    ECOBJECTS_EXPORT virtual byte *   GetData () const;
    ECOBJECTS_EXPORT virtual UInt32   GetBytesAllocated () const;
    
private:
    virtual bool     IsMemoryInitialized () const;    
    virtual UInt32   GetBytesUsed () const;
    virtual void     AdjustBytesUsed (Int32 adjustment);
    virtual void     SetBytesUsed (UInt32 nBytes);
    virtual void     ShrinkAllocation (UInt32 newAllocation);
    virtual void     FreeAllocation ();
    virtual void     AllocateBytes (UInt32 minimumBytesToAllocate);
    virtual void     GrowAllocation (UInt32 bytesNeeded);        
    
protected:
    ECOBJECTS_EXPORT virtual EnablerCP       _GetEnabler() const override;
    ECOBJECTS_EXPORT virtual MemoryEnablerCP GetMemoryEnabler() const override;
    
    ECOBJECTS_EXPORT virtual std::wstring    _GetInstanceID() const override;
 
    ECOBJECTS_EXPORT virtual bool            _IsReadOnly() const override;        

public:
    ECOBJECTS_EXPORT StandaloneInstance (MemoryEnablerCR enabler);
    ECOBJECTS_EXPORT StandaloneInstance (ClassCR ecClass);
    
    //! Provides access to the raw data. For internal use only
    ECOBJECTS_EXPORT byte const * PeekData();
    //! Provides access to the raw data. For internal use only
    ECOBJECTS_EXPORT UInt32 PeekDataSize();

    };

END_BENTLEY_EC_NAMESPACE