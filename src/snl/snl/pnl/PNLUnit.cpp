// SPDX-FileCopyrightText: 2025 The Naja authors
// <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include <cstring>
#include <cstdlib>
#include <limits>
#include "PNLUnit.h"
#include <cassert>


namespace naja {
    namespace PNL {

  const unsigned int  PNLUnit::_maximalPrecision     = 3;
  unsigned int        PNLUnit::_precision            = 1;
  double              PNLUnit::_resolution           = 0.1;
  double              PNLUnit::_gridsPerLambda       = 10.0;
  double              PNLUnit::_physicalsPerGrid     = 1.0;
  double              PNLUnit::_gridMax              = PNLUnit::toGrid    ( PNLUnit::Max );
  double              PNLUnit::_lambdaMax            = PNLUnit::toLambda  ( PNLUnit::Max );
  double              PNLUnit::_physicalMax          = PNLUnit::toPhysical( PNLUnit::Max, PNLUnit::Unity );
  unsigned int        PNLUnit::_stringMode           = PNLUnit::Symbolic;
  PNLUnit::UnitPower      PNLUnit::_stringModeUnitPower  = PNLUnit::Nano;
  PNLUnit::Unit           PNLUnit::_symbolicSnapGridStep = PNLUnit::fromLambda( 1.0);
  PNLUnit::Unit           PNLUnit::_realSnapGridStep     = PNLUnit::fromGrid  (10.0);
  PNLUnit::Unit           PNLUnit::_polygonStep          = PNLUnit::fromGrid  ( 1.0);
  const PNLUnit::Unit     PNLUnit::Min                   = std::numeric_limits<PNLUnit::Unit>::min();
  const PNLUnit::Unit     PNLUnit::Max                   = std::numeric_limits<PNLUnit::Unit>::max();


// // -------------------------------------------------------------------
// // Class :  "Hurricane::DbUSlot".


//   class DbUSlot : public Slot {

//     public:
//     // Constructor.
//                        DbUSlot       ( const string& name, const PNLUnit::Unit* data );
//                        DbUSlot       (       string& name, const PNLUnit::Unit* data );
//     // Accessors.
//       virtual string   getDataString () const;
//       virtual Record*  getDataRecord () const;
//       virtual DbUSlot* getClone      () const;

//     protected:
//     // Internal: Attributes.
//       const PNLUnit::Unit* _unit;

//     private:
//     // Internal: Constructors.
//                        DbUSlot      ( const DbUSlot& );
//               DbUSlot& operator=    ( const DbUSlot& );
//   };


// // Inline Member Functions.
//            DbUSlot::DbUSlot       ( const string& name, const PNLUnit::Unit* unit ) : Slot(name), _unit(unit) {}
//            DbUSlot::DbUSlot       (       string& name, const PNLUnit::Unit* unit ) : Slot(name), _unit(unit) {}
//   string   DbUSlot::getDataString () const { return PNLUnit::getValueString(*_unit); }
//   Record*  DbUSlot::getDataRecord () const { return PNLUnit::getValueRecord( _unit); }
//   DbUSlot* DbUSlot::getClone      () const { return new DbUSlot(_name,_unit); }


// -------------------------------------------------------------------
// Class :  "Hurricane::DbU".


  void  PNLUnit::_updateBounds ()
  {
    _gridMax     = toGrid    ( Max );
    _lambdaMax   = toLambda  ( Max );
    _physicalMax = toPhysical( Max, Unity );
  }


  void  PNLUnit::checkGridBound ( double value )
  {
    if (value < 0) value = -value;
    if (value >= _gridMax) {
      //throw Error( "Grid value %.1f converts to out of range DbU value (maximum is: %.1f)."
      //           , value, _gridMax );
      assert(false);
    }
  }


  void  PNLUnit::checkLambdaBound ( double value )
  {
    if (value < 0) value = -value;
    if (value >= _lambdaMax) {
     // throw Error( "Lambda value %.1f converts to out of range DbU (maximum is: %.1f)."
       //          , value, _lambdaMax );
        assert(false);
    }
  }


  void  PNLUnit::checkPhysicalBound ( double value, UnitPower p )
  {
    if (value < 0) value = -value;
    value *= getUnitPower(p);
    if (value >= _physicalMax) {
      //throw Error( "Physical value %.1fnm converts to out of range DbU (maximum is: %.1f)."
          //       , (value/1.0e-9), _physicalMax );
        assert(false);
    }
  }


  unsigned int PNLUnit::getPrecision ()
  { return _precision; }


  unsigned int PNLUnit::getMaximalPrecision ()
  { return _maximalPrecision; }


  double  PNLUnit::getResolution ()
  { return _resolution; }


