#include "huffmanTree.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <vector>
#include <map>
#include <pthread.h>
using namespace std;

int main()
{
    pthread_mutex_t lockBinary;// critical section for binary string
    pthread_mutex_t lockPrint; // critical section for output printing
    pthread_mutex_t lockDecompress; // critical section for decompression 
    pthread_mutex_init(&lockBinary, NULL); // first lock
    pthread_mutex_init(&lockPrint, NULL); // second lock
    pthread_mutex_init(&lockDecompress, NULL); // third lock 

    string inputfilename;
    char symbol;
    int freq;
    int totalFreq;
    string symbolString;
    string freqString;

    int sym;
    cin >> sym; 
    cin.ignore();
    priority_queue<TreeNode*, vector<TreeNode*>, comparator> pq;

    for( int i = 0; i < sym; i++)
    {
        getline(cin, symbolString);
        TreeNode *Node = new TreeNode;
        symbol = symbolString[0]; //first index of inputfile = the letter
        freqString = symbolString.substr(2); // last index of inputfile = the frequency
        freq = stoi(freqString); // convert to int
        totalFreq += freq; //add the total of freq
        Node->symbol = symbol; // assign the data
        Node->frequency = freq;
        Node->left = nullptr;
        Node->right = nullptr;
        pq.push(Node);
    }

    char* originalMessage = new char[totalFreq];
    HuffmanTree tree = HuffmanTree();
    tree.buildHuffmanTree(pq);// build the tree

    vector<pthread_t> threadVectors; // using vector for firstline-th Threads
    int value = 0; // this the value for mutex id basically, it is used for mutex syncing (relate to currentId from ThreadInfo) lmao.

    ThreadInfo *arg = new ThreadInfo();
    arg->tree = tree;
    arg->treeRoot = tree.getRoot();
    arg->lockBinary = lockBinary;
    arg->lockDecompress = lockDecompress; //pass values into arguments one by one
    arg->originMessage = originalMessage;
    arg->lockPrint = lockPrint;
    arg->currentId = &value;

    for(int i=0; i < sym; i++) // read line by line 
    {
        getline(cin, symbolString);
        vector<string> stringVectors;
        splitString(symbolString, " ", stringVectors); // symbolString acts as the container for all the symbols
        vector<int> indexVectors;
        for (int i = 1; i < stringVectors.size(); i++)
        {
            indexVectors.push_back(stoi(stringVectors[i]));
        }

        pthread_mutex_lock(&(arg->lockBinary)); // first lock due to critical section at Binary location read/changing
        arg->id = i; // ASSIGN ID
        arg->binCodeLine = stringVectors[0]; // assigning elements to binCodeLine field from stringVectors at index 0
        arg->indexes = indexVectors; // POSITIONS OF THEM

        pthread_t tid;
        //PTHREAD CREATE
        if(pthread_create(&tid, NULL, decompressionFunc, (void*) arg)) //pthread creating that hold the huffman tree's info, the decompressed
        {
            cout << "Error: Failed to create thread." << endl;
            exit(0);
        }
        threadVectors.push_back(tid); // add the new thread into the vector of threads
    }
    //PTHREAD JOIN
    for (int i = 0; i < threadVectors.size(); i++)
    {
        pthread_join(threadVectors[i], NULL); //join the thread from the threadVectors
    }
    // FINALLY WE PRINT THIS ORIGINAL MESSAGE THINGY LOL
    cout << "Original message: ";

    for(int i=0; i<totalFreq; i++)
    {
        cout << originalMessage[i]; //print original message characters by characters
    }

    return 0;
}
