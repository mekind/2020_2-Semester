select name from Pokemon 
where type in (
select type from Pokemon group by type 
having count(*) >= (select distinct count(type) as cnt from Pokemon group by type order by cnt desc limit 1,1) )
order by name;
