select a.name, sum(b.level) from Trainer a, CatchedPokemon b
where a.id = b.owner_id
group by a.id
having sum(b.level) = (
	select sum(level) as cnt from CatchedPokemon group by owner_id order by cnt desc limit 0,1)
order by a.name;