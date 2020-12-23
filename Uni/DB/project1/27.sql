select a.name, max(b.level) from Trainer a, CatchedPokemon b
where a.id = b.owner_id
group by a.id
having count(*)>=4
order by a.name;
