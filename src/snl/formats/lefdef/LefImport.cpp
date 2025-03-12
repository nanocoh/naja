// SPDX-FileCopyrightText: 2025 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <boost/algorithm/string.hpp>
#include  "lefrReader.hpp"
// #include "hurricane/configuration/Configuration.h"
// #include "hurricane/Error.h"
// #include "hurricane/Warning.h"
// #include "hurricane/DataBase.h"
// #include "hurricane/BasicLayer.h"
// #include "hurricane/Technology.h"
#include "PNLNet.h"
#include "PNLTerm.h"
// #include "hurricane/Contact.h"
// #include "hurricane/Horizontal.h"
// #include "hurricane/Vertical.h"
// #include "hurricane/Rectilinear.h"
#include "PNLDesign.h"
#include "PNLPoint.h"
#include "SNLLibrary.h"
// #include "hurricane/UpdateSession.h"
// #include "crlcore/Utilities.h"
// #include "crlcore/ToolBox.h"
// #include "crlcore/RoutingGauge.h"
// #include "crlcore/PNLDesignGauge.h"
// #include "crlcore/AllianceFramework.h"
#include "LefImport.h"
//#include "crlcore/Gds.h"
#include "SNLDB.h"
#include "SNLUniverse.h"
#include "SNLName.h"
#include "PNLUnit.h"
#include <fstream>
namespace {

  class mstream : public std::ostream {
    public:
      enum StreamMasks { PassThrough   = 0x00000001
                       , Verbose0      = 0x00000002
                       , Verbose1      = 0x00000004
                       , Verbose2      = 0x00000008
                       , Info          = 0x00000010
                       , Paranoid      = 0x00000020
                       , Bug           = 0x00000040
                       };
    public:
      static        void          enable          ( unsigned int mask );
      static        void          disable         ( unsigned int mask );
      inline                      mstream         ( unsigned int mask, std::ostream &s );
      inline        bool          enabled         () const;
      inline        unsigned int  getStreamMask   () const;
      static inline unsigned int  getActiveMask   ();
      inline        void          setStreamMask   ( unsigned int mask );
      inline        void          unsetStreamMask ( unsigned int mask );
    // Overload for formatted outputs.
      template<typename T> inline mstream& operator<< ( T& t );
      template<typename T> inline mstream& operator<< ( T* t );
      template<typename T> inline mstream& operator<< ( const T& t );
      template<typename T> inline mstream& operator<< ( const T* t );
                           inline mstream& put        ( char c );
                           inline mstream& flush      ();
    // Overload for manipulators.
                           inline mstream &operator<< ( std::ostream &(*pf)(std::ostream &) );

    // Internal: Attributes.
    private:
      static unsigned int  _activeMask;
             unsigned int  _streamMask;
  };

  inline               mstream::mstream        ( unsigned int mask, std::ostream& s ): std::ostream(s.rdbuf()) , _streamMask(mask) {}  
  inline bool          mstream::enabled        () const { return (_streamMask & _activeMask); }
  inline unsigned int  mstream::getStreamMask  () const { return  _streamMask; }
  inline unsigned int  mstream::getActiveMask  ()       { return  _activeMask; }
  inline void          mstream::setStreamMask  ( unsigned int mask ) { _streamMask |= mask; }
  inline void          mstream::unsetStreamMask( unsigned int mask ) { _streamMask &= ~mask; }
  inline mstream&      mstream::put            ( char c ) { if (enabled()) static_cast<std::ostream*>(this)->put(c); return *this; }  
  inline mstream&      mstream::flush          () { if (enabled()) static_cast<std::ostream*>(this)->flush(); return *this; }  
  inline mstream&      mstream::operator<<     ( std::ostream& (*pf)(std::ostream&) ) { if (enabled()) (*pf)(*this); return *this; }

// For POD Types.
  template<typename T>
  inline mstream& mstream::operator<< ( T& t )
  { if (enabled()) { *(static_cast<std::ostream*>(this)) << t; } return *this; };

  template<typename T>
  inline mstream& mstream::operator<< ( T* t )
  { if (enabled()) { *(static_cast<std::ostream*>(this)) << t; } return *this; };

  template<typename T>
  inline mstream& mstream::operator<< ( const T& t )
  { if (enabled()) { *(static_cast<std::ostream*>(this)) << t; } return *this; };

  template<typename T>
  inline mstream& mstream::operator<< ( const T* t )
  { if (enabled()) { *(static_cast<std::ostream*>(this)) << t; } return *this; };

// For STL Types.
  inline mstream& operator<< ( mstream& o, const std::string& s )
  { if (o.enabled()) { static_cast<std::ostream&>(o) << s; } return o; };

// Specific non-member operator overload. Must be one for each type.
#define  MSTREAM_V_SUPPORT(Type)                           \
  inline mstream& operator<< ( mstream& o, Type t )        \
  { if (o.enabled()) { static_cast<std::ostream&>(o) << t; } return o; }; \
                                                           \
  inline mstream& operator<< ( mstream& o, const Type t )  \
  { if (o.enabled()) { static_cast<std::ostream&>(o) << t; } return o; };

#define  MSTREAM_R_SUPPORT(Type)                           \
  inline mstream& operator<< ( mstream& o, const Type& t ) \
  { if (o.enabled()) { static_cast<std::ostream&>(o) << t; } return o; }; \
                                                           \
  inline mstream& operator<< ( mstream& o, Type& t )       \
  { if (o.enabled()) { static_cast<std::ostream&>(o) << t; } return o; };

#define  MSTREAM_P_SUPPORT(Type)                           \
  inline mstream& operator<< ( mstream& o, const Type* t ) \
  { if (o.enabled()) { static_cast<std::ostream&>(o) << t; } return o; }; \
                                                           \
  inline mstream& operator<< ( mstream& o, Type* t )       \
  { if (o.enabled()) { static_cast<std::ostream&>(o) << t; } return o; };


  unsigned int  mstream::_activeMask = 0;
  extern mstream  cmess1;
  mstream  cmess1    ( mstream::Verbose0, std::cout );


  using namespace std;
  using namespace naja::SNL;
  using namespace naja::PNL;

#if THIS_IS_DISABLED
  void  addSupplyPNLNets ( PNLDesign* cell )
  {
    PNLNet* vss = PNLNet::create( cell, "vss" );
    vss->setExternal( true );
    vss->setGlobal  ( true );
    vss->setType    ( PNLNet::Type::GROUND );

    PNLNet* vdd = PNLNet::create( cell, "vdd" );
    vdd->setExternal( true );
    vdd->setGlobal  ( true );
    vdd->setType    ( PNLNet::Type::POWER );
  }
#endif


