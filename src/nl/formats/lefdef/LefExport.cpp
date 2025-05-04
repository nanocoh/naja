// SPDX-FileCopyrightText: 2025 The Naja authors
// <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include <memory>
#include "lefwWriter.hpp"
#include "lefwWriterCalls.hpp"
#include "PNLNet.h"
#include "NLLibrary.h"
#include "PNLDesign.h"
#include <set>
#include "LefExport.h"
#include "PNLBox.h"
// for ostringstream
#include <sstream>
#include "PNLSite.h"
#include "PNLTechnology.h"
#include "PNLUnit.h"

using namespace std;
using namespace naja::NL;
using namespace naja::NL;

namespace {

struct Comparator {
  bool operator()(PNLDesign* a, PNLDesign* b) const {
    return a->getID() > b->getID();
  }
};

#define CHECK_STATUS(status) \
  if ((status) != 0)         \
    return checkStatus(status);
#define RETURN_CHECK_STATUS(status) return checkStatus(status);

class LefDriver {
 public:
  static void drive(const std::vector<PNLDesign*>&,
                    const string& libraryName,
                    unsigned int flags);
  static int getUnits();
  // static double             toLefUnits            ( PNLBox::Unit );
  static PNLBox::Unit getSliceHeight();
  static PNLBox::Unit getPitchWidth();
  ~LefDriver();
  int write();

 private:
  LefDriver(const std::vector<PNLDesign*>&,
            const string& libraryName,
            unsigned int flags,
            FILE*);
  inline unsigned int getFlags() const;
  inline const std::vector<PNLDesign*> getPNLDesigns() const;
  inline const string& getNLLibraryName() const;
  // inline AllianceFramework* getFramework          ();
  inline int getStatus() const;
  int checkStatus(int status);

 private:
  static int versionCbk_(lefwCallbackType_e, lefiUserData);
  static int busBitCharsCbk_(lefwCallbackType_e, lefiUserData);
  static int clearanceMeasureCbk_(lefwCallbackType_e, lefiUserData);
  static int dividerCharCbk_(lefwCallbackType_e, lefiUserData);
  static int unitsCbk_(lefwCallbackType_e, lefiUserData);
  static int extCbk_(lefwCallbackType_e, lefiUserData);
  static int propDefCbk_(lefwCallbackType_e, lefiUserData);
  static int endLibCbk_(lefwCallbackType_e, lefiUserData);
  static int layerCbk_(lefwCallbackType_e, lefiUserData);
  static int macroCbk_(lefwCallbackType_e, lefiUserData);
  static int manufacturingGridCbk_(lefwCallbackType_e, lefiUserData);
  static int nonDefaultCbk_(lefwCallbackType_e, lefiUserData);
  static int siteCbk_(lefwCallbackType_e, lefiUserData);
  static int spacingCbk_(lefwCallbackType_e, lefiUserData);
  static int useMinSpacingCbk_(lefwCallbackType_e, lefiUserData);
  static int viaCbk_(lefwCallbackType_e, lefiUserData);
  static int viaRuleCbk_(lefwCallbackType_e, lefiUserData);
  //  int                _driveRoutingLayer    ( RoutingLayerGauge* );
  //  int                _driveCutLayer        ( Layer* );
  int _driveMacro(PNLDesign*);

