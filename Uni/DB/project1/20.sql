select a.name, count(*) as cnt from Trainer a, CatchedPokemon b 
where a.id = b.owner_id 
and a.hometown='Sangnok City' 
group by a.name
order by cnt asc;