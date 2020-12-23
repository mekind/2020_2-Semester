select type from Pokemon 
where id in (select before_id from Evolution)
group by type
having count(*) >= 3 
order by type desc;
