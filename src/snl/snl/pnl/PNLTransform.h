#ifndef __PNL_TRANSFORM_H_
#define __PNL_TRANSFORM_H_

#include "PNLBox.h"
#include "PNLOrientation.h"
#include "PNLPoint.h"
#include "PNLUnit.h"

namespace naja {
namespace PNL {

class PNLTransform {
 public:
  static constexpr int A[8] = {1, 0, -1, 0, -1, 0, 1, 0};
  static constexpr int B[8] = {0, -1, 0, 1, 0, -1, 0, 1};
  static constexpr int C[8] = {0, 1, 0, -1, 0, -1, 0, 1};
  static constexpr int D[8] = {1, 0, -1, 0, 1, 0, -1, 0};
  static constexpr int DISCREMINENT[8] = {1, 1, 1, 1, -1, -1, -1, -1};

  PNLTransform() = default;
  PNLTransform(const PNLPoint& offset, const PNLOrientation& orientation)
      : offset_(offset), orientation_(orientation) {}

  PNLPoint getOffset() const { return offset_; }
  PNLOrientation getOrientation() const { return orientation_; }
  PNLUnit::Unit getX(const PNLUnit::Unit& x, const PNLUnit::Unit& y) const
  // **********************************************************
  {
    return (x * A[(int) orientation_.getType()]) + (y * B[(int) orientation_.getType()]) + offset_.getX();
  }

  PNLUnit::Unit getY(const PNLUnit::Unit& x, const PNLUnit::Unit& y) const
  // **********************************************************
  {
    return (x * C[(int) orientation_.getType()]) + (y * D[(int) orientation_.getType()]) + offset_.getY();
  }
  PNLBox getBox(const PNLUnit::Unit& x1,
                const PNLUnit::Unit& y1,
                const PNLUnit::Unit& x2,
                const PNLUnit::Unit& y2) const {
    return PNLBox(getX(x1, y1), getY(x1, y1), getX(x2, y2), getY(x2, y2));
  }
  PNLBox getBox(const PNLBox& box) const {
    if (box.isEmpty())
      return box;
    return getBox(box.getXMin(), box.getYMin(), box.getXMax(), box.getYMax());
  }

 private:
  PNLPoint offset_{0, 0};
  PNLOrientation orientation_;
};

}  // namespace PNL
}  // namespace naja

#endif  // __PNL_TRANSßFORM_H_