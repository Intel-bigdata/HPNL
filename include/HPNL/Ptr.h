#ifndef PTR_H
#define PTR_H

#include <memory>
#include <assert.h>
#include <iostream>

class EventHandler;
class Handle;

using EventHandlerPtr = std::shared_ptr<EventHandler>;
using HandlePtr = std::shared_ptr<Handle>;

#endif
