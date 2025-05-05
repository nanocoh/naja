// SPDX-FileCopyrightText: 2025 The Naja authors
// <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace naja {
namespace NL {

class NLLibrary;
namespace PNL {
class PNLDesign;
}

class LefExport {
 public:
  enum Flag { WithTechnology = 0x1, WithSpacers = 0x2 };

 public:
  static void dump(naja::NL::NLLibrary*, unsigned int flags);
};

}  // namespace NL
}  // namespace naja