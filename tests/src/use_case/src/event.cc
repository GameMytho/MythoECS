#include <gtest/gtest.h>
#include <ecs/ecs.hpp>
#include <random>

using namespace mecs;

namespace ebo {
    struct Damage {
        float value;
    };

    struct AppExit {
        bool exit;
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

    void damage_event_receive(EventReader<Damage> er, EventWriter<AppExit> ew) {
        const auto& events = er.read();

        for (const auto& event : events) {
            EXPECT_EQ(event.value, 0.5f);
        }

        if (events.size() > 9) {
            ew.write(AppExit { true });
        }
    }

    void stop(Commands cmds, EventReader<AppExit> er) {
        const auto& events = er.read();

        for (const auto& event : events) {
            EXPECT_EQ(event.exit, true);
            if (event.exit) {
                cmds.registry().stop();
            }
        }
    }
}

TEST(EventTest, BasicOperation) {
    Registry reg;

    reg.init_event<ebo::Damage>()
       .init_event<ebo::AppExit>()
       .add_system(ebo::damage_event_send)
       .add_system(system(ebo::damage_event_adjust).after(ebo::damage_event_send))
       .add_system(system(ebo::damage_event_receive).after(ebo::damage_event_adjust))
       .add_system(system(ebo::stop).after(ebo::damage_event_receive))
       .run();
}