/*
 * Unit Tests for querier.hpp - Querier & Helpers Testing
 * 
 * This test suite validates:
 * - basic_querier: iteration, tuple binding, types mapping (entity, const/mut)
 * - basic_removed_entities: iteration, size, empty state, const iteration
 * - helpers: traits/concepts/type transforms and internal meta composition
 * 
 * Querier Helpers Test Cases:
 * 1. QueryWrapperTraits - Verify wrapper traits for mut/with/without/added/changed
 * 2. Concepts - Verify QueryValueType and PureComponentType
 * 3. RemoveMut - Verify rm_mut_t mapping
 * 4. RemoveTemplate - Verify rm_with_t/rm_without_t/rm_changed_t
 * 5. TypeListConvert - Verify datatype/prototype list conversions
 * 6. QueryTypes - Verify internal::query_types composition results
 * 
 * Querier Test Cases:
 * 1. ConstructionAndIteration - Verify tuple binding and mut propagation
 * 2. QuerierDataTypeMapping - Verify component_bundle_type static mapping
 * 3. EmptyContainer - Verify empty container behavior
 * 
 * Removed Entities Test Cases:
 * 1. BasicOperations - Verify basic operations (size, empty, iteration)
 * 2. EmptyContainer - Verify empty container behavior
 * 3. ConstIteration - Verify const iteration functionality
 * 4. ModificationThroughReference - Verify modification through underlying vector
 * 5. TypeTraits - Verify type aliases and traits
 * 
 * Querier Trait Test Cases:
 * 1. IsQuerierTrait - Verify is_querier_v trait
 * 
 * Removed Entities Trait Test Cases:
 * 1. IsRemovedEntitiesTrait - Verify is_removed_entities_v trait
 */

#include <gtest/gtest.h>
#include <type_traits>
#include <ecs/registry.hpp>
#include <ecs/querier.hpp>

using namespace mytho::ecs;

/*
 * ======================================== Helper Structures/Functions ==================================
 */

#include "components.hpp"

using entity = basic_entity<uint32_t, uint16_t>;
using registry = basic_registry<entity, uint8_t, uint8_t, uint8_t, 1024>;

template<typename T>
using data_wrapper = mytho::utils::internal::data_wrapper<T>;

/*
 * ======================================== Querier Helpers Test Cases ===============================
 */

TEST(QuerierHelpersTest, QueryWrapperTraits) {
    // is_mut_v / is_with_v / is_without_v / is_added_v / is_changed_v
    EXPECT_TRUE((mytho::utils::is_mut_v<mut<Position>>));
    EXPECT_TRUE((mytho::utils::is_with_v<with<Position>>));
    EXPECT_TRUE((mytho::utils::is_without_v<without<Position>>));
    EXPECT_TRUE((mytho::utils::is_added_v<added<Position>>));
    EXPECT_TRUE((mytho::utils::is_changed_v<changed<Position>>));

    EXPECT_FALSE((mytho::utils::is_mut_v<Position>));
    EXPECT_FALSE((mytho::utils::is_with_v<Position>));
    EXPECT_FALSE((mytho::utils::is_without_v<Position>));
    EXPECT_FALSE((mytho::utils::is_added_v<Position>));
    EXPECT_FALSE((mytho::utils::is_changed_v<Position>));
}

TEST(QuerierHelpersTest, Concepts) {
    using mytho::utils::QueryValueType;
    using mytho::utils::PureComponentType;

    static_assert(QueryValueType<Position>);
    static_assert(QueryValueType<entity>);
    static_assert(!QueryValueType<const Position>);
    static_assert(!QueryValueType<Position&>);
    static_assert(!QueryValueType<Position*>);

    static_assert(PureComponentType<Position>);
    static_assert(!PureComponentType<mut<Position>>);
    static_assert(!PureComponentType<with<Position>>);
    static_assert(!PureComponentType<without<Position>>);
    static_assert(!PureComponentType<added<Position>>);
    static_assert(!PureComponentType<changed<Position>>);
}

