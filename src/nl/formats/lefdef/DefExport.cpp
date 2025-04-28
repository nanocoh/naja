// SPDX-FileCopyrightText: 2025 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include  <memory>
#include  "lefwWriter.hpp"
#include  "defwWriter.hpp"
#include  "defwWriterCalls.hpp"
// #include  "hurricane/Error.h"
// #include  "hurricane/Warning.h"
// #include  "hurricane/DataBase.h"
// #include  "hurricane/Technology.h"
#include  "PNLNet.h"
//#include  "hurricane/PNLNetExternalComponents.h"
// #include  "hurricane/RoutingPad.h"
// #include  "hurricane/Horizontal.h"
// #include  "hurricane/Vertical.h"
#include  "PNLDesign.h"
#include  "NLLibrary.h"
#include "PNLOrientation.h"
// #include  "hurricane/UpdateSession.h"
// #include  "hurricane/ViaLayer.h"
// #include  "hurricane/Rectilinear.h"

// #include  "crlcore/Utilities.h"
// #include  "crlcore/ToolBox.h"
// #include  "crlcore/RoutingGauge.h"
// #include  "crlcore/RoutingLayerGauge.h"
// #include  "crlcore/AllianceFramework.h"
// #include  "crlcore/PNLDesignGauge.h"
#include  "LefExport.h"
#include  "DefExport.h"
#include "PNLTransform.h"
#include "PNLBox.h"
#include "PNLInstance.h"
// For ostringstream
#include  <sstream>
// For setw
#include  <iomanip>
#include "PNLInstTerm.h"

namespace {

  using namespace std;
  using namespace naja::NL;
  using namespace naja::NL;

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

  string toDefName ( string name )
  {
    if (name.empty()) return name;

    if (name[0] == '<')             name.erase ( 0, 1 );
    if (name[name.size()-1] == '>') name.erase ( name.size()-1 );
    for ( size_t i=0 ; i<name.size() ; ++i ) {
      switch ( name[i] ) {
        case ':':
        case '.': name[i] = '_'; break;
      }
    }
    return name;
  }


  // string  extractInstanceName ( const RoutingPad* rp )
  // {
  //   ostringstream name;

  //   Occurrence occurrence = rp->getOccurrence();

  //   name << getString(occurrence.getOwnerPNLDesign()->getName()) << '.';

  //   if (not rp->getOccurrence().getPath().getHeadPath().isEmpty())
  //     name << getString(rp->getOccurrence().getPath().getHeadPath().getName()) << ".";

  //   name << "I." << getString(rp->getOccurrence().getPath().getTailInstance()->getName());

  //   return toDefName(name.str());
  // }


#define  CHECK_STATUS_CBK(status,info)         if ((status) != 0) return driver->checkStatus(status,info);
#define  CHECK_STATUS_DRV(status,info)         if ((status) != 0) return checkStatus(status,info);
#define  RETURN_CHECK_STATUS_CBK(status,info)  return driver->checkStatus(status,info);
#define  RETURN_CHECK_STATUS_DRV(status,info)  return checkStatus(status,info);


  class DefDriver {
    public:
      static void          drive            ( PNLDesign* PNLDesign, uint32_t flags );
      static int           getUnits         ();
      static int           toDefUnits       ( PNLBox::Unit );
      static int           toDefOrient      ( PNLOrientation );
      static void          toDefCoordinates ( PNLInstance*, PNLTransform, int& statusX, int& statusY, int& statusOrient );
      static PNLBox::Unit     getSliceHeight   ();
      static PNLBox::Unit     getPitchWidth    ();
                          ~DefDriver        ();
             int           write            ();
    private:                                
                           DefDriver        ( PNLDesign*, const string& designName, FILE*, uint32_t flags );
      inline PNLDesign*         getDesign          ();
      inline const string& getDesignName    () const;
      inline uint32_t      getFlags         () const;
      inline int           getStatus        () const;
             int           checkStatus      ( int status, string info );
      //static int           writeRouting     ( PNLNet*, bool special );
    private:               
      static int           _designCbk       ( defwCallbackType_e, defiUserData );
      static int           _designEndCbk    ( defwCallbackType_e, defiUserData );
      static int           _historyCbk      ( defwCallbackType_e, defiUserData );
      static int           _versionCbk      ( defwCallbackType_e, defiUserData );
      static int           _dividerCbk      ( defwCallbackType_e, defiUserData );
      static int           _busBitCbk       ( defwCallbackType_e, defiUserData );
      static int           _unitsCbk        ( defwCallbackType_e, defiUserData );
      static int           _technologyCbk   ( defwCallbackType_e, defiUserData );
      static int           _dieAreaCbk      ( defwCallbackType_e, defiUserData );
      static int           _gPNLDesignGridCbk    ( defwCallbackType_e, defiUserData );
      static int           _rowCbk          ( defwCallbackType_e, defiUserData );
      static int           _trackCbk        ( defwCallbackType_e, defiUserData );
      static int           _viaCbk          ( defwCallbackType_e, defiUserData );
      static int           _pinCbk          ( defwCallbackType_e, defiUserData );
      static int           _pinPropCbk      ( defwCallbackType_e, defiUserData );
      static int           _componentCbk    ( defwCallbackType_e, defiUserData );
      static int           _netCbk          ( defwCallbackType_e, defiUserData );
      static int           _snetCbk         ( defwCallbackType_e, defiUserData );
      static int           _extensionCbk    ( defwCallbackType_e, defiUserData );
      static int           _groupCbk        ( defwCallbackType_e, defiUserData );
      static int           _propDefCbk      ( defwCallbackType_e, defiUserData );
      static int           _regionCbk       ( defwCallbackType_e, defiUserData );
      static int           _scanchainCbk    ( defwCallbackType_e, defiUserData );
    private:
      static int           _units;
      static PNLBox::Unit     _sliceHeight;
      static PNLBox::Unit     _pitchWidth;
             PNLDesign*         _cell;
             string        _designName;
             FILE*         _defStream;
             uint32_t      _flags;
             int           _status;
  };


