select avg(a.level) from CatchedPokemon a, Trainer b
where a.owner_id = b.id
and b.id in (select leader_id from Gym);
