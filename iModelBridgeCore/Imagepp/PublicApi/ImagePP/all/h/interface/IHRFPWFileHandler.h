//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/interface/IHRFPWFileHandler.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once


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
                                   unsigned long    pi_Page,
                                   unsigned short   pi_Res,
                                   unsigned long    pi_PosX,
                                   unsigned long    pi_PosY,
                                   Byte*            po_pData) = 0;


    };