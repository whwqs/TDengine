#ifndef TDENGINE_TTYPE_H
#define TDENGINE_TTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "taosdef.h"

#define GET_TYPED_DATA(_v, _finalType, _type, _data) \
  switch (_type) {                                   \
    case TSDB_DATA_TYPE_BOOL:                        \
    case TSDB_DATA_TYPE_TINYINT:                     \
      (_v) = (_finalType)GET_INT8_VAL(_data);        \
      break;                                         \
    case TSDB_DATA_TYPE_SMALLINT:                    \
      (_v) = (_finalType)GET_INT16_VAL(_data);       \
      break;                                         \
    case TSDB_DATA_TYPE_BIGINT:                      \
      (_v) = (_finalType)(GET_INT64_VAL(_data));     \
      break;                                         \
    case TSDB_DATA_TYPE_FLOAT:                       \
      (_v) = (_finalType)GET_FLOAT_VAL(_data);       \
      break;                                         \
    case TSDB_DATA_TYPE_DOUBLE:                      \
      (_v) = (_finalType)GET_DOUBLE_VAL(_data);      \
      break;                                         \
    default:                                         \
      (_v) = (_finalType)GET_INT32_VAL(_data);       \
      break;                                         \
  };

#ifdef __cplusplus
}
#endif

#endif  // TDENGINE_TTYPE_H
