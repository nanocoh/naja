// SPDX-FileCopyrightText: 2025 The Naja authors
// <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include <boost/algorithm/string.hpp>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include "lefrReader.hpp"
#include "PNLNet.h"
#include "PNLTerm.h"
#include "NLLibrary.h"
#include "PNLDesign.h"
#include "PNLPoint.h"
#include "LefImport.h"
#include <fstream>
#include "NLDB.h"
#include "NLName.h"
#include "NLUniverse.h"
#include "PNLSite.h"
#include "PNLUnit.h"

// sstream
#include <sstream>

namespace {

class mstream : public std::ostream {
 public:
  enum StreamMasks {
    PassThrough = 0x00000001,
    Verbose0 = 0x00000002,
    Verbose1 = 0x00000004,
    Verbose2 = 0x00000008,
    Info = 0x00000010,
    Paranoid = 0x00000020,
    Bug = 0x00000040
  };

 public:
  static void enable(unsigned int mask);
  static void disable(unsigned int mask);
  inline mstream(unsigned int mask, std::ostream& s);
  inline bool enabled() const;
  inline unsigned int getStreamMask() const;
  static inline unsigned int getActiveMask();
  inline void setStreamMask(unsigned int mask);
  inline void unsetStreamMask(unsigned int mask);
  // Overload for formatted outputs.
  template <typename T>
  inline mstream& operator<<(T& t);
  template <typename T>
  inline mstream& operator<<(T* t);
  template <typename T>
  inline mstream& operator<<(const T& t);
  template <typename T>
  inline mstream& operator<<(const T* t);
  inline mstream& put(char c);
  inline mstream& flush();
  // Overload for manipulators.
  inline mstream& operator<<(std::ostream& (*pf)(std::ostream&));

