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
  ChunkType m_type;
  // The length of the chunk, excluding the fixed length of this header.
  uint32_t m_dataLength;

  ChunkHeader(ChunkType type, uint32_t length) : m_type(type), m_dataLength(length) { }
};

static_assert(sizeof(ChunkHeader) == 8, "unexpected ChunkHeader size");

struct Chunk : ChunkHeader {
  uint8_t const* m_data;

  Chunk(ChunkType type, uint32_t length, uint8_t const* data) : ChunkHeader(type, length), m_data(data) { }

  uint32_t GetTotalSize() const {
    return sizeof(ChunkHeader) + m_dataLength;
  }
};

struct ElementMeshWorker : DgnDbWorker {
private:
  // Input
  DgnElementId m_elementId;
  ElementMeshOptions m_options;

  // Output
  ByteStream m_result;

  void Execute() final;
  void OnOK() final;

  ElementMeshWorker(DgnDbR db, Napi::Env env, DgnElementId elementId, ElementMeshOptions const& options)
    : DgnDbWorker(db, env), m_elementId(elementId), m_options(options) {
    Append(Chunk(ChunkType::ElementMeshes, 0, nullptr));
  }

  void Append(Chunk const& chunk) {
    BeAssert((nullptr == chunk.m_data) == (chunk.m_dataLength == 0));

    m_result.Reserve(m_result.GetSize() + chunk.GetTotalSize());
    m_result.Append(static_cast<ChunkHeader>(chunk));
    if (nullptr != chunk.m_data)
      m_result.Append(chunk.m_data, chunk.m_dataLength);
  }

  void Append(PolyfaceQueryCR polyface) {
    bvector<uint8_t> flatBuffer;
    BentleyGeometryFlatBuffer::GeometryToBytes(polyface, flatBuffer);
    if (!flatBuffer.empty())
      Append(Chunk(ChunkType::Polyface, static_cast<uint32_t>(flatBuffer.size()), &flatBuffer[0]));
  }
public:
  static DgnDbWorkerPtr Create(DgnDbR db, Napi::CallbackInfo const& info);
};

