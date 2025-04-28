// SPDX-FileCopyrightText: 2025 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include <cstdio>
#include <cstring>
#include <memory>
#include <boost/algorithm/string.hpp>
#  include "lefrReader.hpp"
#  include "defrReader.hpp"
// #include "hurricane/Error.h"
// #include "hurricane/Warning.h"
// #include "hurricane/DataBase.h"
// #include "hurricane/Technology.h"
// #include "hurricane/BasicLayer.h"
#include "PNLNet.h"
//#include "hurricane/PNLNetExternalComponents.h"
//#include "hurricane/Pad.h"
//#include "hurricane/Contact.h"
//#include "hurricane/Horizontal.h"
//#include "hurricane/Vertical.h"
#include "PNLDesign.h"
#include "NLLibrary.h"
//#include "hurricane/UpdateSession.h"
//#include "crlcore/Utilities.h"
//#include "crlcore/ToolPNLBox.h"
//#include "crlcore/AllianceFramework.h"
#include "DefImport.h"
#include "PNLBox.h"
#include "PNLTransform.h"
#include "PNLOrientation.h"
#include "PNLInstTerm.h"
#include "NLDB.h"
//for ostringstream
#include <sstream>
#include "NLUniverse.h"


using namespace std;
using namespace naja::NL;
using namespace naja::NL;

namespace {

  using namespace std;

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
  extern mstream  cmess2;
  mstream  cmess2    ( mstream::Verbose0, std::cout );
  extern mstream  cmess1;
  mstream  cmess1    ( mstream::Verbose0, std::cout );

  void  addSupplyPNLNets ( PNLDesign* cell )
  {
    /*PNLNet* vcc = cell->addNet(NLName("VDD") );
    vcc->setExternal ( true );
    vcc->setGlobal   ( true );
    vcc->setType     ( PNLNet::Type::VDD );

    PNLNet* gnd = cell->addNet(NLName("GND") );
    gnd->setExternal ( true );
    gnd->setGlobal   ( true );
    gnd->setType     ( PNLNet::Type::GND );*/
  }


  typedef  tuple<PNLNet* ,uint32_t>  PNLNetDatas;
  typedef  tuple<PNLDesign*,uint32_t>  ViaDatas;


  class DefParser {
    public:
      const uint32_t NoPatch =  0;
      const uint32_t Sky130  = (1 << 10);
    public:
      //static AllianceFramework* getFramework             ();
      PNLDesign*              getLefPNLDesign               ( string name );
      static void               setUnits                 ( double );
      static PNLBox::Unit          fromDefUnits             ( int );
      static PNLOrientation                 
                                fromDefOrientation       ( int orient );
      inline NLLibrary*      getLibrary  ( bool create ) { if (not _library /*and create*/) createDEFLibrary(); return _library; }
      NLLibrary* createDEFLibrary ();
      static PNLTransform     getPNLTransform        ( const PNLBox&
                                                         , const PNLBox::Unit x
                                                         , const PNLBox::Unit y
                                                         , const PNLOrientation
                                                         );
      static PNLDesign*         parse                    ( string file, unsigned int flags, naja::NL::NLDB* db );
                                DefParser                ( string file, /*AllianceNLLibrary*,*/ unsigned int flags, naja::NL::NLDB* db );
                               ~DefParser                ();
      inline bool               hasErrors                ();
      inline bool               isSky130                 () const;
      inline unsigned int       getFlags                 () const;
      //inline AllianceNLLibrary*   getLibrary               ();
      inline PNLDesign*              getPNLDesign                  ();
      inline size_t             getPitchs                () const;
      inline size_t             getSlices                () const;
      inline const PNLBox&         getFitOnPNLDesignsDieArea     () const;
             PNLNet*               getPrebuildPNLNet           ( bool create=true );
      inline string             getBusBits               () const;
             PNLNetDatas*          lookupPNLNet                ( string );
             ViaDatas*          lookupVia                ( string );
            //  Layer*             lookupLayer              ( string );
      inline vector<string>&    getErrors                ();
      inline void               pushError                ( string );
             int                flushErrors              ();
      inline void               clearErrors              ();
      inline void               setPitchs                ( size_t );
      inline void               setSlices                ( size_t );
      inline void               setPrebuildPNLNet           ( PNLNet* );
      inline void               setBusBits               ( string );
             PNLNetDatas*          addPNLNetLookup             ( string netName, PNLNet* );
            ViaDatas*          addViaLookup             ( string viaName, PNLDesign* );
             void               toHurricaneName          ( string& );
      inline void               mergeToFitOnPNLDesignsDieArea ( const PNLBox& );
            // Contact*           createVia                ( string viaName, PNLNet*, PNLBox::Unit x, PNLBox::Unit y );
    private:                                         
      static int                _unitsCbk                ( defrCallbackType_e, double        , defiUserData );
      static int                _busBitCbk               ( defrCallbackType_e, const char*   , defiUserData );
      static int                _designEndCbk            ( defrCallbackType_e, void*         , defiUserData );
      static int                _dieAreaCbk              ( defrCallbackType_e, defiBox*      , defiUserData );
      static int                _pinCbk                  ( defrCallbackType_e, defiPin*      , defiUserData );
      static int                _viaCbk                  ( defrCallbackType_e, defiVia*      , defiUserData );
      static int                _componentCbk            ( defrCallbackType_e, defiComponent*, defiUserData );
      static int                _componentEndCbk         ( defrCallbackType_e, void*         , defiUserData );
      static int                _netCbk                  ( defrCallbackType_e, defiNet*      , defiUserData );
      static int                _netEndCbk               ( defrCallbackType_e, void*         , defiUserData );
      static int                _snetCbk                 ( defrCallbackType_e, defiNet*      , defiUserData );
      static int                _pathCbk                 ( defrCallbackType_e, defiPath*     , defiUserData );
             PNLDesign*              _createPNLDesign              ( const char* name );
    private:
      static double                _defUnits;
      //static AllianceFramework*    _framework;
      //static Technology*           _technology;
      static NLLibrary*              _lefRootNLLibrary;
             uint32_t              _flags;
             string                _file;
             //AllianceNLLibrary*      _library;
             string                _busBits;
             PNLDesign*                 _cell;
             size_t                _pitchs;
             size_t                _slices;
             PNLBox                   _fitOnPNLDesignsDieArea;
             PNLNet*                  _prebuildPNLNet;
             map<string,PNLNetDatas>  _netsLookup;
             map<string,ViaDatas>  _viasLookup;
             vector<string>        _errors;
             naja::NL::NLLibrary*  _library;
             naja::NL::NLDB*                _db = nullptr;           
  };