  int        DefDriver::_units       = 1000;
  PNLBox::Unit  DefDriver::_sliceHeight = 0;
  PNLBox::Unit  DefDriver::_pitchWidth  = 0;


         int           DefDriver::getUnits       () { return _units; }
         int           DefDriver::toDefUnits     ( PNLBox::Unit u ) { return u;/*(int)(round(PNLUnit::toMicrons(u)*_units));*/ }
         PNLBox::Unit     DefDriver::getSliceHeight () { return _sliceHeight; }
         PNLBox::Unit     DefDriver::getPitchWidth  () { return _pitchWidth; }; 
  inline PNLDesign*         DefDriver::getDesign        () { return _cell; }
  inline uint32_t      DefDriver::getFlags       () const { return _flags; }
  inline int           DefDriver::getStatus      () const { return _status; }
  inline const string& DefDriver::getDesignName  () const { return _designName; }


  int  DefDriver::toDefOrient ( PNLOrientation orient )
  {
    switch ( orient.getType().getType() ) {
      case PNLOrientation::Type::R0: return 0; // N.
      case PNLOrientation::Type::R90: return 1; // W.
      case PNLOrientation::Type::R180: return 2; // S.
      case PNLOrientation::Type::R270: return 3; // E.
      case PNLOrientation::Type::MX: return 4; // FN.
      case PNLOrientation::Type::MXR90: return 5; // FE.
      case PNLOrientation::Type::MY: return 6; // FS.
      case PNLOrientation::Type::MYR90: return 7; // FW.
    }

    return 0; // N
  }


  void  DefDriver::toDefCoordinates ( PNLInstance* instance, PNLTransform transf, int& statusX, int& statusY, int& statusOrient )
  {
    PNLTransform inst_transf = instance->getTransform();
    transf.applyOn( inst_transf );
    statusX      = toDefUnits ( inst_transf.getOffset().getX() );
    statusY      = toDefUnits ( inst_transf.getOffset().getY() );
    statusOrient = toDefOrient( inst_transf.getOrientation() );

    switch ( inst_transf.getOrientation().getType().getType() ) {
      case PNLOrientation::Type::R0: break;
      case PNLOrientation::Type::R90: break;
      case PNLOrientation::Type::R180:
        statusX -= toDefUnits( instance->getDesign()->getAbutmentBox().getWidth() );
        statusY -= toDefUnits( instance->getDesign()->getAbutmentBox().getHeight() );
        break;
      case PNLOrientation::Type::R270: break;
      case PNLOrientation::Type::MX:
        statusX -= toDefUnits( instance->getDesign()->getAbutmentBox().getWidth() );
        break;
      case PNLOrientation::Type::MXR90:
        break;
      case PNLOrientation::Type::MY:
        statusY -= toDefUnits( instance->getDesign()->getAbutmentBox().getHeight() );
        break;
      case PNLOrientation::Type::MYR90:
        break;
    }

  }


