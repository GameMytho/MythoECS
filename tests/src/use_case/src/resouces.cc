#include <gtest/gtest.h>
#include <ecs/ecs.hpp>
#include <random>

using namespace mecs;

namespace rbo {
    struct Time {
        unsigned int seconds;
    };

    struct KeyBoard {
        unsigned int key;
    };

    void time_and_keyboard_change(ResMut<Time, KeyBoard> r) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(11, 100);

        auto [time, keyboard] = r;

        using time_type = decltype(time);
        EXPECT_EQ((std::is_same_v<time_type, DataWrapper<Time>>), true);
        EXPECT_EQ(time->seconds, 10);
        time->seconds = dis(gen);

        using keyboard_type = decltype(keyboard);
        EXPECT_EQ((std::is_same_v<keyboard_type, DataWrapper<KeyBoard>>), true);
        EXPECT_EQ(keyboard->key, 5);

        keyboard->key = dis(gen);
    }

    void time_and_keyboard_restore(ResMut<Time, KeyBoard> r) {
        auto [time, keyboard] = r;

        using time_type = decltype(time);
        EXPECT_EQ((std::is_same_v<time_type, DataWrapper<Time>>), true);
        EXPECT_NE(time->seconds, 10);
        time->seconds = 10;

        using keyboard_type = decltype(keyboard);
        EXPECT_EQ((std::is_same_v<keyboard_type, DataWrapper<KeyBoard>>), true);
        EXPECT_NE(keyboard->key, 5);
        keyboard->key = 5;
    }

    void exit(Commands cmds, Res<Time, KeyBoard> r) {
        auto [time, keyboard] = r;

        if (time->seconds > 91 && keyboard->key > 91) {
            cmds.registry().exit();
        }
    }
}

TEST(ResourcesTest, BasicOperation) {
    Registry reg;

    reg.init_resource<rbo::Time>(10u)
       .init_resource<rbo::KeyBoard>(5u)
       .add_system(rbo::time_and_keyboard_change)
       .add_system(system(rbo::exit).after(rbo::time_and_keyboard_change))
       .add_system(system(rbo::time_and_keyboard_restore).after(rbo::exit))
       .run();
}