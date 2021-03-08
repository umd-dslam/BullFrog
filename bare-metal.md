## BullFrog

### 1. Build BullFrog

```shell
git clone https://github.com/DSLAM-UMD/BullFrog

cd BullFrog

# set environment variables for bullfrog installation
# you can set these variables in `~/.bashrc`, and then `source ~/.bashrc`
# warn: Please pay attention to the settings of `POSTGRES_INSTALLDIR`.
export POSTGRES_INSTALLDIR=$PWD/dev
export LD_LIBRARY_PATH=$POSTGRES_INSTALLDIR/lib:$LD_LIBRARY_PATH
export PATH=$POSTGRES_INSTALLDIR/bin:$PATH
export PGDATA=$POSTGRES_INSTALLDIR/data

# create installation dir
mkdir -p $POSTGRES_INSTALLDIR
cd postgresql-11.0/
./configure --prefix=$POSTGRES_INSTALLDIR --enable-cassert --enable-debug CFLAGS="-ggdb -Og -g3 -fno-omit-frame-pointer"

# compile bullfrog
make -j8
# install bullfrog
make install
```

### 2. Deploy BullFrog

```shell
# create a new BullFrog database
rm -rf $PGDATA
initdb -D $PGDATA

# set postgresql configuration file
cp postgresql.conf $PGDATA/

# you can now start the database server
pg_ctl -D $PGDATA -o "-F -p 5433" start
pg_ctl -D $PGDATA status

createdb -h localhost -p 5433 tpcc
psql -h localhost -p 5433 tpcc -c "CREATE USER postgres WITH SUPERUSER PASSWORD 'postgres';"
# "ERROR:  role "postgres" already exists" This error is fine. 
```

## BullFrog OLTP-Benchmark

### 1. Build BullFrog OLTP-Benchmark

```shell
git clone https://github.com/DSLAM-UMD/BullFrog-Oltpbench

cd BullFrog-Oltpbench

ant bootstrap
ant resolve
ant build
```

### 2. Load TPC-C Dataset

```shell
./oltpbenchmark -b tpcc -c config/pgtpcc_lazy_proj.xml --create=true --load=true
```

### 3. Run TPC-C Benchmark with Online Schema Migration

> Please re-execute all commands in this step if the benchmark fails to run.

```shell
# Clean tuples in new tables with new schemas
# `clean_new_tables.sql` is located in the folder `BullFrog-Oltpbench`.
$ psql -h localhost -p 5433 tpcc -f clean_new_tables.sql

# Clean shared memory via restarting database
$ pg_ctl -D $PGDATA restart 

# run benchmark
$ ./oltpbenchmark -b tpcc -c config/pgtpcc_lazy_proj.xml  --execute=true -s 1 -o lazy_proj --port=5433 --bgthread=proj
```

## Stop BullFrog

```shell
pg_ctl -D $PGDATA stop
pg_ctl -D $PGDATA status
```