  double             DefParser::_defUnits       = 0.01;
  //AllianceFramework* DefParser::_framework      = NULL;
  //Technology*        DefParser::_technology     = NULL;
  NLLibrary*           DefParser::_lefRootNLLibrary = NULL;


  DefParser::DefParser ( string file, /*AllianceNLLibrary* library,*/ unsigned int flags, naja::NL::NLDB* db )
    : _flags            (flags)
    , _file             (file)
    //, _library          (library)
    , _busBits          ("()")
    , _cell             (NULL)
    , _pitchs           (0)
    , _slices           (0)
    , _fitOnPNLDesignsDieArea()
    , _prebuildPNLNet      (NULL)
    , _netsLookup       ()
    , _viasLookup       ()
    , _errors           ()
    , _db(db)
  {
    defrInit               ();
    defrSetUnitsCbk        ( _unitsCbk );
    defrSetBusBitCbk       ( _busBitCbk );
    defrSetDesignEndCbk    ( _designEndCbk );
    defrSetDieAreaCbk      ( _dieAreaCbk );
    defrSetViaCbk          ( _viaCbk );
    defrSetPinCbk          ( _pinCbk );
    defrSetComponentCbk    ( _componentCbk );
    defrSetComponentEndCbk ( _componentEndCbk );
    defrSetNetCbk          ( _netCbk );
    defrSetNetEndCbk       ( _netEndCbk );
    defrSetSNetCbk         ( _snetCbk );
    defrSetPathCbk         ( _pathCbk );

    /*if (DataBase::getDB()->getTechnology()->getName() == "Sky130") {
      cmess1 << "     - Enabling SkyWater 130nm harness hacks." << endl;
      _flags |= Sky130;
    }*/
  }


  DefParser::~DefParser ()
  {
    defrReset ();
  }


         //AllianceFramework* DefParser::getFramework             () { return _framework; }
  inline void               DefParser::setUnits                 ( double units ) { _defUnits = 1/units; }
  inline PNLBox::Unit          DefParser::fromDefUnits             ( int u ) { return u;/*PNLUnit::fromPhysical(_defUnits*(double)u,PNLUnit::UnitPower::Micro);*/ }
  inline bool               DefParser::isSky130                 () const { return _flags & Sky130; }
  inline bool               DefParser::hasErrors                () { return not _errors.empty(); }
  inline unsigned int       DefParser::getFlags                 () const { return _flags; }
  inline string             DefParser::getBusBits               () const { return _busBits; }
  //inline AllianceNLLibrary*   DefParser::getLibrary               () { return _library; }
  inline PNLDesign*              DefParser::getPNLDesign                  () { return _cell; }
  inline size_t             DefParser::getPitchs                () const { return _pitchs; }
  inline size_t             DefParser::getSlices                () const { return _slices; }
  inline const PNLBox&         DefParser::getFitOnPNLDesignsDieArea     () const { return _fitOnPNLDesignsDieArea; }
  inline vector<string>&    DefParser::getErrors                () { return _errors; }
  inline void               DefParser::pushError                ( string error ) { _errors.push_back(error); }
  inline void               DefParser::clearErrors              () { return _errors.clear(); }
  inline void               DefParser::setPitchs                ( size_t pitchs ) { _pitchs=pitchs; }
  inline void               DefParser::setSlices                ( size_t slices ) { _slices=slices; }
  inline void               DefParser::setPrebuildPNLNet           ( PNLNet* net ) { _prebuildPNLNet=net; }
  inline void               DefParser::setBusBits               ( string busbits ) { _busBits = busbits; }
  inline void               DefParser::mergeToFitOnPNLDesignsDieArea ( const PNLBox& box ) { _fitOnPNLDesignsDieArea.merge(box); }

  
  PNLNet* DefParser::getPrebuildPNLNet ( bool create )
  {
    if (create and not _prebuildPNLNet) {
      _prebuildPNLNet = _cell->addNet( naja::NL::NLName( "__prebuildnet__" ));
    }
    return _prebuildPNLNet;
  }

