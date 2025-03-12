// SPDX-FileCopyrightText: 2025 The Naja authors
// <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#pragma  once
#include <cstdint>
#include <cmath>
#include <string>
//#include "hurricane/Commons.h"


namespace naja { 
    namespace PNL {

  //class DataBase;


  class PNLUnit {
      //friend class DataBase;
    public:
      enum FunctionFlags { NoFlags        = 0
                         , NoTechnoUpdate = (1<<0)
                         };
      enum UnitPower     { Pico           = 1
                         , Nano           
                         , Micro          
                         , Milli          
                         , Unity          
                         , Kilo           
                         };               
      enum  StringMode   { Db             = (1<<0)
                         , Grid           = (1<<1)
                         , Symbolic       = (1<<2)
                         , Physical       = (1<<3)
                         , SmartTruncate  = (1<<4)
                         };               
      enum  SnapMode     { Inferior       = 1
                         , Superior       = 2
                         , Nearest        = 4
                         };
    public:
      typedef  std::int64_t  Unit;

    public:
      static        void                checkGridBound          ( double value );
      static        void                checkLambdaBound        ( double value );
      static        void                checkPhysicalBound      ( double value, UnitPower p );
    // User to DB Converters.
      static inline Unit                fromDb                  ( Unit value );
      static inline Unit                fromGrid                ( double value );
      static inline Unit                fromLambda              ( double value );
      static inline Unit                fromPhysical            ( double value, UnitPower p );
      static inline Unit                fromMicrons             ( double value );
      static inline Unit                fromNanos               ( double value );
    // Old naming scheme (was not very clear).
      static inline Unit                db                      ( Unit value );
      static inline Unit                grid                    ( double value );
      static inline Unit                lambda                  ( double value );
      static inline Unit                physicalToDbu           ( double value, UnitPower p );
    // Precision & Resolution Managment.                        
      static        unsigned int        getPrecision            ();
      static        unsigned int        getMaximalPrecision     ();
      static        double              getResolution           ();
      static        void                setPrecision            ( unsigned int precision, unsigned int flags=NoFlags );
    // Foundry Grid Managment.                                  
      static        double              getUnitPower            ( UnitPower p );
      static        void                setPhysicalsPerGrid     ( double gridsPerLambda, UnitPower p );
      static        double              getPhysicalsPerGrid     ();
      static        double              physicalToGrid          ( double physical, UnitPower p );
    // Huge Polygon Step Managment.                                  
      static inline PNLUnit::Unit           getPolygonStep          ();
      static inline void                setPolygonStep          ( PNLUnit::Unit );
    // Lamba Managment.                                         
      static        void                setGridsPerLambda       ( double gridsPerLambda, unsigned int flags=NoFlags );
      static        double              getGridsPerLambda       ();
    // Snap Grid Managment.
      static        PNLUnit::Unit           getRealSnapGridStep     ();
      static        PNLUnit::Unit           getOnRealSnapGrid       ( PNLUnit::Unit u, SnapMode mode=Nearest );
      static inline void                setRealSnapGridStep     ( PNLUnit::Unit step );
      static        PNLUnit::Unit           getSymbolicSnapGridStep ();
      static        PNLUnit::Unit           getOnSymbolicSnapGrid   ( PNLUnit::Unit u, SnapMode mode=Nearest );
      static inline void                setSymbolicSnapGridStep ( PNLUnit::Unit step );
      static        PNLUnit::Unit           getOnCustomGrid         ( PNLUnit::Unit u, PNLUnit::Unit step, SnapMode mode=Nearest );
      static inline PNLUnit::Unit           getOnPhysicalGrid       ( PNLUnit::Unit u, SnapMode mode=Superior );
      static inline PNLUnit::Unit           toCeil                  ( PNLUnit::Unit u, PNLUnit::Unit step );
      static inline PNLUnit::Unit           toFloor                 ( PNLUnit::Unit u, PNLUnit::Unit step );
    // Conversions.
      static inline Unit                toDb                    ( Unit u );
      static inline double              toGrid                  ( Unit u );
      static inline double              toGrid                  ( double u );
      static inline double              toLambda                ( Unit u );
      static inline double              toLambda                ( double u );
      static inline double              toPhysical              ( Unit u, UnitPower p );
      static inline double              toPhysical              ( double u, UnitPower p );
      static inline double              toMicrons               ( Unit u );
      static inline double              toNanos                 ( Unit u );
    // Old naming scheme (not very clear).
      static inline Unit                getDb                   ( Unit u );
      static inline double              getGrid                 ( Unit u );
      static inline double              getGrid                 ( double u );
      static inline double              getLambda               ( Unit u );
      static inline double              getLambda               ( double u );
      static inline double              getPhysical             ( Unit u, UnitPower p );
      static inline double              getPhysical             ( double u, UnitPower p );
      //static        std::string              getValueString          ( Unit u, int mode=SmartTruncate );
      //static        std::string              getValueString          ( double u, int mode=SmartTruncate );
      //static        Record*             getValueRecord          ( const Unit* u );
      //static        Slot*               getValueSlot            ( const std::string& name, const Unit* u );
      static        void                setStringMode           ( unsigned int mode, UnitPower p=Nano );
      static        void                getStringMode           ( unsigned int& mode, UnitPower& p );
    private:
      static        void                _updateBounds           ();

