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
    string inputfilename;
    char symbol; 
    int freq; 
    int totalFreq; 
    string symbolString;
    string freqString;
    
    cin >> inputfilename;
    ifstream inputFile(inputfilename); 
    priority_queue<TreeNode*, vector<TreeNode*>, comparator> pq;
    map<int,char> originalMessage; // declare map of the position and the letter that corresponds to that position
    
    if (inputFile.good())
    {
        while (getline(inputFile, symbolString)) //read each line, create nodes for symbol, assign freq to each, add to prior queue later
        {                                       
            TreeNode *Node = new TreeNode;
            symbol = symbolString[0]; //first index of inputfile = the letter
            freqString = symbolString.substr(2); // last index of inputfile = the frequency
            freq = stoi(freqString); 
            totalFreq += freq; //add the total of freq
            Node->symbol = symbol; // assign the data
            Node->frequency = freq;
            Node->left = NULL;
            Node->right = NULL; 
            pq.push(Node);
        }
    }
    else
    if (inputFile.fail()) // check for error inputfile
    {
        cout << "Wrong Input File name." << endl;
        exit(0);
    }

    HuffmanTree tree = HuffmanTree(); 
    tree.buildHuffmanTree(pq);// build the tree

    string compressedfilename;
    cin >> compressedfilename;
    ifstream compressedFile(compressedfilename);
    
    vector<pthread_t> threadVectors; // using vector for N-th Threads
    vector<ThreadInfo*> threadReturns; // using vector for N-th of return Threads

    if (compressedFile.good()) // create N-th Threads for each line that it reads
    {
        while (getline(compressedFile, symbolString)) // read line by line of compressedFile
        {
            vector<string> stringVectors;
            splitString(symbolString, " ", stringVectors); // symbolString acts as the container for all the symbols
            vector<int> indexVectors; 

            for (int i = 1; i < stringVectors.size(); i++)
            {
                indexVectors.push_back(stoi(stringVectors[i]));
            }
            ThreadInfo *arg = new ThreadInfo();
            arg->binCodeLine = stringVectors[0];
            arg->indexes = indexVectors;
            arg->tree = tree;
            arg->treeRoot = tree.getRoot();
                    
            pthread_t tid; 
            if(pthread_create(&tid, NULL, decompressionFunc, (void*) arg)) //pthread creating that hold the huffman tree's info, the decompressed
            {
                 cout << "Error: Failed to create thread." << endl;
                exit(0);
            }  
            threadVectors.push_back(tid); // add the new thread into the vector of threads 
            threadReturns.push_back(arg); // add the information values it gathered to the vector of threads for return       
        }
    }
    else
    if(compressedFile.fail()) //check for compressedFile error
    {
        cout << "Wrong Compressed File Name." << endl;
        exit(0);
    }
    
    for (int i = 0; i < threadVectors.size(); i++)
    { 
        pthread_join(threadVectors[i], NULL); //join the thread from the threadVectors
    }    

    for (const auto& threadReturner : threadReturns) // build map
    { 
       for(const auto& letterPosition: threadReturner->characterStorage)
       {
            originalMessage[letterPosition.first] = letterPosition.second; // read information from threadReturns to the originalMessage map
       }
    }
    cout << "Original message: "; 

    for(const auto& letters : originalMessage) 
    {
        cout << letters.second; //print original message characters by characters
    }
    
    inputFile.close();
    compressedFile.close(); // close to avoid memory leak
    return 0;
}