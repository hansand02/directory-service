# Project Name




### D1 Helper functions

#### `void check_error(int res, char *msg, int line, char* file)`
This function is designed to help with error handling in the program by checking the result of operations and printing error messages with line numbers and file names if the result indicates an error.

#### `void print_error_line(int line, char* file, char* message)`
Prints an error message with the file name and line number, enhancing the traceability of issues during debugging.

#### `void print_line(int line, const char *file, const char *message)`
Prints debugging information including line numbers and file names, used to track the flow and state of execution in debug mode. Only prints if the global variable `PRINT_DEBUG_INFO` is 1

#### `uint16_t calculate_checksum(char* newBuffer, int size)`
Calculates a checksum for given data, ignoring the bytes reserved for the checksum itself in the calculation.

#### `void reset_socket_options(D1Peer* peer)`
Resets the socket options to default settings, which is particularly useful for ensuring that timeouts or other settings do not persist beyond their intended scope (d1_wait_ack).

--- 
