# Having Some (Sub)Class

This tutorial covers how to decorate your structures in such a way that RTTR understands the class hierarchy when serializing objects.  It will also leverage what we have learned about property registration (or the lack thereof) to show a potential way to deal with martialling APIs that have a consistent field for a message _subject_ or _topic_.  We will accomplish that by registering an enumeration.

The files you will be editing are:

| Referred to as | Location |
| -------------- | -------- |
| _tutorial header_ | `lib/include/tutorial.hpp` |
| _tutorial source_ | `lib/src/tutorial.cpp` |
| _tutorial test_ | `tests/src/tutorial-test.cpp` |

## Getting Started

Some JSON-based APIs have a member for _subject_ or _topic_ which is typically an enumeration based off a string value.  If your application were trying to route these messages (assuming they're unsolicited), the flow of your application is usually to check for that member, and once found, `switch` into a handler for that specific type of message.  Therefore in this section we will leverage what was learned in the various _Properties_ tutorial pieces to establish an `ApiBase` class with a `topic` member that is read-only (to the application).  We will then build off this to explore some avenues that might make this more friendly to C/C++ application development.

In the tutorial header, begin by adding:

```cpp
struct ApiBase {
  ApiBase(const std::string &topic = "") : __topic(topic) {}
  virtual ~ApiBase() {}

  auto Topic() { return __topic; }

  private:
  std::string __topic;

  RTTR_ENABLE();
  RTTR_REGISTRATION_FRIEND;
};
```

For now we will let the enumerated topic be just a string even though we know from experience this results in a lot of defensive programming patterns that we could get around if we were dealing with an actual `enum class` (we'll do that later).

Having said that caveat though, the above should look familiar.  The structure can be created with a `topic`, which will be used as our indicator of which subclass we can expect the members to match.  The _getter_ for that member will not be registered; it is for the application to use.  Instead we are identifying the `ApiBase` as a having RTTR as a friend.

Next, add a couple derived message classes to the tutorial header:

```cpp
struct ApiFirst : public ApiBase {
  ApiFirst() : ApiBase("first") {}
  virtual ~ApiFirst() {}

  std::vector<long> longs;

  RTTR_ENABLE(ApiBase);
};

struct ApiSecond : public ApiBase {
  ApiSecond() : ApiBase("second") {}
  virtual ~ApiSecond() {}

  std::map<std::string, std::string> kvps;

  RTTR_ENABLE(ApiBase);
};
```

> NOTE: The argument passed to `RTTR_ENABLE(...)` is the parent class.

Then change to the tutorial source to register these types:

```cpp
  ::rttr::registration::class_<ApiBase>("api-base")
    .property("topic", &ApiBase::__topic)
    ;

  ::rttr::registration::class_<ApiFirst>("api-first")
    .property("longs", &ApiFirst::longs)
    ;

  ::rttr::registration::class_<ApiSecond>("api-second")
    .property("kvps", &ApiSecond::kvps)
    ;
```

Again, there nothing extravagant or out-of-the-ordinary with what was added.  The base and derived classes are all registered with their unique properties that are specific to the derived class itself.

Now, continue over to the tutorial test to do a basic sanity check for that base class:

```cpp
TEST(ApiTutorial, TopicIsRequired) {
  ::tutorial::ApiBase out_base;
  auto temp = json_from_string(R"({ 'longs': [5,6,7] })", NULL);

  // Expect this to fail since 'topic' is required and missing.
  EXPECT_FALSE(converters::from_json_glib(temp, out_base));
  json_node_unref(temp);
}
```

Compile and run the test.  It passes -- so far so good, right?  Now, let's verify we can inspect whatever that topic is set to be via this test:

```cpp
TEST(ApiTutorial, TopicCanBeInspected) {
  ::tutorial::ApiBase out_base;

  // This should work, as the only required field is 'topic' and the ApiBase class
  // doesn't care what it is set to be.
  auto temp = json_from_string(R"({ 'topic': 'first', 'longs': [1,2,3] })", NULL);
  EXPECT_TRUE(converters::from_json_glib(temp, out_base));
  EXPECT_EQ(out_base.Topic(), "first");
  json_node_unref(temp);
}
```

Compile and run the tests again.  Both tests pass because the only required field is `topic` and it can be any string.  Next, let's verify that we can _NOT_ ingest a malformed message:

```cpp
TEST(ApiTutorial, GuardOnTopic) {
  auto temp = json_from_string(R"({ 'topic': 'second', 'longs': [1,2,3] })", NULL);
  ::tutorial::ApiFirst out_first;

  // This should _NOT_ work.  The object is malformed; the topic matches ApiSecond but the
  // other member, 'longs', is from ApiFirst.
  EXPECT_FALSE(converters::from_json_glib(temp, out_first));
  json_node_unref(temp);
}
```

Compile, run, fail.  In the next section, we will work towards fixing this by adding an enumeration and guarding on changing it.

## Leveraging an Enumeration as a Guard

Over in the tutorial header, change `ApiBase` to use an enumeration:

```cpp
struct ApiBase {
  enum class TopicName {
    not_set = -1,
    first,
    second
  };

  ApiBase() : __topic(TopicName::not_set) {}

  virtual ~ApiBase() {}

  auto Topic() { return __topic; }

  protected:
  ApiBase(TopicName topic) : __topic(topic) {}

  private:
  void __Topic(TopicName topic);

  TopicName __topic = TopicName::not_set;

  RTTR_ENABLE();
  RTTR_REGISTRATION_FRIEND;
};
```

> NOTE: we can debate whether `ApiBase` should "own" the enumeration but just go with me here.  There's no technical reason I did this; it would work fine if the `enum class` were in some other accessible namespace.

We're keeping `RTTR_REGISTRATION_FRIEND` so that RTTR can call the new setter for `topic`: `__Topic(topic)`.  The implementation of that method will come in a moment which will serve as our guard against trying to marshall one JSON message into the wrong destination type by letting `topic` be our guard.  The protected constructor will be used by derived classes for setting the `topic` to the associated value for that derived class.

Still in the tutorial header, update the derived classes accordingly:

```cpp
struct ApiFirst : public ApiBase {
  ApiFirst() : ApiBase(TopicName::first) {}
  virtual ~ApiFirst() {}

  std::vector<long> longs;

  RTTR_ENABLE(ApiBase);
};

struct ApiSecond : public ApiBase {
  ApiSecond() : ApiBase(TopicName::second) {}
  virtual ~ApiSecond() {}

  std::map<std::string, std::string> kvps;

  RTTR_ENABLE(ApiBase);
};
```

In the tutorial source, and above the `RTTR_REGISTRATION` section, define `ApiBase::__Topic(topic)`:

```cpp
void tutorial::ApiBase::__Topic(TopicName topic) {
  // Allow this to get serialized to anything exactly once.
  if (__topic != TopicName::not_set && topic != __topic)
    throw ::lldc::reflection::exceptions::ReferenceValueComparisonMismatch();
  __topic = topic;
}
// vvv --- RTTR_PLUGIN_REGISTRATION --- vvv
```

If RTTR attempts to set the topic to something other than the currently-set value (and it isn't the default) then throw an exception.  Otherwise, store the incoming value.  This will allow our inspection to work as well as guard against trying to _change_ the topic when fully deserializing the message.

Within the `RTTR_PLUGIN_REGISTRATION` section, add the enumeration:

```cpp
::rttr::registration::enumeration<ApiBase::TopicName>("api-base::topic-name")
(
  rttr::value("not_set", ApiBase::TopicName::not_set),
  rttr::value("first", ApiBase::TopicName::first),
  rttr::value("second", ApiBase::TopicName::second)
);
```

The above change in the tutorial source allows for RTTR to marshall a string value of `first` in this case to a value of 0, making the writing of switch statements and other comparisons fairly cheap compared to dealing with strings everywhere.

We also need to update the `ApiBase` registration to use the getter/setter for the `topic` member so that the guard functionality will work:

```cpp
::rttr::registration::class_<ApiBase>("api-base")
  .property("topic", &ApiBase::Topic, &ApiBase::__Topic)
  ;
```

Next, update the tutorial test.  The `TopicIsRequired` test needs no changes, however `TopicCanBeInspected` need to change to using the enumeration:

```cpp
TEST(ApiTutorial, TopicCanBeInspected) {
  ::tutorial::ApiBase out_base;

  // This should work, as the only required field is 'topic' and the ApiBase class
  // doesn't care what it is set to be.
  auto temp = json_from_string(R"({ 'topic': 'first', 'longs': [1,2,3] })", NULL);
  EXPECT_TRUE(converters::from_json_glib(temp, out_base));
  EXPECT_EQ(out_base.Topic(), ::tutorial::ApiBase::TopicName::first);
  json_node_unref(temp);
}
```

Compile and run the tests.  The previously-failing `GuardOnTopic` test now passes because thrown exception _prevented_ converting that incoming object into the wrong destination type.

> For the learner: What would you need to add to that test to verify the referenced JSON cannot be read into an `ApiSecond` object either (since it has no `longs` member and the `kvps` is missing)?

Thanks to the above, one could conceivably write an application which routes messages based on the topic by using the base class as a template:

```cpp
// bool handle_api_first(const &ApiFirst msg) { ... }

bool initial_handler (JsonNode *raw) {
  ::tutorial::ApiBase temp;

  if (!converters::from_json_glib(raw, temp))
    return false;

  bool handled = false;
  switch (temp.Topic()) {
    case ApiBase::TopicName::first: {
      ::tutorial::ApiFirst temp_first;
      if (converters::from_json_glib(raw, temp_first))
        handled = handle_api_first(temp_first);
      break;
    }
    // and so forth...
    default:
      break;
  }

  return handled;
}
```

The alternative to this would be repeated attempts at conversion, like this:

```cpp
// bool handle_api_first(const &ApiFirst msg) { ... }

bool initial_handler (JsonNode *raw) {
  bool handled = false;
  ::tutorial::ApiFirst temp_first;
  ::tutorial::ApiSecond temp_second;
  // and so on, and on...

  if (converters::from_json_glib(raw, temp_first)) {
    handled = handle_api_first(temp_first);
  }
  else if (/** wheeee.... **/) {
    // deal with an ApiSecond
  }

  return handled;
}
```

Which is more performant will be related to how many messages one has to serialize to get the right one and not so much how large the incoming JSON blob is.  The conversion to the `JsonNode` in this case would have occurred already exactly once.  The conversion from `JsonNode` to the target type is constrained by the definition of the target type (i.e., the `ApiBase` only has a single property, so the conversion process only checks for that one named member in the `JsonNode`).

## Conclusions

This tutorial showed how to identify subclasses of structures to establish a message hierarchy that one might see in an an API.  A `topic` enumeration was then added along with making use of the getter/setter registration to establish our serialization/deserialization guard so that the reflection library can do the defensive programming for us at the time of conversion.  And finally, a pair of examples were shown for how this API might look in a message router application.
