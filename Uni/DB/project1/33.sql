select sum(a.level) as 'Matis 가 잡은 포켓몬들의 레벨의 총합' from CatchedPokemon a, Trainer b
where a.owner_id = b.id 
and b.name = 'Matis' ;
