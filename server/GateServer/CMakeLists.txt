cmake_minimum_required(VERSION 3.20)
project(GateServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置根目录
set(GRPC_INSTALL_DIR E:/cppsoft/grpc_v1.50/installed)
set(BOOST_DIR      E:/cppsoft/boost_1_90_0/installed)
set(JSONCPP_DIR   E:/cppsoft/jsoncpp-1.9.7/installed)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Proto 文件目录
set(PROTO_DIR ${CMAKE_SOURCE_DIR}/proto)
set(PROTO_OUT_DIR ${CMAKE_BINARY_DIR}/proto_generated)

# 查找所有 proto 文件
file(GLOB_RECURSE PROTO_FILES ${PROTO_DIR}/*.proto)

# 创建输出目录
file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

# 为每个 proto 文件生成代码
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    set(PROTO_OUT_CC ${PROTO_OUT_DIR}/${PROTO_NAME}.pb.cc)
    set(PROTO_OUT_H  ${PROTO_OUT_DIR}/${PROTO_NAME}.pb.h)

    add_custom_command(
    OUTPUT ${PROTO_OUT_CC} ${PROTO_OUT_H} ${PROTO_OUT_DIR}/${PROTO_NAME}.grpc.pb.cc ${PROTO_OUT_DIR}/${PROTO_NAME}.grpc.pb.h
    COMMAND E:/cppsoft/grpc_v1.50/installed/bin/protoc.exe
        --proto_path=${PROTO_DIR}
        --cpp_out=${PROTO_OUT_DIR}
        --grpc_out=${PROTO_OUT_DIR}
        --plugin=protoc-gen-grpc=E:/cppsoft/grpc_v1.50/installed/bin/grpc_cpp_plugin.exe
        ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating ${PROTO_NAME}.pb.cc, .pb.h, .grpc.pb.cc and .grpc.pb.h"
)

    list(APPEND PROTO_SOURCES ${PROTO_OUT_CC} ${PROTO_OUT_DIR}/${PROTO_NAME}.grpc.pb.cc)
endforeach()

# 创建生成源文件的虚拟目标
add_custom_target(generate_proto ALL DEPENDS ${PROTO_SOURCES})

# 自动加载所有源文件
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)
# 添加可执行文件
add_executable(GateServer ${SOURCES} ${PROTO_SOURCES})

# 添加 proto 生成目标依赖
add_dependencies(GateServer generate_proto)

# 添加头文件目录
target_include_directories(GateServer PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/proto
    ${PROTO_OUT_DIR}
    ${BOOST_DIR}/include/boost-1_90
    ${JSONCPP_DIR}/include
    ${GRPC_INSTALL_DIR}/include
)


# 添加链接库
target_link_libraries(GateServer PRIVATE
    # json
    E:/cppsoft/jsoncpp-1.9.7/installed/lib/libjsoncpp.a

    # ========== Boost ==========
    E:/cppsoft/boost_1_90_0/installed/lib/libboost_filesystem-mgw13-mt-x64-1_90.a
    E:/cppsoft/boost_1_90_0/installed/lib/libboost_atomic-mgw13-mt-x64-1_90.a

    # ========== gRPC / Protobuf ==========
    # 顺序：高层库在前，被依赖的库在后
    E:/cppsoft/grpc_v1.50/installed/lib/libgrpc++.a
    E:/cppsoft/grpc_v1.50/installed/lib/libgrpc++_reflection.a
    E:/cppsoft/grpc_v1.50/installed/lib/libgrpc.a
    E:/cppsoft/grpc_v1.50/installed/lib/libgpr.a
    E:/cppsoft/grpc_v1.50/installed/lib/libprotobuf.a
    E:/cppsoft/grpc_v1.50/installed/lib/libaddress_sorting.a
    E:/cppsoft/grpc_v1.50/installed/lib/libcares.a
    E:/cppsoft/grpc_v1.50/installed/lib/libupb.a
    E:/cppsoft/grpc_v1.50/installed/lib/libre2.a
    E:/cppsoft/grpc_v1.50/installed/lib/libssl.a
    E:/cppsoft/grpc_v1.50/installed/lib/libcrypto.a
    E:/cppsoft/grpc_v1.50/installed/lib/libzlibstatic.a

    # Windows 套接字 (必须在 gRPC 之后)
    ws2_32
    mswsock

    # ========== Abseil 全量库 ==========
    # 顺序：被依赖的库放在后面
    # Flags (高层)
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_flags.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_flags_parse.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_flags_usage.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_flags_usage_internal.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_flags_internal.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_flags_marshalling.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_flags_config.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_flags_program_name.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_flags_commandlineflag.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_flags_commandlineflag_internal.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_flags_private_handle_accessor.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_flags_reflection.a
    # Status
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_statusor.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_status.a
    # Synchronization
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_synchronization.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_graphcycles_internal.a
    # Time
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_time.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_time_zone.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_civil_time.a
    # Cord (字符串相关，依赖较多)
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_cord.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_cordz_info.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_cordz_handle.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_cordz_functions.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_cordz_sample_token.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_cord_internal.a
    # Hash
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_hash.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_low_level_hash.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_hashtablez_sampler.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_raw_hash_set.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_city.a
    # Strings
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_str_format_internal.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_strings.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_strings_internal.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_strerror.a
    # Debugging
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_symbolize.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_stacktrace.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_debugging_internal.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_demangle_internal.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_examine_stack.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_failure_signal_handler.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_leak_check.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_scoped_set_env.a
    # Random
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_random_distributions.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_random_seed_sequences.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_random_internal_pool_urbg.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_random_internal_randen.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_random_internal_randen_hwaes.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_random_internal_randen_hwaes_impl.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_random_internal_randen_slow.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_random_internal_seed_material.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_random_internal_platform.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_random_internal_distribution_test_util.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_random_seed_gen_exception.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_periodic_sampler.a
    # 基础库（放在最后）
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_base.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_spinlock_wait.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_malloc_internal.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_raw_logging_internal.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_log_severity.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_throw_delegate.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_int128.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_exponential_biased.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_bad_any_cast_impl.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_bad_optional_access.a
    E:/cppsoft/grpc_v1.50/installed/lib/libabsl_bad_variant_access.a

    # Windows 系统库 (必须在所有库之后)
    DbgHelp
    bcrypt
)