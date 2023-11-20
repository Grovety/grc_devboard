#pragma once
#include "Event.hpp"

class App;

/*!
 * \brief App state interface.
 */
struct State {
  virtual ~State() = default;
  virtual void enterAction(App *) {}
  virtual void exitAction(App *) {}
  /*! \brief Called periodically on event. */
  virtual void update(App *) {}
  virtual void handleEvent(App *, eEvent) {}
};
