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
	GlobalBitmap = (uint64 *) ShmemInitStruct("Global Bitmap", (3 * BITMAPSIZE * sizeof(uint64)), &found);

	if (!found)
	{
		printf("Shared Global Bitmap created!\n");
		memset(GlobalBitmap, 0, (3 * BITMAPSIZE * sizeof(uint64)));
	}
}
