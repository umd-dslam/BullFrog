#include "utils/migrate_schema.h"
#include "storage/shmem.h"

/* flag to indicate if a query is a part of a migration */
bool migrateflag = false;

/* count of the number of tuples migrated (per process) */
uint64 tuplemigratecount = 0;
uint32 count_inprogress = 0;

/* MigrateJoinHashTable : global hash table */
HTAB *MigrateJoinHashTable = NULL;

/* LocalJoinHashTable : local hash table */
HTAB *LocalJoinHashTable0 = NULL;
HTAB *LocalJoinHashTable1 = NULL;

void
InitLocalJoinHashTable(void)
{
	HASHCTL		ctl;
	int 		size;

	memset(&ctl, 0, sizeof(ctl));

	ctl.keysize = sizeof(LocalJoinHashKey);
	ctl.entrysize = sizeof(LocalJoinHashValue);

	size = 200;

	LocalJoinHashTable0 = hash_create("Local Join Hash Table",
										size, &ctl,
										HASH_ELEM | HASH_BLOBS);

	LocalJoinHashTable1 = hash_create("Local Join Hash Table",
										size, &ctl,
										HASH_ELEM | HASH_BLOBS);
}

void
InitMigrateJoinHashTable(void)
{
	HASHCTL		ctl;
	int 		size;

	memset(&ctl, 0, sizeof(ctl));

	ctl.keysize = sizeof(MigrateJoinHashKey);
	ctl.entrysize = sizeof(MigrateJoinHashValue);
	ctl.num_partitions = NUM_MIGRATE_JOIN_PARTITIONS;
	//ctl.hash = tag_hash;

	size = MIGRATE_JOIN_HASH_TABLE_SIZE + NUM_MIGRATE_JOIN_PARTITIONS;

	MigrateJoinHashTable = ShmemInitHash("Shared Migrate Join Hash Table",
											500000, size, &ctl,
											HASH_ELEM | HASH_BLOBS | HASH_PARTITION);

	printf ("MigrateJoinHashTable created.\n");
}

uint32
MigrateJoinHashCode(MigrateJoinHashKey *tagPtr)
{
    return get_hash_value(MigrateJoinHashTable, (void *) tagPtr);
}

bool
migratejoinhashtable_lookup(uint32 k1, uint32 k2, uint32 k3, uint32 k4, uint32 k5, uint32 k6, uint8 *hval)
{
	MigrateJoinHashKey key;
	MigrateJoinHashValue *hvalue;
	bool found;
	LWLock *partitionLock;
	uint32 hashcode;

	key.key1 = k1;
	key.key2 = k2;
	key.key3 = k3;
	key.key4 = k4;
	key.key5 = k5;
	key.key6 = k6;

	/* determine its hash code and partition lock ID */
	hashcode = MigrateJoinHashCode(&key);
	partitionLock = MigrateJoinPartitionLock(hashcode);

	LWLockAcquire(partitionLock, LW_SHARED);

	hvalue = (MigrateJoinHashValue *) hash_search_with_hash_value(MigrateJoinHashTable,
							    				(void *)&key, hashcode, HASH_FIND, &found);

	if (found)
	{
		*hval = hvalue->migrateByte;
	}

	LWLockRelease(partitionLock);

	return found;
}

bool
migratejoinhashtable_insert(uint32 k1, uint32 k2, uint32 k3, uint32 k4, uint32 k5, uint32 k6, uint8 *hval, bool setMigrate)
{
	MigrateJoinHashKey key;
	MigrateJoinHashValue *hvalue;
	bool found;
	LWLock *partitionLock;
	uint32 hashcode;

	key.key1 = k1;
	key.key2 = k2;
	key.key3 = k3;
	key.key4 = k4;
	key.key5 = k5;
	key.key6 = k6;

	/* determine its hash code and partition lock ID */
	hashcode = MigrateJoinHashCode(&key);
	partitionLock = MigrateJoinPartitionLock(hashcode);


	LWLockAcquire(partitionLock, LW_EXCLUSIVE);

	hvalue = (MigrateJoinHashValue *) hash_search_with_hash_value(MigrateJoinHashTable,
								    			(void *)&key, hashcode, HASH_ENTER, &found);

	if (found)
	{
		if (!setMigrate)
			*hval = hvalue->migrateByte;
		else
			hvalue->migrateByte = *hval;
	}
	else
		hvalue->migrateByte = *hval;

	LWLockRelease(partitionLock);

	return found;
}

bool
localjoinhashtable_lookup(uint32 k1, uint32 k2, uint32 k3, uint32 k4, uint32 k5, uint32 k6, uint8 *hval, uint8 id)
{
	LocalJoinHashKey key;
	LocalJoinHashValue *hvalue;
	HTAB* localhtable;
	bool found;

	key.key1 = k1;
	key.key2 = k2;
	key.key3 = k3;
	key.key4 = k4;
	key.key5 = k5;
	key.key6 = k6;
	
	if (id == 0) {
		localhtable = LocalJoinHashTable0;
	} else {
		localhtable = LocalJoinHashTable1;
	}

	hvalue = (LocalJoinHashValue *) hash_search(localhtable,
							    				(void *)&key, HASH_FIND, &found);

	if (found)
		*hval = hvalue->migrateByte;


	return found;
}

void
localjoinhashtable_insert(uint32 k1, uint32 k2, uint32 k3, uint32 k4, uint32 k5, uint32 k6, uint8 hval, uint8 id)
{
	LocalJoinHashKey key;
	LocalJoinHashValue *hvalue;
	HTAB* localhtable;
	bool found;

	key.key1 = k1;
	key.key2 = k2;
	key.key3 = k3;
	key.key4 = k4;
	key.key5 = k5;
	key.key6 = k6;

	if (id == 0) {
		localhtable = LocalJoinHashTable0;
	} else {
		localhtable = LocalJoinHashTable1;
	}

	hvalue = (LocalJoinHashValue *) hash_search(localhtable,
								    			(void *)&key, HASH_ENTER, &found);

	hvalue->migrateByte = hval;
}
