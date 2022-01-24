from socket import AF_INET, socket, SOCK_STREAM
from threading import Thread
import tkinter

# Thread the function
def receive():
    while True:
        try:
            # receives messages
            msg = client_socket.recv(BUFFER_SIZE).decode("utf8")
            msg_list.insert(tkinter.END, msg)
            msg_list.see(tkinter.END)
        except OSError:
            break

# Use binders to pass the event
def send(event=None):  
    msg = my_msg.get()
    # Clear text field for each message
    my_msg.set("")      
    global current_room
    # If client quits the window (presses 'x'), then the user will leave the messenger app (the socket will close)
    if msg == "{quit}":
        client_socket.send(bytes(my_username.get() + " has closed the chat room!", "utf8"))
        client_socket.close()
        top.quit()
        return
    client_socket.send(bytes(my_username.get() + ": " + msg, "utf8"))


# If the client quits, then notify the server
def on_closing(event=None):
    my_msg.set("{quit}")
    send()

# Client can switch chat rooms
def change_room():
    global current_room
    # get value of current room
    current_room = ((chatRoomSelected.get()).split(' '))[2]
    client_socket.send(bytes("/" + current_room, "utf8"))
    # remove current room
    msg_list.delete(0, tkinter.END)
    # change current room
    msg_list.insert(tkinter.END, "You are now in room " + str(current_room))
    msg_list.see(tkinter.END)


number_of_rooms = 0
current_room = 0

# Set title of app
top = tkinter.Tk()
top.title("Group 26 SE3313 Chat Room App 2021")

messages_frame = tkinter.Frame(top)
# Allow the messages to be sent
my_msg = tkinter.StringVar()  
my_msg.set("")
# Allow input of username
my_username = tkinter.StringVar()
my_username.set("")

# Socket with Oracle VM Server
HOST = "192.168.2.97"
PORT = 3005
BUFFER_SIZE = 1024
ADDR = (HOST, PORT)

client_socket = socket(AF_INET, SOCK_STREAM)
client_socket.connect(ADDR)

# Create input for username
username_label = tkinter.Label(top, text="Username: ", foreground="blue")
username_label.pack()
username_field = tkinter.Entry(top, textvariable=my_username, foreground="blue")
username_field.pack()

# Create input for message / send message
message_label = tkinter.Label(top, text="Message (Press 'enter' key to send): ", foreground="blue")
message_label.pack()
entry_field = tkinter.Entry(top, textvariable=my_msg, width=80, foreground="blue")
entry_field.bind("<Return>", send)
entry_field.pack()


# Log of chat history
scrollbar = tkinter.Scrollbar(messages_frame)
msg_list = tkinter.Listbox(messages_frame, height=15, width=50, yscrollcommand=scrollbar.set, foreground="blue")
scrollbar.pack(side=tkinter.RIGHT, fill=tkinter.Y)
msg_list.pack(side=tkinter.LEFT, fill=tkinter.BOTH)
msg_list.pack()
messages_frame.pack()

top.protocol("WM_DELETE_WINDOW", on_closing)

# Server response of number of rooms available and generate drop down list.
first_msg = client_socket.recv(BUFFER_SIZE).decode("utf8")
number_of_rooms = int(first_msg)
chatRoomSelected = tkinter.StringVar(top)
chatRoomSelected.set("Available Chat Rooms")
rooms_list = []
for i in range(number_of_rooms):
    rooms_list.append("Chat Room " + str(i + 1))

# Allow client to select room
chat_rooms = tkinter.OptionMenu(top, chatRoomSelected, *rooms_list)
chat_rooms.pack()
change_button = tkinter.Button(top, text="Confirm Selection", command=change_room)
change_button.pack()

# threading
receive_thread = Thread(target=receive)
receive_thread.start()
top.resizable(width=True, height=True) 
# start GUI application
tkinter.mainloop()
