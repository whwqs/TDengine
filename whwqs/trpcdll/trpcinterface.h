#pragma once
#ifndef TRPCINTERFACE_H
#define TRPCINTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#define DLLAPI __declspec(dllexport)

typedef struct _SRpcInit {
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

typedef struct TrpcServerInit {
  boolean         commit;
  RequestCallback requestcbk;
  char            dataFile[20];
  _SRpcInit       rpcInit;
} TrpcServerInit;

typedef struct TrpcEpSet {
  int8_t   inUse;
  int8_t   numOfEps;
  uint16_t port[TSDB_MAX_REPLICA];
  char *   fqdn[TSDB_MAX_REPLICA];
  //[TSDB_MAX_REPLICA][TSDB_FQDN_LEN];
} TrpcEpSet;

void SetpRpcCfp(void *param, void (*cfp)(SRpcMsg *, SRpcEpSet *));

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