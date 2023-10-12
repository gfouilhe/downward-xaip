#ifndef POLICY_CLIENT_H
#define POLICY_CLIENT_H

#include "../../operator_id.h"
#include "../../search_space.h"

#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

namespace policy {
class PolicyClient{

private:
    int port;
    int sockfd;
    struct sockaddr_in servaddr; 

    std::shared_ptr<AbstractTask> task;
    

public:
    explicit PolicyClient(const int port);

    void initialize(const std::shared_ptr<AbstractTask> &task);

    std::vector<double> get_value(const State & state, std::vector<OperatorID> & operators);
};
}

#endif