  NLLibrary* DefParser::createDEFLibrary ()
  {
    // if (_mergeNLLibrary) {
    //   _library = _mergeNLLibrary;
    //   return _library;
    // }
    NLLibrary* rootNLLibrary = _db->getLibrary( NLName("LIB1") );
    if (rootNLLibrary == nullptr) {
      NLLibrary* rootNLLibrary = NLLibrary::create( _db, NLName("LIB1") );
    }
    NLLibrary* defRootNLLibrary = NLLibrary::create( rootNLLibrary, NLName("DEF") );
    _library = defRootNLLibrary;
    /*_library = lefRootNLLibrary->getLibrary( NLName(_libraryName) );
    if (_library) {
      assert(false);
      // Error
    } else {
      _library = NLLibrary::create( lefRootNLLibrary, NLName(_libraryName) );
    }*/
    return _library;
  }


  PNLDesign* DefParser::getLefPNLDesign ( string name )
  {
    // if (not _lefRootNLLibrary) {
    //   NLLibrary*  rootNLLibrary = _db->getRootNLLibrary();

    //   if (rootNLLibrary) {
    //     _lefRootNLLibrary = rootNLLibrary->getLibrary( "LEF" );
    //   }
    // }
    printf("getLefPNLDesign\n");
    NLLibrary* rootNLLibrary = _db->getLibrary(NLName("LIB1") );
    _lefRootNLLibrary =  _db->getLibrary(NLName("LEF") );
    assert(_lefRootNLLibrary != nullptr);
    PNLDesign* masterPNLDesign = NULL;
    if (_lefRootNLLibrary) {
      printf("getLefPNLDesign here\n");
      for ( NLLibrary* library : _lefRootNLLibrary->getLibraries() ) {
        printf("library %s\n",library->getName().getString().c_str() );
        masterPNLDesign = library->getPNLDesign( NLName(name) );
        if (masterPNLDesign) break;
      }
    }

    //if (not masterPNLDesign)
    //  masterPNLDesign = DefParser::getFramework()->getPNLDesign ( name, Catalog::State::Views );

    return masterPNLDesign;
  }


  PNLOrientation  DefParser::fromDefOrientation ( int orient )
  {
  // Note : the codes between DEF & Hurricane matches.
  // This function is just to be clear.
    switch ( orient ) {
      default:
      case 0: break;                                  // N, default.
      case 1: return PNLOrientation::Type::R90; // W
      case 2: return PNLOrientation::Type::R180; // S
      case 3: return PNLOrientation::Type::R270; // E
      case 4: return PNLOrientation::Type::MX; // FN
      case 5: return PNLOrientation::Type::MXR90; // FW
      case 6: return PNLOrientation::Type::MY; // FS
      case 7: return PNLOrientation::Type::MYR90; // FE
    }
    return PNLOrientation::Type::R0;
  }


  void  DefParser::toHurricaneName ( string& defName )
  {
    if (_busBits != "()") {
      if (defName[defName.size()-1] == _busBits[1]) {
        size_t pos = defName.rfind( _busBits[0] );
        if (pos != string::npos) {
          defName[pos]              = '(';
          defName[defName.size()-1] = ')';
        }
      }
    }
  }


  PNLTransform  DefParser::getPNLTransform ( const PNLBox& abox
                                               , PNLBox::Unit  x
                                               , PNLBox::Unit  y
                                               , const PNLOrientation orientation
                                               )
  {
	switch (orientation.getType().getType()) {
      default:
      case PNLOrientation::Type::R0: return PNLTransform ( x, y, orientation );
      case PNLOrientation::Type::R90: return PNLTransform ( x, y+abox.getWidth(), orientation );
      case PNLOrientation::Type::R180: return PNLTransform ( x+abox.getWidth (), y+abox.getHeight(), orientation );
      case PNLOrientation::Type::R270: return PNLTransform ( x+abox.getHeight(), y, orientation );
      case PNLOrientation::Type::MX: return PNLTransform ( x+abox.getWidth (), y, orientation );
      case PNLOrientation::Type::MXR90: return PNLTransform ( x+abox.getHeight(), y+abox.getWidth(), orientation );
      case PNLOrientation::Type::MY: return PNLTransform ( x, y+abox.getHeight(), orientation );
      case PNLOrientation::Type::MYR90: return PNLTransform ( x+abox.getHeight(), y+abox.getWidth(), orientation );
	}
	return PNLTransform ();
  }


  PNLDesign* DefParser::_createPNLDesign ( const char* name )
  {
    //_cell = DefParser::getFramework()->createPNLDesign ( name, NULL );
    _cell = PNLDesign::create( getLibrary(true), NLName(name) );
    _cell->setClassType(PNLDesign::ClassType::BLOCK);
    addSupplyPNLNets ( _cell );
    return _cell;
  }


