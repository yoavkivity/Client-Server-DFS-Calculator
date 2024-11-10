# Client-Server-DFS-Calculator
A client-server system designed to calculate routes between two specified nodes using DFS algorithm




## _Overview_

This project is a server-client application that computes the shortest path between two nodes in a graph using Depth-First Search (DFS). The graph is loaded from a CSV file (db.csv). The server supports multiple clients concurrently using threads and implements a cache to store the last 10 search results, improving performance.

## _Key Features_

**DFS Algorithm:** Computes the path between two nodes.

**Cache Mechanism:** Stores the last 10 requests to avoid redundant calculations.

**Multi-Threading:** Handles multiple client requests concurrently.

**CSV Data Storage:** Graph edges are stored in db.csv.



## _**Server**_
**Graph Representation:** The graph is loaded from db.csv and stored as an adjacency list.

**Cache:** The server checks the cache for pre-computed results. If not found, it computes the path using DFS.

**Threads:** Each client request is handled in a separate thread using pthread.



## _**Client**_
**Request:** The client sends a node pair to the server.

**Response:** The server sends back the computed path or a cached result.



## _**Dependencies**_

C++ Standard Library (std_lib_facilities.h)

POSIX Threads (pthread)

CSV File (db.csv): Contains the graph edges.

Compilation and Execution


## _**Compilation:**_
**Clone the repository:**

git clone https://github.com/yoavkivity/Client-Server-DFS-Calculator.git

cd Client-Server-DFS-Calculator

g++ server.cpp -o server

g++ client.cpp -o client

./server db.csv 8080

./client 8080 1 5

### _**Cache Mechanism**_
The server stores the last 10 search results in a cache. If the result is found in the cache, it is returned immediately. Otherwise, the path is computed and cached.
