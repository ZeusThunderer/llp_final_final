enum DataType_ {
  INT_TYPE,
  FLOAT_TYPE,
  STRING_TYPE,
  BOOL_TYPE
}

union Cell_ {
  1: i64 integer;
  2: double dbl;
  3: bool bl;
  4: string str;
}

enum QueryType_ {
  CREATE_STMT,
  DROP_STMT,
  SELECT_STMT,
  UPDATE_STMT,
  INSERT_STMT,
  DELETE_STMT
}

enum LogicalOperator_ {
  AND_OP,
  OR_OP
}

struct Column_ {
  1: string column_name;
  2: DataType_ data_type;
  3: i16 size;
}

struct CreateStmt_ {
  1: list<Column_> cols;
  2: string tblName;
}

struct Val_ {
  1: Cell_ cell;
  2: DataType_ type;
}

struct ColVal_ {
  1: string col;
  2: Val_ val;
}

struct InsertStmt_ {
  1: string tblName;
  2: list<list<ColVal_>> rows;
}

struct DropStmt_ {
  1: string tblName;
}

enum CompType_ {
  EQUALS_COMP,
  NOT_EQUALS_COMP,
  GREATER_COMP,
  LESS_COMP,
  GREATER_EQUALS_COMP,
  LESS_EQUALS_COMP,
  LIKE_COMP
}

struct FullColumnName_ {
  1: string row_name;
  2: string col_name;
}

struct Condition_ {
  1: FullColumnName_ column;
  2: CompType_ comp_type;
  3: Val_ value;
}

struct CompoundCondition_ {
  1: Condition_ left;
  2: LogicalOperator_ op;
  3: optional list<CompoundCondition_> right;
}

struct Projection_ {
  1: string new_col;
  2: FullColumnName_ src;
}

struct SelectList_ {
  1: optional string rowName;
  2: optional list<Projection_> projs;
}

struct Join_ {
  1: string rowName;
  2: string tblName;
  3: FullColumnName_ left_col;
  4: FullColumnName_ right_col;
}

struct SelectStmt_ {
  1: string rowName;
  2: string tblName;
  3: optional Join_ join;
  4: CompoundCondition_ where;
  5: SelectList_ select;
}

struct UpdateStmt_ {
  1: string tblName;
  2: list<ColVal_> new_row;
  3: CompoundCondition_ cond;
}

struct DeleteStmt_ {
  1: string tblName;
  2: optional CompoundCondition_ cond;
}

union Stmt_ {
  1: CreateStmt_ create_tbl;
  2: DropStmt_ drop_tbl;
  3: InsertStmt_ insert;
  4: SelectStmt_ select;
  5: UpdateStmt_ update;
  6: DeleteStmt_ dl;
}

struct Statement_{
    1: QueryType_ st_type
    2: Stmt_ stmt
}

enum Status_ {
  OK,
  TABLE_NOT_FOUND,
  BAD_REQUEST,
  SERVER_ERROR    
}

struct ServerResponse {
  1: string info;
  2: list<list<ColVal_>> rows;
  3: Status_ status;
}

service SendStatement_service {
   ServerResponse executeStmt(1: Statement_ statement)
}
