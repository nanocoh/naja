#ifndef __PNL_POINT_H_
#define __PNL_POINT_H_

#include "PNLUnit.h"

namespace naja { namespace PNL {

class PNLPoint {
  public:
    PNLPoint(PNLUnit::Unit x, PNLUnit::Unit y): x_(x), y_(y) {}
    PNLUnit::Unit getX() const { return x_; }
    PNLUnit::Unit getY() const { return y_; }
    // comperators
    bool operator==(const PNLPoint& other) const {
      return x_ == other.x_ && y_ == other.y_;
    }
    bool operator!=(const PNLPoint& other) const {
      return !(*this == other);
    }
    bool operator<(const PNLPoint& other) const {
      return x_ < other.x_ || (x_ == other.x_ && y_ < other.y_);
    }
    bool operator>(const PNLPoint& other) const {
      return other < *this;
    }
    bool operator<=(const PNLPoint& other) const {
      return !(*this > other);
    }
    bool operator>=(const PNLPoint& other) const {
      return !(*this < other);
    }
  private:
    PNLUnit::Unit x_;
    PNLUnit::Unit y_;
};


}} // namespace PNL // namespace naja

#endif // __PNL_POINT_H_