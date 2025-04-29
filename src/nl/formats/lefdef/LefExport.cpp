// SPDX-FileCopyrightText: 2025 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include  <memory>
#include  "lefwWriter.hpp"
#include  "lefwWriterCalls.hpp"
// #include  "hurricane/Error.h"
// #include  "hurricane/Warning.h"
// #include  "hurricane/DataBase.h"
// #include  "hurricane/RegularLayer.h"
// #include  "hurricane/Technology.h"
#include  "PNLNet.h"
// #include  "hurricane/PNLNetExternalComponents.h"
// #include  "hurricane/Horizontal.h"
// #include  "hurricane/Vertical.h"
#include  "PNLDesign.h"
#include  "NLLibrary.h"
// #include  "hurricane/UpdateSession.h"
// #include  "crlcore/Utilities.h"
// #include  "crlcore/ToolBox.h"
// #include  "crlcore/RoutingLayerGauge.h"
// #include  "crlcore/RoutingGauge.h"
// #include  "crlcore/PNLDesignGauge.h"
// #include  "crlcore/Catalog.h"
// #include  "crlcore/AllianceFramework.h"
#include  "LefExport.h"
#include <set>
#include "PNLBox.h"
// for ostringstream
#include <sstream>
#include "PNLUnit.h"
#include "PNLTechnology.h"
#include "PNLSite.h"

using namespace std;
using namespace naja::NL;
using namespace naja::NL;

namespace {

struct Comparator {
    bool operator()(PNLDesign* a, PNLDesign* b) const {
        return a->getID() > b->getID();
    }
};

#define  CHECK_STATUS(status)         if ((status) != 0) return checkStatus(status);
#define  RETURN_CHECK_STATUS(status)  return checkStatus(status);

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

  class LefDriver {
    public:
      static void               drive                 ( const std::vector<PNLDesign*>&
                                                      , const string& libraryName
                                                      , unsigned int  flags
                                                      );
      static int                getUnits              ();
      //static double             toLefUnits            ( PNLBox::Unit );
      static PNLBox::Unit          getSliceHeight        ();
      static PNLBox::Unit          getPitchWidth         ();
                               ~LefDriver             ();
             int                write                 ();
    private:                                          
                                LefDriver             ( const std::vector<PNLDesign*>&
                                                      , const string& libraryName
                                                      , unsigned int  flags
                                                      , FILE*
                                                      );
      inline unsigned int       getFlags              () const;
      inline const std::vector<PNLDesign*>   getPNLDesigns              () const;
      inline const string&      getNLLibraryName        () const;
      //inline AllianceFramework* getFramework          ();
      inline int                getStatus             () const;
             int                checkStatus           ( int status );
    private:                                          
      static int                _versionCbk           ( lefwCallbackType_e, lefiUserData );
      static int                _busBitCharsCbk       ( lefwCallbackType_e, lefiUserData );
      static int                _clearanceMeasureCbk  ( lefwCallbackType_e, lefiUserData );
      static int                _dividerCharCbk       ( lefwCallbackType_e, lefiUserData );
      static int                _unitsCbk             ( lefwCallbackType_e, lefiUserData );
      static int                _extCbk               ( lefwCallbackType_e, lefiUserData );
      static int                _propDefCbk           ( lefwCallbackType_e, lefiUserData );
      static int                _endLibCbk            ( lefwCallbackType_e, lefiUserData );
      static int                _layerCbk             ( lefwCallbackType_e, lefiUserData );
      static int                _macroCbk             ( lefwCallbackType_e, lefiUserData );
      static int                _manufacturingGridCbk ( lefwCallbackType_e, lefiUserData );
      static int                _nonDefaultCbk        ( lefwCallbackType_e, lefiUserData );
      static int                _siteCbk              ( lefwCallbackType_e, lefiUserData );
      static int                _spacingCbk           ( lefwCallbackType_e, lefiUserData );
      static int                _useMinSpacingCbk     ( lefwCallbackType_e, lefiUserData );
      static int                _viaCbk               ( lefwCallbackType_e, lefiUserData );
      static int                _viaRuleCbk           ( lefwCallbackType_e, lefiUserData );
            //  int                _driveRoutingLayer    ( RoutingLayerGauge* );
            //  int                _driveCutLayer        ( Layer* );
             int                _driveMacro           ( PNLDesign* );
    private:
      //static AllianceFramework* _framework;
      static int                _units;
      static PNLBox::Unit          _sliceHeight;
      static PNLBox::Unit          _pitchWidth;
             unsigned int       _flags;
             const std::vector<PNLDesign*>   _cells;
             string             _libraryName;
             FILE*              _lefStream;
             int                _status;
  };