  void PNLUnit::setPrecision ( unsigned int precision, unsigned int flags )
  {
    if ( _maximalPrecision < precision) {
    //   throw Error ( "PNLUnit::Unit::setPrecision(): Precision %ud exceed maximal precision %ud."
    //               , precision
    //               , _maximalPrecision
    //               );
        assert(false);
    }

    //float scale = (float)precision / (float)_precision;

    _precision  = precision;

    _resolution = 1;
    while ( precision-- ) _resolution /= 10;

    //if (not (flags & NoTechnoUpdate))
    //    and DataBase::getDB()
    //    and DataBase::getDB()->getTechnology())
    //   DataBase::getDB()->getTechnology()->_onDbuChange ( scale );

    setSymbolicSnapGridStep ( PNLUnit::lambda( 1.0) );
    setRealSnapGridStep     ( PNLUnit::grid  (10.0) );

    _updateBounds();
  }


  double PNLUnit::getUnitPower ( UnitPower p )
  {
    switch ( p ) {
      case Pico:  return 1.0e-12;
      case Nano:  return 1.0e-9;
      case Micro: return 1.0e-6;
      case Milli: return 1.0e-3;
      case Unity: return 1.0;
      case Kilo:  return 1.0e+3;
    }
    return 1.0;
  }


  void  PNLUnit::setPhysicalsPerGrid ( double physicalsPerGrid, UnitPower p )
  {
    _physicalsPerGrid = physicalsPerGrid * getUnitPower(p);
    _updateBounds();
  }


  double  PNLUnit::getPhysicalsPerGrid ()
  { return _physicalsPerGrid; }


  double  PNLUnit::physicalToGrid ( double physical, UnitPower p )
  { return ( physical * getUnitPower(p) ) / _physicalsPerGrid; }


  void  PNLUnit::setGridsPerLambda ( double gridsPerLambda, unsigned int flags )
  {
    if ((rint(gridsPerLambda) != gridsPerLambda) /*or (remainder(gridsPerLambda,2.0) != 0.0)*/)
      //throw Error ( "PNLUnit::Unit::setGridPerLambdas(): \"gridsPerLambda\" (%f) must be an even integer."
      //            , gridsPerLambda
      //            );
      assert(false);

    //float scale = gridsPerLambda / (float)_gridsPerLambda;

    _gridsPerLambda = gridsPerLambda;

    //if (not (flags & NoTechnoUpdate))
    //    and DataBase::getDB()
    //    and DataBase::getDB()->getTechnology())
    //   DataBase::getDB()->getTechnology()->_onDbuChange ( scale );

    setSymbolicSnapGridStep ( PNLUnit::lambda(1) );

    _updateBounds();
  }


  double  PNLUnit::getGridsPerLambda ()
  { return _gridsPerLambda; }


  PNLUnit::Unit  PNLUnit::getSymbolicSnapGridStep ()
  { return _symbolicSnapGridStep; }


  PNLUnit::Unit  PNLUnit::getOnSymbolicSnapGrid ( PNLUnit::Unit u, SnapMode mode )
  {
    PNLUnit::Unit  inferior = ( u / _symbolicSnapGridStep ) * _symbolicSnapGridStep;
    PNLUnit::Unit  modulo   =   u % _symbolicSnapGridStep;

    if ( !modulo ) return u;
    if (  modulo < 0  ) inferior -= _symbolicSnapGridStep;

    if      ( mode == Inferior ) { return inferior; }
    else if ( mode == Superior ) { return inferior + _symbolicSnapGridStep; }
     
    if ( modulo < 0 )
      return inferior + ( (modulo > - (_symbolicSnapGridStep/2)) ? _symbolicSnapGridStep : 0 );

    return inferior + ( (modulo > (_symbolicSnapGridStep/2)) ? _symbolicSnapGridStep : 0 );
  }


  PNLUnit::Unit  PNLUnit::getRealSnapGridStep ()
  { return _realSnapGridStep; }


  PNLUnit::Unit  PNLUnit::getOnRealSnapGrid ( PNLUnit::Unit u, SnapMode mode )
  {
    PNLUnit::Unit  inferior = ( u / _realSnapGridStep ) * _realSnapGridStep;
    PNLUnit::Unit  modulo   =   u % _realSnapGridStep;

    if ( !modulo ) return u;
    if (  modulo < 0  ) inferior -= _realSnapGridStep;

    if      ( mode == Inferior ) { return inferior; }
    else if ( mode == Superior ) { return inferior + _realSnapGridStep; }
     
    if ( modulo < 0 )
      return inferior + ( (modulo > - (_realSnapGridStep/2)) ? _realSnapGridStep : 0 );
     
    return inferior + ( (modulo > (_realSnapGridStep/2)) ? _realSnapGridStep : 0 );
  }


  PNLUnit::Unit  PNLUnit::getOnCustomGrid ( PNLUnit::Unit u, PNLUnit::Unit step, SnapMode mode )
  {
    PNLUnit::Unit  inferior = ( u / step ) * step;
    PNLUnit::Unit  modulo   = ( u % step );

    if ( !modulo ) return u;
    if (  modulo < 0  ) inferior -= step;

    if      ( mode == Inferior ) { return inferior; }
    else if ( mode == Superior ) { return inferior + step; }
     
    if ( modulo < 0 )
      return inferior + ( (modulo > - (step/2)) ? step : 0 );
     
    return inferior + ( (modulo > (step/2)) ? step : 0 );
  }