  int  DefParser::flushErrors ()
  {
    int code = (hasErrors()) ? 1 : 0;

    for ( size_t ierror=0 ; ierror < _errors.size() ; ++ierror ) {
        string message = "DefImport::load(): " + _errors[ierror];
        cerr << message.c_str() << " " << _cell->getName().getString().c_str() << endl;
    }
    clearErrors ();

    return code;
  }


  PNLNetDatas* DefParser::lookupPNLNet ( string netName )
  {
    map<string,PNLNetDatas>::iterator imap = _netsLookup.find(netName);
    if ( imap == _netsLookup.end() ) return NULL;

    return &( (*imap).second );
  }


  PNLNetDatas* DefParser::addPNLNetLookup ( string netName, PNLNet* net )
  {
    PNLNetDatas* netDatas = lookupPNLNet( netName );
    if (not netDatas) {
      auto insertIt = _netsLookup.insert( make_pair( netName, make_tuple(net,0) ));
      netDatas = &( ((*(insertIt.first)).second) );
    }
    return netDatas;
  }


  ViaDatas* DefParser::lookupVia ( string viaName )
  {
    map<string,ViaDatas>::iterator imap = _viasLookup.find(viaName);
    if (imap == _viasLookup.end() ) return NULL;

    return &( (*imap).second );
  }


  ViaDatas* DefParser::addViaLookup ( string viaName, PNLDesign* via )
  {
    ViaDatas* viaDatas = lookupVia( viaName );
    if (not viaDatas) {
      auto insertIt = _viasLookup.insert( make_pair( viaName, make_tuple(via,0) ));
      viaDatas = &( ((*(insertIt.first)).second) );
    }
    return viaDatas;
  }


  // Contact* DefParser::createVia ( string viaName, PNLNet* net, PNLBox::Unit x, PNLBox::Unit y )
  // {
  //   ViaDatas* viaDatas = lookupVia( viaName );
  //   if (not viaDatas) return NULL;

  //   string instName = viaName + "_" + getString( get<1>(*viaDatas)++ );
  //   PNLDesign*     viaPNLDesign = get<0>( *viaDatas );
  //   PNLInstance::create( getPNLDesign()
  //                   , instName
  //                   , viaPNLDesign
  //                   , PNLTransform( x, y )
  //                   , PNLInstance::PlacementStatus::Fixed
  //                   );
  //   PNLNet* viaPNLNet     = viaPNLDesign->getNet( "via" );
  //   Pad* metalPlate = NULL;
  //   for ( Pad* pad : viaPNLNet->getPads() ) {
  //     const BasicLayer* basicLayer = dynamic_cast<const BasicLayer*>( pad->getLayer() );
  //     if (basicLayer and (basicLayer->getMaterial() == BasicLayer::Material::metal)) {
  //       metalPlate = pad;
  //       break;
  //     }
  //   }

  //   return Contact::create( net, metalPlate->getLayer(), x, y, 0, 0 );
  // }


  // Layer* DefParser::lookupLayer ( string layerName )
  // {
  //   if (_flags & Sky130) {
  //     if (layerName.substr(0,3) == "met") layerName.erase( 1, 2 );
  //   }
  //   return _technology->getLayer( layerName );
  // }


  int  DefParser::_unitsCbk ( defrCallbackType_e c, double defUnits, lefiUserData ud )
  {
    DefParser* parser = (DefParser*)ud;
    parser->setUnits( defUnits );
    return 0;
  }


  int  DefParser::_busBitCbk ( defrCallbackType_e c, const char* busbits, lefiUserData ud )
  {
    DefParser* parser = (DefParser*)ud;

    if (strlen(busbits) == 2) {
      parser->setBusBits( busbits );
    } else {
      // ostringstream message;
      // message << "BUSBITCHARS is not two character long (" << busbits << ")";
      // parser->pushError( message.str() );
      assert(false);
    }

    return 0;
  }


  int  DefParser::_designEndCbk ( defrCallbackType_e c, void*, lefiUserData ud )
  {
    DefParser* parser = (DefParser*)ud;

    if (      (parser->getFlags() & DefImport::FitAbOnDesigns)
       and not parser->getFitOnPNLDesignsDieArea().isEmpty() ) {
      parser->getPNLDesign()->setAbutmentBox ( parser->getFitOnPNLDesignsDieArea() );
    }
      
    return 0;
  }


  int  DefParser::_dieAreaCbk ( defrCallbackType_e c, defiBox* box, lefiUserData ud )
  {
    DefParser* parser = (DefParser*)ud;

    parser->getPNLDesign()->setAbutmentBox
      ( PNLBox ( fromDefUnits(box->xl())
            , fromDefUnits(box->yl())
            , fromDefUnits(box->xh())
            , fromDefUnits(box->yh()) ) );

    return 0;
  }