  DefDriver::DefDriver ( PNLDesign* PNLDesign, const string& designName, FILE* defStream, uint32_t flags )
    : _cell      (PNLDesign)
    , _designName(designName)
    , _defStream (defStream)
    , _flags     (flags)
    , _status    (0)
  {
    //AllianceFramework* framework = AllianceFramework::get ();
    //PNLDesignGauge*         cg        = framework->getDesignGauge();

    //_sliceHeight = cg->getSliceHeight ();
    //_pitchWidth  = cg->getPitch       ();
  //_units       = PNLUnit::toGrid( PNLUnit::fromMicrons(1.0) );
    _units       = 1000;

    _status = defwInitCbk ( _defStream );
    if ( _status != 0 ) return;

    defwSetDesignCbk     ( _designCbk     );
    defwSetDesignEndCbk  ( _designEndCbk  );
    defwSetHistoryCbk    ( _historyCbk    );
    defwSetVersionCbk    ( _versionCbk    );
    defwSetDividerCbk    ( _dividerCbk    );
    defwSetBusBitCbk     ( _busBitCbk     );
    defwSetUnitsCbk      ( _unitsCbk      );
    defwSetTechnologyCbk ( _technologyCbk );
    defwSetDieAreaCbk    ( _dieAreaCbk    );
    defwSetDesignEndCbk  ( _gPNLDesignGridCbk  );
    defwSetRowCbk        ( _rowCbk        );
    defwSetTrackCbk      ( _trackCbk      );
    defwSetViaCbk        ( _viaCbk        );
    defwSetPinCbk        ( _pinCbk        );
    defwSetPinPropCbk    ( _pinPropCbk    );
    defwSetComponentCbk  ( _componentCbk  );
    defwSetNetCbk        ( _netCbk        );
    defwSetSNetCbk       ( _snetCbk       );
    defwSetExtCbk        ( _extensionCbk  );
    defwSetGroupCbk      ( _groupCbk      );
    defwSetPropDefCbk    ( _propDefCbk    );
    defwSetRegionCbk     ( _regionCbk     );
    defwSetScanchainCbk  ( _scanchainCbk  );
  }


  DefDriver::~DefDriver ()
  { }


  int  DefDriver::write ()
  {
    //_cell->flattenPNLNets( PNLDesign::Flags::NoFlags );
    return checkStatus( defwWrite(_defStream,_designName.c_str(), (void*)this )
                      , "write(): Problem while writing DEF." );
  }


  int  DefDriver::checkStatus ( int status, string info )
  {
    if ((_status=status) != 0) {
      defwPrintError( _status );
      ostringstream message;
      message << "DefDriver::checkStatus(): Error occured while driving \"" << _designName << "\".\n";
      message << "        " << info;
      cerr << message.str() << endl;
    }
    return _status;
  }


  int  DefDriver::_versionCbk ( defwCallbackType_e, defiUserData udata )
  {
    DefDriver* driver = (DefDriver*)udata;

    defwNewLine ();
    defwAddComment ( "DEF generated by najaeda." );
    defwNewLine ();

    return driver->checkStatus( defwVersion(5,7), "_versionCnk(): Failed to write VERSION" );
  }


  int  DefDriver::_designCbk ( defwCallbackType_e, defiUserData udata )
  {
    DefDriver* driver = (DefDriver*)udata;
    defwNewLine ();
    return driver->checkStatus( defwDesignName(driver->getDesignName().c_str())
                              , "_designCbk(): Failed to write DESIGN" );
  }


  int  DefDriver::_designEndCbk ( defwCallbackType_e, defiUserData udata )
  {
    DefDriver* driver = (DefDriver*)udata;
    return driver->checkStatus( defwEnd(), "_designEndCbk(): Failed to END design" );
  }


  int  DefDriver::_historyCbk ( defwCallbackType_e, defiUserData udata )
  {
  //DefDriver* driver = (DefDriver*)udata;
    return 0;
  }


  int  DefDriver::_dividerCbk ( defwCallbackType_e, defiUserData udata )
  {
    DefDriver* driver = (DefDriver*)udata;
    defwNewLine ();
    return driver->checkStatus( defwDividerChar("."), "_dividerCbk(): Failed to drive DIVIDER" );
  }


  int  DefDriver::_busBitCbk ( defwCallbackType_e, defiUserData udata )
  {
    DefDriver* driver = (DefDriver*)udata;
    return driver->checkStatus( defwBusBitChars("()")
                              , "_busBitCbk(): Failed to drive BUSBITCHAR" );
  }


  int  DefDriver::_unitsCbk ( defwCallbackType_e, defiUserData udata )
  {
    DefDriver* driver = (DefDriver*)udata;
    defwNewLine ();
    ostringstream info;
    info << "_unitsCnk(): Failed to drive UNITS (" << DefDriver::getUnits() << ")";
    return driver->checkStatus( defwUnits(DefDriver::getUnits()), info.str() );
  }


  int  DefDriver::_technologyCbk ( defwCallbackType_e, defiUserData udata )
  {
    DefDriver* driver = (DefDriver*)udata;
    return driver->checkStatus( defwTechnology( driver->getDesign()->getLibrary()->getName().getString().c_str() )
                              , "_technologycbk(): Failed to drive TECHNOLOGY" );
  }


