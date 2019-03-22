//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRATransaction.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Class : HRATransaction
//:>---------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HRATransaction : public HFCShareableObject<HRATransaction>
    {
public:

    HRATransaction();
    virtual ~HRATransaction ();


    virtual void    PushEntry               (uint64_t       pi_PosX,
                                             uint64_t       pi_PosY,
                                             uint32_t       pi_Width,
                                             uint32_t       pi_Height,
                                             size_t         pi_DataSize,
                                             const void*    pi_pData) = 0;

    virtual bool   PopEntry                (uint64_t*       po_pPosX,
                                             uint64_t*      po_pPosY,
                                             uint32_t*      po_pWidth,
                                             uint32_t*      po_pHeight,
                                             size_t*        po_pDataSize,
                                             bool           pi_FirstCall = false) = 0;


    virtual size_t  ReadEntryData           (size_t         pi_DataSize,
                                             void*          po_pData) = 0;



    virtual void    Commit          () = 0;
    virtual void    Rollback        () = 0;

    virtual void    Clear           () = 0;

    virtual bool   IsEmpty         () const = 0;

    virtual uint32_t GetID           () const = 0;

protected:

private:
    // members

    // disabled methods
    HRATransaction(const HRATransaction&);
    HRATransaction& operator=(const HRATransaction&);
    };
END_IMAGEPP_NAMESPACE



