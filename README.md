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

<a href="https://asciinema.org/a/391400?speed=4" target="_blank"><img src="https://asciinema.org/a/391400.svg" width="90%" /></a>

