%module foo

%{
#include "Foo.hpp"
%}

%include "std_unique_ptr.i"

 //wrap_unique_ptr(FooUniquePtr, Foo::Foobar);

%include "Foo.hpp"
