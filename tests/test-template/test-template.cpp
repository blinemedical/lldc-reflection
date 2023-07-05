#include <gtest/gtest.h>
#include <common/common.h>

#if TEST_JSON_GLIB
  #include <lldc-reflection/converters/json-glib.h>
  #define to_conversion lldc::reflection::converters::to_json_glib
  #define from_conversion lldc::reflection::converters::from_json_glib
  #define uut_type JsonNode*
  #define uut_unref(t) {if (t) json_node_unref(t);}

#elif TEST_SOCKET_IO
  #include <lldc-reflection/converters/socket-io.h>
  #define to_conversion lldc::reflection::converters::to_socket_io
  #define from_conversion lldc::reflection::converters::from_socket_io
  #define uut_type sio::message::ptr
  #define uut_unref(t) t.reset()

#else
  #error Must define a test type
#endif

using namespace lldc::testing;

template <typename T>
static bool member_check_function(T ref, const std::string& name) {
  bool result = false;

#if TEST_JSON_GLIB
  auto ref_obj = json_node_get_object(ref);
  result = json_object_has_member(ref_obj, name.c_str());

#elif TEST_SOCKET_IO
  auto ref_obj = ref->get_map();
  result = (ref_obj.end() != ref_obj.find(name));
#endif

  return result;
}

/**
 * @brief Unit testing apparatus for the behavior
 * of the ApiMessage::[Get/Set]Subject API as it
 * pertains to the reflection library to make
 * SetSubject public, facilitating testing the
 * exceptions, etc.
 */
struct SubjectGuardUUT : ApiMessage
{
  SubjectGuardUUT(Subject subject) :
    ApiMessage(subject) {}

  void SetSubject(Subject subject) {
    ApiMessage::SetSubject(subject);
  }
};

TEST(Examples, SetOnceBehavior) {
  /**
   * @brief The 'subject' field of the base class can be set
   * to 'not_set' (default) for the sake of introspecting the
   * message type before fully converting it.  Derived classes
   * set the value to something other than 'not_set', which
   * enables a guard for converting that will throw an exception
   * if the converter tries to set the subject to something the
   * derived class is not configured to accept.  This test:
   *
   * 1. Creates the base message
   * 2. Sets the subject to something non -'not_set'
   * 3. Verifies the next attempt to set it to something not (2)
   *    results in an exception.
   */
  SubjectGuardUUT uut(Subject::not_set);

  EXPECT_NO_THROW(uut.SetSubject(Subject::second_message));
  EXPECT_THROW(uut.SetSubject(Subject::first_message),
    lldc::reflection::exceptions::ReferenceValueComparisonMismatch);
}

/**
 * @brief The behavior of the 'from' conversion is to return
 * false if it could not be completed.  The exception thrown
 * when the subject is set to something mismatched is a solid
 * reason for returning false, so verify we get false on the
 * bad conversion.
 */
TEST(Examples, GuardIncorrectConversions) {
  FirstMessage uut_first;
  SecondMessage uut_second;
  uut_type converted_first = nullptr;

  EXPECT_NE(uut_first.GetSubject(), uut_second.GetSubject());
  EXPECT_NO_THROW(converted_first = to_conversion(uut_first));
  EXPECT_FALSE(from_conversion(converted_first, uut_second));
  uut_unref(converted_first);
}

/**
 * @brief Use an ApiMessage with 'not_set' subject as a means
 * to determine what the message type should be and then verify
 * serializing to it works.  This also verifies the non-destructive
 * nature of the 'from' converter to support this behavior.
 */
TEST(Examples, InspectableProperty) {
  ApiMessage tester;
  FirstMessage first;
  SecondMessage second;
  SecondMessage input;
  uut_type temp = nullptr;

  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(from_conversion(temp, tester));
  EXPECT_EQ(tester.GetSubject(), input.GetSubject());
  EXPECT_FALSE(from_conversion(temp, first));
  EXPECT_TRUE(from_conversion(temp, second));

  uut_unref(temp);
}

