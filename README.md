// Remember to change the REPOSITORY_DIR to your own specific path to the Repository directory, in the server code.
// Do the same with SAVE_PATH for the client code (This is where the files are transferred to).

// Use the WSL Terminal with David.cpp -o server and then use Edward.cpp -o client
// Make sure you split the terminal so that you can see both the server and client
// After that you FIRST do ./server (remember this MUST come first) on one terminal and then follow with ./client 12345 (12345 is the port number) with the other terminal

// When you run ./client 12345 the client should pop with message "Please enter the command (get filename/exit/terminate):" and then everything should work from there
// For the "get filename" you will need to put the file name in the command. For example "Please enter the command (get filename/exit/terminate): get input10.txt"

// Also after you use the terminate command, you'll need to re-run ./server FIRST again, following with ./client 12345
