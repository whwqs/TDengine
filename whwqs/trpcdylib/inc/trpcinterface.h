#ifndef TRPCINTERFACE_H
#define TRPCINTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _TD_LINUX_64
#define DLLAPI
#else
#define DLLAPI __declspec(dllexport)
#endif

#include "exception.h"
#include "os.h"
#include "rpcLog.h"
#include "tglobal.h"
#include "tqueue.h"
#include "trpc.h"
#include "hash.h"
#include "rpcHead.h"
#include "tsocket.h"
#include "taosmsg.h"

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
  int       index;
  SRpcEpSet epSet;
  int       num;
  int       numOfReqs;
  SRpcMsg * pMsg;
  tsem_t    rspSem;
  tsem_t *  pOverSem;
  pthread_t thread;
  void *    pRpc;
  char *    result;
} _SInfo;

typedef struct {
  uint16_t localPort;              // local port
  char     label[TSDB_LABEL_LEN];  // for debug purpose
  int      numOfThreads;           // number of threads to handle connections
  int      sessions;               // number of sessions allowed
  int8_t   connType;               // TAOS_CONN_UDP, TAOS_CONN_TCPC, TAOS_CONN_TCPS
  int      idleTime;               // milliseconds, 0 means idle timer is disabled

  // the following is for client app ecurity only
  char user[TSDB_USER_LEN];   // user name
  char spi;                   // security parameter index
  char encrypt;               // encrypt algorithm
  char secret[TSDB_KEY_LEN];  // key for authentication
  char ckey[TSDB_KEY_LEN];    // ciphering key

} _SRpcInit;

typedef struct {
  char            info[48];                 // debug info: label + pConn + ahandle
  int             sid;                      // session ID
  uint32_t        ownId;                    // own link ID
  uint32_t        peerId;                   // peer link ID
  char            user[TSDB_UNI_LEN];       // user ID for the link
  char            spi;                      // security parameter index
  char            encrypt;                  // encryption, 0:1
  char            secret[TSDB_KEY_LEN];     // secret for the link
  char            ckey[TSDB_KEY_LEN];       // ciphering key
  char            secured;                  // if set to 1, no authentication
  uint16_t        localPort;                // for UDP only
  uint32_t        linkUid;                  // connection unique ID assigned by client
  uint32_t        peerIp;                   // peer IP
  uint16_t        peerPort;                 // peer port
  char            peerFqdn[TSDB_FQDN_LEN];  // peer FQDN or ip string
  uint16_t        tranId;                   // outgoing transcation ID, for build message
  uint16_t        outTranId;                // outgoing transcation ID
  uint16_t        inTranId;                 // transcation ID for incoming msg
  uint8_t         outType;                  // message type for outgoing request
  uint8_t         inType;                   // message type for incoming request
  void *          chandle;                  // handle passed by TCP/UDP connection layer
  void *          ahandle;                  // handle provided by upper app layter
  int             retry;                    // number of retry for sending request
  int             tretry;                   // total retry
  void *          pTimer;                   // retry timer to monitor the response
  void *          pIdleTimer;               // idle timer
  char *          pRspMsg;                  // response message including header
  int             rspMsgLen;                // response messag length
  char *          pReqMsg;                  // request message including header
  int             reqMsgLen;                // request message length
  _SRpcInfo *      pRpc;                     // the associated SRpcInfo
  int8_t          connType;                 // connection type
  int64_t         lockedBy;                 // lock for connection
  //SRpcReqContext *pContext;                 // request context
} _SRpcConn;

typedef char *(*RequestCallback)(char *pContent);

typedef struct {
  bool         commit;
  RequestCallback requestcbk;
  char            dataFile[20];
  _SRpcInit       rpcInit;
} TrpcServerInit;

typedef struct {
  int8_t   inUse;
  int8_t   numOfEps;
  uint16_t port[TSDB_MAX_REPLICA];
  char *   fqdn[TSDB_MAX_REPLICA];
  //[TSDB_MAX_REPLICA][TSDB_FQDN_LEN];
} TrpcEpSet;

void SetRpcCfp(void *param, void (*cfp)(SRpcMsg *, SRpcEpSet *));

DLLAPI void *StartServerListen(TrpcServerInit initData);

DLLAPI char *ClientSendAndReceive(void *pRpc, TrpcEpSet serverEps, char *pCont);

DLLAPI void *_RpcOpen(_SRpcInit rpcInit);

DLLAPI void _RpcClose(void *param);

DLLAPI int32_t InitLog(char *logName, int32_t numOfLogLines, int32_t maxFiles);
DLLAPI void    CloseLog();
DLLAPI void    ResetLog();
DLLAPI void    SetDebug(int32_t rpcDebugFlag);
DLLAPI void    SetCompressMsgSize(int32_t CompressMsgSize);
#ifdef __cplusplus
}
#endif

#endif