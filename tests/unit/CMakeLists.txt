# Every source file (and header file) should be listed here.
# If you add a new file, add it to this list.
target_sources(EndlessSkyTests PRIVATE
	include/catch.hpp
	include/datanode-factory.h
	include/es-test.hpp
	include/output-capture.hpp
	src/comparators/test_byGivenOrder.cpp
	src/comparators/test_byName.cpp
	src/helpers/datanode-factory.cpp
	src/test_account.cpp
	src/test_angle.cpp
	src/test_bitset.cpp
	src/test_conditionSet.cpp
	src/test_datafile.cpp
	src/test_datanode.cpp
	src/test_esuuid.cpp
	src/test_exclusiveItem.cpp
	src/test_firecommand.cpp
	src/test_main.cpp
	src/test_point.cpp
	src/test_random.cpp
	src/test_set.cpp
	src/test_ship.cpp
	src/test_template.txt
	src/test_weightedList.cpp
)

target_include_directories(EndlessSkyTests PRIVATE include)
