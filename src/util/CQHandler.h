#ifndef CQHANDLER_H
#define CQHANDLER_H

#include "core/Connection.h"
#include "core/Stack.h"
#include "core/FIStack.h"
#include "util/EventHandler.h"
#include "util/Callback.h"

class CQHandler : public EventHandler {
  public:
    CQHandler(Connection *con_, HandlePtr handle_);
    virtual ~CQHandler() {}
    virtual int handle_event(EventType et, void *context) override;
    virtual HandlePtr get_handle(void) const override;

    void set_read_callback(Callback *callback) override;
  private:
    HandlePtr cqHandle;
    Connection *con;
    Callback *readCallback;
};

#endif
