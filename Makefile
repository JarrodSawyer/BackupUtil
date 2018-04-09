MAKE = make

CLIENT=./Client
SERVER=./Server

all : client server

client:
	$(MAKE) -C $(CLIENT)

server:
	$(MAKE) -C $(SERVER)

.PHONY: clean
clean:
	$(MAKE) -C $(CLIENT) clean
	$(MAKE) -C $(SERVER) clean
