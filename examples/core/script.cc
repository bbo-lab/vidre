#include <vidre/script.hh>
#include <vidre/log.hh>
#include <vidre/exception.hh>

using namespace vidre;

void handle_lua_error(lua_State *L, int status) {
    if (status != LUA_OK) {
        /* Get the error message from the stack */
        const char *err_msg = lua_tostring(L, -1);
        
        /* Try to get debug information */
        lua_Debug ar;
        int line_number = -1;
        const char *source = "unknown";
        
        /* Look for debug info about the error */
        if (lua_getstack(L, 0, &ar)) {
            if (lua_getinfo(L, "Sl", &ar)) {
                if (ar.source) {
                    source = ar.source;
                    /* Remove '@' prefix if present (file source) */
                    if (source[0] == '@') {
                        source++;
                    }
                }
                line_number = ar.currentline;
            }
        }
        
        /* Print detailed error information */
        if (line_number > 0) {
            fprintf(stderr, "Lua error in %s at line %d: %s\n", 
                    source, line_number, err_msg);
        } else {
            fprintf(stderr, "Lua error: %s\n", err_msg);
        }
        
        /* Remove error message from stack */
        lua_pop(L, 1);
    }
}

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

        if (script::g.getglobal("package") != lua::type::table)  // +1
            throw exception();

        script::g.getfield(-1, "path");  // +2
        log::info() << "package.path: " << script::g.tostring(-1) << log::endl;

        script::g.getfield(-2, "cpath");  // +3
        log::info() << "package.cpath: " << script::g.tostring(-1) << log::endl;
    }
    //! [stack-top]

    //! [script-error]
    handle_lua_error(script::g, (int) script::g.dofile(PROJECT_SOURCE_DIR "/examples/core/lua/script_syntax.lua"));
    handle_lua_error(script::g, (int) script::g.dofile(PROJECT_SOURCE_DIR "/examples/core/lua/script_error.lua"));
    //! [script-error]

    return 0;
}

/**@example examples/core/script.cc
 * @brief Script-API usage.
 *
 * */
