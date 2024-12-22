server:
	g++ util.cpp InetAddress.cpp Socket.cpp client.cpp -o client && \
	g++ util.cpp InetAddress.cpp Socket.cpp Epoll.cpp server.cpp -o server
clean:
	rm server && rm client