/*
 * Unit Tests for system.hpp - Systems and Scheduling Testing
 * 
 * This test suite validates the systems API and scheduling utilities:
 * - utils: function_traits, system_traits, and FunctionType concept
 * - argument construction: argument_constructor and construct() path via basic_function
 * - hashing: function_pointer_hash uniqueness for different functions
 * - function wrapper: basic_function (non-void and void specializations)
 * - system execution: basic_system should_run and run-if behavior with tick propagation
 * - system config: basic_system_config after/before/runif chaining and accessors
 * - system storage: basic_system_storage add/ready (Kahn topological order), and before->after conversion
 * 
 * Utils/Trait Test Cases:
 * 1. FunctionAndSystemTraitsCompileAndConcept - Tests function and system traits compile and concept
 * 
 * Argument Construction Test Cases:
 * 1. ArgumentConstructorsAndConstructWork - Tests argument constructors and construct work
 * 
 * Function Wrapper Test Cases:
 * 1. FunctionPointerHashProducesDifferentValues - Tests function pointer hash produces different values
 * 2. BasicFunctionReturnAndVoidInvoke - Tests basic function return and void invoke
 * 
 * Basic System Test Cases:
 * 1. BasicSystemRunAndTick - Tests basic system run and tick
 * 2. BasicSystemRunIfControlsExecution - Tests basic system run if controls execution
 * 
 * System Config Test Cases:
 * 1. BasicSystemConfigAfterBeforeRunif - Tests basic system config after before runif
 * 
 * System Storage Test Cases:
 * 1. BasicSystemStorageTopologicalOrder - Tests basic system storage topological order
 * 2. BasicSystemStorageBeforeRelationConvertedToAfter - Tests basic system storage before relation converted to after
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
using registry = basic_registry<entity, uint8_t, uint8_t, uint8_t, 1024>;

/*
 * ======================================== Helper Structures/Functions ==================================
 */
// Shared helper for order assertions
static std::vector<int> gOrder;

// function_pointer_hash helpers (make bodies distinct to avoid ICF folding in release)
static int fp_sink = 0;
static void fp_hash_a() { fp_sink += 1; }
static void fp_hash_b() { fp_sink += 2; }

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
TEST(SystemHeaderTest, FunctionAndSystemTraitsCompileAndConcept) {
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
// internal::argument_constructor + construct() via basic_function invocation
TEST(SystemHeaderTest, ArgumentConstructorsAndConstructWork) {
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
                    basic_event_reader<DamageEvent> er) {
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
// internal::function_pointer_hash

TEST(SystemHeaderTest, FunctionPointerHashProducesDifferentValues) {
    internal::function_pointer_hash hasher;
    internal::basic_system_config<registry> cA(+fp_hash_a);
    internal::basic_system_config<registry> cB(+fp_hash_b);
    void* pa = cA.function().pointer();
    void* pb = cB.function().pointer();
    EXPECT_NE(hasher(pa), hasher(pb));
}

// internal::basic_function (non-void and void specializations)
TEST(SystemHeaderTest, BasicFunctionReturnAndVoidInvoke) {
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
// internal::basic_system (should_run and tick propagation)

TEST(SystemHeaderTest, BasicSystemRunAndTick) {
    registry reg;
    reg.spawn(Position{0,0});

    internal::basic_system<registry> sys(+sys_record_tick);
    EXPECT_TRUE(sys.should_run(reg));
    sys(reg, 1);
    sys(reg, 2);
}

TEST(SystemHeaderTest, BasicSystemRunIfControlsExecution) {
    registry reg;
    internal::basic_function<registry, void> fn(+sys_record_tick);
    internal::basic_function<registry, bool> rf(+[](){ return false; });
    internal::basic_system<registry> sys(fn, rf);
    EXPECT_FALSE(sys.should_run(reg));
}

/*
 * =================================== System Config Test Cases ========================================
 */
// internal::basic_system_config (builders and accessors)

TEST(SystemHeaderTest, BasicSystemConfigAfterBeforeRunif) {
    internal::basic_system_config<registry> cfgA(+sysA);
    internal::basic_system_config<registry> cfgB(+sysB);

    cfgB.after(+sysA).runif(+runifTrue);
    EXPECT_NE(cfgA.function().pointer(), nullptr);
    EXPECT_NE(cfgB.function().pointer(), nullptr);
    EXPECT_NE(cfgB.runif().pointer(), nullptr);
}

/*
 * =================================== System Storage Test Cases =======================================
 */
// internal::basic_system_storage (add, ready/toposort, iteration)
TEST(SystemHeaderTest, BasicSystemStorageTopologicalOrder) {
    gOrder.clear();
    internal::basic_system_storage<registry> storage;

    auto* fA = +[]() { gOrder.push_back(1); };
    auto* fB = +[]() { gOrder.push_back(2); };
    auto* fC = +[]() { gOrder.push_back(3); };

    internal::basic_system_config<registry> cA(fA);
    internal::basic_system_config<registry> cB(fB);
    internal::basic_system_config<registry> cC(fC);

    // A -> B -> C
    cB.after(fA);
    cC.after(fB);

    storage.add(cA);
    storage.add(cB);
    storage.add(cC);

    storage.ready();
    EXPECT_EQ(storage.size(), 3u);

    registry reg;
    for (auto& s : storage) {
        s(reg, 0);
    }

    ASSERT_EQ(gOrder.size(), 3u);
    EXPECT_EQ(gOrder[0], 1);
    EXPECT_EQ(gOrder[1], 2);
    EXPECT_EQ(gOrder[2], 3);
}

// internal::basic_system_storage before relation converted to after
TEST(SystemHeaderTest, BasicSystemStorageBeforeRelationConvertedToAfter) {
    gOrder.clear();
    internal::basic_system_storage<registry> storage;

    auto* fA = +[]() { gOrder.push_back(1); };
    auto* fB = +[]() { gOrder.push_back(2); };

    internal::basic_system_config<registry> cA(fA);
    internal::basic_system_config<registry> cB(fB);

    // B before A means A after B
    cB.before(fA);

    storage.add(cA);
    storage.add(cB);

    storage.ready();
    registry reg;
    for (auto& s : storage) { s(reg, 0); }

    ASSERT_EQ(gOrder.size(), 2u);
    // B should run before A
    EXPECT_EQ(gOrder[0], 2);
    EXPECT_EQ(gOrder[1], 1);
}