  class LefParser {
    public:
      static       void               setMergeLibrary          ( SNLLibrary* );
      static       void               setGdsForeignDirectory   ( string );
      //static       PNLUnit::Unit          fromLefUnits             ( int );
      // static       Layer*             getLayer                 ( string );
      // static       void               addLayer                 ( string, Layer* );
      static       void               reset                    ();
      static       SNLLibrary*           parse                    ( string file );
                                      LefParser                ( string file, string libraryName );
                                     ~LefParser                ();
      inline       bool               isVH                     () const;
                   bool               isUnmatchedLayer         ( string );
                   SNLLibrary*           createSNLLibrary            ();
                   PNLDesign*              earlyGetPNLDesign             ( bool& created, string name="" );
                   PNLNet*               earlygetNet              ( string name );
                   PNLTerm*               earlygetTerm              ( string name );
      inline       string             getLibraryName           () const;
      inline       SNLLibrary*           getLibrary               ( bool create=false );
      inline       string             getForeignPath           () const;
      inline       void               setForeignPath           ( string );
      inline const PNLPoint&             getForeignPosition       () const;
      inline       void               setForeignPosition       ( const PNLPoint&  );
      inline       PNLNet*               getGdsPower              () const;
      inline       void               setGdsPower              ( PNLNet* );
      inline       PNLNet*               getGdsGround             () const;
      inline       void               setGdsGround             ( PNLNet* );
      inline       PNLDesign*              getPNLDesign                  () const;
      inline       void               setPNLDesign                  ( PNLDesign* );
      // inline       PNLDesignGauge*         getPNLDesignGauge             () const;
      // inline       void               setPNLDesignGauge             ( PNLDesignGauge* );
      inline       PNLNet*               getNet                   () const;
      inline       void               setPNLNet                   ( PNLNet* );
      static       void               setCoreSite              ( PNLUnit::Unit x, PNLUnit::Unit y );
      static       PNLUnit::Unit          getCoreSiteX             ();
      static       PNLUnit::Unit          getCoreSiteY             ();
      inline       PNLUnit::Unit          getMinTerminalWidth      () const;
      inline       double             getUnitsMicrons          () const;
      inline       PNLUnit::Unit          fromUnitsMicrons         ( double ) const;
      inline       void               setUnitsMicrons          ( double );
      inline       bool               hasErrors                () const;
      inline const vector<string>&    getErrors                () const;
      inline       void               pushError                ( const string& );
                   int                flushErrors              ();
      inline       void               clearErrors              ();
      inline       int                getNthMetal              () const;
      inline       void               incNthMetal              ();
      inline       int                getNthCut                () const;
      inline       void               incNthCut                ();
      inline       int                getNthRouting            () const;
      inline       void               incNthRouting            ();
      //inline       RoutingGauge*      getRoutingGauge          () const;
       inline       void               addPinComponent          ( string name, PNLTerm* );
      inline       void               clearPinComponents       ();
    private:                                               
      static       int                _unitsCbk                ( lefrCallbackType_e,       lefiUnits*       , lefiUserData );
      static       int                _layerCbk                ( lefrCallbackType_e,       lefiLayer*       , lefiUserData );
      static       int                _siteCbk                 ( lefrCallbackType_e,       lefiSite*        , lefiUserData );
      static       int                _obstructionCbk          ( lefrCallbackType_e,       lefiObstruction* , lefiUserData );
      static       int                _macroCbk                ( lefrCallbackType_e,       lefiMacro*       , lefiUserData );
      static       int                _macroSiteCbk            ( lefrCallbackType_e, const lefiMacroSite*   , lefiUserData );
      static       int                _macroForeignCbk         ( lefrCallbackType_e, const lefiMacroForeign*, lefiUserData );
      static       int                _pinCbk                  ( lefrCallbackType_e,       lefiPin*         , lefiUserData );
                   void               _pinStdPostProcess       ();
                   void               _pinPadPostProcess       ();
      static       int               _viaCbk                  ( lefrCallbackType_e type,       lefiVia*         via, lefiUserData) { printf("via name %s cb\n", via->name()); return 0; }
      static void _logFunction(const char* message);
    private:                                               
      static       string              _gdsForeignDirectory;
      static       SNLLibrary*            _mergeSNLLibrary;
                   string              _file;
                   string              _libraryName;
                   SNLLibrary*            _library;
                   string              _foreignPath;
                   PNLPoint               _foreignPosition;
                   PNLNet*                _gdsPower;
                   PNLNet*                _gdsGround;
                   PNLDesign*               _cell;
                   PNLNet*                _net;
                   string              _busBits;
                   double              _unitsMicrons;
                  PNLUnit::Unit           _oneGrid;
                  map< string, vector<PNLTerm*> >  _pinComponents;
     // static       map<string,Layer*>  _layerLut;
                   vector<string>      _unmatchedLayers;
                   vector<string>      _errors;
                   int                 _nthMetal;
                   int                 _nthCut;
                   int                 _nthRouting;
                  //  RoutingGauge*       _routingGauge;
                  //  PNLDesignGauge*          _cellGauge;
                    PNLUnit::Unit           _minTerminalWidth;
       static       PNLUnit::Unit           _coreSiteX;
       static       PNLUnit::Unit           _coreSiteY;
  };


  // inline       bool              LefParser::isVH                     () const { return _routingGauge->isVH(); }
  inline       PNLUnit::Unit         LefParser::getMinTerminalWidth      () const { return _minTerminalWidth; }
  inline       string            LefParser::getLibraryName           () const { return _libraryName; }
  inline       SNLLibrary*          LefParser::getLibrary               ( bool create ) { if (not _library and create) createSNLLibrary(); return _library; }
  inline       PNLDesign*             LefParser::getPNLDesign                  () const { return _cell; }
  inline       void              LefParser::setPNLDesign                  ( PNLDesign* cell ) { _cell=cell; }
  inline       string            LefParser::getForeignPath           () const { return _foreignPath; }
  inline       void              LefParser::setForeignPath           ( string path ) { _foreignPath=path; }
  inline const PNLPoint&            LefParser::getForeignPosition       () const { return _foreignPosition; }
  inline       void              LefParser::setForeignPosition       ( const PNLPoint& position ) { _foreignPosition=position; }
  inline       PNLNet*              LefParser::getGdsPower              () const { return _gdsPower; }
  inline       void              LefParser::setGdsPower              ( PNLNet* net ) { _gdsPower=net; }
  inline       PNLNet*              LefParser::getGdsGround             () const { return _gdsGround; }
  inline       void              LefParser::setGdsGround             ( PNLNet* net ) { _gdsGround=net; }
  // inline       void              LefParser::setPNLDesignGauge             ( PNLDesignGauge* gauge ) { _cellGauge=gauge; }
  inline       PNLNet*              LefParser::getNet                   () const { return _net; }
  inline       void              LefParser::setPNLNet                   ( PNLNet* net ) { _net=net; }
  inline       double            LefParser::getUnitsMicrons          () const { return _unitsMicrons; }
  inline       void              LefParser::setUnitsMicrons          ( double precision ) { _unitsMicrons=precision; }
  inline       int               LefParser::getNthMetal              () const { return _nthMetal; }
  inline       void              LefParser::incNthMetal              () { ++_nthMetal; }
  inline       int               LefParser::getNthCut                () const { return _nthCut; }
  inline       void              LefParser::incNthCut                () { ++_nthCut; }
  inline       int               LefParser::getNthRouting            () const { return _nthRouting; }
  inline       void              LefParser::incNthRouting            () { ++_nthRouting; }
  //inline       RoutingGauge*     LefParser::getRoutingGauge          () const { return _routingGauge; }
  //inline       PNLDesignGauge*        LefParser::getPNLDesignGauge             () const { return _cellGauge; }
  inline       void              LefParser::setCoreSite              ( PNLUnit::Unit x, PNLUnit::Unit y ) { _coreSiteX=x; _coreSiteY=y; }
  inline       PNLUnit::Unit         LefParser::getCoreSiteX             () { return _coreSiteX; }
  inline       PNLUnit::Unit         LefParser::getCoreSiteY             () { return _coreSiteY; }
  inline       bool              LefParser::hasErrors                () const { return not _errors.empty(); }
  inline const vector<string>&   LefParser::getErrors                () const { return _errors; }
  inline       void              LefParser::pushError                ( const string& error ) { _errors.push_back(error); }
  inline       void              LefParser::clearErrors              () { return _errors.clear(); }
  inline       void              LefParser::addPinComponent          ( string name, PNLTerm* comp ) { _pinComponents[name].push_back(comp); }
  inline       void              LefParser::clearPinComponents       () { _pinComponents.clear(); }

