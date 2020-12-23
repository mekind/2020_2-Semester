select b.hometown, avg(a.level) as average from CatchedPokemon a, Trainer b 
where a.owner_id = b.id 
group by b.hometown
order by average asc;
