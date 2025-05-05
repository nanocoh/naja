// SPDX-FileCopyrightText: 2025 The Naja authors
// <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include "LefExport.h"
#include <memory>
#include <set>
#include "NLLibrary.h"
#include "PNLBox.h"
#include "PNLDesign.h"
#include "PNLNet.h"
#include "lefwWriter.hpp"
#include "lefwWriterCalls.hpp"
// for ostringstream
#include <sstream>
#include "PNLSite.h"
#include "PNLTechnology.h"
#include "PNLUnit.h"

using namespace std;
using namespace naja::NL;
using namespace naja::NL;

struct Comparator {
  bool operator()(PNLDesign* a, PNLDesign* b) const {
    return a->getID() > b->getID();
  }
};

#define CHECKstatus_(status) \
  if ((status) != 0)         \
    return checkStatus(status);
#define RETURN_CHECKstatus_(status) return checkStatus(status);

class LefDumper {
 public:
  static void dump(const std::vector<PNLDesign*>&,
                    const string& libraryName,
                    unsigned int flags);
  static int getUnits();
  // static double             toLefUnits            ( PNLBox::Unit );
  static PNLBox::Unit getSliceHeight();
  static PNLBox::Unit getPitchWidth();
  ~LefDumper();
  int write();

 private:
  LefDumper(const std::vector<PNLDesign*>&,
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
  //  int                _dumpRoutingLayer    ( RoutingLayerGauge* );
  //  int                _dumpCutLayer        ( Layer* );
  int dumpMacro_(PNLDesign*);

 private:
  // static AllianceFramework* _framework;
  static int units_;
  static PNLBox::Unit sliceHeight_;
  static PNLBox::Unit pitchWidth_;
  unsigned int flags_;
  const std::vector<PNLDesign*> cells_;
  string libraryName_;
  FILE* lefStream_;
  int status_;
};

int LefDumper::units_ = 100;
// AllianceFramework* LefDumper::_framework   = NULL;
PNLBox::Unit LefDumper::sliceHeight_ = 0;
PNLBox::Unit LefDumper::pitchWidth_ = 0;

int LefDumper::getUnits() {
  return units_;
}
// double             LefDumper::toLefUnits     ( PNLBox::Unit u ) { return
// u;/*PNLUnit::toMicrons(u)*//**getUnits()*/; }
PNLBox::Unit LefDumper::getSliceHeight() {
  return sliceHeight_;
}
PNLBox::Unit LefDumper::getPitchWidth() {
  return pitchWidth_;
};
// inline AllianceFramework* LefDumper::getFramework   () { return _framework; }
inline unsigned int LefDumper::getFlags() const {
  return flags_;
}
inline const std::vector<PNLDesign*> LefDumper::getPNLDesigns() const {
  return cells_;
}
inline int LefDumper::getStatus() const {
  return status_;
}
inline const string& LefDumper::getNLLibraryName() const {
  return libraryName_;
}

LefDumper::LefDumper(const std::vector<PNLDesign*>& cells,
                     const string& libraryName,
                     unsigned int flags,
                     FILE* lefStream)
    : flags_(flags),
      cells_(cells),
      libraryName_(libraryName),
      lefStream_(lefStream),
      status_(0) {
  status_ = lefwInitCbk(lefStream_);
  if (status_ != 0)
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

LefDumper::~LefDumper() {}

int LefDumper::write() {
  return checkStatus(lefwWrite(lefStream_, libraryName_.c_str(), (void*)this));
}

int LefDumper::checkStatus(int status) {
  if ((status_ = status) != 0) {
    lefwPrintError(status_);
    assert(false);
    // cerr << Error("LefDumper::dump(): Error occured while driving
    // <%s>.",libraryName_.c_str()) << endl;
  }
  return status_;
}

int LefDumper::dumpMacro_(PNLDesign* cell) {
  status_ = lefwStartMacro(cell->getName().getString().c_str());
  CHECKstatus_(status_);
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
  status_ = lefwMacroClass(macroClass, macroSubClass);
  CHECKstatus_(status_);
  double originX = abutmentBox.getLeft();
  double originY = abutmentBox.getBottom();
  status_ = lefwMacroOrigin(0.0, 0.0);
  CHECKstatus_(status_);
  double sizeX = abutmentBox.getWidth();
  double sizeY = abutmentBox.getHeight();
  status_ = lefwMacroSize(sizeX, sizeY);
  CHECKstatus_(status_);
  status_ = lefwMacroSymmetry("X Y");
  CHECKstatus_(status_);
  if (cell->getSite() != NULL) {
    status_ = lefwMacroSite(cell->getSite()->getName().getString().c_str());
  }
  CHECKstatus_(status_);

  PNLNet* blockagePNLNet = NULL;

  for (PNLNet* inet : cell->getNets()) {
    PNLNet* net = inet;
    // if ( (blockagePNLNet == NULL) and _framework->isBLOCKAGE(net->getName())
    // )
    //  blockagePNLNet = net;

    if (not net->isExternal())
      continue;

    status_ = lefwStartMacroPin(net->getName().getString().c_str());
    CHECKstatus_(status_);

    status_ = lefwMacroPinDirection("INPUT");
    CHECKstatus_(status_);

    if (net->isSupply()) {
      status_ = lefwMacroPinShape("ABUTMENT");
      CHECKstatus_(status_);
    }

    const char* pinUse = "SIGNAL";
    if (net->isGND())
      pinUse = "GROUND";
    else if (net->isVDD())
      pinUse = "POWER";
    else if (net->isClock())
      pinUse = "CLOCK";
    status_ = lefwMacroPinUse(pinUse);
    CHECKstatus_(status_);

    status_ = lefwStartMacroPinPort(NULL);
    CHECKstatus_(status_);

    status_ = lefwEndMacroPinPort();
    CHECKstatus_(status_);

    status_ = lefwEndMacroPin(net->getName().getString().c_str());
    CHECKstatus_(status_);
  }

  status_ = lefwEndMacro(cell->getName().getString().c_str());
  RETURN_CHECKstatus_(status_);
}

int LefDumper::versionCbk_(lefwCallbackType_e, lefiUserData udata) {
  LefDumper* dumpr = (LefDumper*)udata;

  ostringstream comment;
  comment << "For design <" << dumpr->getNLLibraryName() << ">.";

  lefwNewLine();
  lefwAddComment("LEF generated by najaeda.");
  lefwAddComment(comment.str().c_str());
  lefwNewLine();

  return dumpr->checkStatus(lefwVersion(5, 7));
}

int LefDumper::busBitCharsCbk_(lefwCallbackType_e, lefiUserData udata) {
  LefDumper* dumpr = (LefDumper*)udata;
  lefwNewLine();
  return dumpr->checkStatus(lefwBusBitChars("()"));
}

int LefDumper::dividerCharCbk_(lefwCallbackType_e, lefiUserData udata) {
  LefDumper* dumpr = (LefDumper*)udata;
  return dumpr->checkStatus(lefwDividerChar("."));
}

int LefDumper::unitsCbk_(lefwCallbackType_e, lefiUserData udata) {
  LefDumper* dumpr = (LefDumper*)udata;
  lefwNewLine();

  int status = lefwStartUnits();
  if (status != 0)
    return dumpr->checkStatus(status);

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
                     LefDumper::getUnits()  // database.
  );
  if (status != 0)
    return dumpr->checkStatus(status);

  status = lefwEndUnits();

  status = lefwManufacturingGrid(
      PNLTechnology::getOrCreate()
          ->getManufacturingGrid() /*LefDumper::toLefUnits(PNLUnit::fromGrid(1.0))*/);

  if (status != 0)
    return dumpr->checkStatus(status);

  return dumpr->checkStatus(status);
}

int LefDumper::layerCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDumper::siteCbk_(lefwCallbackType_e, lefiUserData udata) {
  LefDumper* dumpr = (LefDumper*)udata;

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
      return dumpr->checkStatus(status);  // Handle error status
    }

