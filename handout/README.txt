I have also made the readme in nicely formatted MD, if you have the option to view that.

 ___  ________     _______   _____  ___   ___  ________     
|\  \|\   ___  \  /  ___  \ / __  \|\  \ |\  \|\   __  \    
\ \  \ \  \\ \  \/__/|_/  /|\/_|\  \ \  \\_\  \ \  \|\  \   
 \ \  \ \  \\ \  \__|//  / ||/ \ \  \ \______  \ \  \\\  \  
  \ \  \ \  \\ \  \  /  /_/__   \ \  \|_____|\  \ \  \\\  \ 
   \ \__\ \__\\ \__\|\________\  \ \__\     \ \__\ \_______\
    \|__|\|__| \|__| \|_______|   \|__|      \|__|\|_______|
                                                            
                                                                                        

## Implementation
------------------------------------------------------------------------

All aspects of the exam works as intended. All d1 and d2 functions have
the required functionality, and everything is freem from errors listed
by -Wextra and -Wall. There are also no memory leaks, and no valgrind
errors. What i just stated is also true in the case of user inputing
unvalid port, peername, or lookup id, and other possible errors, which
are handled with memory leaks in mind.

## Core functions
------------------------------------------------------------------------

All the core functions are made by the specifications give in the header
files. I will not describe each of these functions here, as i have added
documentation to all of them in `d1_udp.c` and `d2_udp.c` describing my
implementation, and what you need to now.

## Important
------------------------------------------------------------------------

I was unsure about wether you wanted to have the extended debug print
info or not. In `d1_udp.c` and `d2_udp.c` there is a defined value
PRINT_DEBUG_INFO atht the top. Set this to 1 if you want extra debug
print info.

## Helper functions
------------------------------------------------------------------------

#### `void check_error(_d1/_d2)(int res, char *msg, int line, char* file)`

This function is designed to help with error handling in the program by
checking the result of operations and printing error messages with line
numbers and file names if the result indicates an error.

#### `void print_error_line(_d1/_d2)(int line, char* file, char* message)`

Prints an error message with the file name and line number, enhancing
the traceability of issues during debugging.

#### `void print_line(_d1/_d2)(int line, const char *file, const char *message)`

Prints debugging information including line numbers and file names, used
to track the flow and state of execution in debug mode. Only prints if
the global variable `PRINT_DEBUG_INFO` is 1, standard value for this is
0.

#### `uint16_t calculate_checksum(char* newBuffer, int size)`

Calculates a checksum for given data, ignoring the bytes reserved for
the checksum itself in the calculation. (Very specific calculation)

#### `void reset_socket_options(D1Peer* peer)`

Resets the socket options to default settings, which is used for
ensuring that timeouts or other settings do not persist beyond their
intended scope (d1_wait_ack).

#### `void display_node(LocalTreeStore *store, int index, int level)`

Displays a node from the LocalTreeStore based on the specified index.
Each node's indent level, id, value, and number of children are printed
to visually represent the node's position and hierarchy within the tree.
The function uses recursion, and is, i believe, a depth first search.

------------------------------------------------------------------------

## Changes and assumptions
------------------------------------------------------------------------

#### `struct LocalTreeStore*`

In coherence with "Change the LocalTreeStore to suit your need.", i
changed this to have a struct NetNode\* root; field. To hold the root
NetNode.

#### Note on deletion and error handling

Given that there are given test files, i have not used
d1/d2_delete_client(), where the test files handled this. The program is
also not using Signals to make it safe from ctrl + c, since the test
file does not accomodate for this. But all runtime errors should be
handled. This is also the reason that check_error() does not terminate
the program, as it should be handled at a higher level, and this is just
for info.

#### Correct input

The program assumes that the input given is of correct types. E.g., will
we only check if the lookup id is larger than 1000, not if it is of type
boolean or string or etc.

#### Recursion

The program also uses recursion and some infinite while loops. Which
could go on forever if the server has some really unexpected behaviour,
like not sending anything. The printing of the tree also relies on there
not being any loops in the nodes, but this should not be possible either
i guess.

------------------------------------------------------------------------
