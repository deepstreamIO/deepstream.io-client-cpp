%module foo

%{
#include "foo.hpp"
%}
 
/* Parse the header file to generate wrappers */
%include "foo.hpp"
