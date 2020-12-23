select b.name, a.level, a.nickname from CatchedPokemon a, Pokemon b
where a.pid = b.id 
and a.nickname like 'A%'
and a.owner_id in ( select leader_id from Gym)
order by b.name desc;