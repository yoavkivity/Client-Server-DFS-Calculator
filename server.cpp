// Yoav Kivity 206745531
// Yakir Zindani 207872664
// Daniel Elnekave 208267096

#include <iostream>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "std_lib_facilities.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <queue>
#include <set>
#include <algorithm>
#include <cstring>
#include <pthread.h>

using namespace std;

// Global variables for adjacency list and search cache
map<int, vector<int>> adjacency_list;
map<string, vector<int>> searches;
int counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void saveSearch(map<string, vector<int>> &searches, const string &key, const vector<int> &values, int &counter)
{
    int maxSize = 10;
    pthread_mutex_lock(&mutex); // Lock the mutex before accessing shared resources
    // Update the counter for this key
    counter++;

    // Check if the key already exists in the map
    auto it = searches.find(key);
    if (it != searches.end())
    {
        // Update the values associated with the key
        it->second = values;
        it->second.push_back(counter); // Append the counter to keep track of the order
    }
    else
    {
        // Insert the new entry
        searches[key] = values;
        searches[key].push_back(-1);
        searches[key].push_back(counter); // Append the counter

        // Check if the map size exceeds the maximum allowed size
        if (searches.size() > maxSize)
        {
            // Find the oldest entry in the map
            auto oldest = searches.begin();
            for (auto it = searches.begin(); it != searches.end(); ++it)
            {
                if (it->second.back() < oldest->second.back())
                {
                    oldest = it;
                }
            }
            // Erase the oldest entry
            searches.erase(oldest);
        }
    }
    
    pthread_mutex_unlock(&mutex); // Unlock the mutex after accessing shared resources
}

// Function to retrieve search results from the cache
vector<int> getSearch(map<string, vector<int>> &searches, const string &key)
{
    pthread_mutex_lock(&mutex); // Lock the mutex before accessing shared resources
    
    auto it = searches.find(key);
    if (it != searches.end())
    {
        auto iter = it->second;
        pthread_mutex_unlock(&mutex); // Unlock the mutex before returning
        return iter;            // Return the vector associated with the key
    }
    else
    {
        vector <int> iter;
        pthread_mutex_unlock(&mutex); // Unlock the mutex before returning
        return iter;                    // Return an empty vector
    }
}

// // Function to read the adjacency list from a file
void readAdjacencyList(const string &filename)
{
    pthread_mutex_lock(&mutex); // Lock the mutex before accessing shared resources
    ifstream file(filename);

    for (int x, y; file >> x >> y;)
    {
        adjacency_list[x].push_back(y);
        adjacency_list[y].push_back(x); // Add the reverse edge for undirected graph
    }
    
    file.close();
    pthread_mutex_unlock(&mutex); // Unlock the mutex before returning
}

// Function to reconstruct the path from start node to end node
vector<int> reconstructPath(int start_node, int end_node, map<int, int> parent_map)
{
    vector<int> path;
    int current = end_node;
    while (current != start_node)
    {
        path.push_back(current);
        current = parent_map[current];
    }
    path.push_back(start_node);
    reverse(path.begin(), path.end());
    return path;
}

// Function to find the shortest path between two nodes
vector<int> shortestPath(int start_node, int end_node)
{
    queue<int> q;
    q.push(start_node);
    map<int, int> parent_map; // Map to store parent nodes for path reconstruction
    set<int> visited;
    bool found = false;
    while (!q.empty() && !found)
    {
        int node = q.front();
        q.pop();
        if (adjacency_list.find(node) != adjacency_list.end())
        {
            for (int neighbor : adjacency_list.at(node))
            {
                if (visited.find(neighbor) == visited.end())
                {
                    q.push(neighbor);
                    parent_map[neighbor] = node; // Set parent of neighbor to current node
                    visited.insert(neighbor);
                    if (neighbor == end_node)
                    {
                        found = true;
                        break;
                    }
                }
            }
        }
    }

    vector<int> path;
    if (found)
    {
        path = reconstructPath(start_node, end_node, parent_map);
    }
    else
    {
        if (visited.find(end_node) != visited.end())
        {
            path.push_back(end_node);
        }
    }

    return path;
}

// Function to print the path
string printPath(int start_node, int end_node, const vector<int> &path)
{
    stringstream ss;
    if (!path.empty())
    {
        for (int node : path)
        {
            if (node == -1)
            {
                break;
            }
            ss << node << " ";
        }
        ss << endl;
    }
    else
    {
        ss << "No path found from " << start_node << " to " << end_node << endl;
    }
    return ss.str();
}

// Client thread function
void *client_thread_func(void *arg)
{
    int connfd = *(int *)arg;
    char pair[256];
    int n = read(connfd, pair, sizeof(pair));
    pair[n] = '\0';
    string s_pair(pair);

    // Split the pair into two numbers
    stringstream ss(s_pair);
    int first_number, second_number;
    ss >> first_number >> second_number;

    // cout << "First number: " << first_number << endl;
    // cout << "Second number: " << second_number << endl;

    // Retrieve search results from the cache
    vector<int> search_values = getSearch(searches, s_pair);
    if (search_values.empty())
    {
        // If not found in cache, calculate shortest path
        cout << "NO Cache Found" << endl;
        vector<int> path = shortestPath(first_number, second_number);
        if (path.empty())
        {
            // Save the result indicating no path exists
            search_values.push_back(-1);
        }
        else
        {
            // Save the search result
            search_values = path;
        }
        string print = printPath(first_number, second_number, path);

        // Save the search result regardless of whether a path was found
        saveSearch(searches, s_pair, search_values, counter);

        cout << print << endl;
        write(connfd, print.c_str(), print.size()); // Use c_str() to get const char* for write
    }
    else
    {
        // If found in cache, retrieve and send the cached result
        cout << "Cache Found" << endl;
        if (search_values.front() == -1) // Check if the cached result indicates no path
        {
            string print = "No path found from " + to_string(first_number) + " to " + to_string(second_number) + "\n";
            cout << print << endl;
            write(connfd, print.c_str(), print.size()); // Use c_str() to get const char* for write
        }
        else
        {
            string print = printPath(first_number, second_number, search_values);
            cout << print << endl;
            write(connfd, print.c_str(), print.size()); // Use c_str() to get const char* for write
        }
    }

    close(connfd);
    return NULL;
}

// Main function
int main(int argc, char **argv)
{
    // Read adjacency list from file
    readAdjacencyList(argv[1]); // server

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket
    int res = bind(sockfd, (sockaddr *)&addr, sizeof(addr));
    listen(sockfd, 5);

    // Accept and handle client connections
    for (;;)
    {
        int *connfd = new int; // Dynamically allocate memory for connfd
        *connfd = accept(sockfd, NULL, NULL);
        // cout << "connected " << *connfd << endl;
        pthread_t thread;
        pthread_create(&thread, NULL, client_thread_func, connfd); // Pass the address of connfd
    }

    // Close socket
    close(sockfd);
    return 0;
}
