select distinct b.name from CatchedPokemon a, Pokemon b 
where a.pid = b.id 
and a.nickname like '% %'
order by b.name desc;