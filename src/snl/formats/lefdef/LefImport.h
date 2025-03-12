// -*- C++ -*-
//
// This file is part of the Coriolis Software.
// Copyright (c) UPMC 2017-2018, All Rights Reserved
//
// +-----------------------------------------------------------------+
// |                   C O R I O L I S                               |
// |        C a d e n c e   L E F   I m p o r t e r                  |
// |                                                                 |
// |  Author      :                    Jean-Paul CHAPUT              |
// |  E-mail      :i           Jean-Paul.Chaput@lip6.fr              |
// | =============================================================== |
// |  C++ Header  :       "./crlcore/LefImport.h"                    |
// +-----------------------------------------------------------------+

#ifndef CRL_LEF_IMPORT_H
#define CRL_LEF_IMPORT_H

#include <string>

namespace naja {
namespace SNL {
class SNLLibrary;
}
}  // namespace naja // namespace SNL

class LefImport {
 public:
  static void reset();
  static naja::SNL::SNLLibrary* load(std::string fileName);
  static void setMergeLibrary(naja::SNL::SNLLibrary*);
  static void setGdsForeignDirectory(std::string path);
};

#endif  // CRL_DEF_IMPORT_H