  int  DefDriver::_dieAreaCbk ( defwCallbackType_e, defiUserData udata )
  {
    DefDriver* driver = (DefDriver*)udata;

    PNLBox abutmentBox ( driver->getDesign()->getAbutmentBox() );
    if ( driver->getFlags() & DefExport::ExpandDieArea ) {
      abutmentBox.increase ( DefDriver::getPitchWidth(), DefDriver::getPitchWidth() );
    }

    if ( not abutmentBox.isEmpty()) {
      defwNewLine ();
      driver->checkStatus
        ( defwDieArea ( (int)( toDefUnits(abutmentBox.getLeft()) )
                      , (int)( toDefUnits(abutmentBox.getBottom()) )
                      , (int)( toDefUnits(abutmentBox.getRight()) )
                      , (int)( toDefUnits(abutmentBox.getTop()) )
                      )
        , "_dieAreaCbk(): Failed to write DIEAERA"
        );
    }
    return driver->getStatus();
  }


  int  DefDriver::_gPNLDesignGridCbk ( defwCallbackType_e, defiUserData udata )
  {
  //DefDriver* driver = (DefDriver*)udata;

    return 0;
  }


  int  DefDriver::_rowCbk ( defwCallbackType_e, defiUserData udata )
  {
    DefDriver* driver      = (DefDriver*)udata;
    int        status      = 0;
    PNLDesign*      PNLDesign        = driver->getDesign();
    PNLBox        abutmentBox ( PNLDesign->getAbutmentBox() );

    if (abutmentBox.isEmpty()) return 0;

    int   origY     = (int)( toDefUnits(abutmentBox.getBottom()) );
    int   origX     = (int)( toDefUnits(abutmentBox.getLeft()) );
    int   stepY     = (int)( toDefUnits(DefDriver::getSliceHeight()) );
    int   stepX     = (int)( toDefUnits(DefDriver::getPitchWidth ()) );
    int   rowsNb    = abutmentBox.getHeight() / DefDriver::getSliceHeight();
    int   columnsNb = abutmentBox.getWidth () / DefDriver::getPitchWidth ();

    ostringstream comment;
    comment << rowsNb << " rows of " << columnsNb << " pitchs.";
    defwNewLine ();
    defwAddComment ( comment.str().c_str() );

    for ( int row=0 ; row < rowsNb ; ++row ) {
      ostringstream rowId;
      rowId << "row_" << setw(5) << setfill('0') << row;

      status = driver->checkStatus
        ( defwRowStr ( rowId.str().c_str()
                     , "core"
                     , origX
                     , origY
                     , (row%2) ? "FN" : "N"
                     , columnsNb
                     , 1
                     , stepX
                     , stepY
                     )
        , "_rowCbk(): Failed to write ROW"
        );

      if ( status != 0 ) break;

      origY += stepY;
    }

    return 0;
  }


  int  DefDriver::_trackCbk ( defwCallbackType_e, defiUserData udata )
  {
    /*DefDriver* driver      = (DefDriver*)udata;
    PNLDesign*      PNLDesign        = driver->getDesign();
    PNLBox        abutmentBox ( PNLDesign->getAbutmentBox() );

    if (abutmentBox.isEmpty()) return 0;

    //const vector<RoutingLayerGauge*>& rg
     // = AllianceFramework::get()->getRoutingGauge()->getLayerGauges();

    int status = 0;

    const char* layerName[1];
    for ( size_t ilayer=0 ; ilayer<rg.size() ; ++ilayer ) {
      string name = getString ( rg[ilayer]->getLayer()->getName() );
      layerName[0] = name.c_str();

      const char* master;  // i.e. direction.
      int         doCount;
      int         doStart;
      int         doStep = toDefUnits ( rg[ilayer]->getPitch() );

      if ( rg[ilayer]->getDirection() == Constant::Horizontal ) {
        master  = "X";
        doStart = toDefUnits ( abutmentBox.getBottom() + rg[ilayer]->getOffset() );
        doCount = toDefUnits ( abutmentBox.getHeight() ) / doStep;
      } else {
        master  = "Y";
        doStart = toDefUnits ( abutmentBox.getLeft() + rg[ilayer]->getOffset() );
        doCount = toDefUnits ( abutmentBox.getWidth() ) / doStep;
      }
      
      status = defwTracks ( master, doStart, doCount, doStep, 1, layerName );
      CHECK_STATUS_CBK(status,"_trackCbk(): Direction neither vertical nor horizontal.");
    }

    RETURN_CHECK_STATUS_CBK(status,"_trackCbk(): Did not complete properly");*/
    return 0;
  }


  int  DefDriver::_viaCbk ( defwCallbackType_e, defiUserData udata )
  {
  //DefDriver* driver = (DefDriver*)udata;
    return 0;
  }


