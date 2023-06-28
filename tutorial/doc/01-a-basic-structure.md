# Adding a Basic Structure

Anytime you add or modify a structure and want to also expose that property (or function) to RTTR, you need to perform at least two steps:

1. Update the structure declaration (and definition, depending on what you are doing).
2. Update the related RTTR registration for the change(s).

The files you will be editing are:

| Referred to as | Location |
| -------------- | -------- |
| _tutorial header_ | `lib/include/tutorial.hpp` |
| _tutorial source_ | `lib/src/tutorial.cpp` |
| _tutorial test_ | `tests/src/tutorial-test.cpp` |

> **IMPORTANT:** Before we begin, please read the `main` in the tutorial test.  There is an important comment about our use of dynamic libraries here vs. potentially having to have RTTR load that library into memory for the test application.  If you find that the converter functions are doing nothing in the application you develop: this very well could be why!

## Creating the First Structure

Open the tutorial header file and find the marked area within the `tutorial` namespace where you should add your structure.  We will start with something very simple:

```cpp
// tutorial.hpp within the 'tutorial' namespace
struct First {
  std::string s_value;
  uint64_t ui64_value;

  friend bool operator==(const First &lhs, const First &rhs) {
    return (lhs.s_value == rhs.s_value) && (lhs.ui64_value == rhs.ui64_value);
  }

  RTTR_ENABLE();
};
```

The `RTTR_ENABLE(subclass or empty)` macro adds some private members to the class to facilitate RTTR's inspection of the type.  The `operator==` we'll use for now to help make tests easier to read.

## Registering the First Structure

Open the tutorial source and find the `RTTR_PLUGIN_REGISTRATION` body.

> **IMPORTANT:** The tutorial library is being compiled as a shared library, which implies one could have multiple libraries defined with structures/classes utilizing RTTR, also linked dynamically, and serialization will work* (see introduction note).  On the other hand if you link to RTTR statically, you will need to build `lldc-reflection` statically as well as your library _and_ use `RTTR_REGISTRATION` (vs. `_PLUGIN_`) in a single object file where _ALL_ of your RTTR-enabled structures will be registered.

Declare that this class (struct) exists, giving it a unique name.

```cpp
::rttr::registration::class_<First>("first");
```

If left completely empty like this, any valid object instance could be serialized to it as there are no property names to check or reject.  Prove that to yourself by updating `tutorial/tests/src/tutorial-test.cpp` with the following test:

```cpp
TEST(First, CanSerialize) {
  auto raw_input = R"({ 'anything': 42 })";
  auto input = json_from_string(raw_input, NULL);

  ::tutorial::First output;
  EXPECT_TRUE(lldc::reflection::converters::from_json_glib(input, output));
  EXPECT_TRUE(output.s_value.empty());
}
```

Recompile and run the test:

```
meson test -C builddir tutorial-test
```

It should succeed.

> _TAKEAWAY:_ If you do not register properties, there is no way for anything to happen. Likewise if you want it ignored, just do not mention it, however the library does provide metadata for declaring to the reader that something is being treated as optional.  It is covered in a later tutorial.

## Registering Properties

Let's map the string property by updating the tutorial source file:

```cpp
::rttr::registration::class_<First>("first")
  .property("something", &First::s_value)
  .property("anything", &First::ui64_value)
  ;
```

This is telling RTTR to marshal to/from the `s_value` member under the name `something` and `ui64_value` as `anything`.  Moreover by registering the properties, they are implicitly required the conversion routines.  Change the test to verify this new behavior:

```cpp
TEST(First, CanSerialize) {
  auto input_json = json_from_string(R"({ 'anything': 42 })", NULL);
  ::tutorial::First output;

  // This should fail since 'something' is now registered, but is not
  // in the input_json message.
  EXPECT_FALSE(converters::from_json_glib(input_json, output));
  json_node_unref(input_json);

  // Change the input_json to also include 'something'.  Now that required property
  // can be found in the source object so conversion will succeed.
  input_json = json_from_string(R"({'anything': 42, 'something': 'everything'})", NULL);
  output = ::tutorial::First();
  EXPECT_TRUE(converters::from_json_glib(input_json, output));
  EXPECT_FALSE(output.s_value.empty());
  EXPECT_EQ(output.s_value, "everything");
  EXPECT_EQ(output.ui64_value, 42);
  json_node_unref(input_json);

  // Likewise this should work.
  ::tutorial::First input;
  input.s_value    = "whatever";
  input.ui64_value = 86;
  EXPECT_TRUE((input_json = converters::to_json_glib(input)));
  EXPECT_TRUE(converters::from_json_glib(input_json, output));
  EXPECT_TRUE(input == output);
  json_node_unref(input_json);
}
```

Re-run the test (`meson test -C builddir tutorial-test`; if it does not automatically recompile, do that).  It should succed.

First, the test attempts to convert what is an incomplete representation of the `First` object to verify that it will fail to perform that conversion to an instance of that class.

Second, the test successfully converts a complete representation of the `First` object and verifies the output object has the expected values.

Third, the test proves the other half of the conversion works too -- it converts from an instance of `First` to the intermediate `JsonNode*` instance and then back to another instance of `First`, which it confirms is equal to the input value.

## Conclusion

In this tutorial, it was shown how to declare a C++ structure for handling via RTTR.  Also shown are the nuances of library loading and the default behavior of property registration, making the property _required to be present_ to successfully deserialize it from the intermediate type.
