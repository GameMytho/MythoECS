#include <gtest/gtest.h>
#include "ecs/ecs.hpp"

struct Position {
    float x;
    float y;
};

struct Vectory {
    float x;
    float y;
};

struct Direction {
    float x;
    float y;
};

void startup(commands cmds, querier<entity, Position, Vectory, Direction> q) {
    cmds.spawn(Position{1.0f, 2.0f}, Vectory{3.0f, 4.0f}, Direction{5.0f, 6.0f});

    EXPECT_EQ(q.size(), 0);
}

void update(commands cmds, querier<entity, Position, Vectory, Direction> q) {
    cmds.spawn(Position{10.0f, 20.0f}, Vectory{30.0f, 40.0f}, Direction{50.0f, 60.0f});

    EXPECT_EQ(q.size(), 1);

    for (auto [e, pos, vec, dir] : q) {
        EXPECT_EQ(e->id(), 0);
        EXPECT_EQ(e->version(), 0);
        EXPECT_EQ(pos->x, 1.0f);
        EXPECT_EQ(pos->y, 2.0f);
        EXPECT_EQ(vec->x, 3.0f);
        EXPECT_EQ(vec->y, 4.0f);
        EXPECT_EQ(dir->x, 5.0f);
        EXPECT_EQ(dir->y, 6.0f);

        cmds.replace(*e, Position{100.0f, 200.0f});
    }
}

void update_added(querier<Position, Vectory, Direction, added<Position>> q) {
    EXPECT_EQ(q.size(), 1);

    for (auto [pos, vec, dir] : q) {
        EXPECT_EQ(pos->x, 1.0f);
        EXPECT_EQ(pos->y, 2.0f);
        EXPECT_EQ(vec->x, 3.0f);
        EXPECT_EQ(vec->y, 4.0f);
        EXPECT_EQ(dir->x, 5.0f);
        EXPECT_EQ(dir->y, 6.0f);
    }
}

void shutdown(commands cmds, querier<entity, Position, Vectory, Direction> q) {
    EXPECT_EQ(q.size(), 2);

    for (auto [e, pos, vec, dir] : q) {
        if (e->id() == 0) {
            EXPECT_EQ(e->version(), 0);
            EXPECT_EQ(pos->x, 100.0f);
            EXPECT_EQ(pos->y, 200.0f);
            EXPECT_EQ(vec->x, 3.0f);
            EXPECT_EQ(vec->y, 4.0f);
            EXPECT_EQ(dir->x, 5.0f);
            EXPECT_EQ(dir->y, 6.0f);
        } else if (e->id() == 1) {
            EXPECT_EQ(e->version(), 0);
            EXPECT_EQ(pos->x, 10.0f);
            EXPECT_EQ(pos->y, 20.0f);
            EXPECT_EQ(vec->x, 30.0f);
            EXPECT_EQ(vec->y, 40.0f);
            EXPECT_EQ(dir->x, 50.0f);
            EXPECT_EQ(dir->y, 60.0f);
        } else {
            FAIL() << "This should not be reached";
        }
    }
}

void shutdown_added(querier<entity, Position, Vectory, Direction, added<Position>> q) {
    EXPECT_EQ(q.size(), 2);

    for (auto [e, pos, vec, dir] : q) {
        if (e->id() == 0) {
            EXPECT_EQ(pos->x, 100.0f);
            EXPECT_EQ(pos->y, 200.0f);
            EXPECT_EQ(vec->x, 3.0f);
            EXPECT_EQ(vec->y, 4.0f);
            EXPECT_EQ(dir->x, 5.0f);
            EXPECT_EQ(dir->y, 6.0f);
        } else if (e->id() == 1) {
            EXPECT_EQ(pos->x, 10.0f);
            EXPECT_EQ(pos->y, 20.0f);
            EXPECT_EQ(vec->x, 30.0f);
            EXPECT_EQ(vec->y, 40.0f);
            EXPECT_EQ(dir->x, 50.0f);
            EXPECT_EQ(dir->y, 60.0f);
        } else {
            FAIL() << "This should not be reached";
        }
    }
}

void shutdown_changed(querier<entity, Position, Vectory, Direction, changed<Position>> q) {
    EXPECT_EQ(q.size(), 2);

    for (auto [e, pos, vec, dir] : q) {
        if (e->id() == 0) {
            EXPECT_EQ(pos->x, 100.0f);
            EXPECT_EQ(pos->y, 200.0f);
            EXPECT_EQ(vec->x, 3.0f);
            EXPECT_EQ(vec->y, 4.0f);
            EXPECT_EQ(dir->x, 5.0f);
            EXPECT_EQ(dir->y, 6.0f);
        } else if (e->id() == 1) {
            EXPECT_EQ(pos->x, 10.0f);
            EXPECT_EQ(pos->y, 20.0f);
            EXPECT_EQ(vec->x, 30.0f);
            EXPECT_EQ(vec->y, 40.0f);
            EXPECT_EQ(dir->x, 50.0f);
            EXPECT_EQ(dir->y, 60.0f);
        } else {
            FAIL() << "This should not be reached";
        }
    }
}

TEST(CommandsTest, CommandsQueueTest) {
    registry reg;

    reg.add_startup_system(startup);
    reg.add_update_system(system(update).before(update_added));
    reg.add_update_system(update_added);
    reg.add_shutdown_system(system(shutdown).before(shutdown_added));
    reg.add_shutdown_system(system(shutdown_added).before(shutdown_changed));
    reg.add_shutdown_system(shutdown_changed);

    reg.ready();

    reg.startup();

    reg.update();

    reg.shutdown();
}