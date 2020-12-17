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
#ifndef TDENGINE_QUERYUTIL_H
#define TDENGINE_QUERYUTIL_H

#define SET_RES_WINDOW_KEY(_k, _ori, _len, _uid)     \
  do {                                               \
    assert(sizeof(_uid) == sizeof(uint64_t));        \
    *(uint64_t *)(_k) = (_uid);                      \
    memcpy((_k) + sizeof(uint64_t), (_ori), (_len)); \
  } while (0)

#define GET_RES_WINDOW_KEY_LEN(_l) ((_l) + sizeof(uint64_t))

#define curTimeWindowIndex(_winres)        ((_winres)->curIndex)
#define GET_ROW_PARAM_FOR_MULTIOUTPUT(_q, tbq, sq) (((tbq) && (!sq))? (_q)->pExpr1[1].base.arg->argValue.i64:1)

int32_t getOutputInterResultBufSize(SQuery* pQuery);

size_t  getResultRowSize(SQueryRuntimeEnv* pRuntimeEnv);
int32_t initResultRowInfo(SResultRowInfo* pResultRowInfo, int32_t size, int16_t type);
void    cleanupResultRowInfo(SResultRowInfo* pResultRowInfo);

void    resetResultRowInfo(SQueryRuntimeEnv* pRuntimeEnv, SResultRowInfo* pResultRowInfo);
void    popFrontResultRow(SQueryRuntimeEnv *pRuntimeEnv, SResultRowInfo *pResultRowInfo, int32_t num);
void    clearClosedResultRows(SQueryRuntimeEnv* pRuntimeEnv, SResultRowInfo *pResultRowInfo);
int32_t numOfClosedResultRows(SResultRowInfo* pResultRowInfo);
void    closeAllResultRows(SResultRowInfo* pResultRowInfo);
void    removeRedundantResultRows(SResultRowInfo *pResultRowInfo, TSKEY lastKey, int32_t order);

int32_t initResultRow(SResultRow *pResultRow);
void    closeResultRow(SResultRowInfo* pResultRowInfo, int32_t slot);
bool    isResultRowClosed(SResultRowInfo *pResultRowInfo, int32_t slot);
void    clearResultRow(SQueryRuntimeEnv* pRuntimeEnv, SResultRow* pResultRow, int16_t type);
void    copyResultRow(SQueryRuntimeEnv* pRuntimeEnv, SResultRow* dst, const SResultRow* src, int16_t type);

SResultRowCellInfo* getResultCell(SQueryRuntimeEnv* pRuntimeEnv, const SResultRow* pRow, int32_t index);

static FORCE_INLINE SResultRow *getResultRow(SResultRowInfo *pResultRowInfo, int32_t slot) {
  assert(pResultRowInfo != NULL && slot >= 0 && slot < pResultRowInfo->size);
  return pResultRowInfo->pResult[slot];
}

static FORCE_INLINE char *getPosInResultPage(SQueryRuntimeEnv *pRuntimeEnv, int32_t columnIndex, SResultRow *pResult,
    tFilePage* page) {
  assert(pResult != NULL && pRuntimeEnv != NULL);

  SQuery *pQuery = pRuntimeEnv->pQuery;

  int32_t realRowId = (int32_t)(pResult->rowId * GET_ROW_PARAM_FOR_MULTIOUTPUT(pQuery, pRuntimeEnv->topBotQuery, pRuntimeEnv->stableQuery));
  return ((char *)page->data) + pRuntimeEnv->offset[columnIndex] * pRuntimeEnv->numOfRowsPerPage +
      pQuery->pExpr1[columnIndex].bytes * realRowId;
}

bool isNull_filter(SColumnFilterElem *pFilter, char* minval, char* maxval);
bool notNull_filter(SColumnFilterElem *pFilter, char* minval, char* maxval);

__filter_func_t *getRangeFilterFuncArray(int32_t type);
__filter_func_t *getValueFilterFuncArray(int32_t type);

SResultRowPool* initResultRowPool(size_t size);
SResultRow* getNewResultRow(SResultRowPool* p);
int64_t getResultRowPoolMemSize(SResultRowPool* p);
void* destroyResultRowPool(SResultRowPool* p);
int32_t getNumOfAllocatedResultRows(SResultRowPool* p);
int32_t getNumOfUsedResultRows(SResultRowPool* p);

uint64_t getResultInfoUId(SQueryRuntimeEnv* pRuntimeEnv);
bool isPointInterpoQuery(SQuery *pQuery);


#endif  // TDENGINE_QUERYUTIL_H