  int  DefDriver::_pinCbk ( defwCallbackType_e, defiUserData udata )
  {
    DefDriver* driver      = (DefDriver*)udata;
    int        status      = 0;
    PNLDesign*      PNLDesign        = driver->getDesign();
    int        pinsNb      = 0;

    for (PNLNet* iPNLNet : PNLDesign->getNets() ) {
      if ( iPNLNet->isExternal() ) ++pinsNb;
    }

    status = defwStartPins ( pinsNb );
    if ( status != 0 )
      return driver->checkStatus( status, "_pinCbk(): Failed to start PINS" );

    for (PNLNet* iPNLNet : PNLDesign->getNets() ) {
      if ( not iPNLNet->isExternal() ) continue;

      const char* PNLNetUse = NULL;
      if ( iPNLNet->isGND() ) PNLNetUse = "GROUND";
      if ( iPNLNet->isVDD () ) PNLNetUse = "POWER";
      if ( iPNLNet->isClock () ) PNLNetUse = "CLOCK";

      status = defwPin ( iPNLNet->getName().getString().c_str() // pin name.
                       , iPNLNet->getName().getString().c_str() // PNLNet name (same).
                       , (PNLNetUse != NULL) ? 1 : 0              // special.
                       , (PNLNetUse != NULL) ? "INPUT" : "INOUT"  // direction.
                       , PNLNetUse                                // use.
                       , NULL                                  // placement status.
                       , 0                                     // status X.
                       , 0                                     // status Y.
                       , -1                                    // orient.
                       , NULL                                  // layer.
                       , 0, 0, 0, 0                            // geometry.
                       );
      if ( status != 0 ) return driver->checkStatus(status,"_pinCbk(): Failed to write PIN");
    }

    return driver->checkStatus ( defwEndPins(), "_pinCbk(): Failed to close PINS" );
  }


  int  DefDriver::_pinPropCbk ( defwCallbackType_e, defiUserData udata )
  {
  //DefDriver* driver = (DefDriver*)udata;
    return 0;
  }


