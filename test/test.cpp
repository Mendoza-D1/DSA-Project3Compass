#include <catch2/catch_test_macros.hpp>
#include <iostream>

// change if you choose to use a different header name
#include "CampusCompass.h"

using namespace std;

// the syntax for defining a test is below. It is important for the name to be
// unique, but you can group multiple tests with [tags]. A test can have
// [multiple][tags] using that syntax.
TEST_CASE("Example Test Name - Change me!", "[tag]") {
  // instantiate any class members that you need to test here
  int one = 1;

  // anything that evaluates to false in a REQUIRE block will result in a
  // failing test
  REQUIRE(one == 1); // fix me!

  // all REQUIRE blocks must evaluate to true for the whole test to pass
  REQUIRE(true); // also fix me!
}

TEST_CASE("Test 2", "[tag]") {
  // you can also use "sections" to share setup code between tests, for example:
  int one = 1;

  SECTION("num is 2") {
    int num = one + 1;
    REQUIRE(num == 2);
  };

  SECTION("num is 3") {
    int num = one + 2;
    REQUIRE(num == 3);
  };

  // each section runs the setup code independently to ensure that they don't
  // affect each other
}

// Refer to Canvas for a list of required tests. 
// We encourage you to write more than required to ensure proper functionality, but only the ones on Canvas will be graded.

// See the following for an example of how to easily test your output.
// Note that while this works, I recommend also creating plenty of unit tests for particular functions within your code.
// This pattern should only be used for final, end-to-end testing.

// This uses C++ "raw strings" and assumes your CampusCompass outputs a string with
//   the same thing you print.
TEST_CASE("Direct Insert Function Test") {
  CampusCompass compass;
  REQUIRE(compass.ParseCSV("data/edges.csv", "data/classes.csv") == true);
  REQUIRE(compass.InsertStudent("Brandon", "45679999", 20, {"COP3530", "MAC2311"}) == true);

}


TEST_CASE("Direct Remove Function Test") {
  CampusCompass compass;
  REQUIRE(compass.ParseCSV("data/edges.csv", "data/classes.csv") == true);
  REQUIRE(compass.InsertStudent("Brandon", "45679999", 20, {"COP3530", "MAC2311"}) == true);
  REQUIRE(compass.RemoveStudent("45679999") == true);
}

TEST_CASE("Remove Nonexistent Student Fails") {
  CampusCompass compass;
  REQUIRE(compass.ParseCSV("data/edges.csv", "data/classes.csv") == true);
  REQUIRE(compass.RemoveStudent("45679999") == false);
}