  int                LefDriver::_units       = 100;
  //AllianceFramework* LefDriver::_framework   = NULL;
  PNLBox::Unit          LefDriver::_sliceHeight = 0;
  PNLBox::Unit          LefDriver::_pitchWidth  = 0;


         int                LefDriver::getUnits       () { return _units; }
         //double             LefDriver::toLefUnits     ( PNLBox::Unit u ) { return u;/*PNLUnit::toMicrons(u)*//**getUnits()*/; }
         PNLBox::Unit          LefDriver::getSliceHeight () { return _sliceHeight; }
         PNLBox::Unit          LefDriver::getPitchWidth  () { return _pitchWidth; }; 
  //inline AllianceFramework* LefDriver::getFramework   () { return _framework; }
  inline unsigned int       LefDriver::getFlags       () const { return _flags; }
  inline const std::vector<PNLDesign*>   LefDriver::getPNLDesigns       () const { return _cells; }
  inline int                LefDriver::getStatus      () const { return _status; }
  inline const string&      LefDriver::getNLLibraryName () const { return _libraryName; }


  LefDriver::LefDriver ( const std::vector<PNLDesign*>& cells, const string& libraryName, unsigned int flags, FILE* lefStream )
    : _flags      (flags)
    , _cells      (cells)
    , _libraryName(libraryName)
    , _lefStream  (lefStream)
    , _status     (0)
  {
    // if ( _framework == NULL ) {
    //   _framework = AllianceFramework::get ();
    //   PNLDesignGauge* cg = _framework->getPNLDesignGauge();

    //   _sliceHeight = cg->getSliceHeight ();
    //   _pitchWidth  = cg->getPitch       ();
    // }

    // PNLTechnology* tech = PNLTechnology::getOrCreate();
    // PNLSite* site = tech->getSiteByClass("CORE");
    // _sliceHeight = site->getHeight();
    // _pitchWidth  = site->getWidth();
    // printf("sliceHeight %ld pitchWidth %ld\n", _sliceHeight, _pitchWidth);
    // printf("sliceHeight %f pitchWidth %f\n", PNLUnit::toMicrons(_sliceHeight), PNLUnit::toMicrons(_pitchWidth));

    // _units = PNLUnit::toGrid(PNLUnit::fromMicrons(1.0));

    _status = lefwInitCbk ( _lefStream );
    if ( _status != 0 ) return;



    lefwSetVersionCbk           ( _versionCbk           );
    lefwSetBusBitCharsCbk       ( _busBitCharsCbk       );
    lefwSetDividerCharCbk       ( _dividerCharCbk       );
    lefwSetSiteCbk              ( _siteCbk              );
    lefwSetUnitsCbk             ( _unitsCbk             );
    lefwSetManufacturingGridCbk ( _manufacturingGridCbk );
    lefwSetClearanceMeasureCbk  ( _clearanceMeasureCbk  );
    lefwSetExtCbk               ( _extCbk               );
    lefwSetLayerCbk             ( _layerCbk             );
    lefwSetMacroCbk             ( _macroCbk             );
    lefwSetPropDefCbk           ( _propDefCbk           );
    lefwSetSpacingCbk           ( _spacingCbk           );
    lefwSetUseMinSpacingCbk     ( _useMinSpacingCbk     );
    lefwSetNonDefaultCbk        ( _nonDefaultCbk        );
    lefwSetViaCbk               ( _viaCbk               );
    lefwSetViaRuleCbk           ( _viaRuleCbk           );
    lefwSetEndLibCbk            ( _endLibCbk            );
  }


  LefDriver::~LefDriver ()
  { }


  int  LefDriver::write ()
  {
    return checkStatus ( lefwWrite(_lefStream,_libraryName.c_str(),(void*)this) );
  }


  int  LefDriver::checkStatus ( int status )
  {
    if ( (_status=status) != 0 ) {
      lefwPrintError ( _status );
      assert(false);
      //cerr << Error("LefDriver::drive(): Error occured while driving <%s>.",_libraryName.c_str()) << endl;
    }
    return _status;
  }


  // int  LefDriver::_driveRoutingLayer ( RoutingLayerGauge* lg )
  // {
  //   // if ( lg == NULL ) return 0;

  //   // string layerName = getString(lg->getLayer()->getName());

  //   // _status = lefwStartLayerRouting ( layerName.c_str() );
  //   // if ( _status != 0 ) return _status;

