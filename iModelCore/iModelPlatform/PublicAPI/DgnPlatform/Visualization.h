/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/WString.h>
#include "DgnPlatform.h"
#include "Render.h"
#include <memory>

// Some of our build machines don't currently supply std::variant.
#ifdef BENTLEY_HAVE_VARIANT
#include <variant>
#endif

BEGIN_BENTLEY_DGN_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Visualization);
using VisualizationUPtr = std::unique_ptr<Visualization>;

DEFINE_POINTER_SUFFIX_TYPEDEFS(TileContent);
DEFINE_REF_COUNTED_PTR(TileContent);

DEFINE_POINTER_SUFFIX_TYPEDEFS(ElementGraphicsRequests);
using ElementGraphicsRequestsUPtr = std::unique_ptr<ElementGraphicsRequests>;

struct TileContent : RefCountedBase {
private:
  ByteStream m_bytes;
public:
  explicit TileContent(ByteStream&& bytes) : m_bytes(std::move(bytes)) { }

  ByteStreamCR GetBytes() const { return m_bytes; }
};

// An object supplied by Visualization::CreateElementGraphicsRequests to fulfill requests to produce graphics for individual elements or geometry streams.
struct ElementGraphicsRequests {
  virtual ~ElementGraphicsRequests() { }

  // Invoked from main thread when DgnDb is about to be closed.
  virtual void Terminate() = 0;

  // Invoked from main thread to mark the specified requests as cancelled.
  virtual void Cancel(bvector<Utf8String> const& requestIds) = 0;

  // Invoked from the main thread  to enqueue a request for element graphics. Returns a Promise<ElementGraphicsResult>.
  virtual Napi::Value Enqueue(BeJsConst requestProps, Napi::Env env) = 0;
};

struct Visualization {
  enum class PollState {
    New, // Request was just created and enqueued
    Pending, // Request is already queued but not yet being processed
    Loading, // Request is being processed on worker thread
  };

  struct PolledTileContent {
    TileContentCPtr m_content;
    double m_elapsedSeconds;

    PolledTileContent(TileContentCR content, double elapsedSeconds) : m_content(&content), m_elapsedSeconds(elapsedSeconds) { }

#ifndef BENTLEY_HAVE_VARIANT
    PolledTileContent() : m_elapsedSeconds(0) { }
    bool IsValid() const { return m_content.IsValid(); }
#endif
  };

#ifdef BENTLEY_HAVE_VARIANT
  using TileContentResult = std::variant<TileContentCPtr, DgnDbStatus>;
  using PollContentResult = std::variant<PollState, PolledTileContent, DgnDbStatus>;
#else
  struct TileContentResult {
    TileContentCPtr m_content;
    DgnDbStatus m_status;

    TileContentResult(TileContentCPtr content) : m_content(content), m_status(DgnDbStatus::Success) { BeAssert(content.IsValid()); }
    TileContentResult(DgnDbStatus status) : m_status(status) { BeAssert(DgnDbStatus::Success != status); }
  };

  struct PollContentResult {
    PolledTileContent m_content;
    DgnDbStatus m_status;
    PollState m_state;

    PollContentResult(PolledTileContent&& content) : m_content(content), m_status(DgnDbStatus::Success) { }
    PollContentResult(DgnDbStatus status) : m_status(status) { BeAssert(DgnDbStatus::Success != m_status); }
    PollContentResult(PollState state) : m_status(DgnDbStatus::Success), m_state(state) { }
  };
#endif

private:
  Render::System* m_renderSystem;
public:
  explicit Visualization(Render::System* renderSystem = nullptr) : m_renderSystem(renderSystem) { }

  Render::System* GetRenderSystem() { return m_renderSystem; }

  virtual ~Visualization() { }
  virtual uint32_t GetCurrentFormatVersion() { return 0; }

  virtual uint64_t GetMaxTileCacheSize() const { return 0; }
  virtual void SetMaxTileCacheSize(uint64_t maxSizeInBytes) { }
  bool IsTileCacheEnabled() const { return GetMaxTileCacheSize() > 0; }

  virtual void SetTrackingGeometry(GeometricModelR, bool isTracking) { }
  virtual void AddChanges(GeometricModelR, bset<DgnElementId> const& inserts, bset<DgnElementId> const& updates, bset<DgnElementId> const& deletes) { }

  virtual DgnDbStatus GetTileTreeJson(BeJsValue treeJson, DgnDbR db, Utf8StringCR treeId, bool createIfNotFound) { return DgnDbStatus::NotFound; }
  virtual void PurgeTileTrees(DgnDbR db, bvector<DgnModelId> const* modelIds) { }

  virtual TileContentResult GetTileContent(DgnDbR db, Utf8StringCR treeId, Utf8StringCR contentId) { return DgnDbStatus::NotFound; }
  virtual PollContentResult PollTileContent(ICancellableP, DgnDbR db, Utf8StringCR treeId, Utf8StringCR contentId) { return DgnDbStatus::NotFound; }
  virtual void CancelContentRequests(DgnDbR db, Utf8StringCR treeId, bvector<Utf8String> const& contentIds) { }

  virtual ElementGraphicsRequestsUPtr CreateElementGraphicsRequests(DgnDbR) { return nullptr; }
};

END_BENTLEY_DGN_NAMESPACE

