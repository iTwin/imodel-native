/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

struct ElementMeshWorker : DgnDbWorker {
private:
  // Input
  DgnElementId m_elementId;
  ElementMeshOptions m_options;

  // Output

  void Execute() final;
  void OnOK() final;

  ElementMeshWorker(DgnDbR db, Napi::Env env, DgnElementId elementId, ElementMeshOptions const& options)
    : DgnDbWorker(db, env), m_elementId(elementId), m_options(options) {
  }
public:
  static DgnDbWorkerPtr Create(DgnDbR db, Napi::CallbackInfo const& info);
};