  inline PNLUnit::Unit  LefParser::fromUnitsMicrons ( double d ) const
  {
    PNLUnit::Unit u = PNLUnit::fromPhysical(d,PNLUnit::Micro);
    if (u % _oneGrid) {
      // cerr << Error( "LefParser::fromUnitsMicrons(): Offgrid value %s (DbU=%d), grid %s (DbU=%d)."
      //              , PNLUnit::getValueString(u).c_str(), u
      //              , PNLUnit::getValueString(_oneGrid).c_str(), _oneGrid )
      //      << endl;
      assert(false);
    }
    return u;
  }


  string              LefParser::_gdsForeignDirectory = "";
  SNLLibrary*            LefParser::_mergeSNLLibrary = nullptr;
  //map<string,Layer*>  LefParser::_layerLut;
  PNLUnit::Unit           LefParser::_coreSiteX = 0;
  PNLUnit::Unit           LefParser::_coreSiteY = 0;


  void  LefParser::setMergeLibrary ( SNLLibrary* library )
  { _mergeSNLLibrary = library; }
  

  void  LefParser::setGdsForeignDirectory ( string path )
  { _gdsForeignDirectory = path; }


  void  LefParser::reset ()
  {
    // _layerLut.clear();
    _coreSiteX = 0;
    _coreSiteY = 0;
  }
  

  // Layer* LefParser::getLayer ( string layerName )
  // {
  //   //auto item = _layerLut.find( layerName );
  //   //if (item != _layerLut.end()) return (*item).second;
  //   return NULL;
  // }


  // void  LefParser::addLayer ( string layerName, Layer* layer )
  // {
  //   if (getLayer(layerName)) {
  //     cerr << Warning( "LefParser::addLayer(): Duplicated layer name \"%s\" (ignored).", layerName.c_str() );
  //     return;
  //   }

  //   _layerLut[ layerName ] = layer;
  // }


  bool  LefParser::isUnmatchedLayer ( string layerName )
  {
    for ( string layer : _unmatchedLayers ) {
      if (layer == layerName) return true;
    }
    return false;
  }


  LefParser::LefParser ( string file, string libraryName )
    : _file            (file)
    , _libraryName     (libraryName)
    , _library         (nullptr)
    , _foreignPath     ()
    , _foreignPosition (PNLPoint(0,0))
    , _gdsPower        (nullptr)
    , _gdsGround       (nullptr)
    , _cell            (nullptr)
    , _net             (nullptr)
    , _busBits         ("()")
    , _unitsMicrons    (0.01)
    // , _oneGrid         (PNLUnit::fromGrid(1.0))
    , _unmatchedLayers ()
    , _errors          ()
    , _nthMetal        (0)
    , _nthCut          (0)
    , _nthRouting      (0)
    // , _routingGauge    (nullptr)
    // , _cellGauge       (nullptr)
    //, _minTerminalWidth(PNLUnit::fromPhysical(Cfg::getParamDouble("lefImport.minTerminalWidth",0.0)->asDouble(),PNLUnit::UnitPower::Micro))
  {
    // _routingGauge = AllianceFramework::get()->getRoutingGauge();
    // _cellGauge    = AllianceFramework::get()->getPNLDesignGauge();

    // if (not _routingGauge)
    //   throw Error( "LefParser::LefParser(): No default routing gauge defined in Alliance framework." );

    // if (not _cellGauge)
    //   throw Error( "LefParser::LefParser(): No default cell gauge defined in Alliance framework." );

    // string unmatcheds = Cfg::getParamString("lefImport.unmatchedLayers","")->asString();
    // if (not unmatcheds.empty()) {
    //   size_t ibegin = 0;
    //   size_t iend   = unmatcheds.find( ',', ibegin );
    //   while (iend != string::npos) {
    //     _unmatchedLayers.push_back( unmatcheds.substr(ibegin,iend-ibegin) );
    //     ibegin = iend+1;
    //     iend   = unmatcheds.find( ',', ibegin );
    //   }
    //   _unmatchedLayers.push_back( unmatcheds.substr(ibegin) );
    // }
    printf("LefParser::LefParser(): %s\n", file.c_str()); 
    lefrSetLogFunction(_logFunction);
    lefrInit();
    lefrSetUnitsCbk       ( _unitsCbk        );
    lefrSetLayerCbk       ( _layerCbk        );
    lefrSetSiteCbk        ( _siteCbk         );
    lefrSetObstructionCbk ( _obstructionCbk  );
    lefrSetMacroCbk       ( _macroCbk        );
    lefrSetMacroSiteCbk   ( _macroSiteCbk    );
    lefrSetMacroForeignCbk( _macroForeignCbk );
    lefrSetPinCbk         ( _pinCbk          );
    lefrSetViaCbk         ( _viaCbk          );
  }


  LefParser::~LefParser ()
  {
    lefrReset();
  }


  SNLLibrary* LefParser::createSNLLibrary ()
  {
    if (_mergeSNLLibrary) {
      _library = _mergeSNLLibrary;
      return _library;
    }
    SNLDB* db = SNLDB::create(SNLUniverse::get());
    SNLLibrary* rootSNLLibrary = SNLLibrary::create( db, SNLName("LIB1") );

    SNLLibrary* lefRootSNLLibrary = SNLLibrary::create( rootSNLLibrary, SNLName("LEF") );

    _library = lefRootSNLLibrary->getLibrary( SNLName(_libraryName) );
    if (_library) {
      // Error
    } else {
      _library = SNLLibrary::create( lefRootSNLLibrary, SNLName(_libraryName) );
    }
    return _library;
  }


  PNLDesign* LefParser::earlyGetPNLDesign ( bool& created, string name )
  {
    if (not _cell) {
      if (name.empty())
        name = "EarlyLEFPNLDesign";
      _cell = getLibrary(true)->getPNLDesign( SNLName(name) );
      if (not _cell) {
        created = true;
        _cell = PNLDesign::create( getLibrary(true), SNLName(name) );
      }
    }
    printf("name %s design name %s\n", name.c_str(), _cell->getName().getString().c_str());
    return _cell;
  }


  PNLNet* LefParser::earlygetNet ( string name )
  {
    bool created = false;
    if (not _cell) earlyGetPNLDesign( created );
    PNLNet* net = _cell->getNet( SNLName(name) );
    if (not net)
      net = _cell->addNet(SNLName(name));
    return net;
  }

  PNLTerm* LefParser::earlygetTerm ( string name )
  {
    bool created = false;
    if (not _cell) earlyGetPNLDesign( created );
    PNLTerm* term = _cell->getTerm( SNLName(name) );
    if (not term)
      term = _cell->addTerm(SNLName(name));
    return term;
  }

  void LefParser::_logFunction(const char* message) {
      std::cout << message << std::endl;
  }
  
