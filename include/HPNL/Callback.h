#ifndef CALLBACK_H
#define CALLBACK_H

class Callback {
  public:
    virtual      ~Callback() {}
    virtual void operator()(void *param_1, void *param_2) = 0;
};

#endif
