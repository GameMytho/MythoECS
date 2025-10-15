// resources.hpp - shared test resource structs for unit_case tests

#pragma once

#include <string>
#include <vector>

struct GameConfig {
    int value;
    std::string name;

    GameConfig(int v, const std::string& n) : value(v), name(n) {}
    GameConfig() : value(0), name("default") {}

    bool operator==(const GameConfig& other) const {
        return value == other.value && name == other.name;
    }
};

struct PhysicsSettings {
    float data;
    bool active;

    PhysicsSettings(float d, bool a) : data(d), active(a) {}
    PhysicsSettings() : data(0.0f), active(false) {}

    bool operator==(const PhysicsSettings& other) const {
        return data == other.data && active == other.active;
    }
};

struct LevelData {
    std::vector<int> numbers;
    std::string description;

    LevelData(const std::vector<int>& nums, const std::string& desc)
        : numbers(nums), description(desc) {}
    LevelData() : numbers(), description("") {}

    bool operator==(const LevelData& other) const {
        return numbers == other.numbers && description == other.description;
    }
};