TEST(QuerierHelpersTest, RemoveMut) {
    using mytho::utils::type_list;
    using mytho::utils::internal::rm_template_t;
    using mytho::utils::rm_mut_t;
    using mytho::utils::internal::data_wrapper;

    // rm_mut on plain T yields const data_wrapper<T>
    static_assert(std::is_same_v<rm_mut_t<Position>, type_list<const data_wrapper<Position>>>);

    // rm_mut on entity specialization yields const data_wrapper<entity>
    static_assert(std::is_same_v<rm_mut_t<entity>, type_list<const data_wrapper<entity>>>);

    // rm_mut on mut<Ts...> yields data_wrapper<Ts>...
    static_assert(std::is_same_v<rm_mut_t<mut<Position, Velocity>>, type_list<data_wrapper<Position>, data_wrapper<Velocity>>>);
}

TEST(QuerierHelpersTest, RemoveTemplate) {
    using mytho::utils::type_list;
    using mytho::utils::rm_with_t;
    using mytho::utils::rm_without_t;
    using mytho::utils::rm_changed_t;

    static_assert(std::is_same_v<rm_with_t<with<Position, Velocity>>, type_list<Position, Velocity>>);
    static_assert(std::is_same_v<rm_without_t<without<Position, Velocity>>, type_list<Position, Velocity>>);
    static_assert(std::is_same_v<rm_changed_t<changed<Position, Velocity>>, type_list<Position, Velocity>>);
}

TEST(QuerierHelpersTest, TypeListConvert) {
    using mytho::utils::type_list;
    using mytho::utils::datatype_list_convert_t;
    using mytho::utils::prototype_list_convert_t;
    using mytho::utils::internal::data_wrapper;

    // datatype_list_convert: entity => const wrapper; mut<T> => non-const wrapper
    using L = type_list<entity, Position, mut<Velocity>>;
    using DL = datatype_list_convert_t<L>;
    using ExpectedDL = type_list<const data_wrapper<entity>, const data_wrapper<Position>, data_wrapper<Velocity>>;
    static_assert(std::is_same_v<DL, ExpectedDL>);

    // prototype_list_convert w.r.t a specific wrapper template
    using L2 = type_list<with<entity, Position>, changed<Velocity>, without<Health>>;
    using PWith = prototype_list_convert_t<L2, with>;
    using PChanged = prototype_list_convert_t<L2, changed>;
    static_assert(std::is_same_v<PWith, type_list<entity, Position, changed<Velocity>, without<Health>>>);
    static_assert(std::is_same_v<PChanged, type_list<with<entity, Position>, Velocity, without<Health>>>);
}

TEST(QuerierHelpersTest, QueryTypes) {
    using mytho::utils::type_list;
    using QT = mytho::ecs::internal::query_types<
        entity,
        Position,
        mut<Velocity>,
        with<Health>,
        without<Position>,
        added<Velocity>,
        changed<Health>
    >;

    using ReqList = typename QT::require_list;
    using ReqProto = typename QT::require_prototype_list;
    using ReqData = typename QT::require_datatype_list;
    using WithProto = typename QT::with_prototype_list;
    using WithoutProto = typename QT::without_prototype_list;
    using AddedProto = typename QT::added_prototype_list;
    using ChangedProto = typename QT::changed_prototype_list;

    using mytho::utils::internal::data_wrapper;

    static_assert(std::is_same_v<ReqList, type_list<entity, Position, mut<Velocity>>>);
    static_assert(std::is_same_v<ReqProto, type_list<entity, Position, Velocity>>);
    static_assert(std::is_same_v<WithProto, type_list<Health>>);
    static_assert(std::is_same_v<WithoutProto, type_list<Position>>);
    static_assert(std::is_same_v<AddedProto, type_list<Velocity>>);
    static_assert(std::is_same_v<ChangedProto, type_list<Health>>);

    using ExpectedReqData = type_list<
        const data_wrapper<entity>,
        const data_wrapper<Position>,
        data_wrapper<Velocity>
    >;
    static_assert(std::is_same_v<ReqData, ExpectedReqData>);
}

/*
 * ======================================== Querier Test Cases =====================================
 */

