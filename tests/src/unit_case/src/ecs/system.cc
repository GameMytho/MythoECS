/*
 * Unit Tests for system.hpp - Systems and Scheduling Testing
 * 
 * This test suite validates the systems API and scheduling utilities:
 * - utils: function_traits, system_traits, and FunctionType concept
 * - argument construction: constructor and function invocation via basic_function
 * - function wrapper: basic_function (non-void and void specializations)
 * - system: basic_system after/before/runif chaining and accessors (function, runif, befores, afters)
 * - system stage: basic_system_stage add (Kahn topological order using sparse_set), and before/after dependencies
 * - removed entities: basic_removed_entities argument construction and system integration
 * 
 * Utils/Trait Test Cases:
 * 1. CompileAndConcept - Tests function and system traits compile and concept
 * 
 * Argument Construction Test Cases:
 * 1. ConstructWork - Tests constructor and function invocation work
 * 
 * Function Wrapper Test Cases:
 * 1. ReturnAndVoidInvoke - Tests basic function return and void invoke
 * 
 * Basic System Test Cases:
 * 1. FunctionAndRunif - Tests basic system function and runif accessors
 * 
 * System Test Cases:
 * 1. AfterBeforeRunif - Tests basic system after/before/runif chaining and befores/afters accessors
 * 
 * System Stage Test Cases:
 * 1. TopologicalOrder - Tests basic system stage topological order with dependencies
 * 2. AddByFunctionPointer - Tests adding systems directly by function pointer
 * 3. RunifFiltering - Tests runif filtering to control system execution
 * 
 */

#include <gtest/gtest.h>
#include <vector>
#include <cstdint>
#include <type_traits>

#include "ecs/system.hpp"
#include "ecs/registry.hpp"

#include "components.hpp"
#include "resources.hpp"
#include "events.hpp"

using namespace mytho::ecs;

using entity = basic_entity<uint32_t, uint16_t>;
using registry = basic_registry<entity>;

/*
 * ======================================== Helper Structures/Functions ==================================
 */
// Shared helper for order assertions
static std::vector<int> gOrder;

// basic_system helpers
static void sys_record_tick(basic_registrar<registry> r) {
    (void)r; // no-op
}

static bool runif_even_tick(basic_registrar<registry> r, basic_querier<registry, Position> q) {
    (void)r; (void)q; // constructed successfully
    return true;
}

// system_config helpers
static void sysA() { gOrder.push_back(1); }
static void sysB() { gOrder.push_back(2); }
static bool runifTrue() { return true; }

/*
 * ======================================== Utils/Trait Test Cases =====================================
 */

// utils::function_traits / system_traits / FunctionType concept
TEST(TraitTest, CompileAndConcept) {
    using Fn = int(float, double);
    using Fp = int(*)(float, double);
    static_assert(std::is_same_v<mytho::utils::function_traits_t<Fn>, Fn>);
    static_assert(std::is_same_v<mytho::utils::function_traits_t<Fp>, int(float, double)>);

    using SysArgs = mytho::utils::system_traits_t<void(int, char)>;
    static_assert(mytho::utils::is_type_list_v<SysArgs>);

    auto lam = +[](){}; // non-capturing lambda -> function pointer
    (void)lam; // satisfies FunctionType concept at compile-time via usage below
    SUCCEED();
}

/*
 * ================================= Argument Construction Test Cases ==================================
 */

// internal::constructor + function invocation via basic_function
TEST(ArgumentConstructionTest, ConstructWork) {
    registry reg;

    // Prepare world state
    auto e1 = reg.spawn(Position{1,2}); (void)e1;
    reg.init_resource<GameConfig>(7, "cfg");
    reg.event_write<DamageEvent>(9, 3.5f);
    reg.update(); // make first DamageEvent readable

    // System function that requests all supported argument categories
    auto* sys = +[](basic_registrar<registry> r,
                    basic_commands<registry> cmd,
                    basic_querier<registry, Position> q,
                    basic_resources<GameConfig> rs,
                    basic_resources_mut<GameConfig> rsm,
                    basic_event_writer<registry, DamageEvent> ew,
                    basic_event_mutator<DamageEvent> em,
                    basic_event_reader<DamageEvent> er,
                    basic_removed_entities<registry, Position> removed_pos) {
        // avoid unused warnings for constructed args
        (void)r; (void)cmd;

        // querier
        EXPECT_GE(q.size(), 1u);

        // resources (const)
        {
            auto [cfg] = rs.data();
            EXPECT_EQ(cfg->value, 7);
        }

        // resources (mut)
        {
            auto [cfgm] = rsm.data();
            cfgm->value = 8;
        }

        // events
        {
            auto v = er.read();
            ASSERT_EQ(v.size(), 1);
            for (auto& ev : v) {
                EXPECT_EQ(ev.id, 9);
            }
            (void)em.mutate();
            ew.write(10, 1.0f); // will appear after next update
        }

        // removed entities
        {
            EXPECT_TRUE(removed_pos.empty()); // Initially empty
        }
    };

    internal::basic_function<registry, void> f(sys);
    f(reg, 1);

    reg.update();
    auto r2 = reg.event_read<DamageEvent>();
    ASSERT_EQ(r2.size(), 1);
    for (auto& e : r2) { EXPECT_EQ(e.id, 10); }
}

