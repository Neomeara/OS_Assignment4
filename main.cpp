// CSC 456 Assignment 4
// Nate O'Meara && Alex Zschoche
// Due 12/07/2021

// Libraries
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

// Function Prototypes
void memoryManager( int, int);
int allocate( int, int);
int deallocate( int );
int write( int, int);
int read( int, int);
void printMemory();
void runCommands(string);
void updateLruStack( int, int);


// Global Variables

struct pageTableItem
{
    int frameNumber;
    int valid;
};

struct process
{
    int pid;
    int size;
    vector<pageTableItem> pageTable;
};


struct lruItem
{
    int pid;
    int pageNumber;
};

vector<int> freeFrameList;
vector<process> processList;
vector<int> memory;
vector<lruItem> LRU_stack;


int main()
{

    // get the command from user and loop
    string command = "";
    cout << "Enter a command or exit to quit: ";
    getline(cin, command);
    while ( command != "exit")
    {

        runCommands(command);
    
        cout << "\nEnter a command or exit to quit: ";
        getline(cin, command);

    }

    return 0;
}

// initialize memory and freeFrameList
void memoryManager(int memSize, int frameSize)
{
    for (int i = 0; i < memSize; i++)
    {
        memory.push_back(0);
        freeFrameList.push_back(i);
    }
    
}


int allocate(int allocSize, int pid)
{


    // check if process is already allocated
    for (int i =0; i < processList.size(); i++)
    {
        if(processList[i].pid == pid)
        {
            cout << "Process " << pid << " is already in use." << endl;
            return -1;
            
        }
    }

    process newProcess = {.pid = pid, .size = allocSize};
    for(int i =0; i < allocSize; i++)
    {
        if(freeFrameList.size() > 0)
        {
            // add page to pageTable and set to random free frame
            int randomFrame = rand() % freeFrameList.size();
            pageTableItem pti = {.frameNumber = freeFrameList[randomFrame], .valid = 1};
            newProcess.pageTable.push_back(pti);
            freeFrameList.erase(freeFrameList.begin() + randomFrame);  
            updateLruStack(pid, i);
    

        }
        // Replace victim
        else
        {
            lruItem victim = LRU_stack[0];
            LRU_stack.erase(LRU_stack.begin());
            updateLruStack(pid, i);

            // mark victim as invalid
            for (int j = 0; j < processList.size(); j++)
            {
                if(victim.pid == processList[j].pid)
                {
                    processList[j].pageTable[victim.pageNumber].valid = 0;
                    // delete contents of victim page
                    memory[processList[j].pageTable[victim.pageNumber].frameNumber] = NULL;
                    // add newly empty space to free frame list
                    freeFrameList.push_back(processList[j].pageTable[victim.pageNumber].frameNumber);

                    break;
                }
            }
            

            // add page to pageTable and set to random free frame
            int randomFrame = rand() % freeFrameList.size();
            pageTableItem pti = {.frameNumber = freeFrameList[randomFrame], .valid = 1};
            newProcess.pageTable.push_back(pti);
            freeFrameList.erase(freeFrameList.begin() + randomFrame); 

        }

    }

    processList.push_back(newProcess);



    return 1;
}

int deallocate( int pid)
{

     int processIndex = -1;
    // Find the index of PID from process list
    for (int i = 0; i < processList.size(); i++)
    {
        if(processList[i].pid == pid)
        {
            processIndex = i;
        }
    }

    //check if valid PID
    if(processIndex == -1)
    {
        cout << "\nNo process found with id of: "<<pid << endl;
        return -1;
    }

    // add allocated frames to the freeFrameList and erase process
    int frameNum = 0;
    for(int x = 0; x < processList[processIndex].size; x++)
    {
        frameNum = processList[processIndex].pageTable[x].frameNumber;
        freeFrameList.push_back(frameNum);

    }

     processList.erase(processList.begin() + processIndex);

    return 1;

}

int write( int pid, int logical_address)
{

    int value = 1;

    int processIndex = -1;

    for (int i = 0; i < processList.size(); i++)
    {
        if(processList[i].pid == pid)
        {
            processIndex = i;
        }
    }


    // check if PID exists and if logical_address exists
    if(processIndex == -1)
    {
        cout << "\nNo process found with id of: "<<pid << endl;
        return -1;
    }
    else if (logical_address > processList[processIndex].size)
    {
        cout << "\nNo logical_address found with location of: "<<logical_address << endl;
        return -1;
    }
    
    // chek if page is valid

    if(processList[processIndex].pageTable[logical_address].valid == 1)
    {

        int frameIndex = processList[processIndex].pageTable[logical_address].frameNumber;
        memory[frameIndex] = value;
        updateLruStack(pid, logical_address);


    }
    else
    {
        lruItem victim = LRU_stack[0];
        LRU_stack.erase(LRU_stack.begin());
        updateLruStack(pid, logical_address);

        // mark victim as invalid
        for (int j = 0; j < processList.size(); j++)
        {
            if(victim.pid == processList[j].pid)
            {
                processList[j].pageTable[victim.pageNumber].valid = 0;
                // delete contents of victim page
                memory[processList[j].pageTable[victim.pageNumber].frameNumber] = NULL;
                freeFrameList.push_back(processList[j].pageTable[victim.pageNumber].frameNumber);



                break;
            }
        }

        // assign frame to logical_address
        int randomFrame = rand() % freeFrameList.size();
        freeFrameList.erase(freeFrameList.begin() + randomFrame); 


        processList[processIndex].pageTable[logical_address].frameNumber = randomFrame;
        memory[randomFrame] = value;

    }

    return 1;

}

