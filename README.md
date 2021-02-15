## BullFrog: Online Schema Evolution via Lazy Evaluation

> This repository contains an experimental prototype of the system, which is not suitable for use in production.

## Overview

**BullFrog** is a system that performs immediate schema migration in a single step, without advanced warning or time to prepare, including backwards-incompatible migrations. It implements *concurrency control algorithms* and *data structures* to enable concurrent processing of schema migration operations with post-migration transactions, while ensuring *exactly-once* migration of all old data into the physical layout required by the new schema. 

BullFrog is an open source extension to PostgreSQL. Experiments using this prototype over a TPC-C based workload (supplemented to include schema migrations) show that BullFrog can achieve *zero-downtime* migration to non-trivial new schemas with *near-invisible* impact on transaction throughput and latency.

For more details, please see the upcoming blog posts or the BullFrog paper from recent and imminent [SIGMOD 2021](https://2021.sigmod.org/).
  
## Benchmark

We extended Oltp-bench framework to measure online schema migration. You can find the experimental code in [BullFrog-Oltpbench](https://github.com/DSLAM-UMD/BullFrog-Oltpbench). If you have any questions about online schema migration on TPC-C Benchmark, feel free to contact me.

## Quick Start

Using the following as a guide, we will walk you through the setup of BullFrog. To make it smoother, we record terminal sessions and share them on the web.

**[Deployment]**

<a href="https://asciinema.org/a/390474?speed=4" target="_blank"><img src="https://asciinema.org/a/390474.svg" width="70%" /></a>

**[Run Experiment]**

<a href="https://asciinema.org/a/390487?speed=4" target="_blank"><img src="https://asciinema.org/a/390487.svg" width="70%" /></a>

### I. Docker Image

This tutorial assumes you have a current version of Docker installed on your machine. If you do not have Docker installed, choose your preferred operating system below to download Docker:

- [Download Docker Desktop for Mac](https://desktop.docker.com/mac/stable/Docker.dmg)
- [Download Docker Desktop for Windows](https://desktop.docker.com/win/stable/Docker%20Desktop%20Installer.exe)
- [Install Docker Engine on Linux](https://docs.docker.com/engine/install/)

After you installed Docker, you can issue a command to pull our docker image and start a container:

```shell
# Pulls Docker image and run it on your local machine.
docker run --rm -it -d  --name bullfrog gangliao/bullfrog:latest
# Enters the container.
docker exec -u postgres -it bullfrog bash
```

### II. BullFrog Experiments

1. Rebooting a DB instance:

    ```shell
    # Deploys the postgres backend
    cd /home/postgres/BullFrog && ./deploy.sh
    # Output:
    #
    # +++ case $- in
    # +++ return
    # ++ rm -rf /home/postgres/BullFrog/build/data
    # ++ initdb -D /home/postgres/BullFrog/build/data
    # The files belonging to this database system will be owned by user "postgres".
    # This user must also own the server process.
    # 
    # The database cluster will be initialized with locale "C".
    # The default database encoding has accordingly been set to "SQL_ASCII".
    # The default text search configuration will be set to "english".
    # 
    # Data page checksums are disabled.
    # 
    # creating directory /home/postgres/BullFrog/build/data ... ok
    # creating subdirectories ... ok
    # selecting default max_connections ... 100
    # selecting default shared_buffers ... 128MB
    # selecting dynamic shared memory implementation ... posix
    # creating configuration files ... ok
    # running bootstrap script ... Shared Global Bitmap created!
    # ok
    # performing post-bootstrap initialization ... ok
    # syncing data to disk ... ok
    # 
    # WARNING: enabling "trust" authentication for local connections
    # You can change this by editing pg_hba.conf or using the option -A, or
    # --auth-local and --auth-host, the next time you run initdb.
    # 
    # Success. You can now start the database server using:
    # 
    #     pg_ctl -D /home/postgres/BullFrog/build/data -l logfile start
    # 
    # ++ cp /home/postgres/postgresql.conf /home/postgres/BullFrog/build/data/
    # ++ pg_ctl -D /home/postgres/BullFrog/build/data -o '-F -p 5433' start
    ```

2. Reloading TPC-C Dataset: Some tables may take an unusually long time (**~30-50 minutes**) to load in the docker container.

    ```shell
    cd /home/postgres/BullFrog-Oltpbench

    # Loads TPC-C dataset
    ./oltpbenchmark -b tpcc -c config/pgtpcc_lazy_proj.xml --create=true --load=true --port=5433

    # Output:
    #
    # 21:08:25,517 (DBWorkload.java:222) INFO  - Enable on-conflict clause: false
    # 21:08:26,184 (DBWorkload.java:311) INFO  - ======================================================================

    # Benchmark:     TPCC {com.oltpbenchmark.benchmarks.tpcc.TPCCBenchmark}
    # Configuration: config/pgtpcc_lazy_proj.xml
    # Type:          POSTGRES
    # Driver:        org.postgresql.Driver
    # URL:           jdbc:postgresql://localhost:5433/tpcc
    # Isolation:     TRANSACTION_SERIALIZABLE
    # Scale Factor:  50.0

    # 21:08:26,184 (DBWorkload.java:312) INFO  - ======================================================================
    # 21:08:26,210 (DBWorkload.java:575) INFO  - Creating new TPCC database...
    # 21:08:26,446 (DBWorkload.java:577) INFO  - Finished!
    # 21:08:26,446 (DBWorkload.java:578) INFO  - ======================================================================
    # 21:08:26,446 (DBWorkload.java:601) INFO  - Loading data into TPCC database with 8 threads...
    # 21:50:02,260 (DBWorkload.java:605) INFO  - Finished!
    # 21:50:02,300 (DBWorkload.java:606) INFO  - ======================================================================
    # 21:50:02,302 (DBWorkload.java:646) INFO  - Skipping benchmark workload execution
    ```

3. Runnning a TPC-C Benchmark where data is already loaded into the database.
    ```shell
    # Clean tuples in new tables with new schemas
    psql -h localhost -p 5433 tpcc -f /home/postgres/BullFrog-Oltpbench/clean_new_tables.sql

    # Clean shared memory via restarting database
    pg_ctl -D $PGDATA restart 

    # run benchmark
    ./oltpbenchmark -b tpcc -c config/pgtpcc_lazy_proj.xml  --execute=true -s 1 -o lazy_proj --port=5433 --bgthread=proj
    ```

### III. Stop Database & Container

1. You may want to shut down the database for any reason:

    ```shell
    cd /home/postgres/BullFrog && ./shutdown.sh
    ```

2. You may stop a running container when you finish the tutorial:

    ```shell
    docker stop bullfrog
    ```


<details>
<summary>
<strong>Advanced Options</strong>
</summary>

1.  If you changed the codebase, you must re-build BullFrog and its OLTP-Benchmark. There are many configuration files under <a href="https://github.com/DSLAM-UMD/BullFrog-Oltpbench/tree/master/config">BullFrog-Oltpbench/config</a>. You can pick any of them if it relates to TPC-C, but you have to use <i>git checkout</i> to switch BullFrog's branches and build BullFrog first.

    ```shell
    # 1. build BullFrog
    cd /home/postgres/BullFrog && ./build.sh

    # 2. build oltp-benchmark
    cd /home/postgres/BullFrog-Oltpbench && ./build.sh
    ```
</details>


