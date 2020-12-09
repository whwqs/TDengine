#include "os.h"
#include "tglobal.h"
#include "rpcLog.h"
#include "trpc.h"
#include "tqueue.h"
#include "exception.h"
#include "trpcinterface.h"

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
} SInfo;

static void processResponse(SRpcMsg *pMsg, SRpcEpSet *pEpSet) {
  SInfo *pInfo = (SInfo *)pMsg->ahandle;
  if (pEpSet) pInfo->epSet = *pEpSet; 
  pInfo->result = calloc(1, pMsg->contLen);
  strcpy(pInfo->result, pMsg->pCont);  
  tsem_post(&pInfo->rspSem);
}

static void *sendRequest(void *param) {
  SInfo * pInfo = (SInfo *)param;
  rpcSendRequest(pInfo->pRpc, &pInfo->epSet, pInfo->pMsg, NULL);
  return NULL;
}

char *ClientSendAndReceive(void *pRpc, TrpcEpSet serverEps, char *pCont) {
    
    if (!pCont) {
      return NULL;
    }

    int length = (int)strlen(pCont);  

    if (length == 0) {
      return NULL;
    }

    SetRpcCfp(pRpc, processResponse);

    SRpcEpSet epSet;
    epSet.numOfEps = serverEps.numOfEps;
    epSet.inUse = serverEps.inUse;
    for (int i = 0; i < TSDB_MAX_REPLICA; i++) {
      epSet.port[i] = serverEps.port[i];
      strcpy(epSet.fqdn[i], serverEps.fqdn[i]);
    }   

    SRpcMsg rpcMsg = {0};
    rpcMsg.pCont = rpcMallocCont(length);
    rpcMsg.contLen = length;
    strcpy(rpcMsg.pCont, pCont);
    
    rpcMsg.msgType = 1;

    pthread_attr_t thattr;
    pthread_attr_init(&thattr);
    pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_JOINABLE);  // PTHREAD_CREATE_JOINABLE PTHREAD_CREATE_DETACHED

    SInfo *pInfo = (SInfo *)calloc(1, sizeof(SInfo) * 1);
    rpcMsg.ahandle = pInfo;
    pInfo->index = 0;
    pInfo->epSet = epSet;
    pInfo->numOfReqs = 1;
    pInfo->num = 0;
    pInfo->pMsg = &rpcMsg;
    pInfo->pRpc = pRpc;
    tsem_init(&pInfo->rspSem, 0, 0);
    
    pthread_create(&pInfo->thread, &thattr, sendRequest, pInfo);
    
    tsem_wait(&pInfo->rspSem);    
    
	return pInfo->result;
}