select count(distinct a.pid)
from CatchedPokemon a, Trainer b
where a.owner_id = b.id 
and b.hometown ='Sangnok City';
