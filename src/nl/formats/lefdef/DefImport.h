
// SPDX-FileCopyrightText: 2025 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0 

#ifndef  CRL_DEF_IMPORT_H
#define  CRL_DEF_IMPORT_H

#include <string>

namespace naja {
  namespace PNL {
    class PNLDesign;
  }
  namespace NL {
    class NLDB;
  }
}
  class DefImport {
    public:
      enum Flags { FitAbOnDesigns=0x1 };
    public:
      static void             reset ();
      static naja::NL::PNLDesign* load  ( std::string design, unsigned int flags, naja::NL::NLDB* db );
  };

#endif  // CRL_DEF_IMPORT_H
