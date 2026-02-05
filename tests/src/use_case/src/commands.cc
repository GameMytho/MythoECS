#include <gtest/gtest.h>
#include <ecs/ecs.hpp>
#include <random>

using namespace mecs;

/*-------------------------------------------------------------------- Test For Component Opertions API ------------------------------------------------------------------------------------*/

namespace cco {
    struct Position {
        float x;
        float y;
        float z;
    };

    struct Velocity {
        float x;
        float y;
        float z;
    };

    struct Direction {
        float x;
        float y;
        float z;
    };

    struct Name {
        std::string value;
    };

    void entity_spawn(Commands cmds) {
        cmds.spawn(
            Position{0.25f, 0.28f, 0.35f}
        );
    }

    void position_change(Commands cmds) {
        auto q = cmds.registry().query<Entity, Position, Without<Velocity, Direction>>();
        for (auto& [e, pos] : q) {
            EXPECT_TRUE(cmds.components_added<Position>());
            EXPECT_TRUE(cmds.components_changed<Position>());

            EXPECT_EQ(pos->x, 0.25f);
            EXPECT_EQ(pos->y, 0.28f);
            EXPECT_EQ(pos->z, 0.35f);

            cmds.replace(*e, Position{2.5f, 2.8f, 3.0f});
            cmds.insert(*e, Velocity{0.2f, 0.3f, 0.4f});
            cmds.insert(*e, Name{std::string("entity") + std::to_string(e->id())});
        }
    }

    void velocity_change(Commands cmds) {
        auto q = cmds.registry().query<Entity, Position, Velocity, Name, Without<Direction>>();
        for (auto& [e, pos, vel, name] : q) {
            EXPECT_TRUE(cmds.components_added<Position>());
            EXPECT_TRUE(cmds.components_changed<Position>());
            EXPECT_TRUE(cmds.components_added<Velocity>());
            EXPECT_TRUE(cmds.components_changed<Velocity>());
            EXPECT_TRUE(cmds.components_added<Name>());
            EXPECT_TRUE(cmds.components_changed<Name>());

            EXPECT_EQ(pos->x, 2.5f);
            EXPECT_EQ(pos->y, 2.8f);
            EXPECT_EQ(pos->z, 3.0f);

            EXPECT_EQ(vel->x, 0.2f);
            EXPECT_EQ(vel->y, 0.3f);
            EXPECT_EQ(vel->z, 0.4f);

            EXPECT_EQ(name->value, std::string("entity") + std::to_string(e->id()));

            cmds.replace(*e, Velocity{2.f, 3.f, 4.f});
            cmds.insert(*e, Direction{0.f, 0.f, 1.f});
            cmds.remove<Name>(*e);
        }
    }

    void final_check(Commands cmds) {
        auto q1 = cmds.registry().query<Entity, Position, Velocity, Direction>();
        for (auto& [e, pos, vel, dir] : q1) {
            EXPECT_TRUE(cmds.components_added<Position>());
            EXPECT_TRUE(cmds.components_changed<Position>());
            EXPECT_TRUE(cmds.components_added<Velocity>());
            EXPECT_TRUE(cmds.components_changed<Velocity>());
            EXPECT_TRUE(cmds.components_added<Direction>());
            EXPECT_TRUE(cmds.components_changed<Direction>());
            EXPECT_TRUE(cmds.components_removed<Name>());

            EXPECT_EQ(pos->x, 2.5f);
            EXPECT_EQ(pos->y, 2.8f);
            EXPECT_EQ(pos->z, 3.0f);

            EXPECT_EQ(vel->x, 2.f);
            EXPECT_EQ(vel->y, 3.f);
            EXPECT_EQ(vel->z, 4.f);

            EXPECT_EQ(dir->x, 0.f);
            EXPECT_EQ(dir->y, 0.f);
            EXPECT_EQ(dir->z, 1.f);
        }

        auto q2 = cmds.registry().query<Entity, With<Direction, Name>>();
        EXPECT_EQ(0, q2.size());
    }

