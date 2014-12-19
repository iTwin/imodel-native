//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTIFFTagDefinition.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HTIFFTagDefinition
//-----------------------------------------------------------------------------

#pragma once

#include "HTIFFTag.h"
#include "HTagDefinition.h"

class HTIFFTagInfo : public HTagInfo
    {
public:
    virtual bool                   IsBigTiffTag                       (uint32_t FileTagNumber) const override;

    virtual HTagID                  GetFreeOffsetsTagID                () const override {
        return ImagePP::FREEOFFSETS;
    }
    virtual HTagID                  GetFreeByteCountsTagID             () const override {
        return ImagePP::FREEBYTECOUNTS;
    }

    virtual HTagID                  GetSubFileTypeTagID                () const override {
        return ImagePP::SUBFILETYPE;
    }
    virtual HTagID                  GetHMRSyncronizationTagID          () const override {
        return ImagePP::HMR_SYNCHRONIZE_FIELD;
    }
    virtual HTagID                  GetHMRDirectoryV1TagID             () const override {
        return ImagePP::HMR_IMAGEINFORMATION;
    }
    virtual HTagID                  GetHMRDirectoryV2TagID             () const override {
        return ImagePP::HMR2_IMAGEINFORMATION;
    }

    virtual HTagID                  GetHMRDecimationMethodTagID        () const override {
        return ImagePP::HMR_DECIMATION_METHOD;
    }

    virtual HTagID                  GetGeoKeyDirectoryTagID            () const override {
        return ImagePP::GEOKEYDIRECTORY;
    }
    virtual HTagID                  GetGeoDoubleParamsTagID            () const override {
        return ImagePP::GEODOUBLEPARAMS;
    }
    virtual HTagID                  GetGeoAsciiParamsTagID             () const override {
        return ImagePP::GEOASCIIPARAMS;
    }

    virtual HTagID                  GetNotSavedTagIDBegin              () const override {
        return ImagePP::TAG_NOT_SAVED_FILE;
    }
    virtual uint32_t                GetTagQty                          () const override {
        return ImagePP::EndOfTag;
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