  int  DefParser::_viaCbk ( defrCallbackType_e c, defiVia* via, defiUserData ud )
  {
    printf("via cb\n");
    DefParser* parser = (DefParser*)ud;
    string viaName = via->name();

    PNLDesign* viaPNLDesign = PNLDesign::create( parser->getPNLDesign()->getLibrary(), NLName(viaName) );
    viaPNLDesign->setTerminalNetlist( true );
    if (via->hasViaRule()) {
      char* viaRuleName;
      char* defbotLayer;
      char* defcutLayer;
      char* deftopLayer;
      int   defxCutSize    = 0;
      int   defyCutSize    = 0;
      int   defxCutSpacing = 0;
      int   defyCutSpacing = 0;
      int   defxBotEnc     = 0;
      int   defyBotEnc     = 0;
      int   defxTopEnc     = 0;
      int   defyTopEnc     = 0;
      int   numCutRows     = 1;
      int   numCutCols     = 1;
      via->viaRule( &viaRuleName
                  , &defxCutSize
                  , &defyCutSize
                  , &defbotLayer
                  , &defcutLayer
                  , &deftopLayer
                  , &defxCutSpacing
                  , &defyCutSpacing
                  , &defxBotEnc
                  , &defyBotEnc
                  , &defxTopEnc
                  , &defyTopEnc );
      if (via->hasRowCol())
        via->rowCol( &numCutRows, &numCutCols );
      PNLBox::Unit  xCutSize    = fromDefUnits( defxCutSize );
      PNLBox::Unit  yCutSize    = fromDefUnits( defyCutSize );
      PNLBox::Unit  xCutSpacing = fromDefUnits( defxCutSpacing );
      PNLBox::Unit  yCutSpacing = fromDefUnits( defyCutSpacing );
      PNLBox::Unit  xBotEnc     = fromDefUnits( defxBotEnc );
      PNLBox::Unit  yBotEnc     = fromDefUnits( defyBotEnc );
      PNLBox::Unit  xTopEnc     = fromDefUnits( defxTopEnc );
      PNLBox::Unit  yTopEnc     = fromDefUnits( defyTopEnc );
      // Layer*     botLayer    = parser->lookupLayer( defbotLayer );
      // Layer*     cutLayer    = parser->lookupLayer( defcutLayer );
      // Layer*     topLayer    = parser->lookupLayer( deftopLayer );
      PNLNet*       net         = viaPNLDesign->addNet(NLName("via")); 
      PNLBox        cellBb;

      PNLBox::Unit  halfXSide   = xTopEnc + (xCutSize*numCutRows + xCutSpacing*(numCutRows-1)) / 2;
      PNLBox::Unit  halfYSide   = yTopEnc + (xCutSize*numCutCols + xCutSpacing*(numCutRows-1)) / 2;
      PNLBox padBb = PNLBox( 0, 0 );
      padBb.increase( halfXSide, halfYSide );
      cellBb.merge( padBb );
      //Pad::create( net, topLayer, padBb );
      halfXSide = xBotEnc + (xCutSize*numCutRows + xCutSpacing*(numCutRows-1)) / 2;
      halfYSide = yBotEnc + (xCutSize*numCutCols + xCutSpacing*(numCutRows-1)) / 2;
      padBb = PNLBox( 0, 0 );
      padBb.increase( halfXSide, halfYSide );
      cellBb.merge( padBb );
      // Pad::create( net, botLayer, padBb );

      // PNLBox::Unit x = - (xCutSize*numCutRows + xCutSpacing*(numCutRows-1)) / 2;
      // for ( int row=0 ; row<numCutRows ; ++row ) {
      //   PNLBox::Unit y = - (yCutSize*numCutCols + xCutSpacing*(numCutCols-1)) / 2;
      //   for ( int col=0 ; col<numCutCols ; ++col ) {
      //     Pad::create( net, cutLayer, PNLBox( x, y, x+xCutSize, y+yCutSize ));
      //     y += yCutSize + yCutSpacing;
      //   }
      //   x += xCutSize + xCutSpacing;
      // }
      viaPNLDesign->setAbutmentBox( cellBb );
    }
    parser->addViaLookup( viaName, viaPNLDesign );

    return 0;
  }
  

