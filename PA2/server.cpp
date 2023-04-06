#include "huffmanTree.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <vector>
#include <map>
#include <pthread.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
using namespace std;

void fireman(int)
{
    while(waitpid(-1, NULL, WNOHANG ) > 0)
        cout << " A child process ended" << endl;
}
int parseInt(string s) // character converter, convert character of each string to int value, one way to make data transfering works
{
    int intValue = 0;
    for(int i=0; i<s.length(); i++)
    {
        intValue = intValue * 10 + s[i] - '0';
    }
    return intValue;
}

bool isValid(char c)
{
    return c == '0' || c == '1';
}

void processSocket(int socket, HuffmanTree tree) // read from socket function
{
    char socket_buffer[1024];
    read(socket, socket_buffer, 1024);

    char origin_buffer[1024];
    int pos;
    for(pos=0; pos<strlen(socket_buffer); pos++)
    {
        if(isValid(socket_buffer[pos])) // used to check for tree node position, then copy only the valid ones to new buffer data
            origin_buffer[pos] = socket_buffer[pos];
        else
            break;
    }

    origin_buffer[pos] = '\0'; // terminate string 
    string binary(origin_buffer);// converted to string so it can be used in Binary to Symbols function
    char decode = BinToSymbols(tree.getRoot(), binary,0);

    char* data_transfer = new char[3]{decode, '\n', '\0'};
    send(socket, data_transfer, strlen(data_transfer), 0); // send data
    close(socket);
}

int openSocketServer(int& server_fd, struct sockaddr_in& address, int &opt, int& addrlen, int port) // code stole from Mr.Rincon
{
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        return -1;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
        return -1;

    if (listen(server_fd, 3) < 0)
        return -1;
    return 0;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cout << "Valid command: ./file_server <port>" << endl;
        return 0;
    }


    char symbol;
    int freq;
    int totalFreq; // reusing stuff from PA1
    string symbolString;
    string freqString;

    priority_queue<TreeNode*, vector<TreeNode*>, comparator> pq;
    map<int,char> originalMessage; // declare map of the position and the letter that corresponds to that position

    while (getline(cin, symbolString)) //read each line, create nodes for symbol, assign freq to each, add to prior queue later
    {
        TreeNode *Node = new TreeNode;
        symbol = symbolString[0]; //first index of inputfile = the letter
        freqString = symbolString.substr(2); // last index of inputfile = the frequency
        freq = stoi(freqString); // convert to int
        totalFreq += freq; //add the total of freq
        Node->symbol = symbol; // assign the data
        Node->frequency = freq;
        Node->left = NULL;
        Node->right = NULL; 
        pq.push(Node);
    }

    HuffmanTree tree = HuffmanTree();
    tree.buildHuffmanTree(pq);// build the tree

    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int port = parseInt(argv[1]);

    if(openSocketServer(server_fd, address, opt, addrlen, port) == -1)
    {
        return 0;
    }
    signal(SIGCHLD, fireman);
    while(true)
    {
        int socket_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if(fork() == 0)
        {
            processSocket(socket_fd, tree);
        }
    }

    return 0;
}
