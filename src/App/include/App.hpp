#pragma once
#include <vector>

#include "Event.hpp"
#include "State.hpp"
#include "Common.hpp"

#include "BaseGrc.hpp"
#include "GrcController.hpp"
#include "IDisplay.hpp"
#include "IStorage.hpp"
#include "Listener.hpp"
#include "Status.hpp"

#define SUSPEND_AFTER_MS unsigned(1000) * 60 * 2

struct StateInfo {
  State *ptr = nullptr;
  unsigned enter_time = 0;
};

/*!
 * \brief Simple fsm.
 */
class App {
public:
  /*!
   * \brief Constructor.
   */
  App();
  /*!
   * \brief Start processing events.
   */
  void run();
  /*!
   * \brief State transition.
   * \param target_state Pointer to next state.
   */
  void transition(State *target_state);
  /*!
   * \brief Check current state timeout.
   * \param timeout Timeout ms.
   */
  bool check_timeout(size_t timeout) const;
  /*! \brief Remove context from stack. */
  void pop_ctx() {
    if (context_.size() > 1)
      context_.pop_back();
  }
  /*!
   * \brief Push context to stack.
   * \param ctx String representing next context.
   */
  void set_ctx(const char *ctx) { context_.push_back(ctx); }
  /*!
   * \brief Get current context.
   * \return Current context.
   */
  const char *get_ctx() const { return context_.back(); }

  /*! \brief Pointer to storage. */
  std::unique_ptr<IStorage> p_storage;
  /*! \brief Pointer to display. */
  std::unique_ptr<IDisplay> p_display;

private:
  /*! \brief Current state of the application. */
  StateInfo current_state_;
  /*! \brief Nested contexts of the application. */
  std::vector<const char *> context_;
};

namespace Common {
/*! \brief Static app states. */
struct States {
  static State &init;
  static State &init_gesture_scenario;
  static State &init_rhythm_scenario;
  static State &init_vibro_scenario;
  static State &select_scenario_menu;
};

/*!
 * \brief State to select and transition to another state.
 */
struct Menu : State {
  struct Item {
    const char *name;
    State *target_state;
  };

  Menu(std::initializer_list<Item> item_list, State *back_state)
      : items_(item_list), current_item_(0), back_state_(back_state) {}
  void enterAction(App *app) override;
  void exitAction(App *app) override;
  void handleEvent(App *app, eEvent ev) override;

protected:
  virtual void drawMenuItems(App *app) const;
  virtual void drawMenu(App *app) const;
  unsigned getCurrentItem() const;

  const std::vector<Item> items_;
  unsigned current_item_;
  State *back_state_;
};

/*!
 * \brief Yes/No dialog screen.
 */
struct ConfirmDialog : Menu {
  ConfirmDialog(State *yes, State *no) : Menu({{"Y", yes}, {"N", no}}, no) {}
  void drawMenuItems(App *app) const override;
  void handleEvent(App *app, eEvent ev) override;
};

/*!
 * \brief Return to state, leave context.
 */
struct BackTo : State {
  BackTo(State *state) : target_state_(state) {}
  void update(App *app) override final;

protected:
  State *target_state_;
};

/*!
 * \brief Common state to listen to sensor data.
 */
struct Listen : State {
  Listen(State *forward_state, State *back_state)
      : forward_state_(forward_state), back_state_(back_state) {}
  virtual void updateDisplay(App *app) = 0;
  void enterAction(App *app) override;
  void exitAction(App *app) override;
  void handleEvent(App *app, eEvent ev) override = 0;

protected:
  /*! \brief Next state on Start event. */
  State *forward_state_;
  /*! \brief Return state. */
  State *back_state_;
};

/*!
 * \brief Common state to init scenario.
 */
struct InitScenario : State {
  /*! \brief Pointer to Grc application. */
  std::unique_ptr<BaseGrc> p_grc;
};

struct InitGestureScenario : InitScenario {
  void enterAction(App *app) override final;
  void update(App *app) override final;
};

struct InitRhythmScenario : InitScenario {
  void enterAction(App *app) override final;
  void update(App *app) override final;
};

struct InitVibroScenario : InitScenario {
  void enterAction(App *app) override final;
  void update(App *app) override final;
};

/*!
 * \brief Enter this state after no user actions.
 */
struct Suspended : State {
  Suspended(State *back_state) : back_state_(back_state) {}
  void enterAction(App *app) override final;
  void exitAction(App *app) override final;
  void handleEvent(App *app, eEvent ev) override final;

protected:
  State *back_state_;
};

/*!
 * \brief Get GrcReply_t data.
 * \param id Reply id.
 * \return Reply data.
 */
int process_grc_replies(eGrcReplyId id);
} // namespace Common