  // Internal: Attributes.
 private:
  static unsigned int _activeMask;
  unsigned int _streamMask;
};

inline mstream::mstream(unsigned int mask, std::ostream& s)
    : std::ostream(s.rdbuf()), _streamMask(mask) {}
inline bool mstream::enabled() const {
  return (_streamMask & _activeMask);
}
inline unsigned int mstream::getStreamMask() const {
  return _streamMask;
}
inline unsigned int mstream::getActiveMask() {
  return _activeMask;
}
inline void mstream::setStreamMask(unsigned int mask) {
  _streamMask |= mask;
}
inline void mstream::unsetStreamMask(unsigned int mask) {
  _streamMask &= ~mask;
}
inline mstream& mstream::put(char c) {
  if (enabled())
    static_cast<std::ostream*>(this)->put(c);
  return *this;
}
inline mstream& mstream::flush() {
  if (enabled())
    static_cast<std::ostream*>(this)->flush();
  return *this;
}
inline mstream& mstream::operator<<(std::ostream& (*pf)(std::ostream&)) {
  if (enabled())
    (*pf)(*this);
  return *this;
}

// For POD Types.
template <typename T>
inline mstream& mstream::operator<<(T& t) {
  if (enabled()) {
    *(static_cast<std::ostream*>(this)) << t;
  }
  return *this;
};

template <typename T>
inline mstream& mstream::operator<<(T* t) {
  if (enabled()) {
    *(static_cast<std::ostream*>(this)) << t;
  }
  return *this;
};

template <typename T>
inline mstream& mstream::operator<<(const T& t) {
  if (enabled()) {
    *(static_cast<std::ostream*>(this)) << t;
  }
  return *this;
};

template <typename T>
inline mstream& mstream::operator<<(const T* t) {
  if (enabled()) {
    *(static_cast<std::ostream*>(this)) << t;
  }
  return *this;
};

// For STL Types.
inline mstream& operator<<(mstream& o, const std::string& s) {
  if (o.enabled()) {
    static_cast<std::ostream&>(o) << s;
  }
  return o;
};

// Specific non-member operator overload. Must be one for each type.
#define MSTREAM_V_SUPPORT(Type)                          \
  inline mstream& operator<<(mstream& o, Type t) {       \
    if (o.enabled()) {                                   \
      static_cast<std::ostream&>(o) << t;                \
    }                                                    \
    return o;                                            \
  };                                                     \
                                                         \
  inline mstream& operator<<(mstream& o, const Type t) { \
    if (o.enabled()) {                                   \
      static_cast<std::ostream&>(o) << t;                \
    }                                                    \
    return o;                                            \
  };

#define MSTREAM_R_SUPPORT(Type)                           \
  inline mstream& operator<<(mstream& o, const Type& t) { \
    if (o.enabled()) {                                    \
      static_cast<std::ostream&>(o) << t;                 \
    }                                                     \
    return o;                                             \
  };                                                      \
                                                          \
  inline mstream& operator<<(mstream& o, Type& t) {       \
    if (o.enabled()) {                                    \
      static_cast<std::ostream&>(o) << t;                 \
    }                                                     \
    return o;                                             \
  };

#define MSTREAM_P_SUPPORT(Type)                           \
  inline mstream& operator<<(mstream& o, const Type* t) { \
    if (o.enabled()) {                                    \
      static_cast<std::ostream&>(o) << t;                 \
    }                                                     \
    return o;                                             \
  };                                                      \
                                                          \
  inline mstream& operator<<(mstream& o, Type* t) {       \
    if (o.enabled()) {                                    \
      static_cast<std::ostream&>(o) << t;                 \
    }                                                     \
    return o;                                             \
  };

unsigned int mstream::_activeMask = 0;
extern mstream cmess1;
mstream cmess1(mstream::Verbose0, std::cout);

using namespace std;
using namespace naja::NL;
using namespace naja::NL;

#if THIS_IS_DISABLED
void addSupplyPNLNets(PNLDesign* cell) {
  PNLNet* vss = PNLNet::create(cell, "vss");
  vss->setExternal(true);
  vss->setGlobal(true);
  vss->setType(PNLNet::Type::GROUND);

  PNLNet* vdd = PNLNet::create(cell, "vdd");
  vdd->setExternal(true);
  vdd->setGlobal(true);
  vdd->setType(PNLNet::Type::POWER);
}
#endif

class LefParser {
 public:
  static void setMergeLibrary(NLLibrary*);
  static void setGdsForeignDirectory(string);
  // static       PNLBox::Unit          fromLefUnits             ( int );
  //  static       Layer*             getLayer                 ( string );
  //  static       void               addLayer                 ( string, Layer*
  //  );
  static void reset();
  static NLLibrary* parse(string file);
  LefParser(string file, string libraryName);
  ~LefParser();
  inline bool isVH() const;
  bool isUnmatchedLayer(string);
  NLLibrary* createNLLibrary();
  PNLDesign* earlyGetPNLDesign(bool& created, string name = "");
  PNLNet* earlygetNet(string name);
  PNLTerm* earlygetTerm(string name);
  inline string getLibraryName() const;
  inline NLLibrary* getLibrary(bool create = false);
  inline string getForeignPath() const;
  inline void setForeignPath(string);
  inline const PNLPoint& getForeignPosition() const;
  inline void setForeignPosition(const PNLPoint&);
  inline PNLNet* getGdsPower() const;
  inline void setGdsPower(PNLNet*);
  inline PNLNet* getGdsGround() const;
  inline void setGdsGround(PNLNet*);
  inline PNLDesign* getPNLDesign() const;
  inline void setPNLDesign(PNLDesign*);
  // inline       PNLDesignGauge*         getPNLDesignGauge             ()
  // const; inline       void               setPNLDesignGauge             (
  // PNLDesignGauge* );
  inline PNLNet* getNet() const;
  inline void setPNLNet(PNLNet*);
  static void setCoreSite(PNLBox::Unit x, PNLBox::Unit y);
  static PNLBox::Unit getCoreSiteX();
  static PNLBox::Unit getCoreSiteY();
  inline PNLBox::Unit getMinTerminalWidth() const;
  inline double getUnitsMicrons() const;
  // inline       PNLBox::Unit          fromUnitsMicrons         ( double )
  // const;
  inline void setUnitsMicrons(double);
  inline bool hasErrors() const;
  inline const vector<string>& getErrors() const;
  inline void pushError(const string&);
  int flushErrors();
  inline void clearErrors();
  inline int getNthMetal() const;
  inline void incNthMetal();
  inline int getNthCut() const;
  inline void incNthCut();
  inline int getNthRouting() const;
  inline void incNthRouting();
  // inline       RoutingGauge*      getRoutingGauge          () const;
  inline void addPinComponent(string name, PNLTerm*);
  inline void clearPinComponents();
  naja::NL::NLDB* getDB() { return _db; }

 private:
  static int _unitsCbk(lefrCallbackType_e, lefiUnits*, lefiUserData);
  static int _layerCbk(lefrCallbackType_e, lefiLayer*, lefiUserData);
  static int _siteCbk(lefrCallbackType_e, lefiSite*, lefiUserData);
  static int _obstructionCbk(lefrCallbackType_e,
                             lefiObstruction*,
                             lefiUserData);
  static int _macroCbk(lefrCallbackType_e, lefiMacro*, lefiUserData);
  static int _macroSiteCbk(lefrCallbackType_e,
                           const lefiMacroSite*,
                           lefiUserData);
  static int _macroForeignCbk(lefrCallbackType_e,
                              const lefiMacroForeign*,
                              lefiUserData);
  static int _pinCbk(lefrCallbackType_e, lefiPin*, lefiUserData);
  void _pinStdPostProcess();
  void _pinPadPostProcess();
  static int _viaCbk(lefrCallbackType_e type, lefiVia* via, lefiUserData) {
    printf("via name %s cb\n", via->name());
    return 0;
  }
  static int _manufacturingCB(lefrCallbackType_e /* unused: c */,
                              double num,
                              lefiUserData ud) {
    // lefinReader* lef = (lefinReader*) ud;
    // lef->manufacturing(num);
    PNLTechnology::getOrCreate()->setManufacturingGrid(num);
    return 0;
  }
  static void _logFunction(const char* message);