  int  DefParser::_pinCbk ( defrCallbackType_e c, defiPin* pin, lefiUserData ud )
  {
    /*DefParser* parser = (DefParser*)ud;

  //cerr << "     - Pin " << pin->pinName() << ":" << pin->netName() << endl;

    string netName = pin->netName();
    string pinName = pin->pinName();
    parser->toHurricaneName( netName );
    parser->toHurricaneName( pinName );
    if (parser->isSky130() and (pinName.substr(0,3) == "io_" ))
      netName = pinName;

    PNLNetDatas* netDatas = parser->lookupPNLNet( netName );
    PNLNet*      hnet     = NULL;
    if (not netDatas) {
      hnet     = PNLNet::create( parser->getPNLDesign(), netName );
      netDatas = parser->addPNLNetLookup( netName, hnet );
    //if (not netName.compare(pin->pinName()))
    //   parser->addPNLNetLookup( pin->pinName(), hnet );
    } else
      hnet = get<0>( *netDatas );
    pinName += '.' + getString( get<1>(*netDatas)++ );

    if (pin->hasDirection()) {
      string defDir = pin->direction();
      boost::to_upper( defDir );
      if (defDir == "INPUT"          ) hnet->setDirection( PNLNet::Direction::Input       );
      if (defDir == "OUTPUT"         ) hnet->setDirection( PNLNet::Direction::Output      );
      if (defDir == "OUTPUT TRISTATE") hnet->setDirection( PNLNet::Direction::Tristate );
      if (defDir == "INOUT"          ) hnet->setDirection( PNLNet::Direction::InOut    );
    }

    if (pin->hasUse()) {
      string defUse = pin->use();
      boost::to_upper( defUse );
      if (defUse == "SIGNAL") hnet->setType( PNLNet::Type::LOGICAL );
    //if (defUse == "ANALOG") hnet->setType( PNLNet::Type::ANALOG  );
      if (defUse == "CLOCK" ) hnet->setType( PNLNet::Type::CLOCK   );
      if (defUse == "VDD" ) hnet->setType( PNLNet::Type::VDD   );
      if (defUse == "GND") hnet->setType( PNLNet::Type::GND  );
    }

    if (pin->hasSpecial() and (hnet->isSupply() or hnet->isClock()))
       hnet->setGlobal( true );

    if (pin->isPlaced() or pin->isFixed()) {
      PNLPoint position ( fromDefUnits(pin->placementX()), fromDefUnits(pin->placementY()) );
      string layerName = pin->layer(0);
      // Layer* layer     = parser->lookupLayer( layerName );
      int    x1        = 0;
      int    y1        = 0;
      int    x2        = 0;
      int    y2        = 0;
      pin->bounds( 0, &x1, &y1, &x2, &y2 );
      PNLBox shape ( fromDefUnits(x1)
                , fromDefUnits(y1)
                , fromDefUnits(x2)
                , fromDefUnits(y2) );

      if (not layer) {
        ostringstream message;
        message << "PIN \"" << pinName << "\" of net \"" << netName << "\" use an unkwown layer \""
                << layerName << "\".";
        parser->pushError( message.str() );
        return 0;
      }

      Pin* pin = Pin::create( hnet
                            , pinName
                            , Pin::AccessDirection::UNDEFINED
                            , Pin::PlacementStatus::Fixed
                            , layer
                            , position.getX()
                            , position.getY()
                            , shape.getWidth()
                            , shape.getHeight()
                            );
      if (not hnet->isExternal()) hnet->setExternal( true );
      PNLNetExternalComponents::setExternal( pin );
    }*/

    return 0;
  }


  int  DefParser::_componentCbk ( defrCallbackType_e c, defiComponent* component, lefiUserData ud )
  {
    DefParser* parser = (DefParser*)ud;

    string componentName = component->name();
    string componentId   = component->id();
    PNLDesign*  masterPNLDesign    = parser->getLefPNLDesign( componentName );

    if ( masterPNLDesign == NULL ) {
      ostringstream message;
      message << "Unknown model/PNLDesign (LEF MACRO) " << componentName << " in <%s>.";
      parser->pushError ( message.str() );
      return 0;
    }

    PNLTransform            placement;
    PNLInstance::PlacementStatus state     ( PNLInstance::PlacementStatus::Unplaced );
    if ( component->isPlaced() or component->isFixed() ) {
      state = (component->isPlaced()) ? PNLInstance::PlacementStatus::Placed
                                      : PNLInstance::PlacementStatus::Fixed;

      placement = getPNLTransform ( masterPNLDesign->getAbutmentBox()
                                    , fromDefUnits(component->placementX())
                                    , fromDefUnits(component->placementY())
                                    , fromDefOrientation ( component->placementOrient() )
                                    );
    }
    printf("component %s %s\n",componentId.c_str(), componentName.c_str() );
    PNLInstance* instance = PNLInstance::create ( parser->getPNLDesign()
                                          , masterPNLDesign
                                          , NLName(componentId));
                                          // , placement
                                          // , state
                                          // );
    assert(parser->getPNLDesign()->getInstance( NLName(componentId) ));
    instance->setPlacementStatus ( state );
    instance->setTransform ( placement );
    if ( state != PNLInstance::PlacementStatus::Unplaced ) {
      parser->mergeToFitOnPNLDesignsDieArea ( instance->getModel()->getAbutmentBox() );
    }

  //cerr << "Create " << componentId << " of " << masterPNLDesign
  //      << " ab:" << masterPNLDesign->getAbutmentBox() << " @" << placement << endl;

    return 0;
  }


  int  DefParser::_componentEndCbk ( defrCallbackType_e c, void*, lefiUserData ud )
  {
    DefParser* parser = (DefParser*)ud;
    return parser->flushErrors ();
  }