TEST(QuerierTest, ConstructionAndIteration) {
    using Q = basic_querier<registry, entity, Position, mut<Velocity>, Health>;
    using Bundle = typename Q::component_bundle_type;
    using Container = typename Q::component_bundle_container_type;

    entity e1{1u, 0u};
    entity e2{2u, 0u};

    Position p1{1, 2};
    Position p2{3, 4};
    Velocity v1{5, 6};
    Velocity v2{7, 8};
    Health h1{9, 10};
    Health h2{11, 12};

    uint64_t ct = 100;
    uint64_t px1 = 0, px2 = 0, vx1 = 0, vx2 = 0, hx1 = 0, hx2 = 0;

    data_wrapper<entity> we1(e1), we2(e2);
    data_wrapper<Position> wp1(&p1, px1, ct), wp2(&p2, px2, ct);
    data_wrapper<Velocity> wv1(&v1, vx1, ct), wv2(&v2, vx2, ct);
    data_wrapper<Health> wh1(&h1, hx1, ct), wh2(&h2, hx2, ct);

    Container bundles;
    bundles.emplace_back(Bundle{we1, wp1, wv1, wh1});
    bundles.emplace_back(Bundle{we2, wp2, wv2, wh2});

    Q qu(bundles);
    ASSERT_EQ(qu.size(), 2);

    int i = 0;
    for (auto [ent, pos, vel, hp] : qu) {
        if (i == 0) {
            EXPECT_EQ(ent->id(), 1u);
            EXPECT_EQ(pos->x, 1); EXPECT_EQ(pos->y, 2);
            EXPECT_EQ(vel->vx, 5); EXPECT_EQ(vel->vy, 6);
            EXPECT_EQ(hp->current, 9); EXPECT_EQ(hp->max, 10);

            vel->vx += 10; vel->vy += 10; // mut propagation
        } else {
            EXPECT_EQ(ent->id(), 2u);
            EXPECT_EQ(pos->x, 3); EXPECT_EQ(pos->y, 4);
            EXPECT_EQ(vel->vx, 7); EXPECT_EQ(vel->vy, 8);
            EXPECT_EQ(hp->current, 11); EXPECT_EQ(hp->max, 12);

            vel->vx += 20; vel->vy += 20; // mut propagation
        }
        i++;
    }

    EXPECT_EQ(v1.vx, 15); EXPECT_EQ(v1.vy, 16);
    EXPECT_EQ(v2.vx, 27); EXPECT_EQ(v2.vy, 28);
}

TEST(QuerierTest, QuerierDataTypeMapping) {
    using Q1 = basic_querier<registry, entity, Position, mut<Velocity>, Health>;
    using Bundle1 = typename Q1::component_bundle_type;

    // element 0: const data_wrapper<entity>
    static_assert(std::is_same_v<std::tuple_element_t<0, Bundle1>, const data_wrapper<entity>>);
    // element 1: const data_wrapper<Position>
    static_assert(std::is_same_v<std::tuple_element_t<1, Bundle1>, const data_wrapper<Position>>);
    // element 2: data_wrapper<Velocity> (mut)
    static_assert(std::is_same_v<std::tuple_element_t<2, Bundle1>, data_wrapper<Velocity>>);
    // element 3: const data_wrapper<Health>
    static_assert(std::is_same_v<std::tuple_element_t<3, Bundle1>, const data_wrapper<Health>>);

    using Q2 = basic_querier<registry, mut<Position, Velocity>>;
    using Bundle2 = typename Q2::component_bundle_type;

    static_assert(std::is_same_v<std::tuple_element_t<0, Bundle2>, data_wrapper<Position>>);
    static_assert(std::is_same_v<std::tuple_element_t<1, Bundle2>, data_wrapper<Velocity>>);
}

TEST(QuerierTest, EmptyContainer) {
    using Q = basic_querier<registry, Position>;
    using Container = typename Q::component_bundle_container_type;

    Container bundles; // empty
    Q qu(bundles);

    EXPECT_TRUE(qu.empty());
    EXPECT_EQ(qu.size(), 0);
}

/*
 * ======================================== Querier Trait Test Cases ====================================
 */

