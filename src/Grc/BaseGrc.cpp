#include "BaseGrc.hpp"

BaseGrc::BaseGrc(const char *name) : name_(name) {}

BaseGrc::~BaseGrc() { clearReq(); }

int BaseGrc::init(HP hp) {
  hyper_params_ = hp;
  return initReq(hp);
}

int BaseGrc::clear() { return clearReq(); }

unsigned BaseGrc::save(std::vector<RT> &data) {
  LOGD(name_, __FUNCTION__);
  return saveTrainDataReq(data);
}

bool BaseGrc::load(unsigned qty, const std::vector<RT> &data) {
  LOGD(name_, __FUNCTION__);
  return loadTrainDataReq(qty, data.size(), data.data());
}

unsigned BaseGrc::getQty() const { return getQtyReq(); }

const char *BaseGrc::getName() const { return name_; }

HP BaseGrc::getHyperParams() const { return hyper_params_; }
