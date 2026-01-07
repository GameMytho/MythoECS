#include <gtest/gtest.h>
#include <ecs/ecs.hpp>
#include <iostream>

using namespace mecs;

namespace qbo {
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

    void entity_spawn(Commands cmds) {
        cmds.spawn(
            Position{0.25f, 0.28f, 0.35f}
        );
    }

    void position_change(Querier<Entity, Mut<Position>, Added<Position>> q) {
        for (auto& [e, pos] : q) {
            EXPECT_EQ(pos->x, 0.25f);
            EXPECT_EQ(pos->y, 0.28f);
            EXPECT_EQ(pos->z, 0.35f);

            pos->x = 2.5f;
            pos->y = 2.8f;
            pos->z = 3.0f;
        }
    }

    void velocity_attach(Commands cmds, Querier<Entity, Position, Changed<Position>, Without<Velocity>> q) {
        for (auto& [e, pos] : q) {
            EXPECT_EQ(pos->x, 2.5f);
            EXPECT_EQ(pos->y, 2.8f);
            EXPECT_EQ(pos->z, 3.0f);

            cmds.insert(*e, Velocity{0.2f, 0.3f, 0.4f});
        }
    }

    void position_and_velocity_change(Querier<Entity, Mut<Position, Velocity>, Added<Velocity>> q) {
        for (auto& [e, pos, vel] : q) {
            EXPECT_EQ(pos->x, 2.5f);
            EXPECT_EQ(pos->y, 2.8f);
            EXPECT_EQ(pos->z, 3.0f);

            pos->x = 3.1f;
            pos->y = 3.2f;
            pos->z = 3.3f;

            EXPECT_EQ(vel->x, 0.2f);
            EXPECT_EQ(vel->y, 0.3f);
            EXPECT_EQ(vel->z, 0.4f);

            vel->x = 2.f;
            vel->y = 3.f;
            vel->z = 4.f;
        }
    }

    void final_check(Querier<Position, Without<Velocity>> q1, Querier<Position, With<Velocity>> q2) {
        for (auto& [pos] : q1) {
            EXPECT_EQ(pos->x, 2.5f);
            EXPECT_EQ(pos->y, 2.8f);
            EXPECT_EQ(pos->z, 3.0f);
        }

        for (auto& [pos] : q2) {
            EXPECT_EQ(pos->x, 3.1f);
            EXPECT_EQ(pos->y, 3.2f);
            EXPECT_EQ(pos->z, 3.3f);
        }
    }
}

TEST(QuerierTest, BasicOperation) {
    Registry reg;

    reg.add_update_system(qbo::entity_spawn)
       .add_update_system(system(qbo::position_change).after(qbo::entity_spawn))
       .add_update_system(system(qbo::velocity_attach).after(qbo::position_change))
       .add_update_system(system(qbo::position_and_velocity_change).after(qbo::velocity_attach))
       .add_update_system(system(qbo::final_check).after(qbo::position_and_velocity_change))
       .ready();

    reg.startup();

    do {
        reg.update();
    } while(reg.count<Entity, With<qbo::Position>>() < 100);
}