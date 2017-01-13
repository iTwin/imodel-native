#pragma once

#include "ISMDataStore.h"
#include "SMStoreUtils.h"
#include <ImagePP/all/h/HRARaster.h>

template <class DATATYPE, class EXTENT> class SMStreamedSourceStore : public ISMNodeDataStore<DATATYPE>
    {
    private:

        SMSQLiteFilePtr            m_smSQLiteFile;
        SMIndexNodeHeader<EXTENT>* m_nodeHeader;
        SMStoreDataType            m_dataType;
        HFCPtr<ImagePP::HRARaster>          m_source;

    public:

        SMStreamedSourceStore(SMStoreDataType dataType, SMIndexNodeHeader<EXTENT>* nodeHeader, SMSQLiteFilePtr& smSQLiteFile, DRange3d totalExt);

        virtual ~SMStreamedSourceStore();

        virtual HPMBlockID StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID) override //This store is read-only 
            {
            return HPMBlockID();
            }

        virtual size_t GetBlockDataCount(HPMBlockID blockID) const override;

        virtual size_t GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const override;

        virtual size_t LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID) override;

        virtual bool DestroyBlock(HPMBlockID blockID) override //This store is read-only 
            {
            return false;
            }

        virtual void ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) override //This store is read-only 
            {}

        virtual void ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta, SMStoreDataType dataType) override //This store is read-only 
            {}
    };