  int  DefParser::_netCbk ( defrCallbackType_e c, defiNet* net, lefiUserData ud )
  {
    printf("_netCbk\n");
    static size_t netCount = 0;

    DefParser* parser = (DefParser*)ud;

  //cerr << "     - PNLNet " << net->name() << endl;

    string name = net->name();
    parser->toHurricaneName( name );
    
    PNLNetDatas* netDatas = parser->lookupPNLNet( name );
    PNLNet*      hnet     = NULL;
    if (not netDatas) {
      hnet = parser->getPNLDesign()->addNet(NLName(name));
      parser->addPNLNetLookup( name, hnet );
    } else
      hnet = get<0>( *netDatas );

    // if (parser->getPrebuildPNLNet(false)) {
    //   NLName prebuildAlias = parser->getPrebuildPNLNet()->getName();
    //   hnet->merge( parser->getPrebuildPNLNet() );
    //   hnet->removeAlias( prebuildAlias );
    //   parser->setPrebuildPNLNet( NULL );
    // }

    if (name.size() > 78) {
      name.erase ( 0, name.size()-75 );
      name.insert( 0, 3, '.' );
    }
    name.insert( 0, "\"" );
    name.insert( name.size(), "\"" );
    if (name.size() < 80) name.insert( name.size(), 80-name.size(), ' ' );

    // if (tty::enabled()) {
    //   cmess2 << "     <net:"
    //          << tty::bold  << setw(7)  << setfill('0') << ++netCount << "> " << setfill(' ')
    //          << tty::reset << setw(80) << name << tty::cr;
    //   cmess2.flush ();
    // }

    int numConnections = net->numConnections();
    for ( int icon=0 ; icon<numConnections ; ++icon ) {
      string instanceName = net->instance(icon);
      string pinName      = net->pin(icon);

    // Connect to an external pin.
      if (instanceName.compare("PIN") == 0) continue;
      parser->toHurricaneName( pinName );

      PNLInstance* instance = parser->getPNLDesign()->getInstance( NLName(instanceName) );
      if ( instance == NULL ) {
        ostringstream message;
        message << "Unknown instance (DEF COMPONENT) <" << instanceName << "> in <%s>.";
        parser->pushError( message.str() );
        continue;
      }

      /*PNLNet* masterPNLNet = instance->getModel()->getNet( NLName(pinName) );
      if (not masterPNLNet) {
        ostringstream message;
        message << "Unknown PIN <" << pinName << "> in instance <"
                << instanceName << "> (LEF MACRO) in <%s>.";
        parser->pushError( message.str() );
        continue;
      }*/
      printf("set %s(%s) to net %s\n",pinName.c_str(), instance->getName().getString().c_str(), hnet->getName().getString().c_str());
      assert(instance->getInstTerm( NLName(pinName) ) != NULL);
      instance->getInstTerm( NLName(pinName) )->setNet( hnet );
    }

    return 0;
  }


  int  DefParser::_snetCbk ( defrCallbackType_e c, defiNet* net, lefiUserData ud )
  {
    printf("_snetCbk\n");
    static size_t netCount = 0;

    DefParser* parser = (DefParser*)ud;
  //cerr << "     - Special PNLNet " << net->name() << endl;
    
    string name = net->name();
    parser->toHurricaneName( name );
    
    PNLNetDatas* netDatas = parser->lookupPNLNet( name );
    PNLNet*      hnet     = NULL;
    if (not netDatas) {
      hnet = parser->getPNLDesign()->addNet( NLName(name) );
      parser->addPNLNetLookup( name, hnet );
    } else
      hnet = get<0>( *netDatas );

    // if (parser->getPrebuildPNLNet(false)) {
    //   const NLName & prebuildAlias = parser->getPrebuildPNLNet()->getName();
    //   hnet->merge( parser->getPrebuildPNLNet() );
    //   hnet->removeAlias( prebuildAlias );
    //   parser->setPrebuildPNLNet( NULL );
    // }

    if (name.size() > 78) {
      name.erase ( 0, name.size()-75 );
      name.insert( 0, 3, '.' );
    }
    name.insert( 0, "\"" );
    name.insert( name.size(), "\"" );
    if (name.size() < 80) name.insert( name.size(), 80-name.size(), ' ' );

    // if (tty::enabled()) {
    //   cmess2 << "     <net:"
    //          << tty::bold  << setw(7)  << setfill('0') << ++netCount << "> " << setfill(' ')
    //          << tty::reset << setw(80) << name << tty::cr;
    //   cmess2.flush ();
    // }

    int numConnections = net->numConnections();
    for ( int icon=0 ; icon<numConnections ; ++icon ) {
      string instanceName = net->instance(icon);
      string pinName      = net->pin(icon);

    // Connect to an external pin.
      if (instanceName.compare("PIN") == 0) continue;
      parser->toHurricaneName( pinName );

      PNLInstance* instance = parser->getPNLDesign()->getInstance( NLName(instanceName) );
      if ( instance == NULL ) {
        ostringstream message;
        message << "Unknown instance (DEF COMPONENT) <" << instanceName << "> in <%s>.";
        parser->pushError( message.str() );
        continue;
      }

      /*PNLNet* masterPNLNet = instance->getModel()->getNet( NLName(pinName) );
      if (not masterPNLNet) {
        ostringstream message;
        message << "Unknown PIN <" << pinName << "> in instance <"
                << instanceName << "> (LEF MACRO) in <%s>.";
        parser->pushError( message.str() );
        continue;
      }*/

      instance->getInstTerm( NLName(pinName) )->setNet( hnet );
    }

    return 0;
  }


