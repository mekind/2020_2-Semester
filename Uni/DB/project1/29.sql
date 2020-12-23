select count(a.id) as cnt 
from CatchedPokemon a, Pokemon b
where a.pid = b.id
group by b.type
order by b.type asc;
