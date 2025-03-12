// ****************************************************************************************************
// File: ./hurricane/PNLBox.h
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

#ifndef PNL_BOX
#define PNL_BOX

#include "PNLPoint.h"
#include "PNLUnit.h"

using namespace std;

namespace naja { namespace PNL {

// ****************************************************************************************************
// PNLBox declaration
// ****************************************************************************************************

class PNLBox {
// ******

// Attributes
// **********

    private: PNLUnit::Unit _xMin;
    private: PNLUnit::Unit _yMin;
    private: PNLUnit::Unit _xMax;
    private: PNLUnit::Unit _yMax;

// constructors
// ************

    public: PNLBox();

    public: PNLBox(const PNLUnit::Unit& x, const PNLUnit::Unit& y);
    public: PNLBox(const PNLPoint& PNLPoint);
    public: PNLBox(const PNLUnit::Unit& x1, const PNLUnit::Unit& y1, const PNLUnit::Unit& x2, const PNLUnit::Unit& y2);
    public: PNLBox(const PNLPoint& PNLPoint1, const PNLPoint& PNLPoint2);

    public: PNLBox(const PNLBox& box);

// Operators
// *********

    public: PNLBox& operator=(const PNLBox& box);

    public: bool operator==(const PNLBox& box) const;
    public: bool operator!=(const PNLBox& box) const;

// Accessors
// *********

    public: const PNLUnit::Unit& getXMin() const {return _xMin;};
    public: const PNLUnit::Unit& getYMin() const {return _yMin;};
    public: const PNLUnit::Unit& getXMax() const {return _xMax;};
    public: const PNLUnit::Unit& getYMax() const {return _yMax;};

    public: PNLUnit::Unit getXCenter() const {return ((_xMin + _xMax) / 2);};
    public: PNLUnit::Unit getYCenter() const {return ((_yMin + _yMax) / 2);};
    public: PNLPoint getCenter() const {return PNLPoint(getXCenter(), getYCenter());};
    public: PNLPoint getCornerBL() const { return PNLPoint(_xMin,_yMin); }
    public: PNLPoint getCornerTL() const { return PNLPoint(_xMin,_yMax); }
    public: PNLPoint getCornerTR() const { return PNLPoint(_xMax,_yMax); }
    public: PNLPoint getCornerBR() const { return PNLPoint(_xMax,_yMin); }

    public: PNLUnit::Unit getWidth() const {return (_xMax - _xMin);};
    public: PNLUnit::Unit getHalfWidth() const {return (getWidth() / 2);};
    public: PNLUnit::Unit getHeight() const {return (_yMax - _yMin);};
    public: PNLUnit::Unit getHalfHeight() const {return (getHeight() / 2);};

    public: PNLBox getUnion(const PNLBox& box) const;

    public: PNLBox getIntersection(const PNLBox& box) const;
    public: PNLUnit::Unit manhattanDistance(const PNLPoint& pt) const;
    public: PNLUnit::Unit manhattanDistance(const PNLBox& box) const;

// Predicates
// **********

    public: bool isEmpty() const;
    public: bool isFlat() const;
    public: bool isPonctual() const;

    public: bool contains(const PNLUnit::Unit& x, const PNLUnit::Unit& y) const;
    public: bool contains(const PNLPoint& PNLPoint) const;
    public: bool contains(const PNLBox& box) const;

    public: bool intersect(const PNLBox& box) const;

    public: bool isConstrainedBy(const PNLBox& box) const;

// Updators
// ********

    public: PNLBox& makeEmpty();

    public: PNLBox& inflate(const PNLUnit::Unit& d);
    public: PNLBox& inflate(const PNLUnit::Unit& dx, const PNLUnit::Unit& dy);
    public: PNLBox& inflate(const PNLUnit::Unit& dxMin, const PNLUnit::Unit& dyMin, const PNLUnit::Unit& dxMax, const PNLUnit::Unit& dyMax);
    public: PNLBox  getInflated(const PNLUnit::Unit& d) const;
    public: PNLBox& shrinkByFactor(double factor);   // 0 <= factor <= 1

    public: PNLBox& merge(const PNLUnit::Unit& x, const PNLUnit::Unit& y);
    public: PNLBox& merge(const PNLPoint& PNLPoint);
    public: PNLBox& merge(const PNLUnit::Unit& x1, const PNLUnit::Unit& y1, const PNLUnit::Unit& x2, const PNLUnit::Unit& y2);
    public: PNLBox& merge(const PNLBox& box);

    public: PNLBox& translate(const PNLUnit::Unit& dx, const PNLUnit::Unit& dy);

// Others
// ******


    //public: string _getTypeName() const { return _TName("PNLBox"); };
    public: string _getString() const;
    // public: Record* _getRecord() const;
    // public: void toJson(JsonWriter*) const;

};


// class JsonPNLBox : public JsonObject {
// // ********************************

//   public: static void initialize();
//   public: JsonPNLBox(unsigned long);
//   public: virtual string getTypeName() const;
//   public: virtual JsonPNLBox* clone(unsigned long) const;
//   public: virtual void toData(JsonStack&); 
// };


} } // namespace PNL // namespace naja

// INSPECTOR_PR_SUPPORT(Hurricane::PNLBox);


#endif // PNL_BOX


// ****************************************************************************************************
// Copyright (c) BULL S.A. 2000-2018, All Rights Reserved
// ****************************************************************************************************