  void  PNLUnit::setStringMode ( unsigned int mode, UnitPower p )
  {
    _stringMode = mode;
    if ( _stringMode == Physical ) _stringModeUnitPower = p;
  }


  void  PNLUnit::getStringMode ( unsigned int& mode, UnitPower& p )
  {
    mode = _stringMode;
    p    = _stringModeUnitPower;
  }


//   string  PNLUnit::getValueString ( PNLUnit::Unit u, int mode )
//   {
//     ostringstream os;
//     char unitPower  = ' ';
//     char unitSymbol = 'u';

//     os << fixed;  
//     if (_stringMode == Grid) {
//       unitSymbol = 'g';
//       os << setprecision(1) << toGrid(u);
//     } else if (_stringMode == Symbolic) {
//       unitSymbol = 'L';
//       os << setprecision(2) << toLambda(u);
//     } else if (_stringMode == Physical) {
//       unitSymbol = 'm';
//       switch ( _stringModeUnitPower ) {
//         case Pico:  unitPower = 'p'; break;
//         case Nano:  unitPower = 'n'; break;
//         case Micro: unitPower = 'u'; break;
//         case Milli: unitPower = 'm'; break;
//         case Unity: unitPower = 'U'; break;
//         case Kilo:  unitPower = 'k'; break;
//         default:    unitPower = '?'; break;
//       }
//       switch ( u ) {
//         case Min: os << "MIN:"; break;
//         case Max: os << "MAX:"; break;
//         default:
//           os << setprecision(4) << toPhysical(u,_stringModeUnitPower);
//       }
//     } else {
//       if (_stringMode != Db)
//         cerr << "[ERROR] Unknown Unit representation mode: " << _stringMode << endl;
//       os << u;
//     }

//     string s = os.str();
//     if (_stringMode == Symbolic) {
//       size_t dot = s.rfind( '.' );
//       if (dot != string::npos) s.erase( dot + 2 );
//     } else if (mode & SmartTruncate) {
//       size_t dot = s.rfind( '.' );
//       if (dot != string::npos) {
//         size_t end     = dot;
//         size_t nonzero = end;
//         for ( ; end < s.size() ; ++end ) {
//           if (s[end] != '0') nonzero = end;
//         }
//         if (nonzero == dot) s.erase( dot );
//         else {
//           if (nonzero < s.size()) s.erase( nonzero+1 );
//         }
//       }
//     }
//     if (unitPower != ' ') s += unitPower;
//     s += unitSymbol;

//     return s;
//   }


//   string  PNLUnit::getValueString ( double u, int mode )
//   {
//     ostringstream os;
//     char unitPower  = ' ';
//     char unitSymbol = 'u';

//     os << fixed;  
//     if (_stringMode == Grid) {
//       unitSymbol = 'g';
//       os << setprecision(1) << toGrid(u);
//     } else if (_stringMode == Symbolic) {
//       unitSymbol = 'l';
//       os << setprecision(1) << toLambda(u);
//     } else if (_stringMode == Physical) {
//       unitSymbol = 'm';
//       switch ( _stringModeUnitPower ) {
//         case Pico:  unitPower = 'p'; break;
//         case Nano:  unitPower = 'n'; break;
//         case Micro: unitPower = 'u'; break;
//         case Milli: unitPower = 'm'; break;
//         case Unity: unitPower = 'U'; break;
//         case Kilo:  unitPower = 'k'; break;
//         default:    unitPower = '?'; break;
//       }
//       os << setprecision(3) << toPhysical(u,_stringModeUnitPower);
//     } else {
//       if (_stringMode != Db)
//         cerr << "[ERROR] Unknown Unit representation mode: " << _stringMode << endl;

//       os << u;
//     }

//     string s = os.str();
//     if (mode & SmartTruncate) {
//       size_t dot = s.rfind( '.' );
//       if (dot != string::npos) {
//         size_t end = dot+1;
//         for ( ; end < s.size() ; ++end ) if (s[end] != '0') break;
//         if (end == s.size()) s.erase( dot );
//       }
//     }

//     if (unitPower != ' ') s += unitPower;
//     s += unitSymbol;

//     return s;
//   }


//   Record* PNLUnit::getValueRecord ( const PNLUnit::Unit* u )
//   {
//     Record* record = new Record(getValueString(*u));
//     record->add(getSlot("PNLUnit::Unit", u));
//     return record;
//   }


//   Slot* PNLUnit::getValueSlot ( const string& name, const PNLUnit::Unit* u )
//   {
//     return new DbUSlot ( name, u );
//   }


    } // End of 
}