  //   // _status = lefwLayerRouting ( (lg->getDirection() == Constant::Horizontal) ? "HORIZONTAL" : "VERTICAL"
  //   //                            , toLefUnits(lg->getWireWidth())
  //   //                            );
  //   // if ( _status != 0 ) return _status;

  //   // _status = lefwLayerRoutingOffset ( toLefUnits(lg->getOffset()) );
  //   // if ( _status != 0 ) return _status;

  //   // _status = lefwLayerRoutingPitch ( toLefUnits(lg->getPitch()) );
  //   // if ( _status != 0 ) return _status;

  //   // _status = lefwLayerRoutingSpacing ( toLefUnits(lg->getPitch()-lg->getWireWidth()/*-PNLUnit::lambda(1.0)*/) );
  //   // if ( _status != 0 ) return _status;
 
  //   // return _status = lefwEndLayerRouting ( layerName.c_str() );
  //   return 0;
  // }


  // int  LefDriver::_driveCutLayer ( Layer* layer )
  // {
  //   /*if ( layer == NULL ) return 0;

  //   _status = lefwStartLayer ( getString(layer->getName()).c_str(), "CUT" );
  //   if ( _status != 0 ) return _status;

  //   _status = lefwLayerWidth ( toLefUnits(PNLUnit::lambda(1.0)) );
  //   if ( _status != 0 ) return _status;
 
  //   return _status = lefwEndLayer ( getString(layer->getName()).c_str() );*/
  //   return 0;
  // }


