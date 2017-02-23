%module pydeepstream

%{
#include "deepstream.hpp"
#include "deepstream/buffer.hpp"
#include "deepstream/impl.hpp"
#include "deepstream/exception.hpp"
#include "deepstream/client.hpp"
%}
 
//%include "std_unique_ptr.i"

%include <std_except.i>
%include <std_string.i>
%include <std_vector.i>

namespace std {
   %template(vectorc) vector<char>;
};

wrap_unique_ptr(deepstreamErrorHandlerUniquePtr, deepstream::ErrorHandler);
wrap_unique_ptr(deepstreamClientUniquePtr, deepstream::Client);

%include "deepstream/buffer.hpp"
%include "deepstream/error_handler.hpp"
%include "deepstream/presence.hpp"
%include "deepstream/event.hpp"
%include "deepstream/client.hpp"
%include "deepstream.hpp"

