# Tic-Tac-Toe-On-line
#Amani Islam - ai326
#Madhur Nutulapati - mn712


ttt (Client)
The ttt program will connect to the server (ttts), display the current state of the Tic-Tac-Toe grid to the player, receive and transmit moves made by the player, and report moves made by the other player.
The client program (ttt.c) should connect to the server and display the current state of the board to the player. It should then prompt the player to make a move and send this move to the server. The client program should also listen for incoming messages from the server, including updates to the board state and notifications of game outcomes.


1.Parse the command line arguments for the server's domain name and port number.DONE 
2. Create a socket and connect to the server.DONE 
3.Send a PLAY message with the player's name. WORK ON
4.Wait for a BEGN message from the server, indicating the player's role and the name of their opponent.
5.Display the current state of the game board to the player.
6.Wait for MOVE messages from the server and update the game board accordingly.
7.Prompt the player for their move and send a MOVE message to the server.
Repeat steps 6-7 until the game is over.
8.Display the outcome and reason for the game ending.
9.Close the connection and exit the program.



ttts (Server)
The ttts program will pair up players, choose who will go first, receive commands from the players, track the state of the Tic-Tac-Toe grid, ensure that invalid moves are rejected, and determine when the game has ended.
The server program (ttts.c) should listen for incoming connections and create a separate thread for each player. These threads will handle incoming messages from the players and send outgoing messages to the players. The server program should also keep track of the state of the game, including the positions of the qX and O markers on the board, whose turn it is, and whether the game has ended.



For the server (ttts.c), here are the basic steps:
1.Create a socket and bind it to a port number.DONE
2.Listen for incoming connections from clients.DONE
3.When a client connects, accept the connection and create a new thread to handle the client.DONE
4.In the client thread, wait for the PLAY message from the client, which includes the player's name.DONE
5.Add the player to a waiting list until another player connects.DONE
6.Once two players are waiting, randomly select one to be X and the other to be O.DONE


7.Send a BEGN message to each player, indicating their role and the name of their opponent. 
8.Wait for MOVE messages from the players and update the game board accordingly.
9.Check for win/draw conditions after each move and send the appropriate message to the players.
If a player resigns, send an RSGN message to the other player and end the game.
If both players agree to a draw, send a DRAW message and end the game.
Close the connection and thread when the game is over.









Protocol
The game protocol will involve nine message types that will be exchanged between the client (ttt) and server (ttts):

PLAY name: The client sends its name to the server to join the game.

WAIT: The server informs the client to wait for another player to join.

BEGN role name: The server notifies the client about the beginning of the game, assigns a role (X or O) to the client, and sends the name of the opponent player.

MOVE role position: The client sends its move (position) to the server.

MOVD role position board: The server notifies the client about the opponent's move (position) and updates the current state of the grid (board).

INVL reason: The server informs the client that its move was invalid and provides a reason for the rejection.

RSGN: The client sends a resignation request to the server.

DRAW message: The client sends a draw suggestion to the server.

OVER outcome reason: The server informs the clients about the outcome of the game (win, loss, or draw) and provides a reason for the end of the game.

Message Format
Messages will be broken into fields, where each field is separated by a vertical bar. The first field will always be a four-character code representing the message type. The second field will give the length of the remaining message in bytes, represented as a string containing a decimal integer in the range 0-255. Subsequent fields will be variable-length strings and will contain the relevant information for the message type. Messages will always end with a vertical bar, which will be used to detect improperly formatted messages.

12345 localhost

void startGame(Player* p1, Player* p2) {
    // randomly select X and O
    Player* x_player;
    Player* o_player;
    int r = rand() % 2;  // generate random number 0 or 1
    if (r == 0) {
        x_player = p1;
        o_player = p2;
    } else {
        x_player = p2;
        o_player = p1;
    }

    // send BEGN message to players
    char msg1[MSG_SIZE];
    snprintf(msg1, MSG_SIZE, "BEGN:X,%s", o_player->name);
    send_msg(x_player->sock, BEGN, msg1);
    char msg2[MSG_SIZE];
    snprintf(msg2, MSG_SIZE, "BEGN:O,%s", x_player->name);
    send_msg(o_player->sock, BEGN, msg2);
}
