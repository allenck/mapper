.mode column
.headers on

-- DO NOT EDIT HERE - LOAD EXTENSION - START ----
.load ./sqlite3ext_sqlfun.dll
-- DO NOT EDIT HERE - LOAD EXTENSION - END ------

select define_function('f',2,'select (?1+?2)/2');

.output stdout

drop table if exists prova;

create table prova(
	a               real, 
	b               real, 
	f_should_be     real, 
	f_calculated_is real, 
	constraint pk_prova primary key (a,b)
);

insert into prova(a,b,f_should_be) values ( 2,   2,   2);
insert into prova(a,b,f_should_be) values ( 1,   2, 1.5);
insert into prova(a,b,f_should_be) values (-1,   1,   0);
insert into prova(a,b,f_should_be) values ( 1, 2.2, 1.6);

update prova set f_calculated_is=f(a,b);

select a,b,f_should_be,f_calculated_is,(f_should_be-f_calculated_is) as error from prova;

.quit


