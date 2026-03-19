#include "exception.hh"

using namespace vidre;

const char *system_error::what() const noexcept {
    static thread_local static_error_message msg {};

    msg = ec_.message();

    return msg;
}