  int  DefParser::_netEndCbk ( defrCallbackType_e c, void*, lefiUserData ud )
  {
    printf("end net cb\n");
    DefParser* parser = (DefParser*)ud;
    //if (tty::enabled()) cmess2 << endl;
    return parser->flushErrors ();
  }


  int  DefParser::_pathCbk ( defrCallbackType_e c, defiPath* path, lefiUserData ud )
  {
    // DefParser*  parser       = (DefParser*)ud;
    // //Technology* technology   = DataBase::getDB()->getTechnology();
    // PNLNet*        hnet         = parser->getPrebuildPNLNet();

    // Contact*     source   = NULL;
    // Contact*     target   = NULL;
    // // const Layer* layer    = NULL;
    // // const Layer* viaLayer = NULL;
    // PNLBox::Unit    width    = PNLUnit::lambda(2.0);
    // PNLBox::Unit    x, y;
    // int          defx, defy, defext;
    // int          elementType;

    // path->initTraverse ();
    // while ( (elementType = path->next()) != DEFIPATH_DONE ) {
    //   bool createSegment = false;
    //   bool createVia     = false;
    //   bool createViaInst = false;

    //   switch ( elementType ) {
    //     case DEFIPATH_LAYER:
    //       layer = parser->lookupLayer( path->getLayer() );
    //       break;
    //     case DEFIPATH_WIDTH:
    //       width = fromDefUnits( path->getWidth() );
    //       break;
    //     case DEFIPATH_POINT:
    //       path->getPoint( &defx, &defy );
    //       x = fromDefUnits( defx );
    //       y = fromDefUnits( defy );
    //       createSegment = true;
    //       break;
    //     case DEFIPATH_FLUSHPOINT:
    //       path->getFlushPoint( &defx, &defy, &defext );
    //       x = fromDefUnits( defx );
    //       y = fromDefUnits( defy );
    //       target = NULL;
    //       createSegment = true;
    //       break;
    //     case DEFIPATH_VIA:
    //       viaLayer = technology->getLayer( path->getVia() );
    //       if (not viaLayer) {
    //         createViaInst = parser->lookupVia( path->getVia() );
    //       } else {
    //         createVia = true;
    //       }
    //       break;
    //   }

    //   if (createSegment) {
    //     source = target;
    //     target = Contact::create( hnet, layer, x, y );
    //     if (source) {
    //       if (source->getX() == x) {
    //         Vertical::create( source, target, layer, x, width );
    //       } else if (source->getY() == y) {
    //         Horizontal::create( source, target, layer, y, width );
    //       } else {
    //         ostringstream message;
    //         message << "Non-manhattan segment in net <" << hnet->getName() << ">.";
    //         parser->pushError ( message.str() );
    //       }
    //     }
    //   }
      
    //   if (createVia) {
    //     if (target) {
    //       target = Contact::create( target, viaLayer, 0, 0 );
    //     } else {
    //       target = Contact::create( hnet, viaLayer, x, y, 0, 0 );
    //     }
    //   }

    //   if (createViaInst) {
    //     target = parser->createVia( path->getVia(), hnet, x, y );
    //   }
    // }

    return 0;
  }


  PNLDesign* DefParser::parse ( string file, unsigned int flags, naja::NL::NLDB* db )
  {
    cmess1 << "  o  DEF: <" << file << ">" << endl;

    size_t iext = file.rfind( '.' );
    if ( file.compare(iext,4,".def") != 0 )
    {
      assert(false);
      //throw Error ("DefImport::load(): DEF files must have  \".def\" extension <%s>.",file.c_str());
    }

    //_framework  = AllianceFramework::get();
    //_technology = DataBase::getDB()->getTechnology();

    size_t istart = 0;
    size_t length = file.size() - 4;
    size_t islash = file.rfind ( '/' );
    if (islash != string::npos) {
      istart = islash + 1;
      length = file.size() - istart - 4;
    }
    string                designName = file.substr( istart, length );
    //AllianceNLLibrary*      library    = _framework->getAllianceNLLibrary( (unsigned int)0 );
    unique_ptr<DefParser> parser     ( new DefParser(file,/*library,*/flags, db) );

    FILE* defStream = fopen( file.c_str(), "r" );
    if (not defStream ) {
      assert(false);
      //throw Error ("DefImport::load(): Cannot open DEF file <%s>.",file.c_str());
    }

    parser->_createPNLDesign( designName.c_str() );
    defrRead( defStream, file.c_str(), (defiUserData)parser.get(), 1 );

    fclose( defStream );

    return parser->getPNLDesign();
  }


}  // Anonymous namespace.


  using std::cerr;
  using std::endl;
  using std::string;


  PNLDesign* DefImport::load ( string design, unsigned int flags, naja::NL::NLDB* db )
  {
    //UpdateSession::open ();

    PNLDesign* cell = NULL;
    if ((design.size() > 4) and (design.substr(design.size()-4) != ".def"))
      design += ".def";
    cell = DefParser::parse ( design, flags, db );

    //UpdateSession::close ();

    return cell;
  }


  void  DefImport::reset ()
  {
  // DefParser::reset();
  }


