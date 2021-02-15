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
docker run --rm -it -d  --name bullfrog gangliao/bullfrog:latest bash
# Output:
#
# Unable to find image 'gangliao/bullfrog:latest' locally    
# latest: Pulling from gangliao/bullfrog
# 4007a89234b4: Pull complete
# 5dfa26c6b9c9: Pull complete
# 0ba7bf18aa40: Pull complete
# 4c6ec688ebe3: Pull complete
# 0c9429678a32: Pull complete
# bb929fc8a5f3: Pull complete
# Digest: sha256:2a19ce8ab950649cd77e8ad5a770d1e436faa788e89f7af817feb11dc1aab969
# Status: Downloaded newer image for gangliao/bullfrog:latest  

# Enters the container.
docker exec -u postgres -it bullfrog bash
# Output:
#
# postgres@39f01f1e6b93:/$
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

2. Runnning a TPC-C Benchmark where data is already loaded into the database.
    ```shell
    cd /home/postgres/BullFrog-Oltpbench
    # Clean tuples in new tables with new schemas
    psql -h localhost -p 5433 tpcc -f /home/postgres/BullFrog-Oltpbench/clean_new_tables.sql
    # Output:
    #
    # DROP TABLE
    # CREATE TABLE
    # DROP TABLE
    # CREATE TABLE
    # CREATE INDEX
    # CREATE INDEX

    # Clean shared memory via restarting database
    pg_ctl -D $PGDATA restart 
    # Output:
    #
    # waiting for server to shut down....
    # 2021-02-15 05:22:27.763 UTC [1208] LOG:  background worker "logical replication launcher" (PID 1214) exited with exit code 1
    # 2021-02-15 05:22:27.964 UTC [1210] LOG:  shutting down
    # 2021-02-15 05:22:28.557 UTC [1208] LOG:  database system is shut down
    #  done
    # server stopped
    # waiting for server to start.....2021-02-15 05:22:29.579 UTC [1275] LOG:  listening on IPv4 address "127.0.0.1", port 5433
    # 2021-02-15 05:22:29.579 UTC [1275] LOG:  could not bind IPv6 address "::1": Cannot assign requested address
    # 2021-02-15 05:22:29.579 UTC [1275] HINT:  Is another postmaster already running on port 5433? If not, wait a few seconds and retry.
    # 2021-02-15 05:22:29.579 UTC [1275] LOG:  listening on Unix socket "/tmp/.s.PGSQL.5433"
    # Shared Global Bitmap created!
    # 2021-02-15 05:22:29.863 UTC [1276] LOG:  database system was shut down at 2021-02-15 05:22:28 UTC
    # 2021-02-15 05:22:29.865 UTC [1275] LOG:  database system is ready to accept connections
    #  done
    # server started

    # run benchmark
    ./oltpbenchmark -b tpcc -c config/pgtpcc_lazy_proj.xml  --execute=true -s 1 -o lazy_proj --port=5433 --bgthread=proj
    # Output:
    # ...
    # 05:37:19,860 (ThreadBench.java:473) INFO  - TERMINATE :: Waiting for all terminals to finish ..
    # 05:37:20,010 (ThreadBench.java:534) INFO  - Attempting to stop worker threads and collect measurements
    # 05:37:20,012 (ThreadBench.java:255) INFO  - Starting WatchDogThread
    # 05:37:20,059 (DBWorkload.java:886) INFO  - ======================================================================
    # 05:37:20,059 (DBWorkload.java:887) INFO  - Rate limited reqs/s: Results(nanoSeconds=60000173489, measuredRequests=25774) = 429.56542458540093 requests/sec
    # 05:37:20,071 (DBWorkload.java:708) INFO  - Upload Results URL: com.oltpbenchmark.util.ResultUploader@4690b489
    # 05:37:20,073 (DBWorkload.java:741) INFO  - Output Raw data into file: results/lazy_proj.2.csv
    # 05:37:20,597 (DBWorkload.java:760) INFO  - Output summary data into file: results/lazy_proj.2.summary
    # 05:37:20,625 (DBWorkload.java:767) INFO  - Output DBMS parameters into file: results/lazy_proj.2.params
    # 05:37:20,647 (DBWorkload.java:774) INFO  - Output DBMS metrics into file: results/lazy_proj.2.metrics
    # 05:37:20,720 (DBWorkload.java:781) INFO  - Output experiment config into file: results/lazy_proj.2.expconfig
    # 05:37:20,806 (DBWorkload.java:798) INFO  - Output throughput samples into file: results/lazy_proj.2.res
    # 05:37:20,806 (DBWorkload.java:801) INFO  - Grouped into Buckets of 1 seconds
    ```

