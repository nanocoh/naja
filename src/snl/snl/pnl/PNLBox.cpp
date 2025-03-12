// ****************************************************************************************************
// File: ./PNLBox.cpp
// Authors: R. Escassut
// Copyright (c) BULL S.A. 2000-2018, All Rights Reserved
//
// This file is part of Hurricane.
//
// Hurricane is free software: you can redistribute it  and/or  modify it under the  terms  of the  GNU
// Lesser General Public License as published by the Free Software Foundation, either version 3 of  the
// License, or (at your option) any later version.
//
// Hurricane is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without  even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A  PARTICULAR  PURPOSE. See  the  Lesser  GNU
// General Public License for more details.
//
// You should have received a copy of the Lesser GNU General Public License along  with  Hurricane.  If
// not, see <http://www.gnu.org/licenses/>.
// ****************************************************************************************************

#include "PNLBox.h"
#include <cassert>

namespace naja { namespace PNL {

// ****************************************************************************************************
// PNLBox implementation
// ****************************************************************************************************

PNLBox::PNLBox()
// *******
    : _xMin(1),
    _yMin(1),
    _xMax(-1),
    _yMax(-1)
{}

PNLBox::PNLBox(const PNLUnit::Unit& x, const PNLUnit::Unit& y)
// ***********************************
    : _xMin(x),
    _yMin(y),
    _xMax(x),
    _yMax(y)
{}

PNLBox::PNLBox(const PNLPoint& PNLPoint)
// *************************
    : _xMin(PNLPoint.getX()),
    _yMin(PNLPoint.getY()),
    _xMax(PNLPoint.getX()),
    _yMax(PNLPoint.getY())
{}

PNLBox::PNLBox(const PNLUnit::Unit& x1, const PNLUnit::Unit& y1, const PNLUnit::Unit& x2, const PNLUnit::Unit& y2)
// *********************************************************************
    : _xMin(min(x1, x2)),
    _yMin(min(y1, y2)),
    _xMax(max(x1, x2)),
    _yMax(max(y1, y2))
{}

PNLBox::PNLBox(const PNLPoint& PNLPoint1, const PNLPoint& PNLPoint2)
// ***********************************************
    : _xMin(min(PNLPoint1.getX(), PNLPoint2.getX())),
    _yMin(min(PNLPoint1.getY(), PNLPoint2.getY())),
    _xMax(max(PNLPoint1.getX(), PNLPoint2.getX())),
    _yMax(max(PNLPoint1.getY(), PNLPoint2.getY()))
{}

PNLBox::PNLBox(const PNLBox& box)
// *********************
    : _xMin(box._xMin),
    _yMin(box._yMin),
    _xMax(box._xMax),
    _yMax(box._yMax)
{}

PNLBox& PNLBox::operator=(const PNLBox& box)
// ********************************
{
    _xMin = box._xMin;
    _yMin = box._yMin;
    _xMax = box._xMax;
    _yMax = box._yMax;
    return *this;
}

bool PNLBox::operator==(const PNLBox& box) const
// ***************************************
{
    return (!isEmpty() &&
              !box.isEmpty() &&
              (_xMin == box._xMin) &&
              (_yMin == box._yMin) &&
              (_xMax == box._xMax) &&
              (_yMax == box._yMax));
}

bool PNLBox::operator!=(const PNLBox& box) const
// ***************************************
{
    return (isEmpty() ||
              box.isEmpty() ||
              (_xMin != box._xMin) ||
              (_yMin != box._yMin) ||
              (_xMax != box._xMax) ||
              (_yMax != box._yMax));
}

PNLBox PNLBox::getUnion(const PNLBox& box) const
// ************************************
{
    if (isEmpty() && box.isEmpty()) return PNLBox();
    return PNLBox(min(_xMin, box._xMin),
                  min(_yMin, box._yMin),
                  max(_xMax, box._xMax),
                  max(_yMax, box._yMax));
}

PNLBox PNLBox::getIntersection(const PNLBox& box) const
// *******************************************
{
    if (!intersect(box)) return PNLBox();
    return PNLBox(max(_xMin, box._xMin),
                  max(_yMin, box._yMin),
                  min(_xMax, box._xMax),
                  min(_yMax, box._yMax));
}

PNLUnit::Unit PNLBox::manhattanDistance(const PNLPoint& pt) const
// ***********************************************
{
    PNLUnit::Unit dist = 0;
    if (isEmpty()) {
        //throw Error("Can't compute distance to an empty PNLBox");
        assert(false);
    }
    if (pt.getX() < _xMin) dist = _xMin - pt.getX();
    else if (pt.getX() > _xMax) dist = pt.getX() - _xMax;
    // else                     dist = 0;
    if (pt.getY() < _yMin) dist += _yMin - pt.getY();
    else if (pt.getY() > _yMax) dist += pt.getY() - _yMax;
    // else                     dist += 0;
    return dist;
}

PNLUnit::Unit PNLBox::manhattanDistance(const PNLBox& box) const
// **********************************************
{
    if (isEmpty() || box.isEmpty()) {
        //throw Error("Can't compute distance to an empty PNLBox");
        assert(false);
    }
    PNLUnit::Unit dx, dy;
    if ((dx=box.getXMin() - _xMax) < 0)
        if ((dx=_xMin-box.getXMax()) < 0) dx=0;
    if ((dy=box.getYMin() - _yMax) < 0)
        if ((dy=_yMin-box.getYMax()) < 0) dy=0;
    return dx+dy;
}

bool PNLBox::isEmpty() const
// **********************
{
    return ((_xMax < _xMin) || (_yMax < _yMin));
}

bool PNLBox::isFlat() const
// *********************
{
    return (!isEmpty() &&
              (((_xMin == _xMax) && (_yMin < _yMax)) ||
               ((_xMin < _xMax) && (_yMin == _yMax))));
}

bool PNLBox::isPonctual() const
// *************************
{
    return (!isEmpty() && (_xMax == _xMin) && (_yMax == _yMin));
}

bool PNLBox::contains(const PNLUnit::Unit& x, const PNLUnit::Unit& y) const
// ***************************************************
{
    return (!isEmpty() &&
              (_xMin <= x) &&
              (_yMin <= y) &&
              (x <= _xMax) &&
              (y <= _yMax));
}

bool PNLBox::contains(const PNLPoint& PNLPoint) const
// *****************************************
{
    return contains(PNLPoint.getX(), PNLPoint.getY());
}

bool PNLBox::contains(const PNLBox& box) const
// *************************************
{
    return (!isEmpty() &&
              !box.isEmpty() &&
              (_xMin <= box._xMin) &&
              (box._xMax <= _xMax) &&
              (_yMin <= box._yMin) &&
              (box._yMax <= _yMax));
}

bool PNLBox::intersect(const PNLBox& box) const
// **************************************
{
    return (!isEmpty() &&
              !box.isEmpty() &&
              !((_xMax < box._xMin) ||
                 (box._xMax < _xMin) ||
                 (_yMax < box._yMin) ||
                 (box._yMax < _yMin)));
}

bool PNLBox::isConstrainedBy(const PNLBox& box) const
// ********************************************
{
    return (!isEmpty() &&
              !box.isEmpty() &&
              ((_xMin == box.getXMin()) ||
                (_yMin == box.getYMin()) ||
                (_xMax == box.getXMax()) ||
                (_yMax == box.getYMax())));
}

PNLBox& PNLBox::makeEmpty()
// ******************
{
  _xMin = 1;
    _yMin = 1;
    _xMax = -1;
    _yMax = -1;
    return *this;
}

PNLBox& PNLBox::inflate(const PNLUnit::Unit& d)
// *****************************
{
    return inflate(d, d, d, d);
}

PNLBox& PNLBox::inflate(const PNLUnit::Unit& dx, const PNLUnit::Unit& dy)
// **********************************************
{
    return inflate(dx, dy, dx, dy);
}

PNLBox& PNLBox::inflate(const PNLUnit::Unit& dxMin, const PNLUnit::Unit& dyMin, const PNLUnit::Unit& dxMax, const PNLUnit::Unit& dyMax)
// ******************************************************************************************
{
    if (!isEmpty()) {
        _xMin -= dxMin;
        _yMin -= dyMin;
        _xMax += dxMax;
        _yMax += dyMax;
    }
    return *this;
}

PNLBox PNLBox::getInflated(const PNLUnit::Unit& d) const {
    return PNLBox(*this).inflate(d);
}

PNLBox& PNLBox::shrinkByFactor(double factor)
// **************************************
{
    assert((0.0 <= factor) && (factor <= 1.0));
    PNLUnit::Unit dx = PNLUnit::grid ( 0.5 * (1-factor) * (PNLUnit::getGrid(_xMax) - PNLUnit::getGrid(_xMin)) );
    PNLUnit::Unit dy = PNLUnit::grid ( 0.5 * (1-factor) * (PNLUnit::getGrid(_yMax) - PNLUnit::getGrid(_yMin)) );

  //PNLUnit::Unit dx=getUnit(0.5*(1- factor) * (getValue(_xMax) - getValue(_xMin)));
  //PNLUnit::Unit dy=getUnit(0.5*(1- factor) * (getValue(_yMax) - getValue(_yMin)));
    return inflate(-dx, -dy);
}

PNLBox& PNLBox::merge(const PNLUnit::Unit& x, const PNLUnit::Unit& y)
// ******************************************
{
    if (isEmpty()) {
        _xMin = x;
        _yMin = y;
        _xMax = x;
        _yMax = y;
    }
    else {
        _xMin = min(_xMin, x);
        _yMin = min(_yMin, y);
        _xMax = max(_xMax, x);
        _yMax = max(_yMax, y);
    }
    return *this;
}

PNLBox& PNLBox::merge(const PNLPoint& PNLPoint)
// ********************************
{
    return merge(PNLPoint.getX(), PNLPoint.getY());
}

PNLBox& PNLBox::merge(const PNLUnit::Unit& x1, const PNLUnit::Unit& y1, const PNLUnit::Unit& x2, const PNLUnit::Unit& y2)
// ****************************************************************************
{
    merge(x1, y1);
    merge(x2, y2);
    return *this;
}

PNLBox& PNLBox::merge(const PNLBox& box)
// ****************************
{
    if (!box.isEmpty()) {
        merge(box.getXMin(), box.getYMin());
        merge(box.getXMax(), box.getYMax());
    }
    return *this;
}

PNLBox& PNLBox::translate(const PNLUnit::Unit& dx, const PNLUnit::Unit& dy)
// **********************************************************
{
    if (!isEmpty()) {
        _xMin += dx;
        _yMin += dy;
        _xMax += dx;
        _yMax += dy;
    }
    return *this;
}

// string PNLBox::_getString() const
// // ***************************
// {
//   if (isEmpty())
//     return "<" + _TName("PNLBox") + " empty>";
//   else
//     return "<" + _TName("PNLBox") + " "
//       + PNLUnit::getValueString(_xMin) + " " + PNLUnit::getValueString(_yMin) + " "
//       + PNLUnit::getValueString(_xMax) + " " + PNLUnit::getValueString(_yMax) + ">";
// }

// Record* PNLBox::_getRecord() const
// // **********************
// {
//     if (isEmpty()) return NULL;

//     Record* record = new Record(getString(this));
//     record->add(PNLUnit::getValueSlot("XMin", &_xMin));
//     record->add(PNLUnit::getValueSlot("YMin", &_yMin));
//     record->add(PNLUnit::getValueSlot("XMax", &_xMax));
//     record->add(PNLUnit::getValueSlot("YMax", &_yMax));
//     return record;
// }

// void  PNLBox::toJson(JsonWriter* w) const
// // ***********************************
// {
//   w->startObject();
//   jsonWrite( w, "@typename", "PNLBox" );
//   jsonWrite( w, "_xMin", getXMin() );
//   jsonWrite( w, "_yMin", getYMin() );
//   jsonWrite( w, "_xMax", getXMax() );
//   jsonWrite( w, "_yMax", getYMax() );
//   w->endObject();
// }


// Initializer<JsonPNLBox>  jsonPNLBoxInit ( 0 );

// void  JsonPNLBox::initialize()
// // **************************
// { JsonTypes::registerType( new JsonPNLBox (JsonWriter::RegisterMode) ); }

// JsonPNLBox::JsonPNLBox(unsigned long flags)
// // **********************************
//   : JsonObject(flags)
// { 
//   add( "_xMin", typeid(int64_t) );
//   add( "_yMin", typeid(int64_t) );
//   add( "_xMax", typeid(int64_t) );
//   add( "_yMax", typeid(int64_t) );
// }

// string  JsonPNLBox::getTypeName() const
// // *********************************
// { return "PNLBox"; }

// JsonPNLBox* JsonPNLBox::clone(unsigned long flags) const
// // ***********************************************
// { return new JsonPNLBox ( flags ); }

// void JsonPNLBox::toData(JsonStack& stack)
// // ***********************************
// {
//   check( stack, "JsonPNLBox::toData" );

//   PNLUnit::Unit xMin = PNLUnit::fromDb(get<int64_t>(stack,"_xMin"));
//   PNLUnit::Unit yMin = PNLUnit::fromDb(get<int64_t>(stack,"_yMin"));
//   PNLUnit::Unit xMax = PNLUnit::fromDb(get<int64_t>(stack,"_xMax"));
//   PNLUnit::Unit yMax = PNLUnit::fromDb(get<int64_t>(stack,"_yMax"));

//   PNLBox box;
  
//   if ( (xMin <= xMax) and (yMin <= yMax) )
//     box.merge( xMin, yMin, xMax, yMax ); 

//   cdebug_log(19,0) << "PNLBox(" << xMin << ", "
//                  <<           yMin << ", "
//                  <<           xMax << ", "
//                  <<           yMax << ")" << endl;

//   update( stack, box );
// }

} } // namespace naja::PNL


// ****************************************************************************************************
// Copyright (c) BULL S.A. 2000-2018, All Rights Reserved
// ****************************************************************************************************
