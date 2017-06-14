ocelot: a language extension library
====================================

`ocelot` is a collection of libraries to provide features that the C language
lacks, various data structures that most programs use in common, and facilities
for interaction between a program and its environment.

This package collects libraries into three categories called `cbl`, `cdsl` and
`cel`. Libraries belonging to `cbl`(C basic library) provide features that the
the language lacks and include alternative memory allocators and an exception
handling facility. Those to `cdsl`(C data structure library) implement various
data structures frequently used by most programs. Those to `cel`(C environment
library) aid interaction with the execution environment.

The `src` directory contains sub-directories `cbl`, `cdsl` and `cel` for the
libraries of each category:

- `cbl`: C basic library
    - `arena.h/c`: arena library (lifetime-based memory allocator)
    - `assert.h/c`: assertion library
    - `except.h/c`: exception library
    - `memory.h/c`: memory library (for production)
    - `memory.h/memoryd.c`: memory library (for debugging)
    - `text.h/c`: text library (high-level string manipulation)
- `cdsl`: C data structure library
    - `bitv.h/c`: bit-vector library
    - `dlist.h/c`: doubly-linked list library
    - `dwa.h/c`: double-word arithmetic library
    - `hash.h/c`: hash library
    - `list.h/c`: list library (singly-linked list)
    - `set.h/c`: set library
    - `stack.h/c`: stack library
    - `table.h/c`: table library
- `cel`: C environment library
    - `conf.h/c`: configuration library (configuration file parser)
    - `opt.h/c`: option library (option parser)

Libraries had been documented with [doxygen](http://www.doxygen.org), and
changed to use [markdown](http://daringfireball.net/projects/markdown/) for
easier maintenance and access. The `doc` directory contains
[documentation](https://github.com/mycoboco/ocelot/tree/master/doc) for them.

`INSTALL.md` explains how to build and install the libraries. For the copyright
issues, see the accompanying `LICENSE.md` file.

_As of the 0.4.0 release which breaks backward compatibility, the
[soname](https://en.wikipedia.org/wiki/Soname) has been adjusted from 1.x to
0.x in order to match the release version._

If you have a question or suggestion, do not hesitate to contact me via email
(woong.jun at gmail.com) or web (http://code.woong.org/).