  int  LefParser::_unitsCbk ( lefrCallbackType_e c, lefiUnits* units, lefiUserData ud )
  {
    LefParser* parser = (LefParser*)ud;

    if (units->hasDatabase()) {
      parser->_unitsMicrons = 1.0 / units->databaseNumber();
      cerr << "     - Precision: " << parser->_unitsMicrons
           << " (LEF MICRONS scale factor:" << units->databaseNumber() << ")" << endl;
    }
    return 0;
  }

  
  int  LefParser::_layerCbk ( lefrCallbackType_e c, lefiLayer* lefLayer, lefiUserData ud )
   {
    printf("LefParser::_layerCbk(): %s\n", lefLayer->name());
  //   LefParser* parser = (LefParser*)ud;

  //   if (not lefLayer->hasType()) {
  //     cerr << Warning( "LefParser::_layerCbk(): layer \"%s\" has no TYPE (ignored).", lefLayer->name() );
  //     return 0;
  //   }

  //   Technology* techno = DataBase::getDB()->getTechnology();
    
  //   string lefType = lefLayer->type();
  //   boost::to_upper( lefType );

  //   if (lefType == "CUT") {
  //     Layer* layer = techno->getNthCut( parser->getNthCut() );
  //     while (parser->isUnmatchedLayer(getString(layer->getName()))) {
  //       parser->incNthCut();
  //       cerr << "     - Unmapped techno layer \"" << layer->getName() << "\"" << endl;
  //       layer = techno->getNthCut( parser->getNthCut() );
  //     }
  //     if (layer) {
  //       parser->addLayer( lefLayer->name(), layer );
  //       parser->incNthCut();

  //       cerr << "     - \"" << lefLayer->name() << "\" map to \"" << layer->getName() << "\"" << endl;
  //     }
  //   }

  //   if (lefType == "ROUTING") {
  //     Layer* layer = techno->getNthMetal( parser->getNthMetal() );
  //     while (parser->isUnmatchedLayer(getString(layer->getName()))) {
  //       parser->incNthMetal();
  //       cerr << "     - Unmapped techno layer \"" << layer->getName() << "\"" << endl;
  //       layer = techno->getNthMetal( parser->getNthMetal() );
  //     }
      
  //     if (layer) {
  //       BasicLayer* basicLayer = layer->getBasicLayers().getFirst();
  //       parser->addLayer( lefLayer->name(), basicLayer );

  //       cerr << "     - \"" << lefLayer->name() << "\" map to \"" << basicLayer->getName() << "\"" << endl;

  //       RoutingLayerGauge* gauge = parser->getRoutingGauge()->getLayerGauge( parser->getNthRouting() );

  //       if (gauge and (layer == gauge->getLayer())) {
  //         if (lefLayer->hasPitch()) {
  //           double lefPitch = lefLayer->pitch();
  //           double crlPitch = PNLUnit::toPhysical(gauge->getPitch(),PNLUnit::Micro);
  //           if (lefPitch > crlPitch)
  //             cerr << Warning( "LefParser::_layerCbk(): CRL Routing pitch for \"%s\" of %fum is less than %fum."
  //                            , getString( basicLayer->getName() ).c_str() , crlPitch , lefPitch ) << endl;
  //         }

  //         if (lefLayer->hasWidth()) {
  //           double lefWidth = lefLayer->width();
  //           double crlWidth = PNLUnit::toPhysical(gauge->getWireWidth(),PNLUnit::Micro);
  //           if (lefWidth > crlWidth)
  //             cerr << Warning( "LefParser::_layerCbk(): CRL Routing wire width for \"%s\" of %fum is less than %fum."
  //                            , getString( basicLayer->getName() ).c_str() , crlWidth , lefWidth ) << endl;
  //         }

  //         if (lefLayer->hasDirection()) {
  //           string lefDirection = lefLayer->direction();
  //           boost::to_upper( lefDirection );

  //           if ( (lefDirection == "HORIZONTAL") and gauge->isVertical() )
  //             cerr << Warning( "LefParser::_layerCbk(): CRL Routing direction discrepency for \"%s\", LEF is HORIZONTAL."
  //                            , getString( basicLayer->getName() ).c_str() ) << endl;

  //           if ( (lefDirection == "VERTICAL") and gauge->isHorizontal() )
  //             cerr << Warning( "LefParser::_layerCbk(): CRL Routing direction discrepency for \"%s\", LEF is VERTICAL."
  //                            , getString( basicLayer->getName() ).c_str() ) << endl;
  //         }
  //         parser->incNthRouting();
  //       } else {
  //         cerr << Warning( "LefParser::_layerCbk(): No CRL routing gauge defined for \"%s\"."
  //                        , getString( basicLayer->getName() ).c_str()
  //                        ) << endl;
  //       }

  //       parser->incNthMetal();
  //     }
  //   }

    return 0;
  }

  
  int  LefParser::_siteCbk ( lefrCallbackType_e c, lefiSite* site, lefiUserData ud )
  {
    printf("LefParser::_siteCbk\n");  
    // LefParser*         parser = (LefParser*)ud;
    // AllianceFramework* af     = AllianceFramework::get();

    // if (site->hasClass()) {
    //   string siteClass = site->siteClass();
    //   boost::to_upper( siteClass );

    //   PNLUnit::Unit lefSiteWidth  = PNLUnit::fromPhysical( site->sizeX(), PNLUnit::Micro );
    //   PNLUnit::Unit lefSiteHeight = PNLUnit::fromPhysical( site->sizeY(), PNLUnit::Micro );

    //   if (siteClass == "CORE") {
    //     PNLDesignGauge* gauge = parser->getPNLDesignGauge();
    //     if (not gauge)
    //       throw Error( "LefParser::_siteCbk(): Default gauge is not defined. Aborting." );
        
    //     PNLUnit::Unit  crlSliceStep   = gauge->getSliceStep  ();
    //     PNLUnit::Unit  crlSliceHeight = gauge->getSliceHeight();

    //     if (not parser->getCoreSiteX()
    //        or ((parser->getCoreSiteX() != crlSliceStep) and (parser->getCoreSiteY() != crlSliceHeight)) ) {
    //       parser->setCoreSite( lefSiteWidth, lefSiteHeight );

    //       if ( (crlSliceStep == lefSiteWidth) and (crlSliceHeight == lefSiteHeight) )
    //         cerr << "     - Site \"" << site->name() << "\" of class CORE match the Coriolis PNLDesign gauge." << endl;
    //     }
    //   } else if (siteClass == "PAD") {
    //     string     name = string("LEF.") + site->name();
    //     PNLDesignGauge* cg   = af->getPNLDesignGauge( name );

    //     if (cg) {
    //       if ( (cg->getSliceStep() != lefSiteWidth) or (cg->getSliceHeight() != lefSiteHeight)) {
    //         cerr << "     - Site \"" << site->name() << "\" of class PAD has mismatched redefinition OVERWRITING." << endl;
    //         cerr << "       width: "  << PNLUnit::getValueString(cg->getSliceStep  ()) << " vs. " <<  PNLUnit::getValueString(lefSiteWidth)
    //              <<       " height: " << PNLUnit::getValueString(cg->getSliceHeight()) << " vs. " <<  PNLUnit::getValueString(lefSiteHeight)
    //              << endl;
    //       //cg->setPitch      ( lefSiteWidth  );
    //         cg->setSliceStep  ( lefSiteWidth  );
    //         cg->setSliceHeight( lefSiteHeight );
    //       }
    //       cg->setFlags( PNLDesignGauge::Flags::Pad );
    //     } else {
    //       cg = PNLDesignGauge::create( name.c_str(), "unknown", lefSiteWidth, lefSiteHeight, lefSiteWidth );
    //       cg->setFlags( PNLDesignGauge::Flags::Pad );
    //       af->addPNLDesignGauge( cg );
    //     }
    //   }
    // }

    return 0;
  }


