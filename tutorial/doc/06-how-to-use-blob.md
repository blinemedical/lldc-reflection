# How to Use _Blob_

Extending the other tutorial on leveraging the optionality metadata, this tutorial looks at the _blob_ metadata and its potential use cases.  The general concept is having a member which should be stored as-is when doing the conversion _to_ a target type, and if encountered when deserializing back to the C++ type, again: leave it as-is.  The assumption used in this library's conveters are that the C++ type's member will be a string-like container.

The files you will be editing are:

| Referred to as | Location |
| -------------- | -------- |
| _tutorial header_ | `lib/include/tutorial.hpp` |
| _tutorial source_ | `lib/src/tutorial.cpp` |
| _tutorial test_ | `tests/src/tutorial-test.cpp` |

## Getting Started

We will be working with two structures, the main message and a payload which we will assume could be any number of other object types.  In the tutorial header, declare the message and a potential payload:

```cpp
struct BlobPayloadOne {
  BlobPayloadOne() {}
  virtual ~BlobPayloadOne() {}

  std::vector<long> values;

  RTTR_ENABLE();
};

struct BlobDelivery {
  BlobDelivery() {}
  virtual ~BlobDelivery() {}

  std::string payload;

  RTTR_ENABLE();
};
```

In the tutorial source, register the types and identify the `payload` member as being a blob.

```cpp
::rttr::registration::class_<BlobDelivery>("blob-delivery")
  .property("payload", &BlobDelivery::payload) (
    ::lldc::reflection::metadata::set_is_blob()
  )
  ;

::rttr::registration::class_<BlobPayloadOne>("blob-payload-one")
  .property("values", &BlobPayloadOne::values)
  ;
```

The tutorial test for this change will show that the whole incoming JSON can be converted to the `BlobDelivery` type, and that the `payload` member will be populated afterwards.  The test then goes on to verify that `payload` string is a valid JSON object resulting in a `BlobPayloadOne` type with a single number in its `values` vector, 5.

```cpp
TEST(Blob, BlobDelivery) {
  ::tutorial::BlobDelivery output;
  auto temp = json_from_string(R"({ 'payload': { 'values': [5] } })", nullptr);

  ASSERT_TRUE(temp);
  EXPECT_TRUE(converters::from_json_glib(temp, output));
  EXPECT_GT(output.payload.size(), 0);
  json_node_unref(temp);

  ::tutorial::BlobPayloadOne payload_one;
  EXPECT_TRUE(converters::json_glib::from_json(output.payload, payload_one));
  ASSERT_EQ(payload_one.values.size(), 1);
  EXPECT_EQ(payload_one.values[0], 5);
}
```

This shows how to potentially handle variations of a single base message without having to define multiple base types or re-serializing the whole incoming blob a second time as was shown in the second tutorial, _Getters, Setters, and Friends_.  But, if we combine the two patterns, we can choose portions of a JSON blob to be interpreted by different handlers.

## Alternative Pattern

As mentioned at the end of the previous section, it would be possible to combine the getter/setter pattern with the blobbing pattern to pull the serialization and deserialization routines into your own library for particular members of your objects.  That effort is left for the learner, however here is a rough sketch of the design:

1. You have a base class which provides virtual getter and setter methods for the raw type, typically `std::string`, along with a private instance of that type.  This class would be both `RTTR_ENABLE` and `RTTR_REGISTRATION_FRIEND`.
2. Register that base class and that property with its getter and setter.
3. Declare a derived class that implements and/or overrides the getter and setter methods of the base class.  Make those function definitions handle the conversions to and from `std::string` according to your needs.

Your derived class should inherit from the base class but does not need to have `RTTR_ENABLE` or be registered unless you are also adding additional properties to serialize.  Instead, you will leverage the converter like this:

```cpp
MyDerivedType output;
converters::from_json_glib(json_obj, (BaseType)output);
```

Casting to the base type ensures that the registration of the base class ensures the named property is appropriately mapped to the getter and setter, and inheritance will ensure your derived class methods are called for actually handling the contents.

This could also be a useful way to not store the value of a member in an object (like a secret phrase), instead using the setter as a means to perform configuration upon serialization.  _However_, as interesting as that sounds, keep in mind that the marshalling of the `output` type through the converter involves potentially _many_ copy calls.  Design your class accordingly.

## Conclusion

This tutorial briefly showcased how to retain-but-skip a particular member of an object by treating it as a blob (in this case, a JSON stringified object).  A few potential use cases for this feature were also discussed.
