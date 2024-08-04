rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=PPROF
cmake --build build

CPUPROFILE=facility_game.pprof build/facility_game
pprof --web build/facility_game facility_game.pprof

