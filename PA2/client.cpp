#include<iostream>
#include<pthread.h>
#include <queue>
#include<fstream>
#include<sstream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
using namespace std;

int parseInt(string s) // character converter, convert character of each string to int value, one way to make data transfering works
{
    int intValue = 0;
    for(int i=0; i<s.length(); i++)
    {
        intValue = intValue * 10 + s[i] - '0';
    }
    return intValue;
}

int openSocketClient(char* server_ipAddress, int port) // code stole from Mr.Rincon on Blackboard, and improved(I think? LOOOOOL)
{
    int clientSocket, ret;
    struct sockaddr_in serverAddr;
    char buffer[1024];
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0) 
    {
        printf("Error in connection.\n");
        return -1;
    }

    memset(&serverAddr, '\0', sizeof(serverAddr));
    memset(buffer, '\0', sizeof(buffer));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    hostent *he = gethostbyname(server_ipAddress);
    char* host_ip_address = inet_ntoa (*((struct in_addr *) he->h_addr_list[0])); 
    serverAddr.sin_addr.s_addr = inet_addr(host_ip_address);

    ret = connect(clientSocket,(struct sockaddr*)&serverAddr, sizeof(serverAddr));

    if (ret < 0) 
    {
        printf("Error in connection.\n");
        return -1;
    }
    return clientSocket;
}

struct BufferData
{
    string compressed_data;
    vector<int> positions;
    char* origin_text;
    char* server_ipAddress;
    int port;
};

void setPositionDecode(BufferData compressed_data, char* buffer_data)
{
    char decode[1024];
    int i;
    for(i=0; i<strlen(buffer_data); i++)
    {
        if(buffer_data[i] == '\n')
            break;
        decode[i] = buffer_data[i];
    }
    decode[i] = '\0';

    for(int i=0; i<compressed_data.positions.size(); i++)
    {
        compressed_data.origin_text[compressed_data.positions[i]] = decode[0]; // replacing pos in the data received with the right index from encoding string
    }
}

void* create_thread_socket(void* param)
{
    BufferData* valuePointer  = (BufferData*)param;
    BufferData compressed_data = *valuePointer;
    int clientSocket = openSocketClient(compressed_data.server_ipAddress, compressed_data.port);
    compressed_data.compressed_data += "\n";
    int compress_data_length = compressed_data.compressed_data.length();
    char* cipher_text = new char[compress_data_length]; // send the compressed data to server
    for(int i=0; i<compress_data_length; i++)
    {
        cipher_text[i] = compressed_data.compressed_data[i];
    }
    send(clientSocket, cipher_text, strlen(cipher_text), 0);
    char message[1024];
    read(clientSocket, message, 1024); // read the decoded position from server, then update.
    setPositionDecode(compressed_data, message);
    close(clientSocket);
    return NULL;
}

vector<BufferData> readInput(int& decompress_length, int port, char* server_ipAddress) // hoho this stuff here is unfun as hell
{
    vector<BufferData> vec;
    string data;
    while(getline(cin, data))
    {
        stringstream my_stringstream(data);
        string binary_compressed;
        my_stringstream >> binary_compressed; // read input into binary data, decoded pos.
        vector<int> vector_position;
        int one_position;
        while(my_stringstream >> one_position)
            vector_position.push_back(one_position);
        BufferData compress_data;
        compress_data.server_ipAddress = server_ipAddress;
        compress_data.compressed_data = binary_compressed;
        compress_data.positions = vector_position;
        compress_data.port = port;
        decompress_length += vector_position.size();
        vec.push_back(compress_data); // push into vector of the new BufferData
    }
    return vec;
}

void create_thread_pool(vector<BufferData> vec)
{
    int thread_size = (int)vec.size();
    vector<pthread_t> threads(thread_size);
    for(int i=0; i<thread_size; i++)
    {
        pthread_create(&threads[i],NULL,create_thread_socket,&vec[i]); // submit thread
    }

    for(int i=0; i<thread_size; i++)
    {
        pthread_join(threads[i], NULL); // wait thread finish
    }
}

int main(int argc, char* argv[]){
    if(argc != 3)
    {
        cout << "Valid command: ./file_client <ip_address_server> <port>" << endl;
        return 0;
    }

    int decompress_length = 0;
    vector<BufferData> vec = readInput(decompress_length, parseInt(argv[2]), argv[1]);

    char* decompress_text = new char[decompress_length];
    for(int i=0; i<vec.size(); i++)
    {
        vec[i].origin_text = decompress_text;
    }

    create_thread_pool(vec);

    cout << "Original message: " << decompress_text;
    return 0;
}
