%module deepstream

%{
#include "deepstream/core.hpp"
%}

%include <std_except.i>
%include <std_string.i>
%include <std_vector.i>

namespace std {
   %template(vectorc) vector<char>;
};

%rename(EventSendFn) deepstream::Event::SendFn;

%include "deepstream/core/fwd.hpp"
%include "deepstream/core/buffer.hpp"
%include "deepstream/core/error_handler.hpp"
%include "deepstream/core/ws.hpp"
%include "deepstream/core/event.hpp"
%include "deepstream/core/presence.hpp"
%include "deepstream/core/client.hpp"
%include "deepstream/core/version.hpp"
