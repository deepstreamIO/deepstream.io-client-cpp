%module deepstream

%{
#include "deepstream.hpp"
#include "deepstream/buffer.hpp"
#include "deepstream/exception.hpp"
#include "deepstream/client.hpp"
#include "deepstream/presence.hpp"
#include "deepstream/error_handler.hpp"
%}
 
//%include "std_unique_ptr.i"

#if 0

%import <std_alloc.i>
%import <std_array.i>
%import <std_basic_string.i>
%import <std_common.i>
%import <std_container.i>
%import <std_deque.i>
%import <std_except.i>
%import <std_ios.i>
%import <std_iostream.i>
%import <std_list.i>
%import <std_map.i>
%import <std_multimap.i>
%import <std_multiset.i>
%import <std_pair.i>
%import <std_set.i>
%import <std_sstream.i>
%import <std_streambuf.i>
%import <std_string.i>
%import <std_unordered_map.i>
%import <std_unordered_multimap.i>
%import <std_unordered_multiset.i>
%import <std_unordered_set.i>
%import <std_vectora.i>
%import <std_vector.i>

#endif

%include <std_except.i>
%include <std_string.i>
%include <std_vector.i>

namespace std {
   %template(vectorc) vector<char>;
};

/* wrap_unique_ptr(deepstreamErrorHandlerUniquePtr, deepstream::ErrorHandler); */
/* wrap_unique_ptr(deepstreamClientUniquePtr, deepstream::Client); */

%include "deepstream/buffer.hpp"
%include "deepstream/error_handler.hpp"
%include "deepstream/presence.hpp"
%include "deepstream/event.hpp"
%include "deepstream/client.hpp"
%include "deepstream.hpp"

