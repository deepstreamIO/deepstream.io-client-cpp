%module foo

%{
#include "Foo.hpp"
#include "deepstream.hpp"
%}

%include "std_unique_ptr.i"

%include <std_except.i>
%include <std_string.i>
%include <std_vector.i>

namespace std {
   %template(vectorc) vector<char>;
};

wrap_unique_ptr(FooBarUniquePtr, Foo::Bar);

//%include "Foo.hpp"
%include "deepstream/event.hpp"
%include "deepstream/buffer.hpp"
%include "deepstream/presence.hpp"
%include "deepstream.hpp"
