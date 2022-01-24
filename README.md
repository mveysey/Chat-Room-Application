# Chat-Room-Application

**How do the various parts of the application work together?**

The chat room application implements socket programming using c++, python, multithreading,
and UI (user interface). Once the server is started through Oracle VM, clients can join the app
by running the python file. They are then given the option to join a chat room and send
messages to other clients in those chat rooms. The server is notified of all client interactions
with the application, including:

● When a client is waiting (just before they select a chat room)
● When a client changes the current chat room (when a chat room is selected/changed)
● When a client leaves the app (closes the popup window)

The server dictates how many chat rooms are available which can be modified by the
administrator and the client responds by indicating which chat room they are in. To help the
server understand which room a client is in, we added the chat room number with each client
message.

**How many threads are needed?**

The SocketServer uses one single thread as there is no need to split execution paths. However,
each time the server accepts a new connection, a socket thread is generated and stored by the
server within a vector, so this could be a virtually infinite number of threads in terms of client. By
using a vector we can rely on dynamic memory which can be safely deallocated upon
termination. The design uses the idea that each socket thread is related to a specific client,
allowing the server to distinguish between chat rooms.

**What sort of synchronization objects are used and how?**

The server uses Blockable class to allow threading to go on in the background for as long as it
needs. Since only one socket (created at the server) is used for all communication, semaphores
must be used to control the order of execution of all client activities. The server thread contains
the semaphore that is the owner of all socket thread semaphores. In order for clients in the
same chat room to see chat history and not to see messages from other rooms, the server only
allows Read() calls from clients who correspond to the given socket thread’s chat room number
(which we added by appending the chat room number to messages).

**How is termination handled?**
Upon termination in the server, each socket thread removes itself from the vector of client
threads by issuing a blocking call (Wait()) through their own semaphore and freeing themselves
from server memory. When the server abruptly stops or receives the input “shutdown” or “done”,
the server thread destructor loops through all references of created threads, destroying the
instance of those objects, and deleting the vector itself by performing a vector swap() with an
empty vector.
