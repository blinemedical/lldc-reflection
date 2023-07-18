# Reflection Library

This dynamic library is a collection of serialization tools written in C++ that utilize [RTTR](https://www.rttr.org/), the runtime reflection library.  The goal is to make handling API structures a trivial matter.

The library's only dependency is against RTTR itself, however its extensions for dealing with messages from other libraries like JsonGlib require the presence of those libraries.  If the library is not found in the environment, that feature is skipped.

This library's [Tutorial](tutorial/Tutorial.md) covers several aspects of RTTR registration that are supported by the conversion logic in this library, however there are many other options and policies available.  Please see that project's [documentation](https://www.rttr.org/doc/master/classes.html) for more information.

## Building

```
meson setup builddir
meson compile -C builddir
```

Various options exit in the `meson_options.txt`.  For example, enforcing the library builds with SocketIO-Client C++ message compatibility:

```
meson setup builddir -Dsocketio=true
meson compile -C builddir
```

## Usage

Aside from depending against this library, one must declare C++ structures according to the [RTTR documentation](https://www.rttr.org/).  Then separately, typically in an object file, one must register those types using the `RTTR_PLUGIN_REGISTRATION` macro.  This allows for multiple registrations to occur in potentially several dynamic libraries.

> NOTE: `static` libraries can be used, however only one location throughout the entire codebase may register those types using `RTTR_REGISTER`.  The RTTR documentation makes a more vague statement about the use of these macros, however in testing, this previous statement is true.  If you want to register types from multiple locations, the libraries must all be dynamic and `RTTR_PLUGIN_REGISTRATION` used.

This library includes a pair of headers at the top-level, `declaration.h` and `registration.h`, to help with organizing a codebase with the above concepts in mind.  In your header file where you are declaring structures, classes, use `declaration.h`.  When in an object file defining how RTTR should handle each object's members, the metadata, etc., use `registration.h`.  Each header has title comments pertaining to the typical usage of RTTR in each respect.

## Tutorial

This library includes a tutorial (see [Tutorial](tutorial/Tutorial.md)).  Configure the project for building the feature by setting the meson option: `-Dtutorial=enabled`.  Then, follow the linked tutorial.

> NOTE: this requires `json-glib` to also be available in the environment as its library includes a readily-available _to string_ API for easier debugging.
