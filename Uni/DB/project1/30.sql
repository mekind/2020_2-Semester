select a.id, a.name, b.name, c.name from Pokemon a, Pokemon b, Pokemon c
where (a.id, b.id, c.id) in (
	select d.before_id, e.before_id, e.after_id from Evolution d, Evolution e 
    where d.after_id=e.before_id)
order by a.id;
