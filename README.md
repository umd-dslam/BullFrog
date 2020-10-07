## BullFrog: Online Schema Evolution via Lazy Evaluation

## Introduction

**BullFrog** is a system that performs immediate schema migration in a single step, without advanced warning or time to prepare. BullFrog supports all migrations, even those that are not backward-compatible with an existing schema. BullFrog achieves these immediacy and flexibility properties by implementing an instantaneous logical switch to a new schema, along with lazy migration of physical data. It implements concurrency control algorithms and data structures that support concurrent processing of schema migration operations with post-migration transactions, while ensuring exactly-once migration of all old data into the physical layout required by the new schema.

BullFrog is an open-source universal database extension, with a working implementation over PostgreSQL, and with applicability to any database system via the use of plugins. Experiments using this prototype over a TPC-C based workload (supplemented to include schema migrations) show that BullFrog can achieve zero-downtime migration to non-trivial new schemas with near-invisible effects on transaction and latency.

## Schema Migrations

BullFrog provides comprehensive support for three major types of schema migration: `projections`, `aggregations`, and `joins`; other migration patterns are expressed using these primitives; e.g., table splitting can be expressed via projection operations.

Three main types of schema migrations in the experiments:

- Table split migration ([branch](https://github.com/DSLAM-UMD/Darwin))

- Aggregate migration ([branch](https://github.com/DSLAM-UMD/Darwin/tree/migrate-aggregation-on-hashtable))

- Join migration ([branch](https://github.com/DSLAM-UMD/Darwin/tree/migrate-join-on-hashtable))
  
## TPC-C Benchmark

We extended Oltp-bench framework to measure lazy schema migration: https://github.com/DSLAM-UMD/BullFrog-Oltpbench

If you have any questions about schema migration on TPC-C Benchmark, feel free to contact us.

