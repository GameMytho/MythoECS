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

struct Name {
    std::string value;
};

struct Health {
    uint32_t value;
};

void startup(commands cmds) {
    cmds.spawn(Position{0.1f, 0.1f});
    cmds.spawn(Position{0.2f, 0.2f}, Vectory{0.2f, 0.2f});
    cmds.spawn(Position{0.3f, 0.3f}, Vectory{0.3f, 0.3f}, Direction{0.3f, 0.3f});
}

void update_added1(querier<Position, added<Position>> q) {
    EXPECT_EQ(q.size(), 3);

    uint32_t count = 0;
    for (auto [pos] : q) {
        count++;

        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, const data_wrapper<Position>>), true);
        EXPECT_EQ(pos->x, 0.1f * count);
        EXPECT_EQ(pos->y, 0.1f * count);
    }
}

void update_added2(querier<Vectory, added<Vectory>> q) {
    EXPECT_EQ(q.size(), 2);

    uint32_t count = 1;
    for (auto [vec] : q) {
        count++;

        using vec_type = decltype(vec);
        EXPECT_EQ((std::is_same_v<vec_type, const data_wrapper<Vectory>>), true);
        EXPECT_EQ(vec->x, 0.1f * count);
        EXPECT_EQ(vec->y, 0.1f * count);
    }
}

void update_added3(querier<Direction, added<Position, Vectory, Direction>> q) {
    EXPECT_EQ(q.size(), 1);

    for (auto [dir] : q) {
        using dir_type = decltype(dir);
        EXPECT_EQ((std::is_same_v<dir_type, const data_wrapper<Direction>>), true);
        EXPECT_EQ(dir->x, 0.3f);
        EXPECT_EQ(dir->y, 0.3f);
    }
}

void update1(querier<entity, Position, with<Vectory>, without<Direction>> q) {
    EXPECT_EQ(q.size(), 1);

    for (auto [entt, pos] : q) {
        using entt_type = decltype(entt);
        EXPECT_EQ((std::is_same_v<entt_type, const data_wrapper<entity>>), true);
        EXPECT_EQ(entt->id(), 1);
        EXPECT_EQ(entt->version(), 0);

        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, const data_wrapper<Position>>), true);
        EXPECT_EQ(pos->x, 0.2f);
        EXPECT_EQ(pos->y, 0.2f);
    }
}

void update2(querier<mut<Position>, Vectory, with<Direction>, without<Name, Health>> q) {
    EXPECT_EQ(q.size(), 1);

    for (auto [pos, vec] : q) {
        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, data_wrapper<Position>>), true);
        EXPECT_EQ(pos->x, 0.3f);
        EXPECT_EQ(pos->y, 0.3f);

        pos->x = 0.4;
        pos->y = 0.4;

        using vec_type = decltype(vec);
        EXPECT_EQ((std::is_same_v<vec_type, const data_wrapper<Vectory>>), true);
        EXPECT_EQ(vec->x, 0.3f);
        EXPECT_EQ(vec->y, 0.3f);
    }
}

void update2_changed(querier<entity, Position, changed<Position>> q) {
    EXPECT_EQ(q.size(), 3);

    for (auto [e, pos] : q) {
        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, const data_wrapper<Position>>), true);

        if (e->id() == 0) {
            EXPECT_EQ(pos->x, 0.1f);
            EXPECT_EQ(pos->y, 0.1f);
        } else if (e->id() == 1) {
            EXPECT_EQ(pos->x, 0.2f);
            EXPECT_EQ(pos->y, 0.2f);
        } else if (e->id() == 2) {
            EXPECT_EQ(pos->x, 0.4f);
            EXPECT_EQ(pos->y, 0.4f);
        } else {
            FAIL() << "This should not be reached";
        }
    }
}

void update3(querier<Position, mut<Vectory>, with<Vectory, Direction>, without<Name, Health>> q) {
    EXPECT_EQ(q.size(), 1);

    for (auto [pos, vec] : q) {
        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, const data_wrapper<Position>>), true);
        EXPECT_EQ(pos->x, 0.4f);
        EXPECT_EQ(pos->y, 0.4f);

        using vec_type = decltype(vec);
        EXPECT_EQ((std::is_same_v<vec_type, data_wrapper<Vectory>>), true);
        EXPECT_EQ(vec->x, 0.3f);
        EXPECT_EQ(vec->y, 0.3f);

        vec->x *= 2;
        vec->y *= 2;
    }
}

void update3_changed(querier<entity, Vectory, changed<Vectory>> q) {
    EXPECT_EQ(q.size(), 2);

    for (auto [e, vec] : q) {
        using vec_type = decltype(vec);
        EXPECT_EQ((std::is_same_v<vec_type, const data_wrapper<Vectory>>), true);

        if (e->id() == 1) {
            EXPECT_EQ(vec->x, 0.2f);
            EXPECT_EQ(vec->y, 0.2f);
        } else if (e->id() == 2) {
            EXPECT_EQ(vec->x, 0.6f);
            EXPECT_EQ(vec->y, 0.6f);
        } else {
            FAIL() << "This should not be reached";
        }
    }
}

void update4(querier<Vectory, with<Position, Direction>, without<Name, Health>> q) {
    EXPECT_EQ(q.size(), 1);

    for (auto [vec] : q) {
        using vec_type = decltype(vec);
        EXPECT_EQ((std::is_same_v<vec_type, const data_wrapper<Vectory>>), true);
        EXPECT_EQ(vec->x, 0.6f);
        EXPECT_EQ(vec->y, 0.6f);
    }
}

TEST(SystemTest, AddAndRunTest) {
    registry reg;

    reg.add_startup_system(startup)
       .add_update_system(update1);

    reg.ready();

    reg.startup();

    reg.update();
}

TEST(SystemTest, SystemConfigTest) {
    registry reg;

    reg.add_startup_system(startup)
       .add_update_system(system(update_added1).before(update_added2))
       .add_update_system(system(update_added2).before(update_added3))
       .add_update_system(system(update_added3).before(update1).after(update_added1))
       .add_update_system(system(update1).after(update_added2).before(update2))
       .add_update_system(system(update2).after(update1).before(update2_changed))
       .add_update_system(system(update2_changed).after(update2))
       .add_update_system(system(update3).after(update2_changed).before(update3_changed))
       .add_update_system(system(update3_changed).after(update3))
       .add_update_system(system(update4).after(update3_changed));

    reg.ready();

    reg.startup();

    reg.update();
}

TEST(SystemTest, SystemRunifTest) {
    registry reg;

    reg.add_startup_system(startup)
       .add_update_system(system(update1).before(update2).runif(components_added<Position, Vectory>))
       .add_update_system(system(update2).before(update3).runif(components_added<Position, Vectory, Direction>))
       .add_update_system(system(update3).before(update4).runif(components_changed<Position>))
       .add_update_system(system(update4).runif(components_changed<Vectory>));

    reg.ready();

    reg.startup();

    reg.update();
}