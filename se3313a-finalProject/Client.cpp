
#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;

// This thread handles the connection to the server
class ClientThread : public Thread
{
private:
	// Reference to our connected socket
	Socket& socket;

	// Data to send to server
	ByteArray data;
	std::string data_str;
public:
	ClientThread(Socket& socket)
	: socket(socket)
	{}

	// Destructor (clears up memory) will destroy instance of the thread
	~ClientThread()
	{}

	virtual long ThreadMain()
	{
		int result = socket.Open();
		std::cout << "Please input data: ";
		std::cout.flush();

		// Get the data
		std::getline(std::cin, data_str);
		data = ByteArray(data_str);

		// Write to the server
		socket.Write(data);

		// Get the response
		socket.Read(data);
		data_str = data.ToString();
		std::cout << "Response: " << data_str << std::endl;
		return 0;
	}
};

int main(void)
{
	// Welcome the user 
	std::cout << "Welcome Client" << std::endl;

	// Create our socket
	Socket socket("127.0.0.1", 3000);
	// create a client thread that references the socket
	ClientThread clientThread(socket);
	while(1)
	{
		sleep(1);
	}
	socket.Close();

	return 0;
}
