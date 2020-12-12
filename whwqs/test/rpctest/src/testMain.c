#include "trpc.h"
#include <tglobal.h>
#include <hash.h>
#include <rpcHead.h>

typedef struct {
  int      sessions;      // number of sessions allowed
  int      numOfThreads;  // number of threads to process incoming messages
  int      idleTime;      // milliseconds;
  uint16_t localPort;
  int8_t   connType;
  int      index;  // for UDP server only, round robin for multiple threads
  char     label[TSDB_LABEL_LEN];

  char user[TSDB_UNI_LEN];    // meter ID
  char spi;                   // security parameter index
  char encrypt;               // encrypt algorithm
  char secret[TSDB_KEY_LEN];  // secret for the link
  char ckey[TSDB_KEY_LEN];    // ciphering key

  void (*cfp)(SRpcMsg *, SRpcEpSet *);
  int (*afp)(char *user, char *spi, char *encrypt, char *secret, char *ckey);

  int32_t          refCount;
  void *           idPool;     // handle to ID pool
  void *           tmrCtrl;    // handle to timer
  SHashObj *       hash;       // handle returned by hash utility
  void *           tcphandle;  // returned handle from TCP initialization
  void *           udphandle;  // returned handle from UDP initialization
  void *           pCache;     // connection cache
  pthread_mutex_t  mutex;
  struct SRpcConn *connList;  // connection list

} _SRpcInfo;

typedef struct {
  void *              signature;
  SOCKET              fd;           // TCP socket FD
  int                 closedByApp;  // 1: already closed by App
  void *              thandle;      // handle from upper layer, like TAOS
  uint32_t            ip;
  uint16_t            port;
  struct _SThreadObj *pThreadObj;
  struct _SFdObj *    prev;
  struct _SFdObj *    next;
} _SFdObj;

typedef struct {
  pthread_t       thread;
  _SFdObj *       pHead;
  pthread_mutex_t mutex;
  uint32_t        ip;
  bool            stop;
  SOCKET          pollFd;
  int             numOfFds;
  int             threadId;
  char            label[TSDB_LABEL_LEN];
  void *          shandle;  // handle passed by upper layer during server initialization
  void *(*processData)(SRecvInfo *pPacket);
} _SThreadObj;

typedef struct {
  SOCKET        fd;
  uint32_t      ip;
  uint16_t      port;
  char          label[TSDB_LABEL_LEN];
  int           numOfThreads;
  void *        shandle;
  _SThreadObj **pThreadObj;
  pthread_t     thread;
} _SServerObj;

int main(int argc, char* argv[]) {

   SRpcInit rpcInit;
  memset(&rpcInit, 0, sizeof(rpcInit));
  rpcInit.localPort = 7000;
  rpcInit.label = "SER";
  rpcInit.numOfThreads = 1;
  rpcInit.cfp = NULL;
  rpcInit.sessions = 1000;
  rpcInit.idleTime = tsShellActivityTimer * 1500;
  rpcInit.afp = NULL;
  rpcInit.connType = TAOS_CONN_SERVER;
  void* pRpc = rpcOpen(&rpcInit);
  if (NULL != pRpc) {
    _SRpcInfo *pRpc2 = (_SRpcInfo *)pRpc;
    _SServerObj *pServerObj = (_SServerObj *)pRpc2->tcphandle;
    errno = EINVAL;
    taosSetNonblocking(pServerObj->fd, 1);    
    rpcClose(pRpc);
    pRpc = rpcOpen(&rpcInit);
  }
  return 0;
}