  int  LefParser::_macroForeignCbk ( lefrCallbackType_e c, const lefiMacroForeign* foreign, lefiUserData ud )
  {
    printf("LefParser::_macroForeignCbk\n");
    printf("cellName %s\n", foreign->cellName());
    LefParser* parser = (LefParser*)ud;

    bool  created = false;
    PNLDesign* cell    = parser->earlyGetPNLDesign( created, foreign->cellName() );

    if (created) {
      if (_gdsForeignDirectory.empty()) {
        //cerr << Warning( "LefParser::_macroForeignCbk(): GDS directory *not* set, ignoring FOREIGN statement." ) << endl;
        return 0;
      }

      string gdsPath = _gdsForeignDirectory + "/" + foreign->cellName() + ".gds";
      parser->setForeignPath( gdsPath );

      // Gds::setTopPNLDesignName( foreign->cellName() );
      // Gds::load( parser->getLibrary(), parser->getForeignPath()
      //          , Gds::NoBlockages|Gds::Layer_0_IsBoundary);
    }

    parser->setForeignPosition( PNLPoint( parser->fromUnitsMicrons( foreign->px() )
                                     , parser->fromUnitsMicrons( foreign->px() )));

    for ( PNLNet* net : cell->getNets() ) {
      if (net->isVCC ()) parser->setGdsPower ( net );
      if (net->isGND()) parser->setGdsGround( net );
      if (parser->getForeignPosition() != PNLPoint(0,0)) {
        for ( PNLTerm* component : net->getTerms() ) {
          component->translate( parser->getForeignPosition().getX()
                              , parser->getForeignPosition().getY() );
        }
      }
    }

    return 0;
  }
  

