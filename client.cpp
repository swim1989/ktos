#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include "xtos.h"

void split(const std::string& s, std::vector<std::string>& sv, const char* delim = " ") {
    sv.clear();                               
    char* buffer = new char[s.size() + 1];   
    buffer[s.size()] = '\0';
    std::copy(s.begin(), s.end(), buffer);   
    char* p = std::strtok(buffer, delim);     
    do {
        sv.push_back(p);                      
    } while ((p = std::strtok(NULL, delim))); 
    delete[] buffer;
    return;
}

int main(int argc, char *argv[]) {
    std::cout << "This is client" << std::endl;
    // socket
    int client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == -1) {
        std::cout << "Error: socket" << std::endl;
        return 0;
    }

    // connect
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8199);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(client, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cout << "Error: connect" << std::endl;
        return 0;
    }
    std::cout << "connected.." << std::endl;
    // char data[255];
    if(atoi(argv[1]) == 1)
    {
        std::cout << "Request solve" << std::endl;
        std::string tmp =  "xtos_solve#";
        tmp += "6#5#";
        tmp += "1,2,3,4,5,0,0,1,1,1,2,4,0,5,2,3,2,5,0,1,4,3,4,5,3,1,4,3,4,5#";
        tmp += "45,10,0,0,0,0,0,0,0,1365,0,0,0,6,0,1,0,0,0#";
        tmp += "1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0#";
        tmp += "999";
        char send_data[tmp.size() + 1];
        strcpy(send_data, tmp.c_str());

        //char data[] = "xtos_solve#6#5#1,2,3,4,5,0,0,1,2,3,4,5,0,1,2,3,4,5,0,1,2,3,4,5,0,1,2,3,4,5#30,100,0,0,0,0,0,0,0,0,0,0,0,6,0,1,0,0,0#1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0#999";
        send(client, send_data, strlen(send_data), 0);
        char buf[255];
        while (true) {
            memset(buf, 0, sizeof(buf));
            int len = recv(client, buf, sizeof(buf), 0);
            if(len>0){
                buf[len] = '\0';
                std::cout<< buf<<"\n";
                std::string tmp(buf); 
                std::vector<std::string> mdata;           
                split(tmp,mdata,"#");
                std::cout << mdata[1].c_str() <<"\n";
                break;
            }
        }
    }else if(atoi(argv[1]) == 2){
        char data[] = "xtos_get#0#999";
        send(client, data, strlen(data), 0);
        std::cout << "Request Result 0" << std::endl;
        DoraResult* dora = new DoraResult();
        char data2[1024];
        while (true) {                        
            int len =recv(client, data2, sizeof(data2), 0);            
            if(len > 0){
                 std::cout << data2 << "\n";                           
                break;
            }
        }
        std::cout << "Complete Get Results" << std::endl;
        delete dora;
    }else if(atoi(argv[1]) == 3){
        char data[] = "xtos_clean#0#999";
        send(client, data, strlen(data), 0);
        char buf[255];        
        while (true) {            
            memset(buf, 0, sizeof(buf));            
            int len = recv(client, buf, sizeof(buf), 0);
            if(strcmp(buf, "clean_ok") == 0){
                std::cout <<"clean_ok"<<"\n";         
                break;
            }
        }
    } 
    close(client);
    return 0;
}
