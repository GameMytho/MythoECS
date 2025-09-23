/*
 * Unit Tests for event_set.hpp - Event Set Testing
 * 
 * This test suite validates the basic_event_set functionality in event_set.hpp:
 * - Constructor: Default, with data reference, with const data reference
 * - Iterators: begin, end, iterator operations
 * - Size operations: size, empty
 * - Iterator functionality: dereference, increment, decrement, arithmetic, comparison
 * 
 * Test Cases:
 * 1. BasicOperations - Tests basic event set operations and state
 * 2. IteratorOperations - Tests iterator functionality and operations
 * 3. ConstIteratorOperations - Tests const iterator functionality
 * 4. EdgeCases - Tests edge cases and boundary conditions
 * 5. RandomOperations - Tests random operations and data integrity
 */

#include <gtest/gtest.h>
#include <container/event_set.hpp>
#include <random>
#include <vector>
#include <algorithm>

using namespace mytho::container;

/*
 * =============================== Helper Structures/Functions ===============================
 */

struct TestEvent {
    int id;
    float value;

    TestEvent(int i = 0, float v = 0.0f) : id(i), value(v) {}

    bool operator==(const TestEvent& other) const {
        return id == other.id && value == other.value;
    }
};

struct AnotherEvent {
    std::string name;
    bool active;

    AnotherEvent(const std::string& n = "", bool a = false) : name(n), active(a) {}

    bool operator==(const AnotherEvent& other) const {
        return name == other.name && active == other.active;
    }
};

/*
 * ======================================== Test Cases ========================================
 */

// Test basic event set operations and state
TEST(EventSetTest, BasicOperations) {
    basic_event_set<TestEvent>::events_data_type data;
    basic_event_set<TestEvent> event_set(data);

    EXPECT_EQ(event_set.size(), 0);
    EXPECT_TRUE(event_set.empty());

    TestEvent event1(1, 10.5f);
    TestEvent event2(2, 20.5f);
    TestEvent event3(3, 30.5f);

    data.push_back(static_cast<void*>(&event1));
    data.push_back(static_cast<void*>(&event2));
    data.push_back(static_cast<void*>(&event3));

    EXPECT_EQ(event_set.size(), 3);
    EXPECT_FALSE(event_set.empty());

    basic_event_set<AnotherEvent>::events_data_type another_data;
    basic_event_set<AnotherEvent> another_event_set(another_data);

    EXPECT_EQ(another_event_set.size(), 0);
    EXPECT_TRUE(another_event_set.empty());

    AnotherEvent another_event("test", true);
    another_data.push_back(static_cast<void*>(&another_event));

    EXPECT_EQ(another_event_set.size(), 1);
    EXPECT_FALSE(another_event_set.empty());
}

// Test iterator functionality and operations
TEST(EventSetTest, IteratorOperations) {
    basic_event_set<TestEvent>::events_data_type data;
    basic_event_set<TestEvent> event_set(data);

    EXPECT_EQ(event_set.begin(), event_set.end());

    TestEvent events[3] = {
        TestEvent(1, 10.5f),
        TestEvent(2, 20.5f),
        TestEvent(3, 30.5f)
    };

    for (int i = 0; i < 3; ++i) {
        data.push_back(static_cast<void*>(&events[i]));
    }

    auto it = event_set.begin();
    EXPECT_EQ((*it).id, 1);
    EXPECT_EQ((*it).value, 10.5f);
    EXPECT_EQ(it->id, 1);
    EXPECT_EQ(it->value, 10.5f);

    ++it;
    EXPECT_EQ((*it).id, 2);
    EXPECT_EQ((*it).value, 20.5f);

    it++;
    EXPECT_EQ((*it).id, 3);
    EXPECT_EQ((*it).value, 30.5f);

    ++it;
    EXPECT_EQ(it, event_set.end());

    --it;
    EXPECT_EQ((*it).id, 3);
    EXPECT_EQ((*it).value, 30.5f);

    it--;
    EXPECT_EQ((*it).id, 2);
    EXPECT_EQ((*it).value, 20.5f);

    it = event_set.begin();
    it += 2;
    EXPECT_EQ((*it).id, 3);
    EXPECT_EQ((*it).value, 30.5f);

    it -= 1;
    EXPECT_EQ((*it).id, 2);
    EXPECT_EQ((*it).value, 20.5f);

    auto it1 = event_set.begin();
    auto it2 = event_set.begin() + 1;
    auto it3 = event_set.begin() + 1;
    EXPECT_TRUE(it1 < it2);
    EXPECT_TRUE(it2 > it1);
    EXPECT_TRUE(it2 == it3);
    EXPECT_TRUE(it1 != it2);
    EXPECT_TRUE(it1 <= it2);
    EXPECT_TRUE(it2 >= it1);
    EXPECT_EQ(it2 - it1, 1);
    EXPECT_EQ(it1 - it2, -1);
    EXPECT_EQ(event_set.begin()[0].id, 1);
    EXPECT_EQ(event_set.begin()[1].id, 2);
    EXPECT_EQ(event_set.begin()[2].id, 3);

    std::vector<TestEvent> collected_events;
    for (const auto& event : event_set) {
        collected_events.push_back(event);
    }
    EXPECT_EQ(collected_events.size(), 3);
    EXPECT_EQ(collected_events[0].id, 1);
    EXPECT_EQ(collected_events[1].id, 2);
    EXPECT_EQ(collected_events[2].id, 3);
}

