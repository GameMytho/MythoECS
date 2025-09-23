/*
 * Unit Tests for event_storage.hpp - Event Storage Testing
 * 
 * This test suite validates the basic_event_storage functionality in event_storage.hpp:
 * - write: Write events to storage with different types
 * - mutate: Get mutable event set for modification
 * - read: Get const event set for reading
 * - swap: Swap storage with another instance
 * - clear: Clear all events and free memory
 * - size/empty: Get size and check empty state
 * - pool/destroy_funcs: Access internal storage
 * 
 * Test Cases:
 * 1. BasicOperations - Tests basic event storage operations
 * 2. WriteAndRead - Tests writing and reading different event types
 * 3. MutateOperations - Tests mutable event set operations
 * 4. MemoryManagement - Tests memory allocation and deallocation
 * 5. SwapAndClear - Tests swap and clear operations
 * 6. RandomOperations - Tests random operations and data integrity
 */

#include <gtest/gtest.h>
#include <container/event_storage.hpp>
#include <random>
#include <vector>
#include <algorithm>
#include <memory>

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

struct ComplexEvent {
    std::vector<int> data;
    std::string description;
    
    ComplexEvent(const std::vector<int>& d = {}, const std::string& desc = "") 
        : data(d), description(desc) {}
    
    bool operator==(const ComplexEvent& other) const {
        return data == other.data && description == other.description;
    }
};

/*
 * ======================================== Test Cases ========================================
 */

// Test basic event storage operations
TEST(EventStorageTest, BasicOperations) {
    basic_event_storage<> event_storage;
    EXPECT_EQ(event_storage.size(), 0);
    EXPECT_TRUE(event_storage.empty());

    event_storage.write<TestEvent>(1, 10.5f);
    EXPECT_EQ(event_storage.size(), 1); // Only one type, so size is 1
    EXPECT_FALSE(event_storage.empty());

    event_storage.write<AnotherEvent>("test", true);
    EXPECT_EQ(event_storage.size(), 2); // Two types now
    EXPECT_EQ(event_storage.pool().size(), 2);
    EXPECT_EQ(event_storage.destroy_funcs().size(), 2);
}

// Test writing and reading different event types
TEST(EventStorageTest, WriteAndRead) {
    basic_event_storage<> event_storage;

    event_storage.write<TestEvent>(1, 10.5f);
    event_storage.write<TestEvent>(2, 20.5f);
    event_storage.write<TestEvent>(3, 30.5f);

    event_storage.write<AnotherEvent>("first", true);
    event_storage.write<AnotherEvent>("second", false);

    {
        auto test_event_set = event_storage.read<TestEvent>();
        EXPECT_EQ(test_event_set.size(), 3);

        std::vector<TestEvent> test_events;
        for (const auto& event : test_event_set) {
            test_events.push_back(event);
        }

        EXPECT_EQ(test_events.size(), 3);
        EXPECT_TRUE(std::find(test_events.begin(), test_events.end(), TestEvent(1, 10.5f)) != test_events.end());
        EXPECT_TRUE(std::find(test_events.begin(), test_events.end(), TestEvent(2, 20.5f)) != test_events.end());
        EXPECT_TRUE(std::find(test_events.begin(), test_events.end(), TestEvent(3, 30.5f)) != test_events.end());    
    }
    
    {
        auto another_event_set = event_storage.read<AnotherEvent>();
        EXPECT_EQ(another_event_set.size(), 2);

        std::vector<AnotherEvent> another_events;
        for (const auto& event : another_event_set) {
            another_events.push_back(event);
        }

        EXPECT_EQ(another_events.size(), 2);
        EXPECT_TRUE(std::find(another_events.begin(), another_events.end(), AnotherEvent("first", true)) != another_events.end());
        EXPECT_TRUE(std::find(another_events.begin(), another_events.end(), AnotherEvent("second", false)) != another_events.end());
    }

    {
        auto empty_event_set = event_storage.read<ComplexEvent>();
        EXPECT_EQ(empty_event_set.size(), 0);
        EXPECT_TRUE(empty_event_set.empty());
    }
}

// Test mutable event set operations
TEST(EventStorageTest, MutateOperations) {
    basic_event_storage<> event_storage;

    event_storage.write<TestEvent>(1, 10.5f);
    event_storage.write<TestEvent>(2, 20.5f);

    {
        auto mutable_event_set = event_storage.mutate<TestEvent>();
        EXPECT_EQ(mutable_event_set.size(), 2);

        std::vector<TestEvent> events;
        for (const auto& event : mutable_event_set) {
            events.push_back(event);
        }

        EXPECT_EQ(events.size(), 2);
        EXPECT_TRUE(std::find(events.begin(), events.end(), TestEvent(1, 10.5f)) != events.end());
        EXPECT_TRUE(std::find(events.begin(), events.end(), TestEvent(2, 20.5f)) != events.end());
    
    }

    {
        auto empty_mutable_set = event_storage.mutate<ComplexEvent>();
        EXPECT_EQ(empty_mutable_set.size(), 0);
        EXPECT_TRUE(empty_mutable_set.empty());
    }
}

