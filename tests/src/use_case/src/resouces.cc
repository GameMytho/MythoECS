#include <gtest/gtest.h>
#include <ecs/ecs.hpp>

using namespace mecs;

namespace rbo {
    struct Time {
        unsigned int seconds;
    };

    struct KeyBoard {
        unsigned int key;
    };

    void time_and_keyboard_change(ResMut<Time, KeyBoard> r) {
        auto [time, keyboard] = r;

        using time_type = decltype(time);
        EXPECT_EQ((std::is_same_v<time_type, DataWrapper<Time>>), true);
        EXPECT_EQ(time->seconds, 10);
        time->seconds = 20;

        using keyboard_type = decltype(keyboard);
        EXPECT_EQ((std::is_same_v<keyboard_type, DataWrapper<KeyBoard>>), true);
        EXPECT_EQ(keyboard->key, 5);

        keyboard->key = 16;
    }

    void time_and_keyboard_check(Res<Time, KeyBoard> r) {
        auto [time, keyboard] = r;

        using time_type = decltype(time);
        EXPECT_EQ((std::is_same_v<time_type, const DataWrapper<Time>>), true);
        EXPECT_EQ(time->seconds, 20);

        using keyboard_type = decltype(keyboard);
        EXPECT_EQ((std::is_same_v<keyboard_type, const DataWrapper<KeyBoard>>), true);
        EXPECT_EQ(keyboard->key, 16);
    }

    void time_and_keyboard_restore(ResMut<Time, KeyBoard> r) {
        auto [time, keyboard] = r;

        using time_type = decltype(time);
        EXPECT_EQ((std::is_same_v<time_type, DataWrapper<Time>>), true);
        EXPECT_EQ(time->seconds, 20);
        time->seconds = 10;

        using keyboard_type = decltype(keyboard);
        EXPECT_EQ((std::is_same_v<keyboard_type, DataWrapper<KeyBoard>>), true);
        EXPECT_EQ(keyboard->key, 16);
        keyboard->key = 5;
    }
}

TEST(ResourcesTest, BasicOperation) {
    Registry reg;

    reg.init_resource<rbo::Time>(10u)
       .init_resource<rbo::KeyBoard>(5u)
       .add_update_system(rbo::time_and_keyboard_change)
       .add_update_system(system(rbo::time_and_keyboard_check).after(rbo::time_and_keyboard_change))
       .add_update_system(system(rbo::time_and_keyboard_restore).after(rbo::time_and_keyboard_check))
       .ready();

    reg.startup();

    for (auto i = 0; i < 100; ++i) {
        reg.update();
    }
}