// Test const iterator and const methods
TEST(EventSetTest, ConstIteratorOperations) {
    basic_event_set<TestEvent>::events_data_type data;
    const basic_event_set<TestEvent> event_set(data);

    EXPECT_EQ(event_set.begin(), event_set.end());

    TestEvent events[2] = {
        TestEvent(1, 10.5f),
        TestEvent(2, 20.5f)
    };

    data.push_back(static_cast<void*>(&events[0]));
    data.push_back(static_cast<void*>(&events[1]));

    std::vector<TestEvent> collected_events;
    for (const auto& event : event_set) {
        collected_events.push_back(event);
    }

    EXPECT_EQ(collected_events.size(), 2);
    EXPECT_EQ(collected_events[0].id, 1);
    EXPECT_EQ(collected_events[1].id, 2);
    EXPECT_EQ(event_set.size(), 2);
    EXPECT_FALSE(event_set.empty());
}

// Test edge cases and boundary conditions
TEST(EventSetTest, EdgeCases) {
    {
        basic_event_set<TestEvent> empty_event_set;

        EXPECT_EQ(empty_event_set.size(), 0);
        EXPECT_TRUE(empty_event_set.empty());
        EXPECT_EQ(empty_event_set.begin(), empty_event_set.end());
    }

    {
        basic_event_set<TestEvent>::events_data_type single_data;
        basic_event_set<TestEvent> single_event_set(single_data);

        TestEvent single_event(42, 99.9f);
        single_data.push_back(static_cast<void*>(&single_event));

        EXPECT_EQ(single_event_set.size(), 1);
        EXPECT_FALSE(single_event_set.empty());
        EXPECT_NE(single_event_set.begin(), single_event_set.end());

        auto it = single_event_set.begin();
        EXPECT_EQ((*it).id, 42);
        EXPECT_EQ((*it).value, 99.9f);

        ++it;
        EXPECT_EQ(it, single_event_set.end());

        it = single_event_set.begin();
        EXPECT_EQ(it[0].id, 42);
        EXPECT_EQ(it + 1, single_event_set.end());
        EXPECT_EQ(single_event_set.end() - it, 1);
    }
}

// Test random operations and data integrity
TEST(EventSetTest, RandomOperations) {
    std::vector<TestEvent> events;
    basic_event_set<TestEvent>::events_data_type data;
    basic_event_set<TestEvent> event_set(data);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> id_dist(1, 1000);
    std::uniform_real_distribution<float> value_dist(0.0f, 100.0f);
    std::uniform_int_distribution<size_t> size_dist(1, 50);

    for (int round = 0; round < 100; ++round) {
        size_t num_events = size_dist(gen);
        data.clear();
        events.clear();

        events.reserve(num_events); // Ensure enough space to avoid reallocation, otherwise the pointer will be invalid
        for (size_t i = 0; i < num_events; ++i) {
            events.emplace_back(id_dist(gen), value_dist(gen));
            data.push_back(static_cast<void*>(&events[i]));
        }

        EXPECT_EQ(event_set.size(), num_events);
        EXPECT_EQ(event_set.empty(), num_events == 0);

        if (num_events > 0) {
            auto it = event_set.begin();
            auto end_it = event_set.end();

            EXPECT_NE(it, end_it);
            EXPECT_EQ(end_it - it, static_cast<std::ptrdiff_t>(num_events));

            if (num_events > 1) {
                size_t random_index = size_dist(gen) % num_events;
                EXPECT_EQ(it[random_index].id, events[random_index].id);
                EXPECT_EQ(it[random_index].value, events[random_index].value);
            }

            std::vector<TestEvent> collected_events;
            for (const auto& event : event_set) {
                collected_events.push_back(event);
            }

            EXPECT_EQ(collected_events.size(), num_events);
            for (size_t i = 0; i < num_events; ++i) {
                EXPECT_EQ(collected_events[i].id, events[i].id);
                EXPECT_EQ(collected_events[i].value, events[i].value);
            }
        }
    }
}
