#include "exception.h"
#include "os.h"
#include "rpcLog.h"
#include "tglobal.h"
#include "tqueue.h"
#include "trpc.h"
#include "trpcinterface.h"

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
  
} _SRpcInfo;

void *_RpcOpen(_SRpcInit _rpcInit) {
  SRpcInit rpcInit;
  memset(&rpcInit, 0, sizeof(rpcInit));
  rpcInit.localPort = _rpcInit.localPort;
  rpcInit.connType = _rpcInit.connType;
  rpcInit.label = (char *)calloc(1, TSDB_LABEL_LEN);
  tstrncpy(rpcInit.label, _rpcInit.label, TSDB_LABEL_LEN);
  rpcInit.numOfThreads = _rpcInit.numOfThreads;
  if (rpcInit.numOfThreads <= 0) rpcInit.numOfThreads = 1;
  rpcInit.sessions = _rpcInit.sessions;
  rpcInit.idleTime = _rpcInit.idleTime;
  rpcInit.user = (char *)calloc(1, TSDB_UNI_LEN);
  tstrncpy(rpcInit.user, _rpcInit.user, TSDB_UNI_LEN);
  rpcInit.secret = (char *)calloc(1, TSDB_KEY_LEN);
  memcpy(rpcInit.secret, _rpcInit.secret, TSDB_KEY_LEN);
  rpcInit.ckey = (char *)calloc(1, TSDB_KEY_LEN);
  tstrncpy(rpcInit.ckey, _rpcInit.ckey, TSDB_KEY_LEN);
  rpcInit.spi = _rpcInit.spi;
  rpcInit.encrypt = _rpcInit.encrypt;
  void *pRpc = rpcOpen(&rpcInit);
  if (pRpc == NULL) {
    tError("failed to initialize RPC");     
  }
  tInfo("RPC is initialized");
  return pRpc;
}

void _RpcClose(void *param) { rpcClose(param); }

int32_t        InitLog(char *logName, int32_t numOfLogLines, int32_t maxFiles) {
  return taosInitLog(logName, numOfLogLines, maxFiles);
}
void CloseLog() { taosCloseLog(); }
void ResetLog() { taosResetLog(); }

void SetDebug(int32_t _rpcDebugFlag) {
  tsAsyncLog = 0;
  rpcDebugFlag = _rpcDebugFlag;
  dDebugFlag = rpcDebugFlag;
  uDebugFlag = rpcDebugFlag;
}

void SetCompressMsgSize(int32_t CompressMsgSize) { tsCompressMsgSize = CompressMsgSize; }

void SetRpcCfp(void *param, void (*cfp)(SRpcMsg *, SRpcEpSet *)) {
    _SRpcInfo *_pRpc = (_SRpcInfo *)param;
  _pRpc->cfp = cfp;  
}