  int  LefDriver::_driveMacro ( PNLDesign* cell )
  {
    printf("LefDriver::_driveMacro\n");
    _status = lefwStartMacro ( cell->getName().getString().c_str() );
    CHECK_STATUS(_status);
    printf("a\n");
    const PNLBox&         abutmentBox   ( cell->getAbutmentBox() );
    //double      pitchWidth    = toLefUnits ( LefDriver::getPitchWidth () );
    //double      sliceHeight   = toLefUnits ( LefDriver::getSliceHeight() );
    // int         slices        = (int)floor( abutmentBox.getHeight() / LefDriver::getSliceHeight() );
    // int         pitchs        = (int)floor( abutmentBox.getWidth () / LefDriver::getPitchWidth () );
    // int         slices        =  abutmentBox.getHeight() ;
    // int         pitchs        =  abutmentBox.getWidth ();
    const char* macroClass    = NULL;
    const char* macroSubClass = NULL;
    //printf("slices %d\n", slices);
    /*if ( slices > 1 ) {
      printf("case1\n");
      macroClass    = "BLOCK";
      macroSubClass = "BLACKBOX";
    } else {
      printf("case2\n");
      macroClass = "CORE";
      //if ( CatalogExtension::isFeed(cell) ) macroSubClass = "SPACER";
    }*/
    // CORE, CORE_FEEDTHRU, CORE_TIEHIGH, CORE_TIELOW, CORE_SPACER, CORE_ANTENNACELL, CORE_WELLTAP,
    //       PAD, PAD_INPUT, PAD_OUTPUT, PAD_INOUT, PAD_POWER, PAD_SPACER, PAD_AREAIO, 
    //       BLOCK, BLACKBOX, SOFT_MACRO, 
    //       ENDCAP_PRE, ENDCAP_POST, ENDCAP_TOPLEFT, ENDCAP_TOPRIGHT, ENDCAP_BOTTOMLEFT, ENDCAP_BOTTOMRIGHT, 
    //       COVER, COVER_BUMP, RING
    switch (cell->getClassType()) {
        case PNLDesign::ClassType::CORE:
            macroClass = "CORE";
            break;
        case PNLDesign::ClassType::CORE_FEEDTHRU:
            macroClass = "CORE";
            macroSubClass = "FEEDTHRU";
            break;
        case PNLDesign::ClassType::CORE_TIEHIGH:
            macroClass = "CORE";
            macroSubClass = "TIEHIGH";
            break;
        case PNLDesign::ClassType::CORE_TIELOW:
            macroClass = "CORE";
            macroSubClass = "TIELOW";
            break;
        case PNLDesign::ClassType::CORE_SPACER:
            macroClass = "CORE";
            macroSubClass = "SPACER";
            break;
        case PNLDesign::ClassType::CORE_ANTENNACELL:
            macroClass = "CORE ANTENNACELL";
            break;
        case PNLDesign::ClassType::CORE_WELLTAP:
            macroClass = "CORE";
            macroSubClass = "WELLTAP";
            break;
        case PNLDesign::ClassType::PAD:
            macroClass = "PAD";
            break;
        case PNLDesign::ClassType::PAD_INPUT:
            macroClass = "PAD";
            macroSubClass = "INPUT";
            break;
        case PNLDesign::ClassType::PAD_OUTPUT:
            macroClass = "PAD";
            macroSubClass = "OUTPUT";
            break;
        case PNLDesign::ClassType::PAD_INOUT:
            macroClass = "PAD";
            macroSubClass = "INOUT";
            break;
        case PNLDesign::ClassType::PAD_POWER:
            macroClass = "PAD";
            macroSubClass = "POWER";
            break;
        case PNLDesign::ClassType::PAD_SPACER:
            macroClass = "PAD";
            macroSubClass = "SPACER";
            break;
        case PNLDesign::ClassType::PAD_AREAIO:
            macroClass = "PAD";
            macroSubClass = "AREAIO";
            break;
        case PNLDesign::ClassType::BLOCK:
            macroClass = "BLOCK";
            break;
        case PNLDesign::ClassType::BLACKBOX:
            macroClass = "BLACKBOX";
            break;
        case PNLDesign::ClassType::SOFT_MACRO:
            macroClass = "SOFT";
            macroSubClass = "MACRO";
            break;
        case PNLDesign::ClassType::ENDCAP_PRE:
            macroClass = "ENDCAP";
            macroSubClass = "PRE";
            break;
        case PNLDesign::ClassType::ENDCAP_POST:
            macroClass = "ENDCAP";
            macroSubClass = "POST";
            break;
        case PNLDesign::ClassType::ENDCAP_TOPLEFT:
            macroClass = "ENDCAP";
            macroSubClass = "TOPLEFT";
            break;
        case PNLDesign::ClassType::ENDCAP_TOPRIGHT:
            macroClass = "ENDCAP";
            macroSubClass = "TOPRIGHT";
            break;
        case PNLDesign::ClassType::ENDCAP_BOTTOMLEFT: 
            macroClass = "ENDCAP";
            macroSubClass = "BOTTOMLEFT";
            break;
        case PNLDesign::ClassType::ENDCAP_BOTTOMRIGHT:
            macroClass = "ENDCAP";
            macroSubClass = "BOTTOMRIGHT";
            break;
        case PNLDesign::ClassType::COVER:
            macroClass = "COVER";
            break;
        case PNLDesign::ClassType::COVER_BUMP:  
            macroClass = "COVER";
            macroSubClass = "BUMP";
            break;
        case PNLDesign::ClassType::RING:
            macroClass = "RING";
            break;
        case PNLDesign::ClassType::NONE:
            break;
        default:
            assert(false);
    }
    printf("cell name %s\n", cell->getName().getString().c_str());
    printf("macro class type %s\n", cell->getClassType().getString().c_str());
    printf("%p\n",macroClass);
    printf("macro class %s\n",macroClass);
    printf("%p\n",macroSubClass);
    printf("%s\n",macroSubClass);
    printf("a1\n");

    _status = lefwMacroClass ( macroClass, macroSubClass );
    CHECK_STATUS(_status);
    printf("b\n");
    // double originX = toLefUnits ( abutmentBox.getLeft() );
    // double originY = toLefUnits ( abutmentBox.getBottom() );
    double originX = abutmentBox.getLeft();
    double originY = abutmentBox.getBottom();
    //if ( (originX != 0.0) or (originY != 0.0) )
      // cerr << Warning("PNLDesign <%s> origin is in (%s,%s), shifting to (0,0)."
      //                ,cell->getName().getString().c_str()
      //                ,std::to_string(abutmentBox.getXMin()).c_str()
      //                ,std::to_string(abutmentBox.getYMin()).c_str()
      //                ) << endl;
    _status = lefwMacroOrigin ( 0.0, 0.0 );
    CHECK_STATUS(_status);
    printf("c\n");
    // double sizeX = toLefUnits ( abutmentBox.getWidth () );
    // double sizeY = toLefUnits ( abutmentBox.getHeight() );
    double sizeX = abutmentBox.getWidth ();
    double sizeY = abutmentBox.getHeight();
    _status = lefwMacroSize ( sizeX, sizeY );
    CHECK_STATUS(_status);
    printf("d\n");
    _status = lefwMacroSymmetry ( "X Y" );
    CHECK_STATUS(_status);
    printf("e\n");
    if (cell->getSite() != NULL) {  
      _status = lefwMacroSite ( cell->getSite()->getName().getString().c_str() );
    }
    CHECK_STATUS(_status);
    printf("f\n");
    // if ( slices > 1 ) {
    //   for ( int islice=0 ; islice<slices ; ++islice ) {
    //     _status = lefwMacroSitePatternStr ( "core"                        // site name.
    //                                       , originX                       // X origin.
    //                                       , originY /*+ sliceHeight*islice*/  // Y origin.
    //                                       , (islice % 2) ? "FS" : "N"     // orientation.
    //                                       , pitchs                        // num X.
    //                                       , 0                             // num Y.
    //                                       , 0                    // space X (STEP X).
    //                                       , 0                   // space Y (STEP Y).
    //                                       );
    //     CHECK_STATUS(_status);
    //   }
    // }

    PNLNet* blockagePNLNet = NULL;

    for ( PNLNet* inet : cell->getNets() ) {
      PNLNet* net = inet;
      //if ( (blockagePNLNet == NULL) and _framework->isBLOCKAGE(net->getName()) )
      // blockagePNLNet = net;

      if ( not net->isExternal() ) continue;

      _status = lefwStartMacroPin ( net->getName().getString().c_str() );
      CHECK_STATUS(_status);

      _status = lefwMacroPinDirection ( "INPUT" );
      CHECK_STATUS(_status);

      if ( net->isSupply() ) {
        _status = lefwMacroPinShape ( "ABUTMENT" );
        CHECK_STATUS(_status);
      }

      const char* pinUse = "SIGNAL";
      if      ( net->isGND() ) pinUse = "GROUND";
      else if ( net->isVDD () ) pinUse = "POWER";
      else if ( net->isClock () ) pinUse = "CLOCK";
      _status = lefwMacroPinUse ( pinUse );
      CHECK_STATUS(_status);

      _status = lefwStartMacroPinPort ( NULL );
      CHECK_STATUS(_status);

      // const Layer* layer = NULL;
      // forEach ( Component*, icomponent, net->getComponents() ) {
      //   //if ( not PNLNetExternalComponents::isExternal(*icomponent) ) continue;

      //   // if ( layer != (*icomponent)->getLayer() ) {
      //   //   layer = (*icomponent)->getLayer();
      //   //   _status = lefwMacroPinPortLayer ( getString(layer->getName()).c_str(), 0 );
      //   //   CHECK_STATUS(_status);
      //   // }

      //   const PNLBox& bb ( (*icomponent)->getBoundingBox() );
      //   _status = lefwMacroPinPortLayerRect ( toLefUnits(bb.getXMin())-originX  // xl
      //                                       , toLefUnits(bb.getYMin())-originY  // yl
      //                                       , toLefUnits(bb.getXMax())-originX  // xh
      //                                       , toLefUnits(bb.getYMax())-originY  // yh
      //                                       , 0                                 // ITERATE numX
      //                                       , 0                                 // ITERATE numY
      //                                       , 0                                 // ITERATE spaceX
      //                                       , 0                                 // ITERATE spaceY
      //                                       );
      //   CHECK_STATUS(_status);
      // }

      _status = lefwEndMacroPinPort ();
      CHECK_STATUS(_status);

      _status = lefwEndMacroPin ( net->getName().getString().c_str() );
      CHECK_STATUS(_status);
    }

#if 0
    _status = lefwStartMacroObs ();
    CHECK_STATUS(_status);

    double supplyHalfWidth   = toLefUnits ( PNLUnit::lambda(6.0) );
    double sliceHeight       = toLefUnits ( LefDriver::getSliceHeight() );
#endif

   //double METAL1HalfMinDist = toLefUnits ( PNLUnit::lambda(1.0) );

#if 0
    _status = lefwMacroObsLayer ( "METAL1", METAL1HalfMinDist );
    CHECK_STATUS(_status);

    _status = lefwMacroObsLayerRect ( METAL1HalfMinDist                                  // xl
                                    , METAL1HalfMinDist + supplyHalfWidth                // yl
                                    , sizeX       - METAL1HalfMinDist                    // xh
                                    , sliceHeight - METAL1HalfMinDist - supplyHalfWidth  // yh
                                    , 1                                                  // ITERATE columns
                                    , slices                                             // ITERATE rows
                                    , sizeX                                              // ITERATE spaceX
                                    , toLefUnits(LefDriver::getSliceHeight())            // ITERATE spaceY
                                    );
    CHECK_STATUS(_status);
#endif

    // if ( blockagePNLNet != NULL ) {
    //   _status = lefwStartMacroObs ();
    //   CHECK_STATUS(_status);

    //   const Layer* blockageLayer = NULL;
    //   forEach ( Component*, icomponent, blockagePNLNet->getComponents() ) {
    //     if ( dynamic_cast<Segment*>(*icomponent) == NULL ) continue;

    //     if ( blockageLayer != (*icomponent)->getLayer() ) {
    //       blockageLayer = (*icomponent)->getLayer();

    //       RegularLayer* routingLayer = NULL;
    //       forEach ( RegularLayer*, ilayer, DataBase::getDB()->getTechnology()->getRegularLayers() ) {
    //         if ( (*ilayer)->getBlockageLayer() == NULL ) continue;
    //         if ( (*ilayer)->getBlockageLayer()->getMask() != blockageLayer->getMask() ) continue;

    //         routingLayer = *ilayer;
    //         break;
    //       }

    //       if ( routingLayer != NULL ) {
    //         _status = lefwMacroObsLayer ( getString(routingLayer->getName()).c_str(), METAL1HalfMinDist );
    //         CHECK_STATUS(_status);
    //       }
    //     }

    //     const PNLBox& bb ( (*icomponent)->getBoundingBox() );
    //     _status = lefwMacroObsLayerRect ( toLefUnits(bb.getXMin())-originX  // xl
    //                                     , toLefUnits(bb.getYMin())-originY  // yl
    //                                     , toLefUnits(bb.getXMax())-originX  // xh
    //                                     , toLefUnits(bb.getYMax())-originY  // yh
    //                                     , 0                                 // ITERATE numX
    //                                     , 0                                 // ITERATE numY
    //                                     , 0                                 // ITERATE spaceX
    //                                     , 0                                 // ITERATE spaceY
    //                                     );
    //     CHECK_STATUS(_status);
    //   }

    //   _status = lefwEndMacroObs ();
    //   CHECK_STATUS(_status);
    // }

#if 0
    _status = lefwEndMacroObs ();
    CHECK_STATUS(_status);
#endif

    _status = lefwEndMacro ( cell->getName().getString().c_str() );
    RETURN_CHECK_STATUS(_status);
  }


