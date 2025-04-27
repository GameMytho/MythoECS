#include <ecs/ecs.hpp>
#include <cstdint>
#include <iostream>

int main() {
    mytho::container::basic_sparse_set<uint32_t, 1024> s;

    s.add(100000);

    ASSURE(s.contain(100000), "There should be 10 in sparse set");

    s.add(923424);

    ASSURE(s.contain(923424), "There should be 10 in sparse set");

    s.add(8456);

    ASSURE(s.contain(8456), "There should be 10 in sparse set");

    s.add(73);

    ASSURE(s.contain(73), "There should be 10 in sparse set");

    s.remove(100000);

    ASSURE(!s.contain(100000), "There should not be 10 in sparse set");

    s.remove(923424);

    ASSURE(!s.contain(923424), "There should not be 10 in sparse set");

    s.remove(73);

    ASSURE(!s.contain(73), "There should not be 10 in sparse set");

    s.remove(8456);

    ASSURE(!s.contain(8456), "There should not be 10 in sparse set");

    return 0;
}