  int  DefDriver::_componentCbk ( defwCallbackType_e, defiUserData udata )
  {
    DefDriver* driver      = (DefDriver*)udata;
    int        status      = 0;
    PNLDesign*      cell        = driver->getDesign();

    status = defwNewLine ();
    CHECK_STATUS_CBK(status,"_componentCbk(): Did not start properly");

    size_t numInstances = 0;
    for (PNLInstance* instance : cell->getInstances()) {
      // create a DFS visit of all instances under cell
      std::stack<PNLInstance*> stack;
      stack.push(instance);
      while (!stack.empty()) {
        PNLInstance* current = stack.top();
        if (current->getModel()->isTerminalNetlist()) ++numInstances;
        stack.pop();
        for (PNLInstance* child : current->getModel()->getInstances()) {
          stack.push(child);
        }
      }
    }

    status = defwStartComponents ( numInstances );
    CHECK_STATUS_CBK(status,"_componentCbk(): Cannot create instance count");
    
    
    for (PNLInstance* instance : cell->getInstances()) {
      std::vector<PNLInstance*> path;
      path.push_back(instance);
      // create a DFS visit of all instances under cell
      std::stack<PNLInstance*> stack;
      stack.push(instance);
      while (!stack.empty()) {
        PNLInstance* current = stack.top();
        while (!path.empty() && path.back()->getModel() != current->getDesign()) {
          path.pop_back();
        }
        path.push_back(current);
        stack.pop();
        for (PNLInstance* child : current->getModel()->getInstances()) {
          stack.push(child);
        }
        std::string insname("");
        for (PNLInstance* inst : path) {
          insname = insname + inst->getName().getString() + " ";
        }
        const char* source       = NULL;
        const char* statusS      = "UNPLACED";
        int         statusX      = 0;
        int         statusY      = 0;
        int         statusOrient = 0;
        PNLBox            instanceAb     = instance->getDesign()->getAbutmentBox();
        PNLTransform instanceTransf;
        for (PNLInstance* inst : path) {
          instanceTransf = inst->getTransform().getTransform(instanceTransf);
        }
        instanceTransf.applyOn( instanceAb );
        if (instance->getPlacementStatus() == PNLInstance::PlacementStatus::Placed) statusS = "PLACED";
        if (instance->getPlacementStatus() == PNLInstance::PlacementStatus::Fixed ) statusS = "FIXED";
        if (statusS[0] != 'U') {
          toDefCoordinates( instance, instanceTransf, statusX, statusY, statusOrient );
        }

        status = defwComponent ( insname.c_str()
                              , (instance)->getModel()->getName().getString().c_str()
                              , 0             // numPNLNetNames (disabled).
                              , NULL          // PNLNetNames (disabled).
                              , NULL          // eeq (electrical equivalence).
                              , NULL          // genName.
                              , NULL          // genParameters.
                              , source        // source (who has created it).
                              , 0             // numForeigns.
                              , NULL          // foreigns.
                              , NULL          // foreignsX[].
                              , NULL          // foreignsY[].
                              , NULL          // foreignsOrient[].
                              , statusS       // status (placement status).
                              , statusX       // status X (disabled).
                              , statusY       // status Y (disabled).
                              , statusOrient  // status orientation (disabled).
                              , 0.0           // weight (disabled).
                              , NULL          // region (disabled).
                              , 0, 0, 0, 0    // region coordinates.
                              );
        if ( status != 0 ) {
          printf("Failed to write COMPONENT %s\n", insname.c_str());
          return driver->checkStatus(status,"_componentCbk(): Failed to write COMPONENT");
        }
      }
    }

    

    // for ( Occurrence occurrence : cell->getTerminalPNLNetlistInstanceOccurrences() ) {
    // //for ( PNLInstance*   instance : PNLDesign->getInstances() ) {
    //   PNLInstance*   instance     = static_cast<PNLInstance*>(occurrence.getEntity());
    //   string      insname      = toDefName(occurrence.getCompactString());
    //   const char* source       = NULL;
    //   const char* statusS      = "UNPLACED";
    //   int         statusX      = 0;
    //   int         statusY      = 0;
    //   int         statusOrient = 0;

    //   PNLBox            instanceAb     = instance->getDesign()->getAbutmentBox();
    //   PNLTransform instanceTransf = instance->getTransform();
    //   occurrence.getPath().getTransform().applyOn( instanceTransf );
    //   instanceTransf.applyOn( instanceAb );

    //   //if (CatalogExtension::isFeed(instance->getDesign())) source = "DIST";

    //   if (instance->getPlacementStatus() == PNLInstance::PlacementStatus::Placed) statusS = "PLACED";
    //   if (instance->getPlacementStatus() == PNLInstance::PlacementStatus::Fixed ) statusS = "FIXED";
    //   if (statusS[0] != 'U') {
    //     toDefCoordinates( instance, occurrence.getPath().getTransform(), statusX, statusY, statusOrient );
    //   }

    //   status = defwComponent ( insname.c_str()
    //                          , (instance)->getDesign()->getName().getString().c_str()
    //                          , 0             // numPNLNetNames (disabled).
    //                          , NULL          // PNLNetNames (disabled).
    //                          , NULL          // eeq (electrical equivalence).
    //                          , NULL          // genName.
    //                          , NULL          // genParameters.
    //                          , source        // source (who has created it).
    //                          , 0             // numForeigns.
    //                          , NULL          // foreigns.
    //                          , NULL          // foreignsX[].
    //                          , NULL          // foreignsY[].
    //                          , NULL          // foreignsOrient[].
    //                          , statusS       // status (placement status).
    //                          , statusX       // status X (disabled).
    //                          , statusY       // status Y (disabled).
    //                          , statusOrient  // status orientation (disabled).
    //                          , 0.0           // weight (disabled).
    //                          , NULL          // region (disabled).
    //                          , 0, 0, 0, 0    // region coordinates.
    //                          );
    //   if ( status != 0 ) return driver->checkStatus(status,"_componentCbk(): Failed to write COMPONENT");
    // }
    printf("End of component\n");
    auto statusW = defwEndComponents();
    printf("Status %d\n", driver->checkStatus ( statusW,"_componentCbk(): Failed to close COMPONENTS" ));
    printf("checkStatus %s\n", driver->checkStatus ( statusW,"_componentCbk(): Failed to close COMPONENTS" ) == 0 ? "true" : "false");
    return driver->checkStatus ( statusW,"_componentCbk(): Failed to close COMPONENTS" );
  }


