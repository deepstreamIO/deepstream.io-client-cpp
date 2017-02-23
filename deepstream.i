%module deepstream

%{
#include "deepstream.hpp"
#include "deepstream/buffer.hpp"
#include "deepstream/client.hpp"
#include "deepstream/config.h"
#include "deepstream/event.hpp"
#include "deepstream/presence.hpp"
%}
 
/* Parse the header file to generate wrappers */
%include "deepstream.hpp"
%include "deepstream/buffer.hpp"
%include "deepstream/client.hpp"
%include "deepstream/config.h"
%include "deepstream/event.hpp"
%include "deepstream/presence.hpp"