TEST(Examples, ApiMessage) {
  /**
   * @brief Verify the ApiMessage can be converted to sio and back
   * again.  Setting the subject as 'first_message' will verify that
   * when converted back from the SocketIO Message type that the
   * value matches the input.
   */
  ApiMessage uut (Subject::first_message);
  ApiMessage out;
  uut_type converted_uut = nullptr;

  EXPECT_NO_THROW(converted_uut = to_conversion(uut));
  EXPECT_TRUE(from_conversion(converted_uut, out));
  EXPECT_EQ(uut.GetSubject(), out.GetSubject());

  uut_unref(converted_uut);
}

TEST(Examples, FirstMessage) {
  FirstMessage input, output;
  uut_type temp = nullptr;

  input.body.data["some_key"] = "some_value";
  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(input, output);

  uut_unref(temp);
}

TEST(Examples, SecondMessage) {
  SecondMessage input, output;
  uut_type temp = nullptr;

  // Run some values through every member to validate
  // the behavior and integrity of the conversion.
  // IMPORTANT: some converter implementations use JsonGLIB under the hood,
  // which only stores int64 for any integer value, so the converter must
  // deal with static casting values back to the appropriate unsigned scaled
  // integer.  Therefore the uint64's MSB _MUST_ be 1 in this test to verify
  // the converter behavior.
  input.some_bool   = true;
  input.some_char   =  'A';
  input.some_string = "something";
  input.some_float  = 500.0F;
  input.some_double = 1200.1;
  input.some_uint8  = (uint8_t)  0x01;
  input.some_uint16 = (uint16_t) 0x2345;
  input.some_uint32 = (uint32_t) 0x6789ABCD;
  input.some_uint64 = (uint64_t) 0xEF0123456789ABCD; // important that MSB is 1!
  input.some_int8   = (int8_t)   0xFE;
  input.some_int16  = (int16_t)  0xDCBA;
  input.some_int32  = (int32_t)  0x98765432;
  input.some_int64  = (int64_t)  0x10FEDCBA98765432;

  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(input, output);

  uut_unref(temp);
}

/**
  * @brief The 'payload' member of SimpleMessage is registered, but the Payload 'member' is not.
  * Therefore when converting 'to' an intermediate type, the result should be an empty object
  * (e.g., JSON '{}').  When converting 'from', unregistered properties should be skipped/ignored.
  */
TEST(Examples, PropertyRegistrationBehavior) {
  SimpleMessage input, output;
  uut_type temp = nullptr;

  input.name = "clever name";
  input.payload.member = "something testy";

  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(temp != nullptr);

  EXPECT_TRUE(member_check_function(temp, "name"));
  EXPECT_TRUE(member_check_function(temp, "payload"));

  // Check the payload, that it's empty.
#if TEST_JSON_GLIB
  auto ref_obj = json_node_get_object(temp);
  auto payload_obj = json_object_get_object_member(ref_obj, "payload");
  EXPECT_EQ(0, json_object_get_size(payload_obj));

#elif TEST_SOCKET_IO
  auto ref_obj = temp->get_map();
  auto payload_obj = ref_obj["payload"]->get_map();
  EXPECT_EQ(0, payload_obj.size());
#endif

  uut_unref(temp);
}

TEST(Optionals, ToSkippedOnEmptyOrDefaulted) {
  /**
   * Verify that optional members are completely skipped in the 'to'
   * conversion process:
   *   Container types
   *     optional_string
   *     optional_vector
   *     optional_map
   *   Scalar types:
   *     optional_defaulted_uint64 (because it matches a default)
   *   Pointer types:
   *     optional_sptr
   *     optional_rawptr
   *
   * NOTE: There are other optional members which will not be skipped
   */
  OptionalMemberMessage input;
  uut_type temp = nullptr;
  std::vector<std::string> optional_names = {
    "optional_string",
    "optional_vector",
    "optional_map",
    "optional_defaulted_uint64",
    "optional_sptr",
    "optional_rawptr"
  };

  // This better work.
  EXPECT_NO_THROW(temp = to_conversion(input));

  // Verify the names are missing from 'temp', however that intermediate
  // type's API functions.
  for (auto const& name : optional_names) {
    EXPECT_FALSE(member_check_function(temp, name)) <<
      "Unexpected Property Name: " << name;
  }

  // Verify the required members are present despite being empty, nullptr, etc.
  // or because they are by-value and registered with no default to clarify when
  // the associated value is considered a optional/skippable.
  std::vector<std::string> required_names = {
    "required_string",
    "required_vector",
    "required_map",
    "required_sptr",
    "required_rawptr",
    "required_obj",
    "optional_uint64",
    "optional_obj"
  };

  for (auto const& name : required_names) {
    EXPECT_TRUE(member_check_function(temp, name)) <<
      "Missing Property Name: " << name;
  }

  uut_unref(temp);
}


