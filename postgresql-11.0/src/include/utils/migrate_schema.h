#ifndef MIGRATE_SCHEMA_H
#define MIGRATE_SCHEMA_H

#include "postgres.h"
#include "fmgr.h"
#include "storage/lwlock.h"
#include "utils/hsearch.h"

/*
tpcc=# select count(*) from order_line;
  count   
----------
 15000056
(1 row)

tpcc=# select count(*) from stock;
  count  
---------
 5000000
(1 row)

tpcc=# select count(*) from (select distinct (ctid::text::point)[0]::bigint from order_line) t;
 count  
--------
 185190
(1 row)

tpcc=# select max(num_tuples) from (select (ctid::text::point)[0]::bigint, count(*) as num_tuples from order_line group by (ctid::text::point)[0]::bigint) p;


 max 
-----
  81
(1 row)

tpcc=# 
tpcc=# 
tpcc=# select pid, num_tuples from (select (ctid::text::point)[0]::bigint as pid, count(*) as num_tuples from order_line group by (ctid::text::point)[0]::bigint) p where p.pid = 185189;
  pid   | num_tuples 
--------+------------
 185189 |         18
(1 row)
*/

#define NUMTUPLES 20000000
#define MIGRATE_AGG_HASH_TABLE_SIZE 20000000

extern inline uint32  getwordid       (uint32 eid);
extern inline uint32  getlockbitid    (uint32 eid);
extern inline uint32  getmigratebitid (uint32 eid);
extern inline bool    getkthbit       (uint64 word, uint32 k);
extern inline bool    getlockbit      (uint64 *bitmap, uint32 eid);
extern inline void    setlockbit      (uint64 *bitmap, uint32 eid);
extern inline bool    getmigratebit   (uint64 *bitmap, uint32 eid);
extern inline bool    getinprogbit    (uint64 *bitmap, uint32 eid);
extern inline void    setinprogbit    (uint64 *bitmap, uint32 eid);
extern inline void    resetinprogbit  (uint64 *bitmap, uint32 eid);

typedef struct migrateagghashkey
{
	uint32 key1;
	uint32 key2;
	uint32 key3;
} MigrateAggHashKey;

typedef struct migrateagghashvalue
{
	MigrateAggHashKey key;
	uint8 migrateByte;
} MigrateAggHashValue;

typedef struct localagghashkey
{
	uint32 key1;
	uint32 key2;
	uint32 key3;
} LocalAggHashKey;

typedef struct localagghashvalue
{
	LocalAggHashKey key;
	uint8 migrateByte;
} LocalAggHashValue;

extern bool migrateflag;

extern HTAB *MigrateAggHashTable;
extern HTAB *LocalIPAggHashTable0;
extern HTAB *LocalIPAggHashTable1;

extern uint32 count_inprogress;
extern uint64 tuplemigratecount;

extern void InitMigrateAggHashTable(void);
extern uint32 MigrateAggHashCode(MigrateAggHashKey *tagPtr);

extern void InitLocalIPAggHashTable(void);

extern bool migrateagghashtable_lookup(uint32 k1, uint32 k2, uint32 k3, uint8 *hval);
extern bool migrateagghashtable_insert(uint32 k1, uint32 k2, uint32 k3, uint8 *hval, bool setMigrate);

extern bool localagghashtable_ip_lookup(uint32 k1, uint32 k2, uint32 k3, uint8 *hval, uint8 id);
extern void localagghashtable_ip_insert(uint32 k1, uint32 k2, uint32 k3, uint8 hval, uint8 id);

/*
 * The shared migrate aggregate hash table is partitioned to reduce contention.
 * To determine which partition lock a given tag requires, compute the tag's
 * hash code with MigrateAggHashCode(), then apply MigrateAggPartitionLock().
 */

#define MigrateAggHashPartition(hashcode) \
    ((hashcode) % NUM_MIGRATE_AGG_PARTITIONS)

#define MigrateAggPartitionLock(hashcode) \
    (&MainLWLockArray[MIGRATE_AGG_OFFSET + MigrateAggHashPartition(hashcode)].lock)

#define MigrateAggPartitionLockByIndex(i) \
    (&MainLWLockArray[MIGRATE_AGG_OFFSET + (i)].lock)

#endif /* MIGRATE_SCHEMA_H */
