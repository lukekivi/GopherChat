####### PRIMARY TARGETS #######
#-- App:                      #
#   * make run_server         #
#   * make run_client         #
#-- Tests:                    #
#   * make run_all_tests      #
#   * make run_log_tests      #
#   * make run_socket_tests   #
###############################

port_num = 9010
server_ip = 127.0.0.1

build:
	mkdir -p build
	mkdir -p build/server
	mkdir -p build/client
	mkdir -p build/objects
	mkdir -p build/objects/server
	mkdir -p build/objects/client
	mkdir -p build/objects/utils
	mkdir -p build/objects/utils/socket
	mkdir -p build/objects/utils/userInput
	
log:
	mkdir -p log

# RUN EXECUTABLES
run_server: server_main
	./build/server/server_main $(port_num)

run_client: client_main
	./build/client/client_main $(server_ip) $(port_num) 

# EXECUTABLES
server_main: server_main.o server.o socket_messenger.o log.o log
	g++ build/objects/server/server_main.o build/objects/server/server.o build/objects/utils/socket/socket_messenger.o build/objects/utils/log.o -o build/server/server_main

client_main: client_main.o client.o socket_messenger.o log.o log
	g++ -pthread build/objects/client/client_main.o build/objects/client/client.o build/objects/utils/socket/socket_messenger.o build/objects/utils/log.o -o build/client/client_main

# OBJECTS
server_main.o: app/server/server_main.cpp build
	g++ -c app/server/server_main.cpp -o build/objects/server/server_main.o

client_main.o: app/client/client_main.cpp build
	g++ -c app/client/client_main.cpp -o build/objects/client/client_main.o

log.o: app/utils/log.cpp build
	g++ -c app/utils/log.cpp -o build/objects/utils/log.o

server.o: app/server/server.cpp build
	g++ -c app/server/server.cpp -o build/objects/server/server.o

client.o: app/client/client.cpp build
	g++ -c app/client/client.cpp -o build/objects/client/client.o

socket_messenger.o: app/utils/socket/socket_messenger.cpp build
	g++ -c app/utils/socket/socket_messenger.cpp -o build/objects/utils/socket/socket_messenger.o

script_reader.o: app/utils/userInput/script_reader.cpp build
	g++ -c app/utils/userInput/script_reader.cpp -o build/objects/utils/userInput/script_reader.o

clean:
	rm -rf build tests log

# TESTS
tests:
	mkdir -p tests
	mkdir -p tests/utils
	mkdir -p tests/utils/socket
	mkdir -p tests/utils/userInput
	mkdir -p tests/objects
	mkdir -p tests/objects/utils
	mkdir -p tests/objects/utils/socket
	mkdir -p tests/objects/utils/userInput
	
run_all_tests: run_log_tests run_socket_tests run_user_input_tests

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

socket_tests: socket_main.o socket_messenger.o log.o tests
	g++ tests/objects/utils/socket/socket_main.o build/objects/utils/socket/socket_messenger.o build/objects/utils/log.o -o tests/utils/socket/socket_tests

socket_main.o: testfiles/utils/socket/socket_main.cpp tests
	g++ -c testfiles/utils/socket/socket_main.cpp -o tests/objects/utils/socket/socket_main.o


## USER INPUT TESTS
run_user_input_tests: user_input_tests tests
	./tests/utils/userInput/user_input_tests

user_input_tests: user_input_main.o script_reader.o log.o tests
	g++ tests/objects/utils/userInput/user_input_main.o build/objects/utils/userInput/script_reader.o build/objects/utils/log.o -o tests/utils/userInput/user_input_tests

user_input_main.o: testfiles/utils/userInput/user_input_main.cpp tests
	g++ -c testfiles/utils/userInput/user_input_main.cpp -o tests/objects/utils/userInput/user_input_main.o