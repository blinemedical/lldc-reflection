# Optional Data Members

This tutorial introduces an RTTR feature: metadata.  The reflection library uses that feature to codify the idea of _optional_ members and members whose value should determine if it is treated as _optional_.  Testing in this section will involve printing our results into the test log (i.e., visual verification).

The files you will be editing are:

| Referred to as | Location |
| -------------- | -------- |
| _tutorial header_ | `lib/include/tutorial.hpp` |
| _tutorial source_ | `lib/src/tutorial.cpp` |
| _tutorial test_ | `tests/src/tutorial-test.cpp` |

## A Foreword on the Converters

This reflection library's converters use of _optional_ members is nuanced since some members are value types whereas other members have the ability to be _empty_ or otherwise appear _not set_ (`null`).  It is important to keep those distinctions in mind when extending this tutorial's design with other container types.

Moreover the concept of _optional_ is applied bidirectionally.  In the conversion _away from_ the C++ type, _optional_ provides a way to **not represent** a member in the target type if the source meets some condition(s) like emptiness.  Conversely, restoring to the C++ type,  _optional_ provides a way to **not fail** deserialization (return `false`) if the source type is missing the member entirely.

Then there is the concept of _optional with a default_.  This metadata feature means almost nothing on the restoration side since if the source type contains the member, the converter will make an attempt to apply it to the target C++ type.  On the other hand, in conversions _from_ the C++ type to a target type, comparing the named _default_ (via `==`) to the stored value of the source type provides the path to not represent that member in the target type.  If the value does not match the default, it is treated as required.

This tutorial will provide some examples of these statements; the tests in the reflection library provide several others.

## Getting Started

Let us begin with a structure having value members, one of which has the concept of being empty.  To the tutorial header, add:

```cpp
struct Sometimes {
  Sometimes() {}
  virtual ~Sometimes() {}

  long number;
  std::string text;

  RTTR_ENABLE();
};
```

In the tutorial source, register the new type:

```cpp
::rttr::registration::class_<Sometimes>("sometimes")
  .property("number", &Sometimes::number)
  .property("text", &Sometimes::text)
  ;
```

Both members are now **required** for serializing both _to_ and _from_ the target type, `JsonNode`.  Let's add a test and look at the log output.  Add the following to the tutorial test:

```cpp
TEST(Optional, PrintSometimes) {
  ::tutorial::Sometimes sometimes;
  JsonNode* temp = NULL;

  ASSERT_TRUE((temp = converters::to_json_glib(sometimes)));
  auto out = json_to_string(temp, true);

  std::cout << out << std::endl;

  g_free(out);
  json_node_unref(temp);
}
```

Compile and run the test, then open the log.  You should see something like the following since `std::string` would have initialized empty, and the value is just whatever happened to be in memory (depending on what compiler you are using):

```bash
[----------] 1 test from Optional
[ RUN      ] Optional.PrintSometimes
{
  "number" : 94635773910520,
  "text" : ""
}
[       OK ] Optional.PrintSometimes (0 ms)
[----------] 1 test from Optional (0 ms total)
```

Great.  Both are required, therefore both are represented in the target type.

## Behavior of _Optional_

Now, let's go make `text` optional.  Open the tutorial source and make that change:

```cpp
::rttr::registration::class_<Sometimes>("sometimes")
  .property("number", &Sometimes::number)
  .property("text", &Sometimes::text) (
    ::lldc::reflection::metadata::set_is_optional()
  )
  ;
```

Compile, run the test again, and check the log.  It should again have `number` printed, but now `text`, because it's _optional_ and empty-like, is omitted.

```bash
[----------] 1 test from Optional
[ RUN      ] Optional.PrintSometimes
{
  "number" : 94404035604360
}
[       OK ] Optional.PrintSometimes (0 ms)
[----------] 1 test from Optional (0 ms total)
```

Repeat that procedure for `number` back in the tutorial source:

```cpp
::rttr::registration::class_<Sometimes>("sometimes")
  .property("number", &Sometimes::number) (
    ::lldc::reflection::metadata::set_is_optional()
  )
  .property("text", &Sometimes::text) (
    ::lldc::reflection::metadata::set_is_optional()
  )
  ;
```

Compile and re-run the test.

You'll observe the `number` member is still printed despite being optional.  There is no way to check for emptiness on this type, so as far as it pertains to the converter, _optional_ in this context really only means that if the **incoming** JSON blob is missing the member, that's fine, "please continue."  For the impacted member, you will get whatever the class-initialized value of that member happened to be once the conversion is finished.  If you want a value -type member to be skipped in the conversion _to_ the intermediate type, you need to provide a default reference value.

