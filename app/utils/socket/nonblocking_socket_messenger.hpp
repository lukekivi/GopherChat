#ifndef NON_BLOCKING_SOCKET_MESSENGER_HPP
#define NON_BLOCKING_SOCKET_MESSENGER_HPP

#include "../utils.hpp"
#include "../log.hpp"
#include "socket_messenger.hpp"
#include <poll.h>

class NonblockingSocketMessenger: public SocketMessenger {

  public:
    NonblockingSocketMessenger(Log* log) { SetLog(log); }
    ~NonblockingSocketMessenger() { delete log; };

    int SendNonBlocking(struct CONN_STAT* pStat, struct pollfd* pPeer);
    int RecvNonBlocking(struct CONN_STAT* pStat, struct pollfd* pPeer);

  private:
    void SetLog(Log* log);
    Log* log;

};

#endif 