/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmStream.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "malloc.h"
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
//#pragma optimize( "p", on )
#include "bcDTMStream.h"

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
struct BcDtmFileStream : public BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream
    {
    private:
        FILE* m_fP;
    public:
        BcDtmFileStream(FILE* fP)
            {
            m_fP = fP;
            }
        virtual int Seek(long offset, int origin)
            {
            return fseek(m_fP, offset, origin);
            }
        virtual int Ftell()
            {
            return ftell(m_fP);
            }
        virtual size_t Read(void* dest, size_t elementSize, size_t count)
            {
            return bcdtmFread(dest, elementSize, count, m_fP);
            }
        virtual size_t Write(void* source, size_t elementSize, size_t count)
            {
            return bcdtmFwrite(source, elementSize, count, m_fP);
            }
    };

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
 BENTLEYDTM_EXPORT        int bcdtmStream_createFromFILE(FILE* fP, BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream** dtmStreamPP)
 {
     if(*dtmStreamPP) delete *dtmStreamPP;
     *dtmStreamPP = new BcDtmFileStream(fP);
     if(*dtmStreamPP == NULL) return DTM_ERROR;
     return DTM_SUCCESS;
 }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
 BENTLEYDTM_EXPORT        int bcdtmStream_destroy(BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream** dtmStreamPP)
 {
     if(*dtmStreamPP) delete *dtmStreamPP;
     *dtmStreamPP = NULL;
     return DTM_SUCCESS;
 }

 // Functions to query BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
 BENTLEYDTM_EXPORT        int bcdtmStream_fseek(BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream* stream, long offset, int origin)
     {
     return stream->Seek(offset, origin);
     }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
 BENTLEYDTM_EXPORT  int bcdtmStream_ftell(BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream* stream)
     {
     return stream->Ftell();
     }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
 BENTLEYDTM_EXPORT     size_t bcdtmStream_fread(void* dest, size_t elementSize, size_t count, BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream* stream)
     {
     return stream->Read(dest, elementSize, count);
     }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
 BENTLEYDTM_EXPORT     size_t bcdtmStream_fwrite(void* source, size_t elementSize, size_t count, BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream* stream)
     {
     return stream->Write(source, elementSize, count);
     }

