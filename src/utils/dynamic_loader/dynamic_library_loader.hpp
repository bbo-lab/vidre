#pragma once

#include <string>



namespace vidre {
    inline std::string APTINA_LIBRARY_PATH = "";
    inline std::string PYLON_LIBRARY_PATH = "";

    namespace util {

    namespace impl {
        void* get_function(const std::string& library_name, const std::string& function);
    } // namespace impl


    template<typename T>
    T get_function(const std::string& library_name, const std::string& function) {
        return reinterpret_cast<T>(impl::get_function(library_name, function));
    }

    } // namespace util
} // namespace vidre