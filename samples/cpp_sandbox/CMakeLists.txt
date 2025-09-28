add_executable(cpp_sandbox
    test_ranges.cpp
    main.cpp)

set_target_properties(cpp_sandbox PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

