// SPDX-FileCopyrightText: 2025 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include "PNLTerm.h"
#include "PNLDesign.h"

namespace naja { namespace PNL {

PNLTerm::Direction::Direction(const DirectionEnum& dirEnum):
  dirEnum_(dirEnum) 
{}

PNLTerm* PNLTerm::create(PNLDesign* design,
                         naja::SNL::SNLID::DesignObjectID id,
                         const naja::SNL::SNLName& name,
                         Direction direction) {
  PNLTerm* term = new PNLTerm();
  term->design_ = design;
  term->name_ = name;
  term->direction_ = direction;
  term->id_ = id;
  term->preCreate();
  term->postCreate();
  design->addTerm(term);
  printf("add to design %s\n", design->getName().getString().c_str());
  return term;
}

std::string PNLTerm::Direction::getString() const {
  switch (dirEnum_) {
    case Direction::Input: return "Input";
    case Direction::Output: return "Output";
    case Direction::InOut: return "InOut";
    case Direction::None: return "None";
  }
  return "None";
}

PNLTerm::Direction PNLTerm::getDirection() {
  return direction_;
}

PNLNet* PNLTerm::getNet() {
  return net_;
}

void PNLTerm::setNet(PNLNet* net) {
  net_ = net;
}

PNLDesign* PNLTerm::getDesign() const {
  return design_;
}

naja::SNL::SNLID::DesignObjectID PNLTerm::getID() const {
  return id_;
}

void PNLTerm::preCreate() {
}

void PNLTerm::postCreate() {
}

void PNLTerm::preDestroy() {
  design_->detachTerm(id_);
  super::preDestroy();
}

void PNLTerm::destroyFromDesign() {
  preDestroy();
  delete this;
}

void PNLTerm::translate(const PNLUnit::Unit& dx, const PNLUnit::Unit& dy)
// **************************************************************
{
    if ((dx != 0) || (dy != 0)) {
        //invalidate(true);
        _dx += dx;
        _dy += dy;
    }
}

naja::SNL::SNLID PNLTerm::getSNLID() const {
  return PNLDesignObject::getSNLID(naja::SNL::SNLID::Type::Instance, 0, id_, 0);
}

}} // namespace PNL // namespace naja
