#include "exception.hh"

using namespace vidre;

const char *system_error::what() const noexcept {
    static char msgbuf[VIDRE_EXCEPTION_BUFSIZE];
    auto emsg = ec_.message();
    auto len = std::min(std::string::size_type(VIDRE_EXCEPTION_BUFSIZE - 1), emsg.size());
    std::copy_n(emsg.data(), len, msgbuf);
    msgbuf[len] = 0;
    return msgbuf;
}
