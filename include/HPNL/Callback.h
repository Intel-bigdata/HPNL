#ifndef CALLBACK_H
#define CALLBACK_H

// Callback interface
// Application need to implement the different kinds of callbacks based this interface,
// like receive callback, send callback, read callback, write callback...
class Callback {
  public:
    virtual ~Callback() = default;
    // Param_1, param_2 can be whatever you want, like connection, buffer size, buffer id.
    // Please see more examples in example directory.
    virtual void operator()(void *param_1, void *param_2) = 0;
};

#endif
