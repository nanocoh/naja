// SPDX-FileCopyrightText: 2023 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include "SNLLibrary.h"

#include <iostream>
#include <sstream>

#include "SNLDB.h"
#include "SNLException.h"
#include "SNLMacros.h"

using namespace naja::PNL; 

namespace naja { namespace SNL {

SNLLibrary::Type::Type(const TypeEnum& typeEnum):
  typeEnum_(typeEnum) 
{}

//LCOV_EXCL_START
std::string SNLLibrary::Type::getString() const {
  switch (typeEnum_) {
    case Type::Standard: return "Standard";
    case Type::InDB0: return "InDB0";
    case Type::Primitives: return "Primitives";
  }
  return "Unknown";
}
//LCOV_EXCL_STOP

SNLLibrary::SNLLibrary(SNLDB* parent, Type type, const SNLName& name):
  super(),
  name_(name),
  type_(type),
  parent_(parent),
  isRoot_(true)
{}

SNLLibrary::SNLLibrary(SNLDB* parent, SNLID::LibraryID id, Type type, const SNLName& name):
  super(),
  id_(id),
  name_(name),
  type_(type),
  parent_(parent),
  isRoot_(true)
{}

SNLLibrary::SNLLibrary(SNLLibrary* parent, Type type, const SNLName& name):
  super(),
  name_(name),
  type_(type),
  parent_(parent),
  isRoot_(false)
{}

SNLLibrary::SNLLibrary(SNLLibrary* parent, SNLID::LibraryID id, Type type, const SNLName& name):
  super(),
  id_(id),
  name_(name),
  type_(type),
  parent_(parent),
  isRoot_(false)
{}

SNLLibrary* SNLLibrary::create(SNLDB* db, const SNLName& name) {
  preCreate(db, Type::Standard, name);
  SNLLibrary* library = new SNLLibrary(db, Type::Standard, name);
  library->postCreateAndSetID();
  return library;
}

SNLLibrary* SNLLibrary::create(SNLDB* db, Type type, const SNLName& name) {
  preCreate(db, type, name);
  SNLLibrary* library = new SNLLibrary(db, type, name);
  library->postCreateAndSetID();
  return library;
}

SNLLibrary* SNLLibrary::create(SNLDB* db, SNLID::LibraryID libraryID, const Type type, const SNLName& name) {
  preCreate(db, libraryID, type, name);
  SNLLibrary* library = new SNLLibrary(db, libraryID, type, name);
  library->postCreate();
  return library;
}

SNLLibrary* SNLLibrary::create(SNLLibrary* parent, const SNLName& name) {
  preCreate(parent, Type::Standard, name);
  SNLLibrary* library = new SNLLibrary(parent, Type::Standard, name);
  library->postCreateAndSetID();
  return library;
}

SNLLibrary* SNLLibrary::create(SNLLibrary* parent, Type type, const SNLName& name) {
  preCreate(parent, type, name);
  SNLLibrary* library = new SNLLibrary(parent, type, name);
  library->postCreateAndSetID();
  return library;
}

SNLLibrary* SNLLibrary::create(SNLLibrary* parent, SNLID::LibraryID id, Type type, const SNLName& name) {
  preCreate(parent, id, type, name);
  SNLLibrary* library = new SNLLibrary(parent, id, type, name);
  library->postCreate();
  return library;
}

void SNLLibrary::preCreate(SNLDB* db, Type type, const SNLName& name) {
  super::preCreate();
  if (not db) {
    throw SNLException("malformed SNLLibrary creator with NULL db argument");
  }
  if (not name.empty() and db->getLibrary(name)) {
    std::string reason = "SNLDB " + db->getString() + " contains already a SNLLibrary named: " + name.getString();
    throw SNLException(reason);
  }
}

void SNLLibrary::preCreate(SNLDB* db, SNLID::LibraryID libraryID, Type type, const SNLName& name) {
  preCreate(db, type, name);
  if (db->getLibrary(libraryID)) {
    std::string reason = "SNLDB " + db->getString() + " contains already a SNLLibrary with ID: " + std::to_string(libraryID);
    throw SNLException(reason);
  }
}

void SNLLibrary::preCreate(SNLLibrary* parentLibrary, Type type, const SNLName& name) {
  super::preCreate();
  if (not parentLibrary) {
    throw SNLException("malformed SNLLibrary creator with NULL parent library argument");
  }
  if (type not_eq parentLibrary->getType()) {
    throw SNLException("non compatible types in library constructor");
  }
  if (not name.empty() and parentLibrary->getLibrary(name)) {
    std::string reason = "SNLLibrary " + parentLibrary->getString() + " contains already a SNLLibrary named: " + name.getString();
    throw SNLException(reason);
  }
}

void SNLLibrary::preCreate(SNLLibrary* parentLibrary, SNLID::LibraryID id, Type type, const SNLName& name) {
  preCreate(parentLibrary, type, name);
  auto db = parentLibrary->getDB();
  if (db->getLibrary(id)) {
    std::string reason = "SNLDB " + db->getString() + " contains already a SNLLibrary with ID: " + std::to_string(id);
    throw SNLException(reason);
  }
}

void SNLLibrary::postCreateAndSetID() {
  super::postCreate();
  getDB()->addLibraryAndSetID(this);
  if (not isRoot()) {
    static_cast<SNLLibrary*>(parent_)->addLibrary(this);
  }
}

void SNLLibrary::postCreate() {
  super::postCreate();
  getDB()->addLibrary(this);
  if (not isRoot()) {
    static_cast<SNLLibrary*>(parent_)->addLibrary(this);
  }
}

void SNLLibrary::commonPreDestroy() {
#ifdef SNL_DESTROY_DEBUG
  std::cerr << "Destroying " << getDescription() << std::endl; 
#endif
  struct destroyDesignFromLibrary {
    void operator()(SNLDesign* design) {
      design->destroyFromLibrary();
    }
  };
  snlDesigns_.clear_and_dispose(destroyDesignFromLibrary());
  struct destroyLibraryFromLibrary {
    void operator()(SNLLibrary* library) {
      library->destroyFromParentLibrary();
    }
  };
  libraries_.clear_and_dispose(destroyLibraryFromLibrary());
  super::preDestroy();
}

void SNLLibrary::destroyFromDB() {
  commonPreDestroy();
  delete this;
}

void SNLLibrary::destroyFromParentLibrary() {
  getDB()->removeLibrary(this);
  commonPreDestroy();
  delete this;
}

void SNLLibrary::preDestroy() {
  getDB()->removeLibrary(this);
  if (not isRoot()) {
    static_cast<SNLLibrary*>(parent_)->removeLibrary(this);
  }
  commonPreDestroy();
}

void SNLLibrary::setName(const SNLName& name) {
  if (name_ == name) {
    return;
  }
  if (not name.empty()) {
    if (isRoot()) {
      //check in DB
      auto db = getDB();
      if (auto collision = db->getLibrary(name)) {
        std::ostringstream reason;
        //Exclude from coverage to avoid false positive
        //LCOV_EXCL_START
        reason << "In DB " << db->getString() << ", cannot rename "
          << getString() << " to " << name.getString() << ", another library: "
          << collision->getString() << " has already this name.";
        //LCOV_EXCL_STOP
        throw SNLException(reason.str());
      }
    } else {
      auto parentLibrary = getParentLibrary();
      if (auto collision = parentLibrary->getLibrary(name)) {
        std::ostringstream reason;
        //LCOV_EXCL_START
        reason << "In parent library " << parentLibrary->getString()
          << ", cannot rename " << getString() << " to " << name.getString()
          << ", another library: " << collision->getString() << " has already this name.";
        //LCOV_EXCL_STOP
        throw SNLException(reason.str());
      }
    }
  }
  auto previousName = getName();
  name_ = name;
  if (isRoot()) {
    getDB()->rename(this, previousName);
  } else {
    getParentLibrary()->rename(this, previousName);
  }
}

OWNER_RENAME(SNLLibrary, SNLLibrary, libraryNameIDMap_)
OWNER_RENAME(SNLLibrary, SNLDesign, designNameIDMap_)
OWNER_RENAME(SNLLibrary, PNLDesign, pnlDesignNameIDMap_)

SNLDB* SNLLibrary::getDB() const {
  if (isRoot()) {
    return static_cast<SNLDB*>(parent_);
  } else {
    return getParentLibrary()->getDB();
  }
}

SNLLibrary* SNLLibrary::getParentLibrary() const {
  if (not isRoot()) {
    return static_cast<SNLLibrary*>(parent_);
  }
  return nullptr;
}

SNLLibrary* SNLLibrary::getLibrary(SNLID::LibraryID id) const {
  auto it = libraries_.find(SNLID(getDB()->getID(), id), SNLIDComp<SNLLibrary>());
  if (it != libraries_.end()) {
    return const_cast<SNLLibrary*>(&*it);
  }
  return nullptr;
}

SNLLibrary* SNLLibrary::getLibrary(const SNLName& name) const {
  auto lit = libraryNameIDMap_.find(name);
  if (lit != libraryNameIDMap_.end()) {
    SNLID::LibraryID id = lit->second;
    return getLibrary(id);
  }
  return nullptr;
}

NajaCollection<SNLLibrary*> SNLLibrary::getLibraries() const {
  return NajaCollection(new NajaIntrusiveSetCollection(&libraries_));
}

SNLDesign* SNLLibrary::getSNLDesign(SNLID::DesignID id) const {
  auto it = snlDesigns_.find(SNLID(getDB()->getID(), getID(), id), SNLIDComp<SNLDesign>());
  if (it != snlDesigns_.end()) {
    return const_cast<SNLDesign*>(&*it);
  }
  return nullptr;
}

PNLDesign* SNLLibrary::getPNLDesign(SNLID::DesignID id) const {
  auto it = pnlDesigns_.find(SNLID(getDB()->getID(), getID(), id), SNLIDComp<PNLDesign>());
  if (it != pnlDesigns_.end()) {
    return const_cast<PNLDesign*>(&*it);
  }
  return nullptr;
}

SNLDesign* SNLLibrary::getSNLDesign(const SNLName& name) const {
  auto dit = designNameIDMap_.find(name);
  if (dit != designNameIDMap_.end()) {
    SNLID::DesignID id = dit->second;
    return getSNLDesign(id);
  }
  return nullptr;
}

PNLDesign* SNLLibrary::getPNLDesign(const SNLName& name) const {
  auto dit = pnlDesignNameIDMap_.find(name);
  if (dit != pnlDesignNameIDMap_.end()) {
    SNLID::DesignID id = dit->second;
    return getPNLDesign(id);
  }
  return nullptr;
}

NajaCollection<SNLDesign*> SNLLibrary::getSNLDesigns() const {
  return NajaCollection(new NajaIntrusiveSetCollection(&snlDesigns_));
}

NajaCollection<PNLDesign*> SNLLibrary::getPNLDesigns() const {
  return NajaCollection(new NajaIntrusiveSetCollection(&pnlDesigns_));
}

void SNLLibrary::addLibrary(SNLLibrary* library) {
  libraries_.insert(*library);
  if (not library->isAnonymous()) {
    libraryNameIDMap_[library->getName()] = library->id_;
  }
}

void SNLLibrary::removeLibrary(SNLLibrary* library) {
  if (not library->isAnonymous()) {
    libraryNameIDMap_.erase(library->getName());
  }
  libraries_.erase(*library);
}

void SNLLibrary::addSNLDesignAndSetID(SNLDesign* design) {
  if (snlDesigns_.empty()) {
    design->id_ = 0;
  } else {
    auto it = snlDesigns_.rbegin();
    SNLDesign* lastDesign = &(*it);
    SNLID::DesignID designID = lastDesign->id_+1;
    design->id_ = designID;
  }
  addSNLDesign(design);
}

void SNLLibrary::addSNLDesign(SNLDesign* design) {
  snlDesigns_.insert(*design);
  if (not design->isAnonymous()) {
    designNameIDMap_[design->getName()] = design->id_;
  }
}

void SNLLibrary::addPNLDesignAndSetID(PNLDesign* design) {
  if (pnlDesigns_.empty()) {
    design->id_ = 0;
  } else {
    auto it = pnlDesigns_.rbegin();
    PNLDesign* lastDesign = &(*it);
    SNLID::DesignID designID = lastDesign->id_+1;
    design->id_ = designID;
  }
  addPNLDesign(design);
}

void SNLLibrary::addPNLDesign(PNLDesign* design) {
  pnlDesigns_.insert(*design);
  //if (not design->isAnonymous()) {
    pnlDesignNameIDMap_[design->getName()] = design->id_;
  //}
}

void SNLLibrary::removeSNLDesign(SNLDesign* design) {
  if (not design->isAnonymous()) {
    designNameIDMap_.erase(design->getName());
  }
  snlDesigns_.erase(*design);
}

void SNLLibrary::removePNLDesign(PNLDesign* design) {
  //if (not design->isAnonymous()) {
    pnlDesignNameIDMap_.erase(design->getName());
  //}
  pnlDesigns_.erase(*design);
}

bool SNLLibrary::deepCompare(const SNLLibrary* other, std::string& reason) const {
  if (getID() not_eq other->getID()) {
    return false;
  }
  if (name_ not_eq other->getName()) {
    return false;
  }
  if (type_ not_eq other->getType()) {
    return false;
  }
  DEEP_COMPARE_MEMBER(SNLDesigns)
  DEEP_COMPARE_MEMBER(PNLDesigns)
  return true;
}

void SNLLibrary::mergeAssigns() {
  if (isPrimitives()) {
    return;
  }
  for (auto design: getSNLDesigns()) {
    if (not design->isPrimitive()) {
      design->mergeAssigns();
    }
  }
}

//LCOV_EXCL_START
const char* SNLLibrary::getTypeName() const {
  return "SNLLibrary";
}
//LCOV_EXCL_STOP

//LCOV_EXCL_START
std::string SNLLibrary::getString() const {
  return getName().getString();
}
//LCOV_EXCL_STOP

//LCOV_EXCL_START
std::string SNLLibrary::getDescription() const {
  std::ostringstream description;
  description << "<" + std::string(getTypeName());
  if (not isAnonymous()) {
    description << " " + name_.getString();
  }
  if (isPrimitives()) {
    description << " (prim)";
  }
  description << " " << getID();
  description << ">";  
  return description.str();
}
//LCOV_EXCL_STOP

//LCOV_EXCL_START
void SNLLibrary::debugDump(size_t indent, bool recursive, std::ostream& stream) const {
  stream << std::string(indent, ' ') << getDescription() << std::endl;
  if (recursive) {
    for (auto design: getSNLDesigns()) {
      design->debugDump(indent+2, recursive, stream);
    }
  }
}
//LCOV_EXCL_STOP

SNLID SNLLibrary::getSNLID() const {
  return SNLID(getDB()->getID(), getID());
}

}} // namespace SNL // namespace naja
