/*
 * Unit Tests for schedule.hpp - Stages & Scheduling Orchestration Testing
 * 
 * This test suite validates the scheduling API built on top of system stages:
 * - stage management: add_stage/add_stage_before/add_stage_after/insert_stage
 * - system adding: add_system(Func) and add_system(system_type)
 * - execution: run() executes systems in stage order with topological sorting
 * 
 * Schedule Test Cases:
 * 1. AddStageAndRunOrder - Verify add_stage order reflected in run execution
 * 2. AddStageBeforeAfter - Verify before/after insertion affects order correctly
 * 3. InsertStageBehavior - Verify insert_stage behavior when target exists/absent
 * 4. AddSystemOverloads - Verify add_system with Func and Config both work
 */

#include <gtest/gtest.h>
#include <vector>

#include <ecs/registry.hpp>
#include <ecs/schedule.hpp>

using namespace mytho::ecs;

/*
 * ======================================== Helper Structures/Functions ==================================
 */

struct stage_genor final {};

using entity = basic_entity<uint32_t, uint16_t>;
using registry = basic_registry<entity>;

// Shared order sink
static std::vector<int> gOrder;

namespace {
    enum class TestStage {
        A,
        B,
        C,
        D,
        X,
        Y,
        Z
    };
}

// convenience alias
using schedule_t = typename registry::schedule_type;

/*
 * ======================================== Test Cases ==================================
 */

TEST(ScheduleTest, AddStageAndRunOrder) {
    gOrder.clear();

    schedule_t sch;

    sch.add_stage<TestStage::A>()
       .add_stage<TestStage::B>()
       .add_stage<TestStage::C>();

    sch.add_system<TestStage::A>(+[](){ gOrder.push_back(1); })
       .add_system<TestStage::B>(+[](){ gOrder.push_back(2); })
       .add_system<TestStage::C>(+[](){ gOrder.push_back(3); });

    registry reg;
    uint64_t tick = 0;
    sch.run(reg, tick);

    ASSERT_EQ(gOrder.size(), 3u);
    EXPECT_EQ(gOrder[0], 1);
    EXPECT_EQ(gOrder[1], 2);
    EXPECT_EQ(gOrder[2], 3);
}

TEST(ScheduleTest, AddStageBeforeAfter) {
    gOrder.clear();

    schedule_t sch;

    sch.add_stage<TestStage::A>()
       .add_stage<TestStage::C>()
       .add_stage_before<TestStage::B, TestStage::C>()
       .add_stage_after<TestStage::D, TestStage::B>();

    sch.add_system<TestStage::A>(+[](){ gOrder.push_back(1); })
       .add_system<TestStage::B>(+[](){ gOrder.push_back(2); })
       .add_system<TestStage::D>(+[](){ gOrder.push_back(3); })
       .add_system<TestStage::C>(+[](){ gOrder.push_back(4); });
    registry reg;
    uint64_t tick = 0;
    sch.run(reg, tick);

    ASSERT_EQ(gOrder.size(), 4u);
    EXPECT_EQ(gOrder[0], 1);
    EXPECT_EQ(gOrder[1], 2);
    EXPECT_EQ(gOrder[2], 3);
    EXPECT_EQ(gOrder[3], 4);
}

TEST(ScheduleTest, InsertStageBehavior) {
    gOrder.clear();

    schedule_t sch;

    // Start with X and Y
    sch.add_stage<TestStage::X>()
       .add_stage<TestStage::Y>();

    // Insert Z with insert target Y (exists): Z should take Y's index
    sch.insert_stage<TestStage::Z, TestStage::Y>();

    // now, we can not add system into TestStage::Y
    sch.add_system<TestStage::X>(+[](){ gOrder.push_back(1); })
       .add_system<TestStage::Z>(+[](){ gOrder.push_back(2); });

    registry reg;
    uint64_t tick = 0;
    sch.run(reg, tick);

    ASSERT_EQ(gOrder.size(), 2u);
    EXPECT_EQ(gOrder[0], 1); // X
    EXPECT_EQ(gOrder[1], 2); // Z (at Y's previous position)
}

TEST(ScheduleTest, AddSystemOverloads) {
    gOrder.clear();

    schedule_t sch;

    auto sysA = +[](){ gOrder.push_back(1); };
    auto sysB = +[](){ gOrder.push_back(2); };

    internal::basic_system<registry> sys(sysB);

    sch.add_stage<TestStage::A>();

    // Func overload
    sch.add_system<TestStage::A>(sysA)
        .add_system<TestStage::A>(sys.after(sysA));

    registry reg;
    uint64_t tick = 0;
    sch.run(reg, tick);

    ASSERT_EQ(gOrder.size(), 2u);
    EXPECT_EQ(gOrder[0], 1);
    EXPECT_EQ(gOrder[1], 2);
}