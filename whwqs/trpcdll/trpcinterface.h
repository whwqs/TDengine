#pragma once
#ifndef TRPCINTERFACE_H
#define TRPCINTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#define DLLAPI __declspec(dllexport)

#include "exception.h"
#include "os.h"
#include "rpcLog.h"
#include "tglobal.h"
#include "tqueue.h"
#include "trpc.h"
#include "hash.h"
#include "rpcHead.h"
#include "tsocket.h"

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

typedef char *(*RequestCallback)(char *pContent);

typedef struct {
  boolean         commit;
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