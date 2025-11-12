/*
 * Unit Tests for registrar.hpp - Registrar Wrapper Testing
 * 
 * This test suite validates:
 * - basic_registrar: forwards get/contain/count with tick snapshot semantics
 * - components_added/components_changed/components_removed: tick-based detection and removed components detection
 * - resources_exist/resources_added/resources_changed: resource lifecycle with ticks
 * - traits: is_registrar_v
 * 
 * Registrar Test Cases:
 * 1. GetAndContain - Verify get and contain operations
 * 2. ComponentsDetection - Verify components added, changed, and removed operations
 * 3. CountWithFilters - Verify count operations with filters
 * 4. ResourcesLifecycleWithTicks - Verify resources lifecycle operations with ticks
 * 
 * Registrar Trait Test Cases:
 * 1. IsRegistrarTrait - Verify is_registrar_v trait
 */

#include <gtest/gtest.h>
#include <type_traits>
#include <ecs/registry.hpp>
#include <ecs/registrar.hpp>

using namespace mytho::ecs;

/*
 * ======================================== Helper Structures/Functions ==================================
 */

#include "components.hpp"
#include "resources.hpp"

using entity = basic_entity<uint32_t, uint16_t>;
using registry = basic_registry<entity>;
using registrar = basic_registrar<registry>;

/*
 * ======================================== Registrar Basic Test Cases ===================================
 */

TEST(RegistrarTest, GetAndContain) {
    registry reg;
    auto e = reg.spawn(Position{1,2}, Velocity{3,4});
    registrar r(reg, 1);

    EXPECT_TRUE((r.contain<Position, Velocity>(e)));
    {
        auto [pos, vel] = r.get<Position, Velocity>(e);
        EXPECT_EQ(pos.x, 1); EXPECT_EQ(pos.y, 2);
        EXPECT_EQ(vel.vx, 3); EXPECT_EQ(vel.vy, 4);
    }
}

TEST(RegistrarTest, ComponentsDetection) {
    registry reg;
    auto e = reg.spawn(Position{10,20}); // added/changed at tick=1

    registrar r1(reg, 1);
    EXPECT_TRUE(r1.components_added<Position>());
    EXPECT_TRUE(r1.components_changed<Position>());
    EXPECT_FALSE(r1.components_removed<Position>()); // Initially no components removed

    reg.update(); // _current_tick -> 2
    reg.replace(e, Position{11,21}); // changed at tick=2

    registrar r2(reg, 2);
    EXPECT_TRUE(r2.components_changed<Position>());
    EXPECT_FALSE(r2.components_removed<Position>()); // Still no components removed

    registrar r3(reg, 3);
    EXPECT_FALSE(r3.components_changed<Position>());
    EXPECT_FALSE(r3.components_removed<Position>()); // Still no components removed

    // Test components_removed after removing components
    reg.remove<Position>(e);
    registrar r4(reg, 4);
    EXPECT_TRUE(r4.components_removed<Position>()); // Now components are removed

    // Test after update (should be false)
    reg.update();
    registrar r5(reg, 5);
    EXPECT_FALSE(r5.components_removed<Position>()); // Cleared after update
}

TEST(RegistrarTest, CountWithFilters) {
    registry reg;
    auto e1 = reg.spawn(Position{1,1}); (void)e1;
    auto e2 = reg.spawn(Position{2,2}, Velocity{20,20}); (void)e2;
    auto e3 = reg.spawn(Position{3,3}, Velocity{30,30}); (void)e3;

    registrar r(reg, 1);
    EXPECT_EQ((r.count<Position>()), 3);
    EXPECT_EQ((r.count<Position, with<Velocity>>()), 2);
    EXPECT_EQ((r.count<Position, without<Velocity>>()), 1);
}

TEST(RegistrarTest, ResourcesLifecycleWithTicks) {
    registry reg;

    EXPECT_FALSE(reg.resources_exist<GameConfig>());
    reg.init_resource<GameConfig>(42, "cfg"); // added/changed at tick=1

    registrar r1(reg, 1);
    EXPECT_TRUE(r1.resources_exist<GameConfig>());
    EXPECT_TRUE(r1.resources_added<GameConfig>());

    reg.update(); // _current_tick -> 2
    {
        auto [cfg] = reg.resources_mut<GameConfig>();
        cfg->value = 100; // set changed at tick=2 via data_wrapper
    }

    registrar r2(reg, 2);
    EXPECT_TRUE(r2.resources_changed<GameConfig>());

    registrar r3(reg, 3);
    EXPECT_FALSE(r3.resources_changed<GameConfig>());
}


/*
 * ======================================== Trait Test Cases =============================================
 */

TEST(RegistrarTraitTest, IsRegistrarTrait) {
    static_assert(mytho::utils::is_registrar_v<registrar>, "registrar type should satisfy is_registrar_v");
    EXPECT_TRUE(mytho::utils::is_registrar_v<registrar>);
    EXPECT_FALSE(mytho::utils::is_registrar_v<int>);
}


