// SPDX-FileCopyrightText: 2024 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __PNL_DESIGN_H_
#define __PNL_DESIGN_H_

#include "SNLObject.h"
#include "SNLID.h"
#include "PNLPoint.h"
#include "PNLTerm.h"
#include "SNLName.h"
#include "PNLNet.h"
#include "PNLBox.h"
#include "PNLInstance.h"
#include "NajaCollection.h"

namespace naja { namespace SNL {
  class SNLDB;
  class SNLLibrary;
}}

namespace naja { namespace PNL {

class PNLDesign final: public naja::SNL::SNLObject {

  public:
    friend class naja::SNL::SNLLibrary;
    friend class PNLInstance;
    
    using super = naja::SNL::SNLObject;
    using PNLDesignSlaveInstancesHook =
      boost::intrusive::member_hook<PNLInstance, boost::intrusive::set_member_hook<>, &PNLInstance::designSlaveInstancesHook_>;
    using PNLDesignSlaveInstances = boost::intrusive::set<PNLInstance, PNLDesignSlaveInstancesHook>;
    
    static PNLDesign* create(naja::SNL::SNLLibrary* library, const naja::SNL::SNLName& name);

    ///\return owning SNLDB
    naja::SNL::SNLDB* getDB() const;
    /// \return owning SNLLibrary.
    naja::SNL::SNLLibrary* getLibrary() const { return library_; }

    naja::SNL::SNLID::DesignID getID() const { return id_; }
    naja::SNL::SNLID getSNLID() const;

    friend bool operator< (const PNLDesign& ld, const PNLDesign& rd) {
      return ld.getSNLID() < rd.getSNLID();
    }

    const char* getTypeName() const override;
    std::string getString() const override;
    std::string getDescription() const override;
    bool deepCompare(const PNLDesign* other, std::string& reason) const;
    void debugDump(size_t indent, bool recursive=true, std::ostream& stream=std::cerr) const override;

    void addTerm(PNLTerm* term);
    void detachTerm(naja::SNL::SNLID::DesignObjectID id);

    void getPNLNet(const naja::SNL::SNLName& name) const;

    void addInstance(PNLInstance* instance) { instances_.push_back(instance); }
    PNLInstance* getInstance(naja::SNL::SNLID::DesignObjectID id) const;
    void detachInstance(naja::SNL::SNLID::DesignObjectID id);

    void addSlaveInstance(PNLInstance* instance) { slaveInstances_.insert(*instance); }
    void detacSlaveInstance(PNLInstance* instance) { slaveInstances_.erase(*instance); }

    void addNet(PNLNet* net);
    PNLNet* addNet(const naja::SNL::SNLName& name);
    naja::PNL::PNLNet* getNet(naja::SNL::SNLID::DesignObjectID id) const;
    naja::PNL::PNLNet* getNet(const naja::SNL::SNLName& name) const;

    naja::PNL::PNLTerm* addTerm(const naja::SNL::SNLName& name);
    naja::PNL::PNLTerm* getTerm(naja::SNL::SNLID::DesignObjectID id) const;
    naja::PNL::PNLTerm* getTerm(naja::SNL::SNLName name) const;

    void detachNet(naja::SNL::SNLID::DesignObjectID id);
    naja::SNL::SNLName getName() const { return name_; }
    void setName(const naja::SNL::SNLName& name);
    bool isAnonymous() const { return name_.empty(); }
    const auto& getNets() const { return nets_; }
    const auto& getTerms() const { return terms_; }
    void setAbutmentBox(const PNLBox& abutmentBox);
    const PNLBox& getAbutmentBox() const { return abutmentBox_; }

    const std::vector<PNLInstance*>& getInstances() const { return instances_; }

    NajaCollection<PNLInstance*> getSlaveInstances() const {
      return NajaCollection(new NajaIntrusiveSetCollection(&slaveInstances_));
    }

    void setTerminalNetlist(bool terminalNetlist) { terminalNetlist_ = terminalNetlist; }

  private:
    void _fit(const PNLBox& box);
    void _unfit(const PNLBox& box);
    PNLDesign(naja::SNL::SNLLibrary* library, const naja::SNL::SNLName& name);
    static void preCreate(const naja::SNL::SNLLibrary* library);
    void postCreateAndSetID();
    void postCreate();
    void preDestroy() override;
    void _setAbutmentBox(const PNLBox& abutmentBox);

    naja::SNL::SNLID::DesignID                id_;
    naja::SNL::SNLLibrary*                    library_            {};
    naja::PNL::PNLPoint                       origin_             {0, 0};
    boost::intrusive::set_member_hook<>       libraryDesignsHook_ {};
    std::vector<PNLTerm*>                     terms_              {};
    std::vector<PNLInstance*>                 instances_          {};
    std::vector<PNLNet*>                      nets_               {};
    naja::SNL::SNLName                        name_               {};
    PNLBox                                    abutmentBox_;
    PNLBox                                    boundingBox_;
    PNLDesignSlaveInstances                   slaveInstances_     {};
    bool                                      terminalNetlist_    {false};
};

}} // namespace PNL // namespace naja

#endif // __PNL_DESIGN_H_