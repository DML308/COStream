tricky.c:1: Warning: redundant storage class
tricky.c:13: multiple declarations of structure `hi'
	Previous definition: tricky.c:11
tricky.c:14: multiple declarations of union `there'
	Previous definition: tricky.c:12
tricky.c:16: multiple declarations of enum `test'
	Previous definition: tricky.c:15
tricky.c:19: conflicting storage classes `static ' and `typedef '
tricky.c:20: Warning: redundant type qualifier
tricky.c:21: Warning: redundant type qualifier
tricky.c:23: Warning: repeated type specifier: `unsigned'
tricky.c:24: Warning: repeated type specifier: `signed'
tricky.c:29: Warning: repeated type specifier: `unsigned'
tricky.c:30: Warning: repeated type specifier: `signed'
tricky.c:31: Warning: redundant storage class
tricky.c:41: extern `y' redeclared
	Previous declaration: tricky.c:5
tricky.c:43: Warning: `x' shadows previous declaration
	Previous declaration: tricky.c:3
tricky.c:50: illegal storage class for parameter `x'
tricky.c:50: illegal storage class for parameter `T'
tricky.c:54: illegal storage class for parameter `y'
tricky.c:54: Warning: `y' shadows previous declaration
	Previous declaration: tricky.c:41
tricky.c:55: Warning: `x' shadows previous declaration
	Previous declaration: tricky.c:3
tricky.c:56: Warning: `x1' shadows previous declaration
	Previous declaration: tricky.c:35
tricky.c:65: multiple definitions of label `top'
tricky.c:56: Warning: unused variable `x1'
tricky.c:70: Warning: `x' shadows previous declaration
	Previous declaration: tricky.c:3
tricky.c:71: Warning: `p' shadows previous declaration
	Previous declaration: tricky.c:24
tricky.c:72: Warning: `q' shadows previous declaration
	Previous declaration: tricky.c:23
tricky.c:75: parse error
tricky.c:75: declaration without a variable
tricky.c:74: Warning: unused variable `q3'
tricky.c:73: Warning: unused variable `q2'
tricky.c:72: Warning: unused variable `q'
tricky.c:81: Warning: `p2' shadows previous declaration
	Previous declaration: tricky.c:25
tricky.c:81: Warning: unused variable `p2'
tricky.c:94: Warning: `x' shadows previous declaration
	Previous declaration: tricky.c:3
tricky.c:95: Warning: `x' shadows previous declaration
	Previous declaration: tricky.c:3
tricky.c:96: Warning: `x' shadows previous declaration
	Previous declaration: tricky.c:3
tricky.c:97: Warning: `x' shadows previous declaration
	Previous declaration: tricky.c:3
tricky.c:98: Warning: `x' shadows previous declaration
	Previous declaration: tricky.c:3

Compilation Failed: 11 errors, 26 warnings