    void stop(Commands cmds) {
        auto count = cmds.registry().count<Entity, With<cco::Position>>();
        if (count < 100) {
            cmds.registry().stop();
        }
    }
}

TEST(CommandsTest, ComponentOperation) {
    Registry reg;

    reg.add_system(cco::entity_spawn)
       .add_system(system(cco::position_change).after(cco::entity_spawn))
       .add_system(system(cco::velocity_change).after(cco::position_change))
       .add_system(system(cco::final_check).after(cco::velocity_change))
       .add_system(system(cco::stop).after(cco::final_check))
       .run();
}

/*-------------------------------------------------------------------- Test For Entity Opertions API ------------------------------------------------------------------------------------*/

namespace ceo {
    struct Name {
        std::string value;
    };

    void entity_spawn(Commands cmds) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 6);

        auto count = dis(gen);
        for (auto i = 0; i < count; ++i) {
            cmds.spawn(
                Name{"entity" + i}
            );
        }
    }

    void entity_despawn(Commands cmds) {
        auto results = cmds.registry().query<Entity>();
        auto size = results.size();

        if (size < 1) return;

        if (size < 100) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, size - 1);
            auto index = dis(gen);
            auto& [e] = *(results.begin() + index);
            EXPECT_EQ(cmds.registry().alive(*e), true);
            cmds.despawn(*e);
        } else {
            for (auto& [e] : results) {
                EXPECT_EQ(cmds.registry().alive(*e), true);
                cmds.despawn(*e);
            }
        }
    }

    void stop(Commands cmds) {
        auto count = cmds.registry().count<Entity, With<ceo::Name>>();
        if (count > 5) {
            cmds.registry().stop();
        }
    }
}

TEST(CommandsTest, EntityOperation) {
    Registry reg;

    reg.add_system(ceo::entity_spawn)
       .add_system(system(ceo::entity_despawn).after(ceo::entity_spawn))
       .add_system(system(ceo::stop).after(ceo::entity_despawn))
       .run();
}

/*-------------------------------------------------------------------- Test For Resource Opertions API ------------------------------------------------------------------------------------*/

namespace cro {
    struct Time {
        float last;
    };

    struct Frame {
        unsigned int value;
    };

    void time_init(Commands cmds) {
        if (cmds.resources_exist<Time>()) return;

        cmds.init_resource<Time>(0.f);
    }

    void frame_init(Commands cmds) {
        if (cmds.resources_exist<Frame>()) return;

        cmds.init_resource<Frame>(0u);
    }

    void time_update(Commands cmds) {
        if (!cmds.resources_exist<Time>()) return;

        EXPECT_TRUE(cmds.resources_added<Time>());
        EXPECT_TRUE(cmds.resources_changed<Time>());

        auto [time] = cmds.registry().resources_mut<Time>();

        time->last += 0.1f;
    }

    void frame_update(Commands cmds) {
        if (!cmds.resources_exist<Frame>()) return;

        auto [frame] = cmds.registry().resources_mut<Frame>();

        if (frame->value == 0) {
            EXPECT_TRUE(cmds.resources_added<Frame>());
        }

        EXPECT_TRUE(cmds.resources_changed<Frame>());

        ++frame->value;
    }

    void time_deinit(Commands cmds) {
        if (!cmds.resources_exist<Time>()) return;

        cmds.remove_resource<Time>();
    }

    void stop(Commands cmds) {
        if (!cmds.resources_exist<Frame>()) return;

        auto [frame] = cmds.registry().resources<Frame>();

        if (frame->value > 100) {
            cmds.registry().stop();
        }
    }
}

TEST(CommandsTest, ResourceOperation) {
    Registry reg;

    reg.add_system(cro::time_init)
       .add_system(cro::frame_init)
       .add_system(system(cro::time_update).after(cro::time_init))
       .add_system(system(cro::frame_update).after(cro::frame_init))
       .add_system(system(cro::time_deinit).after(cro::time_update))
       .add_system(system(cro::stop).after(cro::frame_update))
       .run();
}