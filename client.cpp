// Yoav Kivity 206745531
// Yakir Zindani 207872664
// Daniel Elnekave 208267096

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "std_lib_facilities.h"

void client(int sockfd, sockaddr_in *addr, string first_number, string second_number)
{
    pthread_t client_send_thread;
    connect(sockfd, (sockaddr *)addr, sizeof(*addr));

    // Send first_number and second_number to the server
    string numbers = first_number + " " + second_number;
    
    write(sockfd, numbers.c_str(), numbers.size());

    string s(256, 0);
    auto p = read(sockfd, &s[0], s.size());
    s = s.substr(0, p);
    cout << s << endl;

    cout << "Cancelling send thread" << endl;
    pthread_cancel(client_send_thread);
}

int main(int argc, char **argv)
{

    string first_number(argv[3]);
    string second_number(argv[4]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(atoi(argv[2]));
    client(sockfd, &addr, first_number, second_number);

    close(sockfd);
}