int read( int pid, int logical_address)
{

    int value = 0;

    int processIndex = 0;

    // check if PID exists and if logical_address exists
    if(processIndex == -1)
    {
        cout << "\nNo process found with id of: "<<pid << endl;
        return -1;
    }
    else if (logical_address > processList[processIndex].size)
    {
        cout << "\nNo logical_address found with location of: "<<logical_address << endl;
        return -1;
    }

    for (int i = 0; i < processList.size(); i++)
    {
        if(processList[i].pid == pid)
        {
            processIndex = i;
        }
    }
    
     if(processList[processIndex].pageTable[logical_address].valid == 1)
    {

        int frameIndex = processList[processIndex].pageTable[logical_address].frameNumber;
        value = memory[frameIndex];

        cout << "The value from logical_address " << logical_address << " in process " << pid << " is: " << value << endl; 
    cout << "The value from logical_address " << logical_address << " in process " << pid << " is: " << value << endl; 
        cout << "The value from logical_address " << logical_address << " in process " << pid << " is: " << value << endl; 

        updateLruStack(pid, logical_address);


    }
    else
    {
        lruItem victim = LRU_stack[0];
        LRU_stack.erase(LRU_stack.begin());
        updateLruStack(pid, logical_address);

        // mark victim as invalid
        for (int j = 0; j < processList.size(); j++)
        {
            if(victim.pid == processList[j].pid)
            {
                processList[j].pageTable[victim.pageNumber].valid = 0;
                // delete contents of victim page
                memory[processList[j].pageTable[victim.pageNumber].frameNumber] = NULL;
                freeFrameList.push_back(processList[j].pageTable[victim.pageNumber].frameNumber);


                break;
            }
        }

        // assign frame to logical_address
        int randomFrame = rand() % freeFrameList.size();
        freeFrameList.erase(freeFrameList.begin() + randomFrame); 

        processList[processIndex].pageTable[logical_address].frameNumber = randomFrame;

        value = memory[randomFrame];

        cout << "The value from logical_address " << logical_address << " in process " << pid << " is: " << value << endl; 

    }


    return 1;

}

void printMemory()
{
    cout << endl << "------------------ PHYSICAL MEMORY ---------------------" << endl << " | ";

    for (int i = 0; i < memory.size(); i++)
    {
        cout << memory[i] << " | ";
    }
    

    cout << endl << "------------------ FREE FRAME LIST ---------------------" << endl;

    for (int i = 0; i < freeFrameList.size(); i++)
    {
        cout << freeFrameList[i] << ", ";
    }
    

    cout << endl << "------------------ PROCESS LIST ------------------------" << endl;

    for (int i = 0; i < processList.size(); i++)
    {
        cout << "PID: " << processList[i].pid << " SIZE: " << processList[i].size << " | ";
    }

    cout << endl << "------------------ LRU STACK ------------------------" << endl;

    for (int i = 0; i < LRU_stack.size(); i++)
    {
        cout << "PID: " << LRU_stack[i].pid << " PAGE NUMBER: " << LRU_stack[i].pageNumber << " | ";
    }
    cout << endl << endl;

}

void runCommands(string command)
{
    stringstream ssCommand(command);

    char functionName = '0';
    string param1 = "";
    string param2 = "";

    ssCommand >> functionName >> param1 >> param2;

    functionName = tolower(functionName);
    switch (functionName)
    {
        case 'm' :
            // ssCommand << param1 << param2;

            memoryManager(stoi(param1), stoi(param2));

            break;
        
        case 'a' :
            // ssCommand << param1 << param2;
            allocate(stoi(param1), stoi(param2));
            break;
        
        case 'w' :
            // ssCommand << param1 << param2;
            write(stoi(param1), stoi(param2));


            break;
        
        case 'r' :
            // ssCommand << param1 << param2;
            read(stoi(param1), stoi(param2));

            break;
        
        case 'd' :
            // ssCommand << param1;
            deallocate(stoi(param1));
            break;
        
        case 'p' :
            printMemory();
            break;

        default:
            cout << "\n Unknown command - Please try again\n";
            break;
        }


}

void updateLruStack( int pid, int pageNumber)
{
    // create new lruItem
    lruItem li = {.pid = pid, .pageNumber = pageNumber};

    // look to see if lruItem is in list then move to back
    for (int i = 0; i < LRU_stack.size(); i++)
    {
        if (LRU_stack[i].pid == pid && LRU_stack[i].pageNumber == pageNumber)
        {
            LRU_stack.erase(LRU_stack.begin() + 1);
            LRU_stack.push_back(li);
            break;
            return;
            
        }
        
    }

    // if get here add reference to the top.
    LRU_stack.push_back(li);
    
}