  int  LefDriver::_versionCbk ( lefwCallbackType_e, lefiUserData udata )
  {
    LefDriver* driver = (LefDriver*)udata;

    ostringstream comment;
    comment << "For design <" << driver->getNLLibraryName() << ">.";

    lefwNewLine ();
    lefwAddComment ( "LEF generated by najaeda." );
    lefwAddComment ( comment.str().c_str() );
    lefwNewLine ();

    return driver->checkStatus ( lefwVersion(5,7) );
  }


  int  LefDriver::_busBitCharsCbk ( lefwCallbackType_e, lefiUserData udata )
  {
    LefDriver* driver = (LefDriver*)udata;
    lefwNewLine ();
    return driver->checkStatus ( lefwBusBitChars("()") );
  }


  int  LefDriver::_dividerCharCbk ( lefwCallbackType_e, lefiUserData udata )
  {
    LefDriver* driver = (LefDriver*)udata;
    return driver->checkStatus ( lefwDividerChar(".") );
  }


  int  LefDriver::_unitsCbk ( lefwCallbackType_e, lefiUserData udata )
  {
    LefDriver* driver = (LefDriver*)udata;
    lefwNewLine ();
 
    int status = lefwStartUnits ();
    if ( status != 0 ) return driver->checkStatus(status);
 
    status = lefwUnits ( 0                      // time.
                       , 0                      // capacitance.
                       , 0                      // resistance.
                       , 0                      // power.
                       , 0                      // current.
                       , 0                      // voltage.
                       , LefDriver::getUnits()  // database.
                       );
    if ( status != 0 ) return driver->checkStatus(status);

    status = lefwEndUnits();

    status = lefwManufacturingGrid ( PNLTechnology::getOrCreate()->getManufacturingGrid()/*LefDriver::toLefUnits(PNLUnit::fromGrid(1.0))*/ );

    if ( status != 0 ) return driver->checkStatus(status);

    return driver->checkStatus ( status );
  }


