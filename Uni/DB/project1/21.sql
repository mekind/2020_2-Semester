select a.name, count(*) as cnt from Trainer a, CatchedPokemon b
where a.id = b.owner_id 
and a.id in (select leader_id from Gym)
group by a.id
order by a.name asc;
