create table artists (
	id int not null,
	name char(128) not null
)\g
create sequence on artists\g
create unique index artists_idx1 on artists (id)\g

create table albums (
	id int not null,
	name char(128) not null,
	artistid int
)\g
create sequence on albums\g
create unique index albums_idx1 on albums (id)\g
create index albums_idx2 on albums (artistid)\g

create table donators (
	id int not null,
	name char(14) not null
)\g
create sequence on donators\g
create unique index donators_idx1 on donators (id)\g
create unique index donators_idx2 on donators (name)\g

create table tracks (
	id int not null,
	name char(128) not null,
	genre int,
	albumid int,
	artistid int,
	donatorid int not null,
	donationday date not null
)\g
create sequence on tracks\g
create unique index tracks_idx1 on tracks (id)\g
create index tracks_idx2 on tracks (albumid)\g
create index tracks_idx3 on tracks (artistid)\g
create index tracks_idx4 on tracks (donatorid)\g


drop table artists\g
drop table albums\g
drop table tracks\g
drop table donators\g