  int  LefDriver::_layerCbk ( lefwCallbackType_e, lefiUserData udata )
  {
    /*LefDriver*  driver     = (LefDriver*)udata;
    Technology* technology = DataBase::getDB()->getTechnology();
    const vector<RoutingLayerGauge*>& rg
      = driver->getFramework()->getRoutingGauge()->getLayerGauges();

    int status = 0;
    for ( size_t ilayer=0 ; ilayer<rg.size() ; ++ilayer ) {
      if ( ilayer > 0 ) {
        status = driver->_driveCutLayer ( technology->getCutBelow(rg[ilayer]->getLayer(), false) );
        if ( status != 0 ) return driver->checkStatus(status);
      }

      status = driver->_driveRoutingLayer ( rg[ilayer] );
      if ( status != 0 ) return driver->checkStatus(status);
    }

    return status;*/
    return 0;
  }


  int LefDriver::_siteCbk(lefwCallbackType_e, lefiUserData udata) {
    printf("LefDriver::_siteCbk triggered\n");
    LefDriver* driver = (LefDriver*)udata;

    // Iterate through all sites
    for (PNLSite* site : PNLTechnology::getOrCreate()->getSites()) {
        // Debugging site details
        printf("Processing SITE: Name=%s, Class=%s, Width=%ld, Height=%ld\n",
               site->getName().getString().c_str(),
               site->getClass().c_str(),
               site->getWidth(),
               site->getHeight());

        // Call lefwSite
        std::string symmetry = "";
        switch (site->getSymmetry()) {
          case PNLSite::Symmetry::X:
            symmetry = "X";
            break;
          case PNLSite::Symmetry::Y:
            symmetry = "Y";
            break;
          case PNLSite::Symmetry::X_Y:
            symmetry = "X Y";
            break;
          case PNLSite::Symmetry::R90:
            symmetry = "R90";
            break;
          default:
            break;
        }
        int status = lefwSite(site->getName().getString().c_str(),
                              site->getClass().c_str(),
                              symmetry == "" ? nullptr : symmetry.c_str(),  // Default orientation
                              site->getWidth(),
                              site->getHeight());

        // Handle errors
        printf("Status for lefwSite: %d\n", status);
        if (status != 0) {
            printf("Error during lefwSite for %s\n", site->getName().getString().c_str());
            return driver->checkStatus(status); // Handle error status
        }

        // End each site to ensure proper state reset
        status = lefwEndSite(site->getName().getString().c_str());
        if (status != 0) {
            printf("Error during lefwEndSite for %s\n", site->getName().getString().c_str());
            return driver->checkStatus(status);
        }
    }

    // Callback completed successfully
    return 0;
}



