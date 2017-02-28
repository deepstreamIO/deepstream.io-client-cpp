add_executable(ds-example ds-example.cpp)
target_link_libraries(ds-example PUBLIC libdeepstream)

add_executable(ds-client ds-client.cpp)
target_link_libraries(ds-client PUBLIC libdeepstream)

add_executable(fuzz-me fuzz-me.cpp)
target_link_libraries(fuzz-me PUBLIC libdeepstream)

add_executable(make-fuzzer-input make-fuzzer-input.cpp)
target_link_libraries(make-fuzzer-input PUBLIC libdeepstream)
