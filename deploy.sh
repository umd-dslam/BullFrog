set -x

source /home/postgres/.bashrc

# create a new PostgreSQL database cluster
# rm -rf $PGDATA
# initdb -D $PGDATA

# you can now start the database server
cp /home/postgres/postgresql.conf $PGDATA/
pg_ctl -D $PGDATA -o "-F -p 5433" start
pg_ctl -D $PGDATA status

# create database: (we assume here that you used post number as 5432 above)
# createdb -h localhost -p 5433 tpcc

# create role
# psql -h localhost -p 5433 tpcc -c "CREATE USER postgres WITH SUPERUSER PASSWORD 'postgres';" || true
