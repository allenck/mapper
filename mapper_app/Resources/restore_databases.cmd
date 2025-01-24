.open berlinerstrassenbahn.sqlite3
.read berlinerstrassenbahn_dump.sql
select "berlinerstrassenbahn restored";

.open cincinnati.sqlite3
.read cincinnati_dump.sql
select "cincinnati restored";

.open cleveland.sqlite3
.read cleveland_dump.sql
select "cleveland restored";

.open indianapolis.sqlite3
.read indianapolis_dump.sql
select "indianapolis restored";

.open louisville.sqlite3
.read Louisville_dump.sql
select "louisville restored";

.open StLouis.sqlite3
.read StLouis_dump.sql
select "StLouis restored";

.exit