  int  LefDriver::_extCbk ( lefwCallbackType_e, lefiUserData udata )
  {
  //LefDriver* driver = (LefDriver*)udata;
    return 0;
  }


  int  LefDriver::_propDefCbk ( lefwCallbackType_e, lefiUserData udata )
  {
  //LefDriver* driver = (LefDriver*)udata;
    return 0;
  }


  int  LefDriver::_clearanceMeasureCbk ( lefwCallbackType_e, lefiUserData udata )
  {
  //LefDriver* driver = (LefDriver*)udata;
    return 0;
  }


  int  LefDriver::_endLibCbk ( lefwCallbackType_e, lefiUserData udata )
  {
    LefDriver* driver = (LefDriver*)udata;
    return driver->checkStatus ( lefwEnd() );
  }


  int  LefDriver::_macroCbk ( lefwCallbackType_e, lefiUserData udata )
  {
    LefDriver* driver = (LefDriver*)udata;

    const std::vector<PNLDesign*>&          cells = driver->getPNLDesigns ();
    std::vector<PNLDesign*>::const_iterator icell = cells.begin();
    for ( ; (icell != cells.end()) and (driver->getStatus() == 0) ; ++icell ) {
      printf("LE: %s\n", (*icell)->getName().getString().c_str());
      driver->_driveMacro ( *icell );
    }

    return driver->getStatus();
  }


  int  LefDriver::_manufacturingGridCbk ( lefwCallbackType_e, lefiUserData udata )
  {
    return 0;
  }


  int  LefDriver::_nonDefaultCbk ( lefwCallbackType_e, lefiUserData udata )
  {
  //LefDriver* driver = (LefDriver*)udata;
    return 0;
  }


  int  LefDriver::_spacingCbk ( lefwCallbackType_e, lefiUserData udata )
  {
  //LefDriver* driver = (LefDriver*)udata;
    return 0;
  }


  int  LefDriver::_useMinSpacingCbk ( lefwCallbackType_e, lefiUserData udata )
  {
  //LefDriver* driver = (LefDriver*)udata;
    return 0;
  }


