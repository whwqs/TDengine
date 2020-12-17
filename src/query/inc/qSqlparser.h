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

#ifndef TDENGINE_QSQLPARSER_H
#define TDENGINE_QSQLPARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "taos.h"
#include "taosmsg.h"
#include "tstoken.h"
#include "tstrbuild.h"
#include "tvariant.h"

#define ParseTOKENTYPE SStrToken
extern char tTokenTypeSwitcher[13];

#define toTSDBType(x)                          \
  do {                                         \
    if ((x) >= tListLen(tTokenTypeSwitcher)) { \
      (x) = TSDB_DATA_TYPE_BINARY;             \
    } else {                                   \
      (x) = tTokenTypeSwitcher[(x)];           \
    }                                          \
  } while (0)

typedef struct SLimitVal {
  int64_t limit;
  int64_t offset;
} SLimitVal;

typedef struct SOrderVal {
  uint32_t order;
  int32_t orderColId;
} SOrderVal;

typedef struct tVariantListItem {
  tVariant pVar;
  uint8_t  sortOrder;
} tVariantListItem;

typedef struct SIntervalVal {
  SStrToken interval;
  SStrToken offset;
} SIntervalVal;

typedef struct SQuerySQL {
  struct tSQLExprList *pSelection;   // select clause
  SArray *             from;         // from clause  SArray<tVariantListItem>
  struct tSQLExpr *    pWhere;       // where clause [optional]
  SArray *             pGroupby;     // groupby clause, only for tags[optional], SArray<tVariantListItem>
  SArray *             pSortOrder;   // orderby [optional], SArray<tVariantListItem>
  SStrToken            interval;     // interval [optional]
  SStrToken            offset;       // offset window [optional]
  SStrToken            sliding;      // sliding window [optional]
  SLimitVal            limit;        // limit offset [optional]
  SLimitVal            slimit;       // group limit offset [optional]
  SArray *             fillType;     // fill type[optional], SArray<tVariantListItem>
  SStrToken            selectToken;  // sql string
} SQuerySQL;

typedef struct SCreatedTableInfo {
  SStrToken  name;        // table name token
  SStrToken  stableName;  // super table name token , for using clause
  SArray    *pTagVals;    // create by using super table, tag value
  char      *fullname;    // table full name
  STagData   tagdata;     // true tag data, super table full name is in STagData
  int8_t     igExist;     // ignore if exists
} SCreatedTableInfo;

typedef struct SCreateTableSQL {
  SStrToken    name;  // table name, create table [name] xxx
  int8_t       type;  // create normal table/from super table/ stream
  bool         existCheck;

  struct {
    SArray    *pTagColumns; // SArray<TAOS_FIELD>
    SArray    *pColumns;    // SArray<TAOS_FIELD>
  } colInfo;

  SArray      *childTableInfo;        // SArray<SCreatedTableInfo>
  SQuerySQL   *pSelect;
} SCreateTableSQL;

typedef struct SAlterTableSQL {
  SStrToken     name;
  int16_t       type;
  STagData      tagData;
  SArray       *pAddColumns; // SArray<TAOS_FIELD>
  SArray       *varList;  // set t=val or: change src dst, SArray<tVariantListItem>
} SAlterTableSQL;

typedef struct SCreateDBInfo {
  SStrToken dbname;
  int32_t   replica;
  int32_t   cacheBlockSize;
  int32_t   maxTablesPerVnode;
  int32_t   numOfBlocks;
  int32_t   daysPerFile;
  int32_t   minRowsPerBlock;
  int32_t   maxRowsPerBlock;
  int32_t   fsyncPeriod;
  int64_t   commitTime;
  int32_t   walLevel;
  int32_t   quorum;
  int32_t   compressionLevel;
  SStrToken precision;
  bool      ignoreExists;
  int8_t    update; 
  
  SArray *keep;
} SCreateDBInfo;

