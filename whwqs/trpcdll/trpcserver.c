#include "exception.h"
#include "os.h"
#include "rpcLog.h"
#include "tglobal.h"
#include "tqueue.h"
#include "trpc.h"
#include "trpcinterface.h"
boolean         commit = FALSE;
int             serverDataFd = -1;
RequestCallback callback;

void serverProcessRequestMsg(SRpcMsg *pMsg, SRpcEpSet *pEpSet) {
  SRpcMsg *pTemp;

  pTemp = taosAllocateQitem(sizeof(SRpcMsg));
  memcpy(pTemp, pMsg, sizeof(SRpcMsg));
  if (commit) {
    if (write(serverDataFd, pMsg->pCont, pMsg->contLen) < 0) {
      tInfo("failed to write data file, reason:%s", strerror(errno));
    }
  }

  if (callback) {
    char *strTmp = (char *)calloc(1, pMsg->contLen + 1);
    strcpy(strTmp, pMsg->pCont);
    strTmp[pMsg->contLen] = '\0';
    char *strMsg = callback(strTmp);
    int   Length = (int)strlen(strMsg);

    pTemp->pCont = rpcMallocCont(Length);
    pTemp->contLen = Length;
    strcpy(pTemp->pCont, strMsg);
    pTemp->msgType = 1;

    rpcSendResponse(pTemp);
     //rpcFreeCont(pTemp->pCont);//会报错
  }
}

void * StartServerListen(TrpcServerInit initData) {
  //taosBlockSIGPIPE();
  SRpcInit serverRpcInit;
  callback = initData.requestcbk; //服务端反馈处理回调
  memset(&serverRpcInit, 0, sizeof(serverRpcInit));
  serverRpcInit.localPort = initData.rpcInit.localPort;
  serverRpcInit.label = initData.rpcInit.label;
  serverRpcInit.numOfThreads = initData.rpcInit.numOfThreads;
  serverRpcInit.cfp = serverProcessRequestMsg;
  serverRpcInit.sessions = initData.rpcInit.sessions;
  serverRpcInit.idleTime = initData.rpcInit.idleTime;
  // rpcInit.afp = retrieveAuthInfo;
  serverRpcInit.connType = TAOS_CONN_SERVER;

  if (initData.commit) {
    serverDataFd = open(initData.dataFile, O_APPEND | O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    if (serverDataFd < 0) {
      tInfo("failed to open data file, reason:%s", strerror(errno));
    } else {
      commit = TRUE;
    }
  }

  void *pRpc = rpcOpen(&serverRpcInit);
  if (pRpc == NULL) {
    tError("failed to start RPC server");
    return NULL;
  }

  return pRpc;
}