    public:
    // Static Attributes: constants.
      static const Unit          Min;
      static const Unit          Max;
    private:
    // Internal: Static Attributes.
      static const unsigned int  _maximalPrecision;
      static unsigned int        _precision;
      static double              _resolution;
      static double              _gridsPerLambda;
      static double              _physicalsPerGrid;
      static unsigned int        _stringMode;
      static PNLUnit::UnitPower      _stringModeUnitPower;
      static PNLUnit::Unit           _realSnapGridStep;
      static PNLUnit::Unit           _symbolicSnapGridStep;
      static PNLUnit::Unit           _polygonStep;
      static double              _gridMax;
      static double              _lambdaMax;
      static double              _physicalMax;
  };


// Inline Functions.
// New converter naming scheme.
  inline PNLUnit::Unit  PNLUnit::fromDb                  ( PNLUnit::Unit value )            { return value; }
  inline PNLUnit::Unit  PNLUnit::fromGrid                ( double value )               { checkGridBound    (value);   return (Unit)rint( value/_resolution ); }
  inline PNLUnit::Unit  PNLUnit::fromLambda              ( double value )               { checkLambdaBound  (value);   return fromGrid(value*_gridsPerLambda); }
  inline PNLUnit::Unit  PNLUnit::fromPhysical            ( double value, UnitPower p )  { checkPhysicalBound(value,p); return fromGrid((value*getUnitPower(p))/_physicalsPerGrid); }
  inline PNLUnit::Unit  PNLUnit::fromMicrons             ( double value )               { return fromPhysical(value,UnitPower::Micro); }
  inline PNLUnit::Unit  PNLUnit::fromNanos               ( double value )               { return fromPhysical(value,UnitPower::Nano); }
  inline PNLUnit::Unit  PNLUnit::toDb                    ( PNLUnit::Unit u )                { return u; }
  inline double     PNLUnit::toGrid                  ( PNLUnit::Unit u )                { return _resolution*(double)u; }
  inline double     PNLUnit::toGrid                  ( double u )                   { return _resolution*u; }
  inline double     PNLUnit::toLambda                ( PNLUnit::Unit u )                { return toGrid(u)/_gridsPerLambda; }
  inline double     PNLUnit::toLambda                ( double u )                   { return toGrid(u)/_gridsPerLambda; }
  inline double     PNLUnit::toPhysical              ( PNLUnit::Unit u, UnitPower p )   { return (_physicalsPerGrid*_resolution*(double)u)/getUnitPower(p); }
  inline double     PNLUnit::toPhysical              ( double u, UnitPower p )      { return (_physicalsPerGrid*_resolution*u)/getUnitPower(p); }
  inline double     PNLUnit::toMicrons               ( Unit u )                     { return toPhysical(u,UnitPower::Micro); }
  inline double     PNLUnit::toNanos                 ( Unit u )                     { return toPhysical(u,UnitPower::Nano); }
  inline PNLUnit::Unit  PNLUnit::getPolygonStep          ()                             { return _polygonStep; }

// Old converter naming scheme.
  inline PNLUnit::Unit  PNLUnit::db                      ( PNLUnit::Unit value )            { return fromDb(value); }
  inline PNLUnit::Unit  PNLUnit::grid                    ( double value )               { return fromGrid(value); }
  inline PNLUnit::Unit  PNLUnit::lambda                  ( double value )               { return fromLambda(value); }
  inline PNLUnit::Unit  PNLUnit::physicalToDbu           ( double value, UnitPower p )  { return fromPhysical(value,p); }
  inline PNLUnit::Unit  PNLUnit::getDb                   ( PNLUnit::Unit u )                { return toDb(u); }
  inline double     PNLUnit::getGrid                 ( PNLUnit::Unit u )                { return toGrid(u); }
  inline double     PNLUnit::getGrid                 ( double u )                   { return toGrid(u); }
  inline double     PNLUnit::getLambda               ( PNLUnit::Unit u )                { return toLambda(u); }
  inline double     PNLUnit::getLambda               ( double u )                   { return toLambda(u); }
  inline double     PNLUnit::getPhysical             ( PNLUnit::Unit u, UnitPower p )   { return toPhysical(u,p); }
  inline double     PNLUnit::getPhysical             ( double u, UnitPower p )      { return toPhysical(u,p); }

