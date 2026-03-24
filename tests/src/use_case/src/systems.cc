#include <gtest/gtest.h>
#include <ecs/ecs.hpp>
#include <cstdio>
#include <random>

using namespace mecs;

namespace sbo {
    struct Player {};

    struct Stamina {
        float value;
    };

    struct RageMode {
        bool value;
    };

    struct Exhausted {
        bool value;
    };

    struct Alive {
        bool value;
    };

    bool player_alive(Res<Alive> r) {
        auto [alive] = r;

        return alive->value;
    }

    bool has_stamina(Res<Stamina> r) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(25, 40);

        auto [stamina] = r;

        return stamina->value > dis(gen);
    }

    bool is_rage_mode(Res<RageMode> r) {
        auto [rage] = r;

        return rage->value;
    }

    bool exhausted(Res<Exhausted> r) {
        auto [exhausted] = r;

        return exhausted->value;
    }

    void attack_system(Commands cmds, Res<Alive, Stamina, RageMode, Exhausted> r) {
        // do attack
        // ...

        // check
        auto [alive, stamina, rage, exhausted] = r;
        EXPECT_TRUE(alive->value);
        EXPECT_EQ(stamina->value, 30.0f);
        EXPECT_FALSE(rage->value);
        EXPECT_FALSE(exhausted->value);

        // end
        cmds.registry().exit();
    }
}

TEST(SystemTest, BasicOperation) {
    Registry reg;

    reg.init_resource<sbo::Alive>(true);
    reg.init_resource<sbo::Stamina>(30.0f);
    reg.init_resource<sbo::RageMode>(false);
    reg.init_resource<sbo::Exhausted>(false);
    reg.add_system<MainSchedules::Update>(
            system(sbo::attack_system).runif(
                all_of<
                    sbo::player_alive,
                    any_of<sbo::is_rage_mode, sbo::has_stamina>(),
                    not_of<sbo::exhausted>()
                >()
            )
        )
       .run();
}