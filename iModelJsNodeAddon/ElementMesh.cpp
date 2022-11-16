/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <GeomSerialization/GeomLibsFlatBufferApi.h>
#include "IModelJsNative.h"
#include "DgnDbWorker.h"

using namespace IModelJsNative;

struct ElementMeshOptions {
  double m_chordTol;
  double m_angleTol;
  double m_decimationTol;
  double m_minBRepFeatureSize;

  explicit ElementMeshOptions(BeJsConst);
};

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

struct ElementMeshWorker : DgnDbWorker {
private:
  // Input
  DgnElementId m_elementId;
  ElementMeshOptions m_options;

  // Output
  ChunkStream m_result;

  void Execute() final;
  void OnOK() final;

  ElementMeshWorker(DgnDbR db, Napi::Env env, DgnElementId elementId, ElementMeshOptions const& options)
    : DgnDbWorker(db, env), m_elementId(elementId), m_options(options) {
    m_result.AppendChunk(ChunkType::ElementMeshes, 0, nullptr);
  }

  void Append(PolyfaceQueryCR polyface) {
    bvector<uint8_t> flatBuffer;
    BentleyGeometryFlatBuffer::GeometryToBytes(polyface, flatBuffer);
    if (!flatBuffer.empty())
      m_result.AppendChunk(ChunkType::Polyface, static_cast<uint32_t>(flatBuffer.size()), &flatBuffer[0]);
  }
public:
  static DgnDbWorkerPtr Create(DgnDbR db, Napi::CallbackInfo const& info);
};

