# LLDC Reflection (RTTR) Tutorial

This tutorial provides the framework to compile a library and GTest -based application showing the steps necessary for using the lldc-reflection (and RTTR) libraries.  The goals of this document are to inform how to:

1. Define a structure with value types that can be handled by the reflection library.
2. Register your base structure with RTTR.
3. Define a GTest that can serialize and deserialize the structure (verify success).
4. Extend the design to include setters and getters for properties.
5. Extend the design further with an object member (by value).
6. Change the storage of the object member to a raw pointer.
7. Change the storage of the object member to a shared pointer.
8. Subclass a base message structure and register that subclass.
9. Leveraging enumerations to strings, values, etc.
10. Metadata: what does `optional` mean?
11. Metadata: what about optionally serializing a member if the value matches a reference?

## Getting Started

For the sake of simplifying debugging, this design will serialize to/from JsonGlib for now, as it has a _to string_ API for easier printing of the intermediate object as well as testing _from string_ direct serialization in a human-readable way.  Therefore enable both the example and use of JsonGlib by at the root directory of this library:

```
rm -rf builddir
meson setup -Djsonglib=true -Dtutorial=enabled builddir
```

Later compilation and tests are the usual:

```
meson compile -C builddir
meson test -C builddir
# etc.
```

The layout of this directory includes:

| Path | Description |
| ------ | --------- |
| `lib` | The library where you will be adding your structures and handling the RTTR Registration definitions. |
| `tests` | The GTest `main()` and related test cases. |

## Tutorial

1. [A Basic Structure (steps 1-3 above)](doc/01-a-basic-structure.md)
2. [Getters, Setters, and Friends (4 above)](doc/02-getters-setters-and-friends.md)
3. [Handling Objects (5, 6, 7)](doc/03-handling-objects.md)
4. [Having Some (Sub)Class (8, 9)](doc/04-having-some-subclass.md)
