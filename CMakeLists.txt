cmake_minimum_required(VERSION 2.8.7)
project(netpop CXX)
include_directories(include)
ADD_DEFINITIONS(
    -std=c++11 
)
#file(GLOB SOURCES "src/*.c++")
set(SOURCES_BIKES src/Utils.c++ src/StationData.c++ src/RideData.c++ src/mainBikes.c++)
set(SOURCES_TRAINS src/Utils.c++ src/StationData.c++ src/TrainData.c++ src/mainTrains.c++)
set(SOURCES_DISTS src/Utils.c++ src/mainStnDists.c++)

add_executable(bikes ${SOURCES_BIKES})
add_executable(trains ${SOURCES_TRAINS})
add_executable(dists ${SOURCES_DISTS})
target_link_libraries(bikes boost_program_options zip)
target_link_libraries(trains boost_program_options zip)
target_link_libraries(dists boost_program_options)
