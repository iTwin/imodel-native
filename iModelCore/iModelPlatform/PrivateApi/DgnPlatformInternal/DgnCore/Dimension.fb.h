//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_DIMENSION_BENTLEYG0601_DGN_FB_H_
#define FLATBUFFERS_GENERATED_DIMENSION_BENTLEYG0601_DGN_FB_H_

#include "flatbuffers/flatbuffers.h"


namespace BentleyM0200 {
namespace Dgn {
namespace FB {

struct DPoint2d;
struct DimensionPoints;

MANUALLY_ALIGNED_STRUCT(8) DPoint2d {
 private:
  double x_;
  double y_;

 public:
  DPoint2d(double x, double y)
    : x_(flatbuffers::EndianScalar(x)), y_(flatbuffers::EndianScalar(y)) { }

  double x() const { return flatbuffers::EndianScalar(x_); }
  double y() const { return flatbuffers::EndianScalar(y_); }
};
STRUCT_END(DPoint2d, 16);

typedef bvector<DPoint2d> DPoint2ds;
typedef flatbuffers::Vector<DPoint2d const*> DPoint2dVector;
typedef flatbuffers::Offset<DPoint2dVector> DPoint2dVectorOffset;

struct DimensionPoints : private flatbuffers::Table {
  const flatbuffers::Vector<const DPoint2d *> *coords() const { return GetPointer<const flatbuffers::Vector<const DPoint2d *> *>(4); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, 4 /* coords */) &&
           verifier.Verify(coords()) &&
           verifier.EndTable();
  }
  bool has_coords() const { return CheckField(4); }
};

struct DimensionPointsBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_coords(flatbuffers::Offset<flatbuffers::Vector<const DPoint2d *>> coords) { fbb_.AddOffset(4, coords); }
  DimensionPointsBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  DimensionPointsBuilder &operator=(const DimensionPointsBuilder &);
  flatbuffers::Offset<DimensionPoints> Finish() {
    auto o = flatbuffers::Offset<DimensionPoints>(fbb_.EndTable(start_, 1));
    return o;
  }
};

inline flatbuffers::Offset<DimensionPoints> CreateDimensionPoints(flatbuffers::FlatBufferBuilder &_fbb,
   flatbuffers::Offset<flatbuffers::Vector<const DPoint2d *>> coords = 0) {
  DimensionPointsBuilder builder_(_fbb);
  builder_.add_coords(coords);
  return builder_.Finish();
}

}  // namespace FB
}  // namespace Dgn
}  // namespace BentleyM0200

#endif  // FLATBUFFERS_GENERATED_DIMENSION_BENTLEYG0601_DGN_FB_H_
