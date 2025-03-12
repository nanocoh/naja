// SPDX-FileCopyrightText: 2024 The Naja authors
// <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include "PNLInstance.h"
#include "PNLDesign.h"
#include "SNLDesignObject.h"

namespace naja {
namespace PNL {

PNLInstance::PNLInstance(PNLDesign* design,
                         PNLDesign* model,
                         naja::SNL::SNLID::DesignObjectID id,
                         const PNLPoint& origin,
                         const PNLTransform& transform)
    : super(),
      design_(design),
      model_(model),
      origin_(origin),
      transform_(transform),
      id_(id) {}

PNLInstance* PNLInstance::create(PNLDesign* design, PNLDesign* model,
                         const PNLPoint& origin,
                         const PNLTransform& transform) {
  preCreate(design, model);
  PNLInstance* instance = new PNLInstance(design, model, (unsigned int) design->getInstances().size(), origin, transform);
  design->addInstance(instance);
  model->addSlaveInstance(instance);
  instance->postCreate();
  return instance;
}

void PNLInstance::preCreate(PNLDesign* design, const PNLDesign* model) {
  super::preCreate();
}

void PNLInstance::postCreate() {
  super::postCreate();
}

void PNLInstance::debugDump(size_t indent,
                            bool recursive,
                            std::ostream& stream) const {}

naja::SNL::SNLID PNLInstance::getSNLID() const {
  return PNLDesignObject::getSNLID(naja::SNL::SNLID::Type::Instance, 0, id_, 0);
}

}  // namespace PNL
}  // namespace naja