/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "os.h"
#include "taosmsg.h"
#include "hash.h"

#include "qExecutor.h"
#include "qUtil.h"
#include "tbuffer.h"

int32_t getOutputInterResultBufSize(SQuery* pQuery) {
  int32_t size = 0;

  for (int32_t i = 0; i < pQuery->numOfOutput; ++i) {
    size += pQuery->pExpr1[i].interBytes;
  }

  assert(size >= 0);
  return size;
}

int32_t initResultRowInfo(SResultRowInfo *pResultRowInfo, int32_t size, int16_t type) {
  pResultRowInfo->capacity = size;

  pResultRowInfo->type = type;
  pResultRowInfo->curIndex = -1;
  pResultRowInfo->size     = 0;
  pResultRowInfo->prevSKey = TSKEY_INITIAL_VAL;

  pResultRowInfo->pResult = calloc(pResultRowInfo->capacity, POINTER_BYTES);
  if (pResultRowInfo->pResult == NULL) {
    return TSDB_CODE_QRY_OUT_OF_MEMORY;
  }

  return TSDB_CODE_SUCCESS;
}

void cleanupResultRowInfo(SResultRowInfo *pResultRowInfo) {
  if (pResultRowInfo == NULL) {
    return;
  }

  if (pResultRowInfo->capacity == 0) {
    assert(pResultRowInfo->pResult == NULL);
    return;
  }

  if (pResultRowInfo->type == TSDB_DATA_TYPE_BINARY || pResultRowInfo->type == TSDB_DATA_TYPE_NCHAR) {
    for(int32_t i = 0; i < pResultRowInfo->size; ++i) {
      tfree(pResultRowInfo->pResult[i]->key);
    }
  }
  
  tfree(pResultRowInfo->pResult);
}

void resetResultRowInfo(SQueryRuntimeEnv *pRuntimeEnv, SResultRowInfo *pResultRowInfo) {
  if (pResultRowInfo == NULL || pResultRowInfo->capacity == 0) {
    return;
  }

  for (int32_t i = 0; i < pResultRowInfo->size; ++i) {
    SResultRow *pWindowRes = pResultRowInfo->pResult[i];
    clearResultRow(pRuntimeEnv, pWindowRes, pResultRowInfo->type);

    int32_t groupIndex = 0;
    int64_t uid = 0;

    SET_RES_WINDOW_KEY(pRuntimeEnv->keyBuf, &groupIndex, sizeof(groupIndex), uid);
    taosHashRemove(pRuntimeEnv->pResultRowHashTable, (const char *)pRuntimeEnv->keyBuf, GET_RES_WINDOW_KEY_LEN(sizeof(groupIndex)));
  }
  
  pResultRowInfo->curIndex = -1;
  pResultRowInfo->size = 0;
  pResultRowInfo->prevSKey = TSKEY_INITIAL_VAL;
}

int32_t numOfClosedResultRows(SResultRowInfo *pResultRowInfo) {
  int32_t i = 0;
  while (i < pResultRowInfo->size && pResultRowInfo->pResult[i]->closed) {
    ++i;
  }
  
  return i;
}

void closeAllResultRows(SResultRowInfo *pResultRowInfo) {
  assert(pResultRowInfo->size >= 0 && pResultRowInfo->capacity >= pResultRowInfo->size);
  
  for (int32_t i = 0; i < pResultRowInfo->size; ++i) {
    SResultRow* pRow = pResultRowInfo->pResult[i];
    if (pRow->closed) {
      continue;
    }
    
    pRow->closed = true;
  }
}

bool isResultRowClosed(SResultRowInfo *pResultRowInfo, int32_t slot) {
  return (getResultRow(pResultRowInfo, slot)->closed == true);
}

void closeResultRow(SResultRowInfo *pResultRowInfo, int32_t slot) {
  getResultRow(pResultRowInfo, slot)->closed = true;
}

void clearResultRow(SQueryRuntimeEnv *pRuntimeEnv, SResultRow *pResultRow, int16_t type) {
  if (pResultRow == NULL) {
    return;
  }

  // the result does not put into the SDiskbasedResultBuf, ignore it.
  if (pResultRow->pageId >= 0) {
    tFilePage *page = getResBufPage(pRuntimeEnv->pResultBuf, pResultRow->pageId);

    for (int32_t i = 0; i < pRuntimeEnv->pQuery->numOfOutput; ++i) {
      SResultRowCellInfo *pResultInfo = &pResultRow->pCellInfo[i];

      char * s = getPosInResultPage(pRuntimeEnv, i, pResultRow, page);
      size_t size = pRuntimeEnv->pQuery->pExpr1[i].bytes;
      memset(s, 0, size);

      RESET_RESULT_INFO(pResultInfo);
    }
  }

  pResultRow->numOfRows = 0;
  pResultRow->pageId = -1;
  pResultRow->rowId = -1;
  pResultRow->closed = false;

  if (type == TSDB_DATA_TYPE_BINARY || type == TSDB_DATA_TYPE_NCHAR) {
    tfree(pResultRow->key);
  } else {
    pResultRow->win = TSWINDOW_INITIALIZER;
  }
}

SResultRowCellInfo* getResultCell(SQueryRuntimeEnv* pRuntimeEnv, const SResultRow* pRow, int32_t index) {
  assert(index >= 0 && index < pRuntimeEnv->pQuery->numOfOutput);
  return (SResultRowCellInfo*)((char*) pRow->pCellInfo + pRuntimeEnv->rowCellInfoOffset[index]);
}

