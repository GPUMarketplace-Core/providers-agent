find_package(Qt6 REQUIRED COMPONENTS Core Network)

set(CMAKE_AUTOMOC ON)

add_library(job_runner
    src/job_runner.cpp
    src/job_manager.cpp
    include/job_runner/job_runner.h
    include/job_runner/job_manager.h
)

target_include_directories(job_runner PUBLIC
    include
)

target_link_libraries(job_runner PRIVATE Qt6::Core Qt6::Network)
target_link_libraries(job_runner PUBLIC common)
