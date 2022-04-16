# GopherChat
**Created by:** \
Lucas Kivi - kivix019 

## Setup
* Open up the app. If you need a new version, it is available via git:
	```
    git clone https://github.com/lukekivi/GopherChat.git
	```
* Build the client executable and server executable with by navigating to the base directory and running:
	```
    make
    ```
## Running The Server
I did all of my testing using make targets. The project description asks for executeables. Both are possible. I will detail both options.
___
#### Your Way
Generated executeables are directed to a `build` directory, so the syntax is a little different than the project description execution. 

**In `GopherChat` directory**
```
./build/server/server_main <server-port-num>
```

**Resetting the Registration Database**
```
./build/server/server_main reset
```
___
#### My Way
Modify the top two values in `Makefile` to match your machine and the port number you will use. That way we have a single source of truth for servers and clients alike.
```
port_num = 9013
server_ip = 54.172.118.127
```
Run command:
```
make run_server
```

## Running the Clients
Again there is your way and my way. Be sure you ran `make` in the base directory prior to attempting to execute a client.
___
#### Your Way

**In `GopherChat` directory**
```
./build/client/client_main <server-ip-address> <server-port-num> <command-file>
```
___
#### My Way

Adjust the top 3 entries in the `Makefile` to point to the server, the port the server is using, and the command file you are using. Here is an example setup
```
port_num = 9013
server_ip = 54.172.118.127
command_file = "commands.txt"
``` 

Then you can just run this command:
```
make run_client
```