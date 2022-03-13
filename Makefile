build:
	mkdir -p build
	
objects: build
	mkdir -p build/objects

log:
	mkdir -p log

tests:
	mkdir -p tests

run_log_tests: log_tests 
	./tests/utils/log_tests

test_objects: tests
	mkdir -p tests/objects

log_tests: log_main.o log.o test_objects log
	mkdir -p tests/utils
	g++ tests/objects/utils/log_main.o build/objects/utils/log.o -o tests/utils/log_tests

log_main.o: testfiles/utils/log_main.cpp objects
	mkdir -p tests/objects/utils
	g++ -c testfiles/utils/log_main.cpp -o tests/objects/utils/log_main.o

log.o: app/utils/log.cpp objects
	mkdir -p build/objects/utils
	g++ -c app/utils/log.cpp -o build/objects/utils/log.o

clean:
	rm -rf build tests log