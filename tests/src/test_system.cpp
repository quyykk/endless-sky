#include "es-test.hpp"

// Include only the tested class's header.
#include "../../source/System.h"

#include "datanode-factory.h"
#include "../../source/GameData.h"
#include "../../source/GameObjects.h"
#include "../../source/Government.h"

// ... and any system includes needed for the test file.
#include <map>
#include <string>



namespace { // test namespace

// #region mock data
void InitializeInitialSystem(System &system, GameObjects &objects)
{
	system.Load(AsDataNode(R"(
system TestSystem
	hidden
	pos 776 123.134
	government TestGov
	attributes fancy bland
	music random/music
	arrival 456
	habitable 3333
	belt 99999
	"jump range" 12
	haze some/haze
	link One
	link Two
	object
		sprite black/hole
		distance 765
		period 3
)"), objects.planets);
}
// #endregion mock data



// #region unit tests
SCENARIO( "Creating a System" , "[System][Creation]" ) {
	System system;
	GameObjects objects;
	GameData::SetObjects(objects);
	GIVEN( "When created" ) {
		THEN( "it has the correct default properties" ){
			CHECK_FALSE( system.IsValid() );
			CHECK( system.Name().empty() );
			CHECK_FALSE( system.Position() );
			CHECK( system.GetGovernment()->GetTrueName().empty() );
			CHECK( system.MusicName().empty() );
			CHECK( system.Attributes().empty() );
			CHECK( system.Links().empty() );
			CHECK( system.JumpNeighbors(System::DEFAULT_NEIGHBOR_DISTANCE).empty() );
			CHECK_FALSE( system.Hidden() );
			CHECK( system.Objects().empty() );
			CHECK( system.Asteroids().empty() );
			CHECK_FALSE( system.Haze() );
			CHECK( system.HabitableZone() == Approx(1000.) );
			CHECK( system.AsteroidBelt() == Approx(1500.) );
			CHECK_FALSE( system.JumpRange() );
			CHECK_FALSE( system.SolarPower() );
			CHECK_FALSE( system.SolarWind() );
			CHECK_FALSE( system.ExtraHyperArrivalDistance() );
			CHECK_FALSE( system.ExtraJumpArrivalDistance() );
		}
	}
	AND_GIVEN( "When loading a system from a DataNode" ) {
		InitializeInitialSystem(system, objects);
		THEN( "it has the correct properties" ) {
			CHECK( system.IsValid() );
			CHECK( system.Name() == "TestSystem" );
			CHECK( system.Position().X() == Approx(776.) );
			CHECK( system.Position().Y() == Approx(123.134) );
			CHECK( system.GetGovernment() == objects.governments.Get("TestGov") );
			CHECK( system.MusicName() == "random/music" );
			CHECK( system.Attributes().size() == 2 );
			CHECK( system.Attributes().count("fancy") );
			CHECK( system.Attributes().count("bland") );
			CHECK( system.Links().size() == 2 );
			CHECK( system.Links().count(objects.systems.Get("One")) );
			CHECK( system.Links().count(objects.systems.Get("Two")) );
			CHECK( system.JumpNeighbors(System::DEFAULT_NEIGHBOR_DISTANCE).empty() );
			CHECK( system.Hidden() );
			REQUIRE( system.Objects().size() == 1 );
			CHECK( system.Objects()[0].Parent() == -1 );
			CHECK( system.Objects()[0].GetSprite()->Name() == "black/hole" );
			CHECK( system.Objects()[0].Distance() == Approx(765.) );
			CHECK( system.Asteroids().empty() );
			CHECK( system.Haze()->Name() == "some/haze" );
			CHECK( system.HabitableZone() == Approx(3333.) );
			CHECK( system.AsteroidBelt() == Approx(99999.) );
			CHECK( system.JumpRange() == Approx(12.) );
			CHECK_FALSE( system.SolarPower() );
			CHECK_FALSE( system.SolarWind() );
			CHECK( system.ExtraHyperArrivalDistance() == Approx(456.) );
			CHECK( system.ExtraJumpArrivalDistance() == Approx(456.) );
		}
	}
	AND_GIVEN( "When loading the same system again from a DataNode" ) {
		InitializeInitialSystem(system, objects);
		system.Load(AsDataNode(R"(
system TestSystem
	remove hidden
	add attributes ggg
	remove music
	asteroids "small rock" 45 1000
	habitable 8765
	haze some/other/haze
	remove link One
	add link Three
	object
		sprite black/hole
		distance 765
		period 3
		object
			sprite moon
			distance 10
			period 1000
)"), objects.planets);

		THEN( "it has the correct properties" ) {
			CHECK( system.IsValid() );
			CHECK( system.Name() == "TestSystem" );
			CHECK( system.Position().X() == Approx(776.) );
			CHECK( system.Position().Y() == Approx(123.134) );
			CHECK( system.GetGovernment() == objects.governments.Get("TestGov") );
			CHECK( system.MusicName().empty() );
			CHECK( system.Attributes().size() == 3 );
			CHECK( system.Attributes().count("fancy") );
			CHECK( system.Attributes().count("bland") );
			CHECK( system.Attributes().count("ggg") );
			CHECK( system.Links().size() == 2 );
			CHECK( system.Links().count(objects.systems.Get("Two")) );
			CHECK( system.Links().count(objects.systems.Get("Three")) );
			CHECK( system.JumpNeighbors(System::DEFAULT_NEIGHBOR_DISTANCE).empty() );
			CHECK_FALSE( system.Hidden() );
			REQUIRE( system.Objects().size() == 2 );
			CHECK( system.Objects()[0].Parent() == -1 );
			CHECK( system.Objects()[0].GetSprite()->Name() == "black/hole" );
			CHECK( system.Objects()[0].Distance() == Approx(765.) );
			CHECK( system.Objects()[1].Parent() == 0 );
			CHECK( system.Objects()[1].GetSprite()->Name() == "moon" );
			CHECK( system.Objects()[1].Distance() == Approx(10.) );
			REQUIRE( system.Asteroids().size() == 1 );
			CHECK( system.Asteroids()[0].Name() == "small rock" );
			CHECK_FALSE( system.Asteroids()[0].Type() );
			CHECK( system.Asteroids()[0].Count() == 45 );
			CHECK( system.Asteroids()[0].Energy() == 1000. );
			CHECK( system.Haze()->Name() == "some/other/haze" );
			CHECK( system.HabitableZone() == Approx(8765.) );
			CHECK( system.AsteroidBelt() == Approx(99999.) );
			CHECK( system.JumpRange() == Approx(12.) );
			CHECK_FALSE( system.SolarPower() );
			CHECK_FALSE( system.SolarWind() );
			CHECK( system.ExtraHyperArrivalDistance() == Approx(456.) );
			CHECK( system.ExtraJumpArrivalDistance() == Approx(456.) );
		}
	}
}
// #endregion unit tests



} // test namespace
