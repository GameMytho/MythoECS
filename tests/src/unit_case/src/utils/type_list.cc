/*
 * Unit Tests for type_list.hpp - Type List Testing
 * 
 * This test suite validates the type list functionality in type_list.hpp:
 * - type_list: Basic type list container
 * - type_list_cat_t: Concatenate multiple type lists
 * - type_list_filter_t: Filter out specific types from type list
 * - type_list_filter_template_t: Filter out template types from type list
 * - type_list_extract_template_t: Extract specific template types from type list
 * - list_to_tuple_t: Convert type list to tuple
 * - rm_template_t: Remove template wrapper and return type list
 * - is_template_v: Check if type is a template instantiation
 * 
 * Test Cases:
 * 1. BasicTypeList - Tests basic type list container and type detection
 * 2. TypeListCat - Tests type list concatenation with multiple lists
 * 3. TypeListFiltering - Tests filtering operations (specific types and template types)
 * 4. TypeListExtract - Tests extraction operations (template types)
 * 5. ListToTuple - Tests type list to tuple conversion
 * 6. RmTemplate - Tests template wrapper removal functionality
 * 7. IsTemplate - Tests template instantiation detection
 * 8. ComplexOperations - Tests complex operations with mixed type lists
 */

#include <gtest/gtest.h>
#include <type_traits>

#include "utils/type_list.hpp"

using namespace mytho::utils;

/*
 * =============================== Helper Structures/Functions ===============================
 */

template<typename... Ts>
struct TestTemplate {};

template<typename... Ts>
struct Component {};

template<typename... Ts>
struct Resource {};

/*
 * ======================================== Test Cases ========================================
 */

// Test basic type list container and type detection
TEST(TypeListTest, BasicTypeList) {
    using List1 = type_list<int, float, double>;
    using List2 = type_list<>;

    EXPECT_TRUE(is_type_list_v<List1>);
    EXPECT_TRUE(is_type_list_v<List2>);
    EXPECT_FALSE(is_type_list_v<int>);
    EXPECT_FALSE((is_type_list_v<std::tuple<int, float>>));
}

// Test type list concatenation with multiple lists
TEST(TypeListTest, TypeListCat) {
    using List1 = type_list<int, float>;
    using List2 = type_list<double, char>;
    using List3 = type_list<bool>;
    using EmptyList = type_list<>;

    using CatResult1 = type_list_cat_t<List1, List2>;
    EXPECT_TRUE((std::is_same_v<CatResult1, type_list<int, float, double, char>>));

    using CatResult2 = type_list_cat_t<List1, EmptyList>;
    EXPECT_TRUE((std::is_same_v<CatResult2, List1>));

    using CatResult3 = type_list_cat_t<EmptyList, List1>;
    EXPECT_TRUE((std::is_same_v<CatResult3, List1>));

    using CatResult4 = type_list_cat_t<List1, List2, List3>;
    EXPECT_TRUE((std::is_same_v<CatResult4, type_list<int, float, double, char, bool>>));

    using CatResult5 = type_list_cat_t<List1>;
    EXPECT_TRUE((std::is_same_v<CatResult5, List1>));

    using CatResult6 = type_list_cat_t<>;
    EXPECT_TRUE((std::is_same_v<CatResult6, type_list<>>));
}

// Test filtering operations (specific types and template types)
TEST(TypeListTest, TypeListFiltering) {
    // Test empty list filtering
    using EmptyList = type_list<>;

    using EL_FilteredResult = type_list_filter_t<EmptyList, int>;
    EXPECT_TRUE((std::is_same_v<EL_FilteredResult, type_list<>>));

    // Test simple type filtering
    using SimpleList = type_list<int, float, double, char, bool>;

    using SL_FilteredResult1 = type_list_filter_t<SimpleList, int, double>;
    EXPECT_TRUE((std::is_same_v<SL_FilteredResult1, type_list<float, char, bool>>));

    using SL_FilteredResult2 = type_list_filter_t<SimpleList, std::string>;
    EXPECT_TRUE((std::is_same_v<SL_FilteredResult2, SimpleList>));

    using SL_FilteredResult3 = type_list_filter_t<SimpleList, int, float, double, char, bool>;
    EXPECT_TRUE((std::is_same_v<SL_FilteredResult3, type_list<>>));

    // Test template type filtering
    using MixedList = type_list<int, TestTemplate<float>, double, TestTemplate<char>, bool>;

    using ML_FilteredResult1 = type_list_filter_template_t<MixedList, TestTemplate>;
    EXPECT_TRUE((std::is_same_v<ML_FilteredResult1, type_list<int, double, bool>>));

    using ML_FilteredResult2 = type_list_filter_template_t<MixedList, std::vector>;
    EXPECT_TRUE((std::is_same_v<ML_FilteredResult2, MixedList>));
}

// Test extraction operations (specific types and template types)
TEST(TypeListTest, TypeListExtract) {
    // Test empty list extraction
    using EmptyList = type_list<>;

    using EL_ExtractedResult = type_list_extract_template_t<EmptyList, TestTemplate>;
    EXPECT_TRUE((std::is_same_v<EL_ExtractedResult, type_list<>>));

    // Test template type extraction
    using MixedList = type_list<int, TestTemplate<float>, double, TestTemplate<char>, bool>;

    using ML_ExtractedResult1 = type_list_extract_template_t<MixedList, TestTemplate>;
    EXPECT_TRUE((std::is_same_v<ML_ExtractedResult1, type_list<TestTemplate<float>, TestTemplate<char>>>));

    using ML_ExtractedResult2 = type_list_extract_template_t<MixedList, std::vector>;
    EXPECT_TRUE((std::is_same_v<ML_ExtractedResult2, type_list<>>));
}

// Test type list to tuple conversion
TEST(TypeListTest, ListToTuple) {
    using List = type_list<int, float, double>;
    using Tuple = list_to_tuple_t<List>;
    
    EXPECT_TRUE((std::is_same_v<Tuple, std::tuple<int, float, double>>));
    
    using EmptyList = type_list<>;
    using EmptyTuple = list_to_tuple_t<EmptyList>;
    EXPECT_TRUE((std::is_same_v<EmptyTuple, std::tuple<>>));
}

// Test template wrapper removal functionality
TEST(TypeListTest, RmTemplate) {    
    using TemplateType = TestTemplate<int, float, double>;
    using RemovedResult = internal::rm_template_t<TemplateType, TestTemplate>;
    EXPECT_TRUE((std::is_same_v<RemovedResult, type_list<int, float, double>>));
    
    using NonTemplateType = int;
    using NonTemplateResult = internal::rm_template_t<NonTemplateType, TestTemplate>;
    EXPECT_TRUE((std::is_same_v<NonTemplateResult, type_list<int>>));
}

// Test template instantiation detection
TEST(TypeListTest, IsTemplate) {
    EXPECT_TRUE((internal::is_template_v<TestTemplate<int, float>, TestTemplate>));
    EXPECT_FALSE((internal::is_template_v<int, TestTemplate>));
    EXPECT_FALSE((internal::is_template_v<std::vector<int>, TestTemplate>));
    EXPECT_TRUE((internal::is_template_v<std::vector<int>, std::vector>));
}

// Test complex operations with mixed type lists
TEST(TypeListTest, ComplexOperations) {    
    using MixedList = type_list<int, Component<float>, double, Resource<char>, bool, Component<int>>;
    
    using Components = type_list_extract_template_t<MixedList, Component>;
    EXPECT_TRUE((std::is_same_v<Components, type_list<Component<float>, Component<int>>>));
    
    using WithoutComponents = type_list_filter_template_t<MixedList, Component>;
    EXPECT_TRUE((std::is_same_v<WithoutComponents, type_list<int, double, Resource<char>, bool>>));
    
    using Resources = type_list_extract_template_t<MixedList, Resource>;
    EXPECT_TRUE((std::is_same_v<Resources, type_list<Resource<char>>>));
    
    using WithoutResources = type_list_filter_template_t<MixedList, Resource>;
    EXPECT_TRUE((std::is_same_v<WithoutResources, type_list<int, Component<float>, double, bool, Component<int>>>));
}