<details>
<summary>
<strong>Experiment Results</strong>
</summary>

- Note: We have simplified the experimental environment and dataset size, and the results may be different from the paper.

    ```bash
    time(sec), throughput(req/sec), avg_lat(ms), min_lat(ms), 25th_lat(ms), median_lat(ms), 75th_lat(ms), 90th_lat(ms), 95th_lat(ms), 99th_lat(ms), max_lat(ms), tp (req/s) scaled
    0,710.000,241.978,14.044,50.706,193.569,413.968,548.516,568.733,601.086,724.115,0.004
    1,746.000,902.760,587.814,729.106,915.356,1092.249,1108.183,1114.801,1141.786,1207.525,0.001
    2,683.000,1240.548,1091.175,1148.044,1219.232,1319.655,1410.800,1429.949,1480.312,1613.212,0.001
    3,696.000,1711.043,1435.643,1639.512,1739.834,1826.163,1836.991,1842.366,1888.606,2022.589,0.001
    4,711.000,2125.284,1832.018,2026.258,2182.180,2223.714,2268.524,2279.916,2347.618,2518.088,0.000
    5,692.000,2267.078,2218.248,2238.881,2256.438,2294.173,2313.620,2321.869,2376.706,2436.215,0.000
    6,649.000,2326.822,2244.838,2285.259,2314.824,2351.500,2399.937,2439.571,2496.754,2772.709,0.000
    7,699.000,2528.481,2437.650,2489.732,2520.076,2555.391,2581.153,2590.661,2674.092,3597.575,0.000
    8,672.000,2670.815,2564.836,2630.448,2658.847,2711.477,2767.316,2781.034,2849.431,2924.000,0.000
    9,675.000,2939.938,2780.157,2827.093,2922.401,3000.240,3173.273,3209.454,3260.319,3408.962,0.000
    10,713.000,3419.162,3219.188,3340.095,3371.313,3434.718,3673.629,3703.129,3835.155,3896.401,0.000
    11,699.000,4075.947,3778.317,3957.152,4034.453,4264.632,4290.512,4299.244,4317.849,4547.130,0.000
    12,698.000,4609.351,4280.767,4464.755,4606.739,4716.990,4862.700,4885.202,4949.873,5058.190,0.000
    13,774.000,5430.109,4906.281,5082.888,5357.865,5703.599,5928.799,6062.573,6117.289,6396.673,0.000
    14,749.000,6568.016,6106.774,6319.239,6586.167,6779.193,6976.512,7045.850,7127.549,7304.344,0.000
    15,712.000,7807.971,7099.286,7429.829,7828.936,8109.333,8337.742,8386.697,8479.885,8709.173,0.000
    16,695.000,9312.523,8484.144,8989.406,9373.225,9652.363,9792.985,9887.388,9966.185,10169.761,0.000
    17,701.000,10486.608,9951.092,10165.869,10418.555,10864.060,10930.075,10996.514,11066.397,11192.538,0.000
    18,693.000,11311.104,11049.190,11161.144,11368.447,11428.003,11442.661,11449.003,11519.775,11623.035,0.000
    19,743.000,11835.479,11429.284,11617.174,11883.032,12032.767,12123.835,12143.030,12243.945,12362.721,0.000
    20,743.000,12441.963,12166.276,12259.673,12413.117,12638.716,12737.166,12868.825,12887.709,13030.135,0.000
    21,750.000,13270.427,12870.730,13040.182,13269.480,13481.469,13571.008,13656.971,13728.295,13840.205,0.000
    22,678.000,13939.808,13693.769,13797.643,13952.730,14048.724,14079.676,14151.422,14223.166,14308.819,0.000
    23,550.000,14321.744,14186.573,14280.265,14327.604,14365.837,14378.413,14386.943,14534.145,14577.047,0.000
    24,517.000,14400.817,14356.157,14377.054,14389.813,14413.491,14427.971,14440.027,14661.117,14840.450,0.000
    25,424.000,14446.584,14400.244,14431.087,14439.772,14451.206,14463.621,14469.722,14621.516,14652.065,0.000
    26,586.000,14432.566,14351.361,14419.615,14432.563,14445.189,14455.068,14462.358,14581.474,14659.120,0.000
    27,362.000,14330.734,14267.744,14310.503,14322.197,14336.933,14350.905,14361.558,14538.056,14577.197,0.000
    28,379.000,14297.802,14256.790,14278.253,14290.664,14300.367,14312.114,14349.087,14509.028,14558.837,0.000
    29,368.000,14249.598,14207.492,14229.296,14238.926,14254.944,14281.493,14293.740,14463.247,14491.570,0.000
    30,363.000,14188.909,14088.866,14121.626,14209.828,14225.376,14241.730,14251.997,14466.845,14529.150,0.000
    31,518.000,14126.162,14069.034,14101.533,14116.017,14145.789,14164.315,14172.041,14313.275,14360.989,0.000
    32,514.000,14249.454,14158.681,14204.124,14225.007,14299.999,14320.929,14339.545,14477.864,14545.244,0.000
    33,250.000,14308.170,14197.253,14247.180,14311.549,14328.339,14372.984,14463.368,14620.581,14651.205,0.000
    34,357.000,14210.962,14168.968,14193.627,14202.122,14214.245,14224.480,14228.395,14430.907,14454.735,0.000
    35,328.000,14186.784,14135.466,14159.671,14172.591,14189.110,14226.300,14336.878,14389.564,14408.299,0.000
    36,420.000,14137.225,14091.972,14112.686,14130.707,14149.171,14168.584,14184.519,14340.017,14379.954,0.000
    37,433.000,14122.121,14074.854,14101.038,14114.853,14129.634,14150.169,14164.716,14325.766,14396.517,0.000
    38,416.000,14230.532,14147.361,14194.343,14215.804,14260.135,14286.540,14323.485,14467.883,14535.479,0.000
    39,450.000,14248.096,14193.856,14219.221,14234.916,14263.392,14297.132,14311.803,14449.613,14486.986,0.000
    40,331.000,14257.947,14198.248,14230.389,14251.091,14271.460,14287.413,14302.977,14492.968,14505.534,0.000
    41,379.000,14289.122,14218.884,14259.381,14287.435,14300.532,14314.663,14338.363,14528.725,14552.769,0.000
    42,414.000,14298.013,14244.810,14280.471,14293.301,14308.258,14320.975,14333.780,14501.730,14541.390,0.000
    43,384.000,14305.991,14243.008,14285.555,14303.272,14317.463,14332.280,14337.922,14525.288,14546.130,0.000
    44,507.000,14280.904,14210.035,14244.241,14262.527,14310.895,14346.422,14361.806,14466.475,14536.531,0.000
    45,443.000,14366.163,14326.845,14349.530,14359.403,14373.589,14383.614,14394.415,14555.123,14584.749,0.000
    ```

