select type, count(*) as cnt from Pokemon
group by type
order by cnt asc, type asc;
