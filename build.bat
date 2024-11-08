cmake -S .\ -B .\build_out
cmake --build .\build_out --target ALL_BUILD --config Debug
cmake --build .\build_out --target ALL_BUILD --config Release
