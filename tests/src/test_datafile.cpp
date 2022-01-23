/* test_datafile.cpp
Copyright (c) 2022 by quyykk

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "es-test.hpp"

// Include only the tested class's header.
#include "../../source/DataFile.h"

// Include a helper functions.
#include "datanode-factory.h"
#include "output-capture.hpp"
#include "../../source/text/Format.h"

// ... and any system includes needed for the test file.
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace { // test namespace
// #region mock data

// Insert file-local data here, e.g. classes, structs, or fixtures that will be useful
// to help test this class/method.

// #endregion mock data



// #region unit tests
SCENARIO( "Creating a DataFile", "[DataFile]") {
	GIVEN( "A default constructed DataFile" ) {
		const DataFile root;
		THEN( "it has the correct default properties" ) {
			CHECK( root.begin() == root.end() );
		}
	}
	GIVEN( "A DataFile created with a stream" ) {
		std::istringstream stream(R"(
node1
	something
node2 hi
	something else
)");
		const DataFile root(stream);

		THEN( "it has the correct properties" ) {
			REQUIRE( std::distance(root.begin(), root.end()) == 2);
			CHECK( root.begin()->Token(0) == "node1" );
			CHECK( std::next(root.begin())->Token(0) == "node2" );
		}
	}
}

SCENARIO( "Loading an ill-formed DataFile", "[DataFile]") {
	OutputSink sink(std::cerr);

	GIVEN( "A DataFile with missing quotes" ) {
		std::istringstream stream(R"(
"system Sol
	haze "some/haze

# The below is not a syntax error, but valid.
planet Earth"
)");
		const DataFile root(stream);

		THEN( "they are correctly identified" ) {
			const auto warnings = Format::Split(sink.Flush(), "\n");
			const std::string warning = "Warning: Closing quotation mark is missing:";

			REQUIRE( warnings.size() >= 5 );

			CHECK( warnings[0] == warning );
			CHECK( warnings[1].find("system Sol") != std::string::npos );
			CHECK( warnings[3] == warning );
			CHECK( warnings[4].find("system Sol") != std::string::npos );
			CHECK( warnings[5].find("haze some/haze") != std::string::npos );
		}
	}

	GIVEN( "A DataFile with mixed whitespace" ) {
		std::istringstream stream(R"(
node using
	# comment
	tabs like vanilla

space
 node
 # and a comment

# a comment
mixed node
 	foo
)");
		const DataFile root(stream);

		THEN( "they are correctly identified") {
			const auto warnings = Format::Split(sink.Flush(), "\n");
			const std::string commentWarning = "Warning: Mixed whitespace usage for comment at line";
			const std::string nodeWarning = "Warning: Mixed whitespace usage at line";

			REQUIRE( warnings.size() >= 7 );

			CHECK( warnings[0] == nodeWarning );
			CHECK( warnings[1].find("space") != std::string::npos );
			CHECK( warnings[2].find("node") != std::string::npos );
			CHECK( warnings[4] == commentWarning + " 8" );
			CHECK( warnings[5] == nodeWarning );
			CHECK( warnings[6].find("mixed node") != std::string::npos );
			CHECK( warnings[7].find("foo") != std::string::npos );
		}
	}
}
// #endregion unit tests



} // test namespace
