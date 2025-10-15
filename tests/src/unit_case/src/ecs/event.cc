/*
 * Unit Tests for event.hpp - Events & Wrappers Testing
 * 
 * This test suite validates:
 * - basic_events: write/swap/read pipeline and mutate/read consistency
 * - basic_event_writer: forwards to registry-like target
 * - basic_event_mutator/basic_event_reader: wrapper access
 * - traits: is_event_writer_v/is_event_mutator_v/is_event_reader_v
 * 
 * Events Test Cases:
 * 1. BasicEventsOperation - Validate basic events operations
 * 
 * Event Writer/Mutator/Reader Test Cases:
 * 1. EventWriter - Validate event writer wrapper forwards writes
 * 2. EventMutator - Validate event mutator wrapper forwards mutates
 * 3. EventReader - Validate event reader wrapper forwards reads
 * 
 * Event Traits Test Cases:
 * 1. EventTraits - Validate event wrapper traits
 */

#include <gtest/gtest.h>
#include <ecs/event.hpp>
#include <ecs/registry.hpp>

using namespace mytho::ecs;

/*
 * ======================================== Helper Structures/Functions ==================================
 */

#include "events.hpp"

using entity = basic_entity<uint32_t, uint16_t>;
using registry = basic_registry<entity, uint8_t, uint8_t, uint8_t, 1024>;

/*
 * ======================================== Events Test Cases ======================================
 */

TEST(EventsTest, BasicEventsOperation) {
    basic_events<size_t> ev;

    ev.write<DamageEvent>(1, 1.5f);
    ev.write<DamageEvent>(2, 2.5f);

    ev.swap();

    auto mut = ev.mutate<DamageEvent>();
    ASSERT_EQ(mut.size(), 2);
    for (auto it = mut.begin(); it != mut.end(); ++it) {
        it->value += 10.0f;
    }

    auto r = ev.read<DamageEvent>();
    ASSERT_EQ(r.size(), 2);
    auto rit = r.begin();
    EXPECT_EQ(rit->id, 1);EXPECT_FLOAT_EQ(rit->value, 11.5f); ++rit;
    EXPECT_EQ(rit->id, 2); EXPECT_FLOAT_EQ(rit->value, 12.5f);

    ev.write<DamageEvent>(3, 3.0f);
    EXPECT_EQ(ev.read<DamageEvent>().size(), 2);

    ev.swap();

    auto r2 = ev.read<DamageEvent>();
    ASSERT_EQ(r2.size(), 1);
    auto it2 = r2.begin();
    EXPECT_EQ(it2->id, 3); EXPECT_FLOAT_EQ(it2->value, 3.0f);
}

/*
 * ======================================== Event Writer/Reader/Mutator Test Cases =========================================
 */

TEST(EventsTest, EventWriter) {
    registry reg;
    basic_event_writer<registry, DamageEvent> writer(reg);

    writer.write(7, 9.5f);
    EXPECT_EQ(reg.event_read<DamageEvent>().size(), 0); // still in write-queue

    reg.update(); // triggers _events.swap()

    auto r3 = reg.event_read<DamageEvent>();
    ASSERT_EQ(r3.size(), 1);
    for (auto& h : r3) { EXPECT_EQ(h.id, 7); EXPECT_FLOAT_EQ(h.value, 9.5f); }
}

TEST(EventsTest, EventMutator) {
    registry reg;

    reg.event_write<DamageEvent>(1, 1.5f);
    reg.event_write<DamageEvent>(2, 2.5f);
    reg.update();

    auto set_mut = reg.event_mutate<DamageEvent>();

    basic_event_mutator<DamageEvent> mutator(set_mut);

    auto& mut_view = mutator.mutate();

    ASSERT_EQ(mut_view.size(), 2);

    for (auto it = mut_view.begin(); it != mut_view.end(); ++it) {
        it->value += 10.0f;
    }

    auto rj = reg.event_read<DamageEvent>().begin();
    EXPECT_EQ(rj->id, 1); EXPECT_FLOAT_EQ(rj->value, 11.5f); ++rj;
    EXPECT_EQ(rj->id, 2); EXPECT_FLOAT_EQ(rj->value, 12.5f);
}

TEST(EventsTest, EventReader) {
    registry reg;

    reg.event_write<DamageEvent>(5, 1.0f);
    reg.event_write<DamageEvent>(6, 2.0f);
    reg.update();

    auto set_read = reg.event_read<DamageEvent>();
    basic_event_reader<DamageEvent> reader(set_read);

    const auto& view = reader.read();
    ASSERT_EQ(view.size(), 2);
    auto it = view.begin();
    EXPECT_EQ(it->id, 5); EXPECT_FLOAT_EQ(it->value, 1.0f); ++it;
    EXPECT_EQ(it->id, 6); EXPECT_FLOAT_EQ(it->value, 2.0f);
}

/*
 * ======================================== Trait Test Cases ============================================
 */

TEST(EventsTest, EventTraits) {
    using Writer = basic_event_writer<registry, DamageEvent>;
    using Mut = basic_event_mutator<DamageEvent>;
    using Reader = basic_event_reader<DamageEvent>;

    static_assert(mytho::utils::is_event_writer_v<Writer>, "Writer should satisfy is_event_writer_v");
    static_assert(mytho::utils::is_event_mutator_v<Mut>, "Mutator should satisfy is_event_mutator_v");
    static_assert(mytho::utils::is_event_reader_v<Reader>, "Reader should satisfy is_event_reader_v");

    EXPECT_TRUE(mytho::utils::is_event_writer_v<Writer>);
    EXPECT_TRUE(mytho::utils::is_event_mutator_v<Mut>);
    EXPECT_TRUE(mytho::utils::is_event_reader_v<Reader>);

    EXPECT_FALSE(mytho::utils::is_event_writer_v<int>);
    EXPECT_FALSE(mytho::utils::is_event_mutator_v<int>);
    EXPECT_FALSE(mytho::utils::is_event_reader_v<int>);
}