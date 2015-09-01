//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTIFFTagDefinition.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HTIFFTagDefinition
//-----------------------------------------------------------------------------

#pragma once

#include "HTIFFTag.h"
#include "HTagDefinition.h"

BEGIN_IMAGEPP_NAMESPACE
class HTIFFTagInfo : public HTagInfo
    {
public:
    virtual bool                   IsBigTiffTag                       (uint32_t FileTagNumber) const override;

    virtual HTagID                  GetFreeOffsetsTagID                () const override {
        return BentleyApi::ImagePP::FREEOFFSETS;
    }
    virtual HTagID                  GetFreeByteCountsTagID             () const override {
        return BentleyApi::ImagePP::FREEBYTECOUNTS;
    }

    virtual HTagID                  GetSubFileTypeTagID                () const override {
        return BentleyApi::ImagePP::SUBFILETYPE;
    }
    virtual HTagID                  GetHMRSyncronizationTagID          () const override {
        return BentleyApi::ImagePP::HMR_SYNCHRONIZE_FIELD;
    }
    virtual HTagID                  GetHMRDirectoryV1TagID             () const override {
        return BentleyApi::ImagePP::HMR_IMAGEINFORMATION;
    }
    virtual HTagID                  GetHMRDirectoryV2TagID             () const override {
        return BentleyApi::ImagePP::HMR2_IMAGEINFORMATION;
    }

    virtual HTagID                  GetHMRDecimationMethodTagID        () const override {
        return BentleyApi::ImagePP::HMR_DECIMATION_METHOD;
    }

    virtual HTagID                  GetGeoKeyDirectoryTagID            () const override {
        return BentleyApi::ImagePP::GEOKEYDIRECTORY;
    }
    virtual HTagID                  GetGeoDoubleParamsTagID            () const override {
        return BentleyApi::ImagePP::GEODOUBLEPARAMS;
    }
    virtual HTagID                  GetGeoAsciiParamsTagID             () const override {
        return BentleyApi::ImagePP::GEOASCIIPARAMS;
    }

    virtual HTagID                  GetNotSavedTagIDBegin              () const override {
        return BentleyApi::ImagePP::TAG_NOT_SAVED_FILE;
    }
    virtual uint32_t                GetTagQty                          () const override {
        return BentleyApi::ImagePP::EndOfTag;
    }

    virtual size_t                  GetTagDefinitionQty                () const override {
        return sNumberOfDef;
    }
    virtual const Info*             GetTagDefinitionArray              () const override {
        return sTagInfo;
    }

private:
    static const Info               sTagInfo[];
    static const uint32_t           sNumberOfDef;
    };
END_IMAGEPP_NAMESPACE


