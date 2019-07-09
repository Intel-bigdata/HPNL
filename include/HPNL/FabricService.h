#ifndef FABRICSERVICE_H
#define FABRICSERVICE_H

#include <rdma/fi_domain.h>

class FabricService {
  public:
    virtual fid_domain* get_domain() = 0;  
};

#endif
