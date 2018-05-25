drop table if exists entries;
create table entries (
  id integer primary key autoincrement,
  time text not null,
  data text not null
);