</details>

### III. Stop Database & Container

1. You may want to shut down the database for any reason:

    ```shell
    cd /home/postgres/BullFrog && ./shutdown.sh
    # Output:
    #
    # ++ source /home/postgres/.bashrc
    # +++ case $- in
    # +++ return
    # ++ pg_ctl -D /home/postgres/BullFrog/build/data stop
    # waiting for server to shut down.... done
    # server stopped
    # ++ pg_ctl -D /home/postgres/BullFrog/build/data status
    # pg_ctl: no server running
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
    # Scale Factor:  8.0

    # 21:08:26,184 (DBWorkload.java:312) INFO  - ======================================================================
    # 21:08:26,210 (DBWorkload.java:575) INFO  - Creating new TPCC database...
    # 21:08:26,446 (DBWorkload.java:577) INFO  - Finished!
    # 21:08:26,446 (DBWorkload.java:578) INFO  - ======================================================================
    # 21:08:26,446 (DBWorkload.java:601) INFO  - Loading data into TPCC database with 8 threads...
    # 21:50:02,260 (DBWorkload.java:605) INFO  - Finished!
    # 21:50:02,300 (DBWorkload.java:606) INFO  - ======================================================================
    # 21:50:02,302 (DBWorkload.java:646) INFO  - Skipping benchmark workload execution
    ```

</details>