    // End each site to ensure proper state reset
    status = lefwEndSite(site->getName().getString().c_str());
    if (status != 0) {
      return dumpr->checkStatus(status);
    }
  }

  // Callback completed successfully
  return 0;
}

int LefDumper::extCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDumper::propDefCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDumper::clearanceMeasureCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDumper::endLibCbk_(lefwCallbackType_e, lefiUserData udata) {
  LefDumper* dumpr = (LefDumper*)udata;
  return dumpr->checkStatus(lefwEnd());
}

int LefDumper::macroCbk_(lefwCallbackType_e, lefiUserData udata) {
  LefDumper* dumpr = (LefDumper*)udata;

  const std::vector<PNLDesign*>& cells = dumpr->getPNLDesigns();
  std::vector<PNLDesign*>::const_iterator icell = cells.begin();
  for (; (icell != cells.end()) and (dumpr->getStatus() == 0); ++icell) {
    dumpr->dumpMacro_(*icell);
  }

  return dumpr->getStatus();
}

int LefDumper::manufacturingGridCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDumper::nonDefaultCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDumper::spacingCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDumper::useMinSpacingCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDumper::viaCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

int LefDumper::viaRuleCbk_(lefwCallbackType_e, lefiUserData udata) {
  return 0;
}

void LefDumper::dump(const std::vector<PNLDesign*>& cells,
                      const string& libraryName,
                      unsigned int flags) {
  FILE* lefStream = NULL;
  try {
    string path = "./" + libraryName + ".lef";

    lefStream = fopen(path.c_str(), "w");
    if (lefStream == NULL) {
      // throw Error("LefDumper::dump(): Cannot open <%s>.",path.c_str());
      assert(false);
    }

    unique_ptr<LefDumper> dumpr(
        new LefDumper(cells, libraryName, flags, lefStream));
    dumpr->write();
  } catch (...) {
    if (lefStream != NULL)
      fclose(lefStream);

    throw;
  }
  fclose(lefStream);
}

using naja::NL::NLLibrary;
using naja::NL::PNLTransform;
using std::cerr;
using std::endl;
using std::string;

void LefExport::dump(NLLibrary* library, unsigned int flags) {
  string libraryName = "symbolic";
  std::vector<PNLDesign*> cells;

  if (library != NULL) {
    libraryName = library->getName().getString();

    for (PNLDesign* icell : library->getPNLDesigns()) {
      if ( std::find(cells.begin(), cells.end(), icell) == cells.end() ) {
        cells.push_back(icell);
      }
    }
  }
  LefDumper::dump(cells, libraryName, flags);
}
