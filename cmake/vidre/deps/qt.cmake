# Qt

find_package(Qt6 6.0 REQUIRED COMPONENTS Core Gui Network Widgets)

# FetchContent_Declare(Qt6
#     URL      https://download.qt.io/official_releases/qt/6.11/6.11.0/single/qt-everywhere-src-6.11.0.zip
#     URL_HASH MD5=a0c1765cfd135eed64a3c0535cf27318
#     FIND_PACKAGE_ARGS
#         6.0 REQUIRED
#         COMPONENTS
#             Core Gui Network Widgets
# )

# FetchContent_MakeAvailable(Qt6)

set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)