 private:
  naja::NL::NLDB* _db = nullptr;
  static string _gdsForeignDirectory;
  static NLLibrary* _mergeNLLibrary;
  string _file;
  string _libraryName;
  NLLibrary* _library;
  string _foreignPath;
  PNLPoint _foreignPosition;
  PNLNet* _gdsPower;
  PNLNet* _gdsGround;
  PNLDesign* _cell;
  PNLNet* _net;
  string _busBits;
  double _unitsMicrons;
  PNLBox::Unit _oneGrid;
  map<string, vector<PNLTerm*> > _pinComponents;
  // static       map<string,Layer*>  _layerLut;
  vector<string> _unmatchedLayers;
  vector<string> _errors;
  int _nthMetal;
  int _nthCut;
  int _nthRouting;
  //  RoutingGauge*       _routingGauge;
  //  PNLDesignGauge*          _cellGauge;
  PNLBox::Unit _minTerminalWidth;
  static PNLBox::Unit _coreSiteX;
  static PNLBox::Unit _coreSiteY;
};

// inline       bool              LefParser::isVH                     () const {
// return _routingGauge->isVH(); }
inline PNLBox::Unit LefParser::getMinTerminalWidth() const {
  return _minTerminalWidth;
}
inline string LefParser::getLibraryName() const {
  return _libraryName;
}
inline NLLibrary* LefParser::getLibrary(bool create) {
  if (not _library and create)
    createNLLibrary();
  return _library;
}
inline PNLDesign* LefParser::getPNLDesign() const {
  return _cell;
}
inline void LefParser::setPNLDesign(PNLDesign* cell) {
  printf("setPNLDesign %p\n", (void*)cell);
  _cell = cell;
}
inline string LefParser::getForeignPath() const {
  return _foreignPath;
}
inline void LefParser::setForeignPath(string path) {
  _foreignPath = path;
}
inline const PNLPoint& LefParser::getForeignPosition() const {
  return _foreignPosition;
}
inline void LefParser::setForeignPosition(const PNLPoint& position) {
  _foreignPosition = position;
}
inline PNLNet* LefParser::getGdsPower() const {
  return _gdsPower;
}
inline void LefParser::setGdsPower(PNLNet* net) {
  _gdsPower = net;
}
inline PNLNet* LefParser::getGdsGround() const {
  return _gdsGround;
}
inline void LefParser::setGdsGround(PNLNet* net) {
  _gdsGround = net;
}
// inline       void              LefParser::setPNLDesignGauge             (
// PNLDesignGauge* gauge ) { _cellGauge=gauge; }
inline PNLNet* LefParser::getNet() const {
  return _net;
}
inline void LefParser::setPNLNet(PNLNet* net) {
  _net = net;
}
inline double LefParser::getUnitsMicrons() const {
  return _unitsMicrons;
}
inline void LefParser::setUnitsMicrons(double precision) {
  _unitsMicrons = precision;
}
inline int LefParser::getNthMetal() const {
  return _nthMetal;
}
inline void LefParser::incNthMetal() {
  ++_nthMetal;
}
inline int LefParser::getNthCut() const {
  return _nthCut;
}
inline void LefParser::incNthCut() {
  ++_nthCut;
}
inline int LefParser::getNthRouting() const {
  return _nthRouting;
}
inline void LefParser::incNthRouting() {
  ++_nthRouting;
}
// inline       RoutingGauge*     LefParser::getRoutingGauge          () const {
// return _routingGauge; } inline       PNLDesignGauge*
// LefParser::getPNLDesignGauge             () const { return _cellGauge; }
inline void LefParser::setCoreSite(PNLBox::Unit x, PNLBox::Unit y) {
  _coreSiteX = x;
  _coreSiteY = y;
}
inline PNLBox::Unit LefParser::getCoreSiteX() {
  return _coreSiteX;
}
inline PNLBox::Unit LefParser::getCoreSiteY() {
  return _coreSiteY;
}
inline bool LefParser::hasErrors() const {
  return not _errors.empty();
}
inline const vector<string>& LefParser::getErrors() const {
  return _errors;
}
inline void LefParser::pushError(const string& error) {
  _errors.push_back(error);
}
inline void LefParser::clearErrors() {
  return _errors.clear();
}
inline void LefParser::addPinComponent(string name, PNLTerm* comp) {
  _pinComponents[name].push_back(comp);
}
inline void LefParser::clearPinComponents() {
  _pinComponents.clear();
}

// inline PNLBox::Unit  LefParser::fromUnitsMicrons ( double d ) const
// {
//   PNLBox::Unit u = PNLUnit::fromPhysical(d,PNLUnit::Micro);
//   if (u % _oneGrid) {
//     // cerr << Error( "LefParser::fromUnitsMicrons(): Offgrid value %s
//     (DbU=%d), grid %s (DbU=%d)."
//     //              , PNLUnit::getValueString(u).c_str(), u
//     //              , PNLUnit::getValueString(_oneGrid).c_str(), _oneGrid )
//     //      << endl;
//     assert(false);
//   }
//   return u;
// }

string LefParser::_gdsForeignDirectory = "";
NLLibrary* LefParser::_mergeNLLibrary = nullptr;
// map<string,Layer*>  LefParser::_layerLut;
PNLBox::Unit LefParser::_coreSiteX = 0;
PNLBox::Unit LefParser::_coreSiteY = 0;

void LefParser::setMergeLibrary(NLLibrary* library) {
  _mergeNLLibrary = library;
}

void LefParser::setGdsForeignDirectory(string path) {
  _gdsForeignDirectory = path;
}

void LefParser::reset() {
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
//     cerr << Warning( "LefParser::addLayer(): Duplicated layer name \"%s\"
//     (ignored).", layerName.c_str() ); return;
//   }

//   _layerLut[ layerName ] = layer;
// }

bool LefParser::isUnmatchedLayer(string layerName) {
  for (string layer : _unmatchedLayers) {
    if (layer == layerName)
      return true;
  }
  return false;
}

LefParser::LefParser(string file, string libraryName)
    : _file(file),
      _libraryName(libraryName),
      _library(nullptr),
      _foreignPath(),
      _foreignPosition(PNLPoint(0, 0)),
      _gdsPower(nullptr),
      _gdsGround(nullptr),
      _cell(nullptr),
      _net(nullptr),
      _busBits("()"),
      _unitsMicrons(0.01)
      // , _oneGrid         (PNLUnit::fromGrid(1.0))
      ,
      _unmatchedLayers(),
      _errors(),
      _nthMetal(0),
      _nthCut(0),
      _nthRouting(0)
// , _routingGauge    (nullptr)
// , _cellGauge       (nullptr)
//,
//_minTerminalWidth(PNLUnit::fromPhysical(Cfg::getParamDouble("lefImport.minTerminalWidth",0.0)->asDouble(),PNLBox::UnitPower::Micro))
{
  // _routingGauge = AllianceFramework::get()->getRoutingGauge();
  // _cellGauge    = AllianceFramework::get()->getPNLDesignGauge();

  // if (not _routingGauge)
  //   throw Error( "LefParser::LefParser(): No default routing gauge defined in
  //   Alliance framework." );

  // if (not _cellGauge)
  //   throw Error( "LefParser::LefParser(): No default cell gauge defined in
  //   Alliance framework." );

  // string unmatcheds =
  // Cfg::getParamString("lefImport.unmatchedLayers","")->asString(); if (not
  // unmatcheds.empty()) {
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
  lefrSetUnitsCbk(_unitsCbk);
  lefrSetLayerCbk(_layerCbk);
  lefrSetSiteCbk(_siteCbk);
  lefrSetObstructionCbk(_obstructionCbk);
  lefrSetMacroCbk(_macroCbk);
  lefrSetMacroSiteCbk(_macroSiteCbk);
  lefrSetMacroForeignCbk(_macroForeignCbk);
  lefrSetPinCbk(_pinCbk);
  lefrSetViaCbk(_viaCbk);
  lefrSetManufacturingCbk(_manufacturingCB);
}

LefParser::~LefParser() {
  lefrReset();
}

NLLibrary* LefParser::createNLLibrary() {
  if (_mergeNLLibrary) {
    _library = _mergeNLLibrary;
    return _library;
  }
  _db = NLDB::create(NLUniverse::get());
  NLLibrary* rootNLLibrary = NLLibrary::create(_db, NLName("LIB1"));

  NLLibrary* lefRootNLLibrary = NLLibrary::create(rootNLLibrary, NLName("LEF"));

  _library = lefRootNLLibrary->getLibrary(NLName(_libraryName));
  if (_library) {
    assert(false);
    // Error
  } else {
    _library = NLLibrary::create(lefRootNLLibrary, NLName(_libraryName));
  }
  return _library;
}

PNLDesign* LefParser::earlyGetPNLDesign(bool& created, string name) {
  if (not _cell) {
    printf("here2\n");
    if (name.empty())
      name = "EarlyLEFPNLDesign";
    _cell = getLibrary(true)->getPNLDesign(NLName(name));
    if (not _cell) {
      created = true;
      _cell = PNLDesign::create(getLibrary(true), NLName(name));
    }
  }
  printf("name %s design name %s\n", name.c_str(),
         _cell->getName().getString().c_str());
  return _cell;
}

PNLNet* LefParser::earlygetNet(string name) {
  bool created = false;
  if (not _cell)
    earlyGetPNLDesign(created);
  PNLNet* net = _cell->getNet(NLName(name));
  if (not net)
    net = _cell->addNet(NLName(name));
  return net;
}

PNLTerm* LefParser::earlygetTerm(string name) {
  bool created = false;
  if (not _cell)
    earlyGetPNLDesign(created);
  PNLTerm* term = _cell->getTerm(NLName(name));
  if (not term)
    term = _cell->addTerm(NLName(name));
  return term;
}

void LefParser::_logFunction(const char* message) {
  std::cout << message << std::endl;
}

int LefParser::_unitsCbk(lefrCallbackType_e c,
                         lefiUnits* units,
                         lefiUserData ud) {
  LefParser* parser = (LefParser*)ud;

  if (units->hasDatabase()) {
    parser->_unitsMicrons = 1.0 / units->databaseNumber();
    cerr << "     - Precision: " << parser->_unitsMicrons
         << " (LEF MICRONS scale factor:" << units->databaseNumber() << ")"
         << endl;
  }
  return 0;
}

int LefParser::_layerCbk(lefrCallbackType_e c,
                         lefiLayer* lefLayer,
                         lefiUserData ud) {
  return 0;
}

int LefParser::_siteCbk(lefrCallbackType_e c, lefiSite* site, lefiUserData ud) {
  printf("LefParser::_siteCbk\n");
  LefParser* parser = (LefParser*)ud;
  string siteClass = "";
  if (site->hasClass()) {
    siteClass = site->siteClass();
    boost::to_upper(siteClass);
  }
  PNLBox::Unit lefSiteWidth =
        site->sizeX();  // PNLUnit::fromPhysical( site->sizeX(), PNLUnit::Micro
                        // );
  PNLBox::Unit lefSiteHeight =
        site->sizeY();  // PNLUnit::fromPhysical( site->sizeY(), PNLUnit::Micro
  auto pnlSite = PNLSite::create(NLName(site->name()), siteClass, lefSiteWidth,
                    lefSiteHeight);
  if (site->hasXSymmetry() && site->hasYSymmetry()) {
    pnlSite->setSymmetry(PNLSite::Symmetry::X_Y);
  } else if (site->hasXSymmetry()) {
    pnlSite->setSymmetry(PNLSite::Symmetry::X);
  } else if (site->hasYSymmetry()) {
    pnlSite->setSymmetry(PNLSite::Symmetry::Y);
  } else if (site->has90Symmetry()) {
    pnlSite->setSymmetry(PNLSite::Symmetry::R90);
  }
  return 0;
}

int LefParser::_macroForeignCbk(lefrCallbackType_e c,
                                const lefiMacroForeign* foreign,
                                lefiUserData ud) {
  printf("LefParser::_macroForeignCbk\n");
  printf("cellName %s\n", foreign->cellName());
  LefParser* parser = (LefParser*)ud;

  bool created = false;
  PNLDesign* cell = parser->earlyGetPNLDesign(created, foreign->cellName());
  cell->setClassType(PNLDesign::ClassType::CORE);  // TODO:: Correct?

  cell->setTerminalNetlist(true);
  if (created) {
    if (_gdsForeignDirectory.empty()) {
      // cerr << Warning( "LefParser::_macroForeignCbk(): GDS directory *not*
      // set, ignoring FOREIGN statement." ) << endl;
      return 0;
    }

    string gdsPath = _gdsForeignDirectory + "/" + foreign->cellName() + ".gds";
    parser->setForeignPath(gdsPath);

    // Gds::setTopPNLDesignName( foreign->cellName() );
    // Gds::load( parser->getLibrary(), parser->getForeignPath()
    //          , Gds::NoBlockages|Gds::Layer_0_IsBoundary);
  }

  // parser->setForeignPosition( PNLPoint( parser->fromUnitsMicrons(
  // foreign->px() )
  //                                  , parser->fromUnitsMicrons( foreign->px()
  //                                  )));
  parser->setForeignPosition(PNLPoint(foreign->px(), foreign->py()));

  for (PNLNet* net : cell->getNets()) {
    PNLBitNet* bitNet = static_cast<PNLBitNet*>(net);
    if (bitNet->isVDD())
      parser->setGdsPower(bitNet);
    if (bitNet->isGND())
      parser->setGdsGround(bitNet);
    // if (parser->getForeignPosition() != PNLPoint(0,0)) {
    //   for ( PNLNetComponent* component : bitNet->getComponents() ) {
    //     PNLTerm* term = static_cast<PNLTerm*>(component);
    //     term->translate( parser->getForeignPosition().getX()
    //                         , parser->getForeignPosition().getY() );
    //   }
    // }
  }

  return 0;
}

int LefParser::_obstructionCbk(lefrCallbackType_e c,
                               lefiObstruction* obstruction,
                               lefiUserData ud) {
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

  // //cerr << "       @ _obstructionCbk: " << blockagePNLNet->getName() <<
  // endl;

  //   lefiGeometries* geoms = obstruction->geometries();
  //   for ( int igeom=0 ; igeom < geoms->numItems() ; ++ igeom ) {
  //     if (geoms->itemType(igeom) == lefiGeomLayerE) {
  //       layer         = parser->getLayer( geoms->getLayer(igeom) );
  //       blockageLayer = layer->getBlockageLayer();
  //     }
  //     if (not blockageLayer) {
  //       cerr << Error( "DefImport::_obstructionCbk(): No blockage layer
  //       associated to \"%s\".\n"
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
  //                                     , parser->fromUnitsMicrons( (r->yl +
  //                                     r->yh)/2 ) , parser->fromUnitsMicrons(
  //                                     h  ) , parser->fromUnitsMicrons( r->xl
  //                                     ) , parser->fromUnitsMicrons( r->xh )
  //                                     );
  //       } else {
  //         segment = Vertical::create( blockagePNLNet, blockageLayer
  //                                   , parser->fromUnitsMicrons( (r->xl +
  //                                   r->xh)/2 ) , parser->fromUnitsMicrons( w
  //                                   ) , parser->fromUnitsMicrons( r->yl ) ,
  //                                   parser->fromUnitsMicrons( r->yh )
  //                                   );
  //       }
  //       cdebug_log(100,0) << "| " << segment << endl;
  //     }

  //     if (geoms->itemType(igeom) == lefiGeomPolygonE) {
  //       lefiGeomPolygon* polygon = geoms->getPolygon(igeom);
  //       vector<PNLPoint>    points;
  //       for ( int ipoint=0 ; ipoint<polygon->numPNLPoints ; ++ipoint ) {
  //         points.push_back( PNLPoint(
  //         parser->fromUnitsMicrons(polygon->x[ipoint])
  //                                ,
  //                                parser->fromUnitsMicrons(polygon->y[ipoint])
  //                                ));
  //       }
  //       points.push_back( PNLPoint( parser->fromUnitsMicrons(polygon->x[0])
  //                              , parser->fromUnitsMicrons(polygon->y[0]) ));
  //       Rectilinear::create( blockagePNLNet, blockageLayer, points );
  //       continue;
  //     }
  //   }

  return 0;
}

int LefParser::_macroCbk(lefrCallbackType_e c,
                         lefiMacro* macro,
                         lefiUserData ud) {
  printf("LefParser::_macroCbk\n");
  // AllianceFramework* af     = AllianceFramework::get();
  LefParser* parser = (LefParser*)ud;

  // parser->setPNLDesignGauge( nullptr );

  bool created = false;
  string cellName = macro->name();
  PNLBox::Unit width = 0;
  PNLBox::Unit height = 0;
  PNLDesign* cell = parser->earlyGetPNLDesign(created, cellName);

  if (cell->getName() != NLName(cellName)) {
    printf("cell name %s\n", cellName.c_str());
    cell->setName(NLName(cellName));
  }

  if (macro->hasSize()) {
    width = macro->sizeX();   // parser->fromUnitsMicrons( macro->sizeX() );
    height = macro->sizeY();  // parser->fromUnitsMicrons( macro->sizeY() );
    cell->setAbutmentBox(PNLBox(0, 0, width, height));
  }

  // Initialize cell type based on macro->macroClass with switch case
  std::string macroClass = macro->macroClass();
  assert(macro->hasClass());

  std::stringstream ss(macroClass);  // Create a stringstream object
  std::string word;
  std::vector<std::string> substrings;

  // Extract substrings separated by spaces
  while (ss >> word) {
    substrings.push_back(word);
  }

  // // Print the substrings
  // for (const auto& substring : substrings) {
  //     std::cout << substring << std::endl;
  // }

  if (substrings[0] == "CORE") {
    //          CORE, CORE_FEEDTHRU, CORE_TIEHIGH, CORE_TIELOW, CORE_SPACER,
    //          CORE_ANTENNACELL, CORE_WELLTAP,
    if (substrings.size() > 1) {
      if (substrings[1] == "FEEDTHRU") {
        cell->setClassType(PNLDesign::ClassType::CORE_FEEDTHRU);
      } else if (substrings[1] == "TIEHIGH") {
        cell->setClassType(PNLDesign::ClassType::CORE_TIEHIGH);
      } else if (substrings[1] == "TIELOW") {
        cell->setClassType(PNLDesign::ClassType::CORE_TIELOW);
      } else if (substrings[1] == "SPACER") {
        cell->setClassType(PNLDesign::ClassType::CORE_SPACER);
      } else if (substrings[1] == "ANTENNACELL") {
        cell->setClassType(PNLDesign::ClassType::CORE_ANTENNACELL);
      } else if (substrings[1] == "WELLTAP") {
        cell->setClassType(PNLDesign::ClassType::CORE_WELLTAP);
      } else {
        assert(false);
      }
    } else {
      cell->setClassType(PNLDesign::ClassType::CORE);
    }
  } else if (substrings[0] == "PAD") {
    // PAD, PAD_INPUT, PAD_OUTPUT, PAD_INOUT, PAD_POWER, PAD_SPACER, PAD_AREAIO,
    if (substrings.size() > 1) {
      if (substrings[1] == "INPUT") {
        cell->setClassType(PNLDesign::ClassType::PAD_INPUT);
      } else if (substrings[1] == "OUTPUT") {
        cell->setClassType(PNLDesign::ClassType::PAD_OUTPUT);
      } else if (substrings[1] == "INOUT") {
        cell->setClassType(PNLDesign::ClassType::PAD_INOUT);
      } else if (substrings[1] == "POWER") {
        cell->setClassType(PNLDesign::ClassType::PAD_POWER);
      } else if (substrings[1] == "SPACER") {
        cell->setClassType(PNLDesign::ClassType::PAD_SPACER);
      } else if (substrings[1] == "AREAIO") {
        cell->setClassType(PNLDesign::ClassType::PAD_AREAIO);
      } else {
        assert(false);
      }
    } else {
      cell->setClassType(PNLDesign::ClassType::PAD);
    }
  } else if (substrings[0] == "BLOCK") {
    cell->setClassType(PNLDesign::ClassType::BLOCK);
  } else if (substrings[0] == "BLACKBOX") {
    cell->setClassType(PNLDesign::ClassType::BLACKBOX);
  } else if (substrings[0] == "SOFT MACRO") {
    cell->setClassType(PNLDesign::ClassType::SOFT_MACRO);
  } else if (substrings[0] == "ENDCAP") {
    if (substrings[1] == "PRE") {
      cell->setClassType(PNLDesign::ClassType::ENDCAP_PRE);
    } else if (substrings[1] == "POST") {
      cell->setClassType(PNLDesign::ClassType::ENDCAP_POST);
    } else if (substrings[1] == "TOPRIGHT") {
      cell->setClassType(PNLDesign::ClassType::ENDCAP_TOPRIGHT);
    } else if (substrings[1] == "TOPLEFT") {
      cell->setClassType(PNLDesign::ClassType::ENDCAP_TOPLEFT);
    } else if (substrings[1] == "BOTTOMRIGHT") {
      cell->setClassType(PNLDesign::ClassType::ENDCAP_BOTTOMRIGHT);
    } else if (substrings[1] == "BOTTOMLEFT") {
      cell->setClassType(PNLDesign::ClassType::ENDCAP_BOTTOMLEFT);
    } else {
      assert(false);  // Handle unknown endcap type
    }
  } else if (substrings[0] == "COVER") {
    if (substrings.size() > 1) {
      if (substrings[1] == "BUMP") {
        cell->setClassType(PNLDesign::ClassType::COVER_BUMP);
      } else {
        assert(false);
      }
    } else {
      cell->setClassType(PNLDesign::ClassType::COVER);
    }
  } else if (substrings[0] == "RING") {
    cell->setClassType(PNLDesign::ClassType::RING);
  } else {
    assert(false);  // Handle unknown macro class
  }
  printf("name %s original type %s type %s has class %d\n",
         cell->getName().getString().c_str(), macro->macroClass(),
         cell->getClassType().getString().c_str(), macro->hasClass());

  bool isPad = false;
  string gaugeName = "Unknown SITE";
  if (macro->hasSiteName()) {
    std::string siteName = macro->siteName();
    PNLSite* site = PNLTechnology::getOrCreate()->getSiteByName(NLName(siteName));
    cell->setSite(site);
    // gaugeName = string("LEF.") + macro->siteName();
    // PNLDesignGauge* cg = af->getPNLDesignGauge( gaugeName );
    // if (cg) {
    //   isPad = cg->isPad();
    //   if (cg->getSliceHeight() != height) {
    //     cerr << Warning( "LefParser::_macroCbk(): PNLDesign height %s do not
    //     match PNLDesignGauge/SITE \"%s\" of %s."
    //                    , PNLUnit::getValueString(height).c_str()
    //                    , getString(cg->getName()).c_str()
    //                    ,
    //                    PNLUnit::getValueString(cg->getSliceHeight()).c_str()
    //                    ) << endl;
    //   }
    //   parser->setPNLDesignGauge( cg );
    // } else {
    //   cerr << Warning( "LefParser::_macroCbk(): No PNLDesignGauge associated
    //   to SITE \"%s\"."
    //                  , macro->siteName() ) << endl;
    // }
  }

  if (not isPad)
    parser->_pinStdPostProcess();
  else
    parser->_pinPadPostProcess();
  parser->clearPinComponents();

  // cerr << "     o " << cellName
  //      << " " << PNLUnit::getValueString(width) << " " <<
  //      PNLUnit::getValueString(height)
  //      << " " << gaugeName;
  if (isPad)
    cerr << " (PAD)";
  cerr << endl;

  // Catalog::State* state = af->getCatalog()->getState( cellName );
  // if (not state) state = af->getCatalog()->getState ( cellName, true );
  // state->setFlags( Catalog::State::Logical
  //                | Catalog::State::Physical
  //                | Catalog::State::InMemory
  //                | Catalog::State::Termina∂lPNLNetlist, true );
  cell->setTerminalNetlist(true);
  parser->setPNLDesign(nullptr);
  parser->setGdsPower(nullptr);
  parser->setGdsGround(nullptr);

  return 0;
}

int LefParser::_macroSiteCbk(lefrCallbackType_e c,
                             const lefiMacroSite* site,
                             lefiUserData ud) {
  printf("LefParser::_macroSiteCbk\n");
  // AllianceFramework* af     = AllianceFramework::get();
  //  LefParser*         parser = (LefParser*)ud;

  // //parser->setPNLDesignGauge( nullptr );

  // bool       created  = false;
  // string     cellName = site->siteName();
  // PNLBox::Unit  width    = 0;
  // PNLBox::Unit  height   = 0;
  // PNLDesign*      cell     = parser->earlyGetPNLDesign( created , cellName);

  // if (cell->getName() != NLName(cellName)) {
  //   cell->setName( NLName(cellName) );
  // }
  return 0;
}

int LefParser::_pinCbk(lefrCallbackType_e c, lefiPin* pin, lefiUserData ud) {
  printf("LefParser::_pinCbk %s\n", pin->name());
  LefParser* parser = (LefParser*)ud;

  // cerr << "       @ _pinCbk: " << pin->name() << endl;

  bool created = false;
  parser->earlyGetPNLDesign(created);

  PNLNet* net = nullptr;
  PNLTerm* term = nullptr;
  PNLNet::Type netType = PNLNet::Type::TypeEnum::Undefined;
  if (pin->hasUse()) {
    string lefUse = pin->use();
    boost::to_upper(lefUse);

    if (lefUse == "SIGNAL")
      netType = PNLNet::Type::TypeEnum::Logical;
    // if (lefUse == "ANALOG") netType = PNLNet::Type::ANALOG;
    if (lefUse == "CLOCK")
      netType = PNLNet::Type::TypeEnum::Clock;
    if (lefUse == "POWER")
      netType = PNLNet::Type::TypeEnum::VDD;
    if (lefUse == "GROUND")
      netType = PNLNet::Type::TypeEnum::GND;
  }

  if ((netType == PNLNet::Type::TypeEnum::VDD) and parser->getGdsPower()) {
    net = parser->getGdsPower();
    // cerr << "       - Renaming GDS power net \"" << net->getName() << "\""
    //      << " to LEF name \"" << pin->name() << "\"." << endl;
    net->setName(NLName(pin->name()));
    parser->setGdsPower(nullptr);
  } else {
    if ((netType == PNLNet::Type::TypeEnum::GND) and parser->getGdsGround()) {
      net = parser->getGdsGround();
      // cerr << "       - Renaming GDS ground net \"" << net->getName() << "\""
      //      << " to LEF name \"" << pin->name() << "\"." << endl;
      net->setName(NLName(pin->name()));
      parser->setGdsGround(nullptr);
    } else {
      net = parser->earlygetNet(pin->name());
      term = parser->earlygetTerm(pin->name());
    }
  }
  net->setExternal(true);
  net->setType(netType);

  if (pin->hasDirection()) {
    string lefDir = pin->direction();
    boost::to_upper(lefDir);

    if (lefDir == "INPUT")
      term->setDirection(PNLNetComponent::Direction::Input);
    if (lefDir == "OUTPUT")
      term->setDirection(PNLNetComponent::Direction::Output);
    if (lefDir == "OUTPUT TRISTATE")
      term->setDirection(PNLNetComponent::Direction::Tristate);
    if (lefDir == "INOUT")
      term->setDirection(PNLNetComponent::Direction::InOut);
  }
  if (net->isSupply())
    net->setGlobal(true);
  if (pin->name()[strlen(pin->name()) - 1] == '!')
    net->setGlobal(true);
  return 0;
}

void LefParser::_pinStdPostProcess() {
}

void LefParser::_pinPadPostProcess() {
}

int LefParser::flushErrors() {
  int code = (hasErrors()) ? 1 : 0;

  for (size_t ierror = 0; ierror < _errors.size(); ++ierror) {
    // string message = "LefImport::load(): " + _errors[ierror];
    // cerr << Error(message.c_str(),getString(_library->getName()).c_str()) <<
    // endl;
    assert(false);
  }
  clearErrors();

  return code;
}

NLLibrary* LefParser::parse(string file) {
  cmess1 << "  o  LEF: <" << file << ">" << endl;

  // PNLUnit::setStringMode( PNLUnit::Physical, PNLUnit::Micro );

  size_t iext = file.rfind('.');
  if (file.compare(iext, 4, ".lef") != 0) {
    // throw Error( "LefImport::load(): DEF files must have  \".lef\" extension
    // <%s>.", file.c_str() );
    assert(false);
  }

  size_t islash = file.rfind('/');
  islash = (islash == string::npos) ? 0 : islash + 1;

  // string                libraryName = file.substr( islash,
  // file.size()-4-islash );
  string libraryName = "LoadedLibrary";
  unique_ptr<LefParser> parser(new LefParser(file, libraryName));
  // std::ifstream f("file.lef");
  // if (f.is_open())
  //     std::cout << f.rdbuf();

  FILE* lefStream = fopen(file.c_str(), "r");

  if (not lefStream) {
    // throw Error( "LefImport::load(): Cannot open LEF file \"%s\".",
    // file.c_str() );
    assert(false);
  }
  parser->createNLLibrary();
  lefrRead(lefStream, file.c_str(), (lefiUserData)parser.get());
  fclose(lefStream);
  return parser->getLibrary();
}

}  // Anonymous namespace.

// namespace CRL {

using std::cerr;
using std::endl;
using std::string;
// using Hurricane::UpdateSession;

NLLibrary* LefImport::load(string fileName) {
  // UpdateSession::open ();

  NLLibrary* library = NULL;
  // #if defined(HAVE_LEFDEF)
  library = LefParser::parse(fileName);

  return library;
}

void LefImport::reset() {
  LefParser::reset();
}

void LefImport::setMergeLibrary(NLLibrary* library) {
  LefParser::setMergeLibrary(library);
}

void LefImport::setGdsForeignDirectory(string path) {
  LefParser::setGdsForeignDirectory(path);
}