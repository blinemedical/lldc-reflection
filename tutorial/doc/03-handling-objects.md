# Handling Objects

Building off the first tutorial, this tutorial will show how to have an object with an object member (by value).  The design will then be extended to use a raw pointer member and finally a shared pointer member.

The files you will be editing are:

| Referred to as | Location |
| -------------- | -------- |
| _tutorial header_ | `lib/include/tutorial.hpp` |
| _tutorial source_ | `lib/src/tutorial.cpp` |
| _tutorial test_ | `tests/src/tutorial-test.cpp` |

## Making Third, Payload

Starting off, the `Third` structure will have a simple `Payload` member with a single `long` `value` member registered.  Edit the tutorial header to include them:

```cpp
struct Third {
  struct Payload {
    long value;

    RTTR_ENABLE();
  };

  Payload payload;

  RTTR_ENABLE();
};
```

If you have been through the other tutorials, the above is self explanatory, so let's move on to the registration in the tutorial source:

```cpp
::rttr::registration::class_<Third>("third")
  .property("payload", &Third::payload)
  ;

::rttr::registration::class_<Third::Payload>("third::payload")
  .property("value", &Third::Payload::value)
  ;
```

Again, you're 2 tutorials in and a pro at this (_right?_), so you wrote the above and went on to the test in the tutorial test:

```cpp
TEST(Third, ObjectMembers) {
  auto temp = json_from_string(R"({'payload': { 'value': 12 } })", NULL);
  ::tutorial::Third output;

  EXPECT_TRUE(converters::from_json_glib(temp, output));
  EXPECT_EQ(output.payload.value, 12);
  json_node_unref(temp);

  ::tutorial::Third input;
  input.payload.value = 83;

  EXPECT_TRUE((temp = converters::to_json_glib(input)));
  EXPECT_TRUE(converters::from_json_glib(temp, output));
  EXPECT_EQ(input.payload.value, output.payload.value);
  json_node_unref(temp);
}
```

Like the other tests, we verify that serialization from what we expect to be our raw source material, a JSON string object representation, works as expected.  And for good measure, the test verifies the to-from transition also works fine.

You compile and run the test; it works.  Good job.

## Raw Pointers

Let us say you have a structure with a member that you want to exist as a pointer.  Feeling confident, you go modify the tutorial header to initialize it as a `nullptr` and delete it if necessary in the destructor:

```cpp
  struct Third {
    struct Payload {
      Payload() {}
      virtual ~Payload() {}

      long value;

      RTTR_ENABLE();
    };

    Payload* payload; // <<<<< raw pointer

    Third() : payload(nullptr) { }

    ~Third() {
      if (payload)
        delete payload;
    }

    RTTR_ENABLE();
  };
```

Over in the tutorial source, you look at the registration.  The member name didn't change, so the registration is the same.  Let's keep going.

You continue to the test to change all your `.` to `->` and make sure you create a `Third::Payload` for that second half of the test:

```cpp
TEST(Third, ObjectMembers) {
  auto temp = json_from_string(R"({'payload': { 'value': 12 } })", NULL);
  ::tutorial::Third output;

  EXPECT_TRUE(converters::from_json_glib(temp, output));
  EXPECT_EQ(output.payload->value, 12);
  json_node_unref(temp);

  ::tutorial::Third input;
  input.payload = new ::tutorial::Third::Payload();
  input.payload->value = 83;

  EXPECT_TRUE((temp = converters::to_json_glib(input)));
  EXPECT_TRUE(converters::from_json_glib(temp, output));
  EXPECT_EQ(input.payload->value, output.payload->value);
  json_node_unref(temp);
}
```

You're a pro.  You got this.  You compile and run the test.  It segfaults.  What is missing?

In the debugger it occurs to you that the first `EXPECT_EQ` just hit an null pointer despite the conversion apparently succeeding.  RTTR needs to know how to make a `Third::Payload`.

Back at the tutorial source, decorate the `Third::Payload` registration:

```cpp
  ::rttr::registration::class_<Third::Payload>("third::payload")
    .constructor<>()(::rttr::policy::ctor::as_raw_ptr) // <_<
    .property("value", &Third::Payload::value)
    ;
```

You recompile and run the test again, it succeeds.

> NOTE: currently, the conversion routines in this library do not treat a missing / unknown constructor for a given type as an error worthy of aborting the deserialization from the intermediate type.

## Shared Pointers

Fresh off that previous experience and knowing you would rather use a smart pointer, you change the tutorial header:

```cpp
struct Third {
  struct Payload {
    Payload() {}
    virtual ~Payload() {}

    long value;

    RTTR_ENABLE();
  };

  std::shared_ptr<Payload> payload;

  Third() : payload(nullptr) { }

  ~Third() { }

  RTTR_ENABLE();
};
```

And then update the registration information for `Third::Payload` because you just learned that matters:

```cpp
::rttr::registration::class_<Third::Payload>("third::payload")
  .constructor<>()(::rttr::policy::ctor::as_std_shared_ptr)
  .property("value", &Third::Payload::value)
  ;
```

Next you update the tutorial test to reset the smart pointer with our instance where that mattered:

```cpp
TEST(Third, ObjectMembers) {
  auto temp = json_from_string(R"({'payload': { 'value': 12 } })", NULL);
  ::tutorial::Third output;

  EXPECT_TRUE(converters::from_json_glib(temp, output));
  EXPECT_EQ(output.payload->value, 12);
  json_node_unref(temp);

  ::tutorial::Third input;
  input.payload.reset(new ::tutorial::Third::Payload()); // <-- shared pointer
  input.payload->value = 83;

  EXPECT_TRUE((temp = converters::to_json_glib(input)));
  EXPECT_TRUE(converters::from_json_glib(temp, output));
  EXPECT_EQ(input.payload->value, output.payload->value);
  json_node_unref(temp);
}
```

You compile and run the test; it passes.  Good job.

## More Information

The `.constructor` registration has another policy for `as_object` for situations where that is necessary (MSVC is one instance where our first test would likely have failed).  Simply add it to your object registration and that constructor will now be known.

The registration also takes arguments, like `.constructor<types...>(args...)(policy)`.  Constructors of this type are not currently supported in the conversion routines since there would be no clear way to indicate from what those `args...` should be sourced.

## Conclusions

In this tutorial you learned how to deal with object members stored by value, raw pointer, and shared pointer.  We walked confidently straight into a segfault and then fixed it.  And you're ready for more.
