// SPDX-FileCopyrightText: 2024 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __PNL_DESIGN_OBJECT_H_
#define __PNL_DESIGN_OBJECT_H_

#include "SNLObject.h"
#include "SNLID.h"

namespace naja { namespace SNL {

class SNLDB;
class SNLLibrary;

} } // namespace SNL // namespace najas

namespace naja { namespace PNL {

class PNLDesign;

class PNLDesignObject: public SNL::SNLObject {
  public:
    virtual PNLDesign* getDesign() const = 0;

    virtual naja::SNL::SNLID getSNLID() const = 0;
    naja::SNL::SNLID getSNLID(const naja::SNL::SNLID::Type& type,
        naja::SNL::SNLID::DesignObjectID id,
        naja::SNL::SNLID::DesignObjectID instanceID,
        naja::SNL::SNLID::Bit bit) const;

  bool operator<(const PNLDesignObject &rhs) const {
      return getSNLID() < rhs.getSNLID();
  }

  naja::SNL::SNLLibrary* getLibrary() const;
  
  naja::SNL::SNLDB* getDB() const;

  protected:
    PNLDesignObject() = default;
    
    static void preCreate();
    void postCreate();
};

}} // namespace PNL // namespace naja

#endif // __PNL_DESIGN_OBJECT_H_