size_t getResultRowSize(SQueryRuntimeEnv* pRuntimeEnv) {
  return (pRuntimeEnv->pQuery->numOfOutput * sizeof(SResultRowCellInfo)) + pRuntimeEnv->interBufSize + sizeof(SResultRow);
}

SResultRowPool* initResultRowPool(size_t size) {
  SResultRowPool* p = calloc(1, sizeof(SResultRowPool));
  if (p == NULL) {
    return NULL;
  }

  p->numOfElemPerBlock = 128;

  p->elemSize = (int32_t) size;
  p->blockSize = p->numOfElemPerBlock * p->elemSize;
  p->position.pos = 0;

  p->pData = taosArrayInit(8, POINTER_BYTES);
  return p;
}

SResultRow* getNewResultRow(SResultRowPool* p) {
  if (p == NULL) {
    return NULL;
  }

  void* ptr = NULL;
  if (p->position.pos == 0) {
    ptr = calloc(1, p->blockSize);
    taosArrayPush(p->pData, &ptr);

  } else {
    size_t last = taosArrayGetSize(p->pData);

    void** pBlock = taosArrayGet(p->pData, last - 1);
    ptr = ((char*) (*pBlock)) + p->elemSize * p->position.pos;
  }

  p->position.pos = (p->position.pos + 1)%p->numOfElemPerBlock;
  initResultRow(ptr);

  return ptr;
}

int64_t getResultRowPoolMemSize(SResultRowPool* p) {
  if (p == NULL) {
    return 0;
  }

  return taosArrayGetSize(p->pData) * p->blockSize;
}

int32_t getNumOfAllocatedResultRows(SResultRowPool* p) {
  return (int32_t) taosArrayGetSize(p->pData) * p->numOfElemPerBlock;
}

int32_t getNumOfUsedResultRows(SResultRowPool* p) {
  return getNumOfAllocatedResultRows(p) - p->numOfElemPerBlock + p->position.pos;
}

void* destroyResultRowPool(SResultRowPool* p) {
  if (p == NULL) {
    return NULL;
  }

  size_t size = taosArrayGetSize(p->pData);
  for(int32_t i = 0; i < size; ++i) {
    void** ptr = taosArrayGet(p->pData, i);
    tfree(*ptr);
  }

  taosArrayDestroy(p->pData);

  tfree(p);
  return NULL;
}

void interResToBinary(SBufferWriter* bw, SArray* pRes, int32_t tagLen) {
  uint32_t numOfGroup = (uint32_t) taosArrayGetSize(pRes);
  tbufWriteUint32(bw, numOfGroup);
  tbufWriteUint16(bw, tagLen);

  for(int32_t i = 0; i < numOfGroup; ++i) {
    SInterResult* pOne = taosArrayGet(pRes, i);
    if (tagLen > 0) {
      tbufWriteBinary(bw, pOne->tags, tagLen);
    }

    uint32_t numOfCols = (uint32_t) taosArrayGetSize(pOne->pResult);
    tbufWriteUint32(bw, numOfCols);
    for(int32_t j = 0; j < numOfCols; ++j) {
      SStddevInterResult* p = taosArrayGet(pOne->pResult, j);
      uint32_t numOfRows = (uint32_t) taosArrayGetSize(p->pResult);

      tbufWriteUint16(bw, p->colId);
      tbufWriteUint32(bw, numOfRows);

      for(int32_t k = 0; k < numOfRows; ++k) {
        SResPair v = *(SResPair*) taosArrayGet(p->pResult, k);
        tbufWriteDouble(bw, v.avg);
        tbufWriteInt64(bw, v.key);
      }
    }
  }
}

SArray* interResFromBinary(const char* data, int32_t len) {
  SBufferReader br = tbufInitReader(data, len, false);
  uint32_t numOfGroup = tbufReadUint32(&br);
  uint16_t tagLen = tbufReadUint16(&br);

  char* tag = NULL;
  if (tagLen > 0) {
    tag = calloc(1, tagLen);
  }

  SArray* pResult = taosArrayInit(4, sizeof(SInterResult));

  for(int32_t i = 0; i < numOfGroup; ++i) {
    if (tagLen > 0) {
      memset(tag, 0, tagLen);
      tbufReadToBinary(&br, tag, tagLen);
    }

    uint32_t numOfCols = tbufReadUint32(&br);

    SArray* p = taosArrayInit(numOfCols, sizeof(SStddevInterResult));
    for(int32_t j = 0; j < numOfCols; ++j) {
      int16_t colId = tbufReadUint16(&br);
      int32_t numOfRows = tbufReadUint32(&br);

      SStddevInterResult interRes = {.colId = colId, .pResult = taosArrayInit(4, sizeof(struct SResPair)),};
      for(int32_t k = 0; k < numOfRows; ++k) {
        SResPair px = {0};
        px.avg = tbufReadDouble(&br);
        px.key = tbufReadInt64(&br);

        taosArrayPush(interRes.pResult, &px);
      }

      taosArrayPush(p, &interRes);
    }

    char* p1 = NULL;
    if (tagLen > 0) {
      p1 = malloc(tagLen);
      memcpy(p1, tag, tagLen);
    }

    SInterResult d = {.pResult = p, .tags = p1,};
    taosArrayPush(pResult, &d);
  }

  tfree(tag);
  return pResult;
}

void freeInterResult(void* param) {
  SInterResult* pResult = (SInterResult*) param;
  tfree(pResult->tags);

  int32_t numOfCols = (int32_t) taosArrayGetSize(pResult->pResult);
  for(int32_t i = 0; i < numOfCols; ++i) {
    SStddevInterResult *p = taosArrayGet(pResult->pResult, i);
    taosArrayDestroy(p->pResult);
  }

  taosArrayDestroy(pResult->pResult);
}