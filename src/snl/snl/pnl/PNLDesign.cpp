// SPDX-FileCopyrightText: 2024 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include "PNLDesign.h"

#include "SNLDB.h"
#include "SNLLibrary.h"

// for ostringstream
#include <sstream>
#include "SNLException.h"

using namespace naja::SNL; 

namespace naja { namespace PNL {

PNLDesign::PNLDesign(SNLLibrary* library, const naja::SNL::SNLName& name):
  super(), library_(library), origin_(0, 0), name_(name)
{}

PNLDesign* PNLDesign::create(SNLLibrary* library, const naja::SNL::SNLName& name) {
  preCreate(library);
  auto design = new PNLDesign(library, name);
  design->postCreateAndSetID();
  return design;
}

void PNLDesign::preCreate(const SNLLibrary* library) {
  super::preCreate();
}

void PNLDesign::postCreateAndSetID() {
  super::postCreate();
  library_->addPNLDesignAndSetID(this);
}

void PNLDesign::postCreate() {
  super::postCreate();
  library_->addPNLDesign(this);
}

void PNLDesign::preDestroy() {
  for (auto term : terms_) {
    if (term) {
      term->destroyFromDesign();
    }
  }
  super::preDestroy();
}

//LCOV_EXCL_START
const char* PNLDesign::getTypeName() const {
  return "PNLDesign";
}
//LCOV_EXCL_STOP

std::string PNLDesign::getString() const {
  return "PNLDesign";
}

std::string PNLDesign::getDescription() const {
  return "PNLDesign";
}

bool PNLDesign::deepCompare(const PNLDesign* other, std::string& reason) const {
  return false;
}

void PNLDesign::debugDump(size_t indent, bool recursive, std::ostream& stream) const {

}

SNLDB* PNLDesign::getDB() const {
  return library_->getDB();
}

SNLID PNLDesign::getSNLID() const {
  return SNLID(getDB()->getID(), library_->getID(), getID());
}

void PNLDesign::addTerm(PNLTerm* term) {
  terms_.push_back(term);
}

PNLTerm* PNLDesign::getTerm(SNLID::DesignObjectID id) const {
  if (id >= terms_.size()) {
    // TODO: throw exception
    return nullptr;
  }
  return terms_[id];
}

void PNLDesign::detachTerm(SNLID::DesignObjectID id) {
  if (id >= terms_.size()) {
    // TODO: throw exception
    return;
  }
  terms_[id] = nullptr;
}

void PNLDesign::detachNet(SNLID::DesignObjectID id) {
  if (id >= nets_.size()) { 
    // TODO: throw exception
  }
  nets_[id] = nullptr;
}

void PNLDesign::addNet(PNLNet* net) {
  nets_.push_back(net);
}

naja::PNL::PNLNet* PNLDesign::getNet(naja::SNL::SNLID::DesignObjectID id) const {
  if (id >= nets_.size()) {
    // TODO: throw exception
    return nullptr;
  }
  return nets_[id];
}

void PNLDesign::detachInstance(SNLID::DesignObjectID id) {
  if (id >= instances_.size()) {
    // TODO: throw exception
  }
  instances_[id] = nullptr;
}

PNLInstance* PNLDesign::getInstance(SNLID::DesignObjectID id) const {
  if (id >= instances_.size()) {
    // TODO: throw exception
  }
  return instances_[id];
}

naja::PNL::PNLNet* PNLDesign::getNet(const naja::SNL::SNLName& name) const {
  for (auto net : nets_) {
    if (net->getName() == name) {
      return net;
    }
  }
  return nullptr;
}

PNLNet* PNLDesign::addNet(const naja::SNL::SNLName& name) {
  PNLNet* net = PNLNet::create(this, name, ( unsigned int ) nets_.size());
  return net;
}

void PNLDesign::setAbutmentBox(const PNLBox& abutmentBox)
// ***********************************************
{
  // SlavedsRelation* slaveds = SlavedsRelation::get( this );
  // if (not slaveds or (this == slaveds->getMasterOwner())) {
  //   _setAbutmentBox( abutmentBox ); 

  //   if (getFlags().isset(Flags::SlavedAb)) return;

  //   for ( Cell* slavedCell : SlavedsSet(this) )
  //     slavedCell->_setAbutmentBox( abutmentBox );
  // } else {
  //   // cerr << Error( "Cell::setAbutmentBox(): Abutment box of \"%s\" is slaved to \"%s\"."
  //   //              , getString(getName()).c_str()
  //   //              , getString(static_cast<Cell*>(slaveds->getMasterOwner())->getName()).c_str()
  //   //              ) << endl;
  //   assert(false);
  // }
  _setAbutmentBox( abutmentBox ); 
}

void PNLDesign::_setAbutmentBox(const PNLBox& abutmentBox)
// ***********************************************
{
  if (abutmentBox != abutmentBox_) {
    if (not abutmentBox_.isEmpty() and
       (abutmentBox.isEmpty() or not abutmentBox.contains(abutmentBox_)))
      _unfit( abutmentBox_ );
    abutmentBox_ = abutmentBox;
    _fit( abutmentBox_ );
  }
}

void PNLDesign::_fit(const PNLBox& box)
// ****************************
{
    if (box.isEmpty()) return;
    if (boundingBox_.isEmpty()) return;
    if (boundingBox_.contains(box)) return;
    boundingBox_.merge(box);
    for ( PNLInstance* iinstance : getSlaveInstances() ) {
      iinstance->getDesign()->_fit(iinstance->getTransform().getBox(box));
    }
}

void PNLDesign::_unfit(const PNLBox& box)
// ******************************
{
    if (box.isEmpty()) return;
    if (boundingBox_.isEmpty()) return;
    if (!boundingBox_.isConstrainedBy(box)) return;
    boundingBox_.makeEmpty();
    for ( PNLInstance* iinstance : getSlaveInstances() ) {
        iinstance->getDesign()->_unfit(iinstance->getTransform().getBox(box));
    }
}

naja::PNL::PNLTerm* PNLDesign::addTerm(const naja::SNL::SNLName& name) {
  PNLTerm* term = PNLTerm::create(this, ( unsigned int ) terms_.size(), name);
  return term;
}

PNLTerm* PNLDesign::getTerm(naja::SNL::SNLName name) const {
  for (auto term : terms_) {
    if (term->getName() == name) {
      return term;
    }
  }
  return nullptr;
}

void PNLDesign::setName(const naja::SNL::SNLName& name) {
  if (name_ == name) {
    return;
  }
  if (not name.empty()) {
    /* check collision */
    if (auto collision = getLibrary()->getSNLDesign(name)) {
      std::ostringstream reason;
      reason << "In library " << getLibrary()->getString()
        << ", cannot rename " << getString() << " to "
        << name.getString() << ", another Design " << collision->getString()
        << " has already this name.";
      throw SNLException(reason.str());
    }
  }
  auto previousName = getName();
  name_ = name;
  getLibrary()->rename(this, previousName);
}

}} // namespace PNL // namespace naja