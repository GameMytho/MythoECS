#include <gtest/gtest.h>
#include "ecs/ecs.hpp"

struct Frame {
    unsigned int value;
};

struct Damage {
    float value;
};

void update1(event_writer<Damage> e) {
    e.write(Damage { 10.0f });
}

void update2(event_mutator<Damage> e, res<Frame> r) {
    auto [frame] = r;

    if (frame->value == 1) {
        EXPECT_EQ(e.mutate().size(), 1);

        for (auto& d : e.mutate()) {
            d.value *= 0.5f;
        }
    } else {
        EXPECT_EQ(e.mutate().size(), 0);
    }
}

void update3(event_reader<Damage> e, res<Frame> r) {
    auto [frame] = r;

    if (frame->value == 1) {
        EXPECT_EQ(e.read().size(), 1);

        for (const auto& d : e.read()) {
            EXPECT_EQ(d.value, 5.0f);
        }
    } else {
        EXPECT_EQ(e.read().size(), 0);
    }
}

void update_frame(res_mut<Frame> r) {
    auto [frame] = r;

    frame->value += 1u;
}

TEST(SystemTest, EventTest) {
    registry reg;

    reg.init_resource<Frame>(0u)
       .init_event<Damage>()
       .add_update_system(update1)
       .add_update_system(system(update2).after(update1))
       .add_update_system(system(update3).after(update2))
       .add_update_system(system(update_frame).after(update3));

    reg.ready();

    reg.startup();

    reg.update();

    reg.update();
}