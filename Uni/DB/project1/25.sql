select distinct name from Pokemon 
where id in (
	select a.pid from CatchedPokemon a, Trainer b
    where a.owner_id = b.id 
    and b.hometown = 'Sangnok City')
    and id in(
    select a.pid from CatchedPokemon a, Trainer b
    where a.owner_id = b.id 
    and b.hometown = 'Brown City')
order by name;