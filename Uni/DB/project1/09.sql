select a.name as '관장 이름', avg(b.level)  as '평균레벨'
from Trainer a, CatchedPokemon b 
where a.id = b.owner_id
and a.id in (select leader_id from Gym)
group by a.id
order by a.name asc;