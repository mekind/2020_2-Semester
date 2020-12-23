select a.name as 'Trainer name', count(*) as 'Catched Pokemon' 
from Trainer a, CatchedPokemon b 
where a.id=b.owner_id 
group by a.id 
having count(*)>=3 
order by 'Catched Pokemon' desc;
