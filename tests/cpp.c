/* tests of some preprocessor directives that c-parser needs to handle  */

#line 403

#pragma ident "hi there"

#pragma foo-foo-foo-foo-foo

#define glue(x, y)   x##y

glue(ch,ar) c;
#