 private:
  // static AllianceFramework* _framework;
  static int _units;
  static PNLBox::Unit _sliceHeight;
  static PNLBox::Unit _pitchWidth;
  unsigned int _flags;
  const std::vector<PNLDesign*> _cells;
  string _libraryName;
  FILE* _lefStream;
  int _status;
};

int LefDriver::_units = 100;
// AllianceFramework* LefDriver::_framework   = NULL;
PNLBox::Unit LefDriver::_sliceHeight = 0;
PNLBox::Unit LefDriver::_pitchWidth = 0;

int LefDriver::getUnits() {
  return _units;
}
// double             LefDriver::toLefUnits     ( PNLBox::Unit u ) { return
// u;/*PNLUnit::toMicrons(u)*//**getUnits()*/; }
PNLBox::Unit LefDriver::getSliceHeight() {
  return _sliceHeight;
}
PNLBox::Unit LefDriver::getPitchWidth() {
  return _pitchWidth;
};
// inline AllianceFramework* LefDriver::getFramework   () { return _framework; }
inline unsigned int LefDriver::getFlags() const {
  return _flags;
}
inline const std::vector<PNLDesign*> LefDriver::getPNLDesigns() const {
  return _cells;
}
inline int LefDriver::getStatus() const {
  return _status;
}
inline const string& LefDriver::getNLLibraryName() const {
  return _libraryName;
}

LefDriver::LefDriver(const std::vector<PNLDesign*>& cells,
                     const string& libraryName,
                     unsigned int flags,
                     FILE* lefStream)
    : _flags(flags),
      _cells(cells),
      _libraryName(libraryName),
      _lefStream(lefStream),
      _status(0) {
  _status = lefwInitCbk(_lefStream);
  if (_status != 0)
    return;

  lefwSetVersionCbk(versionCbk_);
  lefwSetBusBitCharsCbk(busBitCharsCbk_);
  lefwSetDividerCharCbk(dividerCharCbk_);
  lefwSetSiteCbk(siteCbk_);
  lefwSetUnitsCbk(unitsCbk_);
  lefwSetManufacturingGridCbk(manufacturingGridCbk_);
  lefwSetClearanceMeasureCbk(clearanceMeasureCbk_);
  lefwSetExtCbk(extCbk_);
  lefwSetLayerCbk(layerCbk_);
  lefwSetMacroCbk(macroCbk_);
  lefwSetPropDefCbk(propDefCbk_);
  lefwSetSpacingCbk(spacingCbk_);
  lefwSetUseMinSpacingCbk(useMinSpacingCbk_);
  lefwSetNonDefaultCbk(nonDefaultCbk_);
  lefwSetViaCbk(viaCbk_);
  lefwSetViaRuleCbk(viaRuleCbk_);
  lefwSetEndLibCbk(endLibCbk_);
}

LefDriver::~LefDriver() {}

int LefDriver::write() {
  return checkStatus(lefwWrite(_lefStream, _libraryName.c_str(), (void*)this));
}

int LefDriver::checkStatus(int status) {
  if ((_status = status) != 0) {
    lefwPrintError(_status);
    assert(false);
    // cerr << Error("LefDriver::drive(): Error occured while driving
    // <%s>.",_libraryName.c_str()) << endl;
  }
  return _status;
}

int LefDriver::_driveMacro(PNLDesign* cell) {
  printf("LefDriver::_driveMacro\n");
  _status = lefwStartMacro(cell->getName().getString().c_str());
  CHECK_STATUS(_status);
  printf("a\n");
  const PNLBox& abutmentBox(cell->getAbutmentBox());
  const char* macroClass = NULL;
  const char* macroSubClass = NULL;
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
  _status = lefwMacroClass(macroClass, macroSubClass);
  CHECK_STATUS(_status);
  double originX = abutmentBox.getLeft();
  double originY = abutmentBox.getBottom();
  _status = lefwMacroOrigin(0.0, 0.0);
  CHECK_STATUS(_status);
  double sizeX = abutmentBox.getWidth();
  double sizeY = abutmentBox.getHeight();
  _status = lefwMacroSize(sizeX, sizeY);
  CHECK_STATUS(_status);
  _status = lefwMacroSymmetry("X Y");
  CHECK_STATUS(_status);
  if (cell->getSite() != NULL) {
    _status = lefwMacroSite(cell->getSite()->getName().getString().c_str());
  }
  CHECK_STATUS(_status);

  PNLNet* blockagePNLNet = NULL;

  for (PNLNet* inet : cell->getNets()) {
    PNLNet* net = inet;
    // if ( (blockagePNLNet == NULL) and _framework->isBLOCKAGE(net->getName())
    // )
    //  blockagePNLNet = net;

    if (not net->isExternal())
      continue;

    _status = lefwStartMacroPin(net->getName().getString().c_str());
    CHECK_STATUS(_status);

    _status = lefwMacroPinDirection("INPUT");
    CHECK_STATUS(_status);

    if (net->isSupply()) {
      _status = lefwMacroPinShape("ABUTMENT");
      CHECK_STATUS(_status);
    }

    const char* pinUse = "SIGNAL";
    if (net->isGND())
      pinUse = "GROUND";
    else if (net->isVDD())
      pinUse = "POWER";
    else if (net->isClock())
      pinUse = "CLOCK";
    _status = lefwMacroPinUse(pinUse);
    CHECK_STATUS(_status);

    _status = lefwStartMacroPinPort(NULL);
    CHECK_STATUS(_status);

    _status = lefwEndMacroPinPort();
    CHECK_STATUS(_status);

    _status = lefwEndMacroPin(net->getName().getString().c_str());
    CHECK_STATUS(_status);
  }

  _status = lefwEndMacro(cell->getName().getString().c_str());
  RETURN_CHECK_STATUS(_status);
}

int LefDriver::versionCbk_(lefwCallbackType_e, lefiUserData udata) {
  LefDriver* driver = (LefDriver*)udata;

  ostringstream comment;
  comment << "For design <" << driver->getNLLibraryName() << ">.";

  lefwNewLine();
  lefwAddComment("LEF generated by najaeda.");
  lefwAddComment(comment.str().c_str());
  lefwNewLine();

  return driver->checkStatus(lefwVersion(5, 7));
}

int LefDriver::busBitCharsCbk_(lefwCallbackType_e, lefiUserData udata) {
  LefDriver* driver = (LefDriver*)udata;
  lefwNewLine();
  return driver->checkStatus(lefwBusBitChars("()"));
}

int LefDriver::dividerCharCbk_(lefwCallbackType_e, lefiUserData udata) {
  LefDriver* driver = (LefDriver*)udata;
  return driver->checkStatus(lefwDividerChar("."));
}

int LefDriver::unitsCbk_(lefwCallbackType_e, lefiUserData udata) {
  LefDriver* driver = (LefDriver*)udata;
  lefwNewLine();

  int status = lefwStartUnits();
  if (status != 0)
    return driver->checkStatus(status);

  status = lefwUnits(0  // time.
                     ,
                     0  // capacitance.
                     ,
                     0  // resistance.
                     ,
                     0  // power.
                     ,
                     0  // current.
                     ,
                     0  // voltage.
                     ,
                     LefDriver::getUnits()  // database.
  );
  if (status != 0)
    return driver->checkStatus(status);

  status = lefwEndUnits();

  status = lefwManufacturingGrid(
      PNLTechnology::getOrCreate()
          ->getManufacturingGrid() /*LefDriver::toLefUnits(PNLUnit::fromGrid(1.0))*/);

  if (status != 0)
    return driver->checkStatus(status);

  return driver->checkStatus(status);
}

int LefDriver::layerCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDriver::siteCbk_(lefwCallbackType_e, lefiUserData udata) {
  LefDriver* driver = (LefDriver*)udata;

  // Iterate through all sites
  for (PNLSite* site : PNLTechnology::getOrCreate()->getSites()) {
    // Debugging site details
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
    int status = lefwSite(
        site->getName().getString().c_str(), site->getClass().c_str(),
        symmetry == "" ? nullptr : symmetry.c_str(),  // Default orientation
        site->getWidth(), site->getHeight());

    // Handle errors
    if (status != 0) {
      return driver->checkStatus(status);  // Handle error status
    }

    // End each site to ensure proper state reset
    status = lefwEndSite(site->getName().getString().c_str());
    if (status != 0) {
      return driver->checkStatus(status);
    }
  }

  // Callback completed successfully
  return 0;
}

int LefDriver::extCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDriver::propDefCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDriver::clearanceMeasureCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDriver::endLibCbk_(lefwCallbackType_e, lefiUserData udata) {
  LefDriver* driver = (LefDriver*)udata;
  return driver->checkStatus(lefwEnd());
}

int LefDriver::macroCbk_(lefwCallbackType_e, lefiUserData udata) {
  LefDriver* driver = (LefDriver*)udata;

  const std::vector<PNLDesign*>& cells = driver->getPNLDesigns();
  std::vector<PNLDesign*>::const_iterator icell = cells.begin();
  for (; (icell != cells.end()) and (driver->getStatus() == 0); ++icell) {
    printf("LE: %s\n", (*icell)->getName().getString().c_str());
    driver->_driveMacro(*icell);
  }

  return driver->getStatus();
}

int LefDriver::manufacturingGridCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDriver::nonDefaultCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDriver::spacingCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDriver::useMinSpacingCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDriver::viaCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDriver::viaRuleCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

void LefDriver::drive(const std::vector<PNLDesign*>& cells,
                      const string& libraryName,
                      unsigned int flags) {
  FILE* lefStream = NULL;
  try {
    string path = "./" + libraryName + ".lef";

    lefStream = fopen(path.c_str(), "w");
    if (lefStream == NULL) {
      // throw Error("LefDriver::drive(): Cannot open <%s>.",path.c_str());
      assert(false);
    }

    unique_ptr<LefDriver> driver(
        new LefDriver(cells, libraryName, flags, lefStream));
    driver->write();
  } catch (...) {
    if (lefStream != NULL)
      fclose(lefStream);

    throw;
  }
  fclose(lefStream);
}

}  // End of anonymous namespace.

using naja::NL::NLLibrary;
using naja::NL::PNLTransform;
using std::cerr;
using std::endl;
using std::string;

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

void LefExport::drive(NLLibrary* library, unsigned int flags) {
  string libraryName = "symbolic";
  std::vector<PNLDesign*> cells;

  if (library != NULL) {
    libraryName = library->getName().getString();

    for (PNLDesign* icell : library->getPNLDesigns()) {
      // if ( cells.find(icell) == cells.end())
      printf("LE cell name: %s\n", icell->getName().getString().c_str());
      cells.push_back(icell);
    }
  }
  LefDriver::drive(cells, libraryName, flags);
}
