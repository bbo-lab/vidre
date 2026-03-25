# Qt

find_package(Qt6 6.10 REQUIRED COMPONENTS Core Gui Network Widgets)

# FIXME
# if (WIN32 AND NOT UNIX)
#     FetchContent_Declare(Qt6
#         URL      https://download.qt.io/official_releases/qt/6.11/6.11.0/single/qt-everywhere-src-6.11.0.zip
#         URL_HASH MD5=a0c1765cfd135eed64a3c0535cf27318
#         FIND_PACKAGE_ARGS
#             6.10 REQUIRED
#             COMPONENTS
#                 Core Gui Network Widgets
#     )
# else()
#     FetchContent_Declare(Qt6
#         URL      https://download.qt.io/official_releases/qt/6.11/6.11.0/single/qt-everywhere-src-6.11.0.tar.xz
#         URL_HASH MD5=a93e9f424a9d11ee8d67bf8fb1af4772
#         FIND_PACKAGE_ARGS
#             6.10 REQUIRED
#             COMPONENTS
#                 Core Gui Network Widgets
#     )
# endif()
# FetchContent_MakeAvailable(Qt6)

set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)