> If you want a value -type member to be skipped in the conversion _to_ the intermediate type, you need to provide a default reference value.

As an exercise left to the learner, what would you expect for marking a pointer -like member as optional?  (_Answer:_ if it's `null`, it will be skipped.)

## Behavior of _Optional with Default_

Now, how can we omit `number` from the JSON object?  This is where the _optional with default_ behavior becomes useful:

```cpp
// tutorial source
::rttr::registration::class_<Sometimes>("sometimes")
  .property("number", &Sometimes::number) (
    ::lldc::reflection::metadata::set_is_optional_with_default(1234)
  )
  .property("text", &Sometimes::text) (
    ::lldc::reflection::metadata::set_is_optional()
  )
  ;
```

Modify the test to run the procedure twice, once with the `number` member set to the magic `1234` value:

```cpp
TEST(Optional, PrintSometimes) {
  ::tutorial::Sometimes sometimes;
  JsonNode* temp = NULL;
  char* out = NULL;

  sometimes.number = 1233;
  ASSERT_TRUE((temp = converters::to_json_glib(sometimes)));
  ASSERT_TRUE((out = json_to_string(temp, true)));
  std::cout << out << std::endl;
  g_free(out);
  json_node_unref(temp);

  sometimes.number = 1234;
  ASSERT_TRUE((temp = converters::to_json_glib(sometimes)));
  ASSERT_TRUE((out = json_to_string(temp, true)));
  std::cout << out << std::endl;
  g_free(out);
  json_node_unref(temp);
}
```

Compile and re-run the test, the log output should show an object with `number` set to `1233` (non-default value) and the second object will be empty, since both members now can meet the optionality criteria.

```bash
[----------] 1 test from Optional
[ RUN      ] Optional.PrintSometimes
{
  "number" : 1233
}
{}
[       OK ] Optional.PrintSometimes (0 ms)
[----------] 1 test from Optional (0 ms total)
```

How does this work for object members that can support empty-like checks?  Let's find out.

```cpp
// tutorial source
::rttr::registration::class_<Sometimes>("sometimes")
  .property("number", &Sometimes::number)
    (::lldc::reflection::metadata::set_is_optional_with_default(1234))
  .property("text", &Sometimes::text)
    (::lldc::reflection::metadata::set_is_optional_with_default("default"))
  ;
```

Now, update the tutorial test to validate that `text` will be skipped if it matches `"default"`,  but will be present if it's empty (since that is not the default):

```cpp
TEST(Optional, PrintSometimes) {
  ::tutorial::Sometimes sometimes;
  JsonNode* temp = NULL;
  char* out = NULL;

  // If it matches the default value, it should not be present
  // in the JSON blob.
  sometimes.number = 0;
  sometimes.text = "default";
  ASSERT_TRUE((temp = converters::to_json_glib(sometimes)));

  auto temp_obj = json_node_get_object(temp);
  EXPECT_FALSE(json_object_has_member(temp_obj, "text"));

  ASSERT_TRUE((out = json_to_string(temp, true)));
  std::cout << out << std::endl;
  g_free(out);
  json_node_unref(temp);

  // If it is empty-like (i.e., empty string), then it should
  // be represented as an empty string in the JSON blob since
  // it does not match the default value.
  sometimes.text.clear();
  ASSERT_TRUE((temp = converters::to_json_glib(sometimes)));

  temp_obj = json_node_get_object(temp);
  EXPECT_TRUE(json_object_has_member(temp_obj, "text"));

  ASSERT_TRUE((out = json_to_string(temp, true)));
  std::cout << out << std::endl;
  g_free(out);
  json_node_unref(temp);
}
```

Compile and run the test.  Your results should be:

```bash
[----------] 1 test from Optional
[ RUN      ] Optional.PrintSometimes
{
  "number" : 0
}
{
  "number" : 0,
  "text" : ""
}
[       OK ] Optional.PrintSometimes (0 ms)
[----------] 1 test from Optional (0 ms total)
```

The first check is now skipping the `text` member because it was set to the default, and the second check included `text` because it no longer matched the default.

## Conclusions

This tutorial discussed the _Optional_ and _Optional with Default_ metadata features provided by this library.  You learned how to declare members as optional, explored the behavior of value vs. empty-like members, and then explored the impact of specifying a default value.