  //int  DefDriver::writeRouting ( PNLNet* PNLNet, bool special )
  //{
    /*int status = 0;
    int i = 0;
    for ( Component *component : PNLNet->getComponents() ) {
      std::string layer = component->getLayer() ? getString(component->getLayer()->getName()) : "";
      if (layer.size() >= 4 && layer.substr(layer.size() - 4) == ".pin")
        continue;
      if (layer.size() >= 6 && layer.substr(layer.size() - 6) == ".block")
        continue;

      Segment *seg = dynamic_cast<Segment*>(component);
      if (seg) {
        status = (special ? defwSpecialNetPathStart : defwNetPathStart)((i++) ? "NEW" : "ROUTED");
        if (special) {
          status = defwSpecialNetPathLayer(layer.c_str());
          status = defwSpecialNetPathWidth(int(toDefUnits(seg->getWidth())));
        } else {
          status = defwNetPathLayer(layer.c_str(), 0, nullptr);
        }
        double x[2], y[2];
        x[0] = toDefUnits(seg->getSourceX());
        y[0] = toDefUnits(seg->getSourceY());
        x[1] = toDefUnits(seg->getTargetX());
        y[1] = toDefUnits(seg->getTargetY());
        status = (special ? defwSpecialNetPathPoint : defwNetPathPoint)(2, x, y);
      } else {
        Contact *contact = dynamic_cast<Contact*>(component);
        if (contact) {
          const ViaLayer *viaLayer = dynamic_cast<const ViaLayer*>(contact->getLayer());
          if (viaLayer) {
            status = (special ? defwSpecialNetPathStart : defwNetPathStart)((i++) ? "NEW" : "ROUTED");
            if (special)
              status = defwSpecialNetPathLayer(getString(viaLayer->getBottom()->getName()).c_str());
            else
              status = defwNetPathLayer(getString(viaLayer->getBottom()->getName()).c_str(), 0, nullptr);
            double x[1], y[1];
            x[0] = toDefUnits(contact->getX());
            y[0] = toDefUnits(contact->getY());
            status = (special ? defwSpecialNetPathPoint : defwNetPathPoint)(1, x, y);
            status = (special ? defwSpecialNetPathVia : defwNetPathVia)(getString(viaLayer->getName()).c_str());
          }
        } else {
          Rectilinear *rl = dynamic_cast<Rectilinear*>(component);
          if (rl) {
            PNLBox box = rl->getBoundingBox();
            status = (special ? defwSpecialNetPathStart : defwNetPathStart)((i++) ? "NEW" : "ROUTED");
            if (special)
              status = defwSpecialNetPathLayer(layer.c_str());
            else
              status = defwNetPathLayer(layer.c_str(), 0, nullptr);
            double x[1], y[1];
            x[0] = toDefUnits(box.getLeft());
            y[0] = toDefUnits(box.getBottom());
            status = (special ? defwSpecialNetPathPoint : defwNetPathPoint)(1, x, y);
            defwNetPathRect(0, 0, toDefUnits(box.getWidth()), toDefUnits(box.getHeight()));
          }
        }
      }
    }
    if (i > 0)
      status = (special ? defwSpecialNetPathEnd : defwNetPathEnd)();
    return status;*/
    //return 0;
  //}


  int  DefDriver::_netCbk ( defwCallbackType_e, defiUserData udata )
  {
    printf("Net callback\n");
    DefDriver* driver      = (DefDriver*)udata;
    int        status      = 0;
    PNLDesign*      PNLDesign        = driver->getDesign();
    int        PNLNetsNb      = 0;
    //printf("num of nets %lu\n", PNLDesign->getNets().size());
    for ( PNLNet* PNLNet : PNLDesign->getNets() ) {
      
      //printf("PNLNet %s\n", PNLNet->getName().getString().c_str());
      if ( PNLNet->isSupply() or PNLNet->isClock() ) continue;
      ++PNLNetsNb;
    }

    status = defwStartNets ( PNLNetsNb );
    if ( status != 0 )
      return driver->checkStatus(status,"_netCbk(): Failed to begin PNLNetS");

    for ( PNLNet* PNLNet : PNLDesign->getNets() ) {
      if ( PNLNet->isSupply() or PNLNet->isClock() ) continue;

      string PNLNetName = PNLNet->getName().getString();
      if ( driver->getFlags() & DefExport::ProtectNetNames) {
        size_t pos = string::npos;
        if (PNLNetName[PNLNetName.size()-1] == ')') pos = PNLNetName.rfind('(');
        if (pos == string::npos)              pos = PNLNetName.size();
        PNLNetName.insert( pos, "_PNLNet" );
      }
      PNLNetName = toDefName( PNLNetName );

      status = defwNet ( PNLNetName.c_str() );
      if ( status != 0 ) return driver->checkStatus(status,"_netCbk(): Failed to begin PNLNet");

      // for ( RoutingPad* rp : PNLNet->getRoutingPads() ) {
      //   Plug *plug = dynamic_cast<Plug*>(rp->getPlugOccurrence().getEntity());
      //   if (plug) {
      //     status = defwNetConnection ( extractInstanceName(rp).c_str()
      //                                , getString(plug->getMasterPNLNet()->getName()).c_str()
      //                                , 0
      //                                );
      //   } else {
      //     Pin *pin = dynamic_cast<Pin*>(rp->getPlugOccurrence().getEntity());
      //     if (!pin)
      //       throw Error("RP PlugOccurrence neither a plug nor a pin!");
      //     // TODO: do we need to write something ?
      //   }
      //   if ( status != 0 ) return driver->checkStatus(status,"_netCbk(): Failed to write RoutingPad");
      // }
      PNLBitNet* bitNet = dynamic_cast<PNLBitNet*>(PNLNet);
      assert(bitNet != NULL);
      for ( PNLInstTerm* term : bitNet->getInstTerms() ) {
        PNLInstance* instance = term->getInstance();
        status = defwNetConnection ( instance->getName().getString().c_str()
                                      , term->getName().getString().c_str()
                                      , 0
                                      );
        if ( status != 0 ) return driver->checkStatus(status,"_netCbk(): Failed to write RoutingPad");
      }

      //status = writeRouting(PNLNet, false);
      status = defwNetEndOneNet ();
      if ( status != 0 ) return driver->checkStatus( status, "_neCbk(): Failed to end PNLNet" );
    }

    return driver->checkStatus ( defwEndNets(), "_neCbk(): Failed to end PNLNetS" );
  }


