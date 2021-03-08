set -x

source /home/postgres/.bashrc

pg_ctl -D $PGDATA stop

pg_ctl -D $PGDATA status