  inline void       PNLUnit::setRealSnapGridStep     ( PNLUnit::Unit step )             { _realSnapGridStep = step; }
  inline void       PNLUnit::setSymbolicSnapGridStep ( PNLUnit::Unit step )             { _symbolicSnapGridStep = step; }
  inline void       PNLUnit::setPolygonStep          ( PNLUnit::Unit step )             { _polygonStep = step; }
  inline PNLUnit::Unit  PNLUnit::getOnPhysicalGrid       ( PNLUnit::Unit u, SnapMode mode ) { return getOnCustomGrid(u, grid(1), mode); }

  inline PNLUnit::Unit  PNLUnit::toCeil ( PNLUnit::Unit u, PNLUnit::Unit step )
  { PNLUnit::Unit modulo = u % step; return (modulo) ? (u + step - modulo) : u; }
  
  inline PNLUnit::Unit  PNLUnit::toFloor ( PNLUnit::Unit u, PNLUnit::Unit step )
  { PNLUnit::Unit modulo = u % step; return (modulo) ? (u - modulo) : u; }

    }
}


// // inline void  jsonWriteDbU ( JsonWriter* w, const std::string& key, long* value )
// // { w->key( key ); w->write( value ); }


// template<>
// inline std::string  getString ( const std::pair<naja::PNL::PNLUnit::Unit,naja::PNL::PNLUnit::Unit>& p )
// {
//   return "const std::pair<PNLUnit::Unit,PNLUnit::Unit>";
// }


// /*template<>
// inline Hurricane::Record* getRecord ( const std::pair<naja::PNL::PNLUnit::Unit,naja::PNL::PNLUnit::Unit>& p )
// {
//   Hurricane::Record* record = NULL;
//   record = new Hurricane::Record ( "const std::pair<PNLUnit::Unit,PNLUnit::Unit>" );
//   record->add( naja::PNL::PNLUnit::getValueSlot("first" , &p.first ) );
//   record->add( naja::PNL::PNLUnit::getValueSlot("second", &p.second) );
//   return record;
// }*/


// template<>
// inline std::string  getString ( const std::array<naja::PNL::PNLUnit::Unit*,3>& a )
// {
//   return "const array<PNLUnit::Unit*,3>";
// }


// // template<>
// // inline Hurricane::Record* getRecord ( const std::array<naja::PNL::PNLUnit::Unit*,3>& a )
// // {
// //   Hurricane::Record* record = NULL;
// //   record = new Hurricane::Record ( "const array<PNLUnit::Unit*,3>" );

// //   for ( size_t i=0 ; i<a.size() ; ++i ) {
// //     std::string label = "[" + getString(i) + "] ";
// //     record->add( naja::PNL::PNLUnit::getValueSlot(label, a[i]) );
// //   }
// //   return record;
// // }


// template<>
// inline std::string  getString ( const std::vector<naja::PNL::PNLUnit::Unit>* v )
// {
//   std::string name = "const std::vector<PNLUnit::Unit>:";
//   return name + getString<size_t>(v->size());
// }


// template<>
// inline Hurricane::Record* getRecord ( const std::vector<naja::PNL::PNLUnit::Unit>* v )
// {
//   Hurricane::Record* record = NULL;
//   record = new Hurricane::Record ( "const vector<PNLUnit::Unit>" );

//   for ( size_t i=0 ; i<v->size() ; ++i ) {
//     std::string label = "[" + getString(i) + "] ";
//     record->add( naja::PNL::PNLUnit::getValueSlot(label, &(*v)[i]) );
//   }
//   return record;
// }
