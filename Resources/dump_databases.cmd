.open berlinerstrassenbahn.sqlite3
.output berlinerstrassenbahn_dump.sql
.dump
.output stdout
select "berlinerstrassenbahn backed up!";
.open cincinnati.sqlite3
.output cincinnati_dump.sql
.dump
.output stdout
select "cincinnati backed up!";

.open cleveland.sqlite3
.output cleveland_dump.sql
.dump
.output stdout
select "cleveland backed up!";

.open indianapolis.sqlite3
.output indianapolis_dump.sql
.dump
.output stdout
select "indianapolis backed up!";

.open louisville.sqlite3
.output Louisville_dump.sql
.dump
.output stdout
select "louisville backed up!";

.open StLouis.sqlite3
.output StLouis_dump.sql
.dump
.output stdout
select "StLouis backed up!";
.exit
