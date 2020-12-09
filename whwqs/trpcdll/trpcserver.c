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
  if (commit) {
    if (write(serverDataFd, pMsg->pCont, pMsg->contLen) < 0) {
      tInfo("failed to write data file, reason:%s", strerror(errno));
    }
  }

  SRpcMsg *pTemp =  calloc(1, sizeof(SRpcMsg));  
  memcpy(pTemp, pMsg, sizeof(SRpcMsg));  

  if (callback) {    
    /*char *inMsg = malloc(pMsg->contLen+1);
    strcpy(inMsg, pMsg->pCont);
    inMsg[pMsg->contLen] = '\0';*/
    char *outMsg = callback((char *)pMsg->pCont);
    //free(inMsg);
    int   Length = (int)strlen(outMsg);

    pTemp->pCont = rpcMallocCont(Length);
    pTemp->contLen = Length;
    strcpy(pTemp->pCont, outMsg);
    pTemp->msgType = 1;
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