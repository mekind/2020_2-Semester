select a.hometown, b.nickname from Trainer a, CatchedPokemon b 
where a.id	= b.owner_id
and b.level = (
	select max(d.level) from Trainer c, CatchedPokemon d
    where c.id	= d.owner_id
    and c.hometown=a.hometown)
order by a.hometown;