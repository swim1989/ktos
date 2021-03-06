// author:swim 2021-4-13

// basic structure
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <math.h> 
#include <vector>
// For socket and network
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
// toslib
#include "ktos.cpp"



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

std::string result2char(DoraResult *result){
    std::string ret;
    // ret.append("#");
    ret.append(std::to_string(result->weight));
    ret.append(",");
    ret.append(std::to_string(result->final_weight));
    ret.append(",");
    ret.append(std::to_string(result->combos_length));
    ret.append(",");
    ret.append(std::to_string(result->path_count));
    ret.append(",");
    ret.append(std::to_string(result->data->firstBatch));
    ret.append(",");
    ret.append(std::to_string(result->data->initCursor.x));
    ret.append(",");
    ret.append(std::to_string(result->data->initCursor.y));
    ret.append("#");
    if(result->combos_length>0 || result->data->combos[0].mGemCount>0)
    {
        for(int i=0;i<result->combos_length;i++){
                ret.append(std::to_string(result->data->combos[i].mGemType));
                ret.append(std::to_string(result->data->combos[i].mGemCount));
                ret.append(std::to_string(result->data->combos[i].mGemFlag));         
                if(i !=result->combos_length -1)
                    ret.append("|");        
        }
    }
    // std::cout << (int)result->path_count << std::endl;
    ret.append("#");
    unsigned char pathc =  result->path_count;
    if(result->data->combos[0].mGemCount>0)
    {
        for(int i=0;i<(int)pathc;i++){
            ret.append(std::to_string(result->data->dirs[i]));
            // std::cout << std::to_string(result->data->dirs[i]) << std::endl;
        }
    }
    //std::cout << result->path_count << std::endl;
    ret.append("#");
   // ret  = "#"+result.combos_length;

    return ret;

} 


int main() {
    short port = 8199;
    int nNetTimeout=6000;//6???
    time_t start_t, end_t;

    std::cout << "This is server" << std::endl;
    std::cout << "Port:";
    std::cout << port << std::endl;
    // socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        std::cout << fprintf(stderr, "Error creating socket (%d %s)\n", errno, strerror(errno)) << std::endl;
        return 0;
    }
    // bind
    struct sockaddr_in addr;
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;    
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cout << "Error: bind" << std::endl;
        std::cout << printf("Error code: %d\n", errno) << std::endl;
        return 0;
    }
    // listen
    if(listen(listenfd, 5) == -1) {
        std::cout << "Error: listen" << std::endl;
        return 0;
    }
    // accept
    int conn;
    char clientIP[INET_ADDRSTRLEN] = "";
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    //Socket KeepAlive setting
    int TCP_KEEPIDLE = 0x4;
    int TCP_KEEPINTVL = 0x5;
    int TCP_KEEPCNT = 0x6;
    int keepAlive = 1; //Keep Alive open
    int keepIdle = 30; // idle time
    int keepInterval = 5; 
    int keepCount = 3; 
    setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
    setsockopt(listenfd, IPPROTO_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
    setsockopt(listenfd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
    setsockopt(listenfd,IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));



    DoraResultArrayPointer p_array0;
    while (true) {
        std::cout << "***Listening..." << std::endl;
        conn = accept(listenfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (conn < 0) {
            std::cout << "Error: accept" << std::endl;
            continue;
        }
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        std::cout << "->Connect " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;
        char buf[1024];
        while (true) {
            memset(buf, 0, sizeof(buf));
            int len = recv(conn, buf, sizeof(buf), 0);
            buf[len] = '\0';
           if(len<1)
                continue;
            std::string cdata(buf);
            if (strcmp(buf, "exit") == 0) { 
                std::cout << "...Disconnect " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;
                break;
            }
            if(strcmp(cdata.substr(0,4).c_str(),"xtos") == 0) {
                std::cout << "Get the client Parameters" << "\n";
                std::vector<std::string> mdata;
                split(cdata, mdata, "#");
                if(strcmp(mdata[0].c_str(),"xtos_solve") == 0){
                    // std::cout << mdata.size() <<"\n";
                    std::vector<std::string> _idx;
                    std::vector<std::string> _params;
                    std::vector<std::string> _color_priority;
                    std::vector<std::string> _startPoint;
                    int idx[30];
                    int params[19];
                    int color_priority[24];                    
                    // std::cout << "=== Put the Parameters ==="<<"\n";
                    //std::cout << cdata.c_str() <<"\n";
                    split(mdata[3],_idx,",");
                    std::cout << "== The IDX ==" <<"\n";
                    std::cout << mdata[3].substr(0,11) <<"\n";
                    std::cout << mdata[3].substr(12,11) <<"\n";
                    std::cout << mdata[3].substr(22,11) <<"\n";
                    std::cout << mdata[3].substr(34,11) <<"\n";
                    std::cout << mdata[3].substr(44,11) <<"\n";
                    split(mdata[4],_params,",");
                    std::cout << "== The Parameters ==" <<"\n";
                    std::cout << " "+mdata[4] <<"\n";
                    split(mdata[5],_color_priority,",");
                    std::cout << "== The Priority ==" <<"\n";
                    std::cout << " "+ mdata[5] <<"\n";
                    
                    for(int i=0;i<30;i++){
                        idx[i] = atoi(_idx[i].c_str());
                    }
                    for(int i=0;i<19;i++){
                        params[i] = atoi(_params[i].c_str());
                    }                   
                    for(int i=0;i<24;i++){
                        color_priority[i] = atoi(_color_priority[i].c_str());
                    }
                    
                    std::cout << "Start run ..." << "\n";
                    start_t = time(NULL);
                    p_array0  = kora_solve(p_array0,atoi(mdata[1].c_str()),atoi(mdata[2].c_str()),idx, params,color_priority,{});
                    end_t = time(NULL);
                    double diff = difftime(end_t, start_t);
                    std::cout<<printf("Complete Solve, Spend time = %f",diff)<<std::endl;
                    std::cout << (int)(p_array0.ResultPointer[0].path_count)<<"\n";

                    std::string tmp = "solve_ok#"+std::to_string(p_array0.result_index)+"#";
                     char ret[tmp.size() + 1];
                    strcpy(ret, tmp.c_str());
                    std::cout<< ret<<"\n";
                    send(conn,ret,sizeof(ret)/sizeof(char),0);
                    break;
                }else if(strcmp(mdata[0].c_str(),"xtos_get") == 0){                
                     char ret[] = "client_get_start";
                    std::cout << ret <<"\n";   
                    int jindex = atoi(mdata[1].c_str());
                    DoraResult *doraResult = &p_array0.ResultPointer[0]; 
                    std::string ret2 = result2char(doraResult);
                     std::cout << "==== The Results is below  =====" <<"\n";   
                    std::cout << "     "+ret2 <<"\n";
                    char cstr[ret2.size() + 1];
                    strcpy(cstr, ret2.c_str());                    
                    send(conn,cstr,sizeof(cstr),0);
                    // delete doraResult;
                    break;
                }else if(strcmp(mdata[0].c_str(),"xtos_clean") == 0){
                    kora_clean_result(p_array0);
                    char ret[] = "clean_ok";
                    std::cout << ret <<"\n";
                    send(conn,ret,sizeof(ret)/sizeof(char),0);
                    break;                    
                }else if(strcmp(mdata[0].c_str(),"xtos_interrupt") == 0){
                    kora_interrupt();
                    char ret[] = "interrupt_ok";
                    std::cout << ret <<"\n";
                    send(conn,ret,sizeof(ret)/sizeof(char),0);
                    break;                    
                }
            }
        }        
        close(conn);
    }
    close(listenfd);
    return 0;
}