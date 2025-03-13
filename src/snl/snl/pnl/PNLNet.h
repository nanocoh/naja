// SPDX-FileCopyrightText: 2025 The Naja authors
// <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "PNLDesignObject.h"
#include "SNLID.h"
#include "SNLName.h"

namespace naja {
namespace PNL {

class PNLDesign;
class PNLTerm;
class PNLInstTerm;

class PNLNet : public PNLDesignObject {
 public:

  class Direction {
   public:
    enum DirectionEnum {
      Input,   ///< Input direction.
      Output,  ///< Output direction.
      InOut,    ///< InOut direction.
      Tristate, ///< Tristate direction.
    };
    Direction(const DirectionEnum& dirEnum) : dirEnum_(dirEnum) {}
    Direction(const Direction& direction) = default;
    Direction& operator=(const Direction& direction) = default;
    operator const DirectionEnum&() const { return dirEnum_; }
    std::string getString() const;

   private:
    DirectionEnum dirEnum_;
  };

  using super = PNLDesignObject;

  public: enum Type {UNDEFINED=0, LOGICAL=1, CLOCK=2, POWER=3, GROUND=4, BLOCKAGE=5, FUSED=6};

  static PNLNet* create(PNLDesign* design,
                        const naja::SNL::SNLName& name,
                        naja::SNL::SNLID::DesignObjectID id);

  PNLDesign* getDesign() const override;

  naja::SNL::SNLID::DesignObjectID getID() const;

  void destroyFromDesign();

  void addTerm(PNLTerm* term);
  PNLTerm* getTerm(naja::SNL::SNLID::DesignObjectID id) const;
  void detachTerm(naja::SNL::SNLID::DesignObjectID id);

  void addInstTerm(PNLInstTerm* instTerm);
  PNLInstTerm* getInstTerm(naja::SNL::SNLID::DesignObjectID id) const;
  void detachInstTerm(naja::SNL::SNLID::DesignObjectID id);

  const char* getTypeName() const override { return "PNLNet"; }
  std::string getString() const override { return "PNLNet"; }
  std::string getDescription() const override { return "PNLNet"; }
  void debugDump(size_t indent,
                 bool recursive = true,
                 std::ostream& stream = std::cerr) const override {}
  naja::SNL::SNLName getName() const { return name_; }
  bool isVCC() const { return isVCC_; }
  bool isGND() const { return !isVCC_; }
  void seeIsVCC(bool vcc) { isVCC_ = vcc; }
  bool isSupply   () const {return (isVCC() || isGND());};

  const std::vector<PNLTerm*>& getTerms() const { return terms_; }

  public: const Type& getType() const {return type_;};
  void setName(const naja::SNL::SNLName& name) { name_ = name; }
  void setType(Type type) { type_ = type; }
  void setExternal(bool external) { external_ = external; }
  bool isExternal() const { return external_; }

  void setDirection(const Direction& direction) { direction_ = direction; }
  Direction getDirection() const { return direction_; }

  void setGlobal(bool global) { isGlobal_ = global; }
  bool isGlobal() const { return isGlobal_; }
  
  naja::SNL::SNLID getSNLID() const override;

  bool isPower() const { return type_ == Type::POWER; }
  bool isGround() const { return type_ == Type::GROUND; }
  bool isClock() const { return type_ == Type::CLOCK; }
  
 protected:

  PNLNet() = default;

  static void preCreate();
  void postCreate();
  void preDestroy() override;

 private:
  naja::SNL::SNLName name_;
  naja::SNL::SNLID::DesignObjectID id_;
  PNLDesign* design_;
  std::vector<PNLTerm*> terms_;
  std::vector<PNLInstTerm*> instTerms_;
  bool isVCC_ = false;
  Type type_ = Type::UNDEFINED;
  bool external_ = false;
  Direction direction_ = Direction::Input;
  bool isGlobal_ = false;
};

}  // namespace PNL
}  // namespace naja