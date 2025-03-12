// SPDX-FileCopyrightText: 2024 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __PNL_INSTANCE_H_
#define __PNL_INSTANCE_H_

#include "PNLDesignObject.h"
#include "PNLTransform.h"
#include "PNLPoint.h"

namespace naja { namespace PNL {

class PNLInstance final: public PNLDesignObject {
  public:
    using super = PNLDesignObject;
    friend class PNLDesign;

    static PNLInstance* create(PNLDesign* design, PNLDesign* model,
                         const PNLPoint& origin,
                         const PNLTransform& transform);

    PNLDesign* getDesign() const override { return design_; }
    PNLDesign* getModel() const { return model_; }

    const char* getTypeName() const override { return "PNLInstance"; }
    std::string getString() const override { return "PNLInstance"; }
    std::string getDescription() const override { return "PNLInstance"; }
    void debugDump(size_t indent, bool recursive=true, std::ostream& stream=std::cerr) const override;
    naja::SNL::SNLID getSNLID() const override;

    PNLTransform getTransform() const { return transform_; }
    
  private:
    PNLInstance(PNLDesign* design,
                         PNLDesign* model,
                         naja::SNL::SNLID::DesignObjectID id,
                         const PNLPoint& origin,
                         const PNLTransform& transform);
  
    static void preCreate(PNLDesign* design, const PNLDesign* model);
    void postCreate();
  
    PNLDesign*    design_;
    PNLDesign*    model_;
    naja::PNL::PNLPoint      origin_{0, 0};
    naja::PNL::PNLTransform  transform_;
    boost::intrusive::set_member_hook<> designInstancesHook_      {};
    boost::intrusive::set_member_hook<> designSlaveInstancesHook_ {};
    naja::SNL::SNLID::DesignObjectID id_;
};

}} // namespace PNL // namespace naja

#endif // __PNL_INSTANCE_H_