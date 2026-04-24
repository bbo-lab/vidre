#include <vidre/util/uuid.hh>
#include <iostream>

using namespace vidre;

int main(int argc, const char **argv) {
    std::cout << uuid::makeRandom().tostring() << std::endl;
    return 0;
}
/**@example examples/core/uuid.cc
 * @details 
 * This file demonstrates UUIDs.
 */
