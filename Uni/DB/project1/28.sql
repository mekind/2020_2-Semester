select a.name, avg(b.level) as average from Trainer a, CatchedPokemon b
where a.id = b.owner_id 
and b.pid in (select id from Pokemon where type in ('Normal', 'Electric'))
group by a.name 
order by average asc;