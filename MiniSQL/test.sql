create table student (
	sid char(20),
	name char(20) unique,
	age int,
	gender char(10),
	primary key(sid)
);

create index score on student(score);

create index age on student(age);

select * from student;

insert into student values("G701", "Ma RunJie", 19, "Male");
insert into student values("G702", "Zeng ZhiHua", 20, "Male");
insert into student values("G703", "Xiao ZhiQing", "20", "Female");

select name from student;

select * from student;

select * from student where sid = "G703";

select * from student where gender = "Male";

select * from student where age >= 20;

delete from student where sid = "G703";

delete from student;

select * from student;

drop index age;