  int  LefParser::_obstructionCbk ( lefrCallbackType_e c, lefiObstruction* obstruction, lefiUserData ud )
  {
    printf("LefParser::_obstructionCbk\n");
  //   LefParser* parser = (LefParser*)ud;

  //   const Layer* layer         = NULL;
  //   const Layer* blockageLayer = NULL;
  //         PNLDesign*  cell          = parser->getPNLDesign();
  //         PNLNet*   blockagePNLNet   = cell->getNet( "blockage" );

  //   if (not blockagePNLNet) {
  //     blockagePNLNet = PNLNet::create( cell, "blockage" );
  //     blockagePNLNet->setType( PNLNet::Type::BLOCKAGE );
  //   }

  // //cerr << "       @ _obstructionCbk: " << blockagePNLNet->getName() << endl;
      
  //   lefiGeometries* geoms = obstruction->geometries();
  //   for ( int igeom=0 ; igeom < geoms->numItems() ; ++ igeom ) {
  //     if (geoms->itemType(igeom) == lefiGeomLayerE) {
  //       layer         = parser->getLayer( geoms->getLayer(igeom) );
  //       blockageLayer = layer->getBlockageLayer();
  //     }
  //     if (not blockageLayer) {
  //       cerr << Error( "DefImport::_obstructionCbk(): No blockage layer associated to \"%s\".\n"
  //                      "        (while parsing \"%s\")"
  //                    , getString( layer->getName() ).c_str()
  //                    , getString( cell ).c_str()
  //                    ) << endl;
  //       continue;
  //     }
      
  //     if (geoms->itemType(igeom) == lefiGeomRectE) {
  //       lefiGeomRect* r         = geoms->getRect(igeom);
  //       double        w         = r->xh - r->xl;
  //       double        h         = r->yh - r->yl;
  //       Segment*      segment   = NULL;
  //       if (w >= h) {
  //         segment = Horizontal::create( blockagePNLNet, blockageLayer
  //                                     , parser->fromUnitsMicrons( (r->yl + r->yh)/2 )
  //                                     , parser->fromUnitsMicrons( h  )
  //                                     , parser->fromUnitsMicrons( r->xl )
  //                                     , parser->fromUnitsMicrons( r->xh )
  //                                     );
  //       } else {
  //         segment = Vertical::create( blockagePNLNet, blockageLayer
  //                                   , parser->fromUnitsMicrons( (r->xl + r->xh)/2 )
  //                                   , parser->fromUnitsMicrons( w  )
  //                                   , parser->fromUnitsMicrons( r->yl )
  //                                   , parser->fromUnitsMicrons( r->yh )
  //                                   );
  //       }
  //       cdebug_log(100,0) << "| " << segment << endl;
  //     }

  //     if (geoms->itemType(igeom) == lefiGeomPolygonE) {
  //       lefiGeomPolygon* polygon = geoms->getPolygon(igeom);
  //       vector<PNLPoint>    points;
  //       for ( int ipoint=0 ; ipoint<polygon->numPNLPoints ; ++ipoint ) {
  //         points.push_back( PNLPoint( parser->fromUnitsMicrons(polygon->x[ipoint])
  //                                , parser->fromUnitsMicrons(polygon->y[ipoint]) ));
  //       }
  //       points.push_back( PNLPoint( parser->fromUnitsMicrons(polygon->x[0])
  //                              , parser->fromUnitsMicrons(polygon->y[0]) ));
  //       Rectilinear::create( blockagePNLNet, blockageLayer, points );
  //       continue;
  //     }
  //   }

    return 0;
  }

  
  int  LefParser::_macroCbk ( lefrCallbackType_e c, lefiMacro* macro, lefiUserData ud )
  {
    printf("LefParser::_macroCbk\n");
    //AllianceFramework* af     = AllianceFramework::get();
    LefParser*         parser = (LefParser*)ud;

    //parser->setPNLDesignGauge( nullptr );

    bool       created  = false;
    string     cellName = macro->name();
    PNLUnit::Unit  width    = 0;
    PNLUnit::Unit  height   = 0;
    PNLDesign*      cell     = parser->earlyGetPNLDesign( created );

    if (cell->getName() != SNLName(cellName)) {
      cell->setName( SNLName(cellName) );
    }

    if (macro->hasSize()) {
      width  = parser->fromUnitsMicrons( macro->sizeX() );
      height = parser->fromUnitsMicrons( macro->sizeY() );
      cell->setAbutmentBox( PNLBox( 0, 0, width, height ) );
    }

    bool   isPad     = false;
    string gaugeName = "Unknown SITE";
    // if (macro->hasSiteName()) {
    //   gaugeName = string("LEF.") + macro->siteName();
    //   PNLDesignGauge* cg = af->getPNLDesignGauge( gaugeName );
    //   if (cg) {
    //     isPad = cg->isPad();
    //     if (cg->getSliceHeight() != height) {
    //       cerr << Warning( "LefParser::_macroCbk(): PNLDesign height %s do not match PNLDesignGauge/SITE \"%s\" of %s."
    //                      , PNLUnit::getValueString(height).c_str()
    //                      , getString(cg->getName()).c_str()
    //                      , PNLUnit::getValueString(cg->getSliceHeight()).c_str()
    //                      ) << endl;
    //     }
    //     parser->setPNLDesignGauge( cg );
    //   } else {
    //     cerr << Warning( "LefParser::_macroCbk(): No PNLDesignGauge associated to SITE \"%s\"."
    //                    , macro->siteName() ) << endl;
    //   }
    // }

    if (not isPad) parser->_pinStdPostProcess();
    else           parser->_pinPadPostProcess();
    parser->clearPinComponents();

    // cerr << "     o " << cellName
    //      << " " << PNLUnit::getValueString(width) << " " << PNLUnit::getValueString(height)
    //      << " " << gaugeName; 
    if (isPad) cerr << " (PAD)";
    cerr << endl; 

    // Catalog::State* state = af->getCatalog()->getState( cellName );
    // if (not state) state = af->getCatalog()->getState ( cellName, true );
    // state->setFlags( Catalog::State::Logical
    //                | Catalog::State::Physical
    //                | Catalog::State::InMemory
    //                | Catalog::State::Termina∂lPNLNetlist, true );
    cell->setTerminalNetlist( true ); 
    parser->setPNLDesign     ( nullptr );
    parser->setGdsPower ( nullptr );
    parser->setGdsGround( nullptr );

    return 0;
  }

  
  int  LefParser::_macroSiteCbk ( lefrCallbackType_e c, const lefiMacroSite* site, lefiUserData ud )
   {  
    printf("LefParser::_macroSiteCbk\n"); 
    return 0; }

  
  int  LefParser::_pinCbk ( lefrCallbackType_e c, lefiPin* pin, lefiUserData ud )
  {
    printf("LefParser::_pinCbk %s\n", pin->name());
    LefParser* parser = (LefParser*)ud;

  //cerr << "       @ _pinCbk: " << pin->name() << endl;

    bool created = false;
    parser->earlyGetPNLDesign( created );

    PNLNet*      net     = nullptr;
    PNLTerm*     term    = nullptr;
    PNLNet::Type netType = PNLNet::Type::UNDEFINED;
    if (pin->hasUse()) {
      string lefUse = pin->use();
      boost::to_upper( lefUse );
      
      if (lefUse == "SIGNAL") netType = PNLNet::Type::LOGICAL;
    //if (lefUse == "ANALOG") netType = PNLNet::Type::ANALOG;
      if (lefUse == "CLOCK" ) netType = PNLNet::Type::CLOCK;
      if (lefUse == "POWER" ) netType = PNLNet::Type::POWER;
      if (lefUse == "GROUND") netType = PNLNet::Type::GROUND;
    }

    if ((netType == PNLNet::Type::POWER) and parser->getGdsPower()) {
      net = parser->getGdsPower();
      // cerr << "       - Renaming GDS power net \"" << net->getName() << "\""
      //      << " to LEF name \"" << pin->name() << "\"." << endl;
      net->setName( SNLName(pin->name()) );
      parser->setGdsPower( nullptr );
    } else {
      if ((netType == PNLNet::Type::GROUND) and parser->getGdsGround()) {
        net = parser->getGdsGround();
        // cerr << "       - Renaming GDS ground net \"" << net->getName() << "\""
        //      << " to LEF name \"" << pin->name() << "\"." << endl;
        net->setName(  SNLName(pin->name()) );
        parser->setGdsGround( nullptr );
      } else {
        net = parser->earlygetNet( pin->name() );
        term = parser->earlygetTerm( pin->name() );
      }
    }
    net->setExternal( true );
    net->setType    ( netType );

    if (pin->hasDirection()) {
      string lefDir = pin->direction();
      boost::to_upper( lefDir );
      
      if (lefDir == "INPUT"          ) net->setDirection( PNLNet::Direction::Input       );
      if (lefDir == "OUTPUT"         ) net->setDirection( PNLNet::Direction::Output      );
      if (lefDir == "OUTPUT TRISTATE") net->setDirection( PNLNet::Direction::Tristate );
      if (lefDir == "INOUT"          ) net->setDirection( PNLNet::Direction::InOut    );
    }
    if (net->isSupply())                             net->setGlobal( true );
    if (pin->name()[ strlen(pin->name())-1 ] == '!') net->setGlobal( true );

    // for ( int iport=0 ; iport < pin->numPorts() ; ++iport ) {
    //   Layer* layer = NULL;
      
    //   lefiGeometries* geoms = pin->port( iport );
    //   for ( int igeom=0 ; igeom < geoms->numItems() ; ++igeom ) {
    //     if (geoms->itemType(igeom) == lefiGeomLayerE) {
    //       layer = parser->getLayer( geoms->getLayer(igeom) );
    //       continue;
    //     }
    //     if (geoms->itemType(igeom) == lefiGeomRectE) {
    //       lefiGeomRect* r          = geoms->getRect(igeom);
    //       PNLUnit::Unit     w          = parser->fromUnitsMicrons(r->xh - r->xl);
    //       PNLUnit::Unit     h          = parser->fromUnitsMicrons(r->yh - r->yl);
    //       Segment*      segment    = NULL;
    //       float         formFactor = (float)w / (float)h;
          
    //       if ( (formFactor > 0.5) and not parser->isVH() ) {
    //         segment = Horizontal::create( net, layer
    //                                     , parser->fromUnitsMicrons( (r->yl + r->yh)/2 )
    //                                     , h
    //                                     , parser->fromUnitsMicrons( r->xl )
    //                                     , parser->fromUnitsMicrons( r->xh )
    //                                     );
    //       } else {
    //         segment = Vertical::create( net, layer
    //                                   , parser->fromUnitsMicrons( (r->xl + r->xh)/2 )
    //                                   , w
    //                                   , parser->fromUnitsMicrons( r->yl )
    //                                   , parser->fromUnitsMicrons( r->yh )
    //                                   );
    //       }
    //       if (segment) parser->addPinComponent( pin->name(), segment );
    //     //cerr << "       | " << segment << endl;
    //       continue;
    //     }
    //     if (geoms->itemType(igeom) == lefiGeomPolygonE) {
    //       lefiGeomPolygon* polygon = geoms->getPolygon(igeom);
    //       vector<PNLPoint>    points;
    //       for ( int ipoint=0 ; ipoint<polygon->numPNLPoints ; ++ipoint ) {
    //         points.push_back( PNLPoint( parser->fromUnitsMicrons(polygon->x[ipoint])
    //                                , parser->fromUnitsMicrons(polygon->y[ipoint]) ));
    //       }
    //       points.push_back( PNLPoint( parser->fromUnitsMicrons(polygon->x[0])
    //                              , parser->fromUnitsMicrons(polygon->y[0]) ));
    //       Rectilinear* rectilinear = Rectilinear::create( net, layer, points );
    //       if (rectilinear) parser->addPinComponent( pin->name(), rectilinear );
    //       continue;
    //     }
    //     if (geoms->itemType(igeom) == lefiGeomClassE) {
    //     // Ignore CLASS <site>. Deduced from segments positions.
    //       continue;
    //     }

    //     string geomTypeName;
    //     switch ( geoms->itemType(igeom) ) {
    //       case lefiGeomUnknown:           geomTypeName = "lefiGeomUnknown"          ; break;
    //       case lefiGeomLayerE:            geomTypeName = "lefiGeomLayerE"           ; break;
    //       case lefiGeomLayerMinSpacingE:  geomTypeName = "lefiGeomMinSpacingE"      ; break;
    //       case lefiGeomLayerRuleWidthE:   geomTypeName = "lefiGeomRuleWidthE"       ; break;
    //       case lefiGeomWidthE:            geomTypeName = "lefiGeomWidthE"           ; break;
    //       case lefiGeomPathE:             geomTypeName = "lefiGeomPathE"            ; break;
    //       case lefiGeomPathIterE:         geomTypeName = "lefiGeomPathIterE"        ; break;
    //       case lefiGeomRectE:             geomTypeName = "lefiGeomRectE"            ; break;
    //       case lefiGeomRectIterE:         geomTypeName = "lefiGeomRectIterE"        ; break;
    //       case lefiGeomPolygonE:          geomTypeName = "lefiGeomPolygonE"         ; break;
    //       case lefiGeomPolygonIterE:      geomTypeName = "lefiGeomPolygonIterE"     ; break;
    //       case lefiGeomViaE:              geomTypeName = "lefiGeomViaE"             ; break;
    //       case lefiGeomViaIterE:          geomTypeName = "lefiGeomViaIterE"         ; break;
    //       case lefiGeomClassE:            geomTypeName = "lefiGeomClassE"           ; break;
    //       case lefiGeomLayerExceptPgPNLNetE: geomTypeName = "lefiGeomLayerExceptPgPNLNetE"; break;
    //       case lefiGeomEnd:               geomTypeName = "lefiGeomEnd"              ; break;
    //     }

    //     if (not geomTypeName.empty())
    //       cerr << Warning( "LefParser::_pinCkb(): In PIN \"%s\", unsupported geometry \"%s\"."
    //                      , pin->name(), geomTypeName.c_str() ) << endl;
    //   }
    // }

    return 0;
  }


