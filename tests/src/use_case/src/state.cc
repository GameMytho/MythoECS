#include <gtest/gtest.h>
#include <ecs/ecs.hpp>
#include <random>
#include <iostream>

using namespace mecs;

namespace sbo {
    enum class GameState : uint8_t {
        Menu,
        Playing,
        Pause
    };

    void menu_init(Res<NextState<GameState>> rs) {
        auto& [next_state] = rs;

        EXPECT_EQ(next_state->get(), GameState::Menu);
    }

    void menu_deinit(Res<State<GameState>> rs) {
        auto& [state] = rs;

        EXPECT_EQ(state->get(), GameState::Menu);
    }

    void playing_start(Res<NextState<GameState>> rs) {
        auto& [next_state] = rs;

        EXPECT_EQ(next_state->get(), GameState::Playing);
    }

    void playing_end(Res<State<GameState>> rs) {
        auto& [state] = rs;

        EXPECT_EQ(state->get(), GameState::Playing);
    }

    void pause_start(Res<NextState<GameState>> rs) {
        auto& [next_state] = rs;

        EXPECT_EQ(next_state->get(), GameState::Pause);
    }

    void pause_end(Res<State<GameState>> rs) {
        auto& [state] = rs;

        EXPECT_EQ(state->get(), GameState::Pause);
    }

    void game_state_switch(Commands cmds, ResMut<NextState<GameState>> rsm) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 20);

        auto& [next_state] = rsm;

        auto i = dis(gen);
        if (i < 5) {
            next_state->set(GameState::Menu);
        } else if (i < 15) {
            next_state->set(GameState::Playing);
        } else if (i < 20) {
            next_state->set(GameState::Pause);
        } else {
            cmds.registry().exit();
        }
    }
}

TEST(StateTest, BasicOperation) {
    Registry reg;

    reg.init_state<sbo::GameState>(sbo::GameState::Menu)
       .add_system(sbo::game_state_switch) // default schedule: MainSchedules::Update
       .add_system<OnEnter<sbo::GameState::Menu>>(sbo::menu_init)
       .add_system<OnExit<sbo::GameState::Menu>>(sbo::menu_deinit)
       .add_system<OnEnter<sbo::GameState::Playing>>(sbo::playing_start)
       .add_system<OnExit<sbo::GameState::Playing>>(sbo::playing_end)
       .add_system<OnEnter<sbo::GameState::Pause>>(sbo::pause_start)
       .add_system<OnExit<sbo::GameState::Pause>>(sbo::pause_end)
       .run();
}