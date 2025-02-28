cmake_minimum_required(VERSION 3.10)
project(doowm VERSION 0.1.0 LANGUAGES CXX)

# ビルドディレクトリを設定
set(BUILD_DIR "${CMAKE_SOURCE_DIR}/build")
file(MAKE_DIRECTORY ${BUILD_DIR})

# C++17を使用
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# コンパイラ警告を有効化
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 出力ディレクトリの設定
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})

# 必要なパッケージの検索
find_package(PkgConfig REQUIRED)
pkg_check_modules(XCB REQUIRED xcb xcb-util xcb-icccm xcb-keysyms)
pkg_check_modules(X11 REQUIRED x11)

# ソースファイルの設定
set(SOURCES
    src/main.cpp
    src/log/logger.cpp
    src/x/connection/connection.cpp
    src/x/x.cpp
    src/x/window/window.cpp
    src/x/event/event_handler.cpp
    src/x/keyboard/keyboard.cpp
    src/x/launcher/launcher.cpp
)

# 実行ファイルの作成
add_executable(${PROJECT_NAME} ${SOURCES})

# インクルードディレクトリの設定
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${XCB_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
)

# ライブラリのリンク
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${XCB_LIBRARIES}
    ${X11_LIBRARIES}
)

# filesystem ライブラリをリンク (GCC 8 以前では必要)
if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()

# インストールターゲット
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES scripts/build_and_run.sh scripts/xinitrc 
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                   GROUP_EXECUTE GROUP_READ
                   WORLD_EXECUTE WORLD_READ
        DESTINATION bin)

# make cleanで削除する追加ファイルを設定
set_property(DIRECTORY APPEND PROPERTY 
    ADDITIONAL_CLEAN_FILES 
    "${CMAKE_BINARY_DIR}/CMakeCache.txt"
    "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    "${CMAKE_BINARY_DIR}/CMakeFiles"
    "${CMAKE_BINARY_DIR}/Makefile"
    "${BUILD_DIR}"
)

# ビルドディレクトリをクリーンするターゲット（オプション）
add_custom_target(purge
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${BUILD_DIR}
    COMMENT "Removing build directories"
)

# Add custom target for running with Xephyr for testing
add_custom_target(run_xephyr
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/script.sh
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running window manager in Xephyr"
)

# デバッグビルド設定
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-g)
