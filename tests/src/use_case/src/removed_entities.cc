#include <gtest/gtest.h>
#include "ecs/ecs.hpp"

using namespace mecs;

namespace rebo {
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
            Position{0.25f, 0.28f, 0.35f},
            Velocity{0.46f, 0.24f, 0.53f}
        );
    }

    void velocity_remove(Commands cmds, Querier<Entity, With<Position, Velocity>> q) {
        for (auto& [e] : q) {
            cmds.remove<Velocity>(*e);
        }
    }

    void velocity_restore(Commands cmds, RemovedEntities<Velocity> entts) {
        for (auto& e : entts) {
            cmds.insert(e, Velocity{0.2f, 0.3f, 0.4f});
        }
    }

    void entity_check(Commands cmds, Querier<Entity, With<Position>> q1, Querier<Entity, With<Position, Velocity>> q2) {
        EXPECT_EQ(cmds.registry().entities().size(), q1.size());
        EXPECT_EQ((cmds.registry().entities().size() + 1) / 2, q2.size());
    }
}

TEST(RemovedEntitiesTest, BasicOperation) {
    Registry reg;

    reg.add_startup_system(rebo::entity_spawn)
       .add_update_system(rebo::entity_spawn)
       .add_update_system(system(rebo::velocity_remove).after(rebo::entity_spawn))
       .add_update_system(system(rebo::velocity_restore).after(rebo::velocity_remove))
       .add_update_system(system(rebo::entity_check).after(rebo::velocity_restore))
       .ready();

    reg.startup();

    while(reg.count<Entity, With<rebo::Position>>() < 100) {
        reg.update();
    }
}