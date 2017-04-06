add_executable(ds-example ds-example.cpp)
target_link_libraries(ds-example PUBLIC libdeepstream_poco libdeepstream_core)

add_executable(ds-client ds-client.cpp)
target_link_libraries(ds-client PUBLIC libdeepstream_poco)

add_executable(fuzz-me fuzz-me.cpp)
target_link_libraries(fuzz-me PUBLIC libdeepstream_poco libdeepstream_core)

add_executable(make-fuzzer-input make-fuzzer-input.cpp)
target_link_libraries(make-fuzzer-input PUBLIC libdeepstream_poco libdeepstream_core)

add_executable(poco-ws-echo poco-ws-echo.cpp)
target_link_libraries(poco-ws-echo PUBLIC libdeepstream_poco)
