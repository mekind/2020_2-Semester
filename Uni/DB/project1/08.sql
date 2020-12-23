select avg(level) as 'Red가 잡은 포켓몬의 평균레벨' from CatchedPokemon where owner_id = (select id from Trainer where name='Red');
