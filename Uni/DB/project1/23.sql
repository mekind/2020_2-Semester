select distinct a.name from Trainer a, CatchedPokemon b
where a.id = b.owner_id
and b.level <=10
order by a.name asc;