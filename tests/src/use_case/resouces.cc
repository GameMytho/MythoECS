#include <gtest/gtest.h>
#include "ecs/ecs.hpp"

struct Time {
    unsigned int seconds;
};

struct KeyBoard {
    unsigned int key;
};

struct Log {
    unsigned int level;
};

void update1(res<Time> r1, res_mut<KeyBoard> r2) {
    auto [time] = r1;
    using time_type = decltype(time);
    EXPECT_EQ((std::is_same_v<time_type, const data_wrapper<Time>>), true);
    EXPECT_EQ(time->seconds, 10);

    auto [keyboard] = r2;
    using keyboard_type = decltype(keyboard);
    EXPECT_EQ((std::is_same_v<keyboard_type, data_wrapper<KeyBoard>>), true);
    EXPECT_EQ(keyboard->key, 5);

    keyboard->key = 16;
}

void update2(res_mut<Time, KeyBoard> r) {
    auto [time, keyboard] = r;

    using time_type = decltype(time);
    EXPECT_EQ((std::is_same_v<time_type, data_wrapper<Time>>), true);
    EXPECT_EQ(time->seconds, 10);

    using keyboard_type = decltype(keyboard);
    EXPECT_EQ((std::is_same_v<keyboard_type, data_wrapper<KeyBoard>>), true);
    EXPECT_EQ(keyboard->key, 16);

    time->seconds = 31;
    keyboard->key = 45;
}

void update3(res<Time, KeyBoard> r) {
    auto [time, keyboard] = r;

    using time_type = decltype(time);
    EXPECT_EQ((std::is_same_v<time_type, const data_wrapper<Time>>), true);
    EXPECT_EQ(time->seconds, 31);

    using keyboard_type = decltype(keyboard);
    EXPECT_EQ((std::is_same_v<keyboard_type, const data_wrapper<KeyBoard>>), true);
    EXPECT_EQ(keyboard->key, 45);
}

TEST(ResourcesTest, AddAndRemoveTest) {
    registry reg;

    reg.init_resource<Time>(10u)
       .init_resource<KeyBoard>(5u)
       .add_update_system(update1)
       .add_update_system(system(update2).after(update1).runif(resources_exist<Time, KeyBoard>))
       .add_update_system(system(update3).after(update2).runif(resources_exist<Time, KeyBoard>));

    reg.ready();

    reg.startup();

    reg.update();

    reg.shutdown();
}