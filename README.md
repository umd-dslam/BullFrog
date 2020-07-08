# BullFrog


> Online Schema Evolution via Lazy Evaluation

## Introduction

**BullFrog** is a system that performs immediate schema migration in a single step, without advanced warning or time to prepare. BullFrog supports all migrations, even those that are not backward-compatible with an existing schema. BullFrog achieves these immediacy and flexibility properties by implementing an instantaneous logical switch to a new schema, along with lazy migration of physical data. It implements concurrency control algorithms and data structures that support concurrent processing of schema migration operations with post-migration transactions, while ensuring exactly-once migration of all old data into the physical layout required by the new schema.BullFrog is an open-source universal database extension, with a working implementation over PostgreSQL, and with applicability to any database system via the use of plugins. Experiments using this prototype over a TPC-C based workload (supplemented to include schema migrations) show that BullFrog can achieve zero-downtime migration to non-trivial new schemas with near-invisible effects on transaction and latency.

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

