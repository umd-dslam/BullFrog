# BullFrog

<img src="https://i.postimg.cc/SRP8NbN3/Wechat-IMG647.png" width="12%" height="12%">

> Online Schema Evolution: A Lazy Approach

## Introduction

Growing, whether it’s a small business or a large company, data administrators faced with ongoing, continuous production changes requiring modifications to database tables. All existing online-schema-change tools slowly and incrementally migrate data to new tables, meanwhile replaying and propagating ongoing changes on the original tables synchronously or asynchronously to the new tables with danger of never keep up with migration, where new schema must have a primary key or at least unique key and renaming a column is not allowed. In addition to support a limited subset of alter table, there is a lack of more complex migration types such as projection, aggregate and joining.

To overcome these challenges people face around traditional IT or cloud environments, we introduce BullFrog that is the first system to lazily perform schema migrations on-the-fly in a parallel-aware, non-blocking and zero downtime way, where ongoing changes are directly applied on new tables without updating the original tables. BullFrog achieves high throughput, low latency and exactly-once migration guarantees. Experiments find that it can reduce latency by more than an order of magnitude relative to eager migration processing, while maintaining high throughput under contention. BullFrog is an open source universal database extension for developers and administrators to deal with online schema migration without the weaknesses of existing tools. Additionally, its functionality can be extended to any database systems by the use of plugins.

## Schema Migrations

Three main types of schema migrations:

- Projection Migration ([branch](https://github.com/DSLAM-UMD/Darwin))
  - It's defined as migrating a vertical subset from the columns of an old table that retains the unique rows to a new table.

- Aggregate Migration ([branch](https://github.com/DSLAM-UMD/Darwin/tree/migrate-aggregation-on-hashtable))
  - It's typically used in conjunction with aggregate functions to collapse multiple rows belonging to the same group into a single summary row in a new table and perform aggregate calculations to enhance those summary results.

- Joining Migration ([branch](https://github.com/DSLAM-UMD/Darwin/tree/migrate-join-on-hashtable))
  - It's defined as combining columns from one (self-join) or more tables based on the values of the common columns among the old tables, and migrating every combination of rows to a new table.
  
## TPC-C Benchmark

If you have any questions about schema migration on TPC-C Benchmark, feel free to contact me: `gangliao@cs.umd.edu`.

