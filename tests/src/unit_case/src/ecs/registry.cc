/*
 * Unit Tests for registry.hpp - Registry Core API Testing
 * 
 * This test suite validates:
 * - Entity lifecycle: spawn/alive/despawn
 * - Components: insert/get/replace/remove/contain
 * - Tick semantics: components_added/components_changed
 * - Counting: count() with filters (with<>/without<>)
 * - Resources: init/resources/resources_mut/remove + tick semantics
 * - Events: event_write/event_read/event_mutate and swap on update
 * - Systems: add_*_system (Func/Config), ready/startup/update/shutdown execution order
 * - Removed entities: removed_entities() access and update() clearing behavior
 * 
 * Registry Test Cases:
 * 1. BasicEntityLifecycle - Verify entity lifecycle operations
 * 2. ComponentsInsertReplaceRemoveGet - Verify components insert, replace, remove, and get operations
 * 3. ComponentsDetection - Verify components added, changed, removed operations and removed entities tracking
 * 4. CountWithFilters - Verify count operations with filters
 * 5. ResourcesLifecycleAndTicks - Verify resources lifecycle operations with ticks
 * 6. EventsWriteReadMutateAndSwap - Verify events write, read, mutate, and swap operations
 * 7. AddAndRunSystemsWithFunc - Verify add and run systems with functions
 * 8. AddAndRunSystemsWithConfig - Verify add and run systems with configurations
 */

#include <gtest/gtest.h>
#include <type_traits>
#include <ecs/registry.hpp>

using namespace mytho::ecs;

/*
 * ======================================== Helper Structures/Functions ==================================
 */

#include "components.hpp"
#include "resources.hpp"
#include "events.hpp"

using entity = basic_entity<uint32_t, uint16_t>;
using registry = basic_registry<entity, uint8_t, uint8_t, uint8_t, 1024>;
static std::vector<int> gSysOrder;

using StartupStage = mytho::ecs::startup_stage;
using CoreStage = mytho::ecs::core_stage;

enum class CustomStartupStage {
    PreStartup,
    PostStartup,
};

enum class CustomCoreStage {
    PreRender,
    Render,
    PostRender
};

/*
 * ======================================== Registry Test Cases ===================================
 */

TEST(RegistryTest, BasicEntityLifecycle) {
    registry reg;

    auto e = reg.spawn(Position{1, 2}, Velocity{3, 4});
    EXPECT_TRUE(reg.alive(e));
    EXPECT_TRUE((reg.contain<Position, Velocity>(e)));

    {
        auto [pos, vel] = reg.get<Position, Velocity>(e);
        EXPECT_EQ(pos.x, 1); EXPECT_EQ(pos.y, 2);
        EXPECT_EQ(vel.vx, 3); EXPECT_EQ(vel.vy, 4);
    }

    reg.despawn(e);
    EXPECT_FALSE(reg.alive(e));
    EXPECT_FALSE((reg.contain<Position, Velocity>(e)));
}

TEST(RegistryTest, ComponentsInsertReplaceRemoveGet) {
    registry reg;

    auto e = reg.spawn<>();
    EXPECT_FALSE(reg.contain<Position>(e));

    reg.insert(e, Position{7, 8});
    EXPECT_TRUE(reg.contain<Position>(e));
    {
        auto [pos] = reg.get<Position>(e);
        EXPECT_EQ(pos.x, 7); EXPECT_EQ(pos.y, 8);
    }

    reg.replace(e, Position{100, 200});
    {
        auto [pos] = reg.get<Position>(e);
        EXPECT_EQ(pos.x, 100); EXPECT_EQ(pos.y, 200);
    }

    reg.remove<Position>(e);
    EXPECT_FALSE(reg.contain<Position>(e));
}

