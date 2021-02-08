package com.taosdata.jdbc.rs;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.taosdata.jdbc.AbstractStatement;
import com.taosdata.jdbc.TSDBConstants;
import com.taosdata.jdbc.rs.util.HttpClientPoolUtil;
import com.taosdata.jdbc.utils.SqlSyntaxValidator;

import java.sql.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

public class RestfulStatement extends AbstractStatement {

    private boolean closed;
    private String database;
    private final RestfulConnection conn;

    private volatile RestfulResultSet resultSet;
    private volatile int affectedRows;

    public RestfulStatement(RestfulConnection conn, String database) {
        this.conn = conn;
        this.database = database;
    }

    private String[] parseTableIdentifier(String sql) {
        sql = sql.trim().toLowerCase();
        String[] ret = null;
        if (sql.contains("where"))
            sql = sql.substring(0, sql.indexOf("where"));
        if (sql.contains("interval"))
            sql = sql.substring(0, sql.indexOf("interval"));
        if (sql.contains("fill"))
            sql = sql.substring(0, sql.indexOf("fill"));
        if (sql.contains("sliding"))
            sql = sql.substring(0, sql.indexOf("sliding"));
        if (sql.contains("group by"))
            sql = sql.substring(0, sql.indexOf("group by"));
        if (sql.contains("order by"))
            sql = sql.substring(0, sql.indexOf("order by"));
        if (sql.contains("slimit"))
            sql = sql.substring(0, sql.indexOf("slimit"));
        if (sql.contains("limit"))
            sql = sql.substring(0, sql.indexOf("limit"));
        // parse
        if (sql.contains("from")) {
            sql = sql.substring(sql.indexOf("from") + 4).trim();
            return Arrays.asList(sql.split(",")).stream()
                    .map(tableIdentifier -> {
                        tableIdentifier = tableIdentifier.trim();
                        if (tableIdentifier.contains(" "))
                            tableIdentifier = tableIdentifier.substring(0, tableIdentifier.indexOf(" "));
                        return tableIdentifier;
                    }).collect(Collectors.joining(",")).split(",");
        }
        return ret;
    }

    @Override
    public ResultSet executeQuery(String sql) throws SQLException {
        if (isClosed())
            throw new SQLException("statement already closed");
        if (!SqlSyntaxValidator.isValidForExecuteQuery(sql))
            throw new SQLException("not a valid sql for executeQuery: " + sql);

        final String url = "http://" + conn.getHost() + ":" + conn.getPort() + "/rest/sql";
        if (SqlSyntaxValidator.isDatabaseUnspecifiedQuery(sql)) {
            return executeOneQuery(url, sql);
        }

        if (this.database == null || this.database.isEmpty())
            throw new SQLException("Database not specified or available");
        HttpClientPoolUtil.execute(url, "use " + this.database);
        return executeOneQuery(url, sql);
    }

    @Override
    public int executeUpdate(String sql) throws SQLException {
        if (isClosed())
            throw new SQLException("statement already closed");
        if (!SqlSyntaxValidator.isValidForExecuteUpdate(sql))
            throw new SQLException("not a valid sql for executeUpdate: " + sql);

        final String url = "http://" + conn.getHost() + ":" + conn.getPort() + "/rest/sql";
        if (SqlSyntaxValidator.isDatabaseUnspecifiedUpdate(sql)) {
            return executeOneUpdate(url, sql);
        }

        if (this.database == null || this.database.isEmpty())
            throw new SQLException("Database not specified or available");
        HttpClientPoolUtil.execute(url, "use " + this.database);
        return executeOneUpdate(url, sql);
    }

    @Override
    public void close() throws SQLException {
        synchronized (RestfulStatement.class) {
            if (!isClosed())
                this.closed = true;
        }
    }