// Test memory management and cleanup
TEST(EventStorageTest, MemoryManagement) {
    basic_event_storage<> event_storage;

    event_storage.write<ComplexEvent>(std::vector<int>{1, 2, 3}, "test1");
    event_storage.write<ComplexEvent>(std::vector<int>{4, 5, 6}, "test2");
    event_storage.write<TestEvent>(42, 99.9f);

    EXPECT_EQ(event_storage.size(), 3); // Two different types, but ComplexEvent's id is 3, so size is 3

    {
        auto complex_event_set = event_storage.read<ComplexEvent>();
        EXPECT_EQ(complex_event_set.size(), 2);

        std::vector<ComplexEvent> complex_events;
        for (const auto& event : complex_event_set) {
            complex_events.push_back(event);
        }
    
        EXPECT_EQ(complex_events.size(), 2);
        EXPECT_TRUE(std::find(complex_events.begin(), complex_events.end(), ComplexEvent({1, 2, 3}, "test1")) != complex_events.end());
        EXPECT_TRUE(std::find(complex_events.begin(), complex_events.end(), ComplexEvent({4, 5, 6}, "test2")) != complex_events.end());
    }

    {
        auto test_event_set = event_storage.read<TestEvent>();
        EXPECT_EQ(test_event_set.size(), 1);

        std::vector<TestEvent> test_events;
        for (const auto& event : test_event_set) {
            test_events.push_back(event);
        }
    
        EXPECT_EQ(test_events.size(), 1);
        EXPECT_TRUE(std::find(test_events.begin(), test_events.end(), TestEvent(42, 99.9f)) != test_events.end());
    }
}

// Test swap and clear operations
TEST(EventStorageTest, SwapAndClear) {
    basic_event_storage<> event_storage1;
    basic_event_storage<> event_storage2;

    event_storage1.write<TestEvent>(1, 10.5f);
    event_storage1.write<TestEvent>(2, 20.5f);
    event_storage1.write<AnotherEvent>("test1", true);
    EXPECT_EQ(event_storage1.size(), 2);
    EXPECT_FALSE(event_storage1.empty());

    event_storage2.write<TestEvent>(3, 30.5f);
    event_storage2.write<ComplexEvent>(std::vector<int>{1, 2, 3}, "test2");
    EXPECT_EQ(event_storage2.size(), 3); // Two different types, but ComplexEvent's id is 3, so size is 3
    EXPECT_FALSE(event_storage2.empty());

    event_storage1.swap(event_storage2);
    EXPECT_EQ(event_storage1.size(), 3);
    EXPECT_EQ(event_storage2.size(), 2);

    {
        auto test_event_set = event_storage1.read<TestEvent>();
        EXPECT_EQ(test_event_set.size(), 1);

        std::vector<TestEvent> test_events;
        for (const auto& event : test_event_set) {
            test_events.push_back(event);
        }

        EXPECT_EQ(test_events.size(), 1);
        EXPECT_TRUE(std::find(test_events.begin(), test_events.end(), TestEvent(3, 30.5f)) != test_events.end());
    }

    {
        auto complex_event_set = event_storage1.read<ComplexEvent>();
        EXPECT_EQ(complex_event_set.size(), 1);

        std::vector<ComplexEvent> complex_events;
        for (const auto& event : complex_event_set) {
            complex_events.push_back(event);
        }

        EXPECT_EQ(complex_events.size(), 1);
        EXPECT_TRUE(std::find(complex_events.begin(), complex_events.end(), ComplexEvent({1, 2, 3}, "test2")) != complex_events.end());
    }

    {
        auto test_event_set = event_storage2.read<TestEvent>();
        EXPECT_EQ(test_event_set.size(), 2);

        std::vector<TestEvent> test_events;
        for (const auto& event : test_event_set) {
            test_events.push_back(event);
        }

        EXPECT_EQ(test_events.size(), 2);
        EXPECT_TRUE(std::find(test_events.begin(), test_events.end(), TestEvent(1, 10.5f)) != test_events.end());
        EXPECT_TRUE(std::find(test_events.begin(), test_events.end(), TestEvent(2, 20.5f)) != test_events.end());
    }

    {
        auto another_event_set = event_storage2.read<AnotherEvent>();
        EXPECT_EQ(another_event_set.size(), 1);

        std::vector<AnotherEvent> another_events;
        for (const auto& event : another_event_set) {
            another_events.push_back(event);
        }

        EXPECT_EQ(another_events.size(), 1);
        EXPECT_TRUE(std::find(another_events.begin(), another_events.end(), AnotherEvent("test1", true)) != another_events.end());
    }

    event_storage1.clear();
    EXPECT_EQ(event_storage1.size(), 0);
    EXPECT_TRUE(event_storage1.empty());

    event_storage2.clear();
    EXPECT_EQ(event_storage2.size(), 0);
    EXPECT_TRUE(event_storage2.empty());
}

