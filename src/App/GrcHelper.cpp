#include "GrcHelper.hpp"

static constexpr char TAG[] = "GrcHelper";

int grc_helper_infer(BaseGrc *p_grc, Matrix &signal) {
  return p_grc->inference(signal);
}

int grc_helper_train(BaseGrc *p_grc, Matrix &signal, int train_category) {
  const unsigned prev_qty = p_grc->getQty();
  int ret = -1;
  while ((ret = p_grc->train(signal, train_category)) < 0) {
    LOGD(TAG, "train error: %d", ret);
    vTaskDelay(pdMS_TO_TICKS(1000));
    if (p_grc->getQty() > prev_qty) {
      break;
    }
    LOGE(TAG, "train retry");
  }
  return ret;
}

unsigned grc_helper_load_wgts(BaseGrc *p_grc, IStorage *stor) {
  std::vector<float> buffer;
  unsigned cats = 0;
  stor->read(p_grc->getName(), cats, buffer);
  if (!p_grc->load(cats, buffer)) {
    LOGE(TAG, "Error loading weights to GRC");
  }
  return cats;
}

unsigned grc_helper_save_wgts(BaseGrc *p_grc, IStorage *stor) {
  std::vector<float> buffer;
  unsigned cats = p_grc->save(buffer);
  stor->write(p_grc->getName(), cats, buffer);
  return cats;
}

void grc_helper_clear_wgts(BaseGrc *p_grc, IStorage *stor) {
  stor->write(p_grc->getName(), 0, {});
  p_grc->clear();
}
