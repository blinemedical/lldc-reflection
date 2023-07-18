#include <gtest/gtest.h>

#include <tutorial.hpp>

#include <lldc-reflection/converters/json-glib.h>

namespace converters = lldc::reflection::converters;

/**
 * Insert GTests below per the tutorial.
 */

TEST(Sanity, DoesNothing) {
  EXPECT_TRUE(true);
}

// Standard c/c++ + GTest skeleton
int main (int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  // Our linked, shared library w/o the extension.
  // If you find serialization/deserialization is doing nothing, it's probably
  // because for some reason your shared library hasn't been loaded into the runtime
  // and therefore none of the RTTR static registration methods have been called.
  // If your library defines any methods (not constructors, etc.) that are called
  // by your application, then chances are you will not have to deal with this.
  auto lib = ::rttr::library("tutorial");
  if (lib.load())
    return RUN_ALL_TESTS();
  return -1;
}
