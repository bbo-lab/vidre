#include <vidre/script.hh>
#include <vidre/log.hh>
#include <vidre/exception.hh>

using namespace vidre;

int main(int, const char **) {
//! [hello-world]
    // Execute the standard "Hello World!".
    script::g.exec("print('Hello world!')");
//! [hello-world]

//! [stack-top]
    // The Lua stack needs to be cleaned up after operations. This can be done
    // automatically using a stack scope. Here, we log the package.path
    // and package.cpath, noting the number of elements on the stack.
    {
        script::engine::stack scope(script::g);

        if (script::g.getglobal("package") != lua::type::table) // +1
            throw exception();

        script::g.getfield(-1,"path"); // +2
        log::info() << "package.path: " << script::g.tostring(-1) << log::endl;

        script::g.getfield(-2,"cpath"); // +3
        log::info() << "package.cpath: " << script::g.tostring(-1) << log::endl;
    }
//! [stack-top]

    return 0;
}

/**@example examples/core/script.cc
 * @brief Script-API usage.
 *
 * */