  void  LefParser::_pinStdPostProcess ()
  {
    printf("LefParser::_pinStdPostProcess\n");
    // const Layer*              metal1      = _routingGauge->getLayerGauge( (size_t)0 )->getLayer();
    // const RoutingLayerGauge*  gaugeMetal2 = _routingGauge->getLayerGauge( 1 );
    //       Box                 ab          = _cell->getAbutmentBox();

    // //cerr << "       @ _pinStdPostProcess" << endl;

    // for ( auto element : _pinComponents ) {
    //   string              pinName    = element.first;
    //   vector<PNLTerm*>& components = element.second;
    //   vector<Segment*>    ongrids;
    //   bool                isSupply = false;

    //   for ( PNLTerm* component : components ) {
    //     if (component->getNet()->isSupply()) {
    //       isSupply = true;
    //       break;
    //     }

    //     Segment* segment = dynamic_cast<Segment*>( component );
    //     if (segment) {
    //       bool isWide = (segment->getWidth() >= getMinTerminalWidth());

    //       // cerr << "       > " << segment << endl;
    //       // if (not isVH()) cerr << "X NOT isVH()" << endl;
    //       // else            cerr << "X isVH()" << endl;
        
    //       if (isVH() and (segment->getLayer()->getMask() == metal1->getMask())) {
    //       // cerr << "isVH()" << endl;
    //         Vertical* v = dynamic_cast<Vertical*>( segment );
    //         if (v) {
    //           PNLUnit::Unit nearestX = gaugeMetal2->getTrackPosition( ab.getXMin()
    //                                                             , ab.getXMax()
    //                                                             , v->getX()
    //                                                             , Constant::Nearest );

    //           if (nearestX == v->getX()) {
    //           } else {
    //             PNLUnit::Unit neighbor = nearestX
    //               + ((nearestX > v->getX()) ? 1 : -1) * gaugeMetal2->getPitch();

    //           //cerr << "       | X:" << PNLUnit::getValueString(v->getX())
    //           //     <<  " nearestX:" << PNLUnit::getValueString(nearestX)
    //           //     <<  " neighbor:" << PNLUnit::getValueString(neighbor)
    //           //     << endl;

    //             if (  (v->getX() - v->getHalfWidth() > neighbor)
    //                or (v->getX() + v->getHalfWidth() < neighbor) ) {
    //               ongrids.push_back( Vertical::create( v->getNet()
    //                                                  , v->getLayer()
    //                                                  , nearestX
    //                                                  , _routingGauge->getLayerGauge((size_t)0)->getWireWidth()
    //                                                  , v->getDySource()
    //                                                  , v->getDyTarget()
    //                                                  )
    //                                );
    //               cerr << "       | " << ongrids[ongrids.size()-1] << endl;
    //             } else {
    //             // Unpitched and not wide enough to be under a metal2 track, ignore.
    //             }

    //             continue;
    //           }
    //         }
    //       }
      
    //       if (isWide) ongrids.push_back( segment );
    //     }
    //     Rectilinear* rectilinear = dynamic_cast<Rectilinear*>( component );
    //     if (rectilinear) {
    //       cerr << "       > " << rectilinear << endl;
    //       if (rectilinear->getLayer()->getMask() != metal1->getMask())
    //         continue;

    //       vector<Box> boxes;
    //       rectilinear->getAsRectangles( boxes );

    //       if (component->getNet()->isSupply()) {
    //         ongrids.push_back( Horizontal::create( rectilinear->getNet()
    //                                              , rectilinear->getLayer()
    //                                              , boxes.front().getYCenter()
    //                                              , boxes.front().getHeight()
    //                                              , _cell->getAbutmentBox().getXMin()
    //                                              , _cell->getAbutmentBox().getXMax()
    //                                              )
    //                              );
    //       } else {
    //         for ( const Box& box : boxes ) {
    //           PNLUnit::Unit nearestX = gaugeMetal2->getTrackPosition( ab.getXMin()
    //                                                             , ab.getXMax()
    //                                                             , box.getXCenter()
    //                                                             , Constant::Nearest );
    //           PNLUnit::Unit xmin = std::min( box.getXMin(), nearestX - gaugeMetal2->getViaWidth()/2 );
    //           PNLUnit::Unit xmax = std::max( box.getXMax(), nearestX + gaugeMetal2->getViaWidth()/2 );
    //           ongrids.push_back( Vertical::create( rectilinear->getNet()
    //                                              , rectilinear->getLayer()
    //                                              , (xmax+xmin)/2
    //                                              ,  xmax-xmin
    //                                              , box.getYMin()
    //                                              , box.getYMax()
    //                                              )
    //                              );
    //           // PNLUnit::Unit neighbor = nearestY
    //           //   + ((nearestY > box.getYCenter()) ? 1 : -1) * gaugeMetal2->getPitch();
              
    //           // if (  (box.getYMin() > neighbor)
    //           //    or (box.getYMax() < neighbor) ) {
    //           //   ongrids.push_back( Vertical::create( rectilinear->getNet()
    //           //                                      , rectilinear->getLayer()
    //           //                                      , box.getXCenter()
    //           //                                      , box.getWidth()
    //           //                                      , box.getYMin()
    //           //                                      , box.getYMax()
    //           //                                      )
    //           //                    );
    //           // }
    //         }
    //       }
    //     }
    //   }

    //   if (ongrids.empty()) {
    //     if (not isSupply)
    //       cerr << Warning( "LefParser::_pinStdPostProcess(): Pin \"%s\" has no terminal ongrid."
    //                      , pinName.c_str() ) << endl;
    //     for ( PNLTerm* component : components ) {
    //       PNLNetExternalComponents::setExternal( component );
    //     }
    //   } else {
    //     for ( Segment* segment : ongrids ) {
    //       PNLNetExternalComponents::setExternal( segment );
    //     }
    //   }
    // }
  }


