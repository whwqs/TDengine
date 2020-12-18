#include "trpcinterface.h"

static void processResponse(SRpcMsg *pMsg, SRpcEpSet *pEpSet) {
  _SInfo *pInfo = (_SInfo *)pMsg->ahandle;
  if (pEpSet) pInfo->epSet = *pEpSet; 
  if (NULL != pMsg->pCont && pMsg->contLen>0) 
  {
    pInfo->result = calloc(1, pMsg->contLen + 1);
    memcpy(pInfo->result, pMsg->pCont,pMsg->contLen);
    pInfo->result[pMsg->contLen] = '\0';
  } else {
    pInfo->result = NULL;
  }
  tsem_post(&pInfo->rspSem);
}

static void *sendRequest(void *param) {
  _SInfo * pInfo = (_SInfo *)param;
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
    rpcMsg.msgType = TSDB_MSG_TYPE_SUBMIT;  // TSDB_MSG_TYPE_QUERY TSDB_MSG_TYPE_SUBMIT
    _SInfo *pInfo = (_SInfo *)calloc(1, sizeof(_SInfo) * 1);
    rpcMsg.ahandle = pInfo;
    pInfo->index = 0;
    pInfo->epSet = epSet;
    pInfo->numOfReqs = 1;
    pInfo->num = 0;
    pInfo->pMsg = &rpcMsg;
    pInfo->pRpc = pRpc;
    tsem_init(&pInfo->rspSem, 0, 0);

    pthread_attr_t thattr;
    pthread_attr_init(&thattr);
    pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_JOINABLE);  // PTHREAD_CREATE_JOINABLE PTHREAD_CREATE_DETACHED
    pthread_create(&pInfo->thread, &thattr, sendRequest, pInfo);
    
    tsem_wait(&pInfo->rspSem); 
    
	return pInfo->result;
}