  int  DefDriver::_snetCbk ( defwCallbackType_e, defiUserData udata )
  {
    DefDriver* driver      = (DefDriver*)udata;
    int        status      = 0;
    PNLDesign*      PNLDesign        = driver->getDesign();
    int        PNLNetsNb      = 0;

    for (PNLNet* iPNLNet : PNLDesign->getNets() ) {
      if ( iPNLNet->isSupply() or iPNLNet->isClock() ) {
        ++PNLNetsNb;
      }
    }

    status = defwStartSpecialNets ( PNLNetsNb );
    if ( status != 0 ) return driver->checkStatus(status,"_snetCbk(): Failed to begin SPNLNetS");

    for (PNLNet* iPNLNet : PNLDesign->getNets() ) {
      const char* PNLNetUse = NULL;
      if ( iPNLNet->isGND() ) PNLNetUse = "GROUND";
      if ( iPNLNet->isVDD () ) PNLNetUse = "POWER";
      if ( iPNLNet->isClock () ) PNLNetUse = "CLOCK";
      if ( PNLNetUse == NULL ) continue;

      status = defwSpecialNet ( iPNLNet->getName().getString().c_str() );
      if ( status != 0 ) return driver->checkStatus(status,"_snetCbk(): Failed to write SPNLNet");

      status = defwSpecialNetConnection ( "*"
                                        , iPNLNet->getName().getString().c_str()
                                        , 0
                                        );
      if ( status != 0 ) return driver->checkStatus(status,"_snetCbk(): Failed to write CONNEXION");

      status = defwSpecialNetUse ( PNLNetUse );
      if ( status != 0 ) return driver->checkStatus(status,"_sPNLNetCnk(): Failed to write PNLNet USE");

      //status = writeRouting(*iPNLNet, true);
      //if ( status != 0 ) return driver->checkStatus(status,"_sPNLNetCnk(): Failed to write special wiring");

      status = defwSpecialNetEndOneNet ();
      if ( status != 0 ) return driver->checkStatus(status,"_sPNLNetCnk(): Failed to end SPNLNet");
    }

    return driver->checkStatus( defwEndSpecialNets(), "_sPNLNetCnk(): Failed to end SPECIALPNLNetS" );
  }


  int  DefDriver::_extensionCbk ( defwCallbackType_e, defiUserData udata )
  {
  //DefDriver* driver = (DefDriver*)udata;
    return 0;
  }


  int  DefDriver::_groupCbk ( defwCallbackType_e, defiUserData udata )
  {
  //DefDriver* driver = (DefDriver*)udata;
    return 0;
  }


  int  DefDriver::_propDefCbk ( defwCallbackType_e, defiUserData udata )
  {
  //DefDriver* driver = (DefDriver*)udata;
    return 0;
  }


  int  DefDriver::_regionCbk ( defwCallbackType_e, defiUserData udata )
  {
  //DefDriver* driver = (DefDriver*)udata;
    return 0;
  }


  int  DefDriver::_scanchainCbk ( defwCallbackType_e, defiUserData udata )
  {
  //DefDriver* driver = (DefDriver*)udata;
    return 0;
  }


  void  DefDriver::drive ( PNLDesign* PNLDesign, uint32_t flags )
  {
    FILE* defStream = NULL;
    try {
      string designName = PNLDesign->getName().getString() + "_export";
      string path       = "./" + designName + ".def";

      cmess1 << "  o  Export DEF: <" << path << ">" << endl;

      defStream = fopen ( path.c_str(), "w" );
      if ( defStream == NULL )
        throw "DefDriver::drive(): Cannot open <%s>.",path.c_str();

      unique_ptr<DefDriver> driver ( new DefDriver(PNLDesign,designName,defStream,flags) );
      driver->write ();
    }
    catch ( ... ) {
      if ( defStream != NULL ) fclose ( defStream );

      throw;
    }
    fclose ( defStream );
  }


} // End of anonymous namespace.

//namespace CRL {

  using std::cerr;
  using std::endl;
  using std::string;
  using naja::NL::NLLibrary;
  using naja::NL::PNLTransform;
  //using naja::UpdateSession;


  void  DefExport::drive ( PNLDesign* PNLDesign, uint32_t flags )
  {
    DefDriver::drive ( PNLDesign, flags );

    if ( flags & WithLEF ) LefExport::drive ( PNLDesign, LefExport::WithTechnology|LefExport::WithSpacers );
  }


//}  // End of CRL namespace.
