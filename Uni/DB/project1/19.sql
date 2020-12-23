select count(distinct type) as '포켓몬 타입의 개수' from CatchedPokemon a, Pokemon b
where a.pid = b.id
and a.owner_id =
(select id from Trainer c, Gym d 
where c.id=d.leader_id
and d.city='Sangnok City');
