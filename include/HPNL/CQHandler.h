#ifndef CQHANDLER_H
#define CQHANDLER_H

#include "EventHandler.h"
#include "Connection.h"
#include "Stack.h"
#include "FIStack.h"
#include "Callback.h"

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