TEST(QuerierTraitTest, IsQuerierTrait) {
    static_assert(mytho::utils::is_querier_v<basic_querier<registry, Position>>);
    static_assert(mytho::utils::is_querier_v<basic_querier<registry, mut<Position>>>);
    static_assert(mytho::utils::is_querier_v<basic_querier<registry, with<Position>>>);
    static_assert(mytho::utils::is_querier_v<basic_querier<registry, without<Position>>>);
    static_assert(mytho::utils::is_querier_v<basic_querier<registry, added<Position>>>);
    static_assert(mytho::utils::is_querier_v<basic_querier<registry, changed<Position>>>);
}

/*
 * ======================================== Removed Entities Test Cases ====================================
 */

TEST(RemovedEntitiesTest, BasicOperations) {
    using RemovedEntities = basic_removed_entities<registry, Position>;
    using EntitiesType = typename RemovedEntities::entities_type;

    entity e1(1, 10);
    entity e2(2, 20);
    entity e3(3, 30);

    EntitiesType entities = {e1, e2, e3};

    RemovedEntities removed_entities(entities);

    EXPECT_EQ(removed_entities.size(), 3);
    EXPECT_FALSE(removed_entities.empty());

    auto it = removed_entities.begin();
    EXPECT_EQ(*it, e1);
    ++it;
    EXPECT_EQ(*it, e2);
    ++it;
    EXPECT_EQ(*it, e3);
    ++it;
    EXPECT_EQ(it, removed_entities.end());

    int count = 0;
    for (const auto& entity : removed_entities) {
        if (count == 0) {
            EXPECT_EQ(entity, e1);
        } else if (count == 1) {
            EXPECT_EQ(entity, e2);
        } else if (count == 2) {
            EXPECT_EQ(entity, e3);
        }
        count++;
    }
    EXPECT_EQ(count, 3);
}

TEST(RemovedEntitiesTest, EmptyContainer) {
    using RemovedEntities = basic_removed_entities<registry, Position>;
    using EntitiesType = typename RemovedEntities::entities_type;

    EntitiesType entities;

    RemovedEntities removed_entities(entities);

    EXPECT_EQ(removed_entities.size(), 0);
    EXPECT_TRUE(removed_entities.empty());

    EXPECT_EQ(removed_entities.begin(), removed_entities.end());

    int count = 0;
    for (const auto& entity : removed_entities) {
        count++;
    }
    EXPECT_EQ(count, 0);
}

TEST(RemovedEntitiesTest, ConstIteration) {
    using RemovedEntities = basic_removed_entities<registry, Position>;
    using EntitiesType = typename RemovedEntities::entities_type;

    entity e1(1, 10);
    entity e2(2, 20);

    EntitiesType entities = {e1, e2};

    const RemovedEntities removed_entities(entities);

    auto it = removed_entities.begin();
    EXPECT_EQ(*it, e1);
    ++it;
    EXPECT_EQ(*it, e2);
    ++it;
    EXPECT_EQ(it, removed_entities.end());

    int count = 0;
    for (const auto& entity : removed_entities) {
        if (count == 0) {
            EXPECT_EQ(entity, e1);
        } else if (count == 1) {
            EXPECT_EQ(entity, e2);
        }
        count++;
    }
    EXPECT_EQ(count, 2);
}

TEST(RemovedEntitiesTest, ModificationThroughReference) {
    using RemovedEntities = basic_removed_entities<registry, Position>;
    using EntitiesType = typename RemovedEntities::entities_type;

    entity e1(1, 10);
    entity e2(2, 20);

    EntitiesType entities = {e1, e2};
    RemovedEntities removed_entities(entities);

    EXPECT_EQ(removed_entities.size(), 2);

    entities.push_back(entity(3, 30));

    EXPECT_EQ(removed_entities.size(), 3);

    int count = 0;
    for (const auto& entity : removed_entities) {
        count++;
    }
    EXPECT_EQ(count, 3);
}

/*
 * ======================================== Removed Entities Trait Test Cases ===============================
 */

TEST(RemovedEntitiesTraitTest, IsRemovedEntitiesTrait) {
    using RemovedEntities = basic_removed_entities<registry, Position>;

    static_assert(mytho::utils::is_removed_entities_v<RemovedEntities>);
    static_assert(!mytho::utils::is_removed_entities_v<basic_querier<registry, Position>>);
    static_assert(!mytho::utils::is_removed_entities_v<entity>);
    static_assert(!mytho::utils::is_removed_entities_v<int>);
}