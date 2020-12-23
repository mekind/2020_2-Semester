select b.name, b.id from CatchedPokemon a, Pokemon b
where a.pid = b.id
and a.owner_id in 
(select id from Trainer
where hometown='Sangnok City')
order by b.id asc;