// Test random operations and data integrity
TEST(EventStorageTest, RandomOperations) {
    basic_event_storage<> event_storage;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> id_dist(1, 100);
    std::uniform_real_distribution<float> value_dist(0.0f, 100.0f);
    std::uniform_int_distribution<int> type_dist(0, 2);
    std::uniform_int_distribution<int> operation_dist(0, 2);

    std::vector<TestEvent> expected_test_events;
    std::vector<AnotherEvent> expected_another_events;
    std::vector<ComplexEvent> expected_complex_events;

    for (int round = 0; round < 100; ++round) {
        int operation = operation_dist(gen);
        int event_type = type_dist(gen);

        switch (operation) {
            case 0: { // Write operation
                switch (event_type) {
                    case 0: {
                        int id = id_dist(gen);
                        float value = value_dist(gen);
                        event_storage.write<TestEvent>(id, value);
                        expected_test_events.emplace_back(id, value);
                        break;
                    }
                    case 1: {
                        std::string name = "event_" + std::to_string(id_dist(gen));
                        bool active = (id_dist(gen) % 2) == 0;
                        event_storage.write<AnotherEvent>(name, active);
                        expected_another_events.emplace_back(name, active);
                        break;
                    }
                    case 2: {
                        std::vector<int> data = {id_dist(gen), id_dist(gen), id_dist(gen)};
                        std::string desc = "complex_" + std::to_string(id_dist(gen));
                        event_storage.write<ComplexEvent>(data, desc);
                        expected_complex_events.emplace_back(data, desc);
                        break;
                    }
                }
                break;
            }
            case 1: { // Read operation
                {
                    auto test_event_set = event_storage.read<TestEvent>();
                    EXPECT_EQ(test_event_set.size(), expected_test_events.size());
                    for (const auto& event : test_event_set) {
                        EXPECT_TRUE(std::find(expected_test_events.begin(), expected_test_events.end(), event) != expected_test_events.end());
                    }
                }

                {
                    auto another_event_set = event_storage.read<AnotherEvent>();
                    EXPECT_EQ(another_event_set.size(), expected_another_events.size());
                    for (const auto& event : another_event_set) {
                        EXPECT_TRUE(std::find(expected_another_events.begin(), expected_another_events.end(), event) != expected_another_events.end());
                    }
                }

                {
                    auto complex_event_set = event_storage.read<ComplexEvent>();
                    EXPECT_EQ(complex_event_set.size(), expected_complex_events.size());
                    for (const auto& event : complex_event_set) {
                        EXPECT_TRUE(std::find(expected_complex_events.begin(), expected_complex_events.end(), event) != expected_complex_events.end());
                    }
                }

                break;
            }
            case 2: { // Mutate operation
                {
                    auto test_event_set = event_storage.mutate<TestEvent>();
                    EXPECT_EQ(test_event_set.size(), expected_test_events.size());
                    for (const auto& event : test_event_set) {
                        EXPECT_TRUE(std::find(expected_test_events.begin(), expected_test_events.end(), event) != expected_test_events.end());
                    }
                }

                {
                    auto another_event_set = event_storage.mutate<AnotherEvent>();
                    EXPECT_EQ(another_event_set.size(), expected_another_events.size());
                    for (const auto& event : another_event_set) {
                        EXPECT_TRUE(std::find(expected_another_events.begin(), expected_another_events.end(), event) != expected_another_events.end());
                    }
                }

                {
                    auto complex_event_set = event_storage.mutate<ComplexEvent>();
                    EXPECT_EQ(complex_event_set.size(), expected_complex_events.size());
                    for (const auto& event : complex_event_set) {
                        EXPECT_TRUE(std::find(expected_complex_events.begin(), expected_complex_events.end(), event) != expected_complex_events.end());
                    }
                }

                break;
            }
        }
    }

    EXPECT_EQ(event_storage.size(), 3); // Three different types

    {
        auto final_test_event_set = event_storage.read<TestEvent>();
        EXPECT_EQ(final_test_event_set.size(), expected_test_events.size());
        for (const auto& event : final_test_event_set) {
            EXPECT_TRUE(std::find(expected_test_events.begin(), expected_test_events.end(), event) != expected_test_events.end());
        }
    }

    {
        auto final_another_event_set = event_storage.read<AnotherEvent>();
        EXPECT_EQ(final_another_event_set.size(), expected_another_events.size());
        for (const auto& event : final_another_event_set) {
            EXPECT_TRUE(std::find(expected_another_events.begin(), expected_another_events.end(), event) != expected_another_events.end());
        }
    }

    {
        auto final_complex_event_set = event_storage.read<ComplexEvent>();
        EXPECT_EQ(final_complex_event_set.size(), expected_complex_events.size());
        for (const auto& event : final_complex_event_set) {
            EXPECT_TRUE(std::find(expected_complex_events.begin(), expected_complex_events.end(), event) != expected_complex_events.end());
        }
    }
}