TEST(Optionals, MissingRequiredWillFail) {
  /**
   * Verify that a missing non-optional field in the intermediate type
   * will cause a 'from' call to fail (return false).  In this case, the
   * class has a 'required_string' member, which will trigger the failure
   * when restored from an empty object.
   */
  OptionalMemberMessage output;
  uut_type temp = nullptr;

  // Verify that from fails if a required container member
  // is not given in the intermediate class.
#if TEST_JSON_GLIB
  temp = json_from_string("{}", NULL);
#elif TEST_SOCKET_IO
  temp = ::sio::object_message::create();
#endif
  EXPECT_FALSE(from_conversion(temp, output));
  uut_unref(temp);
}

TEST(Optionals, String) {
  OptionalMemberMessage input, output;
  uut_type temp = nullptr;

  input.optional_string = "is now set";
  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(member_check_function(temp, "optional_string"));
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(input, output);
  uut_unref(temp);
}

TEST(Optionals, Vector) {
  OptionalMemberMessage input, output;
  uut_type temp = nullptr;

  input.optional_vector.push_back(5);
  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(member_check_function(temp, "optional_vector"));
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(input, output);
  uut_unref(temp);
}

TEST(Optionals, Map) {
  OptionalMemberMessage input, output;
  uut_type temp = nullptr;

  input.optional_map["test_value"] = 42;
  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(member_check_function(temp, "optional_map"));
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(input, output);
  uut_unref(temp);
}

TEST(Optionals, SharedPointer) {
  OptionalMemberMessage input, output;
  uut_type temp = nullptr;

  input.optional_sptr = std::make_shared<OptionalMemberMessage::Payload>();
  input.optional_sptr->value = 53;
  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(member_check_function(temp, "optional_sptr"));
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(input, output);
  uut_unref(temp);
}

TEST(Optionals, RawPointer) {
  OptionalMemberMessage input, output;
  uut_type temp = nullptr;

  input.optional_rawptr = new OptionalMemberMessage::Payload();
  input.optional_rawptr->value = 87;
  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(member_check_function(temp, "optional_rawptr"));
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(input, output);
  uut_unref(temp);
}

TEST(Optionals, ObjectByValue) {
  OptionalMemberMessage input, output;
  uut_type temp = nullptr;

  input.optional_obj.value = 32;
  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(member_check_function(temp, "optional_obj"));
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(input, output);
  uut_unref(temp);
}

TEST(Optionals, ValueType) {
  OptionalMemberMessage input, output;
  uut_type temp = nullptr;

  input.optional_uint64 = 58;
  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(member_check_function(temp, "optional_uint64"));
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(input, output);
  uut_unref(temp);
}

TEST(Optionals, DefaultedValueType) {
  /**
   * The input and output objects are constructed with the default value already
   * set, so this test confirms that the 'to' conversion skipped the member and
   * the output object once again had the default value, by design.  Then on the
   * second pass of a test, we change the default value and confirm that it both
   * existed in the intermediate stage and the output as the new value.
   */
  OptionalMemberMessage input, output;
  uut_type temp = nullptr;

  EXPECT_EQ(input.optional_defaulted_uint64, OptionalMemberMessage::DEFAULT_U64_VALUE);
  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_FALSE(member_check_function(temp, "optional_defaulted_uint64"));
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(input, output);
  uut_unref(temp);

  // Now it won't match the default value, so it _should_ be in the intermediate step
  // and therefore should also result in the output being set.
  input.optional_defaulted_uint64 = 50 + OptionalMemberMessage::DEFAULT_U64_VALUE;
  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(member_check_function(temp, "optional_defaulted_uint64"));
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(input, output);
  uut_unref(temp);
}

