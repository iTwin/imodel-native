/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/PointCloud/PointCloudAttributes.h $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudAttributeBase : public RefCounted<IRefCounted>
    {
    public:
        typedef UInt16 DataType;

        // Never changed these ids since they are stored in a xAttribute.
        // Attributes with positive ids are preserved if unknown.
        // Ids greater than 10000 are lost since 8.11.09.xx.
        enum AttributeId 
            {
            AttributeId_MonikerString = 0,
            AttributeId_GlobalOrigin  = 1,
            AttributeId_WktString     = 2,
            };

        virtual void Store(DataExternalizer& dataExternalizer) = 0;
        virtual void Load(DataInternalizer& dataInternalizer) = 0;
        virtual DataType GetDataType() const = 0;

        static  bool IsPreservedIfUnknown(DataType id) { return id <= (DataType)10000; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                    StephanePoulin  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnknownAttributeData : public PointCloudAttributeBase
    {
    public:
        static UnknownAttributeData* Create(DataType dataType, UInt32 dataSize) { return new UnknownAttributeData(dataType, dataSize); }

        virtual ~UnknownAttributeData()
            {
            delete[] m_data;
            }

        UInt8 const* GetDataP() const        { return m_data; }
        UInt32       GetDataSize() const     { return m_dataSize; }

        // from PointCloudAttributeBase
        virtual void Store(DataExternalizer& dataExternalizer)
            {
            dataExternalizer.put(m_dataType);           // DataType
            dataExternalizer.put(m_dataSize);           // DataSize
            dataExternalizer.put(m_data, m_dataSize);   // Data
            }
        virtual void Load(DataInternalizer& dataInternalizer)
            {
            // Load unknown data.
            dataInternalizer.get(m_data, m_dataSize);
            }

        DataType GetDataType () const override { return m_dataType; }

    protected:
        UnknownAttributeData(UInt16 dataType, UInt32 dataSize) 
            : PointCloudAttributeBase(), m_dataType(dataType), m_dataSize(dataSize)
            {
            m_data = new UInt8[dataSize];
            }

    private:
        UnknownAttributeData();                                       //disabled
        UnknownAttributeData(UnknownAttributeData const&);            //disabled
        UnknownAttributeData& operator=(UnknownAttributeData const&); //disabled

        DataType  m_dataType;
        UInt32  m_dataSize;
        UInt8*  m_data;
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE
