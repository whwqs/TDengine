#include "trpcinterface.h"

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

void _RpcClose(void *param) {
  if (NULL != param) {
    _SRpcInfo *pRpc = (_SRpcInfo *)param;    
    rpcClose(param);
    param = NULL;
  }
}

int32_t InitLog(char *logName, int32_t numOfLogLines, int32_t maxFiles) {
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
