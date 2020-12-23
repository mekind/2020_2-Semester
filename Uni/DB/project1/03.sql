select avg(level) 
from CatchedPokemon as a
where owner_id in (select id from Trainer where hometown = 'Sangnok City')
and a.pid in (select id from Pokemon where type = 'Electric');





