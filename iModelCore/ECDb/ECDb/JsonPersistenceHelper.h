/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include <Bentley/ByteStream.h>
#include <Geom/GeomApi.h>
#include <BeRapidJson/BeRapidJson.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Converts different data types to JSON whenever JSON is persisted in the ECDb file.
//! @remarks This helper has to be used whenever JSON is written/read from the DB. This is to
//! ensure that the file format is not changed by API changes. E.g. ECJsonUtilities must not be
//! used as it might change.
// @bsiclass                                                Krischan.Eberle      09/2017
//+===============+===============+===============+===============+===============+======
struct JsonPersistenceHelper
    {
    public:
        static constexpr Utf8CP PointXMemberName() { return "x"; }
        static constexpr Utf8CP PointYMemberName() { return "y"; }
        static constexpr Utf8CP PointZMemberName() { return "z"; }

    private:
        JsonPersistenceHelper() = delete;
        ~JsonPersistenceHelper() = delete;

        static BentleyStatus PointCoordinateFromJson(double&, RapidJsonValueCR, Utf8CP coordinatePropName);

    public:

        //! Converts the specified Byte array to a RapidJsonValue
        //! The Byte array is converted to a string using a Base64 encoding.
        //! @param[out] json the resulting string RapidJsonValue
        //! @param[in] binaryArray the Byte array
        //! @param[in] binarySize size of the Byte array
        //! @param[in] allocator Allocator to use to copy the string into the RapidJson value.
        //! @return SUCCESS or ERROR
        static BentleyStatus BinaryToJson(RapidJsonValueR json, Byte const* binaryArray, size_t binarySize, rapidjson::MemoryPoolAllocator<>& allocator);

        //! Converts the specified RapidJsonValue to a ByteStream
        //! The RapidJsonValue must hold the BLOB as Base64 encoded string.
        //! @param[out] byteStream the resulting ByteStream
        //! @param[in] json The RapidJsonValue
        //! @return SUCCESS or ERROR
        static BentleyStatus JsonToBinary(ByteStream& byteStream, RapidJsonValueCR json);

        //! Converts the specified DPoint2d to a Json value
        //! The point is converted to a Json object with keys "x", "y" and "z".
        //! @param[out] json the resulting Json value
        //! @param[in] pt Point to convert
        //! @param[in] allocator Allocator to use to populate the RapidJson value.
        //! @return SUCCESS or ERROR
        static BentleyStatus Point2dToJson(RapidJsonValueR json, DPoint2d pt, rapidjson::MemoryPoolAllocator<>& allocator);
        //! Converts the specified Json value to a DPoint2d
        //! The Json value must hold the point as Json object with keys "x" and "y"
        //! @param[out] pt the resulting point
        //! @param[in] json the Json value
        //! @return SUCCESS or ERROR
        static BentleyStatus JsonToPoint2d(DPoint2d& pt, RapidJsonValueCR json);
        //! Converts the specified DPoint3d to a Json value
        //! The point is converted to a Json object with keys "x", "y" and "z".
        //! @param[out] json the resulting Json value
        //! @param[in] pt Point to convert
        //! @param[in] allocator Allocator to use to populate the RapidJson value.
        //! @return SUCCESS or ERROR
        static BentleyStatus Point3dToJson(RapidJsonValueR json, DPoint3d pt, rapidjson::MemoryPoolAllocator<>& allocator);
        //! Converts the specified Json value to a DPoint3d
        //! The Json value must hold the point as Json object with keys "x", "y" and "z"
        //! @param[out] pt the resulting point
        //! @param[in] json the Json value
        //! @return SUCCESS or ERROR
        static BentleyStatus JsonToPoint3d(DPoint3d& pt, RapidJsonValueCR json);

   };

END_BENTLEY_SQLITE_EC_NAMESPACE