#### PRIMARY TARGETS ####
## Tests
#   * make run_all_tests
#   * make run_log_tests
#   * make run_socket_tests

port_num = 9000

build:
	mkdir -p build
	mkdir -p build/server
	mkdir -p build/client
	mkdir -p build/objects
	mkdir -p build/objects/server
	mkdir -p build/objects/client
	mkdir -p build/objects/utils
	mkdir -p build/objects/utils/socket
	
log:
	mkdir -p log

# RUN EXECUTABLES
run_server: server_main
	./build/server/server_main port_num

# EXECUTABLES
server_main: server_main.o server.o nonblocking_socket_messenger.o socket_messenger.o log.o log
	g++ build/objects/server/server_main.o build/objects/server/server.o build/objects/utils/socket/nonblocking_socket_messenger.o build/objects/utils/socket/socket_messenger.o build/objects/utils/log.o -o build/server/server_main

# OBJECTS
server_main.o: app/server/server_main.cpp build
	g++ -c app/server/server_main.cpp -o build/objects/server/server_main.o

log.o: app/utils/log.cpp build
	g++ -c app/utils/log.cpp -o build/objects/utils/log.o

server.o: app/server/server.cpp build
	g++ -c app/server/server.cpp -o build/objects/server/server.o

nonblocking_socket_messenger.o: app/utils/socket/socket_messenger.cpp build
	g++ -c app/utils/socket/nonblocking_socket_messenger.cpp -o build/objects/utils/socket/nonblocking_socket_messenger.o

socket_messenger.o: app/utils/socket/socket_messenger.cpp build
	g++ -c app/utils/socket/socket_messenger.cpp -o build/objects/utils/socket/socket_messenger.o

clean:
	rm -rf build tests log

# TESTS
tests:
	mkdir -p tests
	mkdir -p tests/utils
	mkdir -p tests/utils/socket
	mkdir -p tests/objects
	mkdir -p tests/objects/utils
	mkdir -p tests/objects/utils/socket
	
run_all_tests: run_log_tests run_socket_tests

## LOG TESTS
run_log_tests: log_tests 
	./tests/utils/log_tests

log_tests: log_main.o log.o tests log
	g++ tests/objects/utils/log_main.o build/objects/utils/log.o -o tests/utils/log_tests

log_main.o: testfiles/utils/log_main.cpp tests
	g++ -c testfiles/utils/log_main.cpp -o tests/objects/utils/log_main.o

## SOCKET MESSENGER TESTS
run_socket_tests: socket_tests
	./tests/utils/socket/socket_tests

socket_tests: socket_main.o socket_messenger.o tests
	g++ tests/objects/utils/socket/socket_main.o build/objects/utils/socket/socket_messenger.o -o tests/utils/socket/socket_tests

socket_main.o: testfiles/utils/socket/socket_main.cpp tests
	mkdir -p tests/objects/utils/socket
	g++ -c testfiles/utils/socket/socket_main.cpp -o tests/objects/utils/socket/socket_main.o