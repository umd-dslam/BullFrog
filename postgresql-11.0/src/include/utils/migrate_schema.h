

#ifndef MIGRATE_SCHEMA_H
#define MIGRATE_SCHEMA_H

#include "postgres.h"
#include "fmgr.h"
#include "storage/lwlock.h"
#include "utils/hsearch.h"

#define MIGRATE_JOIN_HASH_TABLE_SIZE 8000000

typedef struct migratejoinhashkey
{
	uint32 key1;
	uint32 key2;
	uint32 key3;
	uint32 key4;
	uint32 key5;
	uint32 key6;
} MigrateJoinHashKey;

typedef struct migratejoinhashvalue
{
	MigrateJoinHashKey key;
	uint8 migrateByte;
} MigrateJoinHashValue;

typedef struct localjoinhashkey
{
	uint32 key1;
	uint32 key2;
	uint32 key3;
	uint32 key4;
	uint32 key5;
	uint32 key6;
} LocalJoinHashKey;

typedef struct localjoinhashvalue
{
	LocalJoinHashKey key;
	uint8 migrateByte;
} LocalJoinHashValue;

extern bool migrateflag;

extern uint32 count_inprogress;
extern uint64 tuplemigratecount;

extern HTAB *MigrateJoinHashTable;
extern HTAB *LocalJoinHashTable0;
extern HTAB *LocalJoinHashTable1;

extern void InitMigrateJoinHashTable(void);
extern uint32 MigrateJoinHashCode(MigrateJoinHashKey *tagPtr);

extern void InitLocalJoinHashTable(void);
extern bool localjoinhashtable_lookup(uint32 k1, uint32 k2, uint32 k3, uint32 k4, uint32 k5, uint32 k6, uint8 *hval, uint8 id);
extern void localjoinhashtable_insert(uint32 k1, uint32 k2, uint32 k3, uint32 k4, uint32 k5, uint32 k6, uint8 hval, uint8 id);

extern bool migratejoinhashtable_lookup(uint32 k1, uint32 k2, uint32 k3, uint32 k4, uint32 k5, uint32 k6,uint8 *hval);
extern bool migratejoinhashtable_insert(uint32 k1, uint32 k2, uint32 k3, uint32 k4, uint32 k5, uint32 k6,uint8 *hval, bool setMigrate);

/*
 * The shared migrate join hash table is partitioned to reduce contention.
 * To determine which partition lock a given tag requires, compute the tag's
 * hash code with MigrateJoinHashCode(), then apply MigrateJoinPartitionLock().
 */

#define MigrateJoinHashPartition(hashcode) \
     ((hashcode) % NUM_MIGRATE_JOIN_PARTITIONS)
#define MigrateJoinPartitionLock(hashcode) \
     (&MainLWLockArray[MIGRATE_JOIN_OFFSET + \
		MigrateJoinHashPartition(hashcode)].lock)
#define MigrateJoinPartitionLockByIndex(i) \
     (&MainLWLockArray[MIGRATE_JOIN_OFFSET + (i)].lock)


#endif /* MIGRATE_SCHEMA_H */

