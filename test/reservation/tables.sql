create table timeslots (
	id int not null,
	day date,
	timeofday time,
	exercise int not null,
	tutor int,
	student int
)
create sequence on timeslots step 1 value 1
create unique index timeslotsidx1 on timeslots (id)
