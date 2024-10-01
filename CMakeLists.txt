cmake_minimum_required(VERSION 3.16)
project(zgraph C)

# 查找 ZCollection 库
find_package(ZCollection REQUIRED)

set(CMAKE_C_STANDARD 11)

# 包含头文件目录
include_directories("include")

# 添加静态库 ZJSON
add_library(ZGRAPH STATIC src/zgraph.c)
target_link_libraries(ZGRAPH PRIVATE ZCollection::ZCollection)

# 设置头文件的安装路径
target_include_directories(ZGRAPH PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# 安装库和头文件
install(TARGETS ZGRAPH
        EXPORT ZGRAPHTargets                   # 导出 ZGRAPH 库目标
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include
)

# 批量安装头文件目录
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include)

# 生成并安装 ZJSONConfig.cmake 文件
include(CMakePackageConfigHelpers)

# 配置 ZJSONConfig.cmake 文件
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ZGRAPHConfig.cmake.in"     # 输入模板
        "${CMAKE_CURRENT_BINARY_DIR}/ZGRAPHConfig.cmake"              # 输出配置文件
        INSTALL_DESTINATION lib/cmake/ZJSON                          # 安装路径
)

# 安装 ZJSONConfig.cmake
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ZGRAPHConfig.cmake"
        DESTINATION lib/cmake/ZJSON
)

# 生成 ZJSONTargets.cmake 文件以导出库信息
install(EXPORT ZGRAPHTargets
        FILE ZGRAPHTargets.cmake
        NAMESPACE ZGRAPH::
        DESTINATION lib/cmake/ZGRAPH
)

# 导出 ZJSONTargets，使其可在构建过程中使用
export(EXPORT ZGRAPHTargets
        FILE "${CMAKE_CURRENT_BINARY_DIR}/ZGRAPHTargets.cmake"
        NAMESPACE ZGRAPH::
)

# 获取 test 目录下的所有 .c 文件
file(GLOB test_files "${CMAKE_CURRENT_SOURCE_DIR}/test/*.c")

# 获取 help_test 目录下的所有 .c 文件
file(GLOB help_test_files "${CMAKE_CURRENT_SOURCE_DIR}/test/help_test/*.c")

# 遍历 test 目录下的所有 .c 文件
foreach (test_file ${test_files})
    # 获取文件名，不包含路径和扩展名
    get_filename_component(name ${test_file} NAME_WE)

    # 为每个可执行文件添加 test 文件和 help_test 目录下的文件
    add_executable(${name} ${test_file} ${help_test_files})

    target_link_libraries(${name}
            ZGRAPH
    )
endforeach ()
