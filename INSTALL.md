How to build and install beluga
===============================

This package provides no automated way to install; `beluga` is not a big
compiler and manual installation is not quite difficult.

I wrote this guide based on installation onto my
[Gentoo Linux](https://www.gentoo.org) (x86-64) machine. Installing on
non-Linux systems probably requires meaningful changes on the package; for
example, headers to replace a part of the standard headers may vary.

In this document, the term `beluga`, depending on the context, refers to the
compiler implementation or the whole package including a preprocessor and a
driver.


#### Installation directory

Before installation, you need to determine directories into which built
executables, supporting libraries and headers are placed. In this document, it
is assumed that:

- executables are placed in `/usr/local/bin/`
- supporting libraries in `/usr/local/lib32/bcc/`
- headers in `/usr/local/lib32/bcc/include/` and
- intermediate files in `/tmp/`.

As indicated by the paths, this means system-wide or global installation. Local
installation is also possible by simply changing those paths to your local ones
(always use _absolute_ paths; e.g., `/home/username/var/bin/` instead of
`~/user/var/bin/` or `./var/bin/`).

Some of this configuration has to be put into the following files that are
incorporated to build `beluga`'s driver, `bcc`:

- `bcc/host/beluga.h` for the compiler (`beluga`)
- `bcc/host/as.h` for the assembler (`as`) and
- `bcc/host/ld.h` for the linker (`ld`).

If you find a directory under `bcc/` to contain configuration to meet your
needs, changing the simbolic link `bcc/host` to point to that directory will
save you labor.

`beluga.h` for my system looks like:

    "/usr/local/bin/beluga",
    "-U__GNUC__",
    /* "-D__STRICT_ANSI__", */
    "-D_POSIX_SOURCE",
    "-D__i386__",
    "-D__unix__",
    "-D__linux__",
    "-D__gnuc_va_list=va_list",
    "$1",
    "--include-builtin=/usr/local/lib32/bcc/include",
    "--include-builtin=/usr/local/lib32/bcc/gcc/include",
    "--include-builtin=/usr/local/include",
    "--include-builtin=/usr/local/lib32/bcc/gcc/include-fixed",
    "--include-builtin=/usr/include",
    "--target=x86-linux",
    "-v",
    "-o", "$3",
    "$2",

The first line invokes `beluga` installed in `/usr/local/bin/` and the rest
specifies options to it. `beluga.h` is processed by `#include` and thus C
comments exclude unnecessary lines. `"$1"`, `"$2"` and `"$3"` have special
meanings and will be replaced by user-provided options, an input file name and
an output file name respectively. For example, when you run `bcc` with these
options:

    -I/path/to/headers -o foo.o -c foo.c

options to the preprocessor (`-I/path/to/headers` in this example) substitute
for `$1`, `foo.c` does for `$2`. `$3` is replaced by a generated temporary name
to pass the result to the assembler.

`beluga` takes advantage of an assembler and a linker from the target system
and you have to ensure the driver be able to access them by giving proper paths
in `as.h` and `ld.h`.

This shows an example of `as.h`:

    "/usr/bin/as",
    "--32",
    "$1",
    "-o", "$3",
    "$2",

The `--32` option is to force the assembler to accept x86 assembly code on a
x86-64 system; `beluga` is a 32-bit compiler while my system is 64-bit.

`ld.h` looks complicated:

    "/usr/bin/ld",
    "-m", "elf_i386",
    "-dynamic-linker", "/lib/ld-linux.so.2",
    "/usr/lib32/crt1.o",
    "/usr/lib32/crti.o",
    "/usr/local/lib32/bcc/gcc/32/crtbegin.o",
    "-L/usr/local/lib32/bcc/gcc/32",
    "-L/usr/lib32",
    "-L/lib32",
    "-L/usr/x86_64-pc-linux-gnu/lib",
    "-L/usr/lib",
    "$2",
    "-lc",
    "/usr/local/lib32/bcc/gcc/32/crtend.o",
    "/usr/lib32/crtn.o",
    "-o", "$3",
    "$1",
    "/usr/local/lib32/bcc/xfloat.o",

In the linking phase, a set of
[start-up code](https://en.wikipedia.org/wiki/Crt0) and supporting libraries
are linked to build an executable, which explains why there are many options to
`ld`. The last one, `xfloat.o` is a support object file for compiler-provided
`float.h`.

Search paths for system headers, necessary start-up files and paths to system
libraries can be inspected by running an existing compiler (for example, `gcc`)
with an option to display program invocations as in:

    gcc -v hello.c


#### Configuration macros

When building `beluga`, it is necessary to define several macros properly to
select optional features and to pass environmental information.

The following macros are used by [`ocelot`](http://code.woong.org/ocelot/) that
`beluga` depends on:

- `MEM_MAXALIGN`: specifies the maximum alignment factor of storage returned by
  `malloc()`. 4 or 8 is a good choice on most systems.

The macros used in common include:

- `HAVE_COLOR`: makes `beluga` generate diagnostics in color;
- `HAVE_ICONV`: makes `beluga` take advantage of
  [libiconv](https://www.gnu.org/software/libiconv/) to process character
  encodings other than [ASCII](https://en.wikipedia.org/wiki/ASCII) from input
  files, multibyte and wide characters/strings; and
- `SHOW_WARNCODE`: makes `beluga` display options to control warnings when
  issueing them.

Macros for the preprocessor proper are:

- `HAVE_REALPATH`: makes the preprocessor use
  [`realpath()`](http://man7.org/linux/man-pages/man3/realpath.3.html) for
  path canonicalization and include optimization;
- `DIR_SEPARATOR`: a character to separate directories in paths. The
  default is `/` (no double quotes necessary). No need to change on Unix-like
  machines;
- `PATH_SEPARATOR`: a character to separate paths in `SYSTEM_HEADER_DIR`. The
  default is `:` (no double quotes necessary); and
- `SYSTEM_HEADER_DIR`: paths to search for system headers. This must be a C
  string, e.g., `"/usr/include:/usr/local/include"` (note double quotes).

Besides `SYSTEM_HEADER_DIR` used in build-time, there are two other ways to set
search paths for system headers. One is, as already explained, giving
`--include-builtin` options in `beluga.h`, and the other is using the
environmental variable `C_INCLUDE_PATH` in run-time. These all specify system
header paths.

`-I` options to the driver and the environmental variable `CPATH` exist for
non-system header paths, and they are searched first _before_ looking in
system paths. The exact order in which header files are searched for is as
follows:

- the current working directory (only for `#include "..."`),
- paths from `bcc`'s `-I` options (in _run-time_),
- paths from the environmental variable `CPATH` (in _run-time_)
- paths from `bcc`'s `-isystem` options (in _run-time_; system header paths
  start here),
- paths from the environmental variable `C_INCLUDE_PATH` (in _run-time_)
- paths from `--include-builtin` options given in `beluga.h` (in _build-time_)
- paths from the macro `SYSTEM_HEADER_DIR` (in _build-time_) and
- paths from `-idirafter` options (in _run-time_).

The `-nostdinc` option makes `beluga` skip system paths determined in
build-time by `--include-builtin` and `SYSTEM_HEADER_DIR`; other paths are
still inspected.

`beluga` does its best to ignore redundant paths and to keep non-system paths
from overriding system ones; for example, `-I /usr/include` is silently ignored
when `bcc` is built with `--include-builtin=/usr/include`.

Lastly, this macro is for the driver(`bcc`):

- `TMP_DIR`: driver's temporary directory. This macro must end with a directory
  separator. The default is `"/tmp/"` (note the double quotes).

When passing a C string with the `-D` option, do not forget to _escape_ double
quotes with backslashes; for instance, `-DTMP_DIR=\"var/tmp/\"`.


#### Building `beluga`

A usual setting to build `beluga` on a Unix-like machine is to run `make` on
the project root as follows:

    CFLAGS="-DMEM_MAXALIGN=4 -DHAVE_COLOR -DHAVE_ICONV -DSHOW_WARNCODE -DHAVE_REALPATH" make

If you are on a x86-64 machine, it is necessary to add `-m32` to both `CFLAGS`
and `LDFLAGS`:

    CFLAGS="-DMEM_MAXALIGN=4 -DHAVE_COLOR -DHAVE_ICONV -DSHOW_WARNCODE -DHAVE_REALPATH -m32" LDFLAGS="-m32" make

(Make sure that your system is able to build binaries for x86. For example,
running `yum install glibc-devel.i686 libgcc.i686` on
[Fedora-based distros](https://en.wikipedia.org/wiki/Fedora_%28operating_system%29)
and `sudo apt-get install gcc-multilib` on
[Ubuntu Linux](http://www.ubuntu.com) brings necessary components.)

Successful build of `beluga` generates two executables, `bcc` and `beluga` in
the `build/` directory.


#### Copying files

The generated executables have to be copied into the directory you decided to
make use of. Assuming you are on the project root,

    cp build/{bcc,beluga} /usr/local/bin/

will do that. (Of course, ensure you have proper permission, e.g., by letting
`sudo` run that command.)

Also copy a support object and headers to override existing ones:

    mkdir -p /usr/local/lib32/bcc
    cp build/xfloat.o /usr/local/lib32/bcc/
    cp -Lr build/include /usr/local/lib32/bcc/

`beluga` utilizes and therefore needs to refer to existing libraries and
headers for them. In order to avoid hard-coding a path to existing resources,
it is useful to create a symbolic link to them, which
`/usr/local/lib32/bcc/gcc` is for; for instance:

    ln -s /usr/lib/gcc/x86_64-pc-linux-gnu/4.9.3 /usr/local/lib32/bcc/gcc

on my machine. This path to `gcc`'s resources was also obtained from `gcc -v`.

We have finished to install `beluga`. By compiling a small program that uses
standard headers,

    bcc -W -Wall hello.c

you can examine your installation. Or by adding `CC=bcc` when triggering `make`
to build `beluga`, you can see `beluga` compile itself.


#### Any troubles?

If you encounter any problem while installing `beluga`, let me know so that I
can help. The version of your distro and simple description of the problem
would be enough.
