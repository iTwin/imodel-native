/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "IModelJsNative.h"
#include <GeomSerialization/GeomLibsFlatBufferApi.h>
#include <DgnPlatform/SimplifyGraphic.h>
#include "DgnDbWorker.h"

using namespace IModelJsNative;

enum class ChunkType : uint32_t {
  ElementMeshes = 'HSML',
  Polyface = 'CFLP',
};

struct ChunkHeader {
  ChunkType type;
  uint32_t length;
};

static_assert(sizeof(ChunkHeader) == 8, "unexpected size");

struct ChunkStream : ByteStream {
private:
  void AppendHeader(ChunkType type, uint32_t dataLength) {
    ChunkHeader hdr = { type, dataLength };
    Append(hdr);
  }

public:
  void AppendChunk(ChunkType type, uint32_t dataLength, uint8_t const* data) {
    BeAssert((nullptr == data) == (0 == dataLength));

    Reserve(GetSize() + dataLength + sizeof(ChunkHeader));
    AppendHeader(type, dataLength);
    if (nullptr != data)
      Append(data, dataLength);
  }
};

struct ElementMeshProcessor : IGeometryProcessor {
private:
  ChunkStream& m_output;
  IFacetOptionsR m_facetOptions;

  IFacetOptionsP _GetFacetOptionsP() override {return &m_facetOptions;}
  UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&) const final {return UnhandledPreference::Facet;}
  UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const final {return UnhandledPreference::Facet;}
  UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR, SimplifyGraphic&) const final {return UnhandledPreference::Facet;}
  UnhandledPreference _GetUnhandledPreference(PolyfaceQueryCR, SimplifyGraphic&) const final {return UnhandledPreference::Ignore;}
  UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&) const final {return UnhandledPreference::Facet;}

  bool _ProcessPolyface(PolyfaceQueryCR inputPf, bool filled, SimplifyGraphic& gf) final {
    JsInterop::ProcessPolyface(inputPf, false, [&](PolyfaceQueryCR pf) {
      bvector<uint8_t> flatBuffer;

      auto tf = gf.GetLocalToWorldTransform();
      if (!tf.IsIdentity()) {
        auto transformedPf = pf.Clone();
        transformedPf->Transform(tf);
        BentleyGeometryFlatBuffer::GeometryToBytes(*transformedPf, flatBuffer);
      } else {
        BentleyGeometryFlatBuffer::GeometryToBytes(pf, flatBuffer);
      }

      if (!flatBuffer.empty())
        m_output.AppendChunk(ChunkType::Polyface, static_cast<uint32_t>(flatBuffer.size()), &flatBuffer[0]);
    });

    return true;
  }
public:
  ElementMeshProcessor(ChunkStream& output, IFacetOptionsR facetOptions) : m_output(output), m_facetOptions(facetOptions) { }
  virtual ~ElementMeshProcessor() { }
};

struct ElementMeshWorker : DgnDbWorker {
private:
  // Input
  DgnElementId m_elementId;
  IFacetOptionsPtr m_facetOptions;

  // Output
  ChunkStream m_result;

  void Execute() final;
  void OnOK() final;

  ElementMeshWorker(DgnDbR db, Napi::Env env, DgnElementId elementId, IFacetOptionsR facetOptions)
    : DgnDbWorker(db, env), m_elementId(elementId), m_facetOptions(&facetOptions) {
    m_result.AppendChunk(ChunkType::ElementMeshes, 0, nullptr);
  }
public:
  static DgnDbWorkerPtr Create(DgnDbR db, Napi::Object const& obj);
};

DgnDbWorkerPtr ElementMeshWorker::Create(DgnDbR db, Napi::Object const& obj) {
  BeJsConst props(obj);

  double chordTol = props["chordTolerance"].asDouble(0.0);
  double angleTol = props["angleTolerance"].asDouble(msGeomConst_piOver12);

  // Per Earlin's advice on avoiding topology problems, restrict max angle tolerance to 45 deg
  angleTol = std::min(msGeomConst_piOver4, angleTol);

  double maxEdgeLen = 0.0;
  bool triangulate = true;
  bool normals = false;
  bool params = false;
  auto opts = IFacetOptions::CreateForSurfaces(chordTol, angleTol, maxEdgeLen, triangulate, normals, params);

  auto minFeatSzVal = props["minBRepFeatureSize"];
  if (minFeatSzVal.isNumeric())
    opts->SetBRepIgnoredFeatureSize(minFeatSzVal.asDouble());

  opts->SetIgnoreHiddenBRepEntities(true);
  opts->SetBRepConcurrentFacetting(false);

  DgnElementId elemId;
  elemId.FromJson(props["source"]);

  return new ElementMeshWorker(db, obj.Env(), elemId, *opts);
}

void ElementMeshWorker::Execute() {
  auto elem = GetDb().Elements().Get<GeometricElement>(m_elementId);
  auto geom = elem.IsValid() ? elem->ToGeometrySource() : nullptr;
  if (nullptr == geom) {
    SetError("Geometric element required");
    return;
  }

  ElementMeshProcessor processor(m_result, *m_facetOptions);
  GeometryProcessor::Process(processor, *geom);
}

void ElementMeshWorker::OnOK() {
  auto bytes = Napi::Uint8Array::New(Env(), m_result.size());
  memcpy(bytes.Data(), m_result.GetData(), m_result.size());
  m_promise.Resolve(bytes);
}

Napi::Value JsInterop::GenerateElementMeshes(DgnDbR db, Napi::Object const& requestProps) {
  auto worker = ElementMeshWorker::Create(db, requestProps);
  return worker->Queue();
}