typedef struct SCreateAcctSQL {
  int32_t   maxUsers;
  int32_t   maxDbs;
  int32_t   maxTimeSeries;
  int32_t   maxStreams;
  int32_t   maxPointsPerSecond;
  int64_t   maxStorage;
  int64_t   maxQueryTime;
  int32_t   maxConnections;
  SStrToken stat;
} SCreateAcctSQL;

typedef struct SShowInfo {
  uint8_t showType;
  SStrToken prefix;
  SStrToken pattern;
} SShowInfo;

typedef struct SUserInfo {
  SStrToken user;
  SStrToken passwd;
  SStrToken privilege;
  int16_t   type;
} SUserInfo;

typedef struct tDCLSQL {
  int32_t    nTokens; /* Number of expressions on the list */
  int32_t    nAlloc;  /* Number of entries allocated below */
  SStrToken *a;       /* one entry for element */
  bool  existsCheck;
  
  union {
    SCreateDBInfo  dbOpt;
    SCreateAcctSQL acctOpt;
    SShowInfo      showOpt;
    SStrToken      ip;
  };
  
  SUserInfo user;
  
} tDCLSQL;

typedef struct SSubclauseInfo {  // "UNION" multiple select sub-clause
  SQuerySQL **pClause;
  int32_t     numOfClause;
} SSubclauseInfo;

typedef struct SSqlInfo {
  int32_t            type;
  bool               valid;
  
  union {
    SCreateTableSQL *pCreateTableInfo;
    SAlterTableSQL  *pAlterInfo;
    tDCLSQL         *pDCLInfo;
  };
  
  SSubclauseInfo     subclauseInfo;
  char               pzErrMsg[256];
} SSqlInfo;

typedef struct tSQLExpr {
  uint32_t         nSQLOptr;    // TK_FUNCTION: sql function, TK_LE: less than(binary expr)
  
  // the full sql string of function(col, param), which is actually the raw
  // field name, since the function name is kept in nSQLOptr already
  SStrToken        operand;
  SStrToken        colInfo;     // field id
  tVariant         val;         // value only for string, float, int
  SStrToken        token;       // original sql expr string

  struct tSQLExpr *pLeft;       // left child
  struct tSQLExpr *pRight;      // right child
  struct tSQLExprList *pParam;  // function parameters
} tSQLExpr;

// used in select clause. select <tSQLExprList> from xxx
typedef struct tSQLExprItem {
  tSQLExpr *pNode;      // The list of expressions
  char *    aliasName;  // alias name, null-terminated string
} tSQLExprItem;

// todo refactor by using SArray
typedef struct tSQLExprList {
  int32_t       nExpr;  /* Number of expressions on the list */
  int32_t       nAlloc; /* Number of entries allocated below */
  tSQLExprItem *a;      /* One entry for each expression */
} tSQLExprList;

/**
 *
 * @param yyp      The parser
 * @param yymajor  The major token code number
 * @param yyminor  The value for the token
 */
void Parse(void *yyp, int yymajor, ParseTOKENTYPE yyminor, SSqlInfo *);

/**
 *
 * @param p         The parser to be deleted
 * @param freeProc  Function used to reclaim memory
 */
void ParseFree(void *p, void (*freeProc)(void *));

SArray *tVariantListAppend(SArray *pList, tVariant *pVar, uint8_t sortOrder);
SArray *tVariantListInsert(SArray *pList, tVariant *pVar, uint8_t sortOrder, int32_t index);
SArray *tVariantListAppendToken(SArray *pList, SStrToken *pAliasToken, uint8_t sortOrder);

tSQLExpr *tSQLExprCreate(tSQLExpr *pLeft, tSQLExpr *pRight, int32_t optType);

void tSQLExprDestroy(tSQLExpr *);

tSQLExprList *tSQLExprListAppend(tSQLExprList *pList, tSQLExpr *pNode, SStrToken *pToken);

void tSQLExprListDestroy(tSQLExprList *pList);

