select name from Trainer 
where id in (
	select a.owner_id from CatchedPokemon a, CatchedPokemon b
    where a.id <> b.id 
    and a.pid = b.pid
    and a.owner_id = b.owner_id)
order by name;