TEST(Optionals, EmptyBecauseOptional) {
  /**
   * This test verifies that the "to" conversion will succeed even if all of the
   * object's properties are skipped for one reason or another (i.e., optional):
   */
  MaybeEmpty input;
  uut_type temp = nullptr;

  // Sanity check - non-default value was handled and a valid object was produced.
  input.value = MaybeEmpty::DEFAULT_VALUE + 1;
  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE((temp));
  EXPECT_TRUE(member_check_function(temp, "value"));
  uut_unref(temp);

  // When set to the default value, the object will effectively have no members
  // to insert into the JSON (it will be '{}'), but that should still be permitted
  // even though the result will be ambiguous on de-serialization.
  input.value = MaybeEmpty::DEFAULT_VALUE;
  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE((temp));
  EXPECT_FALSE(member_check_function(temp, "value"));
  uut_unref(temp);
}

TEST(StdAny, MapWithAny) {
  /**
   * The object has a parameter, 'properties' which is a std::map<std::string, std::any>.
   * This test validates that the library handle inferring what to insert for "any".
   */
  MessageWithAnys input, output;
  uut_type temp = nullptr;

  input.properties["int-valued"] = (int) 1234;
  input.properties["string-valued"] = std::string("something");

  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(temp);
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(input.properties.size(), output.properties.size());

  // NOTE: because of the way integers are handled in the intermediate
  // types, and the amorphous nature of 'any' in RTTR, we basically "give up"
  // on the "from" path here and the output type may have been promoted to
  // a higher bit width than the source.
  EXPECT_EQ(
    std::any_cast<int>(input.properties["int-valued"]),
    std::any_cast<int64_t>(output.properties["int-valued"])
  );
  EXPECT_EQ(
    std::any_cast<std::string>(input.properties["string-valued"]),
    std::any_cast<std::string>(output.properties["string-valued"])
  );

  uut_unref(temp);
}

TEST(Pointers, DestinationIsStdShared) {
  std::shared_ptr<SecondMessage> input, output;
  uut_type temp = nullptr;

  input = std::make_shared<SecondMessage>();
  input->some_bool = true;
  input->some_double = 42.0;
  input->some_char = 'X';

  // You must provide the converter with an output instance
  output = std::make_shared<SecondMessage>();

  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(temp);
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(*input, *output);

  uut_unref(temp);
}

TEST(Pointers, DestinationIsRawPtr) {
  SecondMessage* input, * output;
  uut_type temp = nullptr;

  input = new SecondMessage();
  input->some_float = 86.0f;
  input->some_string = "A String";
  input->some_uint8 = 0xB;

  // You must provide the converter with an output instance
  output = new SecondMessage();

  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(temp);
  EXPECT_TRUE(from_conversion(temp, output));
  ASSERT_TRUE(output);
  EXPECT_EQ(*input, *output);

  delete input;
  delete output;

  uut_unref(temp);
}

TEST(Vectors, VectorOfValues) {
  MessageWithVectors input, output;
  uut_type temp = nullptr;

  input.v_int = { 1, 2, 3 };

  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(temp);
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(input.v_int, output.v_int);

  uut_unref(temp);
}

TEST(Vectors, VectorOfVectorOfValues) {
  MessageWithVectors input, output;
  uut_type temp = nullptr;

  input.vv_int.push_back({ 1, 2, 3 });

  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(temp);
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(input.vv_int, output.vv_int);

  uut_unref(temp);
}

TEST(Vectors, VectorOfSharedPointers) {
  MessageWithVectors input, output;
  uut_type temp = nullptr;

  input.v_sptr.push_back(std::make_shared<SimpleMessage>());
  input.v_sptr[0]->name = "Some Name";

  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(temp);
  EXPECT_TRUE(from_conversion(temp, output));
  ASSERT_EQ(1, output.v_sptr.size());
  EXPECT_EQ(input.v_sptr[0]->name, output.v_sptr[0]->name);

  uut_unref(temp);
}

TEST(Vectors, VectorOfValueObjects) {
  MessageWithVectors input, output;
  uut_type temp = nullptr;

  input.v_obj.resize(1);
  input.v_obj[0].name = "Some Other Name";

  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(temp);
  EXPECT_TRUE(from_conversion(temp, output));
  ASSERT_EQ(1, output.v_obj.size());
  EXPECT_EQ(input.v_obj[0].name, output.v_obj[0].name);

  uut_unref(temp);
}

int main (int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
