#pragma once

#include <string>



namespace vidre {
    inline std::string APTINA_LIBRARY_PATH = "";
    inline std::string PYLON_LIBRARY_PATH = "";

    namespace util {
        bool IsLibraryAvailable(const std::string& library_name);


        namespace impl {
            void* GetFunction(const std::string& library_name, const std::string& function);
        } // namespace impl

        template<typename T>
        T GetFunction(const std::string& library_name, const std::string& function) {
            return reinterpret_cast<T>(impl::GetFunction(library_name, function));
        }

    } // namespace util
} // namespace vidre