  int  LefDriver::_viaCbk ( lefwCallbackType_e, lefiUserData udata )
  {
    /*LefDriver*  driver     = (LefDriver*)udata;
    Technology* technology = DataBase::getDB()->getTechnology();
    const vector<RoutingLayerGauge*>& rg
      = driver->getFramework()->getRoutingGauge()->getLayerGauges();

    int status = 0;
    for ( size_t ilayer=1 ; ilayer<rg.size() ; ++ilayer ) {
      const Layer* topLayer    = rg[ilayer]->getLayer();
      const Layer* bottomLayer = topLayer->getMetalBelow(false); 
      const Layer* cutLayer    = topLayer->getCutBelow(false); 
      const Layer* viaLayer    = technology->getViaBetween ( topLayer, bottomLayer );

      if ( !viaLayer ) continue;

      status = lefwStartVia ( getString(viaLayer->getName()).c_str(), "DEFAULT" );
      if ( status != 0 ) return driver->checkStatus(status);

    // Bottom Layer.
      status = lefwViaLayer ( getString(bottomLayer->getName()).c_str() );
      if ( status != 0 ) return driver->checkStatus(status);

      double side = toLefUnits ( (ilayer == 1) ? PNLUnit::lambda(1.0) : PNLUnit::lambda(1.5) );
      status = lefwViaLayerRect ( -side
                                , -side
                                ,  side
                                ,  side
                                );
      if ( status != 0 ) return driver->checkStatus(status);

    // Cut Layer.
      status = lefwViaLayer ( getString(cutLayer->getName()).c_str() );
      if ( status != 0 ) return driver->checkStatus(status);

      side   = toLefUnits ( PNLUnit::lambda(0.5) );
      status = lefwViaLayerRect ( -side
                                , -side
                                ,  side
                                ,  side
                                );
      if ( status != 0 ) return driver->checkStatus(status);

    // Top Layer.
      status = lefwViaLayer ( getString(topLayer->getName()).c_str() );
      if ( status != 0 ) return driver->checkStatus(status);

      side   = toLefUnits ( PNLUnit::lambda(1.5) );
      status = lefwViaLayerRect ( -side
                                , -side
                                ,  side
                                ,  side
                                );
      if ( status != 0 ) return driver->checkStatus(status);

      status = lefwEndVia ( getString(viaLayer->getName()).c_str() );
      if ( status != 0 ) return driver->checkStatus(status);
    }

    return 0;*/
    return 0;
  }


  int  LefDriver::_viaRuleCbk ( lefwCallbackType_e, lefiUserData udata )
  {
  //LefDriver* driver = (LefDriver*)udata;
    return 0;
  }


  void  LefDriver::drive ( const std::vector<PNLDesign*>& cells, const string& libraryName, unsigned int flags )
  {
    FILE* lefStream = NULL;
    try {
      string path = "./" + libraryName + ".lef";
      cmess1 << "  o  Export LEF: <" << path << ">" << endl;

      lefStream = fopen ( path.c_str(), "w" );
      if ( lefStream == NULL ) {
        //throw Error("LefDriver::drive(): Cannot open <%s>.",path.c_str());
        assert(false);  
      }

      unique_ptr<LefDriver> driver ( new LefDriver(cells,libraryName,flags,lefStream) );
      driver->write ();
    }
    catch ( ... ) {
      if ( lefStream != NULL ) fclose ( lefStream );

      throw;
    }
    fclose ( lefStream );
  }


} // End of anonymous namespace.

  using std::string;
  using std::cerr;
  using std::endl;
  using naja::NL::NLLibrary;
  using naja::NL::PNLTransform;
  //using Hurricane::UpdateSession;


  void  LefExport::drive ( PNLDesign* cell, unsigned int flags )
  {
    string     libraryName = "symbolic";
    std::vector<PNLDesign*> cells;

    if ( cell != NULL ) {
      libraryName = cell->getName().getString() + "_export";

      // for ( Occurrence occurrence : cell->getTerminalNetlistInstanceOccurrences() ) {
      //   Instance*   instance = static_cast<Instance*>(occurrence.getEntity());
      //   cells.insert ( instance->getMasterPNLDesign() );
      //}
    }

    /*if ( flags & WithSpacers ) {
    // Ugly. Direct uses of Alliance Framework.
      PNLDesign* spacer = AllianceFramework::get()->getPNLDesign("tie_x0",Catalog::State::Views);
      if ( spacer != NULL ) cells.insert ( spacer );

      spacer = AllianceFramework::get()->getPNLDesign("rowend_x0",Catalog::State::Views);
      if ( spacer != NULL ) cells.insert ( spacer );
    }*/

    LefDriver::drive ( cells, libraryName, flags );
  }


  void  LefExport::drive ( NLLibrary* library, unsigned int flags )
  {
    string     libraryName = "symbolic";
    std::vector<PNLDesign*> cells;

    if ( library != NULL ) {
      libraryName = library->getName().getString();

      for ( PNLDesign* icell : library->getPNLDesigns() ) {
        //if ( cells.find(icell) == cells.end())
        printf("LE cell name: %s\n", icell->getName().getString().c_str());
        cells.push_back ( icell );
      }
    }
    LefDriver::drive ( cells, libraryName, flags );
  }
