// SPDX-FileCopyrightText: 2025 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

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