    @Override
    public boolean execute(String sql) throws SQLException {
        if (isClosed())
            throw new SQLException("Invalid method call on a closed statement.");
        if (!SqlSyntaxValidator.isValidForExecute(sql))
            throw new SQLException("not a valid sql for execute: " + sql);

        //如果执行了use操作应该将当前Statement的catalog设置为新的database
        final String url = "http://" + conn.getHost() + ":" + conn.getPort() + "/rest/sql";
        if (SqlSyntaxValidator.isUseSql(sql)) {
            HttpClientPoolUtil.execute(url, sql);
            this.database = sql.trim().replace("use", "").trim();
            this.conn.setCatalog(this.database);
        } else if (SqlSyntaxValidator.isDatabaseUnspecifiedQuery(sql)) {
            executeOneQuery(url, sql);
        } else if (SqlSyntaxValidator.isDatabaseUnspecifiedUpdate(sql)) {
            executeOneUpdate(url, sql);
        } else {
            if (SqlSyntaxValidator.isValidForExecuteQuery(sql)) {
                executeQuery(sql);
            } else {
                executeUpdate(sql);
            }
        }

        return true;
    }

    private ResultSet executeOneQuery(String url, String sql) throws SQLException {
        if (!SqlSyntaxValidator.isValidForExecuteQuery(sql))
            throw new SQLException("not a select sql for executeQuery: " + sql);

        // row data
        String result = HttpClientPoolUtil.execute(url, sql);
        JSONObject resultJson = JSON.parseObject(result);
        if (resultJson.getString("status").equals("error")) {
            throw new SQLException(TSDBConstants.WrapErrMsg("SQL execution error: " + resultJson.getString("desc") + "\n" + "error code: " + resultJson.getString("code")));
        }
        // parse table name from sql
        String[] tableIdentifiers = parseTableIdentifier(sql);
        if (tableIdentifiers != null) {
            List<JSONObject> fieldJsonList = new ArrayList<>();
            for (String tableIdentifier : tableIdentifiers) {
                // field meta
                String fields = HttpClientPoolUtil.execute(url, "DESCRIBE " + tableIdentifier);
                JSONObject fieldJson = JSON.parseObject(fields);
                if (fieldJson.getString("status").equals("error")) {
                    throw new SQLException(TSDBConstants.WrapErrMsg("SQL execution error: " + fieldJson.getString("desc") + "\n" + "error code: " + fieldJson.getString("code")));
                }
                fieldJsonList.add(fieldJson);
            }
            this.resultSet = new RestfulResultSet(database, this, resultJson, fieldJsonList);
        } else {
            this.resultSet = new RestfulResultSet(database, this, resultJson);
        }
        this.affectedRows = 0;
        return resultSet;
    }

    private int executeOneUpdate(String url, String sql) throws SQLException {
        if (!SqlSyntaxValidator.isValidForExecuteUpdate(sql))
            throw new SQLException("not a valid sql for executeUpdate: " + sql);

        String result = HttpClientPoolUtil.execute(url, sql);
        JSONObject jsonObject = JSON.parseObject(result);
        if (jsonObject.getString("status").equals("error")) {
            throw new SQLException(TSDBConstants.WrapErrMsg("SQL execution error: " + jsonObject.getString("desc") + "\n" + "error code: " + jsonObject.getString("code")));
        }
        this.resultSet = null;
        this.affectedRows = Integer.parseInt(jsonObject.getString("rows"));
        return this.affectedRows;
    }

    @Override
    public ResultSet getResultSet() throws SQLException {
        if (isClosed())
            throw new SQLException(TSDBConstants.STATEMENT_CLOSED);
        return resultSet;
    }

    @Override
    public int getUpdateCount() throws SQLException {
        if (isClosed()) {
            throw new SQLException("Invalid method call on a closed statement.");
        }
        return this.affectedRows;
    }

    @Override
    public void addBatch(String sql) throws SQLException {
        if (isClosed())
            throw new SQLException(TSDBConstants.STATEMENT_CLOSED);
        //TODO:
    }

    @Override
    public void clearBatch() throws SQLException {
        //TODO:
    }

    @Override
    public int[] executeBatch() throws SQLException {
        //TODO:
        return new int[0];
    }

    @Override
    public Connection getConnection() throws SQLException {
        if (isClosed())
            throw new SQLException(TSDBConstants.STATEMENT_CLOSED);
        return this.conn;
    }

    @Override
    public boolean isClosed() throws SQLException {
        return closed;
    }


}
