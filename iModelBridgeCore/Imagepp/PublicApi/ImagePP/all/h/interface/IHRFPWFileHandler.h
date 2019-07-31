//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE

class IHRFPWFileHandler
    {
public:

    enum GETTILE_STATUS
        {
        GETTILE_Success         = 1,
        GETTILE_Error           = 2,
        GETTILE_ConnectionLost  = 3,
        GETTILE_NotFound        = 4,
        GETTILE_Changed         = 5
        };

    virtual GETTILE_STATUS GetTile(GUID             pi_DocumentID,
                                   __time32_t       pi_pDocumentTimestamp,
                                   uint32_t         pi_Page,
                                   uint16_t         pi_Res,
                                   uint32_t         pi_PosX,
                                   uint32_t         pi_PosY,
                                   Byte*            po_pData) = 0;


    };

END_IMAGEPP_NAMESPACE