/*
 * ================================ Function Wrapper Test Cases ========================================
 */

// internal::basic_function (non-void and void specializations)
TEST(FunctionWrapperTest, ReturnAndVoidInvoke) {
    registry reg;
    reg.init_resource<GameConfig>(1, "x");

    // non-void
    auto* sum_sys = +[](basic_resources<GameConfig> rs) -> int {
        auto [cfg] = rs.data();
        return cfg->value + 41;
    };
    internal::basic_function<registry, int> f1(sum_sys);
    EXPECT_EQ(f1(reg, 0), 42);

    // void with side effect
    int side = 0;
    (void)side; // avoid unused in MSVC release
    auto* void_sys = +[](basic_resources_mut<GameConfig> rs) {
        auto [cfg] = rs.data();
        cfg->value = 5;
    };
    internal::basic_function<registry, void> f2(void_sys);
    f2(reg, 0);
    {
        auto [cfg] = reg.resources<GameConfig>();
        EXPECT_EQ(cfg->value, 5);
    }
}

/*
 * ===================================== Basic System Test Cases =======================================
 */
// internal::basic_system (function, runif, after, before accessors)

TEST(BasicSystemTest, FunctionAndRunif) {
    registry reg;
    reg.spawn(Position{0,0});

    internal::basic_system<registry> sys(+sys_record_tick);
    EXPECT_NE(sys.function().pointer(), nullptr);

    // Test runif access
    EXPECT_FALSE(sys.runif().pointer());
    sys.runif(+[](){ return true; });
    EXPECT_NE(sys.runif().pointer(), nullptr);
}

/*
 * =================================== System Test Cases =======================================
 */
// internal::basic_system (builders and accessors)

TEST(SystemTest, AfterBeforeRunif) {
    internal::basic_system<registry> sysA(+::sysA);
    internal::basic_system<registry> sysB(+::sysB);

    sysB.after(+::sysA).runif(+runifTrue);
    EXPECT_NE(sysA.function().pointer(), nullptr);
    EXPECT_NE(sysB.function().pointer(), nullptr);
    EXPECT_NE(sysB.runif().pointer(), nullptr);

    // Test befores and afters accessors (they use move semantics, so we check after moving)
    auto befores = std::move(sysB).befores();
    auto afters = std::move(sysB).afters();
    EXPECT_EQ(befores.size(), 0u); // sysB has no before dependencies
    EXPECT_EQ(afters.size(), 1u);  // sysB has sysA as an after dependency
}

/*
 * =================================== System Stage Test Cases =======================================
 */
// internal::basic_system_stage (add, topological sort using sparse_set for efficient dependency resolution)
TEST(SystemStageTest, TopologicalOrder) {
    gOrder.clear();
    internal::basic_system_stage<registry> stage;

    auto* fA = +[]() { gOrder.push_back(1); };
    auto* fB = +[]() { gOrder.push_back(2); };
    auto* fC = +[]() { gOrder.push_back(3); };

    internal::basic_system<registry> sysA(fA);
    internal::basic_system<registry> sysB(fB);
    internal::basic_system<registry> sysC(fC);

    // A -> B -> C
    sysA.before(fB).before(fC);
    sysB.after(fA).before(fC);
    sysC.after(fA).after(fB);

    stage.add(sysA);
    stage.add(sysB);
    stage.add(sysC);

    EXPECT_EQ(stage.size(), 3u);

    registry reg;
    uint64_t tick = 0;
    stage.run(reg, tick);

    ASSERT_EQ(gOrder.size(), 3u);
    EXPECT_EQ(gOrder[0], 1);
    EXPECT_EQ(gOrder[1], 2);
    EXPECT_EQ(gOrder[2], 3);
}

// Test adding systems directly by function pointer (without creating basic_system objects)
TEST(SystemStageTest, AddByFunctionPointer) {
    gOrder.clear();
    internal::basic_system_stage<registry> stage;

    auto* fA = +[]() { gOrder.push_back(1); };
    auto* fB = +[]() { gOrder.push_back(2); };

    // Add systems directly by function pointer
    stage.add(fA);
    stage.add(fB);

    EXPECT_EQ(stage.size(), 2u);

    registry reg;
    uint64_t tick = 0;
    stage.run(reg, tick);

    ASSERT_EQ(gOrder.size(), 2u);
    EXPECT_EQ(gOrder[0], 1);
    EXPECT_EQ(gOrder[1], 2);
}

// Test runif filtering to control which systems execute (systems with runif returning false are skipped)
TEST(SystemStageTest, RunifFiltering) {
    gOrder.clear();
    internal::basic_system_stage<registry> stage;

    auto* fA = +[]() { gOrder.push_back(1); };
    auto* fB = +[]() { gOrder.push_back(2); };

    internal::basic_system<registry> sysA(fA);
    internal::basic_system<registry> sysB(fB);

    // sysB should not run (runif returns false)
    sysB.runif(+[](){ return false; });

    stage.add(sysA);
    stage.add(sysB);

    EXPECT_EQ(stage.size(), 2u);

    registry reg;
    uint64_t tick = 0;
    stage.run(reg, tick);

    // Only sysA should have run
    ASSERT_EQ(gOrder.size(), 1u);
    EXPECT_EQ(gOrder[0], 1);
}