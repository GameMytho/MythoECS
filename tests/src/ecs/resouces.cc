#include <gtest/gtest.h>
#include "ecs/registry.hpp"

struct Time {
    unsigned int seconds;
};

struct KeyBoard {
    unsigned int key;
};

struct Log {
    unsigned int level;
};

using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
using registry = mytho::ecs::basic_registry<entity, uint8_t, 1024>;

template<typename... Ts>
using res = mytho::ecs::basic_resources<Ts...>;

template<typename... Ts>
using res_mut = mytho::ecs::basic_resources_mut<Ts...>;

void update1(res<Time> r1, res_mut<KeyBoard> r2) {
    auto [time] = r1;
    using time_type = decltype(time);
    EXPECT_EQ((std::is_same_v<time_type, const Time&>), true);
    EXPECT_EQ(time.seconds, 10);

    auto [keyboard] = r2;
    using keyboard_type = decltype(keyboard);
    EXPECT_EQ((std::is_same_v<keyboard_type, KeyBoard&>), true);
    EXPECT_EQ(keyboard.key, 5);

    keyboard.key = 16;
}

void update2(res_mut<Time, KeyBoard> r) {
    auto [time, keyboard] = r;

    using time_type = decltype(time);
    EXPECT_EQ((std::is_same_v<time_type, Time&>), true);
    EXPECT_EQ(time.seconds, 10);

    using keyboard_type = decltype(keyboard);
    EXPECT_EQ((std::is_same_v<keyboard_type, KeyBoard&>), true);
    EXPECT_EQ(keyboard.key, 16);

    time.seconds = 31;
    keyboard.key = 45;
}

void update3(res<Time, KeyBoard> r) {
    auto [time, keyboard] = r;

    using time_type = decltype(time);
    EXPECT_EQ((std::is_same_v<time_type, const Time&>), true);
    EXPECT_EQ(time.seconds, 31);

    using keyboard_type = decltype(keyboard);
    EXPECT_EQ((std::is_same_v<keyboard_type, const KeyBoard&>), true);
    EXPECT_EQ(keyboard.key, 45);
}

TEST(ResourcesTest, AddAndRemoveTest) {
    registry reg;

    reg.init_resource<Time>(10u)
       .init_resource<KeyBoard>(5u)
       .add_update_system(update1)
       .add_update_system(update2)
       .add_update_system(update3);

    reg.startup();

    reg.update();

    reg.shutdown();
}