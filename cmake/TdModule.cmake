function(td_target_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /utf-8)
        if(TD_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
        target_compile_definitions(${target} PRIVATE NOMINMAX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(TD_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

# td_add_module(<name>
#     SOURCES <files...>     # relative to the module directory
#     DEPENDS <targets...>   # other td:: modules this module may use
#     TESTS   <files...>)    # unit tests, optional
#
# Convention: core/<name>/include/td/<name>/*.hpp, src/*.cpp, tests/*.cpp.
# Creates the static library td_<name> (alias td::<name>) and, when tests are
# enabled, the executable td_<name>_tests registered as one ctest entry.
function(td_add_module name)
    cmake_parse_arguments(M "" "" "SOURCES;DEPENDS;TESTS" ${ARGN})
    set(target td_${name})

    add_library(${target} STATIC ${M_SOURCES})
    add_library(td::${name} ALIAS ${target})
    target_include_directories(${target} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
    if(M_DEPENDS)
        target_link_libraries(${target} PUBLIC ${M_DEPENDS})
    endif()
    set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    td_target_warnings(${target})

    if(TD_BUILD_TESTS AND M_TESTS)
        add_executable(${target}_tests ${M_TESTS})
        target_link_libraries(${target}_tests PRIVATE ${target} Catch2::Catch2WithMain)
        td_target_warnings(${target}_tests)
        add_test(NAME ${target} COMMAND ${target}_tests)
    endif()
endfunction()
