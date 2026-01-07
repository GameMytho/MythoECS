#include <gtest/gtest.h>
#include <ecs/ecs.hpp>
#include <random>

using namespace mecs;

namespace ebo {
    struct Damage {
        float value;
    };

    void damage_event_send(EventWriter<Damage> ew) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 10);

        auto count = dis(gen);
        for (auto i = 0; i < count; ++i) {
            ew.write(Damage { 1.0f });
        }
    }

    void damage_event_adjust(EventMutator<Damage> em) {
        auto& events = em.mutate();

        for (auto& event : events) {
            EXPECT_EQ(event.value, 1.0f);
            event.value *= 0.5f;
        }
    }

    void damage_event_receive(EventReader<Damage> er) {
        const auto& events = er.read();

        for (const auto& event : events) {
            EXPECT_EQ(event.value, 0.5f);
        }
    }
}

TEST(EventTest, BasicOperation) {
    Registry reg;

    reg.init_event<ebo::Damage>()
       .add_update_system(ebo::damage_event_send)
       .add_update_system(system(ebo::damage_event_adjust).after(ebo::damage_event_send))
       .add_update_system(system(ebo::damage_event_receive).after(ebo::damage_event_adjust))
       .ready();

    reg.startup();

    for (auto i = 0; i < 100; ++i) {
        reg.update();
    }
}