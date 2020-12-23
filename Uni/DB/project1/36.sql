select a.name from Trainer a, CatchedPokemon b
where a.id = b.owner_id
and b.pid in (select after_id from Evolution)
and b.pid not in (select before_id from Evolution)
order by a.name;
