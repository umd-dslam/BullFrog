MODULES = table_split_migration

PG_CPPFLAGS = -I$(libpq_srcdir) -L$(shell $(PG_CONFIG) --pkglibdir) -lpq
SHLIB_LINK_INTERNAL = $(libpq)

PG_CONFIG = pg_config
PGXS = $(shell $(PG_CONFIG) --pgxs)
INCLUDEDIR = $(shell $(PG_CONFIG) --includedir-server)
include $(PGXS)

table_split_migration.so: table_split_migration.o
	cc -dynamic -bundle -flat_namespace -undefined suppress $(PG_CPPFLAGS) -o table_split_migration.so table_split_migration.o

table_split_migration.o: table_split_migration.c
	cc -o table_split_migration.o -c table_split_migration.c $(CFLAGS) $(PG_CPPFLAGS) -I$(INCLUDEDIR)

# https://www.christian-rossow.de/articles/PostgreSQL_C_language_functions_with_bytea.php

# If you are finished writing the C function, try compiling it until you don't get compiler-warnings anymore.
# Then you should copy the new shared object (.so) file to your pgsql library directory. 
# If you don't know where that is, check it out using (on Linux):
# [user@db ~]# pg_config --pkglibdir
# /usr/lib/pgsql

# We're about to finish. The last thing you need to do is creating the SQL function that bases
# on your created C function. In our example, this would work with:
# CREATE FUNCTION bytea_size(bytea) RETURNS integer
#	AS 'randomness', 'bytea_size'
# 	LANGUAGE C STRICT;
