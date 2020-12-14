#include "trpc.h"
#include <tglobal.h>
#include <hash.h>
#include <rpcHead.h>

int main(int argc, char* argv[]) {

   SRpcInit rpcInit;
  memset(&rpcInit, 0, sizeof(rpcInit));
  rpcInit.localPort = 7000;
  rpcInit.label = "SER";
  rpcInit.numOfThreads = 1;
  rpcInit.cfp = NULL;
  rpcInit.sessions = 1000;
  rpcInit.idleTime = tsShellActivityTimer * 1500;
  rpcInit.afp = NULL;
  rpcInit.connType = TAOS_CONN_SERVER;
  void* pRpc = rpcOpen(&rpcInit);
  
  if (NULL != pRpc) {
   
    rpcClose(pRpc);
    //pRpc = rpcOpen(&rpcInit);
  }
  return 0;
}
