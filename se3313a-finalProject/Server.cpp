#include "thread.h"
#include "socketserver.h"
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include "Semaphore.h"
#include <list>
#include <vector>
#include <thread>

// synchronization
using namespace Sync;


class SocketThread : public Thread
{
private:
    // connected socket.
    Socket &socket;

    // a byte array
    ByteArray data;

	// number of chat rooms.
	int noChatRoom;

	// port
	int port;

    // handles termination.
    bool& terminate;

    // a vector to store SocketThread pointers (implements multithreading)
    std::vector<SocketThread*> &socketThreadsHolder;

    
public:
	SocketThread(Socket& socket, std::vector<SocketThread*> &clientSockThr, bool &terminate, int port) :
		socket(socket), socketThreadsHolder(clientSockThr), terminate(terminate), port(port)
	{}

	// Destructor (clears up memory) will destroy instance of the thread
    ~SocketThread()
    {
		this->terminationEvent.Wait();
	}

    Socket& GetSocket()
    {
        return socket;
    }

    const int GetChatRoom()
    {
        return noChatRoom;
    }

    virtual long ThreadMain()
    {
		// string the value of the port
		std::string stringPort = std::to_string(port);

		// Semaphore generated on each socket thread by referencing port number as the name.
		Semaphore clientBlock(stringPort);


		try {
			// Attempt to gather bytestream data.
			socket.Read(data);

			std::string chatRoomString = data.ToString();
			chatRoomString = chatRoomString.substr(1, chatRoomString.size() - 1);
			noChatRoom = std::stoi(chatRoomString);
			std::cout << "Current chat room number from socket.Read(): " << noChatRoom << std::endl;

			while(!terminate) {
				int socketResult = socket.Read(data);
				// If the socket is closed on the client side, terminate this socket thread.
				if (socketResult == 0)	break;

				std::string recv = data.ToString();
				if(recv == "shutdown\n") {
					// ensure mutual exclusion 
					clientBlock.Wait();

					// Iterator method to select and erase this socket thread.
					socketThreadsHolder.erase(std::remove(socketThreadsHolder.begin(), socketThreadsHolder.end(), this), socketThreadsHolder.end());

					// Exit critical section
					clientBlock.Signal();

					std::cout<< "A client is leaving the server" << std::endl;
					break;
				}

				// Use a slash to indicate a change in chat room 
				if (recv[0] == '/') {
					// split the received string
					std::string stringChat = recv.substr(1, recv.size() - 1);
				
					// the number after the slash represents the chat room 
					noChatRoom = std::stoi(stringChat);
					std::cout << "A client just joined room " << noChatRoom << std::endl;
					continue;
				}

				// use semaphore blocking to enable thread to enter critical section
				clientBlock.Wait();
				for (int i = 0; i < socketThreadsHolder.size(); i++) {
					SocketThread *clientSocketThread = socketThreadsHolder[i];
					if (clientSocketThread->GetChatRoom() == noChatRoom)
					{
						Socket &clientSocket = clientSocketThread->GetSocket();
						ByteArray sendBa(recv);
						clientSocket.Write(sendBa);
					}
				}
				// leave critical section
				clientBlock.Signal();
			}
		} 
		catch(std::string &s) {
			std::cout << s << std::endl;
		}
		catch(std::exception &e){
			std::cout << "A client has encountered an error" << std::endl;
		}
		std::cout << "A client has left" << std::endl;
	}
};

class ServerThread : public Thread
{
private:
	// declare variables
    SocketServer &server;

    std::vector<SocketThread*> socketThrHolder;

	int port;

	int numberRooms;

	// determine if termination should occur
    bool terminate = false;
    
public:
    ServerThread(SocketServer& server, int numberRooms, int port)
    : server(server), numberRooms(numberRooms), port(port)
    {}

	// Destructor
    ~ServerThread()
    {
        // close client sockets
        for (auto thread : socketThrHolder)
        {
            try
            {
                // close the socket
                Socket& toClose = thread->GetSocket();
                toClose.Close();
            }
			// catch all exceptions
            catch (...)
            {
            }
        }
		std::vector<SocketThread*>().swap(socketThrHolder);
        terminate = true;
    }

    virtual long ThreadMain()
    { 
        while (true)
        {
            try {
				// convert port no to string
                std::string stringPortNum = std::to_string(port);
                std::cout << "blocking call on client" <<std::endl;

				// owner semaphore to block other semaphores by their name
                Semaphore serverBlock(stringPortNum, 1, true);

				// send number of chats through socket
                std::string allChats = std::to_string(numberRooms) + '\n';

				// byte array conversion for number of chats
                ByteArray allChats_conv(allChats); 


                // wait for a client socket connection
                Socket sock = server.Accept();

				// send number of chats
                sock.Write(allChats_conv);
                Socket* newConnection = new Socket(sock);

                // pass a reference to this pointer into a new socket thread
                Socket &socketReference = *newConnection;
                socketThrHolder.push_back(new SocketThread(socketReference, std::ref(socketThrHolder), terminate, port));
            }
            catch (std::string error)
            {
                std::cout << "ERROR: " << error << std::endl;
				// exit thread function
                return 1;
            }
			catch (TerminationException terminationException)
			{
				std::cout << "Server has shut down" << std::endl;
				return terminationException;
			}
        }
    }
};

int main(void) {
	// Beginning of server
	std::cout << "Server running" << std::endl;
    string input;

	int rooms = 10;

	std::cout << "Chat App Server" << std::endl 
		<<"Type 'done' to stop the server" << std::endl;
    
    // create server
    SocketServer server(3005);    
    
    // create thread to perform server operations
    ServerThread serverthread(server);
    ServerThread st(server, rooms, 3005);
    
	// wait for input to shutdown the server
	FlexWait cinWaiter(1, stdin);
	cinWaiter.Wait();
	std::cin.get();
    // Shut down and clean up the server
    server.Shutdown();

    std::cout << "Good-bye" << std::endl;
}
}