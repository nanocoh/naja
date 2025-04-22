// SPDX-FileCopyrightText: 2025 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef  __CRL_LEF_EXPORT__
#define  __CRL_LEF_EXPORT__


namespace naja {
namespace NL {
class NLLibrary;
}
namespace PNL {
class PNLDesign;
}
}  // namespace naja // namespace NL



  class LefExport {
    public:
      enum Flag { WithTechnology=0x1, WithSpacers=0x2 };
    public:
      static void  drive ( naja::NL::PNLDesign*   , unsigned int flags );
      static void  drive ( naja::NL::NLLibrary*, unsigned int flags );
  };



#endif  // __CRL_LEF_EXPORT__
