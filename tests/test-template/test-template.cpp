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

TEST(unit, SubjectGuard) {
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
TEST(unit, FailedConversion) {
  FirstMessage uut_first;
  SecondMessage uut_second;
  uut_type converted_first;

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
TEST(unit, CanInspect) {
  ApiMessage tester;
  FirstMessage first;
  SecondMessage second;
  SecondMessage input;
  uut_type temp;

  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(from_conversion(temp, tester));
  EXPECT_EQ(tester.GetSubject(), input.GetSubject());
  EXPECT_FALSE(from_conversion(temp, first));
  EXPECT_TRUE(from_conversion(temp, second));

  uut_unref(temp);
}

TEST(unit, ApiMessage) {
  /**
   * @brief Verify the ApiMessage can be converted to sio and back
   * again.  Setting the subject as 'first_message' will verify that
   * when converted back from the SocketIO Message type that the
   * value matches the input.
   */
  ApiMessage uut (Subject::first_message);
  ApiMessage out;
  uut_type converted_uut;

  EXPECT_NO_THROW(converted_uut = to_conversion(uut));
  EXPECT_TRUE(from_conversion(converted_uut, out));
  EXPECT_EQ(uut.GetSubject(), out.GetSubject());

  uut_unref(converted_uut);
}

TEST(unit, FirstMessage) {
  FirstMessage input, output;
  uut_type temp;

  input.body.data["some_key"] = "some_value";
  EXPECT_NO_THROW(temp = to_conversion(input));
  EXPECT_TRUE(from_conversion(temp, output));
  EXPECT_EQ(input, output);

  uut_unref(temp);
}

TEST(unit, SecondMessage) {
  SecondMessage input, output;
  uut_type temp;

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

int main (int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
