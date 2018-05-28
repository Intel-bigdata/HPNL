#ifndef PTR_H
#define PTR_H

#include <memory>
#include <assert.h>
#include <iostream>

#include "Log.h"

class EventHandler;
class Handle;

using EventHandlerPtr = std::shared_ptr<EventHandler>;
using HandlePtr = std::shared_ptr<Handle>;
using LogPtr = std::shared_ptr<Log>;

#endif
