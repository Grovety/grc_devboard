#include "App.hpp"
#include "utils.hpp"

extern "C" void app_main(void) {
  c3_reset();

  App app;
  app.run();
}
