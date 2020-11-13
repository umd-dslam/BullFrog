#include "postgres.h"
#include "fmgr.h"
#include "storage/lwlock.h"
#include "storage/shmem.h"
#include "utils/geo_decls.h"
#include "utils/hsearch.h"
#include "utils/migrate_schema.h"

#include <string.h>
#include "libpq-fe.h"

PG_MODULE_MAGIC;

const char* PQ_CONN_DEFUALTS = "user=postgres password=postgres dbname=tpcc port=5433"; 

void do_exit(PGconn *conn, PGresult *res);
void exec_txns(int32 worker_id, char *buffer);

void do_exit(PGconn *conn, PGresult *res)
{
    fprintf(stderr, "%s\n", PQerrorMessage(conn));

    PQclear(res);
    PQfinish(conn);

    exit(1);
}

void exec_txns(int32 worker_id, char *buffer)
{
    HTAB *hash_table = TrackingHashTables[worker_id];

    PGconn *conn = PQconnectdb(PQ_CONN_DEFUALTS);
    if (PQstatus(conn) == CONNECTION_BAD)
    {
        fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        exit(1);
    }

    // loop migration transactions
    PGresult *res;
    do
    {
        // BEGIN
        res = PQexec(conn, "BEGIN");
        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            printf("BEGIN command failed\n");
            PQclear(res);
            PQfinish(conn);
            exit(1);
        }
        PQclear(res);

        // TRANSACTION
        res = PQexec(conn, buffer);
        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            do_exit(conn, res);
        }
        PQclear(res);

        // COMMIT
        res = PQexec(conn, "COMMIT");
        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            printf("COMMIT command failed\n");
            PQclear(res);
            PQfinish(conn);
            exit(1);
        }
    } while (hash_get_num_entries(hash_table));

    PQclear(res);
    PQfinish(conn);
}

// -----------------------------------------------------------------------------
/* customer_proj1 query 1 */
PG_FUNCTION_INFO_V1(customer_proj1_q1);

Datum customer_proj1_q1(PG_FUNCTION_ARGS)
{
    int32 c_w_id    = PG_GETARG_INT32(0);
    int32 c_d_id    = PG_GETARG_INT32(1);
    int32 c_id      = PG_GETARG_INT32(2);
    int32 worker_id = PG_GETARG_INT32(3);

    char buffer[500];
    char *q1 =
        " insert into customer_proj1("
        "  c_w_id, c_d_id, c_id, c_discount, c_credit, c_last, c_first, "
        "  c_balance, c_ytd_payment, c_payment_cnt, c_delivery_cnt, c_data) "
        "(select "
        "  c_w_id, c_d_id, c_id, c_discount, c_credit, c_last, c_first, "
        "  c_balance, c_ytd_payment, c_payment_cnt, c_delivery_cnt, c_data "
        "from customer "
        "where c_w_id = %d "
        "  and c_d_id = %d "
        "  and c_id = %d);%d";
    sprintf(buffer, q1, c_w_id, c_d_id, c_id, worker_id);

    exec_txns(worker_id, buffer);

    PG_RETURN_VOID();
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
/* customer_proj1 query 2 */
PG_FUNCTION_INFO_V1(customer_proj1_q2);

Datum customer_proj1_q2(PG_FUNCTION_ARGS)
{
    int32 c_w_id    = PG_GETARG_INT32(0);
    int32 c_d_id    = PG_GETARG_INT32(1);
    char* c_last    = PG_GETARG_CSTRING(2);
    int32 worker_id = PG_GETARG_INT32(3);

    char buffer[500];
    char *q2 =
        " insert into customer_proj1("
        "  c_w_id, c_d_id, c_id, c_discount, c_credit, c_last, c_first, "
        "  c_balance, c_ytd_payment, c_payment_cnt, c_delivery_cnt, c_data) "
        "(select "
        "  c_w_id, c_d_id, c_id, c_discount, c_credit, c_last, c_first, "
        "  c_balance, c_ytd_payment, c_payment_cnt, c_delivery_cnt, c_data "
        "from customer "
        "where c_w_id = %d "
        "  and c_d_id = %d "
        "  and c_last = '%s');%d";
    sprintf(buffer, q2, c_w_id, c_d_id, c_last, worker_id);

    exec_txns(worker_id, buffer);

    PG_RETURN_VOID();
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
/* customer_proj2 query 1 */
PG_FUNCTION_INFO_V1(customer_proj2_q1);

Datum customer_proj2_q1(PG_FUNCTION_ARGS)
{
    int32 c_w_id    = PG_GETARG_INT32(0);
    int32 c_d_id    = PG_GETARG_INT32(1);
    int32 c_id      = PG_GETARG_INT32(2);
    int32 worker_id = PG_GETARG_INT32(3);

    char buffer[500];
    char *q1 =
        " insert into customer_proj2("
        "  c_w_id, c_d_id, c_id, c_last, c_first, "
        "  c_street_1, c_city, c_state, c_zip) "
        "(select "
        "  c_w_id, c_d_id, c_id, c_last, c_first, "
        "  c_street_1, c_city, c_state, c_zip "
        "from customer "
        "where c_w_id = %d "
        "  and c_d_id = %d "
        "  and c_id = %d);%d";
    sprintf(buffer, q1, c_w_id, c_d_id, c_id, worker_id);

    exec_txns(worker_id, buffer);

    PG_RETURN_VOID();
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
/* customer_proj2 query 2 */
PG_FUNCTION_INFO_V1(customer_proj2_q2);

Datum customer_proj2_q2(PG_FUNCTION_ARGS)
{
    int32 c_w_id    = PG_GETARG_INT32(0);
    int32 c_d_id    = PG_GETARG_INT32(1);
    char* c_last    = PG_GETARG_CSTRING(2);
    int32 worker_id = PG_GETARG_INT32(3);

    char buffer[500];
    char *q2 =
        " insert into customer_proj2("
        "  c_w_id, c_d_id, c_id, c_last, c_first, "
        "  c_street_1, c_city, c_state, c_zip) "
        "(select "
        "  c_w_id, c_d_id, c_id, c_last, c_first, "
        "  c_street_1, c_city, c_state, c_zip "
        "from customer "
        "where c_w_id = %d "
        "  and c_d_id = %d "
        "  and c_last = '%s');%d";
    sprintf(buffer, q2, c_w_id, c_d_id, c_last, worker_id);

    exec_txns(worker_id, buffer);

    PG_RETURN_VOID();
}
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
//                          Load into PostgreSQL
// -----------------------------------------------------------------------------
// CREATE FUNCTION customer_proj1_q1(integer, integer, integer, integer) RETURNS
// integer
//      AS 'table_split_migration', 'customer_proj1_q1'
//      LANGUAGE C STRICT;

// CREATE FUNCTION customer_proj1_q2(integer, integer, varchar, integer) RETURNS
// integer
//      AS 'table_split_migration', 'customer_proj1_q2'
//      LANGUAGE C STRICT;

// CREATE FUNCTION customer_proj2_q1(integer, integer, integer, integer) RETURNS
// integer
//      AS 'table_split_migration', 'customer_proj2_q1'
//      LANGUAGE C STRICT;

// CREATE FUNCTION customer_proj2_q2(integer, integer, varchar, integer) RETURNS
// integer
//      AS 'table_split_migration', 'customer_proj2_q2'
//      LANGUAGE C STRICT;
// -----------------------------------------------------------------------------