  void  LefParser::_pinPadPostProcess ()
  {
    printf("LefParser::_pinPadPostProcess\n");
    // Box  ab          = getPNLDesign()->getAbutmentBox();
    // bool isCornerPad = false; //(_cellGauge) and (_cellGauge->getSliceHeight() == _cellGauge->getSliceStep());

    // for ( auto element : _pinComponents ) {
    //   string              pinName  = element.first;
    //   vector<PNLTerm*>& segments = element.second;
    //   vector<Segment*>    ongrids;

    //   if (segments.empty()) continue;
      
    //   PNLNet* net = segments[0]->getNet();

    //   for ( size_t i=0 ; i<segments.size() ; ++i ) {
    //     Box      bb         = segments[i]->getBoundingBox();
    //     Interval hspan      = Interval( bb.getXMin(), bb.getXMax() );
    //     Interval vspan      = Interval( bb.getYMin(), bb.getYMax() );
    //     Segment* capSegment = NULL;

    //     if (segments[i]->getLayer()->isBlockage()) continue;

    //     if (net->isSupply()) {
    //       if (hspan.contains(ab.getXMin())) {
    //         capSegment = Horizontal::create( net
    //                                        , segments[i]->getLayer()
    //                                        , vspan.getCenter()
    //                                        , vspan.getSize()
    //                                        , ab.getXMin()
    //                                        , hspan.getVMax()
    //                                        );
    //       } else if (hspan.contains(ab.getXMax())) {
    //         capSegment = Horizontal::create( net
    //                                        , segments[i]->getLayer()
    //                                        , vspan.getCenter()
    //                                        , vspan.getSize()
    //                                        , hspan.getVMin()
    //                                        , ab.getXMax()
    //                                        );
    //       }
    //     }

    //     if (not capSegment) {
    //       vector<PNLUnit::Unit> distanceToSide;
    //       distanceToSide.push_back( std::abs(bb.getXMin() - ab.getXMin()) );  // West.
    //       distanceToSide.push_back( std::abs(ab.getXMax() - bb.getXMax()) );  // East.
    //       distanceToSide.push_back( std::abs(bb.getYMin() - ab.getYMin()) );  // South.
    //       distanceToSide.push_back( std::abs(ab.getYMax() - bb.getYMax()) );  // North.

    //       size_t closestSide = ((isCornerPad) ? 0 : 2);
    //       for ( size_t i=closestSide ; i < distanceToSide.size() ; ++i ) {
    //         if (distanceToSide[i] < distanceToSide[closestSide])
    //           closestSide = i;
    //       }

    //       switch ( closestSide ) {
    //         default:
    //         case 0:  // West.
    //           capSegment = Horizontal::create( net
    //                                          , segments[i]->getLayer()
    //                                          , vspan.getCenter()
    //                                          , vspan.getSize()
    //                                          , ab.getXMin()
    //                                          , hspan.getVMax()
    //                                          );
    //           break;
    //         case 1:  // East.
    //           capSegment = Horizontal::create( net
    //                                          , segments[i]->getLayer()
    //                                          , vspan.getCenter()
    //                                          , vspan.getSize()
    //                                          , hspan.getVMin()
    //                                          , ab.getXMax()
    //                                          );
    //           break;
    //         case 2:  // South.
    //           capSegment = Vertical::create( net
    //                                        , segments[i]->getLayer()
    //                                        , hspan.getCenter()
    //                                        , hspan.getSize()
    //                                        , ab.getYMin()
    //                                        , vspan.getVMax()
    //                                        );
    //           break;
    //         case 3:  // North.
    //           capSegment = Vertical::create( net
    //                                        , segments[i]->getLayer()
    //                                        , hspan.getCenter()
    //                                        , hspan.getSize()
    //                                        , vspan.getVMin()
    //                                        , ab.getYMax()
    //                                        );
    //           break;
    //       }
    //     }

    //     if (capSegment) {
    //       PNLNetExternalComponents::setExternal( capSegment );
    //       segments[i]->destroy();
    //       segments[i] = NULL;
    //     }
    //   }
    // }
  }


  int  LefParser::flushErrors ()
  {
    int code = (hasErrors()) ? 1 : 0;

    for ( size_t ierror=0 ; ierror < _errors.size() ; ++ierror ) {
        //string message = "LefImport::load(): " + _errors[ierror];
       //cerr << Error(message.c_str(),getString(_library->getName()).c_str()) << endl;
       assert(false);
    }
    clearErrors ();

    return code;
  }


  SNLLibrary* LefParser::parse ( string file )
  {
    cmess1 << "  o  LEF: <" << file << ">" << endl;

    PNLUnit::setStringMode( PNLUnit::Physical, PNLUnit::Micro );

    size_t iext = file.rfind( '.' );
    if (file.compare(iext,4,".lef") != 0) {
      //throw Error( "LefImport::load(): DEF files must have  \".lef\" extension <%s>.", file.c_str() );
      assert(false);
    }

    size_t  islash = file.rfind( '/' );
    islash = (islash == string::npos) ? 0 : islash+1;

    string                libraryName = file.substr( islash, file.size()-4-islash );
    unique_ptr<LefParser> parser      ( new LefParser(file,libraryName) );
    // std::ifstream f("file.lef");
    // if (f.is_open())
    //     std::cout << f.rdbuf();

    FILE* lefStream = fopen( file.c_str(), "r" );
    
    if (not lefStream) {
      //throw Error( "LefImport::load(): Cannot open LEF file \"%s\".", file.c_str() );
      assert(false);
    }
    parser->createSNLLibrary( );
    lefrRead( lefStream, file.c_str(), (lefiUserData)parser.get() );
    fclose( lefStream );

    // if (not parser->getPNLDesignGauge()) {
    //   cerr << Warning( "LefParser::parse(): No default Alliance cell gauge, unable to check the PNLDesign gauge." ) << endl;
    // } else if (not parser->getCoreSiteX()) {
    //   cerr << Warning( "LefParser::parse(): No CORE site found in library, unable to check the PNLDesign gauge." ) << endl;
    // } else {
    //   if (parser->getCoreSiteY() != parser->getPNLDesignGauge()->getSliceHeight())
    //     cerr << Warning( "LefParser::parse(): CRL slice height discrepency %s while LEF is %s."
    //                    , PNLUnit::getValueString(parser->getPNLDesignGauge()->getSliceHeight()).c_str()
    //                    , PNLUnit::getValueString(parser->getCoreSiteY()).c_str() ) << endl;
        
    //   if (parser->getCoreSiteX() != parser->getPNLDesignGauge()->getSliceStep())
    //     cerr << Warning( "LefParser::parse(): CRL slice step discrepency %s while LEF is %s."
    //                    , PNLUnit::getValueString(parser->getPNLDesignGauge()->getSliceStep()).c_str()
    //                    , PNLUnit::getValueString(parser->getCoreSiteX()).c_str() ) << endl;
    // }
    
    return parser->getLibrary();
  }

}  // Anonymous namespace.



//namespace CRL {

  using std::cerr;
  using std::endl;
  using std::string;
  //using Hurricane::UpdateSession;


  SNLLibrary* LefImport::load ( string fileName )
  {
    //UpdateSession::open ();

    SNLLibrary* library = NULL;
//#if defined(HAVE_LEFDEF)
    library = LefParser::parse ( fileName );
//#else
//     cerr << "[ERROR] CRL::LefImport::load(): \n"
//          << "  Coriolis2 hasn't been compiled with LEF/DEF support. To enable LEF/DEF\n"
//          << "  support, you may obtain parser/driver from Si2 (www.si2.org) then recompile."
//          << endl;
// #endif

    //UpdateSession::close ();

    return library;
  }


  void  LefImport::reset ()
  {
#if defined(HAVE_LEFDEF)
    LefParser::reset();
#endif
  }


  void  LefImport::setMergeLibrary ( SNLLibrary* library )
  {
#if defined(HAVE_LEFDEF)
    LefParser::setMergeLibrary( library );
#endif
  }


  void  LefImport::setGdsForeignDirectory ( string path )
  {
#if defined(HAVE_LEFDEF)
    LefParser::setGdsForeignDirectory( path );
#endif
  }


//}  // End of CRL namespace.
