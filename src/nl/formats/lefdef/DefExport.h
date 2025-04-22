// SPDX-FileCopyrightText: 2025 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace naja {
  namespace PNL {
    class PNLDesign;
  }
}


//namespace CRL {


// -------------------------------------------------------------------
// Class  :  "CRL::DefExport".

  class DefExport {
    public:
      static const uint32_t WithLEF         = (1 << 0);
      static const uint32_t ExpandDieArea   = (1 << 1);
      static const uint32_t ProtectNetNames = (1 << 2);
    public:
      static void  drive ( naja::NL::PNLDesign*, uint32_t flags );
  };


//} // CRL namespace.
