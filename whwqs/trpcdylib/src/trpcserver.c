#include "trpcinterface.h"
bool            commit = 0;
int             serverDataFd = -1;
RequestCallback callback;

static void serverProcessRequestMsg(SRpcMsg *pMsg, SRpcEpSet *pEpSet) {
  if (commit) {
    if (write(serverDataFd, pMsg->pCont, pMsg->contLen) < 0) {
      tInfo("failed to write data file, reason:%s", strerror(errno));
    }
  }
  SRpcMsg *pTemp = calloc(1, sizeof(SRpcMsg));
  memcpy(pTemp, pMsg, sizeof(SRpcMsg));
  pTemp->pCont = rpcMallocCont(0);
  pTemp->contLen = 0;

  if (callback) {
    TrpcInOut input = {0, NULL};
    if (pMsg->contLen > 0) {
      input.length = pMsg->contLen;
      input.buffer = calloc(pMsg->contLen, 1);
      memcpy(input.buffer, pMsg->pCont, pMsg->contLen);
      TrpcInOut output = callback(input);
      pTemp->pCont = rpcMallocCont(output.length);
      pTemp->contLen = output.length;
      memcpy(pTemp->pCont, output.buffer, output.length);
    }
  }
  rpcSendResponse(pTemp);
}

void *StartServerListen(TrpcServerInit initData) {
  // taosBlockSIGPIPE();
  SRpcInit serverRpcInit;
  callback = initData.requestcbk;  //服务端反馈处理回调
  memset(&serverRpcInit, 0, sizeof(serverRpcInit));
  serverRpcInit.localPort = initData.rpcInit.localPort;
  serverRpcInit.label = initData.rpcInit.label;
  serverRpcInit.numOfThreads = initData.rpcInit.numOfThreads;
  serverRpcInit.cfp = serverProcessRequestMsg;
  serverRpcInit.sessions = initData.rpcInit.sessions;
  serverRpcInit.idleTime = initData.rpcInit.idleTime;
  // rpcInit.afp = retrieveAuthInfo;
  serverRpcInit.connType = TAOS_CONN_SERVER;
  commit = 0;
  if (initData.commit) {
    serverDataFd = open(initData.dataFile, O_APPEND | O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    if (serverDataFd < 0) {
      tInfo("failed to open data file, reason:%s", strerror(errno));
    } else {
      commit = 1;
    }
  }

  void *pRpc = rpcOpen(&serverRpcInit);
  if (pRpc == NULL) {
    tError("failed to start RPC server");
    return NULL;
  }

  return pRpc;
}