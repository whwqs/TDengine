#include "trpcinterface.h"

typedef struct {
  SRpcEpSet  epSet;
  SRpcMsg *  pMsg;
  tsem_t     rspSem;
  pthread_t  thread;
  void *     pRpc;
  TrpcInOut *result;
} _SInfo;

char *      timeoutMsg = "server timeout";

static void processResponse(SRpcMsg *pMsg, SRpcEpSet *pEpSet) {
  _SInfo *pInfo = (_SInfo *)pMsg->ahandle;
  if (pEpSet) pInfo->epSet = *pEpSet;
  if (pMsg->code != 0) {    
    int32_t len = (int32_t)strlen(timeoutMsg);
    pInfo->result->length = len;
    pInfo->result->buffer = calloc(len, 1);
    memcpy(pInfo->result->buffer, timeoutMsg, len);
  } else if (NULL != pMsg->pCont && pMsg->contLen > 0) {
    pInfo->result->length = pMsg->contLen;
    pInfo->result->buffer = calloc(pMsg->contLen, 1);
    memcpy(pInfo->result->buffer, pMsg->pCont, pMsg->contLen);
    rpcFreeCont(pMsg->pCont);
  }
  tsem_post(&pInfo->rspSem);
}

static void *sendRequest(void *param) {
  _SInfo *pInfo = (_SInfo *)param;
  rpcSendRequest(pInfo->pRpc, &pInfo->epSet, pInfo->pMsg, NULL);
  return NULL;
}

void ClientSendAndReceive(void *pRpc, TrpcEpSet serverEps, TrpcInOut input, ResponseCallback cbk) {
  if (NULL == input.buffer || 0 >= input.length) {
    return ;
  }
  TrpcInOut output;
  output.length = 0;
  output.buffer = NULL;
  
  _SRpcInfo *_pRpc = (_SRpcInfo *)pRpc;
  _pRpc->cfp = processResponse;
  _pRpc->numOfThreads = 1;

  SRpcEpSet epSet;
  epSet.numOfEps = serverEps.numOfEps;
  epSet.inUse = serverEps.inUse;
  for (int i = 0; i < TSDB_MAX_REPLICA; i++) {
    epSet.port[i] = serverEps.port[i];
    strcpy(epSet.fqdn[i], serverEps.fqdn[i]);    
  }

  SRpcMsg rpcMsg;
  rpcMsg.pCont = rpcMallocCont(input.length);
  rpcMsg.contLen = input.length;
  memcpy(rpcMsg.pCont, input.buffer, input.length);
  free(input.buffer);
  rpcMsg.msgType = TSDB_MSG_TYPE_SUBMIT;  // TSDB_MSG_TYPE_QUERY TSDB_MSG_TYPE_SUBMIT
  _SInfo pInfo;
  rpcMsg.ahandle = &pInfo;
  pInfo.epSet = epSet;
  pInfo.pMsg = &rpcMsg;
  pInfo.pRpc = pRpc;
  pInfo.result = &output;
  tsem_init(&pInfo.rspSem, 0, 0);

  pthread_attr_t thattr;
  pthread_attr_init(&thattr);
  pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_JOINABLE);  // PTHREAD_CREATE_JOINABLE PTHREAD_CREATE_DETACHED
  pthread_create(&pInfo.thread, &thattr, sendRequest, &pInfo);

  tsem_wait(&pInfo.rspSem);  //  

  // struct timeval timeSecs;
  // time_t         curTime;
  // gettimeofday(&timeSecs, NULL);
  // curTime = timeSecs.tv_sec;
  // struct timespec ts;
  // ts.tv_sec = curTime + timeout;//服务端无响应timeout秒后认为超时
  // ts.tv_nsec = 0;
  // sem_timedwait(&pInfo->rspSem, &ts);//

  // if (ETIMEDOUT == errno) {
  //  tInfo("sem_timedwait 超时:%s", strerror(errno));
  //}

  tsem_destroy(&pInfo.rspSem); 
  pthread_join(pInfo.thread, NULL);
  //rpcFreeCont(rpcMsg.pCont);
  cbk(output);
  if (output.buffer) {
    free(output.buffer);
    output.buffer = NULL;
  }
  
}