SQuerySQL *tSetQuerySQLElems(SStrToken *pSelectToken, tSQLExprList *pSelection, SArray *pFrom, tSQLExpr *pWhere,
                             SArray *pGroupby, SArray *pSortOrder, SIntervalVal *pInterval,
                             SStrToken *pSliding, SArray *pFill, SLimitVal *pLimit, SLimitVal *pGLimit);

SCreateTableSQL *tSetCreateSQLElems(SArray *pCols, SArray *pTags, SQuerySQL *pSelect, int32_t type);

void tSQLExprNodeDestroy(tSQLExpr *pExpr);

SAlterTableSQL *tAlterTableSQLElems(SStrToken *pTableName, SArray *pCols, SArray *pVals, int32_t type);
SCreatedTableInfo createNewChildTableInfo(SStrToken *pTableName, SArray *pTagVals, SStrToken *pToken, SStrToken* igExists);

void destroyAllSelectClause(SSubclauseInfo *pSql);
void doDestroyQuerySql(SQuerySQL *pSql);
void freeCreateTableInfo(void* p);

SSqlInfo       *setSQLInfo(SSqlInfo *pInfo, void *pSqlExprInfo, SStrToken *pTableName, int32_t type);
SSubclauseInfo *setSubclause(SSubclauseInfo *pClause, void *pSqlExprInfo);

SSubclauseInfo *appendSelectClause(SSubclauseInfo *pInfo, void *pSubclause);

void setCreatedTableName(SSqlInfo *pInfo, SStrToken *pTableNameToken, SStrToken *pIfNotExists);

void SQLInfoDestroy(SSqlInfo *pInfo);

void setDCLSQLElems(SSqlInfo *pInfo, int32_t type, int32_t nParams, ...);
void setDropDBTableInfo(SSqlInfo *pInfo, int32_t type, SStrToken* pToken, SStrToken* existsCheck);
void setShowOptions(SSqlInfo *pInfo, int32_t type, SStrToken* prefix, SStrToken* pPatterns);

tDCLSQL *tTokenListAppend(tDCLSQL *pTokenList, SStrToken *pToken);

void setCreateDBSQL(SSqlInfo *pInfo, int32_t type, SStrToken *pToken, SCreateDBInfo *pDB, SStrToken *pIgExists);

void setCreateAcctSQL(SSqlInfo *pInfo, int32_t type, SStrToken *pName, SStrToken *pPwd, SCreateAcctSQL *pAcctInfo);
void setCreateUserSQL(SSqlInfo *pInfo, SStrToken *pName, SStrToken *pPasswd);
void setKillSQL(SSqlInfo *pInfo, int32_t type, SStrToken *ip);
void setAlterUserSQL(SSqlInfo *pInfo, int16_t type, SStrToken *pName, SStrToken* pPwd, SStrToken *pPrivilege);

void setDefaultCreateDbOption(SCreateDBInfo *pDBInfo);

// prefix show db.tables;
void setDBName(SStrToken *pCpxName, SStrToken *pDB);

tSQLExpr *tSQLExprIdValueCreate(SStrToken *pToken, int32_t optType);

tSQLExpr *tSQLExprCreateFunction(tSQLExprList *pList, SStrToken *pFuncToken, SStrToken *endToken, int32_t optType);

void tSQLSetColumnInfo(TAOS_FIELD *pField, SStrToken *pName, TAOS_FIELD *pType);

void tSQLSetColumnType(TAOS_FIELD *pField, SStrToken *pToken);

void *ParseAlloc(void *(*mallocProc)(size_t));

enum {
  TSQL_NODE_TYPE_EXPR  = 0x1,
  TSQL_NODE_TYPE_ID    = 0x2,
  TSQL_NODE_TYPE_VALUE = 0x4,
};

#define NON_ARITHMEIC_EXPR 0
#define NORMAL_ARITHMETIC  1
#define AGG_ARIGHTMEIC     2

SSqlInfo qSQLParse(const char *str);

#ifdef __cplusplus
}
#endif

#endif  // TDENGINE_QSQLPARSER_H
