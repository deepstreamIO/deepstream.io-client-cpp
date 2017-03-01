#include <deepstream.hpp>

#include <algorithm> // std::copy
#include <iostream> // std::cout
#include <iterator> // std::ostream_iterator

int main()
{
    deepstream::Object object;
    deepstream::Object nestedMap;

    deepstream::Array array; // hmm. what about brace-list initializers???

    for (int i = 0; i < 10; i++) {
        array.values.push_back(i);
    }

    nestedMap.values["e"] = 2.71828;

    object.values["true"] = true;
    object.values["false"] = false;
    object.values["pi"] = 22.0 / 7.0;
    object.values["name"] = deepstream::String("hello world");
    object.values["nested"] = nestedMap;
    object.values["array"] = array;

    std::cout << "ARRAY = " << deepstream::toJSON(array) << std::endl;
    std::cout << "MAP   = " << deepstream::toJSON(object) << std::endl;
}
