// events.hpp - shared test event structs for unit_case tests

#pragma once

#include <string>
#include <vector>

struct event_genor final {};

struct DamageEvent {
    int id;
    float value;

    DamageEvent(int i = 0, float v = 0.0f) : id(i), value(v) {}

    bool operator==(const DamageEvent& other) const {
        return id == other.id && value == other.value;
    }
};

struct StatusEvent {
    std::string name;
    bool active;

    StatusEvent(const std::string& n = "", bool a = false) : name(n), active(a) {}

    bool operator==(const StatusEvent& other) const {
        return name == other.name && active == other.active;
    }
};

struct BulkEvent {
    std::vector<int> data;
    std::string description;

    BulkEvent(const std::vector<int>& d = {}, const std::string& desc = "")
        : data(d), description(desc) {}

    bool operator==(const BulkEvent& other) const {
        return data == other.data && description == other.description;
    }
};


