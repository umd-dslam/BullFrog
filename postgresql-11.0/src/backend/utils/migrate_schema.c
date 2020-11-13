/*-------------------------------------------------------------------------
 *
 * migrate_schema.c
 *	  Definition of (and support for) schema migration functions.
 *
 *
 * Portions Copyright (c) 2020, UMD Database Group
 *
 * src/backend/utils/migrate_schema.c
 *
 *-------------------------------------------------------------------------
 */


#include "utils/migrate_schema.h"
#include "storage/shmem.h"

/* flag to indicate if a query is a part of a migration */
bool    migrateflag         = false;

/* count of the number of tuples migrated (per transaction) */
uint64  tuplemigratecount   = 0;

/* count of the number of tuples migration in progress (per transaction) */
uint32  count_inprogress    = 0;

/* global bitmap for indicating migration status of tuples */
uint64  *GlobalBitmap       = NULL;

/* bitmapNum for indicating bitmap tables */
uint8 BitmapNum = 0;
uint64 *PartialBitmap = NULL;

List    *InProgLocalList0;
List    *InProgLocalList1;

HTAB* TrackingHashTables[10] = {NULL};
HTAB* TrackingTable = NULL;

inline uint32 getwordid(uint32 eid)
{
	return (eid / ELEMCOUNTINWORD);
}

inline uint32 getlockbitid(uint32 eid)
{
	uint32 posinbyte = eid % ELEMCOUNTINWORD;
	return (posinbyte * 2 + LOCKBITPOS);
}

inline uint32 getmigratebitid(uint32 eid)
{
	uint32 posinbyte = eid % ELEMCOUNTINWORD;
	return (posinbyte * 2 + MIGRATEBITPOS);
}

inline bool getkthbit(uint64 word, uint32 k)
{
	return ((word & ((uint64)1 << k)) != 0);
}

inline bool getlockbit(uint64 *bitmap, uint32 eid)
{
	uint32 wordid    = getwordid(eid);
	uint32 lockbitid = getlockbitid(eid);
	return getkthbit(bitmap[wordid], lockbitid);
}

inline void setlockbit(uint64 *bitmap, uint32 eid)
{
	uint32 wordid    = getwordid(eid);
	uint32 lockbitid = getlockbitid(eid);
	bitmap[wordid]  |= ((uint64)1 << lockbitid);
}

inline void resetlockbit(uint64 *bitmap, uint32 eid)
{
	uint32 wordid    = getwordid(eid);
	uint32 lockbitid = getlockbitid(eid);
	bitmap[wordid]  &= ~((uint64)1 << lockbitid);
}

inline void setmigratebit(uint64 *bitmap, uint32 eid)
{
	uint32 wordid       = getwordid(eid);
	uint32 migratebitid = getmigratebitid(eid);
	bitmap[wordid]     |= ((uint64)1 << migratebitid);
}

inline bool getmigratebit(uint64 *bitmap, uint32 eid)
{
	uint32 wordid       = getwordid(eid);
	uint32 migratebitid = getmigratebitid(eid);
	return getkthbit(bitmap[wordid], migratebitid);
}

inline bool getinprogbit(uint64 *bitmap, uint32 eid)
{
	uint32 wordid   = eid / SIZEOFWORD;
	uint32 bitid    = eid % SIZEOFWORD;
	return getkthbit(bitmap[wordid], bitid);
}

inline void setinprogbit(uint64 *bitmap, uint32 eid)
{
	uint32 wordid   = eid / SIZEOFWORD;
	uint32 bitid    = eid % SIZEOFWORD;
	bitmap[wordid] |= ((uint64)1 << bitid);
}

inline void resetinprogbit(uint64 *bitmap, uint32 eid)
{
	uint32 wordid   = eid / SIZEOFWORD;
	uint32 bitid    = eid % SIZEOFWORD;
	bitmap[wordid] &= ~((uint64)1 << bitid);
}

void
InitGlobalBitmap(void)
{
	bool found;
	/* allocate bitmap from shared memory */
	GlobalBitmap = (uint64 *) ShmemInitStruct("Global Bitmap", (2 * BITMAPSIZE * sizeof(uint64)), &found);

	if (!found)
	{
		printf("Shared Global Bitmap created!\n");
		memset(GlobalBitmap, 0, (2 * BITMAPSIZE * sizeof(uint64)));
	}
}

// Initializing shmem hash table for storing in-progress identifiers
// corresponding to base tables in a migration.
void
InitTrackingHashTables()
{
    HASHCTL ctl;
    int size;

    memset(&ctl, 0, sizeof(ctl));

    ctl.keysize        = sizeof(hash_key_t);
    ctl.entrysize      = sizeof(hash_value_t);
    ctl.num_partitions = 1;

    // FIXME: TPC-C: # tuples in a migration <= 100
    size = 100;

	int worker_num = 10;
	for (int i = 0; i < worker_num; ++i)
	{
		char shmem_name[20];
    	sprintf(shmem_name, "%d", i);
		TrackingHashTables[i] = ShmemInitHash(shmem_name, size, size, &ctl, HASH_ELEM | HASH_BLOBS | HASH_PARTITION);
	}
}

bool
trackinghashtable_insert(uint32 hkey, uint8 *hval)
{
	hash_key_t key;
	hash_value_t *hvalue;
	bool found;
	uint32 hashcode;

	key.tid = hkey;
	hashcode = get_hash_value(TrackingTable, (void *) &key);
	hvalue = (hash_value_t *) hash_search_with_hash_value(TrackingTable,
				(void *) &key, hashcode, HASH_ENTER, &found);

	if (!found)
		hvalue->val = *hval;

	return !found;
}

bool
trackinghashtable_lookup(uint32 hkey)
{
	hash_key_t key;
	bool found;

	key.tid = hkey;

	hash_search(TrackingTable, (void *) &key, HASH_FIND, &found);

	return found;
}

void
trackinghashtable_delete(uint32 hkey)
{
	hash_key_t key;

	key.tid = hkey;

	hash_search(TrackingTable, (void *) &key, HASH_REMOVE, NULL);
}
