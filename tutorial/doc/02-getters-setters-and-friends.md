# Getters, Setters, and Friends

This tutorial builds off the introduction, which showed how to register a structure, its properties, and perform conversions in the test environment, by showing how to have RTTR identify getters and setters (public methods) and leverage being a `friend`.

The files you will be editing are:

| Referred to as | Location |
| -------------- | -------- |
| _tutorial header_ | `lib/include/tutorial.hpp` |
| _tutorial source_ | `lib/src/tutorial.cpp` |
| _tutorial test_ | `tests/src/tutorial-test.cpp` |

## Creating "Second"

To the tutorial header, add this structure, `Second`, which has a single private member, a `long`, called `__member` and public getter/setter methods for that member.

```cpp
struct Second {
  auto GetMember() { return __member; }
  void SetMember(long m) { __member = m; }

  private:
  long __member;

  RTTR_ENABLE();
};
```

In the tutorial source, register the property with the getter/setter:

```cpp
::rttr::registration::class_<Second>("second")
  .property("member", &Second::GetMember, &Second::SetMember)
  ;
```

And in the tutorial test, add something simple like this:

```cpp
TEST(Second, GetterSetter) {
  ::tutorial::Second input, output;
  JsonNode* temp = nullptr;

  input.SetMember(50);
  EXPECT_TRUE((temp = converters::to_json_glib(input)));
  EXPECT_TRUE(converters::from_json_glib(temp, output));
  EXPECT_EQ(input.GetMember(), output.GetMember());
  json_node_unref(temp);
}
```

Compile and run the test.  It should succeed.

As seen in the first tutorial, this test serializes and deserializes an object, then verifies the resulting objects are equal.  Since the property getter/setter were used in the registration, this all succeeds fine.

> NOTE: RTTR also supports read-only registration, a topic not covered in this stack of tutorials but [it exists](https://www.rttr.org/doc/master/classrttr_1_1registration.html).

## Being Friends

What if that object's member should not be accessible?  For that, one can register the object with RTTR as a friend.  Modify the tutorial header definition to include:

```cpp
struct FriendSecond {
  auto GetMember() { return __member; }

  private:
  long __member;

  RTTR_REGISTRATION_FRIEND;
};
```

> NOTE: In the above structure, the _getter_ is being included as a means to verify the serialization in the test.

Next, register the structure as you normally would in the tutorial source file:

```cpp
::rttr::registration::class_<FriendSecond>("friend-second")
  .property("member", &FriendSecond::__member)
  ;
```

Then add a simple test to verify RTTR can set `__member` for us via that registered `member` property:

```cpp
TEST(FriendSecond, Friendly) {
  auto input = json_from_string(R"({'member': 58})", NULL);
  ::tutorial::FriendSecond output;

  EXPECT_TRUE(converters::from_json_glib(input, output));
  EXPECT_EQ(output.GetMember(), 58);
  json_node_unref(input);
}
```

Compile and run the test.

The test deserializes an object that would match `FriendSecond`'s registered structure, a single property named `member`, and verifies that the stored value matches what is expected by using a public interface (purely there for conciseness of the tutorial).

## Conclusion

Covered in this tutorial were alternative methods for handling internal properties.  One can either register property getter/setter methods, mark the class as a `friend` of RTTR, or (not shown) register the property as read-only.
