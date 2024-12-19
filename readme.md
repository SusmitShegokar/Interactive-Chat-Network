**code is tested with wsl on windows. it may required some minor changes to run

### How to Run the code :

1. firstly run the server code  

compile : gcc server.c -o server -luuid
run : ./server

then compile client
compile : gcc client.c -o client -luuid
 
then you can create max of 10 clients with below
Run  ./client


### Requirements : 
make sure UUID is supported to your system

### Hardcoded thing :
1. currently IP address is Hardcoded as  "127.0.0.1" and  as port "5566"
if want to use diffent have to change in client and port number in server also.

### details of program and how to access features
* note : some times user > prompt will not come have to press enter to get it else without it also everything works fine

/active
Description: Retrieve a list of active clients.
Usage: /active
Example: /active


/send <dest_uuid> <message>
Description: Send messages to other clients using their unique IDs.
Usage: /send <dest_uuid> <message>
Example: /send <uuid> Hello there!

/logout
Description: Request to exit the application.
Usage: /logout
Example: /logout

/chatbot login
Description: Avail the chatbot feature.
Usage: /chatbot login
Example: /chatbot login

/chatbot logout
Description: Disable the chatbot feature.
Usage: /chatbot logout
Example: /chatbot logout

/history <recipient_id>
Description: Retrieve the conversation history between the requesting client and the specified recipient.
Usage: /history <recipient_id>
Example: /history 987654321

/history_delete <recipient_id>
Description: Delete chats of the specified recipient from the requesting client's chat history.
Usage: /history_delete <recipient_id>
Example: /history_delete 987654321

/delete_all
Description: Delete the complete chat history of the requesting client.
Usage: /delete_all
Example: /delete_all

/chatbot_v2 login
Description: Avail the GPT-2 FAQ chatbot feature.
Usage: /chatbot_v2 login
Example: /chatbot_v2 login

/chatbot_v2 logout
Description: Disable the GPT-2 FAQ chatbot feature.
Usage: /chatbot_v2 logout
Example: /chatbot_v2 logout


### task details (all task are implemented in same file)
Task 1: Peer-to-Peer Chat
Clients connect to the server and receive a unique identifier upon successful connection.
Clients can view active clients, send messages to other clients, and request to exit the application.
The server manages client connections, message sharing, and error handling.

Task 2: FAQ Chatbot
Implements a FAQ chatbot feature with predefined questions and answers.
Clients can toggle the chatbot feature on/off using commands.
The server maintains the chatbot status for each client and responds to queries with appropriate messages.

Task 3: Chat History
Maintains a chat history for each client, storing previous messages and conversations on the server.
Clients can retrieve conversation history with specific recipients, delete chats of specified recipients, or delete their complete chat history.
Messages sent between clients are stored in the chat history for both sender and recipient.

Task 4: FAQ Chatbot (using GPT-2)
Implements a FAQ chatbot feature with predefined questions and answers.
Clients can toggle the chatbot feature on/off using commands.
The server maintains the chatbot status for each client and responds to queries with appropriate messages.

