#include <memory>
#include <string>
#include <iostream>
#include <Poco/Net/HTTPClientSession.h>

namespace Foo {

struct ErrorHandler {
    ErrorHandler() {}
    ~ErrorHandler() {}
};
  
struct Bar {
    Bar () {}
    Bar (std::shared_ptr<ErrorHandler> handler) : errorHandler_(handler) {}
  
    void baz(const std::string& s) { std::cout << "you said " << s << "\n"; }
    int wibble;

    std::shared_ptr<ErrorHandler> errorHandler_;

    ~Bar() {
 	std::cerr << "delete ~Bar()" << reinterpret_cast<void *>(this) << std::endl;
    }
};

std::unique_ptr<Bar> make_example() { 
    return std::unique_ptr<Bar>(new Bar ()); 
}

void dump_example(const std::unique_ptr<Bar>& in) {
    std::cout << in->wibble << "\n";
    in->baz("because you said so");
}

}
