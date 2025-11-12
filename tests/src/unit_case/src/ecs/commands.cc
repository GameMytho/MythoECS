/*
 * Unit Tests for commands.hpp - Commands Queue Testing
 * 
 * This test suite validates the basic_commands and command queue behavior:
 * - spawn: Queue and apply spawning entities with components
 * - insert: Queue and apply inserting components into an existing entity
 * - replace: Queue and apply replacing specific component data
 * - remove: Queue and apply removing specific components from an entity
 * - despawn: Queue and apply removing an entity and its components
 * - init_resource/remove_resource: Queue and apply resource lifecycle operations
 * 
 * Command Queue Test Cases:
 * 1. EntitySpawnDespawn - Spawn entity with components and then despawn
 * 2. ComponentInsertReplaceRemove - Insert/Replace/Remove components on an entity
 * 3. ResourceInitRemove - Resource lifecycle operations
 * 4. ClearQueue - Clear pending commands without applying
 * 
 * Commands Test Cases:
 * 1. EntitySpawnDespawn - Spawn entity with components and then despawn
 * 2. ComponentInsertReplaceRemove - Insert/Replace/Remove components on an entity
 * 3. ResourceInitRemove - Resource lifecycle operations
 * 
 * Commands Trait Test Cases:
 * 1. IsCommandsTrait - Tests commands trait detection
 */

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <algorithm>
#include <string>
#include <ecs/registry.hpp>
#include <ecs/commands.hpp>

using namespace mytho::ecs;

/*
 * ======================================== Helper Structures/Functions ==================================
 */

#include "components.hpp"
#include "resources.hpp"

using entity = basic_entity<uint32_t, uint16_t>;
using registry = basic_registry<entity>;
using queue = internal::basic_command_queue<registry>;
using commands = basic_commands<registry>;

/*
 * ======================================== Command Queue Test Cases =====================================
 */

 TEST(CommandQueueTest, EntitySpawnDespawnQueue) {
    registry reg;
    queue q;

    EXPECT_EQ((reg.query<entity, Position, Velocity, Health>().size()), 0);

    q.spawn(Position{1,2}, Velocity{3,4}, Health{5,6});
    q.apply(reg);

    {
        auto qu = reg.query<entity, Position, Velocity, Health>();
        ASSERT_EQ(qu.size(), 1);
        for (auto [e, pos, vel, hp] : qu) {
            EXPECT_EQ(pos->x, 1);
            EXPECT_EQ(pos->y, 2);
            EXPECT_EQ(vel->vx, 3);
            EXPECT_EQ(vel->vy, 4);
            EXPECT_EQ(hp->current, 5);
            EXPECT_EQ(hp->max, 6);
            q.despawn(*e);
        }
    }

    q.apply(reg);

    EXPECT_EQ((reg.query<entity, Position>().size()), 0);
}

TEST(CommandQueueTest, ComponentInsertRemoveQueue) {
    registry reg;
    queue q;

    auto e = reg.spawn<>();
    EXPECT_FALSE(reg.contain<Position>(e));

    q.insert<Position>(e, Position{7,8});
    q.apply(reg);
    EXPECT_TRUE(reg.contain<Position>(e));
    {
        auto [pos] = reg.get<Position>(e);
        EXPECT_EQ(pos.x, 7);
        EXPECT_EQ(pos.y, 8);
    }

    q.remove<Position>(e);
    q.apply(reg);
    EXPECT_FALSE(reg.contain<Position>(e));
}

TEST(CommandQueueTest, ComponentReplaceQueue) {
    registry reg;
    queue q;

    auto e = reg.spawn<>( );
    q.insert<Position>(e, Position{7,8});
    q.apply(reg);

    q.replace(e, Position{100,200});
    q.apply(reg);
    {
        auto [pos] = reg.get<Position>(e);
        EXPECT_EQ(pos.x, 100);
        EXPECT_EQ(pos.y, 200);
    }
}

TEST(CommandQueueTest, ResourceInitRemoveQueue) {
    registry reg;
    queue q;

    EXPECT_FALSE(reg.resources_exist<GameConfig>());
    q.init_resource<GameConfig>(11, "cfg");
    q.apply(reg);
    EXPECT_TRUE(reg.resources_exist<GameConfig>());

    {
        auto [cfg] = reg.resources<GameConfig>();
        EXPECT_EQ(cfg->value, 11);
        EXPECT_EQ(cfg->name, std::string("cfg"));
    }

    q.remove_resource<GameConfig>();
    q.apply(reg);
    EXPECT_FALSE(reg.resources_exist<GameConfig>());
}

TEST(CommandQueueTest, ClearQueue) {
    registry reg;
    queue q;

    auto e = reg.spawn<>();
    q.insert<Position>(e, Position{1,2});
    q.replace(e, Position{3,4});
    q.remove<Position>(e);
    q.init_resource<GameConfig>(1, "x");
    q.remove_resource<GameConfig>();
    q.despawn(e);

    // Clear before apply, registry should remain unaffected
    q.clear();

    // nothing applied
    EXPECT_FALSE(reg.contain<Position>(e));
    EXPECT_FALSE(reg.resources_exist<GameConfig>());
    EXPECT_TRUE(reg.alive(e));
}

/*
 * ======================================== Commands Test Cases ========================================
 */

TEST(CommandsTest, EntitySpawnDespawn) {
    registry reg;
    commands cmds(reg);

    {
        auto q_before = reg.query<entity, Position, Velocity, Health>();
        EXPECT_EQ(q_before.size(), 0);
    }

    cmds.spawn(Position{10, 20}, Velocity{1, 2}, Health{3, 4});
    reg.apply_commands();

    {
        auto q = reg.query<entity, Position, Velocity, Health>();
        ASSERT_EQ(q.size(), 1);

        for (auto [e, pos, vel, hp] : q) {
            EXPECT_EQ(pos->x, 10);
            EXPECT_EQ(pos->y, 20);
            EXPECT_EQ(vel->vx, 1);
            EXPECT_EQ(vel->vy, 2);
            EXPECT_EQ(hp->current, 3);
            EXPECT_EQ(hp->max, 4);

            cmds.despawn(*e);
            reg.apply_commands();
            EXPECT_FALSE(reg.contain<Position>(*e));
        }
    }
}

TEST(CommandsTest, ComponentInsertRemove) {
    registry reg;
    commands cmds(reg);

    auto e = reg.spawn<>();
    EXPECT_FALSE(reg.contain<Position>(e));

    cmds.insert<Position>(e, Position{7, 8});
    reg.apply_commands();
    EXPECT_TRUE(reg.contain<Position>(e));
    {
        auto [pos] = reg.get<Position>(e);
        EXPECT_EQ(pos.x, 7);
        EXPECT_EQ(pos.y, 8);
    }

    cmds.remove<Position>(e);
    reg.apply_commands();
    EXPECT_FALSE(reg.contain<Position>(e));
}

TEST(CommandsTest, ComponentReplace) {
    registry reg;
    commands cmds(reg);

    auto e = reg.spawn<>();
    cmds.insert<Position>(e, Position{7, 8});
    reg.apply_commands();

    cmds.replace(e, Position{100, 200});
    reg.apply_commands();
    {
        auto [pos] = reg.get<Position>(e);
        EXPECT_EQ(pos.x, 100);
        EXPECT_EQ(pos.y, 200);
    }
}

TEST(CommandsTest, ResourceInitRemove) {
    registry reg;
    commands cmds(reg);

    EXPECT_FALSE(reg.resources_exist<GameConfig>());

    cmds.init_resource<GameConfig>(42, "cfg");
    reg.apply_commands();
    EXPECT_TRUE(reg.resources_exist<GameConfig>());

    {
        auto [cfg] = reg.resources<GameConfig>();
        EXPECT_EQ(cfg->value, 42);
        EXPECT_EQ(cfg->name, std::string("cfg"));
    }

    cmds.remove_resource<GameConfig>();
    reg.apply_commands();
    EXPECT_FALSE(reg.resources_exist<GameConfig>());
}

/*
 * ======================================== Trait Test Cases ===========================================
 */

 TEST(CommandsTraitTest, IsCommandsTrait) {
    static_assert(mytho::utils::is_commands_v<commands>, "commands type should satisfy is_commands_v");
    EXPECT_TRUE(mytho::utils::is_commands_v<commands>);
    EXPECT_FALSE(mytho::utils::is_commands_v<int>);
}