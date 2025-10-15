// components.hpp - shared test component structs for unit_case tests

#pragma once

#include <string>

struct Position {
    int x, y;

    Position() : x(0), y(0) {}
    Position(int x, int y) : x(x), y(y) {}

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

struct Velocity {
    int vx, vy;

    Velocity() : vx(0), vy(0) {}
    Velocity(int vx, int vy) : vx(vx), vy(vy) {}

    bool operator==(const Velocity& other) const {
        return vx == other.vx && vy == other.vy;
    }
};

struct Health {
    int current, max;

    Health() : current(0), max(0) {}
    Health(int current, int max) : current(current), max(max) {}

    bool operator==(const Health& other) const {
        return current == other.current && max == other.max;
    }
};

struct PlayerAttribute {
    int value;
    std::string name;

    PlayerAttribute() : value(0), name("default") {}
    PlayerAttribute(int v, const std::string& n) : value(v), name(n) {}

    bool operator==(const PlayerAttribute& other) const {
        return value == other.value && name == other.name;
    }
};


