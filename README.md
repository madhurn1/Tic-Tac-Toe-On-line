# Tic-Tac-Toe-On-line
#Amani Islam - ai326
#Madhur Nutulapati - mn712

ttts(server)
    Gameplan: - Our ttts program will pair up players - choose who will go first and receive commands from the players - track the state of the grid - ensure that invalid moves are rejected - determine game conclusion. 
    Socket Connection - Listening for incoming connection and create threads for each player (pthread.h). The threads will essentially handle incoming messages from players and send outgoing messages to the players. The server will keep track of the state of the game: The X or O markers of each player. The turn of each player and game conclusion. 

main():
    Obtain the port number through the file arguments. This will help us with the connection requests between server and client. Next our install_handler function will take in a variable,mask, and will install signal handlers for our SIGINT and sigterm signals and adds them to mask. This function allows the program to handle these signals in a custom way, instead of relying on default behavior. Next, we are going to instantiate a struct addrinfo that holds all address information and initialize its hints. The hints will be set up for our later getaddrinfo() to be used to retrieve network address and service translation functions. Then, we will obtain information for listenting socket using getaddrinfo(). Following that, we will attempt to create a socket using the socket() and we will get a sock file descripter as a result. Then, we will bind the socket to the network address specified 'info' so that the socket can listen for incoming connections at the address. After binding, we will listen() which sets the socket to listen for incoming connection requests. And queue size is the maximum number of pending connnection requests that can be queued before the server starts rejecting new connections. Next, we will free the memory allocated by getaddrinfo for the linked list structures. In a while loop we will continously run while active is true allowing the server to keep listening for incoming connections until it is time to shut down. 
    
    Inside the loop - we create and allocate memory for a struct which allows the server to keep listening for incoming connections until termination. Next, we use the accept() to wait for client to connect to the server. Once connection is successful it returns a socket FD and more address info. Next, we use pthread_create() which takes our clientHandle function, and a pointer to the con->fd file descripter as arguments. The clienthandle will be responsible for handeling all communication with client. Then, we call pthread_detach() which will essentially clean up its own resources when terminated. Then our signal handlers we call pthread_sigmask () to block signals that the server doesn't want to handle when it is handling a client connection. This process will be continuous until termination. Then we will close the socket.

    signal_handler() - cleanly shuts down the server by closing all file descriptors and freeing all malloced con structs. It activates when ttts is closed using control+c.
    
clientHandle() - 
    Create a linked list of players to create a waiting list type result to keep track of the players coming in and currently in a game. 
    First we are passed in the socket file descriptor of the connected client. Create a few arrays to hold player names, roles, and their sock file descriptors. Next thing we will be doing is waiting for "PLAY" message from client. For this we have a special function p_recv() that checks to see if we did indeed recieve a message from the client and close if we have or haven't. Then I check to see if the name the user enters is already in use or not by traversing through the linked list and searching for the name. Then finally after the error handeling checks I call addPlayer() which adds the player to the linked list. Then, after calling the function I check my list to see if I have two players ready if not it sleeps for 1 second until there are 2 players are ready. Final step before starting the game I create structs for each player and copy the names to each of its properties(role,name). 

    Game is Now Ready... 
    First, we will format and write the BEGN message to each player indicating their role and opponent's name. 
    
    We start with a while loop that first checks if there is any data to be recieved from the current socket(p_recv). Then we check if the recieved code is move. If it is 'MOVE' we then update the board with the move that was entered in the message. We consequently update the board using our updateBoard (). Next we check for 'INVL' message, if it is we write using the client socket the invalid message. Next, we construct a formatted message to send both the grid update. We then check our game ending conditions using gameEnd() to see if the game has ended and how it has ended (win, loss, draw). Send a formatted message if game is ongoing and we are trying to communicate move has been updated or accounted for. Then, we check if the received code is "RSGN" (resign), if it is we then construct a message to send to both players indicating that the first player has resigned. Next, on to draw command we will ask the player options S, R, A. Then we will write the over message. 

    addPlayer() - create a linked list data structure and add players to a list. This will serve as a waiting list to queue up the players who want to play. 

    p_recv() - is essentially responsible for recieving message from socket file descriptor and parsing into msg_t struct. Then the recieved message is read into msg_t structure's buf using the read function.  

    gameEnd() - a function that will check for all sorts of game ending situations. Diagonals, columns, rows, and lastly a draw. 

    updateBoard - will take in the current grid, player's role (X OR O), x and y coordinates. This will update and print the grid each time as a move is confirmed. 

    checkName() - function that will traverse through the linked list of players to see if the name entered has already been entered.
 
    switchsock() - switch the current socket which we are reading from or writing to 

Our Testplan: 
- We designed various testcases that test from all ranges and conditions. We tested all edge cases and all areas the program can potentially face with client prompt. 

Here are the ways we tested: 
-diagonal
-horizontal 
-vertical
-Resigning option
-draw reject/accept
-draw when board fills up
- Invalid inputs/Invalid message formats
- 2 Unique Multithreading prompts 

ALL OF THESE TEST CASES ARE SUCCESSFUL

ttt (Client)
    The ttt program will connect to the server (ttts)
    Interacts with the user through standard input, receiving data such as the user's name and game commands
    Displays the current state of the Tic-Tac-Toe grid after every move
    Reads messages from the server and transmits messages to the server