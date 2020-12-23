select distinct b.name, b.type from CatchedPokemon a, Pokemon b 
where a.pid = b.id
and a.level >=30
order by b.name;
