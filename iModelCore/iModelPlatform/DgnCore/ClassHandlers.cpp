#include "ClassHandlers.h"
#include "DgnPlatformInternal.h"
using IClassHandler = InstanceRepository::IClassHandler;

namespace Handlers {

    ECSqlStatus BindPlacement3d(Placement3dCR placement, PropertyBinder::Finder finder) {
        ECSqlStatus rc = ECSqlStatus::Success;
        auto& origin = finder("Origin")->GetBinder();
        auto& yaw = finder("Yaw")->GetBinder();
        auto& pitch = finder("Pitch")->GetBinder();
        auto& roll = finder("Roll")->GetBinder();
        auto& bboxLow = finder("BBoxLow")->GetBinder();
        auto& bboxHigh = finder("BBoxHigh")->GetBinder();

        if (placement.IsValid()) {
            rc = origin.BindPoint3d(placement.GetOrigin());
            if (rc != ECSqlStatus::Success) {
                return rc;
            }
            rc = yaw.BindDouble(placement.GetAngles().GetYaw().Degrees());
            if (rc != ECSqlStatus::Success) {
                return rc;
            }
            rc = pitch.BindDouble(placement.GetAngles().GetPitch().Degrees());
            if (rc != ECSqlStatus::Success) {
                return rc;
            }
            rc = roll.BindDouble(placement.GetAngles().GetRoll().Degrees());
            if (rc != ECSqlStatus::Success) {
                return rc;
            }
            rc = bboxLow.BindPoint3d(placement.GetElementBox().low);
            if (rc != ECSqlStatus::Success) {
                return rc;
            }
            rc = bboxHigh.BindPoint3d(placement.GetElementBox().high);
        } else {
            origin.BindNull();
            yaw.BindNull();
            pitch.BindNull();
            roll.BindNull();
            bboxLow.BindNull();
            bboxHigh.BindNull();
        }
        return rc;
    }
    /*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/

    ECSqlStatus BindPlacement2d(Placement2dCR placement, PropertyBinder::Finder finder) {
        ECSqlStatus rc = ECSqlStatus::Success;
        auto& origin = finder("Origin")->GetBinder();
        auto& rotation = finder("Rotation")->GetBinder();
        auto& bboxLow = finder("BBoxLow")->GetBinder();
        auto& bboxHigh = finder("BBoxHigh")->GetBinder();

        if (placement.IsValid()) {
            rc = origin.BindPoint2d(placement.GetOrigin());
            if (rc != ECSqlStatus::Success) {
                return rc;
            }
            rc = rotation.BindDouble(placement.GetAngle().Degrees());
            if (rc != ECSqlStatus::Success) {
                return rc;
            }

            rc = bboxLow.BindPoint2d(placement.GetElementBox().low);
            if (rc != ECSqlStatus::Success) {
                return rc;
            }
            rc = bboxHigh.BindPoint2d(placement.GetElementBox().high);
        } else {
            origin.BindNull();
            rotation.BindNull();
            bboxLow.BindNull();
            bboxHigh.BindNull();
        }
        return rc;
    }
    bool ReadPlacement2d(Placement2d& placement, PropertyReader::Finder finder) {
        auto& originReader = finder("Origin")->GetReader();
        if(originReader.IsNull()) {
            placement = Placement2d();
            return true;
        }

        auto origin = originReader.GetPoint2d();
        auto rotation = finder("Rotation")->GetReader().GetDouble();
        auto boxLow = finder("BBoxLow")->GetReader().GetPoint2d();
        auto boxHi = finder("BBoxHigh")->GetReader().GetPoint2d();
        placement = Placement2d(
            origin,
            Angle::FromDegrees(rotation),
            ElementAlignedBox2d(boxLow.x, boxLow.y, boxHi.x, boxHi.y));
        return placement.IsValid();
    }
    bool ReadPlacement3d(Placement3d& placement, PropertyReader::Finder finder) {
        auto& originReader = finder("Origin")->GetReader();

        if(originReader.IsNull()) {
            placement = Placement3d();
            return true;
        }

        auto origin = originReader.GetPoint3d();
        auto yaw = finder("Yaw")->GetReader().GetDouble();
        auto pitch = finder("Pitch")->GetReader().GetDouble();
        auto roll = finder("Roll")->GetReader().GetDouble();
        auto boxLow = finder("BBoxLow")->GetReader().GetPoint3d();
        auto boxHi = finder("BBoxHigh")->GetReader().GetPoint3d();
        placement = Placement3d(
            origin,
            YawPitchRollAngles(
                Angle::FromDegrees(yaw),
                Angle::FromDegrees(pitch),
                Angle::FromDegrees(roll)),
            ElementAlignedBox3d(boxLow.x, boxLow.y, boxLow.z, boxHi.x, boxHi.y, boxHi.z));
        return placement.IsValid();
    }
    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct JsGeometrySource3d final : public GeometrySource3d {
    private:
        DgnDbR m_db;
        Placement3d m_placement;
        DgnCategoryId m_categoryId;
        GeometryStream m_geometry;

    protected:
        DgnDbR _GetSourceDgnDb() const { return m_db; }
        DgnElementCP _ToElement() const { return nullptr; }
        GeometrySource3dCP _GetAsGeometrySource3d() const { return this; }
        DgnCategoryId _GetCategoryId() const { return m_categoryId; }
        GeometryStreamCR _GetGeometryStream() const { return m_geometry; }
        Placement3dCR _GetPlacement() const { return m_placement; }
        DgnDbStatus _SetPlacement(Placement3dCR placement) {
            m_placement = placement;
            return DgnDbStatus::Success;
        }
        DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) {
            m_categoryId = categoryId;
            return DgnDbStatus::Success;
        }

    public:
        JsGeometrySource3d(DgnDbR dgnDb, DgnCategoryId categoryId, Placement3d&& placement, GeometryStream&& geomStream)
            : m_db(dgnDb), m_placement(std::move(placement)), m_categoryId(categoryId), m_geometry(std::move(geomStream)) {}

        static std::unique_ptr<JsGeometrySource3d> Find(DgnDbR dgnDb, ECInstanceId id) {
            std::unique_ptr<JsGeometrySource3d> source;
            auto pos = InstanceReader::Position(id, "BisCore:GeometrySource3d");
            dgnDb.GetInstanceReader().Seek(pos, [&](const auto& row, auto finder) {
                Placement3d placement;
                ReadPlacement3d(placement, finder);

                auto categoryId = finder("Category")->GetReader().GetNavigation<DgnCategoryId>(nullptr);
                auto& geomReader = finder("GeometryStream")->GetReader();
                auto geomStream = GeometryStream{};
                SnappyFromMemory snappyDecompress;
                auto rc = geomStream.Read(snappyDecompress, dgnDb, geomReader);
                if (rc != ECSqlStatus::Success) {
                    return;
                }
                source = std::make_unique<JsGeometrySource3d>(dgnDb, categoryId, std::move(placement), std::move(geomStream));
            });
            return source;
        }
    };
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct JsGeometrySource2d final : public GeometrySource2d {
    private:
        DgnDbR m_db;
        Placement2d m_placement;
        DgnCategoryId m_categoryId;
        GeometryStream m_geometry;

    protected:
        DgnDbR _GetSourceDgnDb() const { return m_db; }
        DgnElementCP _ToElement() const { return nullptr; }
        GeometrySource2dCP _GetAsGeometrySource2d() const { return this; }
        DgnCategoryId _GetCategoryId() const { return m_categoryId; }
        GeometryStreamCR _GetGeometryStream() const { return m_geometry; }
        Placement2dCR _GetPlacement() const { return m_placement; }
        DgnDbStatus _SetPlacement(Placement2dCR placement) {
            m_placement = placement;
            return DgnDbStatus::Success;
        }
        DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) {
            m_categoryId = categoryId;
            return DgnDbStatus::Success;
        }

    public:
        JsGeometrySource2d(DgnDbR dgnDb, DgnCategoryId categoryId, Placement2d&& placement, GeometryStream&& geomStream)
            : m_db(dgnDb), m_placement(std::move(placement)), m_categoryId(categoryId), m_geometry(std::move(geomStream)) {}

        static std::unique_ptr<JsGeometrySource2d> Find(DgnDbR dgnDb, ECInstanceId id) {
            std::unique_ptr<JsGeometrySource2d> source;
            auto pos = InstanceReader::Position(id, "BisCore:GeometrySource2d");
            dgnDb.GetInstanceReader().Seek(pos, [&](const auto& row, auto finder) {
                Placement2d placement;
                ReadPlacement2d(placement, finder);

                auto categoryId = finder("Category")->GetReader().GetNavigation<DgnCategoryId>(nullptr);
                auto& geomReader = finder("GeometryStream")->GetReader();
                auto geomStream = GeometryStream{};
                SnappyFromMemory snappyDecompress;
                auto rc = geomStream.Read(snappyDecompress, dgnDb, geomReader);
                if (rc != ECSqlStatus::Success) {
                    return;
                }
                source = std::make_unique<JsGeometrySource2d>(dgnDb, categoryId, std::move(placement), std::move(geomStream));
            });
            return source;
        }
    };

    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct JsGeometryPart final : GeometryPartSource {
    private:
        DgnDbR m_db;                //!< Database containing part
        GeometryStream m_geometry;  //!< Geometry of part
        ElementAlignedBox3d m_bbox; //!< Bounding box of part geometry
    protected:
        virtual ElementAlignedBox3dCR _GetPlacement() const { return m_bbox; }
        virtual void _SetPlacement(ElementAlignedBox3dCR bbox) { m_bbox = bbox; }
        virtual GeometryStreamCR _GetGeometryStream() const { return m_geometry; }
        virtual GeometryStreamR _GetGeometryStreamR() { return m_geometry; }
        virtual DgnDbR _GetSourceDgnDb() const { return m_db; }

    public:
        JsGeometryPart(DgnDbR db, GeometryStream&& geom, ElementAlignedBox3d&& bbox) : m_geometry(std::move(geom)), m_bbox(std::move(bbox)), m_db(db) {}
        static std::unique_ptr<JsGeometryPart> Find(DgnDbR db, ECInstanceId id) {
            std::unique_ptr<JsGeometryPart> part;
            auto pos = InstanceReader::Position(id, "BisCore:GeometryPart");
            db.GetInstanceReader().Seek(pos, [&](const auto& row, auto finder) {
                auto& geomReader = finder("GeometryStream")->GetReader();
                auto bboxLow = finder("BBoxLow")->GetReader().GetPoint3d();
                auto bboxHi = finder("BBoxHigh")->GetReader().GetPoint3d();

                ElementAlignedBox3d bbox(bboxLow.x, bboxLow.y, bboxLow.z, bboxHi.x, bboxHi.y, bboxHi.z);
                auto geomStream = GeometryStream{};
                auto rc = geomStream.Read(db.Elements().GetSnappyFrom(), db, geomReader);
                if (rc != ECSqlStatus::Success) {
                    return;
                }
                part = std::make_unique<JsGeometryPart>(db, std::move(geomStream), std::move(bbox));
            });
            return part;
        }
    };

    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct Element : IClassHandler {
        constexpr static auto ClassName = "BisCore:Element";
        void OnNextId(ECInstanceId& id) override {
            if (GetDb<DgnDb>().GetElementIdSequence().GetNextValue(id) != BE_SQLITE_OK) {
                SetError("Failed to get next id for Element");
            }
        };

        PropertyHandlerResult OnBindUserProperty(Utf8CP property, BeJsConst val, PropertyBinder::Finder finder, ECSqlStatus& rc) {
            BeAssert(GetFormat() == JsFormat::JsName);
            if (BeStringUtilities::StricmpAscii(property, "code") == 0) {
                auto code = DgnCode::FromJson(val, GetDb<DgnDb>(), true);
                rc = finder("CodeValue")->GetBinder().BindText(code.GetValue().GetUtf8CP(), IECSqlBinder::MakeCopy::Yes);
                if (rc != ECSqlStatus::Success) {
                    return PropertyHandlerResult::Handled;
                }

                rc = finder("CodeScope")->GetBinder().BindNavigation(code.GetScopeElementId(GetDbR<DgnDb>()));
                if (rc != ECSqlStatus::Success) {
                    return PropertyHandlerResult::Handled;
                }

                rc = finder("CodeSpec")->GetBinder().BindNavigation(code.GetCodeSpecId());
                return PropertyHandlerResult::Handled;
            }
            return PropertyHandlerResult::Continue;
        }

        ECSqlStatus OnReadComplete(BeJsValue& instance, PropertyReader::Finder finder) {
            auto value = finder("CodeValue")->GetReader().GetText();
            auto scopeElementId = finder("CodeScope")->GetReader().GetNavigation<DgnElementId>(nullptr);
            auto specId = finder("CodeSpec")->GetReader().GetNavigation<CodeSpecId>(nullptr);
            auto code = DgnCode::CreateWithDbContext(GetDb<DgnDb>(), specId, scopeElementId, value);
            code.ToJson(instance["code"]);
            // auto jsonProperties = finder("JsonProperties")->GetReader().GetText();
            // if (!Utf8String::IsNullOrEmpty(jsonProperties)) {
            //     BeJsDocument doc;
            //     doc.Parse(jsonProperties);
            //     if (!doc.hasParseError()) {
            //         instance["jsonProperties"].From(doc);
            //     }
            // }

            instance["model"] = finder("Model")->GetReader().GetNavigation<ECInstanceId>(nullptr);
            return ECSqlStatus::Success;
        }
    };
    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct Model : IClassHandler {
        constexpr static auto ClassName = "BisCore:Model";
        void OnNextId(ECInstanceId& id) override {
            BeAssert(GetFormat() == JsFormat::JsName);
            auto modeledElmentId = BeJsPath::Extract(GetInstance(), "$.modeledElement.id");
            if (modeledElmentId.has_value()) {
                id = modeledElmentId->GetId64<ECInstanceId>();
            }
        };
    };

    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct GeometricElement : IClassHandler {
        constexpr static auto ClassName = "BisCore:GeometricElement";

        ECSqlStatus OnReadComplete(BeJsValue& instance, PropertyReader::Finder finder) {
            BeAssert(GetFormat() == JsFormat::JsName);
            if (!GetUserOptions()["wantGeometry"].asBool()) {
                return ECSqlStatus::Success;
            }
        }
        PropertyHandlerResult OnBindUserProperty(Utf8CP property, BeJsConst val, PropertyBinder::Finder finder, ECSqlStatus& rc) {
        }
    };

    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct GeometricElement2d : IClassHandler {
        constexpr static auto ClassName = "BisCore:GeometricElement2d";
        /*---------------------------------------------------------------------------------**/ /**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        PropertyHandlerResult OnBindUserProperty(Utf8CP property, BeJsConst val, PropertyBinder::Finder finder, ECSqlStatus& rc) {
            BeAssert(GetFormat() == JsFormat::JsName);
            if (BeStringUtilities::StricmpAscii(property, "placement") == 0) {
                Placement2d newPlacement;
                newPlacement.FromJson(val);
                rc = BindPlacement2d(newPlacement, finder);
                if (rc != ECSqlStatus::Success) {
                    SetError("Failed to bind placement");
                }
                return PropertyHandlerResult::Handled;
            }
            if (BeStringUtilities::StricmpAscii(property, "elementGeometryBuilderParams") == 0) {
                if (!val.isNull()) {
                    auto napiValue = val.AsNapiValueRef();
                    if (nullptr == napiValue) {
                        SetError("Failed to bind elementGeometryBuilderParams. Only JavaScript is supported");
                        return PropertyHandlerResult::Handled;
                    }

                    auto napiObj = napiValue->m_napiVal.As<Napi::Object>();
                    if (napiObj.Has("is2dPart")) {
                        SetError("BuildGeometryStream failed - invalid builder parameter");
                        return PropertyHandlerResult::Handled;
                    }

                    auto viewIndependentVal = napiObj.Get("viewIndependent");
                    auto entryArrayObj = napiObj.Get("entryArray").As<Napi::Array>();
                    BeAssert(viewIndependentVal.IsUndefined() || viewIndependentVal.IsBoolean());
                    BeAssert(entryArrayObj.IsArray());

                    GeometryBuilderParams bparams;
                    bparams.viewIndependent = viewIndependentVal.IsBoolean() && viewIndependentVal.As<Napi::Boolean>().Value();

                    auto source = JsGeometrySource2d::Find(GetDbR<DgnDb>(), ParseInstanceId());
                    auto status = GeometryStreamIO::BuildFromGeometrySource(*source, bparams, entryArrayObj.As<Napi::Array>());
                    if (DgnDbStatus::Success != status) {
                        SetError("BuildGeometryStream failed");
                        return PropertyHandlerResult::Handled;
                    }
                    return PropertyHandlerResult::Handled;
                }
            }

            if (BeStringUtilities::StricmpAscii(property, "geom") == 0) {
                if (val.isNull()) {
                    SetError("Failed to bind geom. Only JavaScript is supported");
                    return PropertyHandlerResult::Handled;
                }

                auto source = JsGeometrySource2d::Find(GetDbR<DgnDb>(), ParseInstanceId());
                GeometryBuilder::UpdateFromJson(*source, val);
                SnappyToBlob snappy;
                source->GetGeometryStream().Write(snappy, finder("GeometryStream")->GetBinder());

                return PropertyHandlerResult::Handled;
            }

            if (BeStringUtilities::StricmpAscii(property, "geomBinary") == 0) {
                if (val.isNull()) {
                    SetError("Failed to bind geom. Only JavaScript is supported");
                    return PropertyHandlerResult::Handled;
                }

                GeometryStream geomStream;
                val.GetBinary(geomStream);
                SnappyToBlob snappy;
                rc = geomStream.Write(snappy, finder("GeometryStream")->GetBinder());
                if (rc != ECSqlStatus::Success) {
                    SetError("Failed to write GeometryStream");
                    return PropertyHandlerResult::Handled;
                }
                return PropertyHandlerResult::Handled;
            }
            return PropertyHandlerResult::Continue;
        }

        /*---------------------------------------------------------------------------------**/ /**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual ECSqlStatus OnReadComplete(BeJsValue& instance, PropertyReader::Finder finder) override {
            BeAssert(GetFormat() == JsFormat::JsName);
            instance["category"] = finder("Category")->GetReader().GetNavigation<ECInstanceId>(nullptr);
            Placement2d placement;
            if (!ReadPlacement2d(placement, finder)) {
                SetError("Failed to read placement");
                return ECSqlStatus::Error;
            }
            placement.ToJson(instance["placement"]);

            if (GetUserOptions()["wantGeometry"].asBool()) {
                SnappyFromMemory snappy;
                auto categoryId = instance["category"]["id"].GetId64<DgnCategoryId>();
                auto geometryStream = GeometryStream{};
                auto rc = geometryStream.Read(snappy, GetDbR<DgnDb>(), finder("GeometryStream")->GetReader());
                if (rc != ECSqlStatus::Success) {
                    SetError("Failed to read GeometryStream");
                    return rc;
                }

                auto source2d = JsGeometrySource2d{GetDbR<DgnDb>(), categoryId, std::move(placement), std::move(geometryStream)};
                GeometryCollection collection(source2d);
                collection.ToJson(instance["geom"], GetUserOptions());
                return ECSqlStatus::Success;
            }
            return ECSqlStatus::Success;
        }
    };

    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct GeometricElement3d : IClassHandler {
        constexpr static auto ClassName = "BisCore:GeometricElement3d";
        /*---------------------------------------------------------------------------------**/ /**
         * @bsimethod
         +---------------+---------------+---------------+---------------+---------------+------*/
        PropertyHandlerResult OnBindUserProperty(Utf8CP property, BeJsConst val, PropertyBinder::Finder finder, ECSqlStatus& rc) {
            BeAssert(GetFormat() == JsFormat::JsName);
            if (BeStringUtilities::StricmpAscii(property, "placement") == 0) {
                Placement3d newPlacement;
                newPlacement.FromJson(val);
                rc = BindPlacement3d(newPlacement, finder);
                if (rc != ECSqlStatus::Success) {
                    SetError("Failed to bind placement");
                    return PropertyHandlerResult::Handled;
                }
            }

            if (BeStringUtilities::StricmpAscii(property, "elementGeometryBuilderParams") == 0) {
                if (!val.isNull()) {
                    auto napiValue = val.AsNapiValueRef();
                    if (nullptr == napiValue) {
                        SetError("Failed to bind elementGeometryBuilderParams. Only JavaScript is supported");
                        return PropertyHandlerResult::Handled;
                    }

                    auto napiObj = napiValue->m_napiVal.As<Napi::Object>();
                    if (napiObj.Has("is2dPart")) {
                        SetError("BuildGeometryStream failed - invalid builder parameter");
                        return PropertyHandlerResult::Handled;
                    }

                    auto viewIndependentVal = napiObj.Get("viewIndependent");
                    auto entryArrayObj = napiObj.Get("entryArray").As<Napi::Array>();
                    BeAssert(viewIndependentVal.IsUndefined() || viewIndependentVal.IsBoolean());
                    BeAssert(entryArrayObj.IsArray());

                    GeometryBuilderParams bparams;
                    bparams.viewIndependent = viewIndependentVal.IsBoolean() && viewIndependentVal.As<Napi::Boolean>().Value();
                    auto source = JsGeometrySource3d::Find(GetDbR<DgnDb>(), ParseInstanceId());
                    auto status = GeometryStreamIO::BuildFromGeometrySource(*source, bparams, entryArrayObj.As<Napi::Array>());
                    if (DgnDbStatus::Success != status) {
                        SetError("BuildGeometryStream failed");
                        return PropertyHandlerResult::Handled;
                    }
                    return PropertyHandlerResult::Handled;
                }
            }

            if (BeStringUtilities::StricmpAscii(property, "geom") == 0) {
                if (val.isNull()) {
                    SetError("Failed to bind geom. Only JavaScript is supported");
                    return PropertyHandlerResult::Handled;
                }

                auto source = JsGeometrySource2d::Find(GetDbR<DgnDb>(), ParseInstanceId());
                GeometryBuilder::UpdateFromJson(*source, val);
                SnappyToBlob snappy;
                rc = source->GetGeometryStream().Write(snappy, finder("GeometryStream")->GetBinder());
                if (rc != ECSqlStatus::Success) {
                    SetError("Failed to write GeometryStream");
                    return PropertyHandlerResult::Handled;
                }
                return PropertyHandlerResult::Handled;
            }

            if (BeStringUtilities::StricmpAscii(property, "geomBinary") == 0) {
                if (val.isNull()) {
                    SetError("Failed to bind geom. Only JavaScript is supported");
                    return PropertyHandlerResult::Handled;
                }

                GeometryStream geomStream;
                val.GetBinary(geomStream);
                SnappyToBlob snappy;
                rc = geomStream.Write(snappy, finder("GeometryStream")->GetBinder());
                if (rc != ECSqlStatus::Success) {
                    SetError("Failed to write GeometryStream");
                    return PropertyHandlerResult::Handled;
                }
                return PropertyHandlerResult::Handled;
            }
            return PropertyHandlerResult::Continue;
        }

        /*---------------------------------------------------------------------------------**/ /**
         * @bsimethod
         +---------------+---------------+---------------+---------------+---------------+------*/
        virtual ECSqlStatus OnReadComplete(BeJsValue& instance, PropertyReader::Finder finder) override {
            BeAssert(GetFormat() == JsFormat::JsName);
            instance["category"] = finder("Category")->GetReader().GetNavigation<ECInstanceId>(nullptr);
            Placement3d placement;
            if (!ReadPlacement3d(placement, finder)) {
                SetError("Failed to read placement");
                return ECSqlStatus::Error;
            }

            placement.ToJson(instance["placement"]);

            if (GetUserOptions()["wantGeometry"].asBool()) {
                SnappyFromMemory snappy;
                auto categoryId = instance["category"]["id"].GetId64<DgnCategoryId>();
                auto geometryStream = GeometryStream{};
                auto rc = geometryStream.Read(snappy, GetDbR<DgnDb>(), finder("GeometryStream")->GetReader());
                if (rc != ECSqlStatus::Success) {
                    SetError("Failed to read GeometryStream");
                    return rc;
                }
                auto source3d = JsGeometrySource3d{GetDbR<DgnDb>(), categoryId, std::move(placement), std::move(geometryStream)};

                GeometryCollection collection(source3d);
                collection.ToJson(instance["geom"], GetUserOptions());
                return ECSqlStatus::Success;
            }
            return ECSqlStatus::Success;
        }
    };

    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct RenderMaterial : IClassHandler {
        constexpr static auto ClassName = "BisCore:RenderMaterial";
        /*---------------------------------------------------------------------------------**/ /**
         * @bsimethod
         +---------------+---------------+---------------+---------------+---------------+------*/

        // void RemoveNullValues(BeJsValue& json) {
        //     json.ForEachProperty([&](Utf8CP memberName, BeJsConst memberJson) {
        //         if (memberJson.isNull()) {
        //             // Remove the property if it is null
        //             json.removeMember(memberName);
        //         } else if (memberJson.isObject() || memberJson.isArray()) {
        //             // Recursively handle objects or arrays
        //             RemoveNullValues((BeJsValue&)json.Get(memberName));
        //         }
        //         return false; // End of a branch
        //     });
        // }

        virtual ECSqlStatus OnReadComplete(BeJsValue& instance, PropertyReader::Finder _) override {
            BeAssert(GetFormat() == JsFormat::JsName);
            // If jsonProperties is present, process it
            if (instance.hasMember("jsonProperties")) {
                BeJsDocument doc;
                doc.Parse(instance["jsonProperties"].asCString());
                doc.PurgeNulls();
                auto map = BeJsPath::Extract(BeJsValue(doc), "$.materialAssets.renderMaterial.Map");
                if (map.has_value()) {
                    map.value().ForEachProperty([&](auto memberName, auto memberJson) {
                        if (memberJson.isNumericMember("TextureId")) {
                            // Fix IDs that were previously stored as 64-bit integers rather than as ID strings.
                            auto textureIdAsStringForLogging = memberJson["TextureId"].Stringify();
                            auto textureId = memberJson["TextureId"].GetId64<DgnTextureId>();
                            auto textureIdJson = map->Get(memberName)["TextureId"];
                            (BeJsValue&) textureIdJson = textureId.ToHexStr();
                            if (!textureId.IsValid()) {
                                ECInstanceId elementId = ParseInstanceId();
                                if (elementId.IsValid()) {
                                    LOG.warningv("RenderMaterialId: %s, had a TextureId %s which could not be converted to a valid id.", elementId.ToHexStr().c_str(), textureIdAsStringForLogging.c_str());
                                    BeAssert(false && "RenderMaterial had a textureId that we converted to invalid.");
                                }
                            }
                        }
                        return false;
                    });
                    instance["jsonProperties"] = doc.Stringify();
                }
            }
            return ECSqlStatus::Success;
        }
    };
}

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
bool RegisterBisCoreHandlers(DgnDbR db) {
    bool rc = true;
    auto& repo = db.GetInstanceRepository();
    rc &= repo.RegisterClassHandler<Handlers::Element>(Handlers::Element::ClassName,
                                                       {
                                                           "CodeValue",
                                                           "CodeScope",
                                                           "CodeSpec",
                                                           "Model",
                                                           "Category",
                                                       });
    rc &= repo.RegisterClassHandler<Handlers::GeometricElement3d>(Handlers::GeometricElement3d::ClassName,
                                                                  {"Origin",
                                                                   "Yaw",
                                                                   "Pitch",
                                                                   "Roll",
                                                                   "BBoxLow",
                                                                   "BBoxHigh",
                                                                   "GeometryStream"});
    rc &= repo.RegisterClassHandler<Handlers::GeometricElement2d>(Handlers::GeometricElement2d::ClassName,
                                                                  {"Origin",
                                                                   "Rotation",
                                                                   "BBoxLow",
                                                                   "BBoxHigh",
                                                                   "GeometryStream"});
    rc &= repo.RegisterClassHandler<Handlers::RenderMaterial>(Handlers::RenderMaterial::ClassName);
    rc &= repo.RegisterClassHandler<Handlers::Model>(Handlers::Model::ClassName);
    return rc;
}