TEST(RegistryTest, ComponentsDetection) {
    registry reg;

    auto e = reg.spawn(Position{0, 0}); // added/changed at tick=1
    EXPECT_TRUE(reg.contain<Position>(e));
    EXPECT_TRUE(reg.components_added<Position>(1));
    EXPECT_TRUE(reg.components_changed<Position>(1));
    EXPECT_FALSE(reg.components_removed<Position>());
    EXPECT_TRUE(reg.removed_entities<Position>().empty()); // Initially no removed entities

    reg.update(); // current tick -> 2
    reg.replace(e, Position{1, 1});
    EXPECT_TRUE(reg.contain<Position>(e));
    EXPECT_FALSE(reg.components_added<Position>(2));
    EXPECT_TRUE(reg.components_changed<Position>(2));
    EXPECT_FALSE(reg.components_added<Position>(3));
    EXPECT_FALSE(reg.components_changed<Position>(3));
    EXPECT_FALSE(reg.components_removed<Position>());
    EXPECT_TRUE(reg.removed_entities<Position>().empty()); // Still no removed entities

    reg.remove<Position>(e);
    EXPECT_FALSE(reg.contain<Position>(e));
    EXPECT_FALSE(reg.components_added<Position>(2));
    EXPECT_FALSE(reg.components_changed<Position>(2));
    EXPECT_TRUE(reg.components_removed<Position>());
    EXPECT_EQ(reg.removed_entities<Position>().size(), 1); // Now has removed entity
    EXPECT_EQ(reg.removed_entities<Position>()[0], e);

    reg.update();
    EXPECT_FALSE(reg.contain<Position>(e));
    EXPECT_FALSE(reg.components_added<Position>(3));
    EXPECT_FALSE(reg.components_changed<Position>(3));
    EXPECT_FALSE(reg.components_removed<Position>());
    EXPECT_TRUE(reg.removed_entities<Position>().empty()); // Cleared after update
}

TEST(RegistryTest, CountWithFilters) {
    registry reg;

    auto e1 = reg.spawn(Position{1,1}); (void)e1;
    auto e2 = reg.spawn(Position{2,2}, Velocity{2,2}); (void)e2;
    auto e3 = reg.spawn(Position{3,3}, Velocity{3,3}); (void)e3;

    EXPECT_EQ((reg.count<Position>(1)), 3);
    EXPECT_EQ((reg.count<Position, with<Velocity>>(1)), 2);
    EXPECT_EQ((reg.count<Position, without<Velocity>>(1)), 1);
}

TEST(RegistryTest, ResourcesLifecycleAndTicks) {
    registry reg;

    EXPECT_FALSE(reg.resources_exist<GameConfig>());
    reg.init_resource<GameConfig>(42, "cfg"); // added/changed at tick=1
    EXPECT_TRUE(reg.resources_exist<GameConfig>());
    EXPECT_TRUE((reg.resources_added<GameConfig>(1)));

    {
        auto [cfg] = reg.resources<GameConfig>();
        EXPECT_EQ(cfg->value, 42); EXPECT_EQ(cfg->name, std::string("cfg"));
    }

    reg.update(); // current tick -> 2
    {
        auto [cfg] = reg.resources_mut<GameConfig>();
        cfg->value = 100; // set changed at tick=2 via data_wrapper
    }

    EXPECT_TRUE((reg.resources_changed<GameConfig>(2)));
    EXPECT_FALSE((reg.resources_changed<GameConfig>(3)));

    reg.remove_resource<GameConfig>();
    EXPECT_FALSE(reg.resources_exist<GameConfig>());
}

TEST(RegistryTest, EventsWriteReadMutateAndSwap) {
    registry reg;

    reg.event_write<DamageEvent>(1, 1.5f);
    EXPECT_EQ(reg.event_read<DamageEvent>().size(), 0); // still in write queue

    reg.update(); // triggers events swap
    auto r = reg.event_read<DamageEvent>();
    ASSERT_EQ(r.size(), 1);
    for (auto& e : r) { EXPECT_EQ(e.id, 1); EXPECT_FLOAT_EQ(e.value, 1.5f); }

    // mutate view
    auto m = reg.event_mutate<DamageEvent>();
    for (auto it = m.begin(); it != m.end(); ++it) { it->value += 10.0f; }
    auto r2 = reg.event_read<DamageEvent>();
    ASSERT_EQ(r2.size(), 1);
    for (auto& e2 : r2) { EXPECT_EQ(e2.id, 1); EXPECT_FLOAT_EQ(e2.value, 11.5f); }

    // new write is not visible until next update
    reg.event_write<DamageEvent>(2, 2.0f);
    EXPECT_EQ(reg.event_read<DamageEvent>().size(), 1);
    reg.update();
    auto r3 = reg.event_read<DamageEvent>();
    ASSERT_EQ(r3.size(), 1);
}

TEST(RegistryTest, AddAndRunSystemsWithFunc) {
    gSysOrder.clear();

    registry reg;

    reg.add_startup_system(+[](){ gSysOrder.push_back(1); })
        .add_update_system(+[](){ gSysOrder.push_back(2); });

    reg.ready();
    reg.startup();

    reg.update();

    ASSERT_EQ(gSysOrder.size(), 2u);
    EXPECT_EQ(gSysOrder[0], 1);
    EXPECT_EQ(gSysOrder[1], 2);
}

TEST(RegistryTest, AddAndRunSystemsWithConfig) {
    gSysOrder.clear();

    registry reg;

    auto s1 = registry::system(+[](){ gSysOrder.push_back(10); });
    auto s2 = registry::system(+[](){ gSysOrder.push_back(20); });

    reg.add_startup_system(s1)
        .add_update_system(s2);

    reg.ready();
    reg.startup();

    reg.update();

    ASSERT_EQ(gSysOrder.size(), 2u);
    EXPECT_EQ(gSysOrder[0], 10);
    EXPECT_EQ(gSysOrder[1], 20);
}

TEST(RegistryTest, AddAndRunSystemWithSpecifiedStage) {
    gSysOrder.clear();

    registry reg;

    auto s1 = registry::system(+[](){ gSysOrder.push_back(10); });
    auto s2 = registry::system(+[](){ gSysOrder.push_back(20); });
    auto s3 = registry::system(+[](){ gSysOrder.push_back(30); });
    auto s4 = registry::system(+[](){ gSysOrder.push_back(40); });
    auto s5 = registry::system(+[](){ gSysOrder.push_back(50); });
    auto s6 = registry::system(+[](){ gSysOrder.push_back(60); });

    reg.add_startup_system<StartupStage::Startup>(s1)
        .add_update_system<CoreStage::Update>(s5)
        .add_update_system<CoreStage::First>(s6)
        .add_update_system<CoreStage::Last>(s2)
        .add_update_system<CoreStage::PreUpdate>(s3)
        .add_update_system<CoreStage::PostUpdate>(s4);

    reg.ready();
    reg.startup();

    reg.update();

    ASSERT_EQ(gSysOrder.size(), 6u);
    EXPECT_EQ(gSysOrder[0], 10);
    EXPECT_EQ(gSysOrder[1], 60);
    EXPECT_EQ(gSysOrder[2], 30);
    EXPECT_EQ(gSysOrder[3], 50);
    EXPECT_EQ(gSysOrder[4], 40);
    EXPECT_EQ(gSysOrder[5], 20);
}

TEST(RegistryTest, AddAndRunSystemWithCustomStage) {
    gSysOrder.clear();

    registry reg;

    auto s1 = registry::system(+[](){ gSysOrder.push_back(10); });
    auto s2 = registry::system(+[](){ gSysOrder.push_back(20); });
    auto s3 = registry::system(+[](){ gSysOrder.push_back(30); });
    auto s4 = registry::system(+[](){ gSysOrder.push_back(40); });
    auto s5 = registry::system(+[](){ gSysOrder.push_back(50); });
    auto s6 = registry::system(+[](){ gSysOrder.push_back(60); });
    auto s7 = registry::system(+[](){ gSysOrder.push_back(70); });
    auto s8 = registry::system(+[](){ gSysOrder.push_back(80); });
    auto s9 = registry::system(+[](){ gSysOrder.push_back(90); });
    auto s10 = registry::system(+[](){ gSysOrder.push_back(100); });

    reg.add_startup_stage_before<CustomStartupStage::PreStartup, StartupStage::Startup>()
        .add_startup_stage_after<CustomStartupStage::PostStartup, StartupStage::Startup>()
        .add_update_stage_after<CustomCoreStage::PreRender, CoreStage::PreUpdate>()
        .add_update_stage_before<CustomCoreStage::PostRender, CoreStage::PostUpdate>()
        .insert_update_stage<CustomCoreStage::Render, CoreStage::Update>();

    reg.add_startup_system<CustomStartupStage::PreStartup>(s1)
        .add_startup_system<StartupStage::Startup>(s2)
        .add_startup_system<CustomStartupStage::PostStartup>(s3)
        .add_update_system<CoreStage::First>(s4)
        .add_update_system<CoreStage::PreUpdate>(s5)
        .add_update_system<CustomCoreStage::PreRender>(s6)
        .add_update_system<CustomCoreStage::Render>(s7)
        .add_update_system<CustomCoreStage::PostRender>(s8)
        .add_update_system<CoreStage::PostUpdate>(s9)
        .add_update_system<CoreStage::Last>(s10);

    reg.ready();
    reg.startup();

    reg.update();

    ASSERT_EQ(gSysOrder.size(), 10u);
    EXPECT_EQ(gSysOrder[0], 10);
    EXPECT_EQ(gSysOrder[1], 20);
    EXPECT_EQ(gSysOrder[2], 30);
    EXPECT_EQ(gSysOrder[3], 40);
    EXPECT_EQ(gSysOrder[4], 50);
    EXPECT_EQ(gSysOrder[5], 60);
    EXPECT_EQ(gSysOrder[6], 70);
    EXPECT_EQ(gSysOrder[7], 80);
    EXPECT_EQ(gSysOrder[8], 90);
    EXPECT_EQ(gSysOrder[9], 100);
}