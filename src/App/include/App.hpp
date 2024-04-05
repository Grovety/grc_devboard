#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/semphr.h"

#include <memory>
#include <vector>

#include "Event.hpp"
#include "State.hpp"
#include "Common.hpp"

#include "BaseGrc.hpp"
#include "GrcHelper.hpp"
#include "IDisplay.hpp"
#include "IStorage.hpp"
#include "Listener.hpp"
#include "Status.hpp"
#include "VoiceMsgPlayer.hpp"

#define SUSPEND_AFTER_MS unsigned(1000) * 60 * 2

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
  /*! \brief Pointer to sensor. */
  std::unique_ptr<ISensor> p_sensor;
  /*! \brief Pointer to led. */
  std::unique_ptr<ILed> p_led;

private:
  /*!
   * \brief State transition impl.
   * \param target_state Pointer to next state.
   */
  void do_transition(State *target_state);

private:
  /*! \brief Queue of transitions. */
  QueueHandle_t transition_queue_;
  /*! \brief Current state of the application. */
  State* current_state_;
  /*! \brief Nested contexts of the application. */
  std::vector<const char *> context_;
};

using action_t = void (*)(App *app);

namespace Common {
  /*!
  * \brief State to select and transition to another state.
  */
  struct Menu : State {
    struct Item {
      const char *name;
      action_t action;
    };
    Menu(std::initializer_list<Item> item_list, Item back, VoiceMsgId v_msg_id = 0)
        : items_(item_list), current_item_(0), back_(back), v_msg_id_(v_msg_id) {}
    virtual ~Menu() = default;

    void enterAction(App *app) override;
    void handleEvent(App *app, eEvent ev) override;
    State* clone() override;

  protected:
    virtual void drawMenuItems(App *app) const;
    virtual void drawMenu(App *app) const;
    unsigned getCurrentItem() const { return current_item_ % items_.size(); }

    const std::vector<Item> items_;
    unsigned current_item_;
    Item back_;
    VoiceMsgId v_msg_id_;
  };

  /*!
  * \brief Yes/No dialog screen.
  */
  struct ConfirmDialog : Menu {
    using Menu::Menu;
    ConfirmDialog(action_t yes, action_t no, VoiceMsgId v_msg_id = 0) 
      : Menu({{"Y", yes}, {"N", no}}, {"Back", no}, v_msg_id) {}
    void drawMenuItems(App *app) const override;
    void handleEvent(App *app, eEvent ev) override;
    State* clone() override;
  };

  struct ReturnableState : State {
    State* clone() override {
      return new ReturnableState(*this);
    }
    ReturnableState(State *back_state) : back_state_(back_state) {}

  protected:
    State *back_state_;
  };

  /*!
  * \brief Enter this state after no user actions.
  */
  struct Suspended : ReturnableState {
    using ReturnableState::ReturnableState;
    void enterAction(App *app) override final;
    void exitAction(App *app) override final;
    void handleEvent(App *app, eEvent ev) override final;
  };

  /*!
   * \brief Get pointer to MainMenu.
   * \return State pointer.
   */
  State* getMainMenu();
  /*!
   * \brief Initialize Gesture Scenario.
   * \param app Pointer to app.
   */
  void initGestureScenario(App *app);
  /*!
   * \brief Initialize Rhythm Scenario.
   * \param app Pointer to app.
   */
  void initRhythmScenario(App *app);
  /*!
   * \brief Initialize Vibro Scenario.
   * \param app Pointer to app.
   */
  void initVibroScenario(App *app);
  /*!
   * \brief Initialize VoicePINcode Scenario.
   * \param app Pointer to app.
   */
  void initVoicePINcodeScenario(App *app);
} // namespace Common
