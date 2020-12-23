select a.id as '트레이너의 id', count(*) as cnt 
from Trainer a, CatchedPokemon b 
where a.id = b.owner_id
group by a.id 
having count(*) = (
select distinct count(*) c from CatchedPokemon 
group by owner_id order by c desc limit 0,1)
order by  a.id asc;




