set -x

source /home/postgres/.bashrc


mkdir -p $POSTGRES_INSTALLDIR

# /home/postgres/BullFrog/postgresql-11.0/configure --prefix=$POSTGRES_INSTALLDIR --enable-cassert --enable-debug CFLAGS="-ggdb -Og -g3 -fno-omit-frame-pointer"

make -j8

make install

cd /home/postgres/BullFrog/
