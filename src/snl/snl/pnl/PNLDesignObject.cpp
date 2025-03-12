// SPDX-FileCopyrightText: 2024 The Naja authors
// <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include "PNLDesignObject.h"
#include "SNLDB.h"
#include "SNLLibrary.h"

namespace naja {
namespace PNL {

naja::SNL::SNLID PNLDesignObject::getSNLID(const naja::SNL::SNLID::Type& type,
                                naja::SNL::SNLID::DesignObjectID objectID,
                                naja::SNL::SNLID::DesignObjectID instanceID,
                                naja::SNL::SNLID::Bit bit) const {
  return naja::SNL::SNLID(type, getDB()->getID(), getLibrary()->getID(),
               getDesign()->getID(), objectID, instanceID, bit);
}

naja::SNL::SNLLibrary* PNLDesignObject::getLibrary() const {
  return getDesign()->getLibrary();
}

naja::SNL::SNLDB* PNLDesignObject::getDB() const {
  return getLibrary()->getDB();
}

void PNLDesignObject::preCreate() {}

void PNLDesignObject::postCreate() {}

}  // namespace PNL
}  // namespace naja