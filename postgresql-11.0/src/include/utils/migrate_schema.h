/*-------------------------------------------------------------------------
 *
 * migrate_schema.h
 *	  Definition of (and support for) schema migration functions.
 *
 *
 * Portions Copyright (c) 2020, UMD Database Group
 *
 * src/include/utils/migrate_schema.h
 *
 *-------------------------------------------------------------------------
 */

#ifndef MIGRATE_SCHEMA_H
#define MIGRATE_SCHEMA_H

#include "postgres.h"
#include "fmgr.h"
#include "storage/lwlock.h"
#include "nodes/pg_list.h"


#define LOCKBITPOS      0
#define MIGRATEBITPOS   1
#define SIZEOFWORD      (sizeof(uint64) * 8)
#define ELEMCOUNTINWORD (SIZEOFWORD / 2)

/* 
 * Note: The values NUMPAGES, NUMTUPLESPERPAGE & NUMTUPLESLASTPAGE should be updated for every new input table.
 */

/*
tpcc=# select count(*) from customer;
  count  
---------
 1500000
(1 row)

tpcc=# select count(*) from (select distinct (ctid::text::point)[0]::bigint from customer) t;
 count  
--------
 113505
(1 row)

tpcc=# select max(num_tuples) from (select (ctid::text::point)[0]::bigint, count(*) as num_tuples from customer group by (ctid::text::point)[0]::bigint) p;
 max 
-----
  15
(1 row)

tpcc=# select pid, num_tuples from (select (ctid::text::point)[0]::bigint as pid, count(*) as num_tuples from customer group by (ctid::t
  pid   | num_tuples 
--------+------------
 113504 |          4
(1 row)
*/

/*
 * lineitem
 */
#define NUMPAGES            113505
#define NUMTUPLESPERPAGE    15
#define NUMTUPLESLASTPAGE   4
#define NUMTUPLES           1500000
#define ACTUALTUPLES        1500000
#define BITMAPSIZE          (((NUMTUPLES * 2) + (SIZEOFWORD - 1)) / (SIZEOFWORD))

extern inline uint32 getwordid      (uint32 eid);
extern inline uint32 getlockbitid   (uint32 eid);
extern inline uint32 getmigratebitid(uint32 eid);

extern inline bool getkthbit        (uint64 word, uint32 k);
extern inline bool getlockbit       (uint64 *bitmap, uint32 eid);
extern inline void setlockbit       (uint64 *bitmap, uint32 eid);
extern inline void resetlockbit     (uint64 *bitmap, uint32 eid);
extern inline bool getmigratebit    (uint64 *bitmap, uint32 eid);
extern inline void setmigratebit    (uint64 *bitmap, uint32 eid);
extern inline bool getinprogbit     (uint64 *bitmap, uint32 eid);
extern inline void setinprogbit     (uint64 *bitmap, uint32 eid);
extern inline void resetinprogbit   (uint64 *bitmap, uint32 eid);

extern bool migrateflag;

extern uint64 tuplemigratecount;
extern uint32 count_inprogress;

extern uint64 *GlobalBitmap;
extern uint64 *PartialBitmap;
extern uint8 BitmapNum;

extern List *InProgLocalList0;
extern List *InProgLocalList1;

extern void InitGlobalBitmap(void);

#define MigrateBitmapPartition(hashcode) \
    ((hashcode) % NUM_MIGRATE_BITMAP_LOCKS)

#define MigrateBitmapPartitionLock(hashcode, i) \
    (&MainLWLockArray[MIGRATE_BITMAP_OFFSET + \
    MigrateBitmapPartition(hashcode) + i*NUM_MIGRATE_BITMAP_LOCKS].lock)

#define MigrateBitmapLockByIndex(i) \
	  (&MainLWLockArray[MIGRATE_BITMAP_OFFSET + (i)].lock)

#endif /* MIGRATE_SCHEMA_H */
