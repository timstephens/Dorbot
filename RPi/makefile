dorbot_server: dorbot_server.cpp
	g++ -Wall -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -L./librf24/  -lrf24 -lssl -lcrypto dorbot_server.cpp -o dorbot_server `mysql_config --cflags --libs`


clean:
	rm dorbot_server

