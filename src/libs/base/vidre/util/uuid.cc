#include "uuid.hh"

#include <stdexcept>

using namespace vidre;

void vidre::UUID::parse(const std::string &s) {
    if (uuid_parse(s.c_str(), uuid_) != 0)
        throw std::runtime_error("not a UUID");
}

std::string vidre::UUID::tostring(bool lowercase) const noexcept {
    std::string result;
    result.resize(UUID_STR_LEN-1);

    if (lowercase)
        uuid_unparse_lower(uuid_, result.data());
    else
        uuid_unparse_upper(uuid_,  result.data());

    return result;
}
