# Every source file (and header file) should be listed here.
# If you add a new file, add it to this list.
target_sources(EndlessSkyTests PRIVATE
	src/helpers/datanode-factory.cpp
	src/test_conditionSet.cpp
	src/test_datanode.cpp
	src/test_main.cpp
	src/test_point.cpp
	src/test_random.cpp
	src/test_set.cpp
	src/test_ship.cpp
	src/test_template.txt
	src/text/test_alignment.cpp
	src/text/test_displaytext.cpp
	src/text/test_format.cpp
	src/text/test_layout.cpp
	src/text/test_truncate.cpp
	include/catch.hpp
	include/datanode-factory.h
	include/es-test.hpp
	include/output-capture.hpp
)

target_include_directories(